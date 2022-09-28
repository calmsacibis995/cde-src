/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: tt_iostream.h /main/cde1_maint/2 1995/10/07 19:09:06 pascale $ 			 				 */
/* -*-C++-*-
 *
 * @(#)tt_iostream.h	1.3 93/08/15
 * 
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */
#ifndef TT_IOSTREAM_H
#define TT_IOSTREAM_H

#include <stdio.h>
#include "util/tt_string.h"
#include "api/c/tt_c.h"

class _Tt_message;
class _Tt_pattern;

class _Tt_ostream : public virtual _Tt_allocated {
    public:
				_Tt_ostream();
				_Tt_ostream(
					FILE	         *sink
				);
				_Tt_ostream(
					_Tt_string       &sink
				);
				~_Tt_ostream();

	_Tt_ostream	       &operator =(
					FILE	         *sink
				);
	_Tt_ostream	       &operator =(
					_Tt_string       &sink
				);

	const _Tt_ostream      &operator <<(
					const _Tt_message &msg
				) const;
	const _Tt_ostream      &operator <<(
					const _Tt_pattern &pat
				) const;
	const _Tt_ostream      &operator <<(
					const _Tt_string &s
				) const;
	const _Tt_ostream      &operator <<(
					Tt_status         status
				) const;
	const _Tt_ostream      &operator <<(
					Tt_callback_action status
				) const;
	const _Tt_ostream      &operator <<(
					void             *p
				) const;
	const _Tt_ostream      &operator <<(
					const char       *s
				) const;
	const _Tt_ostream      &operator <<(
					char              c
				) const;
	const _Tt_ostream      &operator <<(
					int               n
				) const;

	const _Tt_ostream      &operator <<(
					unsigned int      n
				) const;

	const _Tt_ostream      &operator <<(
					long              n
				) const;
	const _Tt_ostream      &operator <<(
					unsigned long     n
				) const;

	const _Tt_ostream      &print(
					_Tt_object_printfn print_it,
					const _Tt_object  *obj
				) const;
	int			sprintf(
					unsigned int      maxlen,
					const char        *format,
					...
				) const;

	void			set_indent(
					const _Tt_string &indent
				) const;

	FILE		       *theFILE() const {return _f;}
	const _Tt_string       &indent() const {return _indent;}

    private:
	FILE		       *_f;
	_Tt_string	       *_s;
	_Tt_string		_indent;

	// State field for indentation.  If we last printed
	// out a newline, and if _indent is not null, this
	// time around we'll print some indenting characters.
	int			_was_newline;
};

#endif
