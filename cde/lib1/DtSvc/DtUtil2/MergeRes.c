/* $XConsortium: MergeRes.c /main/cde1_maint/1 1995/07/17 18:13:29 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        MergeRes.h
 **
 **   Project:     DT
 **
 **   Description: Contains the DT function used during initialization to 
 **                merge in the set of global DT resources into the 
 **                application's resource database.
 **
 **
 **   (c) Copyright 1987, 1988, 1989 by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netdb.h>
#include <stdlib.h>
#include <X11/Xlib.h>
#include <X11/Xlibint.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <Dt/DtPStrings.h>



/* This function reads in the "Dt" app-defaults file that holds global
 * DT resources and merges it with the standard resource database.  For
 * DT 2.0 compatibility it also reads the /usr/lib/X11/app-defaults/Dt
 * file which help the Dt.TypesDirs resource. 
 */

void 
#ifdef _NO_PROTO
MergeDtResources( display )
        Display *display ;
#else
MergeDtResources(
        Display *display )
#endif /* _NO_PROTO */
{
   char *filename, *tempS;
   Boolean foundOld;
   XrmDatabase vdb;
   struct stat stbuf;

   if ((filename = XtResolvePathname (display, "app-defaults", 
				      DtDT_PROG_CLASS, NULL, NULL, NULL, 0,
				      NULL)) != NULL) {
      vdb = XrmGetFileDatabase (filename);
      XtFree (filename);

      /* The direction of this merge is important.  If "display->db" were
	 the target, then entries in the "Dt" file would overwrite entries
	 in the standard database.  This would be wrong because the 
	 standard database might have "Dt" entries obtained from a user's
	 personal defaults. */
      XrmMergeDatabases (display->db, &vdb);
      display->db = vdb;
   }

   /* Read the old DT 2.0 file.  This is needed so that DT 3.0 can
      continue to look up the old DT 2.0 actions. */
   foundOld = False;
   tempS = getenv("LANG");
   if (tempS != NULL && *tempS != '\0') {
       filename = XtMalloc (strlen ("/usr/lib/X11/app-defaults") + 
			    strlen (tempS) + strlen ("Dt") + 3);
       sprintf (filename, "/usr/lib/X11/app/defaults/%s/Dt", tempS);
       if ((vdb = XrmGetFileDatabase (filename)) != NULL) {
	   XrmMergeDatabases (display->db, &vdb);
	   display->db = vdb;
	   foundOld = True;
       }
       XtFree (filename);
   }

   if (!foundOld && 
       (vdb = XrmGetFileDatabase ("/usr/lib/X11/app-defaults/Dt")) != NULL) {
	   XrmMergeDatabases (display->db, &vdb);
	   display->db = vdb;
   }
}

