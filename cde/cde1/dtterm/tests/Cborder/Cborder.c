/* $XConsortium: Cborder.c /main/cde1_maint/1 1995/07/18 01:42:01 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];


void TestBorder(WinName, String, TestNum)
char *WinName, *String;
int TestNum;
{
  char Str[IMAGE_FILE_LEN];
     PrintTermString(WinName, String);
     sprintf(Str, "%sborder%d", IMAGE_DIR, TestNum);
     MatchWindows(WinName, Str);
     CloseTerm(WinName);
}

static char *OptionArr[] = {
/*0*/ " -bd red -bw 10",
/*1*/ " -bd green -bw 6",
/*2*/ " -bd blue -bw 12",
/*3*/ " -bd yellow -bw 15",
/*4*/ " -bordercolor  cyan -borderwidth 7",
/*5*/ "  -bordercolor  brown -borderwidth 8",
/*6*/ " -w 10"
};

static char *LogAction[] = {
/*0*/ "Testing option -bd red -bw 10",
/*1*/ "Testing option -bd green -bw 6",
/*2*/ "Testing option -bd blue -bw 12",
/*3*/ "Testing option -bd yellow -bw 15",
/*4*/ "Testing option -bordercolor  cyan -borderwidth 7",
/*5*/ "Testing option -bordercolor  brown -borderwidth 8",
/*6*/ "Testing option -w 10"
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
       TestBorder("TermWin", OptionArr[i], i+1);
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

