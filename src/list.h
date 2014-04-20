/*
 * list.h
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
 * 链表的实现
 */

typedef void (*DestroyNotify) (void *data);
typedef void (*ForeachFunc) (void *data, void *userData);

typedef struct _Dlist Dlist;
/* 双向链表 */
struct _Dlist {
	void *data;
	Dlist *next;
	Dlist *prev;
};

#define dlist_next(list)	((list)->next)
#define dlist_prev(list)	((list)->prev)

/* NULL */
Dlist *dlist_new(void);


/* 链表中插入一个元素 */
Dlist *dlist_append(Dlist * list, void *data);

/* 遍历链表 */
void dlist_foreach(Dlist * list, ForeachFunc func, void *userData);

/* 释放链表，但不释放引用的元素 */
void dlist_free(Dlist * list);
/* 释放链表，并调用释放函数释放引用的元素 */
void dlist_free_full(Dlist * list, DestroyNotify destroy);
