/*
 * httpbuff.c
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
#include "httpbuff.h"
#include "Socket.h"


#define BUFFER_INIT_SIZE	(1024)

typedef struct {
	char *buffer;
	int max;					/* 总长度 */
	int len;					/* 当前长度 */
} PthreadData;


/* 如果扩大了返回1，否则返回0 */
static inline int expand_buffer(PthreadData * pdata)
{
	if (pdata->len >= pdata->max) {
		pdata->max = pdata->max * 2;
		pdata->buffer = (char *) Realloc(pdata->buffer, pdata->max);
		return 1;
	}
	return 0;
}

/* 如果buffer中有一行，则返回，否则NULL */
static inline char *get_line(PthreadData * pdata)
{
	int i;
	for (i = 0; i < pdata->len - 1; i++) {
		if (pdata->buffer[i] == '\r' && pdata->buffer[i + 1] == '\n') {
			/* 找到\r\n */
			char *ptr = Strndup(pdata->buffer, i);
			int j = 0;
			int dist = i + 2;
			pdata->len -= dist;
			for (j = 0; j < pdata->len; j++) {
				/* 将buffer中的数据前移 */
				pdata->buffer[j] = pdata->buffer[j + dist];
			}
			return ptr;
		}
	}
	return NULL;
}

/* 线程专有数据 Thread-Specific Data */
static pthread_key_t key;
static pthread_once_t key_once = PTHREAD_ONCE_INIT;

/* 销毁线程专有数据 */
static void destroy_thread_data(void *ptr)
{
	if (ptr == NULL)
		return;
	PthreadData *pdata = (PthreadData *) ptr;
	if (pdata->buffer)
		free(pdata->buffer);
	free(pdata);
}

/* 每个线程中此函数只会调用一次 */
static void make_thread_key(void)
{
	(void) pthread_key_create(&key, destroy_thread_data);
}

char *http_readline(int sockfd)
{
	void *ptr = NULL;
	(void) pthread_once(&key_once, make_thread_key);

	if ((ptr = pthread_getspecific(key)) == NULL) {
		/* 第一次调用，线程专有数据还未初始化 */
		ptr = Malloc(sizeof(PthreadData));
		PthreadData *pData = (PthreadData *) ptr;
		pData->max = BUFFER_INIT_SIZE;	/* 初始化缓冲区长度为1024 */
		pData->len = 0;
		pData->buffer = (char *) Malloc(sizeof(char) * pData->max);
		pData->buffer[0] = '\0';
		(void) pthread_setspecific(key, ptr);
	}
	PthreadData *pdata = (PthreadData *) ptr;

	char *line = NULL;
	line = get_line(pdata);
	if (line)					/* 如果缓冲区已经有一整行，则直接返回该行 */
		return line;

	expand_buffer(pdata);
	int n = 0;
	while ((n = Read(sockfd,
					 pdata->buffer + pdata->len,
					 pdata->max - pdata->len))) {
		if (n == 0) {
			/* EOF */
			/* 读到结尾，则直接返回缓存的所有数据 */
			pdata->len = 0;
			return Strdup(pdata->buffer);
		}
		pdata->len += n;
		line = get_line(pdata);
		if (line)
			return line;
		expand_buffer(pdata);
	}
	return NULL;
}
