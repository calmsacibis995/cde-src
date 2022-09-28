/*******************************************************************************
**
**  blist.h
**
**  #pragma ident "@(#)blist.h	1.24 96/11/12 Sun Microsystems, Inc."
**
**  $XConsortium: blist.h /main/cde1_maint/1 1995/07/14 23:03:23 drk $
**
**  RESTRICTED CONFIDENTIAL INFORMATION:
**
**  The information in this document is subject to special
**  restrictions in a confidential disclosure agreement between
**  HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
**  document outside HP, IBM, Sun, USL, SCO, or Univel without
**  Sun's specific written approval.  This document and all copies
**  and derivative works thereof must be returned or destroyed at
**  Sun's request.
**
**  Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
**
*******************************************************************************/

/*									*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc. 				*
 */

#ifndef _BLIST_H
#define _BLIST_H

#include "ansi_c.h"
#include "util.h"
#ifndef NOTHREADS
#ifdef PTHREAD
#include <pthread.h>
#else /* PTHREAD */
#include <thread.h>
#endif /* PTHREAD */
#include <Xm/Xm.h>
#endif /* NOTHREADS */

/*
 * The list of names we can typically browse is used in three spots - the
 * browse list editor, the multi browser (compare calendars), and the main
 * window to build the browse menu.
 *
 * The list of names actually is a list of the BlistData structures.  The
 * multi browser uses the cal_handle item and therefore, we can't delete the
 * entry from this list until the multi browser is no longer using that pointer.
 */
typedef enum {
	BLIST_INSERT, BLIST_DELETE, BLIST_ACTIVE
} BlistTag;

typedef struct {
	char			*name;
	BlistTag		tag;
	CSA_session_handle	cal_handle;
	CSA_flags		user_access;
	int			version;
	Boolean			set_charset;
	Paint_cache		*cache;
	int			cache_size;
#ifndef NOTHREADS
	XtInputId		bd_id;
	Boolean			bd_live;	/* Is the thread alive? */
	int			bd_in_pfd;
	Boolean			bd_selected;
	CSA_return_code		bd_stat;
#endif /* NOTHREADS */
} BlistData;

typedef struct {
	Widget		button_rc_mgr;
	Widget		edit_rc_mgr;
	Widget		frame;
	Widget		form;
	Widget		username;
	Widget		username_label;
	Widget		add_button;
	Widget		list_label;
	Widget		browse_list;
	Widget		browse_list_sw;
	Widget		remove_button;
	Widget		ok_button;
	Widget		apply_button;
	Widget		reset_button;
	Widget		cancel_button;
	Widget		help_button;
	Widget		message;
	CmDataList	*blist_data;
	Boolean		bl_pending_message_up;
	Boolean		bl_list_changed;
#ifndef NOTHREADS
	Boolean		bl_turn_ok_on; 	  /* Does the ME's ok/apply/reset 
					   * buttons need to be turned on
					   * after the multi browse op is done?
					   */
	Boolean		bl_turn_add_on;	  /* Does the Add buton in the ME need
					   * to be turned back on?
					   */
	Boolean		bl_turn_remove_on;/* Does the Remove buton in the ME 
					   * need to be turned back on?
					   */
	Boolean		bl_suspended; 	  /* Should the ME be suspended? */
#ifdef PTHREAD
	pthread_mutex_t	bl_data_op_lock;
#else /* PTHREAD */
	mutex_t		bl_data_op_lock;
#endif /* PTHREAD */
	XtIntervalId	bl_timer;	  /* Timer used to update footer */
#endif /* NOTHREADS */
} Browselist;

typedef struct browser_state {
	char			*cname;
	Pixmap			*glyph;
	struct browser_state	*next;
} BrowserState;

#ifndef NOTHREADS
typedef struct browselist_thread_data {
	BlistData	*btd_blist_item;
	int		 btd_pfd[2];
	Browselist	*btd_blist;
	Calendar	*btd_calendar;
} BrowselistThreadData;
#endif /* NOTHREADS */

extern void	blist_clean	(Browselist*, Boolean);
extern void	blist_init_names(Calendar*);
extern void	blist_init_ui	(Calendar*);
extern void	blist_reset	(Calendar*);
extern void	make_browselist	(Calendar*);
extern void	show_browselist	(Calendar*);

#endif
