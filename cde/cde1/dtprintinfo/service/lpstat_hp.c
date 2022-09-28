#include <stdio.h>
#include <string.h>
#include "dtlpstat.h"

/*
 Need to parse the following 2 forms of output:

 coseps-28           guest          priority 0  Aug  9 12:54 on coseps
	 test.ps                                  31160 bytes

 OR

 coseps-29           guest          priority 0  Aug  9 12:56 on hostname
	 (standard input)                         31160 bytes
*/

void GetPrintJobs(printer, return_job_list, return_n_jobs)
   char *printer;
   StatusLineList *return_job_list;
   int *return_n_jobs;
{
   FILE *p;
   char buf[100];
   char buf1[100];
   char buf2[100];
   static StatusLineList job_list = NULL;
   static int prev_n_jobs = 0;
   int n_jobs;
   char *qname;
   char *jname;
   char *jnumber;
   char *owner;
   char *month;
   char *day;
   char *stime;
   char *jsize;
   char *hostname;
	
   sprintf(buf, "lpstat -i %s", printer);
   if (!(p = popen(buf, "r")))
    {
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return;
    }

   n_jobs = 0;
   while (fgets(buf1, 200, p))
    {
      qname = strtok(buf1, "-");
      jnumber = strtok(NULL, " ");
      owner = strtok(NULL, " ");
      strtok(NULL, " ");
      strtok(NULL, " ");
      month = strtok(NULL, " ");
      day = strtok(NULL, " ");
      stime = strtok(NULL, " \n");
      strtok(NULL, " ");
      hostname = strtok(NULL, " \n");
      fgets(buf2, 200, p);
      jname = strtok(buf2, " \t");
      if (*jname == '(')
       {
	 char *tmp = strtok(NULL, " ");
	 *(tmp - 1) = ' ';
       }
      jsize = strtok(NULL, " ");
      if (hostname && strcmp(qname, hostname))
         sprintf(buf, "%s|%s|%s|%s@%s|%s %s|%s|%s", qname, jname, jnumber,
                 owner, hostname, month, day, stime, jsize);
      else
         sprintf(buf, "%s|%s|%s|%s|%s %s|%s|%s", qname, jname, jnumber,
                 owner, month, day, stime, jsize);
      if (n_jobs >= prev_n_jobs)
       {
         if (n_jobs == 0)
            job_list = (StatusLineList) malloc(sizeof(StatusLine));
	 else
            job_list = (StatusLineList) realloc(job_list, (n_jobs + 1) *
						sizeof(StatusLine));
	 job_list[n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
	 job_list[n_jobs]->line = (char *) malloc(strlen(buf) + 1);
       }
      else
       {
	 job_list[n_jobs]->line = (char *) realloc(job_list[n_jobs]->line, 
						   strlen(buf) + 1);
       }
      strcpy(job_list[n_jobs]->line, buf);
      n_jobs++;
    }
   pclose(p); 
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs;
   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
}
