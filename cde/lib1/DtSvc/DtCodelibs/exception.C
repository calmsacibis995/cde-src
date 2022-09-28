/*
 * File:	exception.C $Revision: 1.3 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
// C++ exception handler package

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <osfcn.h>
#include <codelibs/boolean.h>
#define __PRIVATE_
#include <codelibs/privbuf.h>
#include <codelibs/exception.h>

_Exception *_curr_exception = NULL;


_Exception::_Exception()
{
	raised = accepted = FALSE;
	val = NULL;
	prev = _curr_exception;
	_curr_exception = this;
}

_Exception::~_Exception()
{
	_curr_exception = prev;
	if (raised && !accepted)
		_raise_exception(val);
}

int _Exception::accept(Exception v)
{
	_curr_exception = this;
	if (val != v)
		return FALSE;
	return accepted = TRUE;
}

int _Exception::recover()
{
	_curr_exception = this;
	return accepted = TRUE;
}

void _Exception::raise(Exception v)
{
	if (raised && accepted)
	{
	    _curr_exception = prev;
	    _raise_exception(v);
	    return;
	}
	raised = TRUE;
	val = v;
	longjmp(buf, 42);
}

static void dumpit(const char *err)
{
	write(2, err, strlen((char *)err));
}

void _raise_exception(Exception v)
{
	if (_curr_exception != NULL)
		_curr_exception->raise(v);
	if (v == NULL)
		dumpit("caught NULL exception\r\n");
	else
	{
		dumpit("caught exception: ");
		dumpit(v);
		dumpit("\r\n");
	}
	abort();
}
