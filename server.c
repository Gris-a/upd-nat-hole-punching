#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

uint16_t PORT;

struct PeerInfo {
    struct sockaddr_in private;
    struct sockaddr_in public;
};

int init() {
    struct sockaddr_in address = {
        .sin_family = AF_INET, 
        .sin_port = htons(PORT),
        .sin_addr = {.s_addr = INADDR_ANY}
    };

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    bind(sock, (struct sockaddr *)&address, sizeof(address));
    
    return sock;
}

int main(int argc, const char **argv) {
    PORT = strtoul(argv[1], NULL, 10);

    int sock = init();
    
    struct PeerInfo client1, client2;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    recvfrom(sock, &client1.private, sizeof(struct sockaddr_in), 0, (struct sockaddr *)&client1.public, &addr_len);
    recvfrom(sock, &client2.private, sizeof(struct sockaddr_in), 0, (struct sockaddr *)&client2.public, &addr_len);

    sendto(sock, &client1, sizeof(client1), 0, (const struct sockaddr *)&client2.public, sizeof(struct sockaddr_in));
    sendto(sock, &client2, sizeof(client2), 0, (const struct sockaddr *)&client1.public, sizeof(struct sockaddr_in));

    close(sock);
}