/*******************************************************************************
**
**  props.c
**
**  $XConsortium: props.c /main/cde1_maint/2 1995/10/10 13:26:13 pascale $
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

#pragma ident "@(#)props.c	1.18 96/11/12 Sun Microsystems, Inc."

#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/Intrinsic.h>
#include "props.h"
#include "util.h"

/*******************************************************************************
**
**  Constants
**
*******************************************************************************/
static const char	*UPGRADED	= "Upgraded";
static const char	*CLASS_NAME	= "deskset";
static const char	*APP_NAME	= "calendar";
static const char	*RC_FILENAME	= "/.cm.rc";
static const char	*DS_FILENAME	= "/.desksetdefaults";
static const char	*OW_FILENAME	= "/lib/app-defaults/Deskset";
static const char	*X_FILENAME	= "/.Xdefaults";

/*
**  List of constant strings in ~/.desksetdefaults which we will pay attention
**  to ... if you add a member to this array, be sure to add it to the
**  enumerated type in props.h
*/
#ifdef __osf__
const char	*property_names[] = {
#else
static const char     *property_names[] = {
#endif
				"BegOp",
				"BeepOn",
				"BeepAdvance",
				"BeepUnit",
				"FlashOn",
				"FlashAdvance",
				"FlashUnit",
				"OpenOn",
				"OpenAdvance",
				"OpenUnit",
				"MailOn",
				"MailAdvance",
				"MailUnit",
				"MailTo",
				"UnixOn",
				"UnixAdvance",
				"UnixCommand",
				"DayBegin",
				"DayEnd",
				"CalendarList",
				"DefaultView",
				"DefaultDisplay",
				"PrintDest",
				"PrintPrivacy",
				"PrinterName",
				"PrintOptions",
				"PrintDirName",
				"PrintFileName",
				"PrintWidth",
				"PrintHeight",
				"PrintPosXOff",
				"PrintPosYOff",
				"PrintUnit",
				"PrintCopies",
				"DefaultCal",
				"Location",
				"DateOrdering",
				"DateSeparator",
				"Privacy",
				"UseFNS",
				"ApptBegin",
				"ApptDuration",
				"EndOp"
			};

/*
**  List of constant strings containing hard-coded defaults.  The program will
**  also attempt to read some stuff from the environment if the hard-coded
**  default is NULL (example:  $PRINTER)
**
**  If you add a member to this array, be sure to add it above and to the
**  enumerated type in props.h
*/
static const char	*def_props[] = {
				"\0",			/* BegOp */
				"True",			/* BeepOn */
				"5",			/* BeepAdvance */
				"Mins",			/* BeepUnit */
				"False",		/* FlashOn */
				"5",			/* FlashAdvance */
				"Mins",			/* FlashUnit */
				"True",			/* OpenOn */
				"5",			/* OpenAdvance */
				"Mins",			/* OpenUnit */
				"False",		/* MailOn */
				"2",			/* MainAdvance */
				"Hrs",			/* MailUnit */
				"\0",			/* MailTo */
				"False",		/* UnixOn */
				"0",			/* UnixAdvance */
				"\0",			/* UnixCommand */
				"7",			/* DayBegin */
				"19",			/* DayEnd */
				"\0",			/* CalendarList */
				"1",			/* DefaultView */
				"0",			/* DefaultDisplay */
				"0",			/* PrintDest */
				"7",			/* PrintPrivacy */
				"\0",			/* PrinterName */
				"\0",			/* PrintOptions */
				"\0",			/* PrintDirName */
				"calendar.ps",		/* PrintFileName */
				"7.50",			/* PrintWidth */
				"10.00",		/* PrintHeight */
				"1.00",			/* PrintPosXOff */
				"1.00",			/* PrintPosYOff */
				"1",			/* PrintUnit */
				"1",			/* PrintCopies */
				"\0",			/* DefaultCal */
				"\0",			/* CalendarLocation */
				"0",			/* DateOrdering */
				"1",			/* DateSeparator */
				"Show Time And Text",	/* Privacy */
				"False",		/* UseFNS */
				"540",			/* ApptBegin */
				"60",			/* ApptDuration */
				"\0"			/* EndOp */
			};

/*******************************************************************************
**
**  Functions static to props.c
**
*******************************************************************************/
static void
p_free_props_entries(Props_entry *e) {
	if (!e)
		return;
	if (e->next)
		p_free_props_entries(e->next);

	if (e->property_name)
		free(e->property_name);
	if (e->property_value)
		free(e->property_value);
	free(e);
}

static Props_entry*
p_get_props_entry(Props *p, Props_op op) {
	Props_op	idx = CP_BEGOP + 1;
	Props_entry	*step = p->p_list;

	while(step && (idx < op)) {
		step = step->next;
		++idx;
	}
	return step;
}

/*******************************************************************************
**
**  Functions external to props.c
**
*******************************************************************************/
/*
**  Moving .cm.rc properties to .desksetdefaults
*/
extern boolean_t
cal_convert_cmrc(Props *p) {
	char		*c_ptr, fn[MAXPATHLEN], *val;
	Props_op	op;
	Resource	*cm_rdb = NULL;

	if (!p->rdb)
		return _B_FALSE;
	if (get_resource(p->rdb, (char *)CLASS_NAME, (char *)APP_NAME,
		(char *)UPGRADED, NULL))
		return _B_TRUE;

	/*
	**  If we're here, the Upgraded resource hasn't been set, so read
	**  the old .cm.rc file then write it to .desksetdefaults.
	*/
	if (getenv("HOME") != NULL)
		sprintf(fn, "%s%s", getenv("HOME"), RC_FILENAME);
	else
		return _B_TRUE;

	if (!load_resources(&cm_rdb, fn))
		return _B_TRUE;

	for (op = CP_BEGOP + 1; op < CP_ENDOP; op++) {
		if (!(val = get_resource(cm_rdb, (char *)CLASS_NAME,
			(char *)APP_NAME, (char *)property_names[op], NULL)))
			continue;

		set_resource(&p->rdb, (char *)CLASS_NAME, (char *)APP_NAME,
			(char *)property_names[op], val);
	}
	set_resource(&p->rdb, (char *)CLASS_NAME, (char *)APP_NAME,
		(char *)UPGRADED, "True");
	save_props(p);
	free_resources(cm_rdb);

	return _B_TRUE;
}

extern char*
get_char_prop(Props *p, Props_op op) {
	Props_entry	*step = p_get_props_entry(p, op);

	if (!step)
		return get_char_prop_default(op);
	else if (!step->property_value || step->property_value == '\0')
		step->property_value = get_char_prop_default(op);

	return (step->property_value);
}

extern char*
get_char_prop_default(Props_op op) {
	char		*val, *ptr, *user, *host;

	switch(op) {
	case CP_MAILTO:
		user = (char *)cm_get_uname();
		host = (char *)cm_get_local_host();
		val = (char *)XtCalloc(1, cm_strlen(user) + cm_strlen(host) +2);
		sprintf(val, "%s@%s", user, host);
		break;
	case CP_PRINTERNAME:
		val = cm_def_printer();
		break;
	case CP_PRINTDIRNAME:
		if (getenv("HOME") != NULL)
			val = (char *)cm_strdup(getenv("HOME"));
		else
			val = (char *)cm_strdup("/");
		break;
	case CP_DEFAULTCAL:
		user = (char *)cm_get_uname();
		host = (char *)cm_get_local_host();
		val = (char *)XtCalloc(1, cm_strlen(user) + cm_strlen(host) +2);
		sprintf(val, "%s@%s", user, host);
		break;
	case CP_CALLOC:
		val = (char *)cm_strdup(cm_get_local_host());
		break;
	default:
		val = (char *)cm_strdup((char *)def_props[op]);
		break;
	}

	return val;
}

extern int
get_int_prop(Props *p, Props_op op) {
	Props_entry	*step = p_get_props_entry(p, op);

	if (!step || !step->property_value || step->property_value == '\0')
		return (atoi(get_char_prop_default(op)));
	return (atoi(step->property_value));
}

extern void
props_clean_up(Props *p) {
	p_free_props_entries(p->p_list);
	free_resources(p->rdb);
}

extern boolean_t
read_props(Props *p) {
	char		*resource, buf[MAXPATHLEN];
	char		*home, *ow_home, *x_env, *ds_def;
	Props_op	op;
	Resource	*other_rdb = NULL;
	Props_entry	*entry_ptr, *last = NULL;

	if (getenv("HOME") != NULL)
		home = getenv("HOME");
	else
		home = "/";

	ow_home = getenv("OPENWINHOME");
	x_env = getenv("XENVIRONMENT");
	ds_def = getenv("DESKSETDEFAULTS");

	if (p->rdb)
		free_resources(p->rdb);
	p->rdb = NULL;
	if (ds_def)
		sprintf(buf, "%s", ds_def);
	else
		sprintf(buf, "%s%s", home, DS_FILENAME);

	load_resources(&p->rdb, buf);

	if (ow_home) {
		sprintf(buf, "%s%s", ow_home, OW_FILENAME);
		load_resources(&other_rdb, buf);
	}
	if (home) {
		sprintf(buf, "%s%s", home, X_FILENAME);
		load_resources(&other_rdb, buf);
	}
	if (x_env) {
		sprintf(buf, "%s%s", x_env, X_FILENAME);
		load_resources(&other_rdb, buf);
	}

	p_free_props_entries(p->p_list);
	for (op = CP_BEGOP + 1; op < CP_ENDOP; op++) {
		entry_ptr = (Props_entry *)XtCalloc(1, sizeof(Props_entry));
		entry_ptr->property_name = (char *)
			cm_strdup((char *)property_names[op]);
		entry_ptr->update = _B_FALSE;
		if (last)
			last->next = entry_ptr;
		else
			p->p_list = entry_ptr;
		entry_ptr->next = NULL;
		last = entry_ptr;

                resource = (char *)get_resource(p->rdb, (char *)CLASS_NAME,
			(char *)APP_NAME, entry_ptr->property_name, NULL);
		if (!resource || *resource == '\0')
                	resource = (char *)get_resource(other_rdb,
				(char *)CLASS_NAME, (char *)APP_NAME,
				entry_ptr->property_name, NULL);
		if (!resource || *resource == '\0')
			/* Delay loading defaults until they are needed.
			entry_ptr->property_value = get_char_prop_default(op);
			*/
			entry_ptr->property_value = NULL;
		else
			entry_ptr->property_value = (char *)cm_strdup(resource);
	}
	free_resources(other_rdb);
	return _B_TRUE;
}

extern boolean_t
save_props(Props *p)
{
	char		buf[MAXPATHLEN], *ds_def = getenv("DESKSETDEFAULTS");
	Props_entry	*p_ptr = p->p_list;

	while(p_ptr) {
		if (p_ptr->update && p_ptr->property_value)
			set_resource(&p->rdb, (char *)CLASS_NAME,
				(char *)APP_NAME, p_ptr->property_name,
				p_ptr->property_value);
		p_ptr = p_ptr->next;
	}

	if (ds_def)
		sprintf(buf, "%s", ds_def);
	else
		if (getenv("HOME") != NULL)
			sprintf(buf, "%s%s", getenv("HOME"), DS_FILENAME);
		else
			sprintf(buf, "/%s", DS_FILENAME);

	return (save_resources(p->rdb, buf));
}        

extern boolean_t
set_char_prop(Props *p, Props_op op, char *value) {
	Props_entry	*step = p_get_props_entry(p, op);

	if (!step)
		return _B_FALSE;

	if (step->property_value)
		free(step->property_value);
	step->property_value = (char *)cm_strdup(value);
	step->update = _B_TRUE;
	return _B_TRUE;
}

extern boolean_t
set_int_prop(Props *p, Props_op op, int value) {
	char buf[MAXNAMELEN];

	sprintf(buf, "%d", value);
	return (set_char_prop(p, op, buf));
}
