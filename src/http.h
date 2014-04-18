/*
 * http.h
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
 * http处理
 */
#include "Socket.h"

/* 线程的参数 */
typedef struct {
	int sockfd;
	struct sockaddr *addr;
	socklen_t addrlen;
} HttpThreadArg;

HttpThreadArg *http_thread_arg_new();
void http_thread_arg_free(HttpThreadArg * arg);

/*
 * @description 处理一个HTTP链接的线程
 * @param pthread_create()
 */
void http_thread(HttpThreadArg * arg);
