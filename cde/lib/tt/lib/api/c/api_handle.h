/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $XConsortium: api_handle.h /main/cde1_maint/1 1995/07/14 16:49:10 drk CHECKEDOUT $ 			 				 */
/*
 * api_handle.h
 *
 * Classes for managing mapping from API handles to internal structures,
 * and maintaining user data.
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */
#ifndef _API_HANDLE_H
#define _API_HANDLE_H

#include "util/tt_list.h"
#include "mp/mp_pattern_utils.h"
#include "mp/mp_c_message_utils.h"
#include "mp/mp_c_message.h"
#include "mp/mp_pattern.h"
#ifndef FILE
#include <stdio.h>
#endif

declare_list_of(_Tt_api_handle)
declare_list_of(_Tt_api_userdata)
declare_list_of(_Tt_api_callback)

class _Tt_api_handle : public _Tt_object {
      public:
	_Tt_api_handle();
	virtual ~_Tt_api_handle();
	void		ptr_set(_Tt_c_message_ptr p);
	void		ptr_set(_Tt_pattern_ptr p);
	_Tt_c_message_ptr mptr();
	_Tt_pattern_ptr pptr();
	void		store(int key, void *userdata);
	void *		fetch(int key);
	void		add_callback(Tt_message_callback f);
	Tt_callback_action run_callbacks(Tt_message mh, Tt_pattern ph);
	void		print(FILE *fs = stdout) const;
      private:
	// Only one of the following 2 pointers will be non-null,
        // as this is either a message handle or a pattern handle.
        // If we wanted to be real object-oriented weenies we would
        // declare two subclasses of _TT_api_handle, one for patterns
	// and one for messages.  But we're not.
	_Tt_c_message_ptr _mptr;
	_Tt_pattern_ptr _pptr;

	_Tt_api_userdata_list_ptr udlp;
	_Tt_api_callback_list_ptr cblp;
};

class _Tt_api_userdata : public _Tt_object {
    friend class _Tt_api_handle;
    friend class _Tt_typecb;
      public:
        _Tt_api_userdata();
        virtual ~_Tt_api_userdata();
	void		print(FILE *fs = stdout) const;
      private:
	int		key;
	void *		userdata;
};

class _Tt_api_callback : public _Tt_object {
    friend class _Tt_api_handle;
    friend class _Tt_typecb;
      public:
        _Tt_api_callback();
        virtual	~_Tt_api_callback();
	void		print(FILE *fs = stdout) const;
      private:
	Tt_message_callback	callback;
};

class _Tt_api_handle_table : public _Tt_object {
      public:
	_Tt_api_handle_table();
	virtual	~_Tt_api_handle_table();
	_Tt_c_message_ptr 	lookup_msg(Tt_message h);
	_Tt_pattern_ptr 	lookup_pat(Tt_pattern h);
	_Tt_pattern_ptr 	lookup_pat_by_id(_Tt_string pattern_id,
						 Tt_pattern &pat);
	Tt_message		lookup_mhandle(_Tt_c_message_ptr m);
	Tt_pattern		lookup_phandle(_Tt_pattern_ptr p);
	void			clear(_Tt_c_message_ptr m);
	void			clear(_Tt_pattern_ptr p);
	void			clear(Tt_message p);
	void			clear(Tt_pattern p);
	Tt_status		store(Tt_message h, int key,
				      void * userdata);
	Tt_status		store(Tt_pattern h, int key,
				      void * userdata);
	void		       *fetch(Tt_message h, int key); 
	void		       *fetch(Tt_pattern h, int key);
	Tt_status		add_callback(Tt_message h,
					     Tt_message_callback f);
	Tt_status		add_callback(Tt_pattern h,
					     Tt_message_callback f);
	Tt_callback_action	run_message_callbacks(Tt_message h,
						      Tt_pattern ph);
	Tt_callback_action	run_pattern_callbacks(Tt_pattern h,
						      Tt_message mh);
	void			print(FILE *fs = stdout) const;
      private:
	_Tt_api_handle_list_ptr	content;
	_Tt_api_handle_ptr	last_mhandle;
	_Tt_string		last_pat_id;
	_Tt_pattern_ptr		last_pattern;
	Tt_pattern		last_phandle;
};
extern _Tt_api_handle_table *_tt_htab;
#endif 

