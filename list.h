/*******************************************************************//**
 *	@file		list.h
 *
 *	@brief		Declarations for a double linked list containing file
 *				descriptors (int).
 *
 *	@author		Alexander Dahl <post@lespocky.de>
 *
 *	@copyright	2013,2018 Alexander Dahl
 *
 *	SPDX-License-Identifier: GPL-3.0-or-later
 *	License-Filename: LICENSE
 **********************************************************************/

#ifndef LIST_H_
#define LIST_H_

#define list_count(A)	((A)->count)
#define list_first(A)	((A)->first != NULL ? (A)->first->value : 0)
#define list_last(A)	((A)->last != NULL ? (A)->last->value : 0)

struct node {
	struct node	*prev;
	struct node	*next;
	int			value;
};

struct list {
	int			count;
	struct node	*first;
	struct node	*last;
};

struct list* list_create( void );

/**
 *	Free list elements itself and free list then.
 *
 *	@param[in]	list	pointer to list
 */
void list_destroy( struct list *list );

void list_push( struct list *list, int value );
int list_pop( struct list *list );

int list_remove( struct list *list, struct node *node );

#endif /* LIST_H_ */

/* vim: set noet sts=0 ts=4 sw=4 sr: */
