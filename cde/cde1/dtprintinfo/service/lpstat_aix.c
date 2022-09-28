#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include "dtlpstat.h"

/*
 Need to parse the following 3 forms of output:

 Queue   Dev   Status    Job     Name           From           To            
                         Submitted        Rnk Pri       Blks  Cp          PP %
 ------- ----- --------- ---------        --- ---      ----- ---        ---- --
 bsh     bshde RUNNING   956     STDIN.14937    root           root          
                        08/02/94 09:55:44    1  15          1   1           0  0
                               /var/spool/qdaemon/tOlkCSM

 OR 

 Queue   Dev   Status    Job     Name           From           To            
                         Submitted        Rnk Pri       Blks  Cp          PP %
 ------- ----- --------- ---------        --- ---      ----- ---        ---- --
 john1   lp0   DOWN     
               QUEUED    957     STDIN.14952    root           root          
                        08/02/94 10:17:53    1  15          1   1               
                               /var/spool/qdaemon/tOmgCrx


Also need to parse filenames that are titles: Example, "My Report" below.

               QUEUED    957     My Report      root           root          
                        08/02/94 10:17:53    1  15          1   1               
                               /var/spool/qdaemon/tOmgCrx

*/

void GetPrintJobs(char *printer, StatusLineList *return_job_list, 
                  int *return_n_jobs)
{
   FILE *p;
   char buf[200];
   char buf1[200];
   char buf2[200];
   char buf3[200];
   char qname[30];
   struct stat statbuff;
   char *s;
   static StatusLineList job_list = NULL;
   static int prev_n_jobs = 0;
   int n_jobs;
   char *jname;
   char *jnumber;
   char *owner;
   char *sdate;
   char *stime;
   char *jsize;
   char *dollar1;

   strcpy(qname, printer);
   if (s = strchr(qname, ':'))
      *s = '\0';
      
   sprintf(buf, "enq -qL -P%s", printer);
   if (!(p = popen(buf, "r")))
    {
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return;
    }

   fgets(buf, 200, p);
   fgets(buf, 200, p);
   fgets(buf, 200, p);
   n_jobs = 0;
   while (fgets(buf, 200, p))
    {
      dollar1 = strtok(buf, " ");
      if (!strncmp(dollar1, qname, strlen(dollar1)))
       {
         strtok(NULL, " ");
         strtok(NULL, " ");
       }
      jnumber = strtok(NULL, " ");
      if (!(jnumber && *jnumber))
	 continue;
      jname = strtok(NULL, "\n");
      if (!(jname && *jname))
	 continue;
      for (s = jname + strlen(jname) - 1; *s == ' '; s--)
	 ;
      for ( ; *s != ' '; s--)
	 ;
      for ( ; *s == ' '; s--)
	 ;
      *(s + 1) = '\0';
      for ( ; *s != ' '; s--)
	 ;
      owner = s + 1;
      for ( ; *s == ' '; s--)
	 ;
      *(s + 1) = '\0';
      for (s = jname; *s == ' '; s++)
	 ;
      jname = s;
      fgets(buf1, 200, p);
      sdate = strtok(buf1, " ");
      stime = strtok(NULL, " ");
      strtok(NULL, " ");
      strtok(NULL, " ");
      jsize = strtok(NULL, " ");
      fgets(buf2, 200, p);
      s = strtok(buf2, " \n");
      if (strstr(s, jname))
	 jname = s;
      if (stat(s, &statbuff) < 0)
         statbuff.st_size = atoi(jsize) * 1024;
      sprintf(buf3, "%s|%s|%s|%s|%s|%s|%d", qname, jname, jnumber,
	      owner, sdate, stime, (int)statbuff.st_size);
      if (n_jobs >= prev_n_jobs)
       {
         if (n_jobs == 0)
            job_list = (StatusLineList) malloc(sizeof(StatusLine));
	 else
            job_list = (StatusLineList) realloc(job_list, (n_jobs + 1) *
						sizeof(StatusLine));
	 job_list[n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
	 job_list[n_jobs]->line = (char *) malloc(strlen(buf3) + 1);
       }
      else
       {
	 job_list[n_jobs]->line = (char *) realloc(job_list[n_jobs]->line, 
						   strlen(buf3) + 1);
       }
      strcpy(job_list[n_jobs]->line, buf3);
      n_jobs++;
    }
   pclose(p);
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs; 
   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
}
