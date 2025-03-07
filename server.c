#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <semaphore.h> 
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <pthread.h>
#include "data.h"

// password for username admin: pass
// password for username user: password

int bookfd, userfd, user_bookfd;
int bookNumber = 0;
int userNumber = 0;
int putBook(Book);
int putUser(User);

sem_t user_mutex, book_mutex, ub_mutex;


int init() {
    bookfd = open("books.dat", O_RDWR | O_CREAT, 0666);
    userfd = open("user.dat", O_RDWR | O_CREAT, 0666);
    user_bookfd = open("user_books.dat", O_RDWR | O_CREAT, 0666);
    int off = lseek(bookfd, 0, SEEK_END);
    bookNumber = off / sizeof(Book);
    off = lseek(userfd, 0, SEEK_END);
    userNumber = off / sizeof(User);
    sem_init(&user_mutex, 0, 1);
    sem_init(&ub_mutex, 0, 1);
    sem_init(&book_mutex, 0, 1);

    // hardcoding a book
    // Book book;
    // strcpy(book.author, "author");
    // strcpy(book.name, "name");
    // book.valid = 1;
    // book.quantity = 5;
    // book.maxQuantity = 5;
    // printf("putBook : %d",putBook(book));
    // User u;
    // u.isAdmin = 1;
    // strcpy(u.name, "admin");
    // strcpy(u.password, "pass");
    // printf("putUser : %d",putUser(u));
    return 0;
}



User *getUserByName(char *name) {
    lseek(userfd, 0, SEEK_SET);
    User *user = malloc(sizeof(User));
    
    while (read(userfd, user, sizeof(User)) > 0) {
        if (!strcmp(name , user->name)) {
            return user;
        }
    }
    free(user);
    return NULL;
}

void getAllAvailableUsers(int sd) {
    sem_wait(&user_mutex);
    lseek(userfd, 0, SEEK_SET);
    User * users = malloc(sizeof(User) * userNumber);
    User user;
    int n = 0;
    while (read(userfd, &user, sizeof(User)) > 0)
        users[n++] = user;
    sem_post(&user_mutex);
    write(sd, &n, sizeof(int));
    for (int i = 0; i < n; i++) {
        write(sd, users + i, sizeof(users[i]));
    }

    free(users);
}

Book *getBookByID(int bookid) {
    lseek(bookfd, 0, SEEK_SET);
    Book *book = malloc(sizeof(Book));
    
    while (read(bookfd, book, sizeof(Book)) > 0) {
        if (book->id == bookid && book->valid) {
            return book;
        }
    }
    free(book);
    return NULL;
}

void getAllAvailableBooks(int sd) {
    lseek(bookfd, 0, SEEK_SET);
    Book * books = malloc(sizeof(Book) * bookNumber);
    Book book;
    int n = 0;
    
    while (read(bookfd, &book, sizeof(Book)) > 0)
        books[n++] = book;
    write(sd, &n, sizeof(int));
    for (int i = 0; i < n; i++) {
        write(sd, books + i, sizeof(books[i]));
    }
    free(books);
}

int putUser(User user) {
    lseek(userfd, 0, SEEK_END);
    
    if (write(userfd, &user, sizeof(user)) != -1) {
        userNumber++;
        return 0;
    }
    return 1;
}

int putBook(Book book) {
    lseek(bookfd, 0, SEEK_END);
    book.id = ++bookNumber;
    if (book.quantity < 0 || book.quantity > book.maxQuantity) return 1;
    if (write(bookfd, &book, sizeof(book)) != -1)
        return 0;
    bookNumber--;
    return 1;
}

int updateBook(int bookId, char *newName, int newQuatity, char *newAuthor) {
    Book book;
    
    if (newQuatity < 0)  {
        return 1;
    }
    lseek(bookfd, 0, SEEK_SET);
    while (read(bookfd, &book, sizeof(book)) != -1) {
        if (book.id == bookId && book.valid) {
            if (book.maxQuantity < newQuatity) break;
            lseek(bookfd, -sizeof(Book), SEEK_CUR);
            strcpy(book.name, newName);
            strcpy(book.author, newAuthor);
            book.quantity = newQuatity;
            write(bookfd, &book, sizeof(book));
            return 0;
        }
    }
    return 1;
}

