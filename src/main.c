/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
 * Copyright (C) 2014 Wiky L <wiiiky@yeah.net>
 * 
 * wdls is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * wdls is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "net.h"
#include "arg.h"
#include "http.h"

static void signalHandler(int sigid);
static void echoListening();

#define BACKLOG	 (10)

int main(int argc, char *argv[])
{
	arg_init(argc, argv);

	int sockfd = Socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr_set((struct sockaddr *) &addr, AF_INET, arg_get_port());
	Bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
	Listen(sockfd, BACKLOG);

	echoListening();

	/* 注册信号CTRL+C的回调函数 */
	Signal(SIGINT, signalHandler);

	int accfd;
	HttpThreadArg *arg = http_thread_arg_new();
	while ((accfd =
			Accept(sockfd, (struct sockaddr *) &arg->addr,
				   &arg->addrlen)) > 0) {
		arg->sockfd = accfd;
		http_thread(arg);
		arg = http_thread_arg_new();
	}
	perror("What?");
	http_thread_arg_free(arg);

	Close(sockfd);
	return 0;
}


static void signalHandler(int sigid)
{
	printf("SIGINT is caught!\n");
	printf("Cancelled by user!\n");
	exit(0);
}

static void echoListening()
{
	printf(" Server address: http://localhost:%d\n", arg_get_port());
	printf(" Server root path: %s\n", arg_get_root());
	printf(" Server running... press ctrl+c to stop.\n");
}
