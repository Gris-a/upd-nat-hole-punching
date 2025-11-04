#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>

const char *IP;
uint16_t PORT;

#define BUFFER_SIZE 4096

struct PeerInfo {
    struct sockaddr_in private;
    struct sockaddr_in public;
};

int init() {
    struct sockaddr_in address = {
        .sin_family = AF_INET, 
        .sin_port = htons(0),
        .sin_addr = {.s_addr = INADDR_ANY}
    };

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr *)&address, sizeof(address));
    
    return sock;
}

void start(int sock, struct sockaddr_in *peer) {
    char buffer[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';

    char *msg = "message\n";
    while (true) {
        send(sock, msg, sizeof(msg), 0);

        ssize_t bytes = recv(sock, buffer, BUFFER_SIZE, 0);
        if (bytes == -1) {
            break;
        }
        buffer[bytes] = '\0';
        printf("%s\n", buffer);
    }

    close(sock);
    exit(EXIT_SUCCESS);
}

int main(int argc, const char **argv) {    
    IP = argv[1];
    PORT = strtoul(argv[2], NULL, 10);

    char buffer[BUFFER_SIZE + 1];
    buffer[BUFFER_SIZE] = '\0';

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(PORT),
        .sin_addr = {.s_addr = inet_addr(IP)}
    };
    socklen_t addr_len = sizeof(struct sockaddr_in);

    int sock = init();
    connect(sock, (const struct sockaddr *)&server_addr, addr_len);
    
    struct sockaddr_in address;
    getsockname(sock, (struct sockaddr *)&address, &addr_len);

    struct PeerInfo peer;

    send(sock, &address, sizeof(struct sockaddr_in), 0);
    recv(sock, &peer, sizeof(peer), 0);

    char p2p_msg[] = "HOLE_PUNCH_REQUEST";

    connect(sock, (const struct sockaddr *)&peer.public, addr_len);
    for (size_t i = 0; i < 5; ++i) {
        send(sock, p2p_msg, sizeof(p2p_msg), 0);
        usleep(100000);
        if (recv(sock, buffer, BUFFER_SIZE, MSG_DONTWAIT) > 0) {
            printf("P2P SUCCESS! Established connection via PUBLIC address.\n");
            start(sock, &peer.public);
        }
    }

    connect(sock, (const struct sockaddr *)&peer.private, addr_len);
    for (size_t i = 0; i < 5; ++i) {
        send(sock, p2p_msg, sizeof(p2p_msg), 0);
        usleep(100000);
        if (recv(sock, buffer, BUFFER_SIZE, MSG_DONTWAIT) > 0) {
            printf("P2P SUCCESS! Established connection via PRIVATE (LAN) address.\n");
            start(sock, &peer.private);
        }
    }
}