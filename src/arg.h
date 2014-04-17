/*
 * param.h
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
 * 解析命令行参数
 */

typedef enum {
	ARG_PORT = 'p',				/* 端口号 */
} Argument;


/*
 * @description 初始化解析命令行参数
 * @param ...
 */
void arg_init(int argc, char *argv[]);

/*
 * @description 获取端口号
 *				如果未设置使用默认
 * @return 端口号
 */
unsigned short arg_get_port(void);
