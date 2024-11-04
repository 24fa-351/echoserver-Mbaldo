#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Function to handle each client connection
void *handle_client(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    ssize_t read_size;

    while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[read_size] = '\0'; // Null-terminate the received data
        send(client_socket, buffer, read_size, 0); // Echo back the received message
    }

    close(client_socket); // Close connection with this client
    free(arg); // Free the memory allocated for client socket
    pthread_exit(NULL); // End thread
}

int main(int argc, char *argv[]) {
    int server_socket, *client_socket, port = 0;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Parse command-line arguments for the port number
    if (argc != 3 || strcmp(argv[1], "-p") != 0) {
        fprintf(stderr, "Usage: %s -p <port>\n", argv[0]);
        return 1;
    }
    port = atoi(argv[2]);

    // Create a TCP socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Could not create socket");
        return 1;
    }

    // Set up the server address structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Bind the socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Bind failed");
        close(server_socket);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_socket, 1) == -1) {
        perror("Listen failed");
        close(server_socket);
        return 1;
    }

    printf("Server is listening on port %d...\n", port);

    while (1) {
        // Accept a new connection
        client_socket = malloc(sizeof(int));
        if ((*client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            free(client_socket);
            continue;
        }
        printf("Client connected.\n");

        // Create a new thread to handle the client
        pthread_t client_thread;
        if (pthread_create(&client_thread, NULL, handle_client, client_socket) != 0) {
            perror("Could not create thread");
            close(*client_socket);
            free(client_socket);
        } else {
            pthread_detach(client_thread); // Automatically release resources when thread finishes
        }
    }

    close(server_socket);
    return 0;
}
