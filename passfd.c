/*-
 * Copyright (c) 2005 Robert N. M. Watson
 * Copyright (c) 2015 Mark Johnston
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "passfd.h"

#define ATF_REQUIRE(x) assert(x)
#define ATF_REQUIRE_MSG(a,b,...) assert(a)
#define SCM_CREDS SCM_CREDENTIALS

static void
putfds(char *buf, int fd, int nfds)
{
	struct cmsghdr *cm;
	int *fdp, i;

	cm = (struct cmsghdr *)buf;
	cm->cmsg_len = CMSG_LEN(nfds * sizeof(int));
	cm->cmsg_level = SOL_SOCKET;
	cm->cmsg_type = SCM_RIGHTS;
	for (fdp = (int *)CMSG_DATA(cm), i = 0; i < nfds; i++)
		*fdp++ = fd;
}

static size_t
sendfd_payload(int sockfd, int send_fd, void *payload, size_t paylen)
{
	struct iovec iovec;
	char message[CMSG_SPACE(sizeof(int))];
	struct msghdr msghdr;
	ssize_t len;

	bzero(&msghdr, sizeof(msghdr));
	bzero(&message, sizeof(message));

	msghdr.msg_control = message;
	msghdr.msg_controllen = sizeof(message);

	iovec.iov_base = payload;
	iovec.iov_len = paylen;

	msghdr.msg_iov = &iovec;
	msghdr.msg_iovlen = 1;

	putfds(message, send_fd, 1);
	len = sendmsg(sockfd, &msghdr, 0);
	ATF_REQUIRE_MSG(len != -1, "sendmsg failed: %s", strerror(errno));
	return ((size_t)len);
}

int sendfd(int sockfd, int send_fd)
{
        size_t len;
        char ch;

        ch = 0;
        len = sendfd_payload(sockfd, send_fd, &ch, sizeof(ch));
        return len;
}

static void
recvfd_payload(int sockfd, int *recv_fd, void *buf, size_t buflen,
    size_t cmsgsz)
{
	struct cmsghdr *cmsghdr;
	struct msghdr msghdr;
	struct iovec iovec;
	char *message;
	ssize_t len;
#if 0
	bool foundcreds;
#endif /* 0 */

	bzero(&msghdr, sizeof(msghdr));
	message = malloc(cmsgsz);
	ATF_REQUIRE(message != NULL);

	msghdr.msg_control = message;
	msghdr.msg_controllen = cmsgsz;

	iovec.iov_base = buf;
	iovec.iov_len = buflen;

	msghdr.msg_iov = &iovec;
	msghdr.msg_iovlen = 1;

	len = recvmsg(sockfd, &msghdr, 0);
#if 0
	ATF_REQUIRE_MSG(len != -1, "recvmsg failed: %s", strerror(errno));
	ATF_REQUIRE_MSG((size_t)len == buflen,
	    "recvmsg: %zd bytes received; expected %zd", len, buflen);
#endif /* 0 */

	cmsghdr = CMSG_FIRSTHDR(&msghdr);
#if 0
	ATF_REQUIRE_MSG(cmsghdr != NULL,
	    "recvmsg: did not receive control message");
	foundcreds = false;
#endif /* 0 */
	*recv_fd = -1;
	for (; cmsghdr != NULL; cmsghdr = CMSG_NXTHDR(&msghdr, cmsghdr)) {
		if (cmsghdr->cmsg_level == SOL_SOCKET &&
		    cmsghdr->cmsg_type == SCM_RIGHTS &&
		    cmsghdr->cmsg_len == CMSG_LEN(sizeof(int))) {
			memcpy(recv_fd, CMSG_DATA(cmsghdr), sizeof(int));
		}
	}
#if 0
			ATF_REQUIRE(*recv_fd != -1);
		}
	}
		} else if (cmsghdr->cmsg_level == SOL_SOCKET &&
		    cmsghdr->cmsg_type == SCM_CREDS)
			foundcreds = true;
	}
	ATF_REQUIRE_MSG(*recv_fd != -1,
	    "recvmsg: did not receive single-fd message");
	ATF_REQUIRE_MSG(!localcreds(sockfd) || foundcreds,
	    "recvmsg: expected credentials were not received");
#endif /* 0 */
}

int recvfd(int sockfd)
{
        int recv_fd = 0;
        char ch = 0;

        recvfd_payload(sockfd, &recv_fd, &ch, sizeof(ch),
            CMSG_SPACE(sizeof(int)));

        return recv_fd;
}
