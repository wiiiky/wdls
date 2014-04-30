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

static void *httpPthread(void *arg);

/* 删除字符串首尾的空格 */
static char *deleteRedundantSpace(const char *str);

HttpThreadArg *http_thread_arg_new(void)
{
	HttpThreadArg *arg = (HttpThreadArg *) Malloc(sizeof(HttpThreadArg));
	memset(&arg->addr, 0, sizeof(struct sockaddr_storage));
	arg->addrlen = sizeof(struct sockaddr_storage);
	arg->sockfd = -1;
	return arg;
}

void http_thread_arg_free(HttpThreadArg * arg)
{
	if (G_UNLIKELY(arg == NULL))
		return;
	Free(arg);
}


void http_thread(HttpThreadArg * arg)
{
	pthread_t tid;
	Pthread_create(&tid, NULL, httpPthread, arg);
	Pthread_detach(tid);
}

static void *httpPthread(void *arg)
{
	HttpThreadArg *args = (HttpThreadArg *) arg;
	int sockfd = args->sockfd;
	socklen_t addrlen = args->addrlen;

	char *line = NULL;
	int first = 1;
	/* 
	 * 获取http请求
	 * 请求中HTTP首行必须是有效的，
	 * 其他首部可有可无
	 */
	HttpRequest *request = http_request_new();
	while ((line = http_readline(sockfd))) {
		if (line == NULL) {		/* 出错 */
			goto CLOSE;
		} else if (line[0] == '\0') {	/* 空行，意味着首部结束 */
			Free(line);
			break;
		} else if (first) {		/* 首行 */
			first = 0;
			request->startLine = http_start_line_parse(line);	/* 解析首行 */
			if (request->startLine == NULL) {
				goto CLOSE;
			}
		} else {				/* HTTP首部 */
			http_request_add_header_from_line(request, line);
		}
		Free(line);
	}

	printf("%s\n",http_start_line_to_string(request->startLine));
	Dlist *headers = request->headers;
	while (headers) {
		HttpHeader *header = (HttpHeader *) headers->data;
		printf("%s:%s\n", header->name, header->value);
		headers = dlist_next(headers);
	}

  CLOSE:
	Close(sockfd);
	http_request_free(request);
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
char *http_start_line_to_string(HttpStartLine *line)
{
	if(line==NULL){
		return NULL;
	}
	const char *method=NULL;
	switch(line->method){
		case HTTP_GET:
			method="GET";
			break;
		case HTTP_POST:
			method="POST";
			break;
		default:
			return NULL;
	}

	const char *version=NULL;
	switch(line->version){
		case HTTP_1_1:
			version="HTTP/1.1";
			break;
		case HTTP_1_0:
			version="HTTP/1.0";
			break;
		default:
			return NULL;
	}

	int len=strlen(method)+strlen(version)+4;
	char *string=(char*)Malloc(sizeof(char)*len);
	snprintf(string,len,"%s %s %s",method,line->url,version);

	return string;
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
	int start = 0, end = 0;
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
	return header;
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
	return req;
}

void http_request_add_header(HttpRequest * req, HttpHeader * header)
{
	if (req == NULL || header == NULL) {
		return;
	}
	req->headers = dlist_append(req->headers, header);
}

void http_request_add_header_from_line(HttpRequest * req, const char *line)
{
	if (req == NULL || line == NULL) {
		return;
	}
	HttpHeader *header = http_header_parse(line);
	if (header == NULL) {
		return;
	}
	http_request_add_header(req, header);
}

void http_request_add_start_line(HttpRequest * req,
								 HttpStartLine * startLine)
{
	if (req == NULL) {
		return;
	}
	http_start_line_free(req->startLine);
	req->startLine = startLine;
}

void http_request_add_start_line_from_line(HttpRequest * req,
										   const char *line)
{
	if (req == NULL || line == NULL) {
		return;
	}
	HttpStartLine *startLine = http_start_line_parse(line);
	http_request_add_start_line(req, startLine);
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

/* 解析HTTP首部，生成HttpHeader结构 */
HttpHeader *http_header_parse(const char *line)
{
	if (G_UNLIKELY(line == NULL)) {
		return NULL;
	}

	char *semicolon = strchr(line, ':');
	if (semicolon == NULL) {	/* 没有找到分号，解析失败 */
		return NULL;
	}

	char *name = Strndup(line, semicolon - line);
	char *value = Strdup(semicolon + 1);
	if (G_UNLIKELY(name == NULL || value == NULL)) {
		Free(name);
		Free(value);
		return NULL;
	}
	char *name_d=deleteRedundantSpace(name);
	char *value_d=deleteRedundantSpace(value);
	HttpHeader *header = http_header_new(name_d, value_d);
	Free(name_d);
	Free(value_d);
	Free(name);
	Free(value);
	return header;
}

static char *deleteRedundantSpace(const char *str)
{
	if (G_UNLIKELY(str == NULL)) {
		return NULL;
	}
	const char *start = NULL;
	const char *end = NULL;
	while (*str) {
		if (start == NULL) {
			if (*str != ' ') {
				start = str;	/* 第一个不是空格的位置 */
				end = str;
			}
		} else if (*str != ' ') {
			end = str;		/* 最后一个不是空格的位置 */
		}
		str++;
	}
	if (start == NULL) {		/* 这种情况是字符串str全空格 */
		return Strdup("");
	}

	return Strndup(start, end - start + 1);
}
