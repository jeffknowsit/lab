/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

int main(void) {
    int sock;
    struct sockaddr_in server;
    char name[64], input[1024], received[2048];
    char recv_buffer[4096];
    size_t used = 0;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }

    printf("Connected to chat server.\n");
    printf("Enter your name: ");
    fflush(stdout);
    if (!fgets(name, sizeof(name), stdin)) {
        close(sock);
        return 1;
    }
    name[strcspn(name, "\r\n")] = '\0';
    if (send(sock, name, strlen(name) + 1, 0) <= 0) {
        perror("send");
        close(sock);
        return 1;
    }

    printf("You are now connected as %s.\n", name);
    printf("Type your message and press Enter.\n");
    printf("Type 'exit' to quit.\n");

    while (1) {
        fd_set readfds;
        int maxfd = sock;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(input, sizeof(input), stdin)) break;
            input[strcspn(input, "\r\n")] = '\0';
            if (send(sock, input, strlen(input) + 1, 0) <= 0) break;
            if (strcmp(input, "exit") == 0) {
                printf("Disconnected from server.\n");
                break;
            }
        }

        if (FD_ISSET(sock, &readfds)) {
            ssize_t n = recv(sock, recv_buffer + used, sizeof(recv_buffer) - used - 1, 0);
            if (n <= 0) {
                printf("Disconnected from server.\n");
                break;
            }
            used += (size_t)n;
            recv_buffer[used] = '\0';

            char *start = recv_buffer;
            char *end;
            while ((end = memchr(start, '\0', (size_t)(recv_buffer + used - start))) != NULL) {
                size_t len = (size_t)(end - start);
                if (len >= sizeof(received)) len = sizeof(received) - 1;
                memcpy(received, start, len);
                received[len] = '\0';
                printf("%s\n", received);
                start = end + 1;
            }
            used = (size_t)(recv_buffer + used - start);
            memmove(recv_buffer, start, used);
            if (used >= sizeof(recv_buffer) - 1) used = 0;
        }
    }

    close(sock);
    return 0;
}
