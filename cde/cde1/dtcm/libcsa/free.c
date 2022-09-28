/* $XConsortium: free.c /main/cde1_maint/1 1995/07/17 19:57:28 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)free.c	1.11 97/01/31 Sun Microsystems, Inc."

#include <stdlib.h>
#include "csa.h"
#include "free.h"
#include <pthread.h>

#define _DtCM_OUR_MEMORY	0x55555555

typedef void (*_DtCmFreeProc)(uint num_elem, void *ptr);

typedef struct {
	int		mark;
	uint		num_elem;
	_DtCmFreeProc	fproc;
} _DtCmMemHeader;

/******************************************************************************
 * forward declaration of static functions used within the file
 ******************************************************************************/
static void _DtCm_free_calendar_users(uint num_elem, void *vptr);

/*****************************************************************************
 * extern functions used in the library
 *****************************************************************************/

extern CSA_calendar_user *
_DtCm_alloc_calendar_users(uint num_elem)
{
	char *ptr;
	_DtCmMemHeader	*hptr;

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) +
	    sizeof(CSA_calendar_user) * num_elem)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = num_elem;
		hptr->fproc = _DtCm_free_calendar_users;
		return ((CSA_calendar_user *)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern char **
_DtCm_alloc_character_pointers(uint num_elem)
{
	char *ptr;
	_DtCmMemHeader	*hptr;

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) +
	    sizeof(char *) * num_elem)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = num_elem;
		hptr->fproc = _DtCm_free_character_pointers;
		return ((char **)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern char *
_DtCm_alloc_character_string(uint size)
{
	char *ptr;
	_DtCmMemHeader	*hptr;

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) + size)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = 0;
		hptr->fproc = NULL;
		return ((char *)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern CSA_attribute *
_DtCm_alloc_attributes(uint num_elem)
{
	char *ptr;
	_DtCmMemHeader	*hptr;
	extern void _DtCm_free_attributes(uint size, void *attrs);

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) +
	    sizeof(CSA_attribute) * num_elem)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = num_elem;
		hptr->fproc = _DtCm_free_attributes;
		return ((CSA_attribute *)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern CSA_entry_handle *
_DtCm_alloc_entry_handles(uint num_elem)
{
	char *ptr;
	_DtCmMemHeader	*hptr;
	extern void _DtCm_free_entry_handles(uint num_entries, void *entries);

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) +
	    sizeof(CSA_entry_handle) * num_elem)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = num_elem;
		hptr->fproc = _DtCm_free_entry_handles;
		return ((CSA_entry_handle *)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern void *
_DtCm_alloc_entry(uint size)
{
	char *ptr;
	_DtCmMemHeader	*hptr;
	extern void _DtCm_free_entry_content(uint dummy, void *entry);

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) + size)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = 1;
		hptr->fproc = _DtCm_free_entry_content;
		return ((void *)(ptr + sizeof(_DtCmMemHeader)));
	}
}

extern CSA_reminder_reference *
_DtCm_alloc_reminder_references(uint num_elem)
{
	char *ptr;
	_DtCmMemHeader	*hptr;
	extern void _DtCm_free_reminder_references(uint num_rems, void *rems);

	if ((ptr = calloc(1, sizeof(_DtCmMemHeader) +
	    sizeof(CSA_reminder_reference) * num_elem)) == NULL)
		return (NULL);
	else {
		hptr = (_DtCmMemHeader *)ptr;
		hptr->mark = _DtCM_OUR_MEMORY;
		hptr->num_elem = num_elem;
		hptr->fproc = _DtCm_free_reminder_references;
		return ((CSA_reminder_reference *)(ptr +
			sizeof(_DtCmMemHeader)));
	}
}

extern CSA_return_code
_DtCm_free(void *ptr)
{
	_DtCmMemHeader	*hptr;
	CSA_return_code	stat = CSA_SUCCESS;

	/* synchronize access to ptr */
	static pthread_mutex_t _free_lock;

	if (ptr == NULL)
		return (CSA_E_INVALID_MEMORY);

	pthread_mutex_lock(&_free_lock);

	hptr = (_DtCmMemHeader *)(((char *)ptr) - sizeof(_DtCmMemHeader));

	if (hptr->mark == _DtCM_OUR_MEMORY)
		hptr->mark = 0;
	else
		stat = CSA_E_INVALID_MEMORY;

	pthread_mutex_unlock(&_free_lock);

	if (stat == CSA_SUCCESS) {
		if (hptr->fproc)
			hptr->fproc(hptr->num_elem, ptr);

		free(hptr);
	}
	return (stat);
}

extern void
_DtCm_free_character_pointers(uint num_elem, void *vptr)
{
	char **ptr = (char **)vptr;
	int	i;

	for (i = 0; i < num_elem; i++) {
		if (ptr[i])
			free(ptr[i]);
		else
			break;
	}
}

/******************************************************************************
 * static functions used within in the file
 ******************************************************************************/

static void
_DtCm_free_calendar_users(uint num_elem, void *vptr)
{
	CSA_calendar_user *ptr = (CSA_calendar_user *)vptr;
	int	i;

	for (i = 0; i < num_elem; i++) {
		if (ptr[i].user_name)
			free(ptr[i].user_name);

		if (ptr[i].calendar_address)
			free(ptr[i].calendar_address);
	}
}

