
/*
 *	$XConsortium: gil_loadattP.h /main/cde1_maint/3 1995/10/16 10:16:07 rcs $
 *
 *	@(#)gil_loadattP.h	1.9 20 Jan 1995	cde_app_builder/src/libABil
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * loadatt.h - load attribute (GIL) header file
 */
#ifndef _ABIL_LOADATT_H_
#define _ABIL_LOADATT_H_

#include <ab_private/obj.h>
#include "gilP.h"

typedef struct
{
	BOOL			bools[AB_OBJ_MAX_CHILDREN];
	int			count;
} BOOL_ARRAY;

typedef struct
{
	AB_OBJECT_STATE		states[AB_OBJ_MAX_CHILDREN];
	int			count;
} INITIAL_STATE_ARRAY;

typedef struct
{
	ISTRING	strings[AB_OBJ_MAX_CHILDREN];
	int	count;
} ISTRING_ARRAY;

typedef struct
{
	AB_LABEL_TYPE	label_types[AB_OBJ_MAX_CHILDREN];
	int		count;
} LABEL_TYPE_ARRAY;

int     istr_array_init(ISTRING_ARRAY *array);
int     istr_array_uninit(ISTRING_ARRAY *array);

int	gilP_load_attribute_value(FILE *inFile, 
			ABObj obj, AB_GIL_ATTRIBUTE attr, ABObj root);
int	gilP_load_bools(FILE *inFile, BOOL_ARRAY *bools);
int	gilP_load_label_types(FILE *inFile, 
			LABEL_TYPE_ARRAY *label_types);
int	gilP_load_initial_states(FILE *inFile, 
			INITIAL_STATE_ARRAY *states);
int	gilP_load_string(FILE *inFile, ISTRING *string);
int	gilP_load_strings(FILE *inFile, ISTRING_ARRAY *strings);
int	gilP_load_name(FILE *inFile, ISTRING *name);
int	gilP_load_names(FILE *inFile, ISTRING_ARRAY *names);
int	gilP_load_handler(FILE *inFile, ISTRING *handler);
int	gilP_load_handlers(FILE *inFile, ISTRING_ARRAY *handlers);
int	gilP_load_file_names(FILE *inFile, ISTRING_ARRAY *names);
int	gilP_load_file_name(FILE *inFile, ISTRING *name);

#endif /* _ABIL_LOADATT_H_ */

