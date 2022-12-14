// User Interface headers
#include "DtApp.h"
#include "DtPrinterIcon.h"
#include "Invoke.h"

#ifndef NO_CDE
extern "C" {
  #include <Dt/EnvControl.h>
}
#endif


// Object headers
#include "PrintSubSys.h"

// Message header
#include "dtprintinfomsg.h"
nl_catd dtprintinfo_cat = NULL;

#include <stdlib.h> // This is for the getenv function
#include <unistd.h> // This is for the getuid function
#include <string.h> 
#include <errno.h> 

#if defined(aix) || defined(USL) || defined(__uxp__)
extern "C" { extern int seteuid(uid_t); }
#endif

#ifdef hpux
static char **msg_strings = NULL;

// Cannot use multiple catgets paramemter calls in functions because the 
// previous catgets returned value is overwritten by later catgets calls
// Example: this would fail on HP systems
//    sprintf(buf, "%s %s" catgets(...), catgets(...))

char *Catgets(nl_catd catd, int set_num, int msg_num, char *s)
{
   if (!msg_strings)
      return s;

   if (!msg_strings[msg_num])
      msg_strings[msg_num] = strdup(catgets(catd, set_num, msg_num, s));
   return msg_strings[msg_num];
}
#endif

#ifdef __osf__
extern "C" { extern int svc_init(); }
#endif

int main(int argc, char **argv)
{

#ifdef __osf__
    svc_init();
#endif
  
#ifndef NO_CDE
   _DtEnvControl(DT_ENV_SET);
#endif

// run as user's UID
#ifdef hpux
   setresuid(getuid(), getuid(), (uid_t)0);
#else
   seteuid(getuid());
#endif

   setlocale(LC_ALL, "");

   char *lang = getenv("LANG");
   if (lang && strcmp(lang, "C"))
    {
      errno = 0;

#ifdef NL_CAT_LOCALE
      dtprintinfo_cat = catopen("dtprintinfo", NL_CAT_LOCALE);
#else
      dtprintinfo_cat = catopen("dtprintinfo", 0);
#endif

      if ((nl_catd) errno || dtprintinfo_cat == (nl_catd) -1)
         dtprintinfo_cat = NULL;
#ifdef hpux
      else
         msg_strings = (char **)calloc(LAST_MSG_NO, sizeof(char *));
#endif
    }

   if (argc > 1) {
   if (((!STRCMP(argv[1], "-help")) || (!STRCMP(argv[1], "-h")))
      || ((!STRCMP(argv[1], "-p")) && (argv[2] == NULL)))
    {
      char *output;
      char *cmd = new char [strlen(LIST_QUEUES) + 30];
      sprintf(cmd, "%s | awk '{print \"\\t\", $1}'", LIST_QUEUES);
      Invoke *_thread = new Invoke(cmd, &output);
      printf(MESSAGE(CommandLineHelpL), output);
      printf("\n");
      delete output;
      delete cmd;
      delete _thread;
      return 0;
    }
    }

   char *progname = strrchr(argv[0], '/');
   if (progname)
      progname++;
   else
      progname = argv[0];
   if (!STRCMP(argv[1], "-populate"))
    {
      if (getuid() != 0)
       {
	 fprintf(stderr, MESSAGE(RootUserL), progname, "-populate");
	 fprintf(stderr, "\n");
	 return 1;
       }

      PrintSubSystem *prt = new PrintSubSystem(NULL);
      int n_queues = prt->NumChildren();
      // Get Print Subsystem children, (these are queues)
      Queue **queues = (Queue **)prt->Children();
      int i;
      for (i = 0; i < n_queues; i++)
       {
         DtPrinterIcon *icon = new DtPrinterIcon(NULL, NULL, queues[i],
						 INITIALIZE_PRINTERS);
	 icon->CreateActionFile();
	 delete icon;
       }
      return 0;
    }

   DtApp *app = new DtApp(progname, &argc, argv);
   app->Visible(true);
   app->Run();

   return 0;
}
