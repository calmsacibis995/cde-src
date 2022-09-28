/* $XConsortium: Rsavelines.c /main/cde1_maint/1 1995/07/18 01:45:15 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"
#include  "common.h"

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
    sprintf(Str, "%srsavelines%d", IMAGE_DIR, TestNum);
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
/*1*/ "Testing Resource Option: saveLines 24l",
/*2*/ "Testing Resource Option: saveLines 48l",
/*3*/ "Testing Resource Option: saveLines 96l",
/*4*/ "Testing Resource Option: saveLines 1s",
/*5*/ "Testing Resource Option: saveLines 2s",
/*6*/ "Testing Resource Option: saveLines 4s"
};

static char *ResourceArr[] = {
/*1*/ "saveLines 24l",
/*2*/ "saveLines 48l",
/*3*/ "saveLines 96l",
/*4*/ "saveLines 1s",
/*5*/ "saveLines 2s",
/*6*/ "saveLines 4s"
};

int MakeResourceFile(ResArr)
char *ResArr;
{
  FILE *ResFile;
  int i, j, Len;
  char Resource[50], Value[50];
    
    if ((ResFile = fopen("res", "w")) == NULL)
       {LogError("Resource File Creation fail"); return(-1);}
    for (i=0; i < 50; i++) Resource[i] = BLANK;
    for (i=0; i < 50; i++) Value[i] = BLANK;
    Len = strlen(ResArr); i=0;
    while (i < Len) {
      j = 0;
      while (ResArr[i] != BLANK) Resource[j++] = ResArr[i++];
      Resource[j] = NULLCHAR;
      i++; j = 0;
      while ((ResArr[i] != BLANK) && (ResArr[i] != NULLCHAR))
            Value[j++] = ResArr[i++];
      Value[j] = NULLCHAR;
      i++;
      fprintf(ResFile, "%s*%s: %s\n", TERM_EMU, Resource, Value);
    }
    fclose(ResFile);
}


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
       CheckCapsLock();
       ExecCommand("xrdb -load xrdb.out");
       MakeResourceFile(ResourceArr[i]);
       LogError(LogAction[i]);
       strcpy(Command, "xrdb -merge res; ");
       strcat(Command, TERM_EMU);
       CheckCapsLock();
       ExecCommand(Command);
       if (WaitWinMap("TermWin") < 0) continue;
       AssignWinName("TermWin", TERM_EMU);
       TestSavelines("TermWin", i+1);
       WaitWinUnMap("TermWin", 60L);
     }
     CheckCapsLock();
     ExecCommand("xrdb -load xrdb.out");
     CloseTest(False);
#ifdef LOG
sprintf(Command, "TestName: <%s> ENDS\n", argv[0]);
LogError(Command);
LogError("****************************************************************************");
fclose(TermLog);
#endif
}

