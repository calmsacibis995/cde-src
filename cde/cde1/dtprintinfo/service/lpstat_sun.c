#include <stdio.h>
#include "dtlpstat.h"

#include <dirent.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>

#define TMP_DIR "/usr/spool/lp/tmp"
#define REQ_DIR "/usr/spool/lp/requests/%s/"
#define SPOOL_DIR "/var/spool/lp/tmp/%s/"

static void check_dir(char *printer, char *tmp_dir, StatusLineList *job_list,
                      int *n_jobs, int prev_n_jobs);
static int SortJobs(StatusLineList, StatusLineList);

void GetPrintJobs(char *printer, StatusLineList *return_job_list, 
                  int *return_n_jobs)
{
   static StatusLineList job_list = NULL;
   static int prev_n_jobs = 0;
   DIR *lp_tmp_dir;
   struct dirent *dir_struct;
   struct stat statbuff;
   int n_jobs = 0;
   int i;
	
   if (getenv("TMP_DIR"))
      chdir(getenv("TMP_DIR"));
   else
      chdir(TMP_DIR);
   if (!(lp_tmp_dir = opendir(".")))
    {
      closedir(lp_tmp_dir);
      *return_job_list = NULL;
      *return_n_jobs = 0;
      return;
    }

   dir_struct = readdir(lp_tmp_dir);
   for ( ; dir_struct ; dir_struct = readdir(lp_tmp_dir))
    {
      if (*dir_struct->d_name == '.')
         continue;
      if (stat(dir_struct->d_name, &statbuff) >= 0 &&
          statbuff.st_mode & S_IFDIR)
       {
         chdir(dir_struct->d_name);
         check_dir(printer, dir_struct->d_name, &job_list, &n_jobs,
		   prev_n_jobs);
         chdir("..");
       }
    }
   closedir(lp_tmp_dir);
   prev_n_jobs = prev_n_jobs > n_jobs ? prev_n_jobs : n_jobs;
   qsort(job_list, n_jobs, sizeof(StatusLineList), SortJobs);

   *return_job_list = job_list;
   *return_n_jobs = n_jobs;
}

