//%%  (c) Copyright 1995 Sun Microsystems, Inc.			

#ifndef _TT_THREADSPECIFIC_H
#define _TT_THREADSPECIFIC_H

#include <tt_options.h>

#ifdef OPT_XTHREADS

#include "mp/mp_c_session.h"
#include "mp/mp_c_procid_utils.h"
#include "api/c/api_storage.h"

// This class holds the thread-specific info we need.  As more internal
// TT data is made thread-specific, it should  be put into this class.

class _Tt_threadspecific : public _Tt_object {
    public:
	_Tt_threadspecific() { _thread_stgstack = 0; };
	_Tt_threadspecific(const _Tt_c_session_ptr& sess,
			   const _Tt_c_procid_ptr& proc);
	~_Tt_threadspecific() { delete _thread_stgstack; };

	_Tt_c_session_ptr&	thread_c_session() {
		return _thread_c_session;
	};

	void	set_thread_c_session(_Tt_c_session_ptr& sess) {
		_thread_c_session = sess;
	};

	_Tt_c_procid_ptr&	thread_c_procid() {
		return _thread_c_procid;
	};

	void	set_thread_c_procid(_Tt_c_procid_ptr& proc) {
		_thread_c_procid = proc;
	};

	_Tt_api_stg_stack*	thread_stgstack();

	// Note that there is no "set" method for the stgstack -- if
	// none exists, it will be created upon the first call to
	// thread_stgstack()

	void		free_procid();

    private:
	_Tt_c_session_ptr	_thread_c_session;
	_Tt_c_procid_ptr	_thread_c_procid;
	_Tt_api_stg_stack*	_thread_stgstack;
};

#endif	// OPT_XTHREADS
#endif	// _TT_THREADSPECIFIC_H

