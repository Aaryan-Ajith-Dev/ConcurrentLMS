#ifndef DATA_H
#define DATA_H

#define SERVER_SUCCESS 0
#define SERVER_FAILURE 1
#define ADMIN_LOGIN 2
#define EXIT -1
#define LOGIN -2
#define SIGNUP -3
#define GET_BOOKS -4
#define ADD_BOOK -5
#define BORROW -6
#define RETURN -7
#define GET_BOOK -8
#define UPDATE_BOOK -9
#define DELETE_BOOK -10
#define GET_USERS -11

#define SIZE 20
#define MAX_BOOKS 5
#define PORT_NUMBER 5000

 
typedef struct {
    char name[SIZE];
    int id;
    int quantity;
    int maxQuantity;
    char author[SIZE];
    int valid;
} Book;

typedef struct {
    char name[SIZE];
    char password[SIZE];
    int isAdmin;
} User;

typedef struct {
    char name[SIZE];
    int bookId;
    int valid;
} UserBook;
extern int bookNumber;

#endif

// invalid book id
// delete and update