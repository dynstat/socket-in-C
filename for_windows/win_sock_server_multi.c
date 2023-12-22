#include <stdio.h>    // Include standard input/output header file for printf function
#include <winsock2.h> // Include winsock2 for network communication functions
#include <windows.h>  // Include windows.h for Windows-specific functions like Sleep and CreateThread

#define MAX_CLIENTS 100 // Define maximum number of clients that can be handled simultaneously
#define TIMEOUT 100     // Define timeout for select function in milliseconds

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib, the library for winsock2

// Function to handle communication with each client
// This function is intended to be run in a separate thread for each client
// The function takes a void pointer as an argument, which is the client socket in this case
// The function returns a DWORD (32-bit unsigned integer) which is the exit code of the thread
DWORD WINAPI ClientHandler(void *client_sock)
{
    // Cast the data to SOCKET*
    // This is the socket connected to the client
    // The client_sock argument is a void pointer, so it needs to be cast to SOCKET* before it can be used
    SOCKET clientSocket = *(SOCKET *)client_sock;
    // Buffer to store received data
    // This is a char array of size 1024, which means it can store up to 1023 characters and a null terminator
    char buffer[1024];
    // Number of bytes received
    // This integer will store the number of bytes received from the client
    int bytesReceived;

    // Infinite loop to handle communication with the client
    // This loop will continue until the client disconnects or an error occurs
    while (1)
    {
        // Receive data from the client
        // The recv function receives data from the client and stores it in the buffer
        // It returns the number of bytes received
        // If the return value is 0 or negative, there was an error or the client disconnected
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
        {
            // If the client disconnected or an error occurred, print a message and close the socket
            printf("Client disconnected\n");
            // The closesocket function closes the socket
            // After this line, the clientSocket can no longer be used
            closesocket(clientSocket);
            // End the thread
            // The return value is 0, which indicates that the thread ended successfully
            return 0;
        }

        // Null-terminate the received data
        // This is necessary because the data received from the client might not be null-terminated
        // Without this line, the printf function might print out extra characters
        buffer[bytesReceived] = '\0';
        // Print the received data
        // This line prints the data received from the client to the console
        printf("Received: %s\n", buffer);

        // Wait for 1 second
        // The Sleep function pauses the thread for a specified amount of time
        // In this case, it pauses the thread for 1000 milliseconds, or 1 second
        Sleep(1000);

        // Send the received data back to the client
        // The send function sends data to the client
        // In this case, it sends back the data that was received from the client
        // This is why this server is called an "echo server"
        send(clientSocket, buffer, bytesReceived, 0);
    }

    // This line will never be reached, but is here to satisfy the function's return type
    // The function needs to return a DWORD, so this line returns 0
    // However, because of the infinite loop, this line will never actually be executed
    return 0;
}

