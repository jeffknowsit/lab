/* Multi-user Chat Server Using TCP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
    int sock;
    struct sockaddr_in server;
    char name[64], message[1024], reply[1200];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(5000);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }
    printf("Connected to chat server.\nEnter your name: ");
    if (!fgets(name, sizeof(name), stdin)) { close(sock); return 1; }
    name[strcspn(name, "\r\n")] = '\0';
    if (send(sock, name, strlen(name) + 1, 0) <= 0) {
        perror("send"); close(sock); return 1;
    }
    printf("You are connected as %s.\n", name);
    printf("Type your message and press Enter.\nType 'exit' to quit.\n");

    while (1) {
        printf("%s: ", name);
        if (!fgets(message, sizeof(message), stdin)) break;
        message[strcspn(message, "\r\n")] = '\0';
        if (send(sock, message, strlen(message) + 1, 0) <= 0) break;
        if (strcmp(message, "exit") == 0) break;
        ssize_t n = recv(sock, reply, sizeof(reply) - 1, 0);
        if (n <= 0) break;
        reply[n] = '\0';
        printf("%s\n", reply);
    }
    printf("Disconnected from server.\n");
    close(sock);
    return 0;
}
