/* $XConsortium: dnd.c /main/cde1_maint/5 1995/11/14 16:57:12 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#pragma ident "@(#)dnd.c	1.62 96/11/12 Sun Microsystems, Inc."

#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <stdlib.h>
#include <rpc/rpc.h>
#include <X11/Xlib.h>

#include <Xm/Xm.h>
#include <Xm/Text.h>
#include <Xm/List.h>
#include <Xm/DragC.h>
#include <Xm/DragIcon.h>
#include <Xm/AtomMgr.h>
#include <Xm/ToggleBG.h>
#include <Dt/Dt.h>
#include <Dt/Dnd.h>
#include <csa.h>

#include "util.h"
#include "editor.h"
#include "todo.h"
#include "group_editor.h"
#include "calendar.h"
#include "props_pu.h"
#include "props.h"
#include "dnd.h"
#include "getdate.h"
#include "cm_tty.h"
#include "misc.h"
#include "help.h"

#ifdef FNS
#include "cmfns.h"
#endif

#include "drag_xbm"
#include "drag_mask_xbm"

extern int	drag_load_proc(char*, Calendar *);
extern char	*sys_errlist[];
static char	dnd_filename[20];
static char	dnd_fnsname[MAXNAMELEN];
static char	dnd_fnsaddr[MAXNAMELEN];

static Boolean 
validate_dropped_appt(char *filename, Calendar *c) {
	Props			*p = (Props *)c->properties;
	Props_pu		*pu = (Props_pu *)c->properties_pu;
	CmDataList		*list = CmDataListCreate();
	Validate_op		op;
	int			i;
	Dtcm_appointment	*a;

	if (!filename || *filename == '\0' || !c->cal_handle)
		return(False);

	op = parse_appt_from_file(c->DT_catd, filename, list, p, query_user, 
				  (void *)c, c->general->version);

	for (i = 1; i <= list->count; i++)
		if (a = (Dtcm_appointment *)CmDataListGetData(list, i))
			free_appt_struct(&a);
	CmDataListDestroy(list, _B_FALSE);

	if (op == VALID_APPT)
		return(True);
	else
		return(False);

}

#ifdef FNS
/* 
   If FNS is supported, read contents of file and assume that it
   contains an FNS name.  Lookup this FNS name to get a calendar address.

   Returns True got calendar address successfully; False otherwise.
 */
static Boolean
validate_fns_name(unsigned int read_from_file, char *filename, Calendar *c, 
		  char *name_buf, int nsize,
		  char *addr_buf, int asize)
{
	char *target = NULL;
	if (!filename || *filename == '\0' || !c->cal_handle ||
	    cmfns_use_fns((Props *)c->properties) == 0)
		return(False);

	if (read_from_file) {
		if (cmfns_read_name_from_file(filename, name_buf, nsize))
			target = name_buf;
	} else {
		target = filename;
		strcpy(name_buf, target);
	}
	if (target != NULL &&
	    cmfns_lookup_calendar(target, addr_buf, asize) > 0) {
		return (True);
	}
	return (False);
}
#endif /* FNS */

