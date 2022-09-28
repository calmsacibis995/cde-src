/* $XConsortium: Dt.c /main/cde1_maint/3 1995/11/07 16:31:57 pascale $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*********************************************************************
 *
 *	File:		Dt.c
 *
 *	Description:	Repository for libDtSvc-wide information.
 *
 *********************************************************************/

#include <Dt/Dt.h>

/* CDE Version information */

externaldef(dtversion) const int DtVersion = DtVERSION_NUMBER;
externaldef(dtversionstring) const char* DtVersionString = DtVERSION_STRING;


/* Solaris CDE Version information */

externaldef(sdtversion) const int SDtVersion = SDtVERSION_NUMBER;
externaldef(sdtversionstring) const char* SDtVersionString = SDtVERSION_STRING;

/* Copyright information */
const char *SDtCopyrightYears = SDtCOPYRIGHT_YEARS;

