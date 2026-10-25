/* Concurrent Time Server Application using UDP */
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
    int sock;
    struct sockaddr_in address, client;
    socklen_t client_len;
    char message[64], time_text[128];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(8080);

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind"); close(sock); return 1;
    }
    printf("UDP Time Server started on port 8080...\n");

    while (1) {
        client_len = sizeof(client);
        ssize_t n = recvfrom(sock, message, sizeof(message) - 1, 0,
                             (struct sockaddr *)&client, &client_len);
        if (n < 0) { perror("recvfrom"); continue; }
        message[n] = '\0';
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        if (tm_info == NULL || strftime(time_text, sizeof(time_text),
                                        "%Y-%m-%d %H:%M:%S", tm_info) == 0)
            strcpy(time_text, "Time unavailable");
        if (sendto(sock, time_text, strlen(time_text) + 1, 0,
                   (struct sockaddr *)&client, client_len) < 0) perror("sendto");
        printf("Time sent to client: %s\n", time_text);
    }
    close(sock);
    return 0;
}