static void
handle_animate_cb(
	Widget		dragContext,
	XtPointer	client_data,
	XtPointer	call_data)
{
	DtDndTransferCallbackStruct *animateInfo = (DtDndTransferCallbackStruct *)call_data;
	Calendar	*c;
	char		*data;
	int		size;
	FILE		*fp;
	int		i;
#if defined(FNS) && defined(FNS_DEMO)
	char		buf[MAXNAMELEN];
	char		addr_buf[MAXNAMELEN];
#endif /* FNS */

	c = (Calendar *)client_data;

	animateInfo->status = DtDND_SUCCESS;

	for (i = 0; i < animateInfo->dropData->numItems; i++) {
		switch(animateInfo->dropData->protocol) {
		case DtDND_FILENAME_TRANSFER:
			data = animateInfo->dropData->data.files[i];

#if defined(FNS) && defined(FNS_DEMO)
			if (cmfns_use_fns(c->properties) &&
			    cmfns_name_from_file(data, buf, sizeof(buf)) == 1) {
				/* 
				 * Looks like an HFS file has been dropped on us.
				 * Get the calendar service associated with the
				 * FNS name and browse it
				 */
				if (cmfns_lookup_calendar(buf,
						addr_buf, sizeof(addr_buf)) > 0) {
					
#ifndef NOTHREADS
					switch_it2(c, buf, addr_buf, main_win);
#else
					switch_it(c, buf, addr_buf, main_win);
#endif
					return;
				}
			}
#endif

#ifdef FNS
			if (dnd_fnsname[0]) {
#ifndef NOTHREADS
				switch_it2(c, dnd_fnsname, dnd_fnsaddr, main_win);
#else
				switch_it(c, dnd_fnsname, dnd_fnsaddr, main_win);
#endif
				dnd_fnsname[0] = NULL;
				dnd_fnsaddr[0] = NULL;
				break;
			}
#endif /* FNS */
			if (drag_load_proc(data, c) < 0) {
				animateInfo->status = DtDND_FAILURE;
			}
			break;
		case DtDND_BUFFER_TRANSFER:
		    /* retrieve data from filename saved in handle_drop_cb() */
#ifdef FNS
			if (dnd_fnsname[0]) {
#ifndef NOTHREADS
				switch_it2(c, dnd_fnsname, dnd_fnsaddr, main_win);
#else
				switch_it(c, dnd_fnsname, dnd_fnsaddr, main_win);
#endif
				dnd_fnsname[0] = NULL;
				dnd_fnsaddr[0] = NULL;
				break;
			}
#endif /* FNS */

			if (!dnd_filename[0]){
				animateInfo->status = DtDND_FAILURE;
				return;
			}
	
			if (drag_load_proc(dnd_filename, c) < 0) {
				animateInfo->status = DtDND_FAILURE;
			}
	
			unlink(dnd_filename);
			dnd_filename[0] = NULL;
			break;

		default:
			animateInfo->status = DtDND_FAILURE;
			return;
		}
	}

	return;
}

static void
handle_drop_cb(
	Widget		w,
	XtPointer	client_data,
	XtPointer	call_data)
{
	Display		*display = XtDisplay(w);
	DtDndTransferCallbackStruct *transfer_info = (DtDndTransferCallbackStruct *)call_data;
	Calendar	*c;
	char		*data;
	int		size;
	FILE		*fp;
	int		i;
#if defined (FNS) && defined(FNS_DEMO)
	char		addr_buf[MAXNAMELEN];
	char		name_buf[MAXNAMELEN];
#endif

	c = (Calendar *)client_data;

	transfer_info->status = DtDND_SUCCESS;

	dnd_filename[0] = NULL;

	for (i = 0; i < transfer_info->dropData->numItems; i++) {
		switch(transfer_info->dropData->protocol) {
		case DtDND_FILENAME_TRANSFER:
			data = transfer_info->dropData->data.files[i];
	
#if defined(FNS) && defined(FNS_DEMO)
			if (cmfns_use_fns(c->properties) &&
			    cmfns_name_from_file(data, buf, sizeof(buf)) == 1) {
				/* 
				 * Looks like an HFS file has been dropped on us.
				 * Get the calendar service associated with the
				 * FNS name and browse it
				 */
				if (cmfns_lookup_calendar(buf,
						addr_buf, sizeof(addr_buf)) > 0) {
					
#ifndef NOTHREADS
					switch_it2(c, buf, addr_buf, main_win);
#else
					switch_it(c, buf, addr_buf, main_win);
#endif
					if (dnd_filename[0]) {
					    unlink(dnd_filename);
					    dnd_filename[0]=NULL;
					}
					return;
				}
			}
#endif
	
			if (validate_dropped_appt(data, c) == False) {
#ifdef FNS
				if (validate_fns_name(1, data, c, 
				      dnd_fnsname, sizeof(dnd_fnsname),
				      dnd_fnsaddr, sizeof(dnd_fnsaddr)) == False) {
					dnd_fnsname[0] = NULL;
					dnd_fnsaddr[0] = NULL;
#endif /* FNS */
					transfer_info->status = DtDND_FAILURE;
#ifdef FNS
			}
#endif /* FNS */
			}
			break;
		case DtDND_BUFFER_TRANSFER:
	
			/*
			 * Save data to a file so we can pass it to 
			   handle_animate_cb() which calls drag_load_proc().
			 */
		        /* if multiple drops, then this will append each
			   drop to dnd_filename */

			if (!dnd_filename[0]){
			    strcpy(dnd_filename, "/tmp/cmXXXXXX");
			    mktemp(dnd_filename);
			}
			if ((fp = fopen(dnd_filename, "a")) == 0) {
			    transfer_info->status = DtDND_FAILURE;
			    dnd_filename[0] = NULL;
			    return;
			}
			data = transfer_info->dropData->data.buffers[i].bp;
			size = transfer_info->dropData->data.buffers[i].size;
			fwrite(data, 1, size, fp);
			fclose(fp);
			if (validate_dropped_appt(dnd_filename, c) == False) {
#ifdef FNS
				if (validate_fns_name(1, dnd_filename, c, 
				      dnd_fnsname, sizeof(dnd_fnsname),
				      dnd_fnsaddr, sizeof(dnd_fnsaddr)) == False) {
					dnd_fnsname[0] = NULL;
					dnd_fnsaddr[0] = NULL;
#endif /* FNS */
					transfer_info->status = DtDND_FAILURE;
#ifdef FNS
				}
#endif /* FNS */
				unlink(dnd_filename);
				dnd_filename[0] = NULL;	
			}
			break;
		default:
			transfer_info->status = DtDND_FAILURE;
			if (dnd_filename[0]) {
			    unlink(dnd_filename);
			    dnd_filename[0] = NULL;
			}
			return;
		}
	}
	return;
}

