/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993, 1994, 1995 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtVirtArray.C	1.7 18 Jan 1994"
#endif

#include <stdlib.h>

#include <DtMail/DtVirtArray.hh>
#include <DtMail/Threads.hh>

#include <DtMail/IO.hh>

DtVirtArrayImpl::DtVirtArrayImpl(const int size)
{
    _size = size;
    _count = 0;
    _elements = (void **)malloc(sizeof(void *) * _size);

    _mutex = MutexInit();
}

DtVirtArrayImpl::~DtVirtArrayImpl(void)
{
    if (_elements) {
	MutexLock lock_scope(_mutex);
	free(_elements);
	_elements = NULL;
	_size = 0;
	_count = 0;
	lock_scope.unlock_and_destroy();
    }
}

int
DtVirtArrayImpl::length(void)
{
    MutexLock lock_scope(_mutex);

    return(_count);
}

void *
DtVirtArrayImpl::operator[](const int at)
{
    MutexLock lock_scope(_mutex);
    if (at < 0 || at >= _count) {
	return(NULL);
    }

    return(_elements[at]);
}

int
DtVirtArrayImpl::index(void * handle)
{
    MutexLock lock_scope(_mutex);

    for (int i = 0; i < _count; i++) {
	if (_elements[i] == handle) {
	    return(i);
	}
    }

    return(-1);
}

int
DtVirtArrayImpl::append(void * handle)
{
    MutexLock lock_scope(_mutex);

    grow();
    int new_pos = _count;
    _elements[_count] = handle;

    _count += 1;

    return(new_pos);
}

void
DtVirtArrayImpl::insert(void * handle, const int at)
{
    MutexLock lock_scope(_mutex);

    make_slot(at);
    _elements[at] = handle;
}

void
DtVirtArrayImpl::remove(const int at)
{
    MutexLock lock_scope(_mutex);

    for (int pos = at; pos < (_count - 1); pos++) {
	_elements[pos] = _elements[pos + 1];
    }

    _count -= 1;
}

void
DtVirtArrayImpl::remove(void * handle)
{
    MutexLock lock_scope(_mutex);
 
    int pos = index(handle);
    remove(pos);
}

void
DtVirtArrayImpl::make_slot(const int at)
{
    grow();
    _count += 1;
    for (int pos = (_count - 1); pos > at; pos--) {
	_elements[pos] = _elements[pos - 1];
    }
    _elements[at] = NULL;
}

void
DtVirtArrayImpl::grow(void)
{
    if (_count == _size) {
	// Grow by 50%, but grow by at least 1!
	//
	int new_size = _size + (_size >> 2) + 1;
	_elements = (void **)realloc(_elements, new_size * sizeof(void *));
	_size = new_size;
    }
}
