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

static inline void sys_exit(const char *errString)
{
	perror(errString);
	exit(errno);
}

ssize_t Read(int fildes, void *buf, size_t nbytes)
{
	ssize_t readn;
  AGAIN:
	readn = read(fildes, buf, nbytes);
	if (G_UNLIKELY(readn < 0)) {
		if (errno == EAGAIN)
			goto AGAIN;			/* 如果被中断，重试 */
		sys_exit("read error");
	}
	return readn;
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
		perror("Fail to close descriptor");
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

int Accept(int sockfd, struct sockaddr *address, socklen_t * address_len)
{
	int fd = accept(sockfd, address, address_len);
	if (G_UNLIKELY(fd < 0)) {
		sys_exit("Fail to accept connection");
	}
	return fd;
}

void *Malloc(size_t size)
{
	void *ptr = malloc(size);
	if (G_UNLIKELY(ptr == NULL)) {
		sys_exit("Fail to allocate memroy");
	}
	return ptr;
}

void *Realloc(void *ptr, size_t size)
{
	void *ret = realloc(ptr, size);
	if (G_UNLIKELY(ret == NULL)) {
		sys_exit("Fail to re-allocate memory");
	}
	return ret;
}

void *Memcpy(void *dest, const void *src, size_t n)
{
	void *ptr = memcpy(dest, src, n);
	if (G_UNLIKELY(ptr == NULL)) {
		sys_exit("Fail to copy memroy");
	}
	return ptr;
}

void Free(void *ptr)
{
	if (ptr == NULL)
		return;
	free(ptr);
}

char *Strdup(const char *s)
{
	if (s == NULL)
		return NULL;
	char *ptr = strdup(s);
	if (G_UNLIKELY(ptr == NULL)) {
		sys_exit("Fail to duplicate a string");
	}
	return ptr;
}

char *Strndup(const char *s, size_t n)
{
	if (s == NULL)
		return NULL;
	if (n == 0)
		return Strdup("\0");
	char *ptr = strndup(s, n);
	if (G_UNLIKELY(ptr == NULL)) {
		sys_exit("Fail to duplicate a string");
	}
	return ptr;
}

char *Strncpy(char *dest, const char *src, size_t n)
{
	if (src == NULL)
		return NULL;
	char *ptr = strncpy(dest, src, n);
	if (G_UNLIKELY(ptr == NULL)) {
		sys_exit("Fail to copy a string");
	}
	return ptr;
}

int Strcmp(const char *s1, const char *s2)
{
	if (G_UNLIKELY(s1 == NULL || s2 == NULL))
		return -1;
	return strcmp(s1, s2);
}

int Pthread_create(pthread_t * thread, const pthread_attr_t * attr,
				   void *(*start_routine) (void *), void *arg)
{
	int ret = pthread_create(thread, attr, start_routine, arg);
	if (G_UNLIKELY(ret != 0)) {
		errno = ret;
		sys_exit("Fail to create POSIX thread");
	}
	return ret;
}

int Pthread_detach(pthread_t thread)
{
	int ret = pthread_detach(thread);
	if (G_UNLIKELY(ret != 0)) {
		errno = ret;
		sys_exit("Fail to detach POSIX thread");
	}
	return ret;
}

sighandler_t Signal(int signum, sighandler_t handler)
{
	sighandler_t prev = signal(signum, handler);
	if (G_UNLIKELY(prev == SIG_ERR)) {
		sys_exit("Fail to set signal handler");
	}
	return prev;
}
