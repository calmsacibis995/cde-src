/* $XConsortium: Rpointer.c /main/cde1_maint/1 1995/07/18 01:45:02 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"
#include  "common.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];


#define BLANK    ' '

void TestPointer(WinName, String, TestNum, Delay)
char *WinName, *String;
int TestNum, Delay;
{
  char Str[IMAGE_FILE_LEN];
     PrintTermString(WinName, String);
     sleep(Delay+1);
     sprintf(Str, "%srpointer%d", IMAGE_DIR, TestNum);
     MatchWindows(WinName, Str);
     CloseTerm(WinName);
}

static char *ResourceArr[] = {
/*0*/ "pointerBlank True pointerBlankDelay 5 pointerColor red pointerColorBackground black pointerShape hand1",
/*1*/ "pointerBlank False pointerBlankDelay 5 pointerColor blue pointerColorBackground greem pointerShape hand2"
};

static char *LogAction[] = {
"Testing Resource Option: pointerBlank True pointerBlankDelay 5 pointerColor red pointerColorBackground black pointerShape hand1",
"Testing Resource Option: pointerBlank False pointerBlankDelay 5 pointerColor blue pointerColorBackground greem pointerShape hand2"
};

#define ArrCount    (int) (sizeof(ResourceArr) / sizeof(char *))

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
fprintf(TermLog, "**************************************************\n");
LogTime();
fprintf(TermLog, "TestName: <%s> STARTS\n", argv[0]);
#endif 
     InitTest(argc, argv);
     for (i=0; i < ArrCount; i++) {
       CheckCapsLock();
       ExecCommand("xrdb -load xrdb.out");
       MakeResourceFile(ResourceArr[i]);
       CheckCapsLock();
       LogError(LogAction[i]);
       if (WaitWinMap("TermWin") < 0) continue;
       strcpy(Command, "xrdb -merge res; ");
       strcat(Command, TERM_EMU);
       ExecCommand(Command);
       AssignWinName("TermWin", TERM_EMU);
       TestPointer("TermWin", "Checking Pointer Resources ", i+1, 5);
       WaitWinUnMap("TermWin", 60L);
     }
     CheckCapsLock();
     ExecCommand("xrdb -load xrdb.out");
     CloseTest(False);
#ifdef LOG
fprintf(TermLog, "TestName: <%s> ENDS\n", argv[0]);
fclose(TermLog);
#endif
}

