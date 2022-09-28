/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "Process.h"
#include "Invoke.h"

#include <string.h>
#include <stdlib.h>

Process::Process()
{
#ifdef aix
   (void)Invoke("ps -e -F \"pid ppid uid command\"", &procs);
#else
   (void)Invoke("/bin/ps -el | awk '{printf(\"%s %s %s %s\\n\",$4,$5,$3,$NF)}'",
		&procs);
#endif
   pprocs = (char **)malloc(sizeof(char *));
   NumProcs = 0;
   strtok(procs, "\n");
   while(pprocs[NumProcs] = strtok(NULL, "\n"))
    {
      NumProcs++;
      pprocs = (char **)realloc(pprocs, sizeof(char *) * (NumProcs + 1));
    }
   last_pid = -1;
}

Process::~Process()
{
   free(procs);
   free(pprocs);
}

char *Process::GetByPid(int _pid)
{
   int i;

   if (last_pid == _pid)
      return last_proc;
   if (_pid)
      for (i = 0; i < NumProcs; i++)
       {
	 sscanf(pprocs[i], "%d %d %d", &pid, &ppid, &uid);
         if (_pid == pid)
	  {
	     last_pid = _pid;
	     last_proc = pprocs[i];
	     return pprocs[i];
	  }
       }
   return NULL;
}

int Process::Parent(int pid)
{
   char *proc = GetByPid(pid);
   if (proc)
      return ppid;
   else
      return -1;
}

int Process::UID(int pid)
{
   char *proc = GetByPid(pid);
   if (proc)
      return uid;
   else
      return -1;
}

char *Process::Command(int _pid)
{
   char *proc = GetByPid(_pid);
   if (proc)
    {
      char *s;
      // Find first field
      for (s = proc; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find second field
      for ( ; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find third field
      for ( ; *s == ' '; s++)
	 ;
      for ( ; *s != ' '; s++)
	 ;
      // Find fourth field
      for ( ; *s == ' '; s++)
	 ;
      return s;
    }
   else
      return NULL;
}
