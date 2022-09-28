/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993-1995  Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#pragma ident "@(#)thread.c	1.12 97/05/13 Sun Microsystems, Inc."

#ifndef NOTHREADS

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifdef PTHREAD
#include <pthread.h>
#else /* PTHREAD */
#include <thread.h>
#endif /* PTHREAD */
#include <sys/param.h>
#include <Xm/Xm.h>
#include <Dt/HourGlass.h>
#include "calendar.h"
#include "tempbr.h"
#include "misc.h"
#include "props_pu.h"
#include "editor.h"
#include "group_editor.h"
#include "todo.h"
#include "revision.h"

typedef struct {
	Calendar		*cal;
	int			 pfd[2];
	char			*cal_name;
	char			*cal_addr;
	CSA_session_handle	 new_cal_handle;
	unsigned int		 user_access;
	char			*cal_charset;
	CSA_return_code		 status;
	XtInputId		 input_id;
} Logon_Data;

static void *logon_thread(Logon_Data *);
static void post_login(Calendar	*, char	*, char *, CSA_session_handle);
static void logon_thread_exit(Logon_Data *, int, XtInputId *);
extern Boolean open_user_calendar(Calendar *, Boolean);
extern void update_handler(CSA_session_handle, CSA_flags, CSA_buffer, 
			   CSA_buffer, CSA_extension *);

#define CALLOG_FROM_BROWSE 0
extern void ufs_callog_check(Calendar *c, char *new_calendar, int init);


int cm_thread_create(
	void 	*(*start_routine)(),
	void	*arg)
{
#ifdef PTHREAD
	return(pthread_create(NULL, NULL, start_routine, (void *)arg));
#else
	return(thr_create(NULL, 0, start_routine, (void *)arg, 0, NULL));
#endif
	
}

void cm_thread_exit(
	void	*status)
{
#ifdef PTHREAD
	pthread_exit(status);
#else
	thr_exit(status);
#endif
}

#ifdef PTHREAD
int cm_mutex_init(
	pthread_mutex_t		*mp)
{
	return(pthread_mutex_init(mp, NULL));
}
#else
int cm_mutex_init(
	mutex_t		*mp)
{
	return(mutex_init(mp, USYNC_THREAD, NULL));
}
#endif

#ifdef PTHREAD
int cm_mutex_lock(
	pthread_mutex_t		*mp)
{
	return(pthread_mutex_lock(mp));
}
#else
int cm_mutex_lock(
	mutex_t		*mp)
{
	return(mutex_lock(mp));
}
#endif

#ifdef PTHREAD
int cm_mutex_unlock(
	pthread_mutex_t		*mp)
{
	return(pthread_mutex_unlock(mp));
}
#else
int cm_mutex_unlock(
	mutex_t		*mp)
{
	return(mutex_unlock(mp));
}
#endif

#ifdef PTHREAD
int cm_mutex_destroy(
	pthread_mutex_t		*mp)
{
	return(pthread_mutex_destroy(mp));
}
#else
int cm_mutex_destroy(
	mutex_t		*mp)
{
	return(mutex_destroy(mp));
}
#endif
	



