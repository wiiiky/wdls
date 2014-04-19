/*
 * Socket.h
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

/*
 * 系统调用的包裹函数
 * 虽然这个文件命名为Socket.h
 * 但包含的函数不仅仅是定义在socket.h中的
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>


/*
 * !!(x)保证非0即1
 */
#define G_LIKELY(x) __builtin_expect(!!(x), 1)
#define G_UNLIKELY(x) __builtin_expect(!!(x), 0)

/* ssize_t read(int fildes, void *buf, size_t nbyte); */
ssize_t Read(int fildes, void *buf, size_t nbytes);

/* int socket(int family,int type,int protocl) */
int Socket(int family, int type, int protocl);

/* int close(int fd) */
int Close(int fd);

/* int bind(int socket, const struct sockaddr *address,
			socklen_t address_len); */
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* int listen(int sockfd, int backlog); */
int Listen(int sockfd, int backlog);

/* int accept(int socket, struct sockaddr *restrict address,
            socklen_t *restrict address_len); */
int Accept(int sockfd, struct sockaddr *address, socklen_t * address_len);

/*  void *malloc(size_t size); */
void *Malloc(size_t size);

/* void *realloc(void *ptr, size_t size); */
void *Realloc(void *ptr, size_t size);

/* void free(void *ptr) */
void Free(void *ptr);

/* char *strdup(const char *s); */
char *Strdup(const char *s);
/* char *strndup(const char *s, size_t n); */
char *Strndup(const char *s, size_t n);

/* int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
            void *(*start_routine) (void *), void *arg); */
int Pthread_create(pthread_t * thread, const pthread_attr_t * attr,
				   void *(*start_routine) (void *), void *arg);

/*  int pthread_detach(pthread_t thread); */
int Pthread_detach(pthread_t thread);

/* sighandler_t signal(int signum, sighandler_t handler); */
typedef void (*sighandler_t) (int);
sighandler_t Signal(int signum, sighandler_t handler);
