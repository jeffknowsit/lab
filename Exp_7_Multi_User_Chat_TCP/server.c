/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
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

static void remove_client(struct client *c) {
    if (c->fd >= 0) close(c->fd);
    c->fd = -1;
    c->used = 0;
    c->name[0] = '\0';
}

int main(void) {
    int server_fd, opt = 1, i;
    struct sockaddr_in address;
    struct client clients[MAX_CLIENTS];

    signal(SIGPIPE, SIG_IGN);
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].fd = -1;
        clients[i].used = 0;
        clients[i].name[0] = '\0';
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return 1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(5000);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        if (errno == EACCES) {
            fprintf(stderr, "Permission denied binding port 5000. Try a port above 1024.\n");
        } else {
            perror("bind");
            if (errno == EADDRINUSE)
                fprintf(stderr, "Port 5000 is already in use. Stop the other server or change the port in both files.\n");
        }
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen");
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

        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR) continue;
            perror("select");
            break;
        }

        if (FD_ISSET(server_fd, &readfds)) {
            int fd = accept(server_fd, NULL, NULL);
            if (fd >= 0) {
                int slot;
                for (slot = 0; slot < MAX_CLIENTS && clients[slot].fd >= 0; slot++);
                if (slot == MAX_CLIENTS) {
                    const char *full = "Server full";
                    send_all(fd, full, strlen(full) + 1);
                    close(fd);
                } else {
                    char name[NAME_SIZE];
                    ssize_t n = recv(fd, name, sizeof(name) - 1, 0);
                    if (n <= 0) {
                        close(fd);
                    } else {
                        name[n] = '\0';
                        name[strcspn(name, "\r\n")] = '\0';
                        strncpy(clients[slot].name, name, NAME_SIZE - 1);
                        clients[slot].name[NAME_SIZE - 1] = '\0';
                        clients[slot].fd = fd;
                        clients[slot].used = 0;
                        printf("New user connected: %s\n", clients[slot].name);
                        fflush(stdout);
                    }
                }
            } else if (errno != EINTR) {
                perror("accept");
            }
        }

        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].fd >= 0 && FD_ISSET(clients[i].fd, &readfds)) {
                char incoming[MESSAGE_SIZE];
                ssize_t n = recv(clients[i].fd, incoming, sizeof(incoming), 0);
                if (n <= 0) {
                    printf("User %s disconnected.\n", clients[i].name);
                    fflush(stdout);
                    remove_client(&clients[i]);
                    continue;
                }

                for (ssize_t k = 0; k < n && clients[i].fd >= 0; k++) {
                    if (incoming[k] == '\0') {
                        clients[i].buffer[clients[i].used] = '\0';
                        if (strcmp(clients[i].buffer, "exit") == 0) {
                            printf("User %s disconnected.\n", clients[i].name);
                            fflush(stdout);
                            remove_client(&clients[i]);
                            break;
                        }

                        char outgoing[NAME_SIZE + BUFFER_SIZE + 4];
                        snprintf(outgoing, sizeof(outgoing), "%.63s: %.4095s",
                                 clients[i].name, clients[i].buffer);
                        printf("%s\n", outgoing);
                        fflush(stdout);

                        for (int j = 0; j < MAX_CLIENTS; j++) {
                            if (clients[j].fd >= 0 &&
                                send_all(clients[j].fd, outgoing, strlen(outgoing) + 1) < 0) {
                                remove_client(&clients[j]);
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

    for (i = 0; i < MAX_CLIENTS; i++) remove_client(&clients[i]);
    close(server_fd);
    return 0;
}
