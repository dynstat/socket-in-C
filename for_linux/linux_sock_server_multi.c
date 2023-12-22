// Required Libraries
#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>    //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <pthread.h>  // for threading, link with lpthread

#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAX_CLIENTS 100

// Function to handle client connection
void *handle_client(void *arg)
{
    int new_socket = *(int *)arg;
    char buffer[1024];
    int len;

    while ((len = recv(new_socket, buffer, sizeof(buffer), 0)) > 0)
    {
        buffer[len] = '\0';
        send(new_socket, buffer, strlen(buffer), 0);
    }

    close(new_socket);

    return NULL;
}

int main(int argc, char *argv[])
{
    int opt = TRUE;
    int master_socket, addrlen, new_socket, client_socket[MAX_CLIENTS], activity, i, valread, sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[1025]; // data buffer of 1K

    // set of socket descriptors
    fd_set readfds;

    // initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_CLIENTS; i++)
    {
        client_socket[i] = 0;
    }

    // create a master socket
    if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // set master socket to allow multiple connections , this is just a good habit, it will work without this
    if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);

    // try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while (TRUE)
    {
        // Clear the socket set using FD_ZERO macro. It initializes the file descriptor set to have zero bits for all file descriptors.
        FD_ZERO(&readfds);

        // Add master socket to set using FD_SET macro. It includes the file descriptor in the set.
        FD_SET(master_socket, &readfds);
        max_sd = master_socket; // Initialize max_sd with master_socket

        // Add child sockets to set
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            // Socket descriptor
            sd = client_socket[i]; // Assigning client_socket[i] to sd

            // If valid socket descriptor then add to read list
            if (sd > 0)               // Checking if sd is greater than 0
                FD_SET(sd, &readfds); // If sd is valid, include it in the file descriptor set

            // Highest file descriptor number, need it for the select function
            if (sd > max_sd) // Checking if sd is greater than max_sd
                max_sd = sd; // If sd is greater, assign it to max_sd
        }

        // Wait for an activity on one of the sockets, timeout is 0.1 sec
        struct timeval timeout;   // Declare struct timeval variable named timeout
        timeout.tv_sec = 0;       // Set the number of whole seconds
        timeout.tv_usec = 100000; // Set the number of microseconds. 100000 microseconds is equal to 0.1 seconds
        // Call select function. It monitors file descriptors to see if they are ready for I/O operations. It returns the total number of ready descriptors in the set.
        activity = select(max_sd + 1, &readfds, NULL, NULL, &timeout);

        if ((activity < 0) && (errno != EINTR))
        {
            printf("select error");
        }

        // If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            // inform user of socket number - used in send and receive commands
            printf("New connection , socket fd is %d , ip is : %s , port : %d\n", new_socket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));

            // add new socket to array of sockets
            for (i = 0; i < MAX_CLIENTS; i++)
            {
                // if position is empty
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Adding to list of sockets as %d\n", i);
                    break;
                }
            }
        }

        // else its some IO operation on some other socket
        for (i = 0; i < MAX_CLIENTS; i++)
        {
            sd = client_socket[i];

            if (FD_ISSET(sd, &readfds))
            {
                // Create a new thread for each client
                pthread_t client_thread;
                if (pthread_create(&client_thread, NULL, handle_client, (void *)&sd) < 0)
                {
                    perror("Could not create thread");
                    return 1;
                }
                printf("Handler assigned\n");
            }
        }
    }

    return 0;
}