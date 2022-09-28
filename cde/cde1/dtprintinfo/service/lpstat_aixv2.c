#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>

#include "dtlpstat.h"

typedef struct 
{
   char *queue;
   char *device;
} QueueDeviceStruct, *QueueDevice, **QueueDeviceList;

static void check_dir();
static int SortJobs();

#define TMP_DIR "/usr/lpd/qdir"

static char *strdup(s)
   char *s;
{
   char *s1 = (char *)malloc(strlen(s) + 1);
   strcpy(s1, s);
   return s1;
}

void
GetPrintJobs(printer, return_job_list, return_n_jobs)
   char *printer;
   StatusLineList *return_job_list;
   int *return_n_jobs;
{
   static StatusLineList job_list = NULL;
   static QueueDeviceList que_list = NULL;
   static int n_queues = 0;
   static int prev_n_jobs = 0;
   FILE *p;
   int n_jobs = 0;
   int i;
   char *device;
	
   if (!que_list)
    {
      FILE *p;
      char buf[100];
      p = popen("/bin/print -q | awk 'NF == 0 {exit 0} NR > 2 {print $2,$1}'",
		"r");
      if (!p)
       {
         *return_job_list = NULL;
         *return_n_jobs = 0;
         return;
       }
      while (fgets(buf, 100, p))
       {
	 if (n_queues == 0)
	    que_list = (QueueDeviceList) malloc(sizeof(QueueDevice));
	 else
	    que_list = (QueueDeviceList) realloc(que_list, (n_queues + 1) *
						 sizeof(QueueDevice));
	 que_list[n_queues] = (QueueDevice) malloc(sizeof(QueueDeviceStruct));
	 que_list[n_queues]->queue = strtok(buf, " ");
	 que_list[n_queues]->queue = strdup(que_list[n_queues]->queue);
	 que_list[n_queues]->device = strtok(NULL, "\n");
	 que_list[n_queues]->device = strdup(que_list[n_queues]->device);
	 n_queues++;
       }
      pclose(p);
    }

   chdir(TMP_DIR);
   for (i = 0; i < n_queues; i++)
      if (!strcmp(que_list[i]->queue, printer))
       {
	 device = que_list[i]->device;
	 break;
       }
   check_dir(printer, device, &job_list, &n_jobs, prev_n_jobs);
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs;
   qsort(job_list, n_jobs, sizeof(StatusLineList), SortJobs);

   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
}

static void
check_dir(printer, device, job_list, n_jobs, prev_n_jobs)
   char *printer;
   char *device;
   StatusLineList *job_list;
   int *n_jobs;
   int prev_n_jobs;
{
   DIR *lp_tmp_dir;
   struct dirent *dir_struct;
   char buf[256];
   int line_ct;
   char *line;
   char date_str[50];
   char owner[100];
   char job_size[10];
   FILE *job;
   time_t secs;
   char *s;
   char filename[200];
   char filename1[200];
   StatusLineList j_list;
   struct stat statbuff;
   struct tm *p_tm;

   if (!(lp_tmp_dir = opendir(".")))
    {
      closedir(lp_tmp_dir);
      exit(0);
    }

   dir_struct = readdir(lp_tmp_dir);
   for ( ; dir_struct ; dir_struct = readdir(lp_tmp_dir))
    {
      int len = strlen(dir_struct->d_name);
      if (len < 3 || *dir_struct->d_name == '.')
	 continue;
      s = strchr(dir_struct->d_name, '.');
      s++;
      if (strncmp(device, s, strlen(device)))
	 continue;
      if (!(job = fopen(dir_struct->d_name, "r")))
	 continue;
      line_ct = 1;
      while (fgets(buf, 100, job))
       {
	 line = strtok(buf, "\n");
	 switch (line_ct)
	 {
	 case 4: strcpy(filename, line); break;
	 case 12: strcpy(owner, line); break;
         }
	 line_ct++;
       }
      s = strtok(line, " ");
      s = strtok(NULL, " ");
      s = strtok(NULL, " ");
      strcpy(filename1, s);
      if (filename1[1])
         strcat(filename1, "/");
      s = strtok(NULL, " ");
      strcat(filename1, s);
      if (stat(filename1, &statbuff) >= 0)
         sprintf(job_size, "%d", (int) statbuff.st_size);
      else
         strcpy(job_size, "0");
      if (stat(dir_struct->d_name, &statbuff) >= 0)
         secs = statbuff.st_mtime;
      else
         time(&secs);
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
      if (*n_jobs >= prev_n_jobs)
       {
         if (*n_jobs == 0)
            *job_list = (StatusLineList) malloc(sizeof(StatusLine));
         else
            *job_list = (StatusLineList) realloc(*job_list, (*n_jobs + 1) *
                                                 sizeof(StatusLine));
         j_list = *job_list;
         j_list[*n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
         len = strlen(printer) + strlen(filename) + 
               strlen(owner) + strlen(date_str) + strlen(job_size) + 11;
         j_list[*n_jobs]->line = (char *) malloc(len);
       }
      else
       {
         j_list = *job_list;
         len = strlen(printer) + strlen(filename) + 
               strlen(owner) + strlen(date_str) + strlen(job_size) + 11;
         j_list[*n_jobs]->line = (char *) realloc(j_list[*n_jobs]->line, len);
       }
      sprintf(j_list[*n_jobs]->line, "%s|%s|%s|%s|%s|%s", printer,
              filename, "N/A", owner, date_str, job_size);
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
