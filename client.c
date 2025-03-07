#include <stdio.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include "data.h"


User globalUser;
int isAuth = 0;

void getBooks(int sd) {
    int sig = GET_BOOKS;
    write(sd, &sig, sizeof(int));
    int n = 0;
    read(sd, &n, sizeof(int));
    Book book;
    for (int i = 0; i < n; i++) {
        read(sd, &book, sizeof(book));
        if (book.valid)
            printf("Name: %s, Id: %d, Quantity: %d, Author: %s\n", book.name, book.id, book.quantity, book.author);
    } 
}

void getUsers(int sd) {
    int sig = GET_USERS;
    write(sd, &sig, sizeof(int));
    int n = 0;
    read(sd, &n, sizeof(int));
    User user;
    for (int i = 0; i < n; i++) {
        read(sd, &user, sizeof(user));
        printf("Name: %s, Admin: %s\n", user.name, user.isAdmin ? "True" : "False");
    } 
}

void signup(int sd) {
    User user;
    printf("Enter Username: ");
    scanf("%s",user.name);
    char *password = getpass("Enter password: ");
    strcpy(user.password, password);
    user.isAdmin = 0;
    
    int sig = SIGNUP;
    write(sd, &sig, sizeof(int));
    write(sd, &user, sizeof(user));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == LOGIN)
        printf("Please login\n");
    else if (retVal == SERVER_FAILURE)
        printf("Error occurred\n");
    else {
        isAuth = 1;
        globalUser = user;
        printf("Signed up successfully\n");
    }
}

void login(int sd) {
    User user;
    printf("Enter Username: ");
    scanf("%s",user.name);
    char *password = getpass("Enter password: ");
    strcpy(user.password, password);
    int sig = LOGIN;
    write(sd, &sig, sizeof(int));
    write(sd, &user, sizeof(user));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == SIGNUP)
        printf("Please signup first\n");
    else if (retVal == SERVER_FAILURE)
        printf("Wrong Password!\n");
    else {
        isAuth = 1;
        globalUser = user;
        printf("Logged in successfully\n");
    }
}

void loginForAdmin(int sd) {
    User user;
    printf("Enter Username: ");
    scanf("%s",user.name);
    char *password = getpass("Enter password: ");
    strcpy(user.password, password);
    int sig = LOGIN;
    write(sd, &sig, sizeof(int));
    write(sd, &user, sizeof(user));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == ADMIN_LOGIN) {
        isAuth = 1;
        globalUser = user;
        printf("Logged in successfully\n");
    }
    else 
        printf("Access denied\n");
} 

void borrowBook(int sd) {
    int sig = BORROW, bookId;
    write(sd, &sig, sizeof(int));
    printf("Enter book id to borrow: ");
    scanf("%d", &bookId);
    write(sd, &bookId, sizeof(int));
    write(sd, globalUser.name, sizeof(globalUser.name));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal)
        printf("An error occurred\n");
    else
        printf("Borrowed book successfully\n");
}

void returnBook(int sd) {
    int sig = RETURN, bookId;
    write(sd, &sig, sizeof(int));
    printf("Enter book id to return: ");
    scanf("%d", &bookId);
    write(sd, &bookId, sizeof(int));
    write(sd, globalUser.name, sizeof(globalUser.name));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal)
        printf("An error occurred\n");
    else
        printf("Returned book successfully\n");
}

void getBookById(int sd) {
    int sig = GET_BOOK, bookId;
    write(sd, &sig, sizeof(int));
    printf("Enter book id: ");
    scanf("%d", &bookId);
    write(sd, &bookId, sizeof(int));
    int valid = 0;
    Book book;
    read(sd, &valid, sizeof(int));
    if (valid) read(sd, &book, sizeof(book));
    if (!valid  || book.valid == 0)
        printf("Enter valid id\n");
    else 
        printf("Name: %s, Id: %d, Quantity: %d, MaxQuantity: %d, Author: %s\n", book.name, book.id, book.quantity, book.maxQuantity, book.author);
}

