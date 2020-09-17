#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "scm_functions.h"

#define BUFSIZE 100

int main(int argc, char** argv) {
    struct sockaddr_un addr;
    char *socket_path = NULL;
    char *default_sock = "socket";
    char buf[BUFSIZE];
    int sockfd, filefd, rc, bytes;

    if (argc < 2) {
        printf("Usage: recvfd <command> [SOCKET]\n");
        exit(-1);
    }

    if (argc > 2) {
	socket_path = argv[2];
    } else {
        socket_path = default_sock;
    }

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ) {
	perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (connect(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("connect error");
        exit(-1);
    }

    if ((filefd = recvfd(sockfd)) == -1) {
        printf("recvfd failed: %d\n", filefd);
        exit(-1);
    }

    lseek(filefd, 0, SEEK_SET);

    memset(buf, '\0', BUFSIZE);
    while(bytes = read(filefd, buf, BUFSIZE-1) > 0) {
        fprintf(stdout, buf, bytes);
        memset(buf, '\0', BUFSIZE);
    }

    return 0;
}