int borrowBook(int bookId, char *name) {
    Book * book = getBookByID(bookId);
    User * user = getUserByName(name);
    if (book == NULL || user == NULL) {
        printf("invalid user or book\n");
        return 1;
    }
    UserBook ub;
    ub.bookId = bookId;
    ub.valid = 1;
    strcpy(ub.name, name);
    lseek(user_bookfd, 0, SEEK_END);
    
    if (write(user_bookfd, &ub, sizeof(ub)) != -1) {
        if (updateBook(book->id, book->name, book->quantity - 1, book->author) == 0) {
            free(book); free(user);
            return 0;
        }
        free(book); free(user);
        return 1;
    }
    free(book); free(user);
    return 1;
}

int returnBook(int bookId, char *name) {
    UserBook ub;
    strcpy(ub.name, name);
    Book * book = getBookByID(bookId);
    User *user = getUserByName(name);
    if (user == NULL || book == NULL) return 1;

    
    lseek(user_bookfd, 0, SEEK_SET);
    while (read(user_bookfd, &ub, sizeof(ub)) > 0) {
        if ((ub.bookId == bookId) && (!strcmp(ub.name, name)) && ub.valid) {
            ub.valid = 0;
            lseek(user_bookfd, -sizeof(UserBook), SEEK_CUR);
            write(user_bookfd, &ub, sizeof(ub));
            int retStat = updateBook(bookId, book->name, book->quantity + 1, book->author);
            free(book); free(user);
            return retStat;
        }
    }
    free(book); free(user);
    return 1;
}

int deleteBook(int bookId) {
    Book * book = getBookByID(bookId);
    if (book == NULL) return 1;

    // critical section
    
    lseek(bookfd, 0, SEEK_SET);
    while (read(bookfd, book, sizeof(Book)) > 0) {
        if (book->id == bookId && book->valid && book->quantity == book->maxQuantity) { // dont allow to delete when someone has borrowed that book
            // set book->valid = 0;
            lseek(bookfd, -sizeof(Book), SEEK_CUR);
            book->valid = 0;
            write(bookfd, book, sizeof(Book));
            free(book);
            return 0;
        }
    }
    free(book);
    return 1;
}

int login(int sd) {
    printf("Login..\n");
    User user;
    read(sd, &user, sizeof(user));
    sem_wait(&user_mutex);
    User *u = getUserByName(user.name);
    sem_post(&user_mutex);
    int sig;
    if (u == NULL) {
        printf("Please Signup\n");
        sig = SIGNUP;
        write(sd, &sig, sizeof(int));
        free(u);
        return 1;
    }
    else if (strcmp(u->password,user.password)) {
        printf("Wrong password\n");
        sig = SERVER_FAILURE;
        write(sd, &sig, sizeof(int));
        free(u);
        return 1;
    } else if (u->isAdmin) {
        printf("Admin logged in\n");
        sig = ADMIN_LOGIN;
        write(sd, &sig, sizeof(int));
        printf("logged in successfully\n");
        free(u);
        return 0;
    }
    free(u);
    sig = SERVER_SUCCESS;
    write(sd, &sig, sizeof(int));
    printf("logged in successfully\n");
    return 0;
}

int signup(int sd) {
    User user;
    User *u;
    read(sd, &user, sizeof(user));
    int sig;
    sem_wait(&user_mutex);
    if ((u = getUserByName(user.name)) != NULL) {
        sig = LOGIN;
        free(u);
        write(sd, &sig, sizeof(int));
        sem_post(&user_mutex);
        return LOGIN;
    }
    if (putUser(user) == 0) {
        sig = SERVER_SUCCESS;
        write(sd, &sig, sizeof(int));
        printf("Wrote user to db %s\n", user.name);
    }
    else {
        sig = SERVER_FAILURE;
        write(sd, &sig, sizeof(int));
        printf("Error occured\n");
        sem_post(&user_mutex);
        return 1;
    }
    sem_post(&user_mutex);
    return 0;
}