void
switch_it2(
	Calendar 	*c, 
	char 		*new_calname, 
	char 		*new_caladdr, 
	WindowType 	 win)
{
	int			 ret,
				 pfd[2];
	char			 buf[MAXNAMELEN], *loc, *user;
	Logon_Data		*logon_data;
	Tempbr			*tb = (Tempbr*)c->tempbr;

	set_message(c->message_text, "\0");

        ufs_callog_check(calendar,new_caladdr,CALLOG_FROM_BROWSE);

        /*
	 * Check to see if we're already browsing the requested calendar ...
	 * If we have a valid calendar handle, we can return otherwise
	 * try logon again
	 */
        if (strcmp(new_caladdr, c->view->current_calendar) == 0 &&
	    c->cal_handle) {
		sprintf(buf, catgets(c->DT_catd, 1, 178,
			"You Are Already Browsing %s"), new_calname);
		set_message(c->message_text, buf);
		if (tb && tb->show_message)
			set_message(tb->show_message, buf);
                return;
        }

	/* 
	 * If the user selected the same calendar that he/she last requested
	 * then there is no reason to go on.  There should already be
	 * a thread outstanding that is handling this request.
	 */
	if (c->last_cal_requested) {
		if (strcmp(new_caladdr, c->last_cal_requested) == 0) {
			return;
		}
		free(c->last_cal_requested);
	}
	c->last_cal_requested = strdup(new_caladdr);

	invalidate_cache(c);

        /*
	 * Open the new calendar for browsing (or use my_cal_handle if we're
	 * switching to our calendar).
	 */
	if (strcmp(new_caladdr, c->caladdr) == 0) {
		if (c->my_cal_handle == 0) {
			if (open_user_calendar(c, False) == True)
				reset_timer(c);
			else
				return;
		}
		c->user_access = c->my_access;
		c->general->version = c->my_cal_version;
		c->set_charset = c->my_set_charset;

		post_login(c, new_calname, new_caladdr, c->my_cal_handle); 
	} else {
		if ((user = cm_target2name(new_caladdr)) == NULL) {
			if (!strcmp(new_caladdr, ""))
				sprintf(buf, catgets(c->DT_catd, 1, 619, "Please enter a calendar name in the format: <user>@<hostname>"));
			else
				sprintf(buf, catgets(c->DT_catd, 1, 620,
		"Unknown calendar. Calendar name needed: <name>%s"),
				new_calname);
#ifdef FNS
			if (cmfns_use_fns((Props *)c->properties))
				strcat(buf, catgets(c->DT_catd, 1, 618,
						    " or supply an FNS name"));
#endif /* FNS */
			set_message(c->message_text, buf);
			if (win == tempbrowser)
				set_message(tb->show_message, buf);
			return;
		}

		if ((loc = cm_target2location(new_caladdr)) == NULL) {
			if (!strcmp(new_caladdr, ""))
				sprintf(buf, catgets(c->DT_catd, 1, 619, "Please enter a calendar name in the format: <user>@<hostname>"));
			else
				sprintf(buf, catgets(c->DT_catd, 1, 622,
	"Unknown calendar. Hostname needed: %s@<hostname>"),
				user);
#ifdef FNS
			if (cmfns_use_fns((Props *)c->properties))
				strcat(buf, catgets(c->DT_catd, 1, 618,
						    " or supply an FNS name"));
#endif /* FNS */
			set_message(c->message_text, buf);
			if (win == tempbrowser)
				set_message(tb->show_message, buf);
			free(user);
			return;
		}
		free(loc);
		free(user);

		if (pipe(pfd) < 0) {
			sprintf(buf, catgets(c->DT_catd, 1, 1094,
                                "Login failed: internal error - bad pipe.")); 
                        set_message(c->message_text, buf);
                        return;
		}

		logon_data = (Logon_Data *)malloc(sizeof(Logon_Data));
		logon_data->pfd[0] = pfd[0];
		logon_data->pfd[1] = pfd[1];
		logon_data->cal_name = strdup(new_calname);
		logon_data->cal_addr = strdup(new_caladdr);
		logon_data->cal = c;

		ret = cm_thread_create(logon_thread, (void *)logon_data);

		sprintf(buf, "Attempting to browse %s...", new_calname);
		set_message(c->message_text, buf);

		logon_data->input_id = XtAppAddInput(c->xcontext->app, pfd[0], 
				     (XtPointer)XtInputReadMask, 
			             (XtInputCallbackProc)logon_thread_exit, 
				     (XtPointer)logon_data);

		return;
	}
}

