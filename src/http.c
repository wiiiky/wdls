/*
 * http.c
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
#include "http.h"
#include "httpbuff.h"


#define BUFFERSIZE  (1024)

static void *http_pthread(void *arg);

HttpThreadArg *http_thread_arg_new(void)
{
	HttpThreadArg *arg = (HttpThreadArg *) Malloc(sizeof(HttpThreadArg));
	arg->addr =
		(struct sockaddr *) Malloc(sizeof(struct sockaddr_storage));
	arg->addrlen = sizeof(struct sockaddr_storage);
	arg->sockfd = -1;
	return arg;
}

void http_thread_arg_free(HttpThreadArg * arg)
{
	if (G_UNLIKELY(arg == NULL))
		return;
	Free(arg->addr);
	Free(arg);
}


void http_thread(HttpThreadArg * arg)
{
	pthread_t tid;
	Pthread_create(&tid, NULL, http_pthread, arg);
	Pthread_detach(tid);
}

static void *http_pthread(void *arg)
{
	HttpThreadArg *args = (HttpThreadArg *) arg;
	int sockfd = args->sockfd;
	const struct sockaddr *addr = args->addr;
	socklen_t addrlen = args->addrlen;

	char buff[BUFFERSIZE];
	int n;
	//while ((n = Read(sockfd, buff, BUFFERSIZE))) {
	//printf("%s", buff);
	//}
	char *line = NULL;
	while ((line = http_readline(sockfd))) {
		printf("%s!!\n", line);
		free(line);
	}

	http_thread_arg_free(args);
	printf("A HTTP thread quits\n");
	return NULL;
}
