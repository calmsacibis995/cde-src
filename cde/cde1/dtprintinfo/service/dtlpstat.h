/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef _DTLPSTAT_H
#define _DTLPSTAT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
   int immediate;
   long secs;
   char *line;
} StatusLineStruct, *StatusLine, **StatusLineList;

#ifdef __cplusplus
extern void GetPrintJobs(char *printer, StatusLineList *return_job_list,
	                 int *return_n_jobs);
#else
extern void GetPrintJobs();
#endif

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _DTLPSTAT_H */