static void *logon_thread(
	Logon_Data	*logon_data)
{
	CSA_calendar_user	csa_user;
	CSA_extension		logon_ext[2];

	csa_user.user_name = logon_data->cal_addr;
	csa_user.user_type = 0;
	csa_user.calendar_user_extensions = NULL;
	csa_user.calendar_address = logon_data->cal_addr;

	/* get user acess */
	logon_ext[0].item_code = CSA_X_DT_GET_USER_ACCESS_EXT;
	logon_ext[0].item_data = 0;
	logon_ext[0].item_reference = NULL;
	logon_ext[0].extension_flags = 0;

	/* get calendar's character set attribute */
	logon_ext[1].item_code = CSA_X_DT_GET_CAL_CHARSET_EXT;
	logon_ext[1].item_data = 0;
	logon_ext[1].item_reference = NULL;
	logon_ext[1].extension_flags = CSA_EXT_LAST_ELEMENT;

	logon_data->status = csa_logon(NULL, &csa_user, NULL, NULL, NULL,
			     	       &logon_data->new_cal_handle, logon_ext);

	if (logon_data->status == CSA_SUCCESS) {
		logon_data->user_access = logon_ext[0].item_data;
		logon_data->cal_charset = logon_ext[1].item_reference;
	}

	/* signal the main thread that logon is complete. */
	write(logon_data->pfd[1], "1", 1);
	cm_thread_exit((void *)0);
	return(0);
}

static void logon_thread_exit(
	Logon_Data	*logon_data,
	int		 source,
	XtInputId	*id)
{
	Tempbr		*tb = (Tempbr*)logon_data->cal->tempbr;
	char		 buf[4];
	CSA_flags	flags;
	CSA_extension	cb_ext;

	/* Read the threads signal byte from the pipe. */
	read(logon_data->pfd[0], buf, 1);
	XtRemoveInput(logon_data->input_id);
	close(logon_data->pfd[0]);
	close(logon_data->pfd[1]);

	/* If the user has attempted to browse another calendar since this
	 * calendar was requested then we ignore this login.
	 */
	if (logon_data->cal->last_cal_requested) {
		if (!(strcmp(logon_data->cal->last_cal_requested, 
			     logon_data->cal_addr) == 0)) {
			/* 
			 * We are logged into a calendar we are no longer
			 * interested in.  Logoff.
			 */
			if (logon_data->status == CSA_SUCCESS)
				csa_logoff(logon_data->new_cal_handle, NULL);
			if (logon_data->cal_name)
				free(logon_data->cal_name);
			if (logon_data->cal_addr)
				free(logon_data->cal_addr);
			free (logon_data);
			/*
			 * We don't free last_cal_requested because we have
			 * to assume that there is another thread that hasn't
			 * exitted yet that may need to use its value.
			 */
			return;
		} else {
			free(logon_data->cal->last_cal_requested);
			logon_data->cal->last_cal_requested = NULL;
		}
	} else {
		/* We are already viewing the calendar the user is interested
		 * in.  The thread that just exitted was an old calendar
		 * that we tried to browser earlier but decided that we
		 * wanted to view a different calendar subsequently.
		 */
		if (logon_data->status == CSA_SUCCESS)
			csa_logoff(logon_data->new_cal_handle, NULL);
		if (logon_data->cal_name)
			free(logon_data->cal_name);
		if (logon_data->cal_addr)
			free(logon_data->cal_addr);
		free (logon_data);
		return;
	}

	set_message(logon_data->cal->message_text, "\0");

	if (logon_data->status != CSA_SUCCESS) {
		backend_err_msg(logon_data->cal->frame, logon_data->cal_name, 
				logon_data->status,
				((Props_pu *)logon_data->cal->properties_pu)->\
				xm_error_pixmap);
		if (tb && tb->show_message)
			set_message(tb->show_message, "\0");
		return;
	}

	logon_data->cal->user_access = logon_data->user_access;
	logon_data->cal->general->version = get_data_version(
						logon_data->new_cal_handle);
	mark_set_entry_charset(logon_data->cal->app_locale,
			       logon_data->cal_charset,
			       logon_data->cal->general->version,
			       &logon_data->cal->set_charset);
	csa_free(logon_data->cal_charset);

	flags = CSA_CB_ENTRY_ADDED | CSA_CB_ENTRY_DELETED |
		CSA_CB_ENTRY_UPDATED;
	cb_ext.item_code = CSA_X_XT_APP_CONTEXT_EXT;
	cb_ext.item_data = (CSA_uint32)logon_data->cal->xcontext->app;
	cb_ext.extension_flags = CSA_EXT_LAST_ELEMENT;
	csa_register_callback(logon_data->new_cal_handle, flags,
			update_handler, NULL, &cb_ext);
	post_login(logon_data->cal, logon_data->cal_name, logon_data->cal_addr,
		   logon_data->new_cal_handle); 
	free(logon_data->cal_addr);
	free(logon_data->cal_name);
	free(logon_data);
}

