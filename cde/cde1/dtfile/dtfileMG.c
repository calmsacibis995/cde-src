/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           dtfileMG.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Contains the nls related routines.
 *
 *   FUNCTIONS: GETMESSAGE
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#define TRUE 1
#define FALSE 0
#include <nl_types.h>
char *
GETMESSAGE(set,n,s)
int set;
int n;
char *s;
{
  	     extern int errno;
	int save;
        char *msg;
	nl_catd catopen();
	char *catgets();
	static int first = TRUE;
	static nl_catd nlmsg_fd;
	save=errno;
	errno=0;
	if ( first ) {
		nlmsg_fd = catopen("dtfile",0);
                if (nlmsg_fd == (nl_catd)-1)
                {
                   char * oldlang;
                   static char resetval[100];

                   oldlang = (char *)getenv("LANG");
                   putenv("LANG=C");
                   nlmsg_fd = catopen("dtfile",0);
                   if (oldlang)
                   {
                      /* Restore C */
                      strcpy(resetval, "LANG=");
                      strcat(resetval, oldlang);
                      putenv(resetval);
                   }
                }
		first = FALSE;
		}
	msg=catgets(nlmsg_fd,set,n,s);
        errno=save;
	return (msg);
}
