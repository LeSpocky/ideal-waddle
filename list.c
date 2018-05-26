/*******************************************************************//**
 *	@file		list.c
 *
 *	@brief		Implementation of a double linked list.
 *
 *	@author		Alexander Dahl <post@lespocky.de>
 *
 *	@copyright	2013,2018 Alexander Dahl
 *
 *	SPDX-License-Identifier: GPL-3.0-or-later
 *	License-Filename: LICENSE
 **********************************************************************/

#include "list.h"

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

struct list* list_create( void )
{
	return calloc( 1, sizeof(struct list) );
}

void list_destroy( struct list *list )
{
	assert( list );

	for ( struct node *node_ = list->first; node_; node_ = node_->next )
	{
		if ( node_->prev ) free( node_->prev );
	}

	free( list->last );
	free( list );
}

void list_push( struct list *list, int value )
{
	struct node *node_ = calloc( 1, sizeof(struct node) );

	/*	check	*/
	assert( node_ );
	if ( node_ == NULL ) return;

	node_->value = value;

	if ( list->last == NULL )
	{
		list->first = node_;
		list->last = node_;
	}
	else
	{
		list->last->next = node_;
		node_->prev = list->last;
		list->last = node_;
	}

	list->count++;

	return;
}

int list_pop( struct list *list )
{
	struct node *node_;

	assert( list );

	node_ = list->last;
	return (node_ != NULL) ? list_remove( list, node_ ) : 0;
}

int list_remove( struct list *list, struct node *node_ )
{
	int result;

	assert( list );
	assert( node_ );

	if ( list->first == NULL || list->last == NULL || node_ == NULL )
	{
		return 0;
	}

	if ( node_ == list->first && node_ == list->last )
	{
		list->first = NULL;
		list->last = NULL;
	}
	else if ( node_ == list->first )
	{
		list->first = node_->next;
		list->first->prev = NULL;
	}
	else if ( node_ == list->last )
	{
		list->last = node_->prev;
		list->last->next = NULL;
	}
	else
	{
		struct node *after = node_->next;
		struct node *before = node_->prev;
		after->prev = before;
		before->next = after;
	}

	list->count--;
	result = node_->value;
	free( node_ );

	return result;
}

/* vim: set noet sts=0 ts=4 sw=4 sr: */
