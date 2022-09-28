/* $XConsortium: ilversion.c /main/cde1_maint/1 1995/07/17 19:08:04 drk $ */
/*  /ilc/ilversion.c : Contains the IL version string
    (which should really be dynamically generated).
*/

#include "ilint.h"

/*
   Image Library version <external>[ <baselevel>], for <OS> <OS version> '<library name>'
   WARNING: FIXED FORMAT EXPECTED BY /ilc/iltiffwrite.c !!!
   See notes below "ilVersionName".
*/

#ifdef __hpux 
    IL_PRIVATE 
   char _ilVersionString[] = "HP IL BETA v 23.  bl08";
/*                                nnnnn vvvvvv bbbb       see below
                                  xxxxxxxxxxxxxxxxx       total size = 17 chars

        The above version string *must* maintain the syntax and position of all fields
        before the ";".  The strings have the following format; each string is 5 chars:
            nnnnn       A 5 char fixed string equal to "HP IL ".
            vvvvvv      A 6 char string in the format "v!!.??", where !! is the two-digit
                        major version number (" n" if less than 10), and ?? is the minor
                        version number ("n " if less than 10).  Examples: "v10.12",
                        "v 9.6 ".
            bbbbb       A 4 char string in the format "bl??", where ?? is the baselevel
                        number (e.g. "bl01", "bl56").

*/

#else

    IL_PRIVATE
   char _ilVersionString[] = "@(#)HP IL v 2.1  bl06; Image Library for Domain/OS SR10.X 'il'";
/*                                nnnnn vvvvvv bbbb       see above
                                  xxxxxxxxxxxxxxxxx       total size = 17 chars
*/

#endif

