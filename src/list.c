/*
 * list.c
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
#include "list.h"
#include "Socket.h"

static inline Dlist *dlist_first(Dlist * list)
{
	if (list == NULL)
		return NULL;
	Dlist *ptr = list;
	while (dlist_prev(ptr)) {
		/* 取到首元素 */
		ptr = dlist_prev(ptr);
	}
	return ptr;
}

Dlist *dlist_new(void)
{
	return NULL;
}

Dlist *dlist_append(Dlist * list, void *data)
{
	Dlist *ptr = NULL;
	if (list == NULL) {
		ptr = (Dlist *) Malloc(sizeof(Dlist));
		ptr->data = data;
		ptr->prev = NULL;
		ptr->next = NULL;
		return ptr;
	}
	ptr = list;
	/* 取最后到一个元素 */
	while (dlist_next(ptr)) {
		ptr = dlist_next(ptr);
	}
	ptr->next = (Dlist *) Malloc(sizeof(Dlist));
	ptr->next->data = data;
	ptr->next->prev = ptr;
	ptr->next->next = NULL;

	return dlist_first(list);
}

void dlist_foreach(Dlist * list, ForeachFunc func, void *userData)
{
	if (list == NULL)
		return;
	Dlist *ptr = list;
	while (ptr) {
		func(ptr->data, userData);
		ptr = dlist_next(ptr);
	}
}

void dlist_free(Dlist * list)
{
	if (list == NULL)
		return;
	Dlist *ptr = dlist_first(list);
	Dlist *tmp = ptr;
	while (ptr) {
		ptr = dlist_next(ptr);
		Free(tmp);
	}
}

void dlist_free_full(Dlist * list, DestroyNotify destroy)
{
	if (list == NULL)
		return;
	Dlist *ptr = dlist_first(list);
	Dlist *tmp = NULL;
	while (ptr) {
		tmp = ptr;
		ptr = dlist_next(ptr);
		destroy(tmp->data);
		Free(tmp);
	}
}
