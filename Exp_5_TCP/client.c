/* Client-Server communication using TCP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static int send_all(int fd, const void *data, size_t size) {
    size_t sent = 0;
    while (sent < size) {
        ssize_t n = send(fd, (const char *)data + sent, size - sent, 0);
        if (n <= 0) return -1;
        sent += (size_t)n;
    }
    return 0;
}


int main(void) {
    int sock, n, i, j, matrix[10][10] = {{0}};
    struct sockaddr_in server;
    char result[128] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect"); close(sock); return 1;
    }
    printf("Connected to server!\n");
    printf("Enter matrix order N: ");
    if (scanf("%d", &n) != 1 || n < 1 || n > 10) {
        printf("Invalid matrix order\n"); close(sock); return 1;
    }

    if (send_all(sock, &n, sizeof(n)) < 0) {
        perror("send"); close(sock); return 1;
    }

    srand((unsigned int)getpid());
    printf("Generated %d x %d Matrix:\n", n, n);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            if (i < j) matrix[i][j] = rand() % 50 + 1;
            else if (i > j) matrix[i][j] = 0;
            else matrix[i][j] = rand() % 50 + 1;
            printf("%d ", matrix[i][j]);
        }
        printf("\n");
    }

    if (send_all(sock, matrix, (size_t)n * n * sizeof(int)) < 0) {
        perror("send"); close(sock); return 1;
    }
    if (recv(sock, result, sizeof(result) - 1, 0) <= 0) {
        perror("recv"); close(sock); return 1;
    }
    printf("Server identified matrix as: %s\n", result);
    close(sock);
    return 0;
}
