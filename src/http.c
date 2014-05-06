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
#include "arg.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#define BUFFERSIZE  (1024)

static void *httpPthread(void *arg);

/* 删除字符串首尾的空格 */
static char *deleteRedundantSpace(const char *str);

/*
 * @decription 根据指定的url以及HTTP的根目录给出资源路径
 * 				这里如果成功返回路径字符串，否则NULL
 * 				这里保证返回的路径在根目录下，在根目录外的
 * 				路径被认为是非法的（潜在的安全问题）
 * @return ...
 */
static char *getResourcePath(const char *path);

/*
 * @description 将指定资源返回给客户端
 *				如果没有找到资源，返回一个404页面
 * @param sockfd 网络套接字
 * @param path 资源路径
 */
static void echoToClient(int sockfd, const char *path);


/*
 * @description 生成HTTP响应404错误
 */
static const char *response404();
/*
 * @decription 生成HTTP响应200
 */
static char *response200();


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
	struct sockaddr_in *addr = (struct sockaddr_in *) &args->addr;
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

	/* 对请求作出相应 */
	char *path = getResourcePath(http_request_get_url(request));
	char *remote = inet_ntoa(addr->sin_addr);
	printf("\t%s:%d requests %s\n", remote, ntohs(addr->sin_port), path);

	echoToClient(sockfd, path);
	Free(path);

  CLOSE:
	Close(sockfd);
	http_request_free(request);
	http_thread_arg_free(args);
	/*printf("A HTTP thread quits\n"); */
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

char *http_start_line_to_string(HttpStartLine * line)
{
	if (line == NULL) {
		return NULL;
	}
	const char *method = NULL;
	switch (line->method) {
	case HTTP_GET:
		method = "GET";
		break;
	case HTTP_POST:
		method = "POST";
		break;
	default:
		return NULL;
	}

	const char *version = NULL;
	switch (line->version) {
	case HTTP_1_1:
		version = "HTTP/1.1";
		break;
	case HTTP_1_0:
		version = "HTTP/1.0";
		break;
	default:
		return NULL;
	}

	int len = strlen(method) + strlen(version) + strlen(line->url) + 4;
	char *string = (char *) Malloc(sizeof(char) * len);
	snprintf(string, len, "%s %s %s", method, line->url, version);

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

	/* 目前只支持GET和HTTP/1.1 HTTP/1.0 */
	if (Strcasecmp(first, "GET")) {
		goto OUT;
	}

	if (Strcasecmp(third, "HTTP/1.1") && Strcasecmp(third, "HTTP/1.0")) {
		goto OUT;
	}

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
	char *name_d = deleteRedundantSpace(name);
	char *value_d = deleteRedundantSpace(value);
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
			end = str;			/* 最后一个不是空格的位置 */
		}
		str++;
	}
	if (start == NULL) {		/* 这种情况是字符串str全空格 */
		return Strdup("");
	}

	return Strndup(start, end - start + 1);
}

const char *http_request_find_header(HttpRequest * req, const char *name)
{
	if (req == NULL || name == NULL) {
		return NULL;
	}

	Dlist *lp = req->headers;
	while (lp) {
		HttpHeader *hdr = (HttpHeader *) lp->data;
		if (Strcasecmp(http_header_get_name(hdr), name) == 0) {
			return hdr->value;
		}
		lp = dlist_next(lp);
	}
	return NULL;
}

/*
 * @description 判断是否为.
 * @return 是返回1，否则返回0
 */
static int isDot(const char *s)
{
	return s[0] == '.' && (s[1] == '/' || s[1] == '\0');
}

/*
 * @description 判断是否为..
 * @return 是返回1，否则返回0
 */
static int isDotDot(const char *s)
{
	return s[0] == '.' && s[1] == '.' && (s[2] == '/' || s[2] == '\0');
}

/*
 * 连接字符串，释放原来的s1
 */
static char *strJoin(char *s1, char *s2)
{
	if (s1 == NULL) {
		return Strdup(s2);
	} else if (s2 == NULL) {
		return s1;
	}
	int len = strlen(s2) + strlen(s1) + 2;
	char *res = (char *) Malloc(sizeof(char) * len);
	snprintf(res, len, "%s/%s", s1, s2);
	Free(s1);
	return res;
}

/*
 * 根据HTTP的根目录以及URL指定的路径
 * 获取资源的路径
 */
static char *getResourcePath(const char *path)
{
	if (path == NULL) {
		return NULL;
	}
	/* 使用链表来实现栈 */
	Dlist *stack = dlist_new();
	const char *strips = NULL;

	char *dir = NULL;

	while (*path) {
		if (*path != '/') {
			dlist_free_full(stack, Free);
			return NULL;
		}
		path++;
		strips = strchr(path, '/');
		if (strips) {
			dir = Strndup(path, strips - path);
			path = strips;
		} else {
			dir = Strdup(path);
			while (*path) {
				path++;
			}
		}
		if (dir[0] == '\0') {
			break;
		}
		if (isDot(dir)) {
			Free(dir);
			continue;
		} else if (isDotDot(dir)) {
			Dlist *last = dlist_last(stack);
			if (last == NULL) {
				return NULL;
			}
			if (last->prev == NULL) {
				stack = NULL;
				Free(last->data);
				Free(last);
			} else {
				last->prev = NULL;
				Free(last->data);
				Free(last);
			}
		} else {
			stack = dlist_append(stack, dir);
		}
	}
	const char *rootFile = "index.html";
	char *target = NULL;
	const char *root = arg_get_root();
	int rootLen = strlen(root);
	int targetLen = rootLen + 1;
	if (stack == NULL) {
		targetLen += 11;
		target = (char *) Malloc(sizeof(char) * targetLen);
		snprintf(target, targetLen, "%s/%s", root, rootFile);
		return target;
	}
	target = Strdup(root);
	Dlist *ptr = stack;
	while (ptr) {
		char *dir = (char *) ptr->data;
		target = strJoin(target, dir);
		ptr = dlist_next(ptr);
	}
	dlist_free_full(stack, Free);
	return target;
}

/* 向客户端返回请求的资源 */
static void echoToClient(int sockfd, const char *path)
{
	/* 这里不用包裹函数，因为要用打开是否成功来判断资源是否有效 */
	int fd = open(path, O_RDONLY);
	if (fd <= 0) {
		/* 指定的资源没有找到，返回404错误 */
		const char *res = response404();
		Write(sockfd, res, strlen(res));
		return;
	}

	const char *res = response200();
	if(Write(sockfd, res, strlen(res))<0){
		goto OUT;
	}

	ssize_t readn;
	char buf[1024];
	while ((readn = Read(fd, buf, 1024)) > 0) {
		if(Write(sockfd, buf, readn)<0){
			goto OUT;
		}
	}
OUT:
	Close(fd);
}

/*
 * @description 生成HTTP响应404错误
 */
static const char *response404()
{
	char *response = "HTTP/1.1 404 not found\r\n"
		"Server:WDLS/1.0.0(Unix)\r\n\r\n"
		"<html><body><h>404</h></body></html>";

	return response;
}

/*
 * @decription 生成HTTP响应200
 */
static char *response200()
{
	char *response = "HTTP/1.1 200 OK\r\n"
		"Content-type:*/*\r\n" "Server:WDLS/1.0.0(Unix)\r\n\r\n";
	return response;
}
