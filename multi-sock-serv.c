#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Winsock Library

int main()
{
    WSADATA wsa;
    SOCKET master, new_socket, client_socket[30], s;
    struct sockaddr_in server, address;
    int max_clients = 30, activity, addrlen, i, valread;
    char *message = "Welcome to the server\r\n";
    fd_set readfds; // define readfds here

    // Initialize Winsock
    printf("\nInitializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create a socket
    if ((master = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create socket : %d", WSAGetLastError());
        return 1;
    }
    printf("Socket created.\n");

    // Set up the server structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(8888);

    // Bind the socket
    if (bind(master, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        return 1;
    }
    puts("Bind done");

    // Listen for incoming connections
    listen(master, 3);

    // Accept and incoming connection
    puts("Waiting for incoming connections...");

    addrlen = sizeof(struct sockaddr_in);

    while (1)
    {
        // Clear the socket set
        FD_ZERO(&readfds);

        // Add master socket to set
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
        activity = select(0, &readfds, NULL, NULL, NULL);

        if ((activity < 0) && (WSAGetLastError() != WSAEINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket, then it's an incoming connection
        if (FD_ISSET(master, &readfds))
        {
            if ((new_socket = accept(master, (struct sockaddr *)&address, &addrlen)) == INVALID_SOCKET)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // Inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d \n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // Send new connection greeting message
            if (send(new_socket, message, strlen(message), 0) != strlen(message))
            {
                perror("send");
            }

            puts("Welcome message sent successfully");

            // Add new socket to array of sockets
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