/* $XConsortium: utility.h /main/cde1_maint/1 1995/07/17 20:20:59 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _UTILITY_H
#define _UTILITY_H

#pragma ident "@(#)utility.h	1.8 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"

extern void _DtCmsComposeReferenceId(long id, char *calname, char buf[]);

extern char * _DtCmsGetNextWord(char **line);

extern int _DtCmsGetConfigInfo();

extern void _DtCmsSyslog(const char *format, ...);


#endif
