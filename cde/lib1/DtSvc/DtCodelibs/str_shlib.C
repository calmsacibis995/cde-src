/*
 * File:	str_shlib.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
// This file is needed for sun shared libraries

#include <stdio.h>

// Globals from strsep.C
//-----------------------
void *_strsep_privbuf = NULL;
void *_strsepb_privbuf = NULL;

void *_strcmbn_privbuf = NULL;
void *_strcmbnb_privbuf = NULL;

