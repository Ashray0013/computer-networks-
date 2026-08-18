#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <time.h>

#define PORT "65432"
#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    int sock_fd;
    char buffer[BUFFER_SIZE];
    ssize_t bytes_sent, bytes_received, int_send, int_received;

    // Check command line arguments
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <server-hostname-or-ip> [port]\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_host = argv[1];
    const char *server_port = (argc == 3) ? argv[2] : PORT;

    printf("Attempting to connect to server at %s:%s\n", server_host, server_port);

    // 1. Create socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    printf("Socket created successfully (fd: %d)\n", sock_fd);

    // 2. Resolve hostname
    struct addrinfo hints = {0};
    struct addrinfo *results;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    printf("Resolving hostname %s...\n", server_host);
    int addr_status = getaddrinfo(server_host, server_port, &hints, &results);
    if (addr_status != 0)
    {
        fprintf(stderr, "Address lookup failed: %s\n", gai_strerror(addr_status));
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    // 3. Try ALL addresses (with timeout)
    int connected = 0;
    int address_num = 0;
    struct timeval timeout;
    timeout.tv_sec = 3; // 3 second timeout
    timeout.tv_usec = 0;

    // Set socket to non-blocking for timeout
    // (Simpler: we'll just try each address quickly)

    for (struct addrinfo *current = results; current != NULL; current = current->ai_next)
    {
        address_num++;

        // Convert address to string for display
        char ip_str[INET_ADDRSTRLEN];
        struct sockaddr_in *addr_in = (struct sockaddr_in *)current->ai_addr;
        inet_ntop(AF_INET, &addr_in->sin_addr, ip_str, sizeof(ip_str));

        printf("Trying address %d: %s:%s (IP: %s)...\n",
               address_num, server_host, server_port, ip_str);
        fflush(stdout); // Force print immediately

        // Try to connect
        if (connect(sock_fd, current->ai_addr, current->ai_addrlen) == 0)
        {
            connected = 1;
            printf("✅ SUCCESS! Connected to %s:%s (IP: %s)\n",
                   server_host, server_port, ip_str);
            break;
        }
        else
        {
            // Print why it failed
            printf("❌ Failed: %s\n", strerror(errno));
            fflush(stdout);

            // If we failed, we need to create a NEW socket for the next attempt
            // because the old one is in an unusable state
            close(sock_fd);
            sock_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (sock_fd < 0)
            {
                perror("socket creation failed");
                freeaddrinfo(results);
                exit(EXIT_FAILURE);
            }
        }
    }
    freeaddrinfo(results);

    if (!connected)
    {
        fprintf(stderr, "\n❌ Could not connect to any address for %s:%s\n",
                server_host, server_port);
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    printf("\n✅ Connected to server! Start chatting (type 'exit' to quit)\n");
    printf("------------------------------------------------------------\n");

    printf("You: ");
    fflush(stdout);

    // if ( == NULL)
    // {

    // }
    fgets(buffer, BUFFER_SIZE, stdin);

    // Remove newline for exit check
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n')
    {
        buffer[len - 1] = '\0';
    }

    // Add newline back for sending
    buffer[len - 1] = '\n';

    // Send message
    bytes_sent = send(sock_fd, buffer, strlen(buffer), 0);
    if (bytes_sent < 0)
    {
        perror("send failed");
    }
    int num;
    printf("Enter an integer: ");
    if (scanf("%d", &num) != 1)
    {
        fprintf(stderr, "Invalid input!\n");
        close(sock_fd);
        return EXIT_FAILURE;
    }
    int net_num = htonl(num); // host → network order
    // Send the integer
    int_send = send(sock_fd, &net_num, sizeof(net_num), 0);
    if (int_send < 0)
    {
        perror("send failed");
    }

    // Receive response
    bytes_received = recv(sock_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received < 0)
    {
        perror("recv failed");
    }
    else if (bytes_received == 0)
    {
        printf("Server closed the connection\n");
    }

    buffer[bytes_received] = '\0';
    printf("Server: %s", buffer);
    int code;
    int_received = recv(sock_fd, &code, sizeof(code), 0);
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
        code = ntohl(code);
        printf("Received code: %d\n", code);
        printf("sum=%d\n\n", code + num);
    }

    // 5. Clean up
    close(sock_fd);
    printf("Connection closed\n");
    return 0;
}