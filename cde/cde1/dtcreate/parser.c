/* $XConsortium: parser.c /main/cde1_maint/3 1995/11/27 18:01:28 gtsang $ */
/***************************************************************************/
/*                                                                         */
/*  parser.c                                                               */
/*                                                                         */
/***************************************************************************/
#ifdef _AIX
#define _ILS_MACRO
#endif /* _AIX */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "dtcreate.h"
#include "parser.h"
#ifdef AIX_ILS
#include <ctype.h>
#endif /* AIX_ILS */

/***************************************************************************
 *
 *  Local procedure declarations
 *
 ***************************************************************************/

#ifdef _NO_PROTO

char ** GetKeywordValuePairs();
FiletypeData ** GetFiletypeData();
char ** ProcessExecString();
int ProcessContents();
Boolean IsLastSingle();

#else

char ** GetKeywordValuePairs(char *, int *, int);
FiletypeData ** GetFiletypeData(FILE *, char *, short *);
char ** ProcessExecString(char *);
int ProcessContents(FiletypeData *);
Boolean IsLastSingle(char *);

#endif

/***************************************************************************
 *
 *  Extern variable declarations
 *
 ***************************************************************************/


/***************************************************************************
 *
 *  Global variable declarations
 *
 ***************************************************************************/

/* Action Keyword Table */
char keywordDB[][30] = { "ACTION"         ,
                          "ICON"          ,
                          "EXEC_STRING"   ,
                          "DESCRIPTION"   ,
                          "TYPE"          ,
                          "WINDOW_TYPE"   ,
                          "ARG_TYPE"      ,
                          "LABEL"         ,
                          NULL            ,
                     };

/* Max Number of fields in Action Keyword Table */
#define MAX_NUM_ACTION_FIELDS     8

/* Filetype Keyword Table */
char FiletypekeywordDB[][30] = { "DATA_ATTRIBUTES"       ,
                                  "ICON"                 ,
                                  "DESCRIPTION"          ,
                                  "EXEC_STRING"          ,
                                  "NAME_PATTERN"         ,
                                  "PATH_PATTERN"         ,
                                  "MODE"                 ,
                                  "CONTENT"              ,
                                  "ACTIONS"              ,
                                  "DATA_ATTRIBUTES_NAME" ,
                                  "DATA_CRITERIA"        ,
                                  "WINDOW_TYPE"          ,
                                  "ACTION"               ,
                                  "ARG_TYPE"             ,
                                  "MAP_ACTION"           ,
                                  "TYPE"                 ,
                                  "LABEL"                ,
                                   NULL                  ,
                     };

/* Max Number of fields in Filetype Keyword Table */
#define MAX_NUM_FILETYPE_FIELDS     17
#ifdef NOT_ILS
#define MAX_NUM_FILETYPE_FIELDS     16
#endif


char *args[3],*fline;
int flinesize=0;
#ifdef NOT_ILS
static int mbcodeset=-1;
#endif
static int state = 0;


/*****************************************************************
**                                                              **
** GetActionData(FILE *fp, ActionData *)                        **
**                                                              **
** Description: Parses the action file and fills the data in the**
**              pointer pointing to the ActionData structure    **
**              passed as input.                                **
**                                                              **
** Input      : Filepointer that points to the ActionFile and,  **
**              a pointer to the ActionData structure.          **
**                                                              **
** Output     : 0 (No error).                                   **
**              1 (error).                                      **
**                                                              **
** references : dtcreate.h, parser.h, and dtcreate spec.        **
**                                                              **
******************************************************************/


int
#ifdef _NO_PROTO
GetActionData(fp,ActionDataptr)
        FILE   *fp;
        ActionData *ActionDataptr;
