#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include "scm_functions.h"

static volatile int doLoop = 1;

void intHandler(int signum) {
    
    printf("SIGINT\n");
    doLoop = 0;
}

int main(int argc, char** argv) {
    struct sockaddr_un addr;
    char *socket_path = NULL;
    char *default_sock = "socket";
    char buf[100];
    int sockfd, filefd, cl, rc;
    struct sigaction new_action, old_action;

    new_action.sa_handler = intHandler;
    sigemptyset(&new_action.sa_mask);
    new_action.sa_flags = 0;

    sigaction(SIGINT, &new_action, NULL);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGINT, &new_action, NULL);

    sigaction (SIGHUP, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGHUP, &new_action, NULL);

    sigaction (SIGTERM, NULL, &old_action);
    if (old_action.sa_handler != SIG_IGN)
        sigaction (SIGTERM, &new_action, NULL);

    if (argc < 2) {
        printf("Usage: sendfd <FILE> [SOCKET]\n");
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

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        exit(-1);
    }

    filefd = open(argv[1], O_RDONLY);

    if (listen(sockfd, 5) == -1) {
        perror("listen error");
        exit(-1);
    }


    while (doLoop == 1) {
        if ( (cl = accept(sockfd, NULL, NULL)) == -1) {
            perror("accept error");
            continue;
        }

        printf("after accept\n");
	rc = sendfd(sockfd, filefd);
    
        if (rc == -1) {
            perror("send_fd failure");
            exit(-1);
        }
        else if (rc == 0) {
            printf("send_fd success\n");
            close(cl);
        }
    }
    unlink(socket_path);

    return 0;
}
