/* $XConsortium: Clogging.c /main/cde1_maint/1 1995/07/18 01:42:40 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];



static char *OptionArr[] = { 
/*0*/ " -l ",                                /* Normal option */
/*1*/ " -xrm 'dtterm*logging: off'  -l ",   /* By resource log off*/
/*2*/ " -l ",                                /* try in un writable directory */
/*3*/ " -xrm 'dtterm*logging: on'  -xrm 'dtterm*logFile: Term.Log ' +l ",
/*4*/ " -lf Term.Log"
};


static char *LogAction[] = { 
/*0*/ "Testing option: -l ",                                
/*1*/ "Testing option -xrm 'dtterm*logging: off'  -l ",   
/*2*/ "Testing option -l in an unwritable directory", 
/*3*/ "Testing option -xrm 'dtterm*logging: on'  -xrm 'dtterm*logFile: Term.Log' +l",
/*4*/ "Testing option -lf Term.Log"
};

#define ArrCount    (int) (sizeof(OptionArr) / sizeof(char *))

main(argc, argv)
int argc;
char *argv[];
{
   SynStatus Result;
   char Command[NEED_LEN];
   int i;
     
#ifdef LOG
if ((TermLog = fopen("term.log", "a")) == NULL) {
   if ((TermLog = fopen("term.log", "w")) == NULL)
      {printf("Logfile could not be opened \n"); exit(-1);}
}
LogError("****************************************************************************");
LogTime();
sprintf(Command, "TestName: <%s> STARTS\n", argv[0]);
LogError(Command);
#endif 
     InitTest(argc, argv);
     CheckCapsLock();
     ExecCommand("mkdir tmplog");
     for (i=0; i < ArrCount; i++) {
       strcpy(Command, TERM_EMU);  
       strcat(Command, OptionArr[i]);
       CheckCapsLock();
       ExecCommand("mv  DTtermLog* tmplog"); sleep(1);
       ExecCommand("mv Term.Log tmplog"); sleep(1);
       if (i == 2) { /* try in un writable directory */
          ExecCommand("mkdir unwrite"); sleep(1);
          ExecCommand("chmod 555 unwrite"); sleep(1);
          ExecCommand("cd  unwrite"); sleep(1);
       }
       LogError(LogAction[i]);
       ExecCommand(Command); sleep(1);
       if (WaitWinMap("TermWin") < 0) continue;
       AssignWinName("TermWin", TERM_EMU);
       JustExit("TermWin");
       WaitWinUnMap("TermWin", 10L);
       switch (i) {
         case 0: 
           ExecCommand("test-l"); break;
         case 1: 
           ExecCommand("test-l"); break;
         case 2: 
           ExecCommand("test+l"); break;
         case 3: ExecCommand("test+l"); break;
         case 4: ExecCommand("test-lf"); break;
       }
       if (i == 2) {
          ExecCommand("cd ..");
          ExecCommand("rmdir unwrite");
       }
sleep(2);
     }
     ExecCommand("mv  tmplog/DTtermLog* ."); sleep(1);
     ExecCommand("mv tmplog/Term.Log ."); sleep(1);
     ExecCommand("rmdir tmplog");
     CloseTest(False);
#ifdef LOG
sprintf(Command, "TestName: <%s> ENDS\n", argv[0]);
LogError(Command);
LogError("****************************************************************************");
fclose(TermLog);
#endif
}