void
cm_register_drop_site(
	Calendar	*c,
	Widget		w)

{
	XtCallbackRec	transfer_cb_rec[] = { {handle_drop_cb, NULL},
					      {NULL, NULL} };
	static XtCallbackRec animateCBRec[] = { {handle_animate_cb, NULL},
						{NULL, NULL} };
	Display		*display = XtDisplayOfObject(w);

	/*
	 * The above string "CalendarAppointment" is hard coded to match the type
	 * used in dtdnddemo.  In the future we need to use the true
	 * type from the data typing database
	 */

	transfer_cb_rec[0].closure = (XtPointer)c;
	animateCBRec[0].closure = (XtPointer)c;

	DtDndVaDropRegister(w, DtDND_FILENAME_TRANSFER | DtDND_BUFFER_TRANSFER,
			XmDROP_COPY,
			transfer_cb_rec, 
			DtNdropAnimateCallback, animateCBRec,
			DtNtextIsBuffer, 	True,
			NULL);
	return;
}

/*
 * Insert the appointment into the calendar.
 *
 * Returns:
 *		1	Success
 *		0	User overuled.  Appointment not inserted.
 *		-1	Failure
 */
static int
schedule_appt(Calendar *c, Dtcm_appointment *a) {
	char		date_buf[MAXNAMELEN], buf[BUFSIZ], buf2[BUFSIZ];
	int		answer;
	Editor		*e = (Editor *)c->editor;
	ToDo		*t = (ToDo *)c->todo;
	Props		*p = (Props *)c->properties;
	CSA_entry_handle	entry;
	OrderingType	ot = get_int_prop(p, CP_DATEORDERING);
	SeparatorType	st = get_int_prop(p, CP_DATESEPARATOR);
	Tick		tick;

	if (strcmp(c->caladdr, c->view->current_calendar) != 0) {
		/*
		 * Make sure user really meant to insert appointment
		 * into somebody elses calendar.
		 */
		char *ident = XtNewString(catgets(c->DT_catd, 1, 923, "Cancel"));
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 212,
				"Calendar : Schedule Appointment"));
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 210, "The appointment will be scheduled in the calendar\nyou are currently browsing.  Do you still want to schedule it?"));
		sprintf(buf2, "%s %s", catgets(c->DT_catd, 1, 211, "Schedule in"),
			c->view->current_calname);
		answer = dialog_popup(c->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, buf,
			BUTTON_IDENT, 1, ident,
			BUTTON_IDENT, 2, buf2,
			NULL);
		XtFree(title);
		XtFree(ident);
		if (answer == 1)
			return 0;
	}

	if (editor_insert(a, &entry, c)) {

		_csa_iso8601_to_tick(a->time->value->item.string_value, &tick);
		format_tick(tick, ot, st, date_buf);
		sprintf(buf, catgets(c->DT_catd, 1, 214,
				     "Appointment scheduled: %s"), date_buf);
		set_message(c->message_text, buf);
		return 1;
	} else {
		set_message(c->message_text, "");
		return -1;
	}
}

