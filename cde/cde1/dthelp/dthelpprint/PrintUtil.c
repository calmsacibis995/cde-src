#if DOC
/*===================================================================
$FILEBEG$:   PrintUtil.c
$COMPONENT$: dthelpprint
$PROJECT$:   Cde1
$SYSTEM$:    HPUX 9.0; AIX 3.2; SunOS 5.3
$REVISION$:  $XConsortium: PrintUtil.c /main/cde1_maint/2 1995/10/08 23:59:58 pascale $
$CHGLOG$:    
$COPYRIGHT$:
   (c) Copyright 1993, 1994 Hewlett-Packard Company
   (c) Copyright 1993, 1994 International Business Machines Corp.
   (c) Copyright 1993, 1994 Sun Microsystems, Inc.
   (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
==$END$==============================================================*/
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <nl_types.h>  /* for message cat processing */
#if defined(sun)
#include <locale.h>
#else
#include <langinfo.h>
#endif

#include "HelpPrintI.h"

/*======== flexible constsnts ==============*/
/* message catalog file */
#define HELPPRINT_CAT_WITH_SUFFIX	"dthelpprint.cat"
#define HELPPRINT_CAT			"dthelpprint"

/*======== dthelpprint.sh options ==============*/
#define OPT_LPDEST		"-d"
#define OPT_COMMAND		"-m"
#define OPT_COPYCOUNT		"-n"
#define OPT_USERFILE		"-u"
#define OPT_FILE		"-f"
#define OPT_SILENT		"-s"
#define OPT_FILEREMOVE		"-e"
#define OPT_RAW			"-w"

/*======== helper values ===============*/
#define EOS           '\0'

/*======== helper variables ===============*/

/* To do:
	* check roman 8/Latin 1
	* check PAGER env variable
	* do character wrap
*/

/*======== data structs ==============*/

/*======== static variables ===============*/

/*======== functions ==============*/
#if DOC
===================================================================
$PFUNBEG$:  PutOpt()
$1LINER$:  Concats option and value strings into cmd str
$DESCRIPT$:
Concats option and value strings into cmd str
$RETURNS$:
$ARGS$:
========================================================$SKIP$=====*/
#endif /*DOC*/

#if _NO_PROTO
static
void PutOpt(cmdStr,option,value,optionHasValue)
      char * cmdStr,
      char * option;
      char * value;
      Boolean optionHasValue;
#else   /*$DEF$*/
static
void PutOpt(
      char * cmdStr,
      char * option,
      char * value,
      Boolean optionHasValue)
#endif  /*$SKIP$*/
{       /*$CODE$*/
   char * start;
   char * fmt;
   
   /* check params */
   if (    option == NULL 
        || option[0] == EOS 
        || (   optionHasValue == True 
            && (value == NULL || value[0] == EOS) ) )
       return;						/* RETURN */
   
   start = &cmdStr[strlen(cmdStr)];
   if ( value == NULL ) fmt = " %s";
   else fmt = " %s '%s'";
   sprintf(start,fmt,option,value);
} /*$END$*/


#if DOC
===================================================================
$FUNBEG$:  _DtHPrGetPrOffsetArg()
$1LINER$:  Builds the pr offset argument string, if needed
$DESCRIPT$:
Concats option and value strings into cmd str
$RETURNS$:
$ARGS$:
========================================================$SKIP$=====*/
#endif /*DOC*/

#if _NO_PROTO
void _DtHPrGetPrOffsetArg(options,argStr)
   _DtHPrOptions * options;
   char *          argStr;
#else   /*$DEF$*/
void _DtHPrGetPrOffsetArg(
   _DtHPrOptions * options,
   char *          argStr)
#endif  /*$SKIP$*/
{       /*$CODE$*/
   if ( options->outputFile && options->outputFile[0] != EOS )
      argStr[0] = EOS;
   else
      sprintf(argStr,options->prOffsetArg, options->colsAdjLeftMargin);
} /*$END$*/


#if DOC
===================================================================
$FUNBEG$:  _DtHPrGenFileOrPrint()
$1LINER$:  Executes print Command to generate file; prints if needed
$DESCRIPT$:
Executes the printCommand that is passed in to generate 
either the desired output file or to print the results of
the command.  If printing the results, the results are first
put in a temporary file, which is then printed indirectly
by invoking a shell script that should print the file.  The
temp file is deleted after the shell script executes.
$RETURNS$:
If generating a file:  result of system(printCommand)
If printing a file:  result of system(printCommand) if fails
                     result of system("sh -c <shellCommand>") otherwise
