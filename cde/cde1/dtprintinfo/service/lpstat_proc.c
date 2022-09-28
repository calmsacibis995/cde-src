#include <stdio.h>
#include <rpc/rpc.h>
#include <string.h>
#include "lpstat.h"
#include "dtlpstat.h"

#ifdef NO_XDR_POINTER
bool_t
xdr_pointer(xdrs,objpp,obj_size,xdr_obj)
   register XDR *xdrs;
   char **objpp;
   u_int obj_size;
   xdrproc_t xdr_obj;
{
   bool_t more_data;

   more_data = (*objpp != NULL);
   if (! xdr_bool(xdrs,&more_data))
      return (FALSE);

   if (! more_data)
    {
      *objpp = NULL;
      return (TRUE);
    }
   return (xdr_reference(xdrs,objpp,obj_size,xdr_obj));
}
#endif

#ifdef aixv2
static char *strdup(s)
   char *s;
{
   char *s1 = (char *)malloc(strlen(s) + 1);
   strcpy(s1, s);
   return s1;
}

void
xdr_free(proc, objp)
   xdrproc_t proc;
   char *objp;
{
   XDR x;

   x.x_op = XDR_FREE;
   (*proc)(&x, objp);
}
#endif

dtlpstat_res *
dtlplistjobs_1(printer)
   nametype *printer;
{
   static dtlpstat_res res; /* must be static! */
   namelist nl;
   namelist *nlp;
   StatusLineList job_list = NULL;
   int i;
   int n_jobs;

   /* Free previous result */
   if (job_list)
      xdr_free(xdr_dtlpstat_res, &res);

   /* Get status line entries */
   GetPrintJobs(*printer, &job_list, &n_jobs);

   /* Collect status line entries */
   nlp = &res.dtlpstat_res_u.list;
   for (i = 0; i < n_jobs; i++)
    {
      nl = *nlp = (namenode *) malloc(sizeof(namenode));
      nl->name = strdup(job_list[i]->line);
      nlp = &nl->next;
    }
   *nlp = NULL;

   /* Return the result */
   res.errno = 0;
   return (&res);
}