/*
 * Call the routines in file_parse (in libDtCm) to read the appointments!
 */
extern int 
drag_load_proc(char *filename, Calendar *c) {
	int			ret_val, i = 1;
	char			buf[MAXNAMELEN * 2];
	CmDataList		*list = CmDataListCreate();
	Props			*p = (Props *)c->properties;
	Props_pu		*pu = (Props_pu *)c->properties_pu;
	Validate_op		op;
	Dtcm_appointment	*a;
	char			*msg;

	if (!filename || *filename == '\0')
		return -1;

	op = parse_appt_from_file(c->DT_catd, filename, list, p, query_user, 
				  (void *)c, c->general->version);
	if (list->count <= 0) {
		op = CANCEL_APPT;
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 842, 
	     "The information transferred did not\ncontain any appointments."));
	}

	switch(op) {
	case COULD_NOT_OPEN_FILE:
	  	msg = XtNewString(catgets(c->DT_catd, 1, 843, 
					"Drag and Drop operation failed."));
		sprintf(buf, "%s\n%s",
			msg,
			catgets(c->DT_catd, 1, 844, 
			      "Unable to locate the transferred information."));
		XtFree(msg);
		break;
	case INVALID_DATE:
		sprintf(buf, "%s",
			catgets(c->DT_catd, 1, 218, "Invalid DATE specified"));
		break;
	case INVALID_START:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 219,
					   "Invalid START time specified"));
		break;
	case INVALID_STOP:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 220,
					   "Invalid END time specified"));
		break;
	case MISSING_DATE:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 221,
					   "Empty or missing DATE field"));
		break;
	case MISSING_START:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 222,
					   "Empty or missing START field"));
		break;
	case MISSING_WHAT:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 223,
					   "Empty or missing WHAT field"));
		break;
	case REPEAT_FOR_MISMATCH:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 224,
					   "REPEAT and FOR field mismatch"));
		break;
	case VALID_APPT:
		break;
	case CANCEL_APPT:
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 225,
					   "Schedule appointment was cancelled."));
		break;
	default:
		op = CANCEL_APPT;
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 225,
					   "Schedule appointment was cancelled."));
		break;
	}

	while (op == VALID_APPT && i <= list->count) {
		extern void scrub_attr_list(Dtcm_appointment *); 

		a = (Dtcm_appointment *)CmDataListGetData(list, i);

		scrub_attr_list(a);

		ret_val = schedule_appt(c, a);
		if (ret_val < 0) {
			op = CANCEL_APPT;
			sprintf(buf, "%s", catgets(c->DT_catd, 1, 226,
				"Internal error scheduling appointment."));
		} else if (ret_val == 0) {
			op = CANCEL_APPT;
			sprintf(buf, "%s", catgets(c->DT_catd, 1, 225,
				"Schedule appointment was cancelled."));
		}
		++i;
	}

	for (i = 1; i <= list->count; i++)
		if (a = (Dtcm_appointment *)CmDataListGetData(list, i))
			free_appt_struct(&a);
	CmDataListDestroy(list, _B_FALSE);

	if (op != VALID_APPT) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 1073,
					  "Calendar : Error - Drag and Drop"));
		char *ident = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		dialog_popup(c->frame,
			DIALOG_TITLE, title,
			DIALOG_TEXT, buf,
			BUTTON_IDENT, 1, ident,
			BUTTON_HELP, DND_ERROR_HELP,
			DIALOG_IMAGE, pu->xm_error_pixmap,
			NULL);
		XtFree(ident);
		XtFree(title);
		return -1;
	}

	return 0;
}

/* gets a pointer to the currently selected appointment in the editor.  
   This will need to be changed if we ever allow more than one item 
   to be selected in the editor at a time. */

