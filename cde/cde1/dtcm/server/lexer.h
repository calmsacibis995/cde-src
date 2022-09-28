/* $XConsortium: lexer.h /main/cde1_maint/1 1995/07/17 20:11:40 drk $ */
/*
 *  (c) Copyright 1993, 1994 Hewlett-Packard Company
 *  (c) Copyright 1993, 1994 International Business Machines Corp.
 *  (c) Copyright 1993, 1994 Novell, Inc.
 *  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 */

#ifndef _LEXER_H
#define _LEXER_H

#pragma ident "@(#)lexer.h	1.4 96/11/12 Sun Microsystems, Inc."

#include "ansi_c.h"
#include "rtable4.h"

extern int		yyylineno;
extern int		externNumberVal;
extern char		*externQuotedString;
extern Period_4		externPeriod;
extern Tag_4		externTag;
extern Appt_Status_4	externApptStatus;
extern Privacy_Level_4	externPrivacy;

extern void		setinput(FILE*);
extern int		yyylex	();

#endif
