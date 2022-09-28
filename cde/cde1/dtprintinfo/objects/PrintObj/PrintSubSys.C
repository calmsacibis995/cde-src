/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "PrintSubSys.h"
#include "Queue.h"

#include <string.h>

#ifdef aix
const char *LIST_QUEUES = "lsallq | grep -v '^bsh$' | sort";
#else
#ifdef hpux
const char *LIST_QUEUES = "LANG=C lpstat -v | "
			    "awk '"
			    " $2 == \"for\" "
			    "   { "
			    "      x = match($3, /:/); "
			    "      print substr($3, 1, x-1)"
			    "   }' | sort";
#else
#ifdef __osf__
  const char *LIST_QUEUES = "LANG=C lpstat -v | "
                            "nawk '"
                            " $2 == \"for\"    "
                            "   { print $10 }' "
                            "   | sort";
#else
const char *LIST_QUEUES = "LANG=C lpstat -v | "
			    "nawk '"
			    " $2 == \"for\" "
			    "   { "
			    "      x = match($3, /:/); "
			    "      print substr($3, 1, x-1)"
			    "   }' | sort";
#endif
#endif
#endif

// Object Class Name
const char *PRINTSUBSYSTEM = "PrintSubSystem";

PrintSubSystem::PrintSubSystem(BaseObj *parent)
	: BaseObj(parent, "PrintSubSystem")
{
   _displayName = strdup(MESSAGE(PrinterMenuL));
   _details = strdup("Status   Number   Owner      Date       Time       Size");
}

PrintSubSystem::~PrintSubSystem()
{
   // Empty
}

void PrintSubSystem::InitChildren()
{
   char *std_out;
   if (RunCommand(LIST_QUEUES, &std_out))
    {
      Error("InitChildren method could not list queues.");
    }
   else
    {
      char *queue = strtok(std_out, " \n");
      while (queue && *queue)
       {
         new Queue(this, queue);
         queue = strtok(NULL, " \n");
       }
    }
   delete std_out;
}
