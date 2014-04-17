/*
 * Socket.c
 *
 * Copyright (C) 2014 - Wiky L
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#include "Socket.h"

/*
 * !!(x)保证非0即1
 */
#define G_LIKELY(x) __builtin_expect(!!(x), 1)
#define G_UNLIKELY(x) __builtin_expect(!!(x), 0)

static inline void sys_exit(const char *errString)
{
	perror(errString);
	exit(errno);
}

int Socket(int family, int type, int protocol)
{
	int sockfd = socket(family, type, protocol);
	if (G_UNLIKELY(sockfd < 0)) {
		sys_exit("Fail to create socket descriptor");
	}
	return sockfd;
}

int Close(int fd)
{
	if (G_UNLIKELY(close(fd) != 0)) {
		sys_exit("Fail to close descriptor");
	}
	return 0;
}

int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret = bind(sockfd, addr, addrlen);
	if (G_UNLIKELY(ret != 0)) {
		sys_exit("Fail to bind local address to socket descriptor");
	}
	return ret;
}

/* 允许通过指定环境变量LISTENQ来设置backlog的值 */
int Listen(int sockfd, int backlog)
{
	char *ptr;
	ptr = getenv("LISTENQ");

	if (G_UNLIKELY(ptr != NULL)) {
		/* 一般情况下不会有LISTENQ */
		backlog = atoi(ptr);
	}

	int ret = listen(sockfd, backlog);
	if (G_UNLIKELY(ret != 0)) {
		sys_exit("Fail to mark sockfd as a passive socket");
	}
	return ret;
}