void addBook(int sd) {
    int sig = ADD_BOOK;
    write(sd, &sig, sizeof(int));
    Book book;
    printf("Name: ");
    scanf("%s", book.name); 
    printf("Quantity: ");
    scanf("%d", &book.maxQuantity);
    book.quantity = book.maxQuantity;
    printf("Author: ");
    scanf("%s", book.author);
    write(sd, &book, sizeof(book));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == 0)
        printf("Added book successfully\n");
    else
        printf("Error in adding book\n");
}

void deleteBook(int sd) {
    int sig = DELETE_BOOK;
    write(sd, &sig, sizeof(int));
    int bookId;
    printf("Enter book id to delete: ");
    scanf("%d", &bookId);
    write(sd, &bookId, sizeof(int));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == 0)
        printf("Deleted book successfully\n");
    else
        printf("Error in deleting book\n");
}

void updateBook(int sd) {
    int sig = UPDATE_BOOK;
    write(sd, &sig, sizeof(int));
    Book book;
    printf("Enter book id to update: ");
    scanf("%d", &book.id);
    printf("Enter name: ");
    scanf("%s", book.name);
    printf("Enter quantity: ");
    scanf("%d", &book.quantity);
    printf("Enter author: ");
    scanf("%s", book.author);

    write(sd, &book, sizeof(book));
    int retVal;
    read(sd, &retVal, sizeof(int));
    if (retVal == 0)
        printf("Updated book successfully\n");
    else
        printf("Error in updating book\n");
}

void logout(int sd) {
    int sig = EXIT;
    write(sd, &sig, sizeof(int));

}

int main() {
    struct sockaddr_in serv;
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv.sin_port = htons(PORT_NUMBER);
    connect(sd, (struct sockaddr *)&serv, sizeof(serv));
    // sleep(10);
    int choice;
    int isAdmin;
    printf("Login as Admin (No: 0 or Yes: 1) ");
    scanf("%d", &isAdmin);
    while (1) {
        if (!isAdmin) {
            printf(
                "\n1. Signup\n"
                "2. Login\n"
                "3. List Available Books\n"
                "4. Borrow Book\n"
                "5. Return Book\n"
                "6. Exit\n"
                "Enter your choice: "
            );
            scanf("%d", &choice);
            switch(choice) {
                case 1: isAuth ? printf("Already logged in") :signup(sd); break;
                case 2: isAuth ? printf("Already logged in") :login(sd); break;
                case 3: isAuth ? getBooks(sd) : printf("Please login / signup to continue\n"); break;
                case 4: isAuth ? borrowBook(sd) : printf("Please login / signup to continue\n"); break;
                case 5: isAuth ? returnBook(sd) : printf("Please login / signup to continue\n"); break;
                default: logout(sd); close(sd); return 0;
            }
        } else {
            printf(
                "\n1. Login\n"
                "2. Get Books by ID\n"
                "3. Add Books\n"
                "4. Delete Book\n"
                "5. Update Book\n"
                "6. Get all users\n"
                "7. Exit\n"
                "Enter your choice: "
            );
            scanf("%d", &choice);
            switch(choice) {
                case 1: isAuth ? printf("Already logged in"): loginForAdmin(sd); break;
                case 2: isAuth ? getBookById(sd) : printf("Please login to continue\n"); break;
                case 3: isAuth ? addBook(sd) : printf("Please login to continue\n"); break;
                case 4: isAuth ? deleteBook(sd) : printf("Please login to continue\n"); break;
                case 5: isAuth ? updateBook(sd) : printf("Please login to continue\n"); break;
                case 6: isAuth ? getUsers(sd) : printf("Please login to continue\n"); break;
                default: logout(sd); close(sd); return 0;
            }
        }
            
    }
    close(sd);
}
