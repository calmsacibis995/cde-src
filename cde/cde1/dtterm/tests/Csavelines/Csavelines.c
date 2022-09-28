/* $XConsortium: Csavelines.c /main/cde1_maint/1 1995/07/18 01:43:05 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];


void TestSavelines(WinName, TestNum)
char *WinName;
int TestNum;
{
  char Str[IMAGE_FILE_LEN];
  int i;
    ExecWinCommand(WinName, "printlines");
sleep(2);
    for (i=0; i < 4; i++) PressPrevKey(WinName);
sleep(2);
    sprintf(Str, "%ssavelines%d", IMAGE_DIR, TestNum);
    MatchWindows(WinName, Str);
sleep(2);
    CloseTerm(WinName);
}

static char *OptionArr[] = { 
/*1*/ " -sl 24l",
/*2*/ " -sl 48l",
/*3*/ " -sl 96l",
/*4*/ " -sl 1s",
/*5*/ " -sl 2s",
/*6*/ " -sl 4s"
};

static char *LogAction[] = { 
/*1*/ "Testing Option  -sl 24l",
/*2*/ "Testing Option -sl 48l",
/*3*/ "Testing Option -sl 96l",
/*4*/ "Testing Option  -sl 1s",
/*5*/ "Testing Option  -sl 2s",
/*6*/ "Testing Option  -sl 4s"
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
       TestSavelines("TermWin", i+1);
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

