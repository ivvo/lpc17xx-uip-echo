#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define BUFF_SIZE 32

int main(int argc, char **argv)
{
    int sockfd;
    char buffer[BUFF_SIZE];

    struct sockaddr_in sock_addr = {    
        .sin_family = AF_INET,
        .sin_port   = htons(80),
        .sin_addr   = inet_addr("192.168.0.2"),
    };

    bzero(sock_addr.sin_zero, sizeof(sock_addr.sin_zero));

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket()");
        exit(1);
    }

    if (connect(sockfd, (struct sockaddr*) &sock_addr, sizeof(struct sockaddr_in)) == -1) {
        perror("socket()");
        exit(1);
    }

    send(sockfd, "Hello", 5, 0);
    printf(">> Hello\n");

    int bytes = recv(sockfd, buffer, BUFF_SIZE, 0);
    buffer[bytes]=0;

    printf("<< %s\n", buffer);
	return 0;
}
