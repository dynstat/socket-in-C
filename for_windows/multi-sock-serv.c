#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Winsock Library

/**
 * This is a server program that listens for incoming connections and accepts them, then adds the new
 * socket to an array of sockets.
 *
 * @return The main function is returning an integer value of 0.
 */
int main()
{
    /* `WSADATA wsa;` is declaring a variable `wsa` of type `WSADATA`. `WSADATA` is a structure that is
    used to store information about the Windows Sockets implementation. This structure is required
    to be passed as a parameter to the `WSAStartup` function, which initializes the Winsock library. */
    WSADATA wsa;
    /* The line `SOCKET master, new_socket, client_socket[30], s;` is declaring four variables of type
    `SOCKET`. */
    SOCKET master, new_socket, client_socket[30], s;
    /* `struct sockaddr_in` is a structure that is used to store information about an internet address.
    It is specifically designed for IPv4 addresses. In the given code, it is used to define the
    server address and client address for establishing a TCP/IP connection. */
    struct sockaddr_in server, address;
    /* The line `int max_clients = 30, activity, addrlen, i, valread;` is declaring multiple variables
    of type `int`. */
    int max_clients = 30, activity, addrlen, i, valread;
    /* The line `char *message = "Welcome to the server\r\n";` is declaring a pointer variable
    `message` of type `char*` and initializing it with the string "Welcome to the server\r\n". */
    char *message = "Welcome to the server\r\n";
    /* The line `fd_set readfds; // define readfds here` is declaring a variable `readfds` of type
    `fd_set`. `fd_set` is a data structure used by the `select` function to hold the file
    descriptors (sockets) that the program is interested in monitoring for activity. In this case,
    `readfds` is used to hold the set of sockets that the program wants to check for incoming data. */
    fd_set readfds; // define readfds here

    // Initialize Winsock
    printf("\nInitializing Winsock...");
    /* The line `if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)` is initializing the Winsock library by
    calling the `WSAStartup` function. */
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create a socket
    /* The line `if ((master = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)` is creating a
    socket using the `socket` function. */
    if ((master = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    // Set up the server structure
    server.sin_family = AF_INET;
    /* The line `server.sin_family = AF_INET;` is setting the address family of the server structure to
    `AF_INET`. */
    server.sin_addr.s_addr = INADDR_ANY;
    /* The line `server.sin_addr.s_addr = INADDR_ANY;` is setting the IP address of the server to
    `INADDR_ANY`. This means that the server will listen for incoming connections on all available
    network interfaces on the machine. By setting it to `INADDR_ANY`, the server can accept
    connections from any IP address. */
    /* The line `server.sin_port = htons(8888);` is setting the port number of the server to 8888. The
    `htons` function is used to convert the port number from host byte order to network byte order.
    This is necessary because network protocols use network byte order to represent port numbers. */
    server.sin_port = htons(8888);

    // Bind the socket
    /* `struct sockaddr` is a generic structure used to represent socket addresses in the networking
    programming. It is a base structure that is used by more specific address structures, such as
    `struct sockaddr_in` for IPv4 addresses. */
    if (bind(master, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    puts("Bind done");

    // Listen for incoming connections
    /* The `listen(master, 3);` line is used to make the server socket `master` start listening for
    incoming connections. The second parameter `3` specifies the maximum number of pending
    connections that can be queued up before the server starts rejecting new connections. In this
    case, it means that the server can handle up to 3 pending connections before it starts rejecting
    new connections. */
    listen(master, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");

    addrlen = sizeof(struct sockaddr_in);

    while (1)
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add master socket to set
        /* The line `FD_SET(master, &readfds);` is adding the master socket `master` to the set of
        sockets `readfds`. This is done in order to include the master socket in the set of sockets
        that the program wants to check for incoming data. */
        FD_SET(master, &readfds);

        // Add child sockets to set
        for (i = 0; i < max_clients; i++)
        {
            s = client_socket[i];
            if (s != 0)
            {
                FD_SET(s, &readfds);
            }
        }

        // Wait for an activity on one of the sockets
        /* The line `activity = select(0, &readfds, NULL, NULL, NULL);` is using the `select` function
        to monitor the set of sockets in `readfds` for any activity. */
        activity = select(0, &readfds, NULL, NULL, NULL);

        /* The line `if ((activity < 0) && (WSAGetLastError() != WSAEINTR))` is checking if the
        `select` function returned a negative value for `activity` and if the error code returned by
        `WSAGetLastError()` is not `WSAEINTR`. */
        if ((activity < 0) && (WSAGetLastError() != WSAEINTR))
        {
            printf("select error: %d\n", WSAGetLastError()); // Added error code to the error message
        }

        // If something happened on the master socket, then it's an incoming connection
        /* The line `if (FD_ISSET(master, &readfds))` is checking if the master socket `master` is set
        in the set of sockets `readfds`. This is done to determine if there is any activity on the
        master socket, indicating an incoming connection. If the master socket is set in `readfds`,
        it means that there is an incoming connection and the program should proceed to accept the
        connection. */
        if (FD_ISSET(master, &readfds))
        {
            /* The line `if ((new_socket = accept(master, (struct sockaddr *)&address, &addrlen)) ==
            INVALID_SOCKET)` is accepting an incoming connection on the master socket `master`. */
            if ((new_socket = accept(master, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Inform user of socket number - used in send and receive commands
            /* The line `printf("New connection , socket fd is %d , ip is : %s , port : %d \n",
            new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));` is printing
            information about the newly accepted connection. */
            /* `ntohs` stands for "network to host short" and it is a function that converts a 16-bit number from network byte order to host byte order. */
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Send new connection greeting message
            if (/* The `send` function is used to send data over a connected socket. In the given code,
            it is used to send the "Welcome to the server\r\n" message to the newly accepted
            connection. The function takes the following parameters: */
                /* The line `send(new_socket, message, strlen(message), 0) != strlen(message))` is using the
                `send` function to send the "Welcome to the server\r\n" message to the newly accepted
                connection. */
                send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            // Add new socket to array of sockets
            /* The code block is iterating through the array `client_socket` to find an empty position
            (where the value is 0) and then adding the newly accepted socket `new_socket` to that
            position in the array. It also prints a message indicating the position where the socket
            is added. This is done to keep track of the connected sockets and their positions in the
            array. */
            for (i = 0; i < max_clients; i++)
            {
                // If position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }
    }

    return 0;
}