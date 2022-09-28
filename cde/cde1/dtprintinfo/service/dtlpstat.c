/* dtrlpstat.c: Remote printer status listing client */

#include <stdio.h>
#include <string.h>
#include "dtrlpstat.h"

#ifdef aixv2
#include <string.h>
#define NO_MESSAGE_CAT
#endif

#ifdef NO_MESSAGE_CAT

#define Name "Job Name"
#define Number "Job Number"
#define Owner "Owner"
#define Date "Date"
#define Time "Time"
#define Size "Size"
#define Bytes "bytes"

#else

/* Message header */
#include "dtprintinfomsg.h"
nl_catd dtprintinfo_cat = NULL;

#endif


main(argc, argv)
   int argc;
   char *argv[];
{
   StatusLineList job_list;
   int n_jobs;
   int widths[7];
   int width;
   int i, j;
   char *s;
   char fmt[50];
   char buf[200];
   char *fields[6];

#ifndef NO_MESSAGE_CAT
   char *Name;
   char *Number;
   char *Owner;
   char *Date;
   char *Time;
   char *Size;
   char *Bytes;

   setlocale(LC_ALL, "");
#ifdef _POWER
   dtprintinfo_cat = catopen("dtprintinfo.cat", NL_CAT_LOCALE);
#else
   dtprintinfo_cat = catopen("dtprintinfo.cat", 0);
#endif
   Name = MESSAGE(JobNameL);
   Number = MESSAGE(JobNumberL);
   Owner = MESSAGE(OwnerL);
   Date = MESSAGE(DateL);
   Time = MESSAGE(TimeL);
   Size = MESSAGE(SizeL);
   Bytes = MESSAGE(BytesL);
#endif

   if (argc == 3)
    {
      if (!GetRemotePrintJobs(argv[1], argv[2], &job_list, &n_jobs))
       {
          fprintf(stderr, "%s", RemoteLpstatErrorString);
          exit(1);
       }
    }
   else if (argc == 2)
    {
      GetPrintJobs(argv[1], &job_list, &n_jobs);
    }
   else
    {
      fprintf(stderr, "usage: %s host printer\n", argv[0]);
      exit(1);
    }
   widths[0] = strlen(Name);
   widths[1] = strlen(Number);
   widths[2] = strlen(Owner);
   widths[3] = strlen(Date);
   widths[4] = strlen(Time);
   widths[5] = strlen(Size);
   widths[6] = strlen(Bytes) + 1;
   for (i = 0; i < n_jobs; i++)
    {
      strcpy(buf, job_list[i]->line);
      s = strtok(buf, "|");
      if ((width = strlen(s)) > widths[0])
         widths[0] = width;
      for (j = 0; j < 5; j++)
       {
         s = strtok(NULL, "|");
         if ((width = strlen(s)) > widths[j])
            widths[j] = width;
       }
      s = strtok(NULL, "|");
      if ((width = strlen(s) + widths[6]) > widths[5])
         widths[5] = width;
    }
   sprintf(fmt, "%%-%ds  %%-%ds  %%-%ds  %%-%ds  %%-%ds  %%-%ds\n",
	   widths[0], widths[1], widths[2], widths[3], widths[4], widths[5]);
   printf(fmt, Name, Number, Owner, Date, Time, Size);
   for (i = 0; i < 5; i++)
    {
      for (j = 0; j < widths[i]; j++)
	putchar('-');
      putchar(' ');
      putchar(' ');
    }
   for (j = 0; j < widths[5]; j++)
      putchar('-');
   putchar('\n');
   widths[5] -= widths[6];
   sprintf(fmt, "%%-%ds  %%%ds  %%-%ds  %%%ds  %%%ds  %%%ds",
	   widths[0], widths[1], widths[2], widths[3], widths[4], widths[5]);
   for (i = 0; i < n_jobs; i++)
    {
      strtok(job_list[i]->line, "|");
      for (j = 0; j < 6; j++)
         fields[j] = strtok(NULL, "|");
      printf(fmt, fields[0], fields[1], fields[2], fields[3], fields[4],
	     fields[5]);
      printf(" %s\n", Bytes);
    }
   FreeRemotePrintJobs();
   exit(0);
}
