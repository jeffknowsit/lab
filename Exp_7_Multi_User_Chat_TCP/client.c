/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define BUFFER_SIZE 4096

static int send_all(int fd, const char *data, size_t len) {
    size_t sent = 0;
    while (sent < len) {
        ssize_t n = send(fd, data + sent, len - sent, 0);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}

int main(void) {
    int sock;
    struct sockaddr_in server;
    char name[64], input[1024], recv_buffer[BUFFER_SIZE];
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

    printf("Connected to chat server.\nEnter your name: ");
    fflush(stdout);
    if (!fgets(name, sizeof(name), stdin)) { close(sock); return 1; }
    name[strcspn(name, "\r\n")] = '\0';
    if (send_all(sock, name, strlen(name) + 1) < 0) {
        perror("send");
        close(sock);
        return 1;
    }

    printf("You are now connected as %s.\n", name);
    printf("Type your message and press Enter.\nType 'exit' to quit.\n");

    while (1) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sock, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        if (select((sock > STDIN_FILENO ? sock : STDIN_FILENO) + 1,
                   &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(input, sizeof(input), stdin)) break;
            input[strcspn(input, "\r\n")] = '\0';
            if (send_all(sock, input, strlen(input) + 1) < 0) break;
            if (strcmp(input, "exit") == 0) {
                printf("Disconnected from server.\n");
                break;
            }
        }

        if (FD_ISSET(sock, &readfds)) {
            char chunk[1024];
            ssize_t n = recv(sock, chunk, sizeof(chunk), 0);
            if (n <= 0) {
                printf("Disconnected from server.\n");
                break;
            }
            for (ssize_t i = 0; i < n; i++) {
                if (chunk[i] == '\0') {
                    recv_buffer[used] = '\0';
                    printf("%s\n", recv_buffer);
                    used = 0;
                } else if (used < sizeof(recv_buffer) - 1) {
                    recv_buffer[used++] = chunk[i];
                } else {
                    used = 0;
                }
            }
        }
    }

    close(sock);
    return 0;
}