#else
GetActionData(FILE *fp, ActionData *ActionDataptr)
#endif
{
int         rc,manflds=0,len,first=TRUE,lastfld=0,fldid=-1;
char        linebuf[1024],**wordPairs,**execstr;


        /* reset the read pointer to zero byte for fp */
        rewind(fp);
        /* Initialize the ActionData structure passed */
        if(ActionDataptr)
           memset((ActionData *)ActionDataptr,
                       NULL,sizeof(ActionData));
        else {
#ifdef DEBUG
           printf("ActionDataptr is NULL\n");
#endif
           return 1;
        }

        /* initialize the linebuf with NULL's  */
        bzero(linebuf,sizeof(linebuf));
        while (fgets (linebuf, sizeof (linebuf)-1, fp))
        {
           /* skip the complete line if it starts with a '{' or a comment ('#')  */
           if(linebuf[0] == '{' || linebuf[0] == '#') continue;
           /* If the first character is '}' reached the end of Action Stanza so quit */
           if(linebuf[0] == '}')
              break;
           len = strlen (linebuf);
           if (linebuf[len-1] == '\n')
              linebuf[len-1] = '\0';
           /* Get the keyword and value pair from the string linebuf */
           /* On return, wordPairs[0] = keyword like ICON
              and        wordPairs[1] = value for the keyword
           */
           if( (wordPairs = GetKeywordValuePairs(linebuf,&fldid,ACTION_TABLE)) != NULL)
           {
              if( first && strcmp(wordPairs[0],"ACTION") )
              {
#ifdef DEBUG
                  printf("first && strcmp(wordPairs[0],ACTION)\n");
#endif
                  return 1;
              }
              else
                  first = FALSE;
              /* Update the mandatory fields counter */
              if( !strcmp(wordPairs[0],"TYPE") ||
                  !strcmp(wordPairs[0],"EXEC_STRING") ||
                  !strcmp(wordPairs[0],"WINDOW_TYPE") )
                     manflds++;
              if( !strcmp(wordPairs[0],"WINDOW_TYPE") )
              {
                 if( !strcmp(wordPairs[1],"PERM_TERMINAL") )
                    ActionDataptr->fsFlags |= CA_WT_PERMTERM;
                 if( !strcmp(wordPairs[1],"TERMINAL") )
                    ActionDataptr->fsFlags |= CA_WT_TERM;
                 if( !strcmp(wordPairs[1],"NO_STDIO") )
                    ActionDataptr->fsFlags |= CA_WT_XWINDOWS;
              }
              if( !strcmp(wordPairs[0],"ARG_TYPE") )
                 ActionDataptr->fsFlags |= CA_DF_ONLYFTFILES;

              /* if linebuf does not contain the keyword then everything
              in wordPairs[0],else, wordPairs[0] contains the keyword
              and wordpairs[1] contains the value.
              */
              if( wordPairs && wordPairs[0] && !wordPairs[1])
              {
                 /* fldid is returned by the GetKeywordValuePairs function.
                 This id determines which field of the ActionDataptr will
                 have the value. Please see KeywordDB array in parser.h
                 for more details.
                 */
                 switch(fldid)
                 {
                     case 7:   /* next line for action_name */
                               ActionDataptr->pszName=wordPairs[0];
                               break;

                     case 1:   /* next line for action_icon_name */
                               ActionDataptr->pszIcon=wordPairs[0];
                               break;

                     case 2 :  /* next line for exec_string */
                               ActionDataptr->pszCmd=wordPairs[0];
                               break;

                     case 3:   /* next line for action_help_text */
                               ActionDataptr->pszHelp=wordPairs[0];
                               break;

                    default:   break;
                 }
             }
             else if(wordPairs && wordPairs[1])
             {
                 if(fldid >= 0 && fldid <=2)
                        state = 0;
                 /* for fldid see the comments above */
                 switch(fldid)
                 {
                     case 7:   /* first line for action_name */
                               ActionDataptr->pszName=wordPairs[1];
                               break;

                     case 1:   /* first line for action_icon_name */
                               ActionDataptr->pszIcon=wordPairs[1];
                               break;

                     case 2 :  /* first line for exec_string */
                               ActionDataptr->pszCmd=wordPairs[1];
                               break;

                     case 3:   /* first line for action_help_text */
                               state = HELP_TEXT;
                               ActionDataptr->pszHelp=wordPairs[1];
                               break;

                    default:   state=0;break;
                 }

              }
              /* reset the linebuf to NULL's */
              bzero(linebuf,sizeof(linebuf));
          }
        }

        state=0;
        /* Done with retrieving Action Data */
        /* Check if we got all the mandatory fields data */
        if(manflds != 3) {
#ifdef DEBUG
            printf("if(manflds != 3)\n");
#endif
            return 1;
        } else
        {
            /* Everything looks right so process the exec_string */
            if( !(execstr = ProcessExecString(ActionDataptr->pszCmd)) )
            {
                ActionDataptr->pszCmd = NULL;
                ActionDataptr->pszPrompt = 0;
            }
            else
            {
                ActionDataptr->pszCmd = execstr[0];
                ActionDataptr->pszPrompt = execstr[1];
            }
            /* Got the ActionData,so, go get the FiletypeData */
            ActionDataptr->papFiletypes =
                        (FiletypeData **) GetFiletypeData(fp,(char *)(ActionDataptr->pszCmd),
                                                         (short *)&(ActionDataptr->cFiletypes) );
            if( !ActionDataptr->papFiletypes && ActionDataptr->cFiletypes > 0) {
#ifdef DEBUG
                 printf("!ActionDataptr->papFiletypes && ActionDataptr->cFiletypes > 0)\n");
#endif
                 return(1);  /* return 1 if error */
            } else
                 return(0);  /* return 0 if no error */
        }


}