$ARGS$:
printCommand: should pt to a very large (e.g. >5000 char) string
              that can be modified by this routine
========================================================$SKIP$=====*/
#endif /*DOC*/

#if _NO_PROTO
int _DtHPrGenFileOrPrint(options,userfile,printCommand)
   _DtHPrOptions * options;
   char *          userfile;
   char *          printCommand;
#else   /*$DEF$*/
int _DtHPrGenFileOrPrint(
   _DtHPrOptions * options,
   char *          userfile,
   char *          printCommand)
#endif  /*$SKIP$*/
{       /*$CODE$*/
   int    status;
   char * tmpfile;
   char   cmdFormat[30];
   
   /* put into specified output file?? */
   if (options->outputFile[0] != EOS) 
   { 
      strcat(printCommand," ");
      sprintf(&printCommand[strlen(printCommand)],
                      options->redirectCmdAndArgs,
                      options->outputFile );     /* file */
      if(options->debugHelpPrint) printf("%s\n",printCommand);
      return (system(printCommand));           /* RETURN */
   }

   /* put into private tmp file */
   strcat(printCommand," ");
   tmpfile = _DtHPrCreateTmpFile(TMPFILE_PREFIX,TMPFILE_SUFFIX);
   sprintf(&printCommand[strlen(printCommand)],
                      options->redirectCmdAndArgs,
                      tmpfile );                          /* file */

   if(options->debugHelpPrint) 
      printf("%s\n",printCommand);

   strcat(printCommand,"\n");
   if ( (status = system(printCommand))!= 0)
   {
      unlink(tmpfile);
      return status;                           /* RETURN */
   }

   /* put the shell print script in there */
   sprintf(printCommand,"%s", options->shCommand,True);

   /* set all the options that are IPC to the print script */
   PutOpt(printCommand,OPT_LPDEST,options->printer,True);
   PutOpt(printCommand,OPT_COMMAND,options->lpCommand,True);
   PutOpt(printCommand,OPT_COPYCOUNT,options->copies,True);
   PutOpt(printCommand,OPT_SILENT,NULL,False);
   PutOpt(printCommand,OPT_FILEREMOVE,NULL,False);
   PutOpt(printCommand,OPT_FILE,tmpfile,True);
   PutOpt(printCommand,OPT_USERFILE,userfile,True);
 
   /* execute the shell command to cause printing */
   if(options->debugHelpPrint) printf("%s\n",printCommand);
   status = system(printCommand);

   /* unlink(tmpfile);  /* NOTE: don't unlink; let the printCommand do it */
                        /* note the DTPRINTFILEREMOVE env var setting above */
      
   return(status);
} /*$END$*/




#ifdef MESSAGE_CAT
#if DOC
===================================================================
$FUNBEG$:  _DtHPrGetMessage()
$1LINER$:  Gets a message string from the msg cat; uses dflt if no msg
$DESCRIPT$:
Gets a message string from the msg cat; uses dflt if no message defined
or LANG is undefined or "C".
$RETURNS$:
$ARGS$:
========================================================$SKIP$=====*/
#endif /*DOC*/

#if _NO_PROTO
char * _DtHPrGetMessage(set,n,s)
        int set;
        int n;
        char *s;
#else   /*$DEF$*/
char * _DtHPrGetMessage(
        int set,
        int n,
        char *s)
#endif  /*$SKIP$*/
{       /*$CODE$*/
   char *msg;
   char *lang;
   nl_catd catopen();
   char *catgets();
   static int s_First = 1;
   static nl_catd s_Nlmsg_fd;
   static char * s_CatFileName = NULL;

   if ( s_First ) 
   {
      /* Setup our default message catalog names if none have been set! */
      if (s_CatFileName  == NULL)
      {
         /* Setup the short and long versions */
#ifdef __ultrix
         s_CatFileName = strdup(HELPPRINT_CAT_WITH_SUFFIX);
#else 
         s_CatFileName = strdup(HELPPRINT_CAT);
#endif
      }
      s_First = 0;

      lang = (char *) getenv ("LANG");

      /* If LANG is not set or if LANG=C, then there
       * is no need to open the message catalog - just
       * return the built-in string "s".  */
      if (!lang || !(strcmp (lang, "C"))) 
         s_Nlmsg_fd = (nl_catd) -1;
      else
         s_Nlmsg_fd = catopen(s_CatFileName, 0);
   }  /* end of first-time processing */

   msg = catgets(s_Nlmsg_fd,set,n,s);
   return (msg);

}
#endif  /* #ifdef MESSAGE_CAT */



