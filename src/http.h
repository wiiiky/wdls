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
#include "list.h"

/* HTTP 方法 */
typedef enum {
	HTTP_GET,
	HTTP_POST,
	HTTP_HEAD,
	/* ... */
} HttpMethod;

/* HTTP 版本 */
typedef enum {
	HTTP_0_9,					/* HTTP/0.9 */
	HTTP_1_0,					/* HTTP/1.0 */
	HTTP_1_1,					/* HTTP/1.1 */
} HttpVersion;

/* 
 * URL最大长度1023,实际中的HTTP服务器一般允许更大的值，比如8192，甚至更大.
 * RFC中描述，HTTP协议没有指定最大的URL长度，要求服务器尽可能处理长URL，
 * 如果服务器无法处理过长URL，返回414
 */
#define HTTP_URL_MAX	(1024)

/* HTTP 请求的起始行 */
typedef struct {
	HttpMethod method;
	char url[HTTP_URL_MAX];
	HttpVersion version;
} HttpStartLine;
HttpStartLine *http_start_line_new(HttpMethod method,
								   const char *url, HttpVersion version);
/* 从HTTP的起始行中解析，成功返回HttpStartLine结构的指针，失败NULL */
HttpStartLine *http_start_line_parse(const char *line);
void http_start_line_free(HttpStartLine * line);
char *http_start_line_to_string(HttpStartLine * line);
#define http_start_line_get_method(line)    ((line)->method)
#define http_start_line_get_url(line)    ((line)->url)
#define http_start_line_get_version(line)    ((line)->version)

/* HTTP首部字段 name: value */
typedef struct {
	char *name;
	char *value;
} HttpHeader;

HttpHeader *http_header_new(const char *name, const char *value);
HttpHeader *http_header_parse(const char *line);
void http_header_free(HttpHeader * header);
#define http_header_get_name(hdr)    ((hdr)->name)
#define http_header_get_value(hdr)    ((hdr)->value)

typedef struct {
	HttpStartLine *startLine;
	Dlist *headers;				/* HttpHeader 的双向链表 */
} HttpRequest;
#define http_request_get_url(request)	(http_start_line_get_url((request)->startLine))
#define http_request_get_method(request)    (http_start_line_get_method((request)->startLine))
#define http_request_get_version(request)    (http_start_line_get_version((request)->startLine))
HttpRequest *http_request_new();
void http_request_free(HttpRequest * req);
void http_request_add_start_line(HttpRequest * req,
								 HttpStartLine * startLine);
void http_request_add_start_line_from_line(HttpRequest * req,
										   const char *line);
void http_request_add_header(HttpRequest * req, HttpHeader * header);
void http_request_add_header_from_line(HttpRequest * req,
									   const char *line);
char *http_request_to_string(HttpRequest * req);
/* 查找http首部,不区分大小写，不存在返回NULL */
const char *http_request_find_header(HttpRequest * req, const char *name);

/* 线程的参数 */
typedef struct {
	int sockfd;
	struct sockaddr_storage addr;
	socklen_t addrlen;
} HttpThreadArg;

HttpThreadArg *http_thread_arg_new();
void http_thread_arg_free(HttpThreadArg * arg);

/*
 * @description 处理一个HTTP链接的线程
 * @param pthread_create()
 */
void http_thread(HttpThreadArg * arg);