/*****************************************************************
**                                                              **
** GetFiletypeData(FILE *fp, char *pszOpenCmd)                  **
**                                                              **
** Description: Parses the action file and returns a pointer    **
**              that points to the data in the form of          **
**              FiletypeData structure (defined in dtcreate.h). **
**                                                              **
** Input      : Filepointer that points to the ActionFile, and, **
**              a char pointer to the exec_string of the        **
**              ActionData structure as obtained in the GetAc-  **
**              -tionData function call.                        **
**                                                              **
** Output     : Pointer to an array of FiletypeData structures  **
**              containing data for all the Filetypes,and,no of **
**              file types.                                     **
**                                                              **
** references : dtcreate.h, parser.h, and dtcreate spec.        **
**                                                              **
******************************************************************/

FiletypeData **
#ifdef _NO_PROTO
GetFiletypeData(fp, pszOpenCmd,nftypes)
        FILE  *fp;
        char  *pszOpenCmd;
        short   *nftypes;
#else
GetFiletypeData(FILE  *fp, char *pszOpenCmd, short *nftypes)
#endif
{
int         manflds=0,len,nfiletypes,previous=0,lastfld=0,fldid;
char        linebuf[1024],**wordPairs,**execstr;
FiletypeData  **ppFiletypeData,**ppnewFiletypeData;

        wordPairs=0;
        ppFiletypeData=0;
        nfiletypes=0;

        /* Initialize the linebuf */
        bzero(linebuf,sizeof(linebuf));
        while (fgets (linebuf, sizeof (linebuf)-1, fp))
        {
           /* If begin of a stanza skip and continue */
           if(linebuf[0] == '{')
           {
               continue;
           }

           /* skip the rest of the line if a comment found */
           if(linebuf[0] == '#') continue;

           /* If end of a stanza check if all the mandatory fields are there */
           if( linebuf[0] == '}' )
           {
               /* Check for atleast one field in DATA_ATTRIBUTES stanza */
               if( previous == DATA_ATTRIBUTES && manflds != 1 )
               {
                   printf("Error in DATA_ATTR stanza of the FiletypeData\n");
                   return NULL;
               }
               /* Check for atleast one field in DATA_CRITERIA stanza */
               else if( previous == DATA_CRITERIA && manflds != 1 )
               {
                   printf("Error in DATA_CRITERIA stanza of the FiletypeData\n");
                   return NULL;
               }
               /* Check for atleast two fields in ACTION_OPEN or ACTION_PRINT stanza */
               else if( (previous == ACTION_OPEN || previous == ACTION_PRINT) )
               {
                   if( manflds != 4)
#ifdef NOT_ILS
                   if( manflds != 3)
#endif
                   {
                      printf("Error in ACTION_OPEN/PRINT stanza of the FiletypeData\n");
                      return NULL;
                   }
               }
               /* Check for atleast three fields in ACTION_PRINT_FTYPE stanza */
               else if( previous == ACTION_PRINT_FTYPE && manflds != 3 )
               {
                   printf("Error in ACTION_PRINT_FTYPE stanza of the FiletypeData\n");
                   return NULL;
               }
               continue;
           }
           len = strlen (linebuf);
           if (linebuf[len-1] == '\n')
              linebuf[len-1] = '\0';

           if( (wordPairs = GetKeywordValuePairs(linebuf,&fldid,FILETYPE_TABLE)) != NULL)
           {
              if( !strcmp(wordPairs[0],"DATA_ATTRIBUTES") )
              {
                /* first filetype */
                if( previous == 0 )
                {
                     /* Allocate a filetypedata pointer to an array of filetypedata records   */
                     if( !ppFiletypeData)
                     {
                         if( (ppFiletypeData = (FiletypeData **)calloc(1,sizeof(FiletypeData *)))
                                                  == NULL )
                         {
                                 printf("\n Cannot allocate memory\n");
                                 return NULL;
                         }
                         /* Allocate a filetypedata record   */
                         else
                         {
                                 if( (ppFiletypeData[nfiletypes] =
                                 (FiletypeData *)calloc(1,sizeof(FiletypeData)) )
                                                   == NULL )
                                 {
                                       printf("\n Cannot allocate memory\n");
                                       return NULL;
                                 }
                          }
                     }

                     ppFiletypeData[nfiletypes]->pszOpenCmd = pszOpenCmd;
                     previous = DATA_ATTRIBUTES;
                     manflds=0;
                }
                else if(previous == ACTION_OPEN || previous == ACTION_PRINT)
                {
                     /* New filetypedata started so allocate a new filetypedata ptr */
                     if( (ppnewFiletypeData =
                        (FiletypeData **)realloc((FiletypeData *)ppFiletypeData,
                        nfiletypes+2 * sizeof(FiletypeData *)))
                                              == NULL )
                     {
                          printf("\n Cannot allocate memory\n");
                          return NULL;
                     }
                     else if(ppFiletypeData)
                     {
                          ppFiletypeData=ppnewFiletypeData;
                          /* Process the contents field if any  */
                          if(ProcessContents((FiletypeData *)ppFiletypeData[nfiletypes]) < 0 )
                          {
                                printf("\n Error in Contents Data\n");
                                return NULL;
                          }
                         /* Everything looks right so process the exec_string */
                         if( !(execstr = ProcessExecString((char *)ppFiletypeData[nfiletypes]->pszPrintCmd)) )
                             ppFiletypeData[nfiletypes]->pszPrintCmd=NULL;
                         else
                             ppFiletypeData[nfiletypes]->pszPrintCmd=execstr[0];
                          nfiletypes++;
                          /* Allocate a new filetypedata record */
                          if( (ppFiletypeData[nfiletypes] =
                                (FiletypeData *)calloc(1,sizeof(FiletypeData)) )
                                                  == NULL )
                          {
                              printf("\n Cannot allocate memory\n");
                              return NULL;
                          }
                          ppFiletypeData[nfiletypes]->pszOpenCmd = pszOpenCmd;
                          previous = DATA_ATTRIBUTES;
                          manflds=0;
                     }

                }
                else
                {
                     printf("Error in filetypedata \n");
                     return NULL;
                }
           }
           else if( !strcmp(wordPairs[0],"DATA_CRITERIA") )
           {
                  if( previous == DATA_ATTRIBUTES )
                  {
                      previous = DATA_CRITERIA;
                      manflds=0;
                  }
                  else
                  {
                      printf("Error in filetypedata \n");
                      return NULL;
                  }
              }
              else if( !strcmp(wordPairs[0],"ACTION") && !strcmp(wordPairs[1],"Open") )
              {
                  if( previous == DATA_CRITERIA )
                  {
                      previous = ACTION_OPEN;
                      manflds=0;
                  }
                  else
                  {
                      printf("Error in filetypedata \n");
                      return NULL;
                  }
              }
           else if( !strcmp(wordPairs[0],"ACTION") && strcmp(wordPairs[1],"Open") &&
                  strcmp(wordPairs[1], "Print")  )
           {
                  if( previous == ACTION_OPEN )
                  {
                      previous = ACTION_PRINT_FTYPE;
                      manflds=0;
                  }
                  else
                  {
                      printf("Error in filetypedata \n");
                      return NULL;
                  }
           }
           else if( !strcmp(wordPairs[0],"ACTION") && !strcmp(wordPairs[1],"Print") )
           {
                  if( previous == ACTION_PRINT_FTYPE )
                  {
                      previous = ACTION_PRINT;
                      manflds=0;
                  }
                  else
                  {
                      printf("Error in filetypedata \n");
                      return NULL;
                  }
           }


           /* update mandatory fields counter */
           if( previous == DATA_ATTRIBUTES && strcmp(wordPairs[0],"DATA_ATTRIBUTES") )
           {
                  if( !strcmp(wordPairs[0],"ACTIONS") )
                      manflds++;
           }
           else if( previous == DATA_CRITERIA )
           {
                  if( !strcmp(wordPairs[0],"DATA_ATTRIBUTES_NAME") )
                      manflds++;
           }
           else if( previous == ACTION_OPEN || previous == ACTION_PRINT )
           {
                  if( !strcmp(wordPairs[0],"ARG_TYPE")   ||
                      !strcmp(wordPairs[0],"TYPE")       ||
                      !strcmp(wordPairs[0],"MAP_ACTION") ||
                      !strcmp(wordPairs[0],"LABEL") )
#ifdef NOT_ILS
                  if( !strcmp(wordPairs[0],"ARG_TYPE") ||
                      !strcmp(wordPairs[0],"TYPE")     ||
                      !strcmp(wordPairs[0],"MAP_ACTION") )
#endif
                      manflds++;
           }
           else if( previous == ACTION_PRINT_FTYPE )
           {
                  if( !strcmp(wordPairs[0],"TYPE") ||
                      !strcmp(wordPairs[0],"WINDOW_TYPE") )
                      manflds++;
           }

       }



   if( wordPairs && !wordPairs[1] && wordPairs[0])
           {
              switch(fldid)
              {
                  case 0:    /* next line for filetype_name */
                             ppFiletypeData[nfiletypes]->pszName=wordPairs[0];
                             break;

                  case 1:    /* next line for icon_name */
                             ppFiletypeData[nfiletypes]->pszIcon=wordPairs[0];
                             break;

                  case 2 :   /* next line for help_text */
                             ppFiletypeData[nfiletypes]->pszHelp=wordPairs[0];
                             break;

                  case 3:    /* next line for print_cmd */
                             ppFiletypeData[nfiletypes]->pszPrintCmd=wordPairs[0];
                             manflds++;
                             break;
                  case 4:    /* next line for name_pattern or path_pattern */
                             ppFiletypeData[nfiletypes]->pszPattern=wordPairs[0];
                             break;
                  case 5:    /* next line for name_pattern or path_pattern */
                             ppFiletypeData[nfiletypes]->pszPattern=wordPairs[0];
                             break;
                  case 6:    /* next line for mode */
                             ppFiletypeData[nfiletypes]->pszPermissions=wordPairs[0];
                             break;
                  case 7:    /* next line for contents */
                             ppFiletypeData[nfiletypes]->pszContents=wordPairs[0];
                             break;

                 default:    break;
              }
           }
           else if(wordPairs && wordPairs[1])
           {
              switch(fldid)
              {
                  case 0:   /* first line for filetype_name */
                            ppFiletypeData[nfiletypes]->pszName = wordPairs[1];
                            break;

                  case 1:   /* first line for icon_name */
                            ppFiletypeData[nfiletypes]->pszIcon=wordPairs[1];
                            break;

                  case 2 :  /* first line for help_text */
                            ppFiletypeData[nfiletypes]->pszHelp=wordPairs[1];
                            break;

                  case 3:   /* first line for print_cmd */
                            ppFiletypeData[nfiletypes]->pszPrintCmd=wordPairs[1];
                            manflds++;
                            break;

                  case 4:   /* first line for name_pattern or path_pattern */
                            ppFiletypeData[nfiletypes]->pszPattern=wordPairs[1];
                            break;

                  case 5:   /* first line for name_pattern or path_pattern */
                            ppFiletypeData[nfiletypes]->pszPattern=wordPairs[1];
                            break;

                  case 6:   /* first line for mode */
                            ppFiletypeData[nfiletypes]->pszPermissions=wordPairs[1];
                            break;

                  case 7:   /* first line for contents */
                            ppFiletypeData[nfiletypes]->pszContents=wordPairs[1];
                            break;

                 default:   break;
              }

           }

              bzero(linebuf,sizeof(linebuf));
        } /* end of while fgets */

        /* Done with retrieving FiletypeData */
        /* Check if we got all the mandatory fields data */
        if( (previous == ACTION_OPEN  && manflds != 4) ||
            (previous == ACTION_PRINT  && manflds != 4)   )
#ifdef NOT_ILS
        if( (previous == ACTION_OPEN  && manflds != 3) ||
            (previous == ACTION_PRINT  && manflds != 3)   )
#endif
            return NULL;
        else if(ppFiletypeData)
        {
            /* Process Contents for the last file filetype if any */
            if(ProcessContents((FiletypeData *)ppFiletypeData[nfiletypes]) < 0)
                          {
                                printf("\n Error in Contents Data\n");
                                return NULL;
                          }
            /* Everything looks right so process the exec_string */
            if(!(execstr =
                    ProcessExecString((char *)ppFiletypeData[nfiletypes]->pszPrintCmd)) )
                ppFiletypeData[nfiletypes]->pszPrintCmd=NULL;
            else
                ppFiletypeData[nfiletypes]->pszPrintCmd=execstr[0];
            if( !ppFiletypeData[nfiletypes])
                ppFiletypeData[nfiletypes] = 0;
            /* return number of filetypes */
            *nftypes = nfiletypes+1;
            return ppFiletypeData;

        }
        else
            return NULL;

}

