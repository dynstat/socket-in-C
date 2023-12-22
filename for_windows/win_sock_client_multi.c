#include <stdio.h>
#include <winsock2.h> // Include winsock2 for network communication functions
#include <windows.h>  // Include windows.h for Windows-specific functions

#define SERVER_PORT 8888        // Define the port number for the server
#define SERVER_ADDR "127.0.0.1" // Define the IP address of the server

#pragma comment(lib, "ws2_32.lib") // Link with ws2_32.lib, the library for winsock2

int main() // Main function
{
    WSADATA wsaData;              // Structure to hold details about the Windows Sockets implementation
    SOCKET clientSocket;          // Variable to hold the socket descriptor
    struct sockaddr_in server;    // Structure to hold server details
    char buffer[1024];            // Buffer to hold the data to be sent/received
    int bytesSent, bytesReceived; // Variables to hold the number of bytes sent/received

    // Initialize Windows Sockets
    // WSAStartup function is used to initiate use of WS2_32.dll
    // MAKEWORD(2,2) makes a request for version 2.2 of Winsock on the system
    // &wsaData is a pointer to the WSADATA data structure that is to receive details of the Windows Sockets implementation
    // If the return value is not 0, there was an error
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("Failed to initialize winsock\n"); // Print error message
        return 1;                                 // Return 1 to indicate error
    }

    // Create a socket for the client
    // socket function creates a socket that is bound to a specific transport service provider
    // AF_INET is the address family specification for IPv4
    // SOCK_STREAM Provides sequenced, reliable, two-way, connection-based byte streams
    // 0 indicates that the default protocol for the address family and type selected is to be used
    // If the return value is INVALID_SOCKET, there was an error
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Failed to create socket\n"); // Print error message
        return 1;                            // Return 1 to indicate error
    }

    // Set up the server details
    server.sin_family = AF_INET;                     // Address family for IPv4
    server.sin_addr.s_addr = inet_addr(SERVER_ADDR); // IP address of the server
    server.sin_port = htons(SERVER_PORT);            // Port number of the server, converted to network byte order

    // Connect to the server
    // connect function establishes a connection to a specified socket
    // clientSocket is the socket that was created with the socket function
    // (struct sockaddr *)&server is a pointer to the sockaddr structure to which the connection should be established
    // sizeof(server) is the length of the sockaddr structure
    // If the return value is less than 0, there was an error
    if (connect(clientSocket, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        printf("Connect failed\n"); // Print error message
        return 1;                   // Return 1 to indicate error
    }

    printf("Connected to server\n"); // Print success message

    // Infinite loop to handle communication with the server
    while (1)
    {
        printf("Enter message: ");            // Prompt user to enter a message
        fgets(buffer, sizeof(buffer), stdin); // Read the message from standard input
        buffer[strcspn(buffer, "\n")] = 0;    // Remove newline character

        // Send data to the server
        // send function sends data on a connected socket
        // clientSocket is the socket that was created with the socket function
        // buffer is a pointer to the buffer containing the data to be transmitted
        // strlen(buffer) is the length of the data in buffer
        // 0 are the flags
        // If the return value is less than 0, there was an error
        bytesSent = send(clientSocket, buffer, strlen(buffer), 0);
        if (bytesSent < 0)
        {
            printf("Send failed\n"); // Print error message
            return 1;                // Return 1 to indicate error
        }

        // Receive data from the server
        // recv function receives data from a connected socket
        // clientSocket is the socket that was created with the socket function
        // buffer is a pointer to the buffer to receive the incoming data
        // sizeof(buffer) - 1 is the length of the buffer
        // 0 are the flags
        // If the return value is 0 or negative, there was an error or the server disconnected
        bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived <= 0)
        {
            printf("Recv failed\n"); // Print error message
            return 1;                // Return 1 to indicate error
        }

        // Null-terminate the received data
        buffer[bytesReceived] = '\0';
        // Print the received data
        printf("Received: %s\n", buffer);
    }

    // Close the socket
    // closesocket function closes an existing socket
    // clientSocket is the socket that was created with the socket function
    closesocket(clientSocket);

    // Terminate use of the Winsock 2 DLL (Ws2_32.dll)
    // WSACleanup function terminates use of the Winsock 2 DLL (Ws2_32.dll)
    WSACleanup();

    return 0; // Return 0 to indicate success
}