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

	Close(sockfd);
	return 0;
}