CSA_entry_handle
get_appt_struct(DragContext *context) {
	int             *item_list = NULL, item_cnt = 0, answer;
	char		buf[MAXNAMELEN];
	Widget 		list;
	Calendar 	*c = context->calendar;
	Props_pu	*pr;
	CSA_entry_handle	entry;
	Access_data	*ad;

	pr = (Props_pu *)(c->properties_pu);

	if (context->editor_type == SingleEditorList)
		list = ((Editor *) context->editor)->appt_list;
	else if (context->editor_type == GroupEditorList)
		list = ((GEditor *) context->editor)->appt_list;
	else if (context->editor_type == TodoEditorList)
		list = ((ToDo *) context->editor)->todo_list;

        if (!XmListGetSelectedPos(list, &item_list, &item_cnt)) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 230, 
				"Calendar : Error - Drag Appointment"));
		char *text = XtNewString(catgets(c->DT_catd, 1, 231, "Select an appointment and DRAG again."));
		char *ident = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
                answer = dialog_popup(c->frame,
                        DIALOG_TITLE, title,
                        DIALOG_TEXT, text,
                        BUTTON_IDENT, 1, ident,
			BUTTON_HELP, RESELECT_ERROR_HELP,
                        DIALOG_IMAGE, pr->xm_error_pixmap,
                        NULL);
		XtFree(ident);
		XtFree(text);
		XtFree(title);
                return(NULL);
        }
 

	if (context->editor_type == SingleEditorList)
		entry = editor_nth_appt((Editor *)context->editor,
					item_list[0] - 1);
	else if (context->editor_type == GroupEditorList)
		entry = geditor_nth_appt((GEditor *)context->editor,
					 item_list[0] - 1, &ad);
	else if (context->editor_type == TodoEditorList)
		entry = t_nth_appt((ToDo *)context->editor,
					 item_list[0] - 1);


        if (!entry) {
	  	char *title = XtNewString(catgets(c->DT_catd, 1, 230, 
					"Calendar : Error - Drag Appointment"));
		char *ident = XtNewString(catgets(c->DT_catd, 1, 95, "Continue"));
		sprintf(buf, "%s", catgets(c->DT_catd, 1, 845,
		"Drag and Drop operation Failed\nInternal consistency error."));
                answer = dialog_popup(c->frame,
                        DIALOG_TITLE, title,
                        DIALOG_TEXT, buf,
                        BUTTON_IDENT, 1, ident,
                        DIALOG_IMAGE, pr->xm_error_pixmap,
                        NULL);
		XtFree(ident);
		XtFree(title);
                XtFree((XtPointer)item_list);
                return(NULL);
        }
	else
		return entry;
}

/*
 * ApptConvertCB
 *
 * Fills in data object with calendar appointment string based on which
 * appointment in the list was under the pointer when the drag started.
 */
static void
ApptConvertCB(
        Widget          dragContext,
        XtPointer       clientData,
        XtPointer       callData)
{
        DtDndConvertCallbackStruct *convertInfo 
					= (DtDndConvertCallbackStruct*)callData;
        DtDndBuffer 	*data		= &(convertInfo->dragData->data.buffers[0]);
        DragContext     *context        = (DragContext *)clientData;
        Display         *display        = XtDisplay(dragContext);
        Atom            CMAPPOINTMENT 	
			= XmInternAtom(display, "CalendarAppointment", False);
        int             selectedPos;
	CSA_entry_handle	entry;
	Calendar	*c = context->calendar;
	Props		*p = (Props *)c->properties;
        char            *apptstr;
	Dtcm_appointment        *appt;
	Editor		*e = (Editor *) context->editor;
	GEditor		*ge = (GEditor *) context->editor;
	ToDo		*t = (ToDo *) context->editor;

	if (convertInfo->reason != DtCR_DND_CONVERT_DATA)
		return;
 
	if ((context->editor_type == SingleEditorList) || 
	    (context->editor_type == GroupEditorList) || 
	    (context->editor_type == TodoEditorList)) {
		if (!(entry = get_appt_struct(context)))
			return;
		if (!(apptstr = parse_appt_to_string(c->cal_handle, 
						entry, p, c->general->version)))
			return;
	}
	else {
		appt = allocate_appt_struct(appt_write, DATAVER_ARCHIVE, NULL);
        	load_appt_defaults(appt, (Props *) c->properties);
		if (context->editor_type == SingleEditorIcon) {
        		dssw_form_to_appt(&e->dssw, appt, c->calname, now());
        		rfp_form_to_appt(&e->rfp, appt, c->calname);
		}
		else if (context->editor_type == GroupEditorIcon) {
        		dssw_form_to_appt(&ge->dssw, appt, c->calname, now());
        		rfp_form_to_appt(&ge->rfp, appt, c->calname);
		}
		else if (context->editor_type == TodoEditorIcon) {
        		dssw_form_to_appt(&t->dssw, appt, c->calname, now());
        		rfp_form_to_appt(&t->rfp, appt, c->calname);
			appt->type->value->item.sint32_value = CSA_TYPE_TODO;
			appt->show_time->value->item.sint32_value = True;
			t->completed_val = 
				XmToggleButtonGadgetGetState(t->completed_toggle);
			appt->state->value->item.sint32_value = 
				(t->completed_val) ? 
				CSA_STATUS_COMPLETED : CSA_X_DT_STATUS_ACTIVE;
		}
		apptstr = parse_attrs_to_string(appt, (Props *)c->properties, 
				attrs_to_string(appt->attrs, appt->count));
		free_appt_struct(&appt);
	}

        /* REMIND: Need to check convertInfo->reason, handle DELETE, etc */
 
        data->bp   = XtNewString(apptstr);
        data->size = strlen(data->bp);
        data->name = XtNewString(catgets(c->DT_catd, 1, 236, "CalendarAppointment"));

	free(apptstr);
}
 

