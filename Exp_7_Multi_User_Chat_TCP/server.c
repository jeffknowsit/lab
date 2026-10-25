/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define MAX_CLIENTS 20
#define NAME_SIZE 64
#define MESSAGE_SIZE 1024
#define BUFFER_SIZE 4096

struct client {
    int fd;
    char name[NAME_SIZE];
    char buffer[BUFFER_SIZE];
    size_t used;
};

int main(void) {
    int server_fd, opt = 1, i;
    struct sockaddr_in address;
    struct client clients[MAX_CLIENTS];

    signal(SIGPIPE, SIG_IGN);
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].used = 0;
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(server_fd, 10) < 0) {
        perror("bind/listen");
        close(server_fd);
        return 1;
    }

    printf("TCP Chat Server started on port 5000...\n");
    fflush(stdout);

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
                if (i == MAX_CLIENTS) {
                    close(fd);
                } else {
                    char name[NAME_SIZE];
                    ssize_t n = recv(fd, name, sizeof(name) - 1, 0);
                    if (n <= 0) {
                        close(fd);
                    } else {
                        name[n] = '\0';
                        name[strcspn(name, "\r\n")] = '\0';
                        strncpy(clients[i].name, name, NAME_SIZE - 1);
                        clients[i].name[NAME_SIZE - 1] = '\0';
                        clients[i].fd = fd;
                        clients[i].used = 0;
                        printf("New user connected: %s\n", clients[i].name);
                        fflush(stdout);
                    }
                }
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0 && FD_ISSET(clients[i].fd, &readfds)) {
                char incoming[MESSAGE_SIZE];
                ssize_t n = recv(clients[i].fd, incoming, sizeof(incoming), 0);
                if (n <= 0) {
                    printf("User %s disconnected.\n", clients[i].name);
                    fflush(stdout);
                    close(clients[i].fd);
                    clients[i].fd = -1;
                    clients[i].used = 0;
                    continue;
                }

                for (ssize_t k = 0; k < n; k++) {
                    if (incoming[k] == '\0') {
                        clients[i].buffer[clients[i].used] = '\0';
                        if (strcmp(clients[i].buffer, "exit") == 0) {
                            printf("User %s disconnected.\n", clients[i].name);
                            fflush(stdout);
                            close(clients[i].fd);
                            clients[i].fd = -1;
                            clients[i].used = 0;
                            break;
                        }

                        char outgoing[NAME_SIZE + MESSAGE_SIZE + 4];
                        snprintf(outgoing, sizeof(outgoing), "%s: %s",
                                 clients[i].name, clients[i].buffer);
                        printf("%s\n", outgoing);
                        fflush(stdout);

                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].fd >= 0) {
                                if (send(clients[j].fd, outgoing, strlen(outgoing) + 1, 0) < 0) {
                                    close(clients[j].fd);
                                    clients[j].fd = -1;
                                    clients[j].used = 0;
                                }
                            }
                        }
                        clients[i].used = 0;
                    } else if (clients[i].used < sizeof(clients[i].buffer) - 1) {
                        clients[i].buffer[clients[i].used++] = incoming[k];
                    } else {
                        clients[i].used = 0;
                    }
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
