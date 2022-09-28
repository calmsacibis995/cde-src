/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * mp_desktop.h 1.6 30 Jul 1993
 *
 * Tool Talk Message Passer (MP) - mp_desktop.h
 *
 * Copyright (c) 1990,1992 by Sun Microsystems, Inc.
 */
#ifndef MP_DESKTOP_H
#define MP_DESKTOP_H
#include <setjmp.h>
#include <mp/mp_global.h>
#include <util/tt_object.h>

#if defined(ultrix)
extern "C"
{ extern struct XSizeHints;
  extern struct XStandardColormap;
  extern struct XTextProperty;
  extern struct XWMHints;
  extern struct XClassHint;
};
#endif


enum _Tt_dt_type {
	_TT_DESKTOP_X11,
	_TT_DESKTOP_LAST
};

typedef int (*_Tt_dt_errfn)(void *);

struct _Tt_desktop_private;

class _Tt_desktop : public _Tt_object {
      public:
	_Tt_desktop();
	virtual ~_Tt_desktop();
	int			init(_Tt_string dt_handle, _Tt_dt_type t);
	int			notify_fd();
	int			process_event();
	int			lock();
	int			unlock();
	int			set_prop(_Tt_string pname, _Tt_string &val);
	int			del_prop(_Tt_string pname);
	int			get_prop(_Tt_string pname, _Tt_string &val);
	_Tt_string		session_name(_Tt_string dt_handle);
	void			set_error_handler(_Tt_dt_errfn efn);
      private:
	int			close();
	void			restore_user_handler();
	static int		io_error_proc(void *);
	int			*user_io_handler;
	static jmp_buf		io_exception;
	_Tt_desktop_private	*priv;
};

#include <mp/mp_desktop_utils.h>

class _Tt_desktop_lock : public _Tt_object {
      public:
	_Tt_desktop_lock();
	_Tt_desktop_lock( const _Tt_desktop_ptr &dt );
	virtual ~_Tt_desktop_lock();
      private:
	_Tt_desktop_ptr		_dt;
};

#endif				/* MP_DESKTOP_H */