/*****************************************************************
**                                                              **
** GetKeywordValuePairs(char *s, int *id, int table)            **
**                                                              **
** Description: Parses the text string in *s and returns a      **
**              pointer to two strings that are Keyword and     **
**              values.However, it returns the complete string  **
**              in the first pointer if keyword not found.      **
**                                                              **
** Input      : A character pointer pointing to text string.    **
**              An integer pointer to return the field id.      **
**              An integer value that determines which table to **
**              use (either ACTION_TABLE of FILETYPE_TABLE).    **
**              see definitions in parser.h                     **
**                                                              **
** Output     : pointer to two pointers pointing to text strings**
**              fieldid in the id parameter.                    **
**                                                              **
** references : dtcreate.h, parser.h, and dtcreate spec.        **
**                                                              **
******************************************************************/

char **
#ifdef _NO_PROTO
GetKeywordValuePairs(s,id,table)
        char *s;
        int  *id;
        int  table;
#else
GetKeywordValuePairs(char *s, int *id, int table)
#endif /* _NO_PROTO */
{
    char    *wordStart;
    static int fldid=-1;
    int        idx;

        args[0] = args[1] = args[2] = NULL;
        /* Skip all leading spaces */
        while (*s && isspace (*s))
            ++s;
        /* Skip the complete line if a '#' character is found (comments)  */
        if (!*s || *s == '#')
            return NULL;
        wordStart = s;
        while (*s && *s != '#' && !isspace (*s))
            ++s;
        if (!args[0])
        {
            args[0] = (char *)malloc (s - wordStart + 1);
            if (!args[0])
                return NULL;
            memset(args[0],0,sizeof(args[0]));
        }
        strncpy (args[0], wordStart, s - wordStart);
        args[0][s-wordStart] = '\0';
        if (!args[1])
        {
            if(s)
               args[1] = (char *)malloc (strlen(s)+1);
            if (!args[1])
            {
                if(args[0])
                free(args[0]);
                return NULL;
            }
            memset(args[1],0,sizeof(args[1]));
        }
        /* Skip all leading spaces */
        while (*s && isspace (*s))
            ++s;
        strcpy (args[1], s);
        args[2]=NULL;
        /* Check for whether args[0] is a keyword or not */
        idx=0;
        if( table == ACTION_TABLE )
        {
           /* Check for keyword in keywordDB defined in parser.h */
           while((strcmp(keywordDB[idx],"")) && (strcmp(args[0],keywordDB[idx])) ) idx++;
           if(idx >= 0 && idx < MAX_NUM_ACTION_FIELDS)
              fldid = idx;
        }
        else if( table == FILETYPE_TABLE )
        {
           /* Check for keyword in FiletypekeywordDB defined in parser.h */
           while((strcmp(FiletypekeywordDB[idx],"")) && (strcmp(args[0],FiletypekeywordDB[idx])) ) idx++;
           if(idx >= 0 && idx < MAX_NUM_FILETYPE_FIELDS)
              fldid = idx;
        }
        /* If no keyword found then keep the complete string
           in the first array
        */
        if( (idx == MAX_NUM_ACTION_FIELDS && table == ACTION_TABLE)  ||
            (idx == MAX_NUM_FILETYPE_FIELDS && table == FILETYPE_TABLE) )
        {
           char *temp4;

           if( (temp4 = (char *)realloc(args[0],strlen(args[0])+strlen(args[1])+2) ) == NULL )
           {
                printf("Cannot Allocate memory\n");
                return NULL;
           }
           args[0] = temp4;
           strcat(args[0]," ");
           strcat(args[0],args[1]);

           if(args[1])
           {
              free(args[1]);
              args[1]=NULL;
           }
           if( IsLastSingle(args[0]) && args[0][strlen(args[0])-1] == '\\' )
               args[0][strlen(args[0])-1] = '\0';
#ifdef NOT_ILS
           if( state != HELP_TEXT && args[0][strlen(args[0])-1] == '\\' )
                args[0][strlen(args[0])-1] = '\0';
           else if(state == HELP_TEXT)
           {
                /* from here the processing depends upon the code set */
                if(mbcodeset == -1)
                {
                    setlocale(LC_ALL,"");
                    if(MB_CUR_MAX == 1)
                       mbcodeset = 0;
                    else
                       mbcodeset = 1;
                }
                if(!mbcodeset && args[0][strlen(args[0])-1] == '\\')
                       args[0][strlen(args[0])-1] = '\0';
                else if(mbcodeset && args[0][mbslen(args[0])-1] == '\\')
                       args[0][mbslen(args[0])-1] = '\0';

           }
#endif
           if( (temp4 = (char *)realloc(fline,flinesize+strlen(args[0])+1)) == NULL)
           {
                printf("Cannot Allocate memory\n");
                return NULL;
           }
          strcat(temp4,args[0]);
          free(args[0]);
          args[0]=temp4;
        }

    *id = fldid;  /* return field id   */
    if(args[1] && IsLastSingle(args[1]) && args[1][strlen(args[1])-1] == '\\')
#ifdef NOT_ILS
    if(args[1] &&  args[1][strlen(args[1])-1] == '\\')
#endif
       args[1][strlen(args[1])-1] = '\0';
    if(args[0])
        args[0][strlen(args[0])] = '\0';
    if(args[1])
        args[1][strlen(args[1])] = '\0';
    if(args[0] && args[1])
    {
        fline = args[1];
        flinesize=strlen(fline);
    }
    if(!args[1] && args[0])
    {
        fline = args[0];
        flinesize=strlen(fline);
    }
    return args;
}


