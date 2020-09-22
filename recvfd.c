/*
 * Copyright (c) 2020, Wind River Systems, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Written by Rob Woolley <rob.woolley@windriver.com>
 */

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "passfd.h"

static volatile int doLoop = 1;

void intHandler(int signum) {
    doLoop = 0;
}

#define BUFSIZE 100

int main(int argc, char** argv) {
    struct sockaddr_un addr;
    char *socket_path = NULL;
    char buf[BUFSIZE];
    int sockfd, filefd, cl, rc, bytes;
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

    if (argc != 2) {
        printf("Usage: recvfd <SOCKET>\n");
        exit(-1);
    }

    socket_path = argv[1];

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1 ) {
	perror("socket error");
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path)-1);

    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind error");
        unlink(socket_path);
        exit(-1);
    }

    if (listen(sockfd, 1) == -1) {
        perror("listen error");
        unlink(socket_path);
        exit(-1);
    }

    while (doLoop == 1) {
        if ( (cl = accept(sockfd, NULL, NULL)) == -1) {
            perror("accept error");
            continue;
        }

	filefd = recvfd(cl);

        if (filefd == -1) {
            perror("recvfd failure");
            close(cl);

            exit(-1);
        }
#ifdef DEBUG
        else {
            lseek(filefd, 0, SEEK_SET);

            memset(buf, '\0', BUFSIZE);
            while(bytes = read(filefd, buf, BUFSIZE-1) > 0) {
                fprintf(stdout, buf, bytes);
                memset(buf, '\0', BUFSIZE);
            }

            close(cl);
        }
#endif /* DEBUG */
    }
    unlink(socket_path);

    return 0;
}
