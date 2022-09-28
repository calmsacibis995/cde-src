/* $XConsortium: Invoke.h /main/cde1_maint/2 1995/10/23 10:30:18 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef INVOKE_H
#define INVOKE_H

#include <stdio.h>

class Invoke
{
 
 public:

  int status;

  Invoke(const char *command,
	 char **std_out = NULL,
	 char **std_err = NULL,
	 int uid = -1); // To sun the command as another, set uid >= 0
};

#endif // INVOKE_H
