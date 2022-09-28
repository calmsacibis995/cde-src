#include "PrintSubSys.h"
#include "Queue.h"
#include "PrintJob.h"

#include <time.h>
#include <fcntl.h>
#include <unistd.h> // for sleep function
#include <stdlib.h> // for atoi function
#include <sys/stat.h>

// Message header
#include "dtprintinfomsg.h"
nl_catd dtprintinfo_cat = NULL;

const time_t INTERVAL = 60;           // check for new jobs every minute
const time_t RESTART_INTERVAL = 1800; // check for new printers every 30 minutes
const char *DEFAULT_PRINTER_STATUS_DIR = "/var/dt/printer_status";
char *PRINTER_STATUS_DIR;
const char *CONSOLE = "/dev/console";

static boolean ParseLpstat(const char *printer, StatusLineList *job_list,
                           int *n_jobs);

static void CheckDir()
{
   struct stat statbuff;
   if (stat(PRINTER_STATUS_DIR, &statbuff) < 0)
    {
      char *buf = new char [strlen(PRINTER_STATUS_DIR) + 10];
      sprintf(buf, "mkdir -p %s", PRINTER_STATUS_DIR);
      system(buf);
      delete buf;
      chmod(PRINTER_STATUS_DIR, 0755);
    }
   else if (!(statbuff.st_mode & S_IFDIR))
    {
      unlink(PRINTER_STATUS_DIR);
      mkdir(PRINTER_STATUS_DIR, 0755);
    }
}

class LpstatClntQueue : public Queue
{
   char *tmp_filename;
 public:

   LpstatClntQueue(BaseObj *parent, char *name) : Queue(parent, name)
    {
      tmp_filename = new char [strlen(name) + 10];
      sprintf(tmp_filename, "%s.%d", name, getpid());
    }
   ~LpstatClntQueue() { delete tmp_filename; }
   void ProcessJobs();
};

void LpstatClntQueue::ProcessJobs()
{
   FILE *tmp_status_file;
   StatusLineList job_list;
   int i, n_jobs;
   struct stat statbuff;

   chdir(PRINTER_STATUS_DIR);

   if (stat(Name(), &statbuff) < 0)
    {
      int fd = creat(Name(), 0644);
      close(fd);
    }
   if (!(tmp_status_file = fopen(tmp_filename, "w")))
      return;

   // Get remote jobs first
   boolean get_local = true;
   if (is_remote)
    {
      if (!GetRemotePrintJobs(remote_server, remote_printer, &job_list, &n_jobs))
       {
	 // An Error occurred, assume the dtlpstatd daemon is not running on the
	 // print server. Attempt to parse output from lpstat commands
         get_local = ParseLpstat(Name(), &job_list, &n_jobs);
       }
      for (i = 0; i < n_jobs; i++)
         fprintf(tmp_status_file, "%s\n", job_list[i]->line);
    }

   // Get local jobs next
   if (get_local)
    {
#ifdef aix
      if (is_remote)
         GetPrintJobs(local_device, &job_list, &n_jobs);
      else
         GetPrintJobs((char*)Name(), &job_list, &n_jobs);
#else
      GetPrintJobs((char*)Name(), &job_list, &n_jobs);
#endif
      for (i = 0; i < n_jobs; i++)
         fprintf(tmp_status_file, "%s\n", job_list[i]->line);
    }
   fclose(tmp_status_file);

   chdir(PRINTER_STATUS_DIR);

   // Check to see if print job status has changed
   boolean has_changed = false;
   if (tmp_status_file = fopen(tmp_filename, "r"))
    {
      FILE *status_file;
      if (status_file = fopen(Name(), "r"))
       {
	 char line1[256], line2[256], *c1, *c2;
	 c1 = fgets(line1, 256, tmp_status_file);
	 c2 = fgets(line2, 256, status_file);
	 while (c1 && c2)
	  {
	    if (strcmp(line1, line2))
	     {
	       has_changed = true;
	       break;
	     }
	    c1 = fgets(line1, 256, tmp_status_file);
	    c2 = fgets(line2, 256, status_file);
	  }
	 if (has_changed == false)
	  {
	    if (c1 || c2)
	       has_changed = true;
	  }
	 fclose(status_file);
       }
      fclose(tmp_status_file);
    }
   if (has_changed)
      rename(tmp_filename, Name());
   else
      unlink(tmp_filename);
}

