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
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>

/* int socket(int family,int type,int protocl) */
int Socket(int family, int type, int protocl);

/* int close(int fd) */
int Close(int fd);

/* int bind(int socket, const struct sockaddr *address,
			socklen_t address_len); */
int Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

/* int listen(int sockfd, int backlog); */
int Listen(int sockfd, int backlog);
