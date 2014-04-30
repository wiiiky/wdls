/*
 * param.c
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
#include "arg.h"
#include <getopt.h>
#include <sys/stat.h>
#include "Socket.h"

#define SERVERPORT  (6323)
#define ROOTPATH	"./root"

typedef enum {
	ARG_PORT = 'p',				/* 端口号 */
	ARG_ROOT = 'r',				/* 根目录 */
} Argument;

static unsigned short port = SERVERPORT;
static char *root = ROOTPATH;


/* 显示帮助 */
static inline void help(void);

void arg_init(int argc, char *argv[])
{
	const char *short_options = "p:r:";
	const struct option long_options[] = {
		{"port", 1, NULL, ARG_PORT},
		{"root", 1, NULL, ARG_ROOT},
		{NULL, 0, NULL, 0},
	};

	int c;
	struct stat sbuf;
	char *ptr;
	while ((c = getopt_long(argc, argv,
							short_options, long_options, NULL)) != -1) {
		switch (c) {
		case ARG_PORT:
			port = (unsigned short) atoi(optarg);
			break;
		case ARG_ROOT:
			if (stat(optarg, &sbuf)) {
				perror("fail to get file status");
				help();
			}
			if (!S_ISDIR(sbuf.st_mode)) {
				fprintf(stderr, "%s is invalid path\n");
				help();
			}
			root = Strdup(optarg);
			break;
		default:
			help();
			break;
		}
	}
}

unsigned short arg_get_port(void)
{
	if (port)
		return port;
	return SERVERPORT;
}

const char *arg_get_root(void)
{
	return root;
}


static inline void help(void)
{
	printf("Usage: wdls [option]\n");
	printf("\t-p\t--port=\tHTTP port\n");
	printf("\t-r\t--root=\tHTTP root path\n");
	exit(-1);
}
