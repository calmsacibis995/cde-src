/* $XConsortium: Process.h /main/cde1_maint/2 1995/10/23 10:30:56 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef PROCESS_H
#define PROCESS_H

class Process {

   int NumProcs;
   int last_pid;
   char **pprocs;
   char *procs;
   char *last_proc;
   char *GetByPid(int);
   int uid;
   int pid;
   int ppid;

 public:

   Process();
   ~Process();

   int Parent(int pid);
   int UID(int pid);
   char *Command(int pid);
};

#endif // PROCESS_H