/*
 * getIcon
 *
 * Returns a new IconInfo structure with bitmap, mask, width, height,
 * icon type and name.
 */
static void
GetIcon(Calendar *calendar)
{
 
        Display        *display = XtDisplay(calendar->frame);
        Window          window = XtWindow(calendar->frame);
        unsigned char  *bitmapData, *bitmapMask;
        Editor          *e = (Editor *) calendar->editor;
        GEditor         *ge = (GEditor *) calendar->geditor;
 
        if (e->drag_bitmap == NULL) {
                e->drag_bitmap = XCreateBitmapFromData(display,
                        window, (char *) drag_xbm_bits,
                        drag_xbm_width, drag_xbm_height);
                if (e->drag_bitmap == NULL) {

                        printf(catgets(calendar->DT_catd, 1, 237, "XCreateBitmapFromData() failed for bitmap.\n"));
                        return;
                }
                else
                        ge->drag_bitmap = e->drag_bitmap;
        }
        if (e->drag_mask == NULL) {
                e->drag_mask = XCreateBitmapFromData(display,
                        window, (char *) drag_mask_xbm_bits,
                        drag_mask_xbm_width, drag_mask_xbm_height);
                if (e->drag_mask == NULL) {
                        printf(catgets(calendar->DT_catd, 1, 238, "XCreateBitmapFromData() failed for mask.\n"));
                        return;
                }
                else
                        ge->drag_mask = e->drag_mask;
        }
}
 
/*
 * DragFinishCB
 *
 * Resets drag state to indicate the drag is over. Free memory allocated
 * with the drag.
 */
static void
DragFinishCB(
        Widget          widget,
        XtPointer       clientData,
        XtPointer       callData)
{
        DragContext     *context = (DragContext *) clientData;

	extern Calendar	*calendar;

	if ((context->editor_type == SingleEditorList) || (context->editor_type == SingleEditorIcon))
        	((Editor *) context->editor)->doing_drag = False;
	
	else if ((context->editor_type == TodoEditorList) || (context->editor_type == TodoEditorIcon))
        	((ToDo *) context->editor)->doing_drag = False;
	else if ((context->editor_type == GroupEditorList) || (context->editor_type == GroupEditorIcon)) 
        	((GEditor *) context->editor)->doing_drag = False;

        if (!context)
                return;
 
        if (context->data)
                free(context->data);
 
        free(context);
}

Widget
CreateDragSourceIcon(
        Widget          widget,
        Pixmap          pixmap,
        Pixmap          mask)
{
        Widget          dragIcon;
        Window          rootWindow;
        int             pixmapX, pixmapY;
        unsigned int    pixmapWidth, pixmapHeight, pixmapBorder, pixmapDepth;
        Arg             args[20];
        Cardinal        nn = 0;
 
        XGetGeometry (XtDisplayOfObject(widget), pixmap, &rootWindow,
                &pixmapX, &pixmapY, &pixmapWidth, &pixmapHeight,
                &pixmapBorder, &pixmapDepth);
 
        XtSetArg(args[nn], XmNwidth, pixmapWidth);  nn++;
        XtSetArg(args[nn], XmNheight, pixmapHeight);  nn++;
        XtSetArg(args[nn], XmNmaxWidth, pixmapWidth);  nn++;
        XtSetArg(args[nn], XmNmaxHeight, pixmapHeight);  nn++;
        XtSetArg(args[nn], XmNpixmap, pixmap);  nn++;
        XtSetArg(args[nn], XmNmask, mask);  nn++;
        XtSetArg(args[nn], XmNdepth, pixmapDepth);  nn++;
        dragIcon = XmCreateDragIcon(widget, "sourceIcon", args, nn);

        return(dragIcon);
}

