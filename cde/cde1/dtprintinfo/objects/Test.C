/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#include "PrintSubSys.h"
#include "Queue.h"
#include "PrintJob.h"

// Message header
#include "dtprintinfomsg.h"
nl_catd dtprintinfo_cat = NULL;

#ifdef hpux
char *Catgets(nl_catd, int msg_set, int msg, char *s)
{
   return catgets(0, msg_set, msg, s);
}
#endif

void PrintDetails(BaseObj *obj, int level)
{
   if (level == 1)
      printf("%-16s %s\n", obj->Name(), obj->Details());
   else if (level == 2)
      printf("   %-16s       %s\n", obj->Name(), obj->Details());
   else
      printf("%16s %s\n", " ", obj->Details());

   int i;
   BaseObj **children = obj->Children();
   int n_children = obj->NumChildren();
   for (i = 0; i < n_children; i++)
      PrintDetails(children[i], level + 1);
}

int main(int /*argc*/, char ** /*argv*/)
{
   PrintSubSystem *prt;
   BaseObj **queues;
   int n_queues;
   int i, j, n_jobs;
   BaseObj **jobs;

   setlocale(LC_ALL, "");
#ifdef _POWER
   dtprintinfo_cat = catopen("dtprintinfo.cat", NL_CAT_LOCALE);
#else
   dtprintinfo_cat = catopen("dtprintinfo.cat", 0);
#endif

   prt = new PrintSubSystem(NULL);
   queues = prt->Children();
   n_queues = prt->NumChildren();

   for (i = 0; i < n_queues; i++)
    {
      queues[i]->DisplayName();
      queues[i]->Details();
      queues[i]->ReadAttributes();
      jobs = queues[i]->Children();
      n_jobs = queues[i]->NumChildren();
      for (j = 0; j < n_jobs; j++)
       {
         jobs[j]->DisplayName();
         jobs[j]->Details();
       }
    }

   prt->DumpHierarchy(true);
   PrintDetails(prt, 0);
   return 0;
}
