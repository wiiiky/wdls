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

	char *line = NULL;
	int start = 1;
	while ((line = http_readline(sockfd))) {
		if (!start) {			/* 首行 */

		} else {
			start = 0;
		}
		if (line[0] == '\0') {	/* 空行，意味着首部结束 */
			free(line);
			break;
		}
		free(line);
	}

	Close(sockfd);
	http_thread_arg_free(args);
	printf("A HTTP thread quits\n");
	return NULL;
}

HttpStartLine *http_start_line_new(HttpMethod method,
								   const char *url, HttpVersion version)
{
	HttpStartLine *line = (HttpStartLine *) Malloc(sizeof(HttpStartLine));
	line->method = method;
	Memcpy(line->url, url, HTTP_URL_MAX - 1);
	line->version = version;
	return line;
}

/*
 * 解析 GET /index.html HTTP/1.1
 * 第一：必须是被空格分开的三部分，不允许tab
 * 第二：第一部分必须是GET、POST等方法名，必须大写
 * 第三：第二部分在这里任意
 * 第四：第三部分必须是HTTP/0.9、HTTP/1.0、HTTP/1.1中的一个
 */
HttpStartLine *http_start_line_parse(const char *line)
{
	if (line == NULL)
		return NULL;

	char *first = NULL;
	char *second = NULL;
	char *third = NULL;
	HttpStartLine *startLine = NULL;

	int i = 0;
	int cur = 0;
	int start, end;
	while (line[i] != '\0') {
		switch (cur) {
		case 0:				/* 刚开始，还没有解析到任何数据，如果有空格，可以忽略 */
			if (line[i] != ' ') {
				/* 遇到非空格字符,认为是第一部分的内容 */
				cur++;
				start = i;
			}
			break;
		case 1:				/* 正在处理第一部分 */
			if (line[i] == ' ') {
				/* 第一部分结束,复制 */
				end = i;
				first = Strndup(line + start, end - start);
				cur++;
			}
			break;
		case 2:				/* 第一部分与第二部分的空格区域 */
			if (line[i] != ' ') {
				cur++;
				start = i;
			}
			break;
		case 3:
			if (line[i] == ' ') {
				end = i;
				cur++;
				second = Strndup(line + start, end - start);
			}
			break;
		case 4:
			if (line[i] != ' ') {
				cur++;
				start = i;
			}
			break;
		case 5:
			if (line[i] == ' ') {
				end = i;
				cur++;
				third = Strndup(line + start, end - start);
			}
			break;
		default:
			if (line[i] != ' ') {
				/* 末尾有多余内容 */
				goto OUT;
			}
			break;
		}
		i++;
	}

	if (first && second && end <= start)
		third = Strndup(line + start, i - start);
	if (first == NULL || second == NULL || third == NULL)
		goto OUT;

	/* 目前只支持GET和HTTP/1.1 */
	if (Strcmp(first, "GET") || Strcmp(third, "HTTP/1.1"))
		goto OUT;

	startLine = http_start_line_new(HTTP_GET, second, HTTP_1_1);
  OUT:
	Free(first);
	Free(second);
	Free(third);
	return startLine;
}

void http_start_line_free(HttpStartLine * line)
{
	Free(line);
}

HttpHeader *http_header_new(const char *name, const char *value)
{
	HttpHeader *header = (HttpHeader *) Malloc(sizeof(HttpHeader));
	header->name = Strdup(name);
	header->value = Strdup(value);
}

void http_header_free(HttpHeader * header)
{
	if (header == NULL)
		return;
	Free(header->name);
	Free(header->value);
	Free(header);
}

HttpRequest *http_request_new()
{
	HttpRequest *req = (HttpRequest *) Malloc(sizeof(HttpRequest));
	req->startLine = NULL;
	req->headers = NULL;
}

void http_request_free(HttpRequest * req)
{
	if (req == NULL)
		return;
	if (req->startLine)
		http_start_line_free(req->startLine);
	if (req->headers)
		dlist_free_full(req->headers, (DestroyNotify) http_header_free);
	Free(req);
}
