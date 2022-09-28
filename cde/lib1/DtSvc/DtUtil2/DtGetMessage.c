/* $XConsortium: DtGetMessage.c /main/cde1_maint/1 1995/07/17 18:09:36 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/******************************************************************************
 *
 * File Name: _DtGetMessage.c
 *
 *  Contains the function for getting localized strings.
 *
 *****************************************************************************/
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Dt/DtP.h>
#include <Dt/UserMsg.h>
#include <Dt/DtNlUtils.h>

/*
 * This is required so that the BMS override of these definitions do not
 * get used.
 */
#undef strcpy
#undef strcat
#undef strlen


#ifdef MESSAGE_CAT
#include <nl_types.h>

/*****************************************************************************
 *
 * Function: Dt11GetMessage
 *
 * Parameters:
 *
 *   char       *filename -     Filename to open.
 *
 *   int        set -           The message catalog set number.
 *
 *   int        n -             The message number.
 *
 *   char       *s -            The default message if the message is not
 *                              retrieved from a message catalog.
 *
 * Returns: the string for set 'set' and number 'n'.
 *
 *****************************************************************************/

char *
#ifdef _NO_PROTO
Dt11GetMessage(filename, set,n,s)
	char *filename;
	int set;
	int n;
	char *s;
#else
Dt11GetMessage(
	char *filename,
	int set,
	int n,
	char *s)
#endif
{
        char *msg;
        char *lang;
        nl_catd catopen();
        char *catgets();
        static int first = 1;
        static nl_catd nlmsg_fd;
        if ( first )
        {
                first = 0;
		nlmsg_fd = catopen(filename, 0);
        }
        msg=catgets(nlmsg_fd,set,n,s);
        return (msg);
}
#endif