static void post_login(
	Calendar		*c,
	char			*new_calname,
	char			*new_caladdr,
	CSA_session_handle	 new_cal_handle)
{
	Tempbr			*tb = (Tempbr*)c->tempbr;
	char			 buf[MAXNAMELEN];

        set_message(((Editor *)c->editor)->message_text, " ");      
        set_message(((GEditor *)c->geditor)->message_text, " ");
        set_message(((ToDo *)c->todo)->message_text, " ");

	/*
	 * Close the calendar we were currently browsing (note we don't ever
	 * close our calendar) and set the callback for the new calendar.
	 */
	if (c->cal_handle != c->my_cal_handle)
		csa_logoff(c->cal_handle, NULL);

	/*
	 * Set variables in the Calendar structure for the new calendar and
	 * repaint.
	 */
	c->cal_handle = new_cal_handle;
        if (c->view->current_calendar != NULL)
                free(c->view->current_calendar);
        if (c->view->current_calname != NULL)
                free(c->view->current_calname);
        c->view->current_calname = cm_strdup(new_calname);
        c->view->current_calendar = cm_strdup(new_caladdr);
	_DtTurnOnHourGlass(c->frame);
        paint_canvas(c, NULL, RENDER_CLEAR_FIRST);
 
	/*
	 * Set the title bars on all the windows!
	 */
	sprintf(buf, "%s : %s", catgets(c->DT_catd, 1, 113, "Calendar"),
				new_calname);
	XtVaSetValues(c->frame, XmNtitle, buf, NULL);
 
	/* Bug 1247373: Reset appt editor when changing calendars. */
	if (editor_showing((Editor *)c->editor))
		set_editor_defaults((Editor *)c->editor, 0, 0, 0);
	if (geditor_showing((GEditor *)c->geditor))
		set_geditor_defaults((GEditor *)c->geditor, 0, 0);
	if (todo_showing((ToDo *)c->todo))
		set_todo_defaults((ToDo *)c->todo);

	set_editor_title((Editor *)c->editor, new_calname);
	add_all_appt((Editor *)c->editor, NULL);
	set_geditor_title((GEditor *)c->geditor, new_calname);
	add_all_gappt((GEditor *)c->geditor);
	set_todo_title((ToDo *)c->todo, new_calname);
	add_all_todo((ToDo *)c->todo, NULL);

	/* set up the repeat menus for all the editors. */

	if (c->editor && (((Editor *)c->editor)->rfp.repeat_menu))
		rfp_set_repeat_values(&((Editor *)c->editor)->rfp);

	if (c->geditor && (((GEditor *)c->geditor)->rfp.repeat_menu))
		rfp_set_repeat_values(&((GEditor *)c->geditor)->rfp);

	if (c->todo && (((ToDo *)c->todo)->rfp.repeat_menu))
		rfp_set_repeat_values(&((ToDo *)c->todo)->rfp);

	if (c->general->version < DATAVER2) {
		if (todo_showing((ToDo *)calendar->todo))
			XtPopdown(((ToDo *)calendar->todo)->frame);
		XtUnmanageChild(c->todo_btn);
		XtSetSensitive(c->todo_edit_menu, False);
		XtSetSensitive(c->todo_view_menu, False);
	}
	else {
		XtManageChild(c->todo_btn);
		XtSetSensitive(c->todo_edit_menu, True);
		XtSetSensitive(c->todo_view_menu, True);
	}

	if (tb && tb->show_message) {
		char	buf[128];

		sprintf (buf, "%s %s %s",
			catgets(c->DT_catd, 1, 919, "Calendar"),
			new_calname,
			catgets(c->DT_catd, 1, 920, "displayed."));
		set_message(tb->show_message, buf);
	}
	_DtTurnOffHourGlass(c->frame);
}

#else /* NOTHREADS */

#include <stdio.h>
	/* Do Nothing */

#endif /* NOTHREADS */
