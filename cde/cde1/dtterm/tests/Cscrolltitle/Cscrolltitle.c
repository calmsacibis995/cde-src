/* $XConsortium: Cscrolltitle.c /main/cde1_maint/1 1995/07/18 01:43:18 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];


void TestScrollbar(WinName, String, TestNum)
char *WinName, *String;
int TestNum;
{
  char Str[IMAGE_FILE_LEN];
     PrintTermString(WinName, String);
     sprintf(Str, "%sscrollbar%d", IMAGE_DIR, TestNum);
     MatchWindows(WinName, Str);
     CloseTerm(WinName);
}

static char *OptionArr[] = { 
/*0*/ " -sb -title TITLE1",
/*1*/ " +sb -title title2",
/*2*/ " -xrm 'dtterm*scrollBar: False' -sb -title TITLE3",
/*3*/ " -xrm 'dtterm*scrollBar: True' +sb -title title4"
};

static char *LogAction[] = { 
/*0*/ "Testing Option -sb ",
/*1*/ "Testing Option +sb ",
/*2*/ "Testing Option -xrm 'dtterm*scrollBar: False' -sb",
/*3*/ "Testing Option -xrm 'dtterm*scrollBar: True' +sb"
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
     for (i=0; i < ArrCount; i++) {
       strcpy(Command, TERM_EMU);  
       strcat(Command, OptionArr[i]);
       CheckCapsLock();
       LogError(LogAction[i]);
       ExecCommand(Command);
       if (WaitWinMap("TermWin") < 0) continue;
       AssignWinName("TermWin", TERM_EMU);
       TestScrollbar("TermWin", OptionArr[i], i+1);
       WaitWinUnMap("TermWin", 60L);
     }
     CloseTest(False);
#ifdef LOG
sprintf(Command, "TestName: <%s> ENDS\n", argv[0]);
LogError(Command);
LogError("****************************************************************************");
fclose(TermLog);
#endif
}