/*****************************************************************
**                                                              **
** ProcessExecString(char *)                                    **
**                                                              **
** Description: Parses the text string and returns two pointers,**
**              the first pointing to cmd part (if any)         **
**              the second pointing to prompt part (if any)     **
**              of either the ActionData structure type or      **
**              FiletypeData structure type. Please see         **
**              dtcreate.h for the above mentioned datastructure**
**              definitions and for the cmd and prompt fields   **
**                                                              **
** Input      : A character pointer that points to the complete **
**              text string comprising the command and arguments**
**              and prompt (if any).                            **
**                                                              **
** Limitation : Supports only ONE prompt.                       **
**                                                              **
** Output     : returns 0 (No error).                           **
**              returns >0 (Error).                             **
**                                                              **
** Assumptions: a) Arg fields start with a '%' character.       **
**              b) Prompt string start and end with '"' chara-  **
**                 cter. Also, the prompt string comes after    **
**                 the Arg strings if any.                      **
**                                                              **
** references : dtcreate.h, parser.h, and dtcreate spec.        **
**                                                              **
******************************************************************/

char **
#ifdef _NO_PROTO
ProcessExecString(cmd)
        char *cmd;
#else
ProcessExecString(char *cmd)
#endif
{

char *s1, *s2,*s3,*s4,*argbuf,**exec_args;
int  done=FALSE, argfound=FALSE,promptfound=FALSE;

        if (!cmd) {
           return((char **)NULL);
        }
	exec_args = (char **)malloc(3 * sizeof(char *));
        s1=s2=s3=s4=argbuf=NULL;
        /* Allocate buffer for the cmd string */
        exec_args[0] = (char *)calloc(1,strlen(cmd)+1);
        exec_args[1] = exec_args[2] = NULL;
        /* check if any Args present */
        s1=cmd;
        while( !done )
        {
                char *tmp;
                tmp=NULL;
                s4 = strstr(s1,"%Arg");
                if(s4)
                {
                      if ( (s4-s1) > 0 )
                      {
                          strncat(exec_args[0],s1,(s4-s1));
                          exec_args[0][strlen(exec_args[0])] = '\0';
                      }
                      s1 = s4;
                      s2 = strchr(s1+1,'%');  /* at this point we got s1 to s2
                                             covers the complete string
                                             between %'s
                                            */
                      if(argbuf) {
			  free(argbuf);
			  argbuf = 0;
		      }
                      if(s2)
                      {
                         argbuf = (char *)calloc(1,(s2-s1)+2);
                         strncpy(argbuf,s1,(s2-s1)+1);
                      }
                      else
                      {
                         strcat(exec_args[0],s1);
                         done=TRUE;
                         continue;
                      }
                      argbuf[strlen(argbuf)]='\0';
                      if( strncmp(argbuf,"%Arg_",5) &&
                               strncmp(argbuf,"%Args%",6) &&
                               strncmp(argbuf,"%Args\"",6)   )
                      {
                           strncat(exec_args[0],argbuf,strlen(argbuf)-1);
                           exec_args[0][strlen(exec_args[0])] = '\0';
                           s1=s2;
                           continue;
                      }
                }
                else if(s1 && strlen(s1) > 0)
                {
                     strcat(exec_args[0],s1);
                     if(argbuf) {
			 free(argbuf);
			 argbuf = 0;
		     }
                     done = TRUE;
                     continue;
                }
                else
                {
                     done = TRUE;
                     continue;
                }

                  /* move s1 until a '_' or 's' */
                  tmp = strchr(argbuf,'_');
                  if(!tmp)
                      tmp= strchr(argbuf,'s');
                  strcat(exec_args[0],"$");
                  argfound=TRUE;
                  /* start of Arg processing */
                  while (*tmp != '%' && *tmp != '"' )
                  {
                      if(*tmp == 's')
                      {
                         strcat(exec_args[0],"*");
                         break;
                      }
                      if( *tmp > '0' && *tmp <= '9')
                      {
                         strncat(exec_args[0],tmp,1);
                         exec_args[0][strlen(exec_args[0])] = '\0';
                      }
                      tmp++;
                  }
                /* end of Arg processing */

                /* Get Prompt string if any */
                s3 = strchr(argbuf,'"');
                if(s3 && *(s3+1) != '"')
                {
                      s3++;
#ifdef NOT_ILS
                      /* from here the processing depends upon the code set */
                      if(mbcodeset == -1 && s4)
                      {
                          setlocale(LC_ALL,"");
                          if(MB_CUR_MAX == 1)
                             mbcodeset = 0;
                          else
                             mbcodeset = 1;
                      }
#endif /* NOT_ILS */
                      if(!promptfound)
#ifdef NOT_ILS
                      if(!mbcodeset && !promptfound)
#endif /* NOT_ILS */
                      {
                          if( (tmp = strchr(s3,'"')) )
                          {
                               promptfound=TRUE;
                               exec_args[1] = (char *)calloc(1,(tmp-s3)+1);
                               strncpy(exec_args[1],s3,(tmp-s3));
                               exec_args[1][strlen(exec_args[1])] = '\0';
                          }
                      }
#ifdef NOT_ILS
                      else if(mbcodeset && !promptfound)
                      {
                          if( (tmp = (char *)mbschr(s3,'"')) )
                          {
                               promptfound=TRUE;
                               exec_args[1]=(char *)calloc(1,mbslen(tmp)-mbslen(s3)+1);
                               mbsncpy(exec_args[1],s3,(tmp-s3));
                               exec_args[1][mbslen(tmp)-mbslen(s3)]='\0';
                          }
                      }
#endif /* NOT_ILS */
                }

                /* see if there is some stuff between the last '%'
                   and the next first '%' of another Arg
                */
                if( argfound) s2++;
                argfound = FALSE;
                s1=s2;
         }
         if(argbuf) {
	     free(argbuf);
	     argbuf = 0;
	 }
   return exec_args;
}