static void check_dir(char *printer, char *tmp_dir, StatusLineList *job_list,
                      int *n_jobs, int prev_n_jobs)
{
   DIR *lp_tmp_dir;
   struct dirent *dir_struct;
   char buf[256];
   char request[300];
   int req_len;
   char spool_dir[300];
   int spool_len;
   int line_ct;
   char *line;
   char date_str[100];
   char owner[100];
   char job_size[100];
   FILE *req, *job;
   time_t secs;
   int found;
   int immediate;
   char *s;
   char title[200];
   char filename[200];
   char filename1[200];
   char *jobname;
   StatusLineList j_list;

   if (getenv("REQ_DIR"))
      sprintf(request, getenv("REQ_DIR"), tmp_dir);
   else
      sprintf(request, REQ_DIR, tmp_dir);
   req_len = strlen(request);
   sprintf(spool_dir, SPOOL_DIR, tmp_dir);
   spool_len = strlen(spool_dir);

   if (!(lp_tmp_dir = opendir(".")))
    {
      closedir(lp_tmp_dir);
      return;
    }

   dir_struct = readdir(lp_tmp_dir);
   for ( ; dir_struct ; dir_struct = readdir(lp_tmp_dir))
    {
      int len = strlen(dir_struct->d_name);
      if (len < 3)
	 continue;
      if (strcmp(dir_struct->d_name + len - 2, "-0"))
	 continue;
      *(request + req_len) = '\0';
      strcat(request + req_len, dir_struct->d_name);
      if (!(req = fopen(request, "r")))
	 continue;
      if (!(job = fopen(dir_struct->d_name, "r")))
       {
         fclose(req);
	 continue;
       }
      found = 1;
      immediate = 0;
      title[0] = '\0';
      filename[0] = '\0';
      filename1[0] = '\0';
      date_str[0] = '\0';
      owner[0] = '\0';
      job_size[0] = '\0';
      while (found && fgets(buf, 100, job))
       {
	 line = strtok(buf, "\n");
	 switch (*line)
	 {
	 case 'H':
	    if (!strcmp("immediate", line + 2))
	       immediate = 1;
	    break;
	 case 'D':
	    if (strcmp(printer, line + 2))
	       found = 0;
	    break;
	 case 'F':
            *(spool_dir + spool_len) = '\0';
            strncat(spool_dir, dir_struct->d_name, len - 1);
	    strcat(spool_dir, "1");
            if (strcmp(spool_dir, line + 2))
	       strcpy(filename1, line + 2);
            else
	       *filename1 = '\0';
	    break;
         case 'O':
            if (s = strrchr(line, ':'))
               *s = '\0';
            if (s = strrchr(line, '\''))
             {
               s++;
               for ( ; *s == ' '; s++)
                  ;
             }
            strcpy(filename, s);
	    if (*s == '\0')
	     {
	       if (s = strstr(line, "-T"))
		{
		  int i = 0;
		  s += 2;
		  while (1)
		   {
		     if (*s == ' ')
		      {
			if (i == 0)
			 {
                           i++;
			   break;
			 }
			else 
			 {
			   i--;
			   if (filename[i] != '\\')
			    {
			      i++;
			      break;
			    }
			 }
		      }
                     filename[i++] = *s;
		     s++;
		   }
                 filename[i] = '\0';
		}
	     }
	    break;
         case 'T':
	    if (s = strstr(line, "\\n"))
	       *s = '\0';
	    strcpy(title, line + 2);
	    break;
	 }
       }
      if (found)
       {
         line_ct = 1;
         while (line_ct)
          {
	    fgets(buf, 100, req);
	    line = strtok(buf, "\n");
	    switch (line_ct)
	    {
	    case 3:
	       if (s = strchr(line, '!'))
                {
		  *s = '\0';
		  s++;
                  strcpy(owner, s); 
                  strcat(owner, "@"); 
                  strcat(owner, line); 
                }
               else
                  strcpy(owner, line); 
	       break;
	    case 5: strcpy(job_size, line); break;
	    case 6: 
               secs = (time_t)atoi(line);
	       strftime(date_str, 100, "%b %e|%T", localtime(&secs));
	       line_ct = -1;
	       break;
	    }
	    line_ct++;
          }
         if (s = strstr(dir_struct->d_name, "-0"))
            *s = '\0';
         if (*filename)
            jobname = filename;
         else if (*filename1)
            jobname = filename1;
         else
            jobname = title;
	 if (*n_jobs >= prev_n_jobs)
	  {
            if (*n_jobs == 0)
               *job_list = (StatusLineList) malloc(sizeof(StatusLine));
	    else
               *job_list = (StatusLineList) realloc(*job_list, (*n_jobs + 1) *
						    sizeof(StatusLine));
            j_list = *job_list;
	    j_list[*n_jobs] = (StatusLine) malloc(sizeof(StatusLineStruct));
	    len = 6 + strlen(printer) + strlen(jobname) + strlen(date_str) +
		  strlen(dir_struct->d_name) + strlen(owner) + strlen(job_size);
	    j_list[*n_jobs]->line = (char *) malloc(len);
	  }
	 else
	  {
	    j_list = *job_list;
	    len = 6 + strlen(printer) + strlen(jobname) + strlen(date_str) +
		  strlen(dir_struct->d_name) + strlen(owner) + strlen(job_size);
	    j_list[*n_jobs]->line = (char *)realloc(j_list[*n_jobs]->line, len);
	  }
         sprintf(j_list[*n_jobs]->line, "%s|%s|%s|%s|%s|%s", printer,
		 jobname, dir_struct->d_name, owner, date_str, job_size);
	 j_list[*n_jobs]->secs = (long) secs;
	 j_list[*n_jobs]->immediate = immediate;
	 (*n_jobs)++;
       }
      fclose(req);
      fclose(job);
    }
   closedir(lp_tmp_dir);
}

static int SortJobs(StatusLineList first, StatusLineList second)
{
   if ((**first).immediate && !(**second).immediate)
      return -1;
   if (!(**first).immediate && (**second).immediate)
      return 1;
   if ((**first).immediate)
      return (int)((**second).secs - (**first).secs);
   else
      return (int)((**first).secs - (**second).secs);
}