// Main function
int main()
{
    // Data structure to hold details about the Windows Sockets implementation
    WSADATA wsaData;
    // Socket for the server and for the client
    SOCKET serverSocket, clientSocket;
    // Structures to hold server and client details
    struct sockaddr_in server, client;
    // Size of the client structure
    int clientLen = sizeof(client);
    // Array to hold thread handles
    HANDLE threads[MAX_CLIENTS];
    // Variable to hold thread ID
    DWORD threadID;
    // Counter for the number of clients
    int i = 0;

    // Initialize Windows Sockets
    // WSAStartup function is used to initiate use of WS2_32.dll
    // MAKEWORD(2,2) makes a request for version 2.2 of Winsock on the system
    // &wsaData is a pointer to the WSADATA data structure that is to receive details of the Windows Sockets implementation
    // If the return value is not 0, there was an error
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Failed to initialize winsock\n");
        return 1;
    }

    // Create a socket for the client
    // socket function creates a socket that is bound to a specific transport service provider
    // AF_INET is the address family specification for IPv4
    // SOCK_STREAM Provides sequenced, reliable, two-way, connection-based byte streams
    // 0 indicates that the default protocol for the address family and type selected is to be used
    // If the return value is INVALID_SOCKET, there was an error
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        printf("Failed to create socket\n");
        return 1;
    }

    // Set up the server structure
    server.sin_family = AF_INET;         // Internet protocol
    server.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    server.sin_port = htons(8888);       // Server port

    // The bind function associates a local address with a socket.
    // In this case, it's binding the server socket to the server address we've specified (IP and port).
    // The parameters are:
    // - serverSocket: the socket that was created with the socket function
    // - (struct sockaddr *)&server: a pointer to the sockaddr structure representing the local address to assign to the socket
    // - sizeof(server): the length of the sockaddr structure
    // The return value is 0 if the function succeeds. If the function fails, it returns SOCKET_ERROR.
    if (bind(serverSocket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed\n");
        return 1;
    }

    // The listen function is used to make the server socket ready to accept incoming connection requests.
    // It has two parameters:
    // - serverSocket: the socket that was created with the socket function and bound to a local address with bind.
    // - 3: the maximum length of the queue of pending connections. If a connection request arrives when the queue is full, the client may receive an error with an indication of ECONNREFUSED.
    // This function does not return a value.
    listen(serverSocket, 3);

    // This is an infinite loop that will keep the server running and ready to accept incoming connections
    while (1)
    {
        // The FD_SET data structure represents a set of sockets that we want to monitor for some activity
        FD_SET readSet;
        // FD_ZERO is a macro that clears (initializes) a set. Here, it's used to clear our set of sockets
        FD_ZERO(&readSet);
        // FD_SET is a macro that adds a given socket (in this case, the server socket) to a set.
        FD_SET(serverSocket, &readSet);

        // Timeout for the select function
        struct timeval timeout;
        timeout.tv_sec = 0;               // Seconds
        timeout.tv_usec = TIMEOUT * 1000; // Microseconds

        // The select function is used to monitor multiple file descriptors, waiting until one or more of the file descriptors become "ready" for some class of I/O operation.
        // In this case, we are monitoring the server socket for incoming connection requests.
        // The parameters are:
        // - 0: the highest-numbered file descriptor in any of the three sets, plus 1. In this case, we only monitor the server socket, so it's 0.
        // - &readSet: pointer to the set of sockets to be checked for readability (incoming connection requests in this case).
        // - NULL: pointer to the set of sockets to be checked for writability. We don't check this, so it's NULL.
        // - NULL: pointer to the set of sockets to be checked for errors. We don't check this, so it's NULL.
        // - &timeout: pointer to a timeval structure that specifies the maximum interval to wait for the selection condition to become true. If this parameter is NULL, the select blocks indefinitely.
        // The return value is the total number of bits set in the bit masks. If the time limit expires, the return value is 0. If an error occurs, it returns SOCKET_ERROR.
        if (select(0, &readSet, NULL, NULL, &timeout) > 0)
        {
            // The accept function is used to extract the first connection request on the queue of pending connections for the listening socket (serverSocket), and creates a new connected socket (clientSocket).
            // The parameters are:
            // - serverSocket: a socket that has been created with socket(), bound to a local address with bind(), and is listening for connections after a listen().
            // - (struct sockaddr *)&client: an optional pointer to a buffer that receives the address of the connecting entity, as known to the communications layer.
            // - &clientLen: an optional pointer to an integer that contains the length of the structure pointed to by the previous parameter.
            // The return value is a handle to the new socket if the function succeeds. If the function fails, it returns INVALID_SOCKET.
            clientSocket = accept(serverSocket, (struct sockaddr *)&client, &clientLen);
            if (clientSocket == INVALID_SOCKET)
            {
                // If the accept function fails, it prints an error message and returns 1 to indicate an error.
                printf("Accept failed\n");
                return 1;
            }

            printf("Client connected\n");

            // If the number of connected clients is less than the maximum allowed (MAX_CLIENTS), a new thread is created to handle the client's requests.
            // The 'if' condition checks this by comparing the current number of clients (i) with MAX_CLIENTS.
            if (i < MAX_CLIENTS)
            {
                // CreateThread is a Windows API function that creates a new thread for a client.
                // Parameters:
                // - NULL: Security attributes. If this parameter is NULL, the handle cannot be inherited by child processes.
                // - 0: The initial size of the stack, in bytes. If this parameter is 0, the new thread uses the default size for the executable.
                // - ClientHandler: A pointer to the application-defined function to be executed by the thread (the thread function).
                // - &clientSocket: A pointer to a variable to be passed to the thread function. Here, it's the client's socket.
                // - 0: The flags that control the creation of the thread. 0 means the thread runs immediately after creation.
                // - &threadID: A pointer to a variable that receives the thread identifier. If this parameter is NULL, the thread identifier is not returned.
                // The return value is a handle to the new thread. If the function fails, the return value is NULL.
                threads[i] = CreateThread(NULL, 0, ClientHandler, &clientSocket, 0, &threadID);
                // Increment the client count (i) as a new client has been accepted and a thread has been created for it.
                i++;
            }
            // If the number of connected clients exceeds the maximum limit (MAX_CLIENTS), the client socket is closed.
            // This is to prevent overloading the server with too many concurrent connections.
            else
            {
                // Print a message to the console indicating that the maximum client limit has been reached.
                printf("Too many clients\n");
                // The closesocket function is used to close an existing socket.
                // The parameter is the descriptor of the socket to be closed.
                // The return value is 0 if the operation was successful. Otherwise, a value of SOCKET_ERROR is returned, and a specific error code can be retrieved by calling WSAGetLastError.
                closesocket(clientSocket);
            }
        }
    }

    // Close the server socket
    closesocket(serverSocket);
    // Clean up Windows Sockets
    WSACleanup();

    // End the program
    return 0;
}