/*****************************************************************
**                                                              **
** ProcessContents(FiletypeData *)                              **
**                                                              **
** Description: Parses the text string in pszContents field and **
**              fills the sStart,fsFlags and pszContents fields **
**              of the filetypeData datastructure that is passed**
**              as input.                                       **
**                                                              **
** Input      : A pointer of type FiletypeData defined in       **
**              dtcreate.h                                      **
**                                                              **
** Output     : Fills the sStart, fsFlags and pszContents fields**
**              of the FiletypeData pointer and returns 0 (No   **
**              error or >0 (error) values.                     **
**                                                              **
** Assumptions: Fields are separated by atleast one blank       **
**              character.                                      **
**                                                              **
** references : dtcreate.h, parser.h, and dtcreate spec.        **
**                                                              **
******************************************************************/

int
#ifdef _NO_PROTO
ProcessContents(pFtD)
        FiletypeData *pFtD;
#else
ProcessContents(FiletypeData *pFtD)
#endif
{

char    *s1,*tmp,*s2,buf[10],*cts;

        s1=s2=tmp=cts=NULL;
        bzero(buf,sizeof(buf));
        /* simply return if there is no data in contents field */
        if(!pFtD->pszContents)
           return (0);
        cts = (char *)calloc(1,strlen(pFtD->pszContents)+1);
        strcpy(cts,pFtD->pszContents);
        s1=cts;
        free(pFtD->pszContents);
        pFtD->pszContents=NULL;
        /* Skip all leading spaces */
        while( *s1 && isspace(*s1) )  s1++;
        /* Get the sStart value if any */
/*
        if( (s1 = strchr(cts,' ')) )
*/
        while( *s1 && !isspace(*s1) && *s1 != '\n') s1++;
        if((s1-cts) > 0)
        {
           strncpy(buf,cts,(s1-cts));
           buf[s1-cts]='\0';
           pFtD->sStart=atoi(buf);
        }
        while( *s1 && isspace(*s1) )  s1++;

        /* Get the fsFlags value if any */
        tmp=s1;
        bzero(buf,sizeof(buf));
/*
        if ( (tmp = strchr(s1,' ')) )
*/
        while( *tmp && !isspace(*tmp) && *tmp != '\n') tmp++;
        if((tmp-s1) > 0)
        {
           strncpy(buf,s1,(tmp-s1));
           buf[tmp-s1]='\0';
           if ( !strcmp(buf,"byte") )
               pFtD->fsFlags|=CA_FT_CNTBYTE;
           else if ( !strcmp(buf,"string") )
               pFtD->fsFlags|=CA_FT_CNTSTRING;
           else if ( !strcmp(buf,"long") )
               pFtD->fsFlags|=CA_FT_CNTLONG;
           else if ( !strcmp(buf,"short") )
               pFtD->fsFlags|=CA_FT_CNTSHORT;
           else
               return (-1);
        }
        while( *tmp && isspace(*tmp) )  tmp++;
        s2=tmp;
        /* Get the contents if any */
        if ( strlen(s2) > 0)
        {
              pFtD->pszContents =  (char *)calloc(1,strlen(s2)+1);
              strcpy(pFtD->pszContents,s2);
              free(cts);
#ifdef NOT_ILS
           /* from here the processing depends upon the code set */
           if(mbcodeset == -1)
           {
               setlocale(LC_ALL,"");
               if(MB_CUR_MAX == 1)
                    mbcodeset = 0;
               else
                    mbcodeset = 1;
            }
            if(!mbcodeset)
            {
                if ( (s3 = strchr(s2,'"')) )
                {
                   pFtD->pszContents =  (char *)calloc(1,s3-s2+1);
                   strncpy(pFtD->pszContents,s2,(s3-s2));
                   pFtD->pszContents[s3-s2]='\0';
                }

            }
            else if(mbcodeset)
            {
                if ( (s3 = (char *)mbschr(s2,'"')) )
                {
                   pFtD->pszContents =  (char *)calloc(1,mbslen(s3)-mbslen(s2)+1);
                   mbsncpy(pFtD->pszContents,s2,(size_t)(s3-s2));
                   pFtD->pszContents[mbslen(s3)-mbslen(s2)]='\0';
                }
            }
#endif /* NOT_ILS */

        }


 return 0;
}

/*****************************************************************
**                                                              **
** IsLastSingle(char *str)                                      **
**                                                              **
** Description: returns TRUE if the last character of the       **
**              string str is a single-byte character, returns  **
**              FALSE if it is a multi-byte character.          **
**                                                              **
******************************************************************/

Boolean
#ifdef _NO_PROTO
IsLastSingle(str)
        char *str;
#else
IsLastSingle(char *str)
#endif
{
    int n;

    if(MB_CUR_MAX == 1)
        return(TRUE);

    while(*str) {
        n = mblen(str, MB_CUR_MAX);
        str += n;
    }
    if(n > 1)
        return(FALSE);
    else
        return(TRUE);
}
