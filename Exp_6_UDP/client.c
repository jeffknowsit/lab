/* Client-Server communication using UDP */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
    int sock;
    struct sockaddr_in server;
    socklen_t server_len = sizeof(server);
    char message[1024], reply[2048];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(9002);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Enter new-generation English sentence: ");
    if (!fgets(message, sizeof(message), stdin)) { close(sock); return 1; }
    if (sendto(sock, message, strlen(message), 0,
               (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("sendto"); close(sock); return 1;
    }

    ssize_t n = recvfrom(sock, reply, sizeof(reply) - 1, 0,
                         (struct sockaddr *)&server, &server_len);
    if (n < 0) { perror("recvfrom"); close(sock); return 1; }
    reply[n] = '\0';
    printf("Formal English: %s", reply);
    close(sock);
    return 0;
}