void handleBorrowBooks(int sd) {
    int bookId;
    char userName[SIZE];
    read(sd, &bookId, sizeof(int));
    read(sd, userName, sizeof(userName));
    sem_wait(&book_mutex);
    sem_wait(&ub_mutex);
    // getchar();
    int ret = borrowBook(bookId, userName);
    sem_post(&book_mutex);
    sem_post(&ub_mutex);
    write(sd, &ret, sizeof(int));
}

void handleReturnBooks(int sd) {
    int bookId;
    char userName[SIZE];
    read(sd, &bookId, sizeof(int));
    read(sd, userName, sizeof(userName));
    sem_wait(&book_mutex);
    sem_wait(&ub_mutex);
    // getchar();
    int ret = returnBook(bookId, userName);
    sem_post(&book_mutex);
    sem_post(&ub_mutex);
    write(sd, &ret, sizeof(int));
}

void handleGetBook(int sd) {
    int bookId = 0;
    read(sd, &bookId, sizeof(int));
    sem_wait(&book_mutex);
    // getchar();
    Book *book = getBookByID(bookId);
    sem_post(&book_mutex);
    int valid = !(book == NULL);
    write(sd, &valid, sizeof(int));
    if (valid) write(sd, book, sizeof(Book));
    free(book);
}

void handlePutBook(int sd) {
    Book book;
    read(sd, &book, sizeof(book));
    sem_wait(&book_mutex);
    // getchar();
    int ret = putBook(book);
    sem_post(&book_mutex);
    write(sd, &ret, sizeof(int));
}

void handleDeleteBook(int sd) { // doesnt delete
    int bookId;
    read(sd, &bookId, sizeof(int));
    printf("Book id: %d", bookId);
    sem_wait(&book_mutex);
    // getchar();
    int ret = deleteBook(bookId);
    sem_post(&book_mutex);
    write(sd, &ret, sizeof(int));
}

void handleUpdateBook(int sd) {
    Book book;
    read(sd, &book, sizeof(book));
    sem_wait(&book_mutex);
    // getchar();
    int ret = updateBook(book.id, book.name, book.quantity, book.author);
    sem_post(&book_mutex);
    write(sd, &ret, sizeof(int));
}

void logout(int sd) {
    printf("logout...\n");
    int ret;
    close(sd);
    pthread_exit(&ret);
}

int navigate(int *nsd) {
    int sd = *nsd;
    while (1) {
        int action;
        read(sd, &action, sizeof(int));

        // printf("Action: %d\n", action);
        switch (action) {
            case SIGNUP: signup(sd); break;
            case LOGIN: login(sd); break;
            case GET_BOOKS: getAllAvailableBooks(sd); break;
            case BORROW: handleBorrowBooks(sd); break;
            case RETURN: handleReturnBooks(sd); break;
            case GET_BOOK: handleGetBook(sd); break;
            case ADD_BOOK: handlePutBook(sd); break;
            case DELETE_BOOK: handleDeleteBook(sd); break;
            case UPDATE_BOOK: handleUpdateBook(sd); break;
            case GET_USERS: getAllAvailableUsers(sd); break;
            default: logout(sd); break;
        }
    }
    return 0;
}

int main() {
    struct sockaddr_in serv, cli;
    init();
    int sd = socket (AF_INET, SOCK_STREAM, 0);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = INADDR_ANY;  // no restrictions on clinet addr
    serv.sin_port = htons (PORT_NUMBER);

    printf("Server Running on port: %d\n", PORT_NUMBER);
    bind (sd, (struct sockaddr *)&serv, sizeof (serv));
    int size = sizeof(cli);
    listen (sd, 5);
    
    while (1) {
        int nsd = accept (sd, (struct sockaddr *)&cli, &size);
        pthread_t ptid;
        pthread_create(&ptid, NULL, (void *)&navigate, &nsd);
    }

    close(sd);
}