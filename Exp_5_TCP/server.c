/* Client-Server communication using TCP */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static int recv_all(int fd, void *data, size_t size) {
    size_t got = 0;
    while (got < size) {
        ssize_t n = recv(fd, (char *)data + got, size - got, 0);
        if (n <= 0) return -1;
        got += (size_t)n;
    }
    return 0;
}

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
    int server_fd, client_fd, opt = 1, n, i, j;
    int matrix[10][10];
    int upper = 1, lower = 1, diagonal = 1;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char result[128];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 ||
        listen(server_fd, 5) < 0) {
        perror("bind/listen"); close(server_fd); return 1;
    }
    printf("Server waiting for client on port 8080...\n");
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0) { perror("accept"); close(server_fd); return 1; }
    printf("Client connected!\n");

    if (recv_all(client_fd, &n, sizeof(n)) < 0 || n < 1 || n > 10) {
        fprintf(stderr, "Invalid matrix size received\n");
        close(client_fd); close(server_fd); return 1;
    }
    printf("Matrix size received: %d x %d\n", n, n);
    if (recv_all(client_fd, matrix, (size_t)n * n * sizeof(int)) < 0) {
        perror("recv"); close(client_fd); close(server_fd); return 1;
    }

    printf("Received Matrix:\n");
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            printf("%d ", matrix[i][j]);
            if (i > j && matrix[i][j] != 0) upper = 0;
            if (i < j && matrix[i][j] != 0) lower = 0;
            if (i != j && matrix[i][j] != 0) diagonal = 0;
        }
        printf("\n");
    }

    if (diagonal) strcpy(result, "Diagonal Matrix");
    else if (upper) strcpy(result, "Upper Triangular Matrix");
    else if (lower) strcpy(result, "Lower Triangular Matrix");
    else strcpy(result, "Normal Matrix");

    if (send_all(client_fd, result, strlen(result) + 1) < 0) perror("send");
    printf("Result sent to client: %s\n", result);
    close(client_fd);
    close(server_fd);
    return 0;
}
