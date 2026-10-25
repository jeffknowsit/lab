/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 20

struct client_info {
    int fd;
    char name[64];
};

static struct client_info clients[MAX_CLIENTS];

int main(void) {
    int server_fd, opt = 1, i;
    struct sockaddr_in address;
    signal(SIGPIPE, SIG_IGN);
    for (i = 0; i < MAX_CLIENTS; i++) clients[i].fd = -1;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(server_fd, 10) < 0) {
        perror("bind/listen"); close(server_fd); return 1;
    }
    printf("TCP Chat Server started on port 5000...\n");

    while (1) {
        fd_set readfds;
        int maxfd = server_fd;
        FD_ZERO(&readfds);
        FD_SET(server_fd, &readfds);
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0) {
                FD_SET(clients[i].fd, &readfds);
                if (clients[i].fd > maxfd) maxfd = clients[i].fd;
            }
        }
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) continue;

        if (FD_ISSET(server_fd, &readfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd >= 0) {
                for (i = 0; i < MAX_CLIENTS && clients[i].fd >= 0; i++);
                if (i == MAX_CLIENTS) close(fd);
                else {
                    ssize_t n = recv(fd, clients[i].name, sizeof(clients[i].name) - 1, 0);
                    if (n <= 0) close(fd);
                    else {
                        clients[i].name[n] = '\0';
                        clients[i].name[strcspn(clients[i].name, "\r\n")] = '\0';
                        clients[i].fd = fd;
                        printf("New user connected: %s\n", clients[i].name);
                    }
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0 && FD_ISSET(clients[i].fd, &readfds)) {
                char message[1024], outgoing[1200];
                ssize_t n = recv(clients[i].fd, message, sizeof(message) - 1, 0);
                if (n <= 0) {
                    printf("User %s disconnected.\n", clients[i].name);
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    continue;
                }
                message[n] = '\0';
                message[strcspn(message, "\r\n")] = '\0';
                if (strcmp(message, "exit") == 0) {
                    printf("User %s disconnected.\n", clients[i].name);
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    continue;
                }
                snprintf(outgoing, sizeof(outgoing), "%s: %s", clients[i].name, message);
                printf("%s\n", outgoing);
                for (int j = 0; j < MAX_CLIENTS; j++) {
                    if (clients[j].fd >= 0 && clients[j].fd != clients[i].fd)
                        send(clients[j].fd, outgoing, strlen(outgoing), 0);
                }
                send(clients[i].fd, outgoing, strlen(outgoing), 0);
            }
        }
    }
    close(server_fd);
    return 0;
}
