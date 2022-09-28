/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 *
 * copyright.h
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 */

#define _TT_COPYRIGHT \
"Copyright (c) 1990, Sun Microsystems, Inc.  All Rights Reserved. Sun considers its source code as an unpublished, proprietary trade secret, and it is available only under strict license provisions.  This copyright notice is placed here only to protect Sun in the event the source is deemed a published work. "

#define _TT_COPYRIGHT2 \
"Dissassembly, decompilation, or other means of reducing the object code to human readable form is prohibited by the license agreement under which this code is provided to the user or company in possession of this copy. "

#define _TT_COPYRIGHT3 \
"RESTRICTED RIGHTS LEGEND: Use, duplication, or disclosure by the Government is subject to restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in Technical Data and Computer Software clause at DFARS 52.227-7013 and in similar clauses in the FAR and NASA FAR Supplement."

#define TT_INSERT_COPYRIGHT \
const char _tt_copyright[] = _TT_COPYRIGHT;	\
const char _tt_copyright2[] = _TT_COPYRIGHT2;	\
const char _tt_copyright3[] = _TT_COPYRIGHT3;	

#if defined(SABER)
#define _TT_PRINT_VERSIONS(progname)
#else
#define _TT_PRINT_VERSIONS(progname) \
printf("%s version: %s\nToolTalk library version: %s\n", \
       progname, TT_VERSION_STRING, _tt_lib_version);
#endif

#if !defined(DONT_EXTERN_TT_LIB_VERSION)
extern const char *_tt_lib_version;
#endif
