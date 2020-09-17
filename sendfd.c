#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "scm_functions.h"

#define BUFSIZE 100

int main(int argc, char** argv) {
    struct sockaddr_un addr;
    char *socket_path = NULL;
    char *default_sock = "socket";
    char buf[BUFSIZE];
    int sockfd, filefd, rc;

    if (argc != 3) {
        printf("Usage: sendfd <unix socket> <file>\n");
        exit(-1);
    }

    /*
    if (argc > 2) {
	socket_path = argv[2];
    } else {
        socket_path = default_sock;
    }
    */
    socket_path = argv[1];

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

    filefd = open(argv[2], O_RDONLY);

    if ((rc = sendfd(sockfd, filefd)) == -1) {
        perror("sendfd failed");
        exit(-1);
    }

    return 0;
}
