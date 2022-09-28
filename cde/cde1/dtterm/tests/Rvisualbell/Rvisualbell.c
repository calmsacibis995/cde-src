/* $XConsortium: Rvisualbell.c /main/cde1_maint/1 1995/07/18 01:45:54 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"
#include  "common.h"

#ifdef LOG
FILE *TermLog;
#define SAVELOG  fclose(TermLog); TermLog = fopen("term.log", "a");
#endif

char LogStr[200];


void TestVisualbell(WinName, TestNum)
char *WinName;
int TestNum;
{
  char Str[IMAGE_FILE_LEN];
  int i;
    for (i=0; i < 4; i++) 
      PressCtrlGKey(WinName);
    if ((TestNum % 2) == 0)
       PrintTermString(WinName, "You should have heard beep");
    else 
       PrintTermString(WinName, "You should have seen screen flashing");
sleep(5);
    CloseTerm(WinName);
}

static char *OptionArr[] = { 
/*0*/ " -vb ",
/*1*/ " -xrm 'visualBell: False' -vb",
/*2*/ " +vb ",
/*3*/ " -xrm 'visualBell: True' +vb"
};

static char *LogAction[] = { 
/*0*/ " Testing Option: -vb ",
/*1*/ " Testing Option: -xrm 'visualBell: False' -vb",
/*2*/ " Testing Option: +vb ",
/*3*/ " Testing Option: -xrm 'visualBell: True' +vb"
};

static char *ResourceArr[] = {
/*0*/ "visualBell True",
/*1*/ "visualBell False ",
/*2*/ "visualBell on ",
/*3*/ "visualBell off "
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


#define ArrCount    (int) (sizeof(ResourceArr) / sizeof(char *))

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
       TestVisualbell("TermWin", i+1);
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

