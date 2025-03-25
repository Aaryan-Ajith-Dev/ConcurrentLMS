# OS Mini Project

This project is designed to demonstrate file locking and concurrent access control in a library management system.
The project consists of a client side and a server side interacting via web sockets. In the client side, the code displays a terminal
interface that the user can use to perform operations. Operations are made persistant via the usage of data files for storage. In this project, an M:N relation between users and books is implemented.

## Features

- User roles (Admin / User) along with user authentication
- Concurrency control
- Network Communication via Web Sockets
- CRUD operations for users to access books and admin to see users.
- Readable code through the use of macros, functions, and a well-structured file organization.

##, Requirements

- Linux Operating System
- GCC
- pthread

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/Aaryan-Ajith-Dev/OS_MiniProject.git
   ```

2. Navigate to the project directory:

   ```bash
   cd OS_MiniProject
   ```

3. Compile the source code:

   ```bash
   gcc -o server server.c -lpthread
   gcc -o client client.c -lpthread
   ```

## Usage

1. Start the server:

   ```bash
   ./server
   ```

2. In a new terminal, start the client:

   ```bash
   ./client
   ```

3. A terminal interface will show up on the client side, which can be used to perform the required operations.