XtActionProc
TranslationDragStart(
        Widget          widget,
        XEvent          *event,
	String		*parms,
	Cardinal	*num_params)
{
        static XtCallbackRec convertCBRec[] = { {ApptConvertCB, NULL},
                                                {NULL, NULL} };
        static XtCallbackRec dragFinishCBRec[] =  { {DragFinishCB, NULL},
                                                    {NULL, NULL} };
 
        Display        *display 	= XtDisplay(widget);
        int             itemCount, selectedPos;
        DragContext     *context = calloc(sizeof(DragContext), 1);
        Editor          *e = (Editor *) calendar->editor;
 
        context->calendar = calendar;

	if (((Editor *)calendar->editor)->appt_list == widget) {
        	context->editor_type = SingleEditorList;
                context->editor = (caddr_t) calendar->editor;
	} else if (((GEditor *)calendar->geditor)->appt_list == widget) { 
		context->editor_type = GroupEditorList;
		context->editor = (caddr_t) calendar->geditor;
	} else if (((ToDo *)calendar->todo)->todo_list == widget) { 
		context->editor_type = TodoEditorList;
		context->editor = (caddr_t) calendar->todo;
	}
	
        GetIcon(calendar);
 
        convertCBRec[0].closure = (XtPointer)context;
        dragFinishCBRec[0].closure = (XtPointer)context;
 
        if (e->drag_icon == NULL) {
                e->drag_icon = CreateDragSourceIcon(widget, 
						    e->drag_bitmap, 
						    e->drag_mask);
        }
 
        if (DtDndVaDragStart(widget, event, DtDND_BUFFER_TRANSFER, 1,
            		     XmDROP_COPY, 
			     convertCBRec, dragFinishCBRec,
			     DtNsourceIcon, e->drag_icon,
			     NULL) == NULL) {
 
                printf(catgets(calendar->DT_catd, 1, 239, 
					"DragStart returned NULL.\n"));
        }

	return NULL;
}
 
void
ApptDragStart(
        Widget          widget,
        XEvent          *event,
        Calendar        *calendar,
        EditorType      editor_type)
{
        static XtCallbackRec convertCBRec[] = { {ApptConvertCB, NULL},
                                                {NULL, NULL} };
        static XtCallbackRec dragFinishCBRec[] =  { {DragFinishCB, NULL},
                                                    {NULL, NULL} };
 
        Display        *display 	= XtDisplay(widget);
        int             itemCount, selectedPos;
        DragContext     *context = calloc(sizeof(DragContext), 1);
        Editor          *e = (Editor *) calendar->editor;
 
        context->calendar = calendar;
        context->editor_type = editor_type;
 
        if (editor_type == SingleEditorIcon)
                context->editor = (caddr_t) calendar->editor;
        else if (editor_type == GroupEditorIcon)
                context->editor = (caddr_t) calendar->geditor;
        else if (editor_type == TodoEditorIcon)
                context->editor = (caddr_t) calendar->todo;
 
        GetIcon(calendar);
 
        convertCBRec[0].closure = (XtPointer)context;
        dragFinishCBRec[0].closure = (XtPointer)context;
 
        if (e->drag_icon == NULL) {
                e->drag_icon = CreateDragSourceIcon(widget, e->drag_bitmap, e->drag_mask);
        }
 
        if (DtDndVaDragStart(widget, event, DtDND_BUFFER_TRANSFER, 1,
            	XmDROP_COPY, 
		convertCBRec, dragFinishCBRec,
		DtNsourceIcon,		e->drag_icon,
		NULL)
            == NULL) {
 
                printf(catgets(calendar->DT_catd, 1, 239, "DragStart returned NULL.\n"));
        }
}