static void CheckQueues(time_t interval)
{
   PrintSubSystem *prt;
   LpstatClntQueue **queues;
   BaseObj **objs;
   int n_queues;
   int i;
   time_t start_queues, before_check, after_check;

   prt = new PrintSubSystem(NULL);
   objs = prt->Children();
   n_queues = prt->NumChildren();
   queues = new LpstatClntQueue *[n_queues];

   for (i = 0; i < n_queues; i++)
      queues[i] = new LpstatClntQueue(NULL, (char *)objs[i]->Name());
   delete prt;

   time(&before_check);
   after_check = before_check + interval;
   start_queues = before_check;
   while (before_check - start_queues < RESTART_INTERVAL)
    {
      unsigned int sleep_time;
      sleep_time = (unsigned int)(interval - (after_check - before_check));
      if (sleep_time > 0)
	 sleep(sleep_time);
      CheckDir();
      time(&before_check);
      for (i = 0; i < n_queues; i++)
         queues[i]->UpdateChildren();
      time(&after_check);
    }
   
   for (i = 0; i < n_queues; i++)
      delete queues[i];
}

void main(int argc, char **argv)
{
   int interval;

   setlocale(LC_ALL, "");
#ifdef _NL_CAT_LOCALE
   dtprintinfo_cat = catopen("dtprintinfo.cat", NL_CAT_LOCALE);
#else
   dtprintinfo_cat = catopen("dtprintinfo.cat", 0);
#endif

   if (getenv("TEST_PRINTER_STATUS_DIR"))
    {
      // The TEST_PRINTER_STATUS_DIR environment variable is used during testing
      PRINTER_STATUS_DIR = getenv("TEST_PRINTER_STATUS_DIR");
    }
   else if (getuid())
    {
      fprintf(stderr, "%s: You must be root to run this program\n", argv[0]);
      exit(1);
    }
   else
    {
      PRINTER_STATUS_DIR = (char *)DEFAULT_PRINTER_STATUS_DIR;
#ifdef aix
      close(0);
      close(1);
      close(2);
      FILE *fp = fopen(CONSOLE, "r");
      fp = fopen(CONSOLE, "w");
      fp = fopen(CONSOLE, "w");
#endif
    }

   CheckDir();
   if (argc == 2)
    {
      if (!strncmp(argv[1], "-i", 2) && strlen(argv[1]) > 2)
       {
	 if ((interval = atoi(argv[1] + 2)) <= 0)
	    interval = (int)INTERVAL;
       }
      else
         interval = (int)INTERVAL;
    }
   else if (argc == 3 && !strcmp(argv[1], "-i"))
    {
      if ((interval = atoi(argv[2])) <= 0)
         interval = (int)INTERVAL;
    }
   else
      interval = (int)INTERVAL;

   while (1)
      CheckQueues((time_t)interval);
}


/////////////////////////////  Lpstat parser /////////////////////////////////

/*
The parser needs to parse at least these 4 lpstat formats.

1. lpstat example for AIX

Queue   Dev   Status    Job Files              User         PP %   Blks  Cp Rnk
------- ----- --------- --- ------------------ ---------- ---- -- ----- --- ---
john1   lp0   DOWN     
              QUEUED     71 /.profile          beer                   2   1   1
              QUEUED     84 STDIN.30583        beer                   1   1   3
              QUEUED     85 My Report          beer                   1   1   4


2. lpstat example for HP

testprinter-84      root           priority 7  Jan 15 01:13 from classy
        motd                                        55 bytes
testprinter-9       root           priority 7  Jan 15 03:12
        (standard input)                            15 bytes


3. lpstat example for SUN

psnull-80               root                55   Jul 26 21:27
psnull-955              warpspeed!root    1303   Jul 30 21:34


4. lpstat example for BSD

Warning: psnull is down: stopped by user
Warning: psnull queue is turned off: 
Rank   Owner      Job             Files                       Total Size
active root       37              /etc/inetd.conf             4722 bytes
1st    root       74              standard input              55 bytes
2nd    root       36              /etc/services               1962 bytes
3rd    root       35              /etc/motd                   55 bytes
4th    root       72              /etc/motd                   55 bytes
5th    root       73              standard input              55 bytes
6th    root       75              <File name not available>   55 bytes

*/

static boolean ParseLpstat(const char *printer, StatusLineList *job_list,
                           int *n_jobs)
{
   char buf[100];
#ifdef sun
   sprintf(buf, "lpstat -o %s", printer);
#else
   sprintf(buf, "lpstat -o%s", printer);
#endif
   boolean get_local = true;
   char *output;
   Invoke *_thread = new Invoke(buf, &output);
   if (_thread->status)
    {
      *job_list = NULL;
      *n_jobs = 0;
    }
   else
    {
      *job_list = NULL;
      *n_jobs = 0;
    }
   delete _thread;
   delete output;
   return get_local;
}
