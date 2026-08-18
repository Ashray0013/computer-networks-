#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <math.h>

#define PORT 65432
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;
    ssize_t int_received;
    ssize_t int_send;

    // 1. Create a TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("Socket created successfully\n\n whith file descriptor: %d\n", server_fd);
    }

    // 2. Configure server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections from any interface
    server_addr.sin_port = htons(PORT);       // Convert port to network byte order
    printf("port in host byte order: %d\n", PORT);
    printf("port in network byte order: %d\n", ntohs(server_addr.sin_port));
    printf("Server address configured: %s:%d\n", inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    // 3. Bind the socket to the address
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Listen for incoming connections (backlog = 3)
    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    // 5. Accept a client connection (blocks until a client connects)
    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (client_fd < 0)
    {
        perror("accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Client connected from %s:%d\n",
           inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    bytes_received = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0)
    {
        perror("recv failed");
    }
    else if (bytes_received == 0)
    {
        printf("Client disconnected\n");
    }

    buffer[bytes_received] = '\0'; // Null-terminate for safe printing
    printf("Received: %s", buffer);
    int num;
    int_received = recv(client_fd, &num, sizeof(num), 0);
    if (int_received < 0)
    {
        perror("recv failed");
    }
    else if (int_received == 0)
    {
        printf("Client disconnected\n");
    }
    else
    {
        // Convert from network byte order to host byte order
        num = ntohl(num);
        printf("Received integer: %d\n", num);
    }

    // Echo back the same data
    const char *reply = "this is server your message is received\n\n";
    send(client_fd, reply, strlen(reply), 0);
    int code = 12;
    int net_num = htonl(code); // host → network order
    // Send the integer
    int_send = send(client_fd, &net_num, sizeof(net_num), 0);
    if (int_send < 0)
    {
        perror("send failed");
    }
    else
    {
        printf("code sent ");
    }

    // 7. Clean up
    close(client_fd);
    close(server_fd);
    printf("Connection closed\n");
    return 0;
}