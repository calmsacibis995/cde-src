/*
** Global Includes
*/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/file.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#include <assert.h>
#include <stropts.h>
#include <poll.h>
#include <errno.h>

#include <PortObjs/DataPort.hh>
#include <SDtMail/Connection.hh>
#include <SDtMail/Session.hh>

#include <ptcresult.hh>

/*
** Global Defines
*/

#define ON		1
#define OFF		0
#define TRUE		1
#define FALSE		0

/*
** Global Structs
*/

/* CMD - Interactive command handler dispatch table format
*/

typedef struct CMD_T
    {
    char *c_name;		/* command name */
    char *c_help;		/* help message */
    int (*c_handler)(int margc, char **margv);		/* routine to do the work */
    int c_nocmdline;		/* command cannot be given from command line */
    int c_nopts;		/* command does NOT accept arguments */
    } CMD;
#define NULLCMD (CMD *)0

/* SUBCMD - sub-command handler dispatch table format
*/

typedef struct SUBCMD_T
    {
    char *c_cmdname;		/* command name */
    int (*c_subhandler)(int margc, char **margv);		/* routine to do the work */
    int c_argsrequired;		/* minimum number of arguments required */
    } SUBCMD;
#define NULLSUBCMD (SUBCMD *)0

extern int isatty0;
extern int isatty1;
extern int vflag;
extern int UserInterrupt;
extern char *Command;
extern char *Prompt;
extern struct pollfd *pollFileDesc;		// three possible descriptors: stdin, isession, osession
extern int	numPollFileDescEntries;
extern int theInputFileDesc;
extern int theOutputFileDesc;
extern int theTooltalkFileDesc;
extern void *theInputQueue;
extern void *theOutputQueue;
extern PtResult ptLastResult;

int main(int pargc, char **pargv);
void TakeInterrupt();
void AllowInterrupts(int pok);
int getnum(char *string,int minval,int maxval);
int extractNumber(char *string,int minval,int maxval,int printMessage);
void addPollFileDesc(int fd, short event);
void removePollFileDesc(int fd);
void initPollFileDesc();
void HexDump(FILE *pfp, const char *pmsg, const unsigned char *pbufr, const int plen, const int plimit, const int pcontents);
void computeSum(SdmString data, unsigned& first, long& second);
