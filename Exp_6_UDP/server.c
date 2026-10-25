/* Client-Server communication using UDP */
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

static void replace_word(char *text, const char *short_word, const char *full_word) {
    char output[2048] = "";
    char *token = strtok(text, " \t\r\n");
    while (token != NULL) {
        if (output[0] != '\0') strncat(output, " ", sizeof(output) - strlen(output) - 1);
        if (strcasecmp(token, short_word) == 0)
            strncat(output, full_word, sizeof(output) - strlen(output) - 1);
        else
            strncat(output, token, sizeof(output) - strlen(output) - 1);
        token = strtok(NULL, " \t\r\n");
    }
    strncpy(text, output, 1023);
    text[1023] = '\0';
}

static void translate(char *text) {
    replace_word(text, "tbh", "to be honest");
    replace_word(text, "ig", "I guess");
    replace_word(text, "tbf", "to be fair");
    replace_word(text, "atm", "at the moment");
    replace_word(text, "irl", "in real life");
    replace_word(text, "lol", "laughing out loud");
    replace_word(text, "asap", "as soon as possible");
    replace_word(text, "omg", "oh my God");
    replace_word(text, "ttyl", "talk to you later");
    replace_word(text, "idk", "I don't know");
    replace_word(text, "nvm", "never mind");
}

int main(void) {
    int sock;
    struct sockaddr_in address, client;
    socklen_t client_len = sizeof(client);
    char message[1024];

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return 1; }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(9002);

    if (bind(sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind"); close(sock); return 1;
    }
    printf("UDP Server is waiting for messages...\n");

    ssize_t n = recvfrom(sock, message, sizeof(message) - 1, 0,
                         (struct sockaddr *)&client, &client_len);
    if (n < 0) { perror("recvfrom"); close(sock); return 1; }
    message[n] = '\0';
    printf("Received: %s", message);
    translate(message);
    printf("Sent: %s\n", message);

    if (sendto(sock, message, strlen(message), 0,
               (struct sockaddr *)&client, client_len) < 0) perror("sendto");
    close(sock);
    return 0;
}
