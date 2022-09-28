#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <sys/stat.h>

#include "dtlpstat.h"

typedef struct
{
   char *printer;
   char *spool_dir;
} PrinterStruct, *Printer, **PrinterList;

static void GetPrinterList();
static void check_dir();
static int SortJobs();

void
GetPrintJobs(printer, return_job_list, return_n_jobs)
   char *printer;
   StatusLineList *return_job_list;
   int *return_n_jobs;
{
   static StatusLineList job_list = NULL;
   static PrinterList que_list = NULL;
   static int n_queues = 0;
   static int prev_n_jobs = 0;
   static char host_name[100] = "";
   int n_jobs = 0;
   int i;
   char *spool_dir;

   if (*host_name == '\0')
    {
      memset(host_name, 0, 100);
      gethostname(host_name, 100);
      /* Only show the hostname, not the domain. Stop at the first period */
      strtok(host_name, ".");
    }
   if (!que_list)
    {
      GetPrinterList(&que_list, &n_queues);
      if (!que_list)
       {
         *return_job_list = NULL;
         *return_n_jobs = 0;
         return;
       }
    } 

   for (i = 0; i < n_queues; i++)
      if (!strcmp(que_list[i]->printer, printer))
       {
         spool_dir = que_list[i]->spool_dir;
         break;
       }
   chdir(spool_dir);
   check_dir(printer, host_name, &job_list, &n_jobs, prev_n_jobs);
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs;
   qsort(job_list, n_jobs, sizeof(StatusLineList), SortJobs);

   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
}

static void
check_dir(printer, host_name, job_list, n_jobs, prev_n_jobs)
   char *printer;
   char *host_name;
   StatusLineList *job_list;
   int *n_jobs;
   int prev_n_jobs;
{
   DIR *lp_tmp_dir;
   struct dirent *dir_struct;
   char buf[256];
   char *line;
   char date_str[50];
   char owner[100];
   char hostname[100];
   char job_number[9];
   char job_size[10];
   FILE *job;
   time_t secs;
   char *s;
   char filename[200];
   char filename1[200];
   StatusLineList j_list;
   struct stat statbuff;
   struct tm *p_tm;
   int i;

   if (!(lp_tmp_dir = opendir(".")))
    {
      closedir(lp_tmp_dir);
      return;
    }

   dir_struct = readdir(lp_tmp_dir);
   for ( ; dir_struct ; dir_struct = readdir(lp_tmp_dir))
    {
      int len = strlen(dir_struct->d_name);
      if (len < 3 || *dir_struct->d_name == '.')
         continue;
      if (strncmp("cf", dir_struct->d_name, 2))
         continue;
      if (!(job = fopen(dir_struct->d_name, "r")))
         continue;
      while (fgets(buf, 100, job))
       {
         line = strtok(buf, "\n");
         switch (*line)
         {
         case 'H': strcpy(hostname, line + 1); break;
         case 'P': strcpy(owner, line + 1); break;
         case 'f': strcpy(filename1, line + 1); break;
         case 'N': strcpy(filename, line + 1); break;
         }
       }
      /* Only show the hostname, not the domain. Stop at the first period */
      strtok(hostname, ".");
      if (strcmp(hostname, host_name))
       {
	 strcat(owner, "@");
	 strcat(owner, hostname);
       }
      if (stat(filename1, &statbuff) >= 0)
         sprintf(job_size, "%d", (int) statbuff.st_size);
      else
         strcpy(job_size, "0");
      p_tm = localtime(&secs);
      sprintf(buf, asctime(p_tm));
      s = strtok(buf, " ");
      s = strtok(NULL, " ");
      strcpy(date_str, s);
      strcat(date_str, " ");
      s = strtok(NULL, " ");
      strcat(date_str, s);
      strcat(date_str, "|");
      s = strtok(NULL, " ");
      strcat(date_str, s);
      s = dir_struct->d_name + 3;
      for (i = 3; i; s++, i--)
	 if (*s != '0')
	    break;
      if (i)
       {
	 strncpy(job_number, s, i);
	 job_number[i] = '\0';
       }
      else
	 strcpy(job_number, "0");
      if (stat(dir_struct->d_name, &statbuff) >= 0)
         secs = statbuff.st_mtime;
      else
         time(&secs);
      if (*n_jobs >= prev_n_jobs)
       {
         if (*n_jobs == 0)
            *job_list = (StatusLineList) malloc(sizeof(StatusLine));
         else
            *job_list = (StatusLineList) realloc(*job_list, (*n_jobs + 1) *
                                                 sizeof(StatusLine));
         j_list = *job_list;
         j_list[*n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
         len = strlen(printer) + strlen(filename) + strlen(job_number) +
               strlen(owner) + strlen(date_str) + strlen(job_size) + 8;
         j_list[*n_jobs]->line = (char *) malloc(len);
       }
      else
       {
         j_list = *job_list;
         len = strlen(printer) + strlen(filename) + strlen(job_number) +
               strlen(owner) + strlen(date_str) + strlen(job_size) + 8;
         j_list[*n_jobs]->line = (char *) realloc(j_list[*n_jobs]->line, len);
       }
      sprintf(j_list[*n_jobs]->line, "%s|%s|%s|%s|%s|%s", printer,
              filename, job_number, owner, date_str, job_size);
      j_list[*n_jobs]->secs = (long) secs;
      (*n_jobs)++;
      fclose(job);
    }
   closedir(lp_tmp_dir);
}

static int
SortJobs(first, second)
   StatusLineList first;
   StatusLineList second;
{
   return (int)((**first).secs - (**second).secs);
}

static void GetPrinterList(que_list, n_queues)
   PrinterList *que_list;
   int *n_queues;
{
   FILE *fp;
   char *s;
   char buf[512];
   char printer[50];
   char spool_dir[300];
   int start_state;

   fp = fopen("/etc/printcap", "r");
   start_state = 1;
   while (fgets(buf, 512, fp))
    {
      if (*buf != '#')
       {
         if (!(s = strtok(buf, " \n\t:")))
            continue;

         if (start_state)
          {
            if (s = strchr(s, '|'))
               *s = '\0';
            strcpy(printer, buf);
            start_state = 0;
          }
         else
          {
            int has_backslash = 0;
            while (s)
             {
               if (!strncmp(s, "sd=", 3))
                  strcpy(spool_dir, s + 3);
               s = strtok(NULL, " \n\t:");
               if (!strcmp(s, "\\"))
                  has_backslash = 1;
             }
             if (!has_backslash)
              {
                if (*n_queues == 0)
                   *que_list = (PrinterList) malloc(sizeof(Printer));
                else
                   *que_list = (PrinterList) realloc(*que_list, (*n_queues + 1)*
                                                     sizeof(Printer));
                (*que_list)[*n_queues] = (Printer)malloc(sizeof(PrinterStruct));
                (*que_list)[*n_queues]->printer = strdup(printer);
                (*que_list)[*n_queues]->spool_dir = strdup(spool_dir);
                (*n_queues)++;
                start_state = 1;
              }
          }
       }
    }
   fclose(fp);
}
