/* $XConsortium: Rborder.c /main/cde1_maint/1 1995/07/18 01:43:58 drk $ */
#include <stdio.h>
#include <math.h>
#include  "synvar.h"
#include  "common.h"

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
     sprintf(Str, "%srborder%d", IMAGE_DIR, TestNum);
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

static char *ResourceArr[] = {
/*0*/ "borderColor red borderWidth 10 ", 
/*1*/ "borderColor green borderWidth 6 ", 
/*2*/ "borderColor blue borderWidth 12 ", 
/*3*/ "borderColor yellow borderWidth 15 ", 
/*4*/ "borderColor cyan borderWidth 7 ", 
/*5*/ "borderColor brown borderWidth 8 ", 
/*6*/ "borderWidth 10 ", 
};

static char *LogAction[] = {
/*0*/ "Testing Resourse Option:borderColor red borderWidth 10 ", 
/*1*/ "Testing Resourse Option:borderColor green borderWidth 6 ", 
/*2*/ "Testing Resourse Option:borderColor blue borderWidth 12 ", 
/*3*/ "Testing Resourse Option:borderColor yellow borderWidth 15 ", 
/*4*/ "Testing Resourse Option:borderColor cyan borderWidth 7 ", 
/*5*/ "Testing Resourse Option:borderColor brown borderWidth 8 ", 
/*6*/ "Testing Resourse Option:borderWidth 10 ", 
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
       CheckCapsLock();
       strcpy(Command, "xrdb -merge res; ");
       strcat(Command, TERM_EMU);
       LogError(LogAction[i]);
       ExecCommand(Command);
       if (WaitWinMap("TermWin") < 0) continue;
       AssignWinName("TermWin", TERM_EMU);
       TestBorder("TermWin", OptionArr[i], i+1);
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

