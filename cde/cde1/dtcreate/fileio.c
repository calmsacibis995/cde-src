#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <Dt/Icon.h>

#include <stdlib.h>
#include "dtcreate.h"
#include "cmnrtns.h"
#include "fileio.h"

#define COMMENT_CHAR      '#'
#define MAX_KEY           10000
#define MAX_KEY_SIZE      5
#define MAX_HEADER_LINES  13

/*****************************************************************************/
/*                                                                           */
/*  CalculateMagicCookie                                                     */
/*                                                                           */
/*   description: Calculates a integer (the "key" value) which reflects the  */
/*                contents of the given file from the current file-offset    */
/*                to the end of file.                                        */
/*                Comment lines (those starting with '!' or '#') are ignored */
/*                in calculating the key value.                              */
/*                                                                           */
/*    input: pointer to FILE                                                 */
/*    output: integer - magic cookie                                         */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
int CalculateMagicCookie(fp)
       FILE *fp;
#else /* _NO_PROTO */
int CalculateMagicCookie(FILE *fp)
#endif /* _NO_PROTO */
{

#define PRIME1  2999
#define PRIME2  353
#define PRIME3  113

  long  save_offset, file_id;
  int   c, prev_c, char_sum, nline, nword, nchar, inword;

  /**************************************************************************/
  /* Since we will be traversing the file, we save the file-offset          */
  /* so that we can restore it before returning                             */
  /**************************************************************************/
  save_offset = ftell( fp );

  inword = FALSE;
  char_sum = nline = nword = nchar = 0;
  prev_c = '0';   /* This is arbitrary, but '0' not '\0' IS intended */

  while ( (c = fgetc( fp )) != EOF ) {
    if ( c == COMMENT_CHAR ) {
       while ( (c = fgetc( fp )) != EOF && c != '\n' )
               ; /* skip characters to end of line */
    } else {
       char_sum += (c * (prev_c%10));
       prev_c = c;
       ++nchar;

       if (c == '\n') {
          ++nline;
       }
       if ( c == ' ' || c == '\n' || c == '\t' ) {
          inword = FALSE;
       } else if ( inword == FALSE ) {
          inword = TRUE;
          ++nword;
       }
    }
  }

  file_id = PRIME1 * nline + PRIME2 * nword + PRIME3 * nchar + char_sum;

#ifdef DEBUG
  printf( "nline = %d nword = %d nchar = %d char_sum = %d\n",
          nline, nword, nchar, char_sum );
  printf( "file_id = %d\n", file_id );
#endif

  /**************************************************************************/
  /* We restore the file-offset to its previous value                       */
  /**************************************************************************/
  (void) fseek( fp, save_offset, 0 );
  return ( file_id % MAX_KEY );
}

/****************************************************************************/
/*                                                                          */
/*  SetCookie   - find the location in the file to write the magic cookie   */
/*                and update the file with the magic cookie.                */
/*                                                                          */
/*  input   - file pointer                                                  */
/*  output  - none                                                          */
/*                                                                          */
/****************************************************************************/
#ifdef _NO_PROTO
void SetCookie(fp)
     FILE *fp;
#else /* _NO_PROTO */
void SetCookie(FILE *fp)
#endif /* _NO_PROTO */
{
  int     iCookie;
  long    lSave_offset;
  char    buffer[256];
  char    *keystr;
  char    *retbuf;
  fpos_t  filepos;
  char    *mc_pos;
  char    *mc_clrpos;
  int     i;
  int     size;

  /**************************************************************************/
  /* Save file pointer initial setting                                      */
  /**************************************************************************/
  lSave_offset = ftell (fp);

  /**************************************************************************/
  /* Set file pointer to beginning of file                                  */
  /**************************************************************************/
  fseek(fp, 0, SEEK_SET);

  /**************************************************************************/
  /* Calculate magic cookie                                                 */
  /**************************************************************************/
  iCookie = CalculateMagicCookie(fp);
#ifdef DEBUG
  printf("Magic Cookie: %d\n", iCookie);
#endif

  /**************************************************************************/
  /* Find location in header and store cookie                               */
  /**************************************************************************/
  keystr = (char *)NULL;
  retbuf = buffer;
  i = 0;
  while ( !keystr && retbuf && (i < MAX_HEADER_LINES) ) {
    fgetpos(fp, &filepos);
    retbuf = fgets(buffer, 80, fp);
    keystr = strstr (buffer, "$KEY:");
    i++;
  }

  if (!keystr) {
#ifdef DEBUG
     printf("No '$KEY' keyword in file. Magic Cookie not written to file.\n");
#endif
     return;
  }
  mc_pos = strchr(keystr, ':');
  if (mc_pos) {
     mc_pos += 2;
     sprintf(mc_pos, "%i", iCookie);
     size = strlen(mc_pos);
     if (size < MAX_KEY_SIZE) {
        mc_clrpos = mc_pos + size;
        for (i=size; i < MAX_KEY_SIZE; i++) {
            *mc_clrpos = ' ';
            mc_clrpos++;
        }
     }
  }
  fsetpos (fp, &filepos);
  fputs (buffer, fp);

  /**************************************************************************/
  /* Set file pointer back to initial setting                               */
  /**************************************************************************/
  fseek (fp, lSave_offset, SEEK_SET);
  return;
}

/****************************************************************************/
/*                                                                          */
/*  CheckCookie  - check if the magic cookie exists in the file and is the  */
/*                 correct value.                                           */
/*                                                                          */
/*  input    - file pointer                                                 */
/*  output   - boolean indicating if correct value or not, TRUE = good      */
/*                                                                          */
/****************************************************************************/
#ifdef _NO_PROTO
Boolean CheckCookie(fp)
     FILE *fp;
#else /* _NO_PROTO */
Boolean CheckCookie(FILE *fp)
#endif /* _NO_PROTO */
{
  int     iNewCookie, iOldCookie;
  long    lSave_offset;
  char    buffer[256];
  char    *keystr;
  char    *retbuf;
  fpos_t  filepos;
  char    *mc_pos;
  int     i;
  Boolean bGoodCookie = TRUE;

  /**************************************************************************/
  /* Save file pointer initial setting                                      */
  /**************************************************************************/
  lSave_offset = ftell (fp);

  /**************************************************************************/
  /* Set file pointer to beginning of file                                  */
  /**************************************************************************/
  fseek (fp, 0, SEEK_SET);

  /**************************************************************************/
  /* Calculate magic cookie                                                 */
  /**************************************************************************/
  iNewCookie = CalculateMagicCookie(fp);
#ifdef DEBUG
  printf("New Magic Cookie: %d\n", iNewCookie);
#endif

  /**************************************************************************/
  /* Find location in header of old cookie and retrieve it.                 */
  /**************************************************************************/
  keystr = (char *)NULL;
  retbuf = buffer;
  i = 0;
  while ( !keystr && retbuf && (i < MAX_HEADER_LINES) ) {
    fgetpos(fp, &filepos);
    retbuf = fgets(buffer, 80, fp);
    keystr = strstr (buffer, "$KEY:");
    i++;
  }

  if (!keystr) {
#ifdef DEBUG
     printf("No '$KEY' keyword in file header. File is invalid.\n");
#endif
     strcpy(buffer, GETMESSAGE(5, 67, "Create Action can only edit files that were created\nand edited by Create Action."));
     display_error_message(CreateActionAppShell, buffer);
     bGoodCookie = FALSE;
     fseek (fp, lSave_offset, SEEK_SET);
     return(bGoodCookie);
  }

  mc_pos = strchr(keystr, ':');
  mc_pos += 2;

  sscanf(mc_pos, "%i", &iOldCookie);
#ifdef DEBUG
  printf("Old Magic Cookie: %d\n", iOldCookie);
#endif


  /**************************************************************************/
  /* Check if cookies are the same.                                         */
  /**************************************************************************/
#ifdef DEBUG
  if (iOldCookie == iNewCookie) {
     printf("Cookies are good = %i\n", iNewCookie);
  }
#endif

  if (iOldCookie != iNewCookie) {

#ifdef DEBUG
     printf("Cookies are bad; n=%i, o=%i\n", iNewCookie, iOldCookie);
#endif

     strcpy(buffer, GETMESSAGE(5, 66, "Could not load the file because it has been altered by some\nother means other than Create Action.  This could have been caused\nby modifying the file with a text editor or some other tool."));
     display_error_message(CreateActionAppShell, buffer);
     bGoodCookie = FALSE;
  }

  /**************************************************************************/
  /* Set file pointer back to initial setting                               */
  /**************************************************************************/
  fseek (fp, lSave_offset, SEEK_SET);
  return(bGoodCookie);
}

/*****************************************************************************/
/*                                                                           */
/*  MakeExecString                                                           */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      ptr  good                                                            */
/*      0    bad                                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
char * MakeExecString(pszCmd, pszPrompt)
        char         *pszCmd;
        char         *pszPrompt;
#else /* _NO_PROTO */
char * MakeExecString(char *pszCmd, char *pszPrompt)
#endif /* _NO_PROTO */

{
  char    *pszExecString;
  char    *pArgSrc = (char *)NULL;
  char    *pArgDst = (char *)NULL;
  char    *pTmp;
  char    pszArgStr[10];
  int     size = 0;
  int     cArgs = 0;
  int     val;
  Boolean bDidPrompt = FALSE;
#define   ARGSIZE  7    /* sizeof "%Arg_n%" */

  pArgSrc = pszCmd;
  while (pArgSrc = strchr(pArgSrc, '$')) {
#ifdef DEBUG
    printf("%d  pArgSrc = %s\n", cArgs, pArgSrc);
#endif
    pArgSrc++;
    val = *pArgSrc;
    switch (val) {
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9': case '*':
                cArgs++;
                break;
    }
  }

#ifdef DEBUG
  printf("Args found = %d\n", cArgs);
#endif

  size = 1;
  size += cArgs * ARGSIZE;
  size += strlen(pszCmd);
  if (pszPrompt) {
     size += strlen(pszPrompt) + 2;
  }
  pszExecString = (char *)XtMalloc(size);

  strcpy(pszExecString, pszCmd);
  pArgSrc = pszCmd;
  pTmp = pszExecString;
  while (pArgSrc = strchr(pArgSrc, '$')) {
    pArgDst = strchr(pTmp, '$');
    pTmp = pArgDst;
    *pArgDst = '\0';
    pTmp++;
    val = *pTmp;
    switch (val) {
      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9': case '*':
                pArgSrc+=2;
                if (*pTmp == '*') {
                   strcat(pszExecString, "%Args");
                } else {
                   sprintf(pszArgStr, "%%Arg_%c", *pTmp);
                   strcat(pszExecString, pszArgStr);
                }
                if ((pszPrompt) && (!bDidPrompt)) {
                   strcat(pszExecString, "\"");
                   strcat(pszExecString, pszPrompt);
                   strcat(pszExecString, "\"");
                   bDidPrompt = TRUE;
                }
                strcat(pszExecString, "%");
                break;
    }
    if (*pArgSrc) {
       strcat(pszExecString, pArgSrc);
       pArgSrc++;
    }
  }
#ifdef DEBUG
  printf("EXEC_STRING = '%s'\n", pszExecString);
#endif
  return(pszExecString);
}

/*****************************************************************************/
/*                                                                           */
/*  WriteDefinitionFile                                                      */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
ushort WriteDefinitionFile(pszFile, pAD)
        char         *pszFile;
        ActionData   *pAD;
#else /* _NO_PROTO */
ushort WriteDefinitionFile(char *pszFile, ActionData *pAD)
#endif /* _NO_PROTO */

{
  FILE            *fp = (FILE *)NULL;
  int             i;
  FiletypeData    *pFtD;
  char            buffer[MAXBUFSIZE];
  char            *ptr;

  /***************************************************************************/
  /* Open action and filetypes definition file for writing.                  */
  /***************************************************************************/
  if ((fp = fopen(pszFile, "w+")) == NULL) {
#ifdef DEBUG
    printf("Can not open file.\n"); /* debug */
#endif
    sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 55, "Could not create the following file:"), pszFile);
    display_error_message(CreateActionAppShell, buffer);
    return(1);
  }
#ifdef DEBUG
  printf("database file = %s\n", pszFile); /* debug */
#endif

  /***************************************************************************/
  /* Write comment area of action and filetype definition file, including    */
  /* revision number and magic cookie.                                       */
  /***************************************************************************/
  fprintf(fp, "######################################################################\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#    Common Desktop Environment (CDE)\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#    Action and DataType Definition File\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#      Generated by the CreateAction tool\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#      $Revision: 1.0\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#      $KEY: nnnnn \n");
  fprintf(fp, "#\n");
  fprintf(fp, "######################################################################\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#    WARNING:\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#    This file was generated by the CDE CreateAction tool.\n");
  fprintf(fp, "#    If this file is modified by some other tool, such as vi,\n");
  fprintf(fp, "#    the CreateAction tool will no longer be able to load and\n");
  fprintf(fp, "#    update this file.  For this reason, changes to this file\n");
  fprintf(fp, "#    should be handled through CreateAction whenever possible.\n");
  fprintf(fp, "#\n");
  fprintf(fp, "######################################################################\n");
  fprintf(fp, "\n");

  /***************************************************************************/
  /* Write the main Action definition.                                       */
  /***************************************************************************/
  ptr = GetCoreName(pszFile);
  fprintf(fp, "ACTION %s\n", ptr);
  free(ptr);
  fprintf(fp, "{\n");
  fprintf(fp, "     LABEL         %s\n", pAD->pszName);
  fprintf(fp, "     TYPE          COMMAND\n");
  ptr = MakeExecString(AD.pszCmd, AD.pszPrompt);
  if (ptr) {
     fprintf(fp, "     EXEC_STRING   %s\n", ptr);
     XtFree(ptr);
  }
  if (pAD->pszIcon) {
#ifdef OLDICON
    fprintf(fp, "     LARGE_ICON    %s.m\n", pAD->pszIcon);
    fprintf(fp, "     SMALL_ICON    %s.t\n", pAD->pszIcon);
#else
    if (ActionHasIcon()) {
       fprintf(fp, "     ICON          %s\n", pAD->pszName);
    } else {
       fprintf(fp, "     ICON          %s\n", pAD->pszIcon);
    }
#endif
  }
#ifdef DEBUG
  printf("CA flags = %X\n", pAD->fsFlags);
#endif
  if (pAD->fsFlags & CA_WT_TERM) fprintf(fp, "     WINDOW_TYPE   TERMINAL\n");
  else if (pAD->fsFlags & CA_WT_PERMTERM) fprintf(fp, "     WINDOW_TYPE   PERM_TERMINAL\n");
  else if (pAD->fsFlags & CA_WT_SHELLTERM) fprintf(fp, "     WINDOW_TYPE   SHELL_TERMINAL\n");
  else fprintf(fp, "     WINDOW_TYPE   NO_STDIO\n");

  if ((pAD->papFiletypes) && (pAD->fsFlags & CA_DF_ONLYFTFILES)) {
    fprintf(fp, "     ARG_TYPE      %s", (pAD->papFiletypes[0])->pszName);
    for (i=1; i < pAD->cFiletypes; i++) {
      pFtD = pAD->papFiletypes[i];
      fprintf(fp, ",%s", pFtD->pszName);
    }
    fprintf(fp, "\n");
  }

  if (pAD->pszHelp) {
    fprintf(fp, "     DESCRIPTION   %s\n", pAD->pszHelp);
  }
  fprintf(fp, "}\n");
  fprintf(fp, "\n");

  /***************************************************************************/
  /* Write the filetype definitions.                                         */
  /***************************************************************************/
  if (pAD->papFiletypes) {
    for (i=0; i < pAD->cFiletypes; i++) {
      pFtD = pAD->papFiletypes[i];
      /***********************************************************************/
      /* Write the data attributes declaration.                              */
      /***********************************************************************/
      fprintf(fp, "DATA_ATTRIBUTES %s\n", pFtD->pszName);
      fprintf(fp, "{\n");
      fprintf(fp, "     ACTIONS       Open");
      if (pFtD->pszPrintCmd) {
        fprintf(fp, ",Print");
      }
      fprintf(fp, "\n");
      if (pFtD->pszIcon) {
#ifdef OLDICON
        fprintf(fp, "     LARGE_ICON    %s.m\n", pFtD->pszIcon);
        fprintf(fp, "     SMALL_ICON    %s.t\n", pFtD->pszIcon);
#else
        if (FiletypeHasIcon(pFtD)) {
           fprintf(fp, "     ICON          %s\n", pFtD->pszName);
        } else {
           fprintf(fp, "     ICON          %s\n", pFtD->pszIcon);
        }
#endif
      }
      if (pFtD->pszHelp) {
        fprintf(fp, "     DESCRIPTION   %s\n", pFtD->pszHelp);
      }
      fprintf(fp, "}\n");
      fprintf(fp, "\n");
      /***********************************************************************/
      /* Write the data criteria   declaration.                              */
      /***********************************************************************/
      fprintf(fp, "DATA_CRITERIA %sA\n", pFtD->pszName);
      fprintf(fp, "{\n");
      fprintf(fp, "     DATA_ATTRIBUTES_NAME %s\n", pFtD->pszName);
      if (pFtD->pszPermissions) {
        fprintf(fp, "     MODE          %s\n", pFtD->pszPermissions);
      }
      if (pFtD->pszPattern) {
	  if (strchr(pFtD->pszPattern, '/')) {
	      fprintf(fp, "     PATH_PATTERN  %s\n", pFtD->pszPattern);
	  } else {
	      fprintf(fp, "     NAME_PATTERN  %s\n", pFtD->pszPattern);
	  }
      }
      if (pFtD->pszContents) {
         if (pFtD->fsFlags & CA_FT_CNTSTRING)
            strcpy(buffer, "string");
         else if (pFtD->fsFlags & CA_FT_CNTBYTE)
            strcpy(buffer, "byte");
         else if (pFtD->fsFlags & CA_FT_CNTSHORT)
            strcpy(buffer, "short");
         else if (pFtD->fsFlags & CA_FT_CNTLONG)
            strcpy(buffer, "long");
         fprintf(fp, "     CONTENT       %d %s %s\n", pFtD->sStart, buffer, pFtD->pszContents);
      }
      fprintf(fp, "}\n");
      fprintf(fp, "\n");
      /***********************************************************************/
      /* Write the Open MAP action.                                          */
      /***********************************************************************/
      fprintf(fp, "ACTION Open\n");
      fprintf(fp, "{\n");
      fprintf(fp, "     ARG_TYPE      %s\n", pFtD->pszName);
      fprintf(fp, "     TYPE          MAP\n");
      fprintf(fp, "     MAP_ACTION    %s\n", pAD->pszName);
      fprintf(fp, "     LABEL         %s\n", GETMESSAGE(7, 30, "Open"));
      fprintf(fp, "}\n");
      fprintf(fp, "\n");
      /***********************************************************************/
      /* Write the Print filetype action and MAP action.                     */
      /***********************************************************************/
      if (pFtD->pszPrintCmd) {
        /*********************************************************************/
        /* First write the print filetype action.                            */
        /*********************************************************************/
        fprintf(fp, "ACTION %s_PRINT\n", pFtD->pszName);
        fprintf(fp, "{\n");
        fprintf(fp, "     TYPE          COMMAND\n");
        ptr = MakeExecString(pFtD->pszPrintCmd, NULL);
        if (ptr) {
           fprintf(fp, "     EXEC_STRING   %s\n", ptr);
         /*fprintf(fp, "     EXEC_STRING   %s\n", pFtD->pszPrintCmd);*/
           XtFree(ptr);
        }
        fprintf(fp, "     WINDOW_TYPE   NO_STDIO\n");
        fprintf(fp, "}\n");
        fprintf(fp, "\n");
        /*********************************************************************/
        /* Now writhe the Print MAP action.                                  */
        /*********************************************************************/
        fprintf(fp, "ACTION Print\n");
        fprintf(fp, "{\n");
        fprintf(fp, "     ARG_TYPE      %s\n", pFtD->pszName);
        fprintf(fp, "     TYPE          MAP\n");
        fprintf(fp, "     MAP_ACTION    %s_PRINT\n", pFtD->pszName);
        fprintf(fp, "     LABEL         %s\n", GETMESSAGE(7, 31, "Print"));
        fprintf(fp, "}\n");
        fprintf(fp, "\n");
      }
    }
  }
  SetCookie(fp);
  if (fp) fclose(fp);
  chmod(pszFile, 0644);
  return(0);
}

/*****************************************************************************/
/*                                                                           */
/*  WriteActionFile                                                          */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
ushort WriteActionFile(pAD)
        ActionData   *pAD;
#else /* _NO_PROTO */
ushort WriteActionFile(ActionData *pAD)
#endif /* _NO_PROTO */

{
  FILE            *fp = (FILE *)NULL;
  char            pszFile[MAXBUFSIZE];
  char            buffer[MAXBUFSIZE];
  char            *newName;
  char            *pszEnvVar;

  /***************************************************************************/
  /* Open action file for writing.                                           */
  /***************************************************************************/
  newName = ReplaceSpaces(AD.pszName);
  pszEnvVar = getenv("HOME");
  if ( pszEnvVar && strlen(pszEnvVar) ) {
     sprintf(pszFile, "%s/%s", pszEnvVar, newName);
  } else {
     sprintf(pszFile, "/tmp/%s", newName);
  }
  free(newName);

  /***************************************************************************/
  /* Open action file for writing.                                           */
  /***************************************************************************/
  if ((fp = fopen(pszFile, "w")) == NULL) {
#ifdef DEBUG
    printf("Can not open file.\n"); /* debug */
#endif
    sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 55, "Could not create the following file:"), pszFile);
    display_error_message(CreateActionAppShell, buffer);
    return(1);
  }
#ifdef DEBUG
  printf("action file = %s\n", pszFile); /* debug */
#endif
  /***************************************************************************/
  /* Write action file contents.                                             */
  /***************************************************************************/
  fprintf(fp, "################################################\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#  action\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#  Common Desktop Environment (CDE)\n");
  fprintf(fp, "#\n");
  fprintf(fp, "#  $XConsortium: fileio.c /main/cde1_maint/3 1995/10/08 00:11:34 pascale $\n");
  fprintf(fp, "#\n");
  fprintf(fp, "################################################\n");
  fprintf(fp, "\n");
  fprintf(fp, "# This file represents a CDE Dt action.  The contents of the file do\n");
  fprintf(fp, "# not matter; however, the file must have executable permission and the\n");
  fprintf(fp, "# name of the file must match an entry in the action database (*.dt files).\n");
  fprintf(fp, "#\n");
  fprintf(fp, "# For more information on Dt actions, see the CDE Dt manuals.\n");
  fprintf(fp, "\n");
  fprintf(fp, "echo \"Common Desktop Environment Dt action\"\n");
  fprintf(fp, "\n");

  if (fp) fclose(fp);
  chmod(pszFile, 0755);
  return(0);
}

/******************************************************************************/
/* check_file_exists - check if the file exists that is passed in, if not     */
/*                     display error dialog.                                  */
/*                                                                            */
/* INPUT:   filename      - name of file to check if it exists or not.        */
/*                                                                            */
/* OUTPUT:  TRUE          - file exists                                       */
/*          FALSE         - file does not exist                               */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean check_file_exists (pszFileName)
     char  *pszFileName;
#else /* _NO_PROTO */
Boolean check_file_exists (char *pszFileName)
#endif /* _NO_PROTO */
{
  FILE *exists;

  /***************************************/
  /* see if file exists                  */
  /***************************************/
  exists = fopen (pszFileName, "r");

  /***************************************/
  /* if exists, return good return code  */
  /***************************************/
  if (exists) {
    fclose (exists);
    return(TRUE);
  } else {
#ifdef DEBUG
    printf("File '%s' does not exist!\n", pszFileName);
#endif
    return(FALSE);
  }
}

/*****************************************************************************/
/*                                                                           */
/*  OpenDefinitionFile                                                       */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    error                                                           */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
int OpenDefinitionFile(pszFile, pAD)
        char         *pszFile;
        ActionData   *pAD;
#else /* _NO_PROTO */
int OpenDefinitionFile(char *pszFile, ActionData *pAD)
#endif /* _NO_PROTO */

{
  FILE       *fp = (FILE *)NULL;
  char       buffer[MAXBUFSIZE];
  int        rc;

  /***************************************************************************/
  /* Open action and filetypes definition file for reading.                  */
  /***************************************************************************/
  if ((fp = fopen(pszFile, "r+")) == NULL) {
#ifdef DEBUG
    printf("Can not open file.\n"); /* debug */
#endif
    sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 60, "Could not open the following file:"), pszFile);
    /*
    sprintf(buffer, "Can not open the file: %s", pszFile);
    */
    display_error_message(CreateActionAppShell, buffer);
    return(1);
  }
#ifdef DEBUG
  printf("Opened database file = %s\n", pszFile); /* debug */
#endif

  /***************************************************************************/
  /* If magic cookie is good, then parse the file.                           */
  /***************************************************************************/
  if (CheckCookie(fp)) {
#ifdef DEBUG
     printf("Cookie good, call parse code\n");
#endif
     rc = GetActionData(fp, pAD);

     /************************************************************************/
     /* If error, display error message.                                     */
     /************************************************************************/
     if (rc) {
        sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 90, "Could not parse the following file:"), pszFile);
        /*
        sprintf(buffer, "Can not parse the file: %s", pszFile);
        */
        display_error_message(CreateActionAppShell, buffer);
        return(1);
     }
  }
  if (fp) fclose(fp);
  return(rc);
}

/******************************************************************************/
/* CheckDirtyIcon - check if icon has a DirtyBit set                          */
/*                                                                            */
/* INPUT:   Icon Widget   - widget id of icon gadget to check                 */
/*                                                                            */
/* OUTPUT:  TRUE          - dirty bit is set                                  */
/*          FALSE         - dirty bit is not set                              */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean CheckDirtyIcon (wid)
     Widget wid;
#else /* _NO_PROTO */
Boolean CheckDirtyIcon (Widget wid)
#endif /* _NO_PROTO */
{
  IconData  *pIconData;

  pIconData = GetIconDataFromWid(wid);
  if (pIconData) {
     if ( (pIconData->pmDirtyBit) || (pIconData->bmDirtyBit) ) {
        return(True);
     }
  }
  return(False);
}

/*****************************************************************************/
/*                                                                           */
/*  CreateIconName                                                           */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
char * CreateIconName(pszDirName, pszBaseName, enumSize, pszType, bIsMask)
         char                  *pszDirName;
         char                  *pszBaseName;
         enum icon_size_range  enumSize;
         char                  *pszType;
         Boolean               bIsMask;
#else /* _NO_PROTO */
char * CreateIconName(char *pszDirName, char *pszBaseName, enum icon_size_range enumSize, char *pszType, Boolean bIsMask)
#endif /* _NO_PROTO */
{
  char  *pszName;
  char  pszSize[MAX_EXT_SIZE];

  switch (enumSize) {
    case Large_Icon :
               strcpy(pszSize, LARGE_EXT);
               break;
    case Medium_Icon :
               strcpy(pszSize, MEDIUM_EXT);
               break;
    case Tiny_Icon :
               strcpy(pszSize, TINY_EXT);
               break;
  }
  if (bIsMask) {
     strcat(pszSize, "_m");
  }
  if (pszDirName) {
     pszName = XtMalloc(strlen(pszDirName) + strlen(pszBaseName) + strlen(pszSize) + strlen(pszType) + 1);
     if (pszName) {
        sprintf(pszName, "%s%s%s%s", pszDirName, pszBaseName, pszSize, pszType);
     }
  } else {
     pszName = XtMalloc(strlen(pszBaseName) + strlen(pszSize) + strlen(pszType) + 1);
     if (pszName) {
        sprintf(pszName, "%s%s%s", pszBaseName, pszSize, pszType);
     }
  }
  return(pszName);
}

/*****************************************************************************/
/*                                                                           */
/*  CopyFile                                                                 */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
ushort CopyFile(pszSource, pszDest)
        char *pszSource;
        char *pszDest;
#else /* _NO_PROTO */
ushort CopyFile(char *pszSource, char *pszDest)
#endif /* _NO_PROTO */

{
  int            fdSource;
  int            fdDest;
  int            len;
  int            rlen;
  ushort         usRc = 0;
  char           *buffer;

  fdSource = open(pszSource, O_RDONLY);
  if (fdSource == -1) {
     sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 60, "Could not open the following file:"), pszSource);
     display_error_message(CreateActionAppShell, pszSource);
     usRc = 1;
  } else {
     /************************************************************/
     /* Find out how big a buffer is needed                      */
     /************************************************************/
     len = lseek(fdSource, 0, SEEK_END);
     (void) lseek(fdSource, 0, SEEK_SET);
     buffer = XtMalloc((Cardinal) (len + 1));
     if ((rlen = read(fdSource, buffer, len)) != len) {
        /*********************************************************/
        /* Didn't read whole file!                               */
        /*********************************************************/
     } else {
        fdDest = creat(pszDest, 0644);
        /*fdDest = open(pszDest, O_WRONLY);*/
        if (fdDest == -1) {
           sprintf(buffer, "%s\n   %s", GETMESSAGE(5, 55, "Could not create the following file:"), pszDest);
           display_error_message(CreateActionAppShell, pszDest);
           usRc = 1;
        } else {
           if ((rlen = write(fdDest, buffer, len)) != len) {
              /***************************************************/
              /* Didn't write the whole file!                    */
              /***************************************************/
           }
        }
     }

     if (fdSource) close(fdSource);
     if (fdDest) close(fdDest);
  }
  return(usRc);
}

/*****************************************************************************/
/*                                                                           */
/*  CreateDirName                                                            */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      NULL        error has occurred                                       */
/*      not NULL    name of directory to store icons                         */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
char * CreateDirName()
#else /* _NO_PROTO */
char * CreateDirName(void)
#endif /* _NO_PROTO */
{
  char  *pszNewName = (char *)NULL;
  char  *pszDirName = (char *)NULL;
  char  *pszEnvVar;
  char  buffer[MAXBUFSIZE];

  pszEnvVar = getenv("HOME");
  if ( pszEnvVar && strlen(pszEnvVar) ) {
     pszDirName = XtMalloc(strlen(pszEnvVar) + strlen("/.dt/icons/") + 1);
     sprintf(pszDirName, "%s/.dt/icons/", pszEnvVar);
  } else {
     sprintf(buffer, "%s", GETMESSAGE(5, 170, "The HOME environment variable is not set.\nThe HOME environment variable must be set in order\nfor this action to be saved."));
     display_error_message(CreateActionAppShell, buffer);
  }
  return(pszDirName);
}

/*****************************************************************************/
/*                                                                           */
/*  CopyFilesFromWid                                                         */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
ushort CopyFilesFromWid(pAD, wid)
        ActionData *pAD;
        Widget     wid;
#else /* _NO_PROTO */
ushort CopyFilesFromWid(ActionData *pAD, Widget wid)
#endif /* _NO_PROTO */

{
  IconData      *pIconData;
  char          *pszDirName = (char *)NULL;
  Boolean       bHaveError = FALSE;
  char          *pszIconName = (char *)NULL;
  char          *pszNewName = (char *)NULL;
  char          *pszMaskName = (char *)NULL;
  char          buffer[MAXBUFSIZE];

  pszDirName = CreateDirName();
  if (!pszDirName) bHaveError = True;

  pIconData = GetIconDataFromWid(wid);
  pszNewName = ReplaceSpaces(pAD->pszName);

  if ((!bHaveError) && (pIconData)) {
     pszIconName = CreateIconName(pszDirName, pszNewName, pIconData->size, PIXMAP_EXT, FALSE);
     if ((!pszIconName) || (CopyFile(pIconData->pmFileName, pszIconName))) {
        bHaveError = True;
     }
     if (pszIconName) XtFree(pszIconName);
  }

  if ((!bHaveError) && (pIconData)) {
     pszIconName = CreateIconName(pszDirName, pszNewName, pIconData->size, BITMAP_EXT, FALSE);
     if ((!pszIconName) || (CopyFile(pIconData->bmFileName, pszIconName))) {
        bHaveError = True;
     }
     if (pszIconName) XtFree(pszIconName);

     /**********************************************/
     /* Now see if it has a mask file.             */
     /**********************************************/
     if (!bHaveError) {
        pszMaskName = CreateMaskName(pIconData->bmFileName);
        if ((pszMaskName) && (check_file_exists(pszMaskName))) {
#ifdef DEBUG
           printf("Mask file exists = %s\n", pszMaskName); /* debug */
#endif
           /*sprintf(buffer, "%s", pszNewName);*/
           pszIconName = CreateIconName(pszDirName, pszNewName, pIconData->size, BITMAP_EXT, TRUE);
           if ((!pszIconName) || (CopyFile(pszMaskName, pszIconName))) {
              bHaveError = True;
           }
           if (pszIconName) XtFree(pszIconName);
           XtFree(pszMaskName);
        }
     }
  }

  if (pszNewName) XtFree(pszNewName);
  if (pszDirName) XtFree(pszDirName);

  if (bHaveError) {
     return(1);
  } else {
     return(0);
  }
}
/******************************************************************************/
/* ActionHasIcon - check if any of the main action's icons have dirty bit set */
/*                                                                            */
/* INPUT:   none                                                              */
/*                                                                            */
/* OUTPUT:  TRUE          - action has icon                                   */
/*          FALSE         - no icon for this action                           */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean ActionHasIcon ()
#else /* _NO_PROTO */
Boolean ActionHasIcon (void)
#endif /* _NO_PROTO */
{
  if ( CheckDirtyIcon(CA_LRG_IconGadget) ||
       CheckDirtyIcon(CA_MED_IconGadget) ||
       CheckDirtyIcon(CA_TINY_IconGadget) ) {
     return(True);
  } else {
     return(False);
  }

}

/******************************************************************************/
/* FiletypeHasIcon - check if the filetype has modified icons.                */
/*                                                                            */
/* INPUT:   Filetype data structure pointer                                   */
/*                                                                            */
/* OUTPUT:  TRUE          - action has icon                                   */
/*          FALSE         - no icon for this action                           */
/*                                                                            */
/******************************************************************************/
#ifdef _NO_PROTO
Boolean FiletypeHasIcon (pFtD)
         FiletypeData *pFtD;
#else /* _NO_PROTO */
Boolean FiletypeHasIcon (FiletypeData *pFtD)
#endif /* _NO_PROTO */
{
  if ( (pFtD->pszMedPmIcon) ||
       (pFtD->pszMedBmIcon) ||
       (pFtD->pszTinyPmIcon) ||
       (pFtD->pszTinyBmIcon) ) {
     return(True);
  } else {
     return(False);
  }

}

/*****************************************************************************/
/*                                                                           */
/*  CopyIconFiles                                                            */
/*                                                                           */
/*    return codes:                                                          */
/*                                                                           */
/*      0    successful                                                      */
/*      1    could not open file                                             */
/*                                                                           */
/*****************************************************************************/
#ifdef _NO_PROTO
ushort CopyIconFiles(pAD)
        ActionData   *pAD;
#else /* _NO_PROTO */
ushort CopyIconFiles(ActionData *pAD)
#endif /* _NO_PROTO */

{
  Boolean       bHaveError = FALSE;
  FiletypeData  *pFtD;
  int           i;
  char          *pszDirName = (char *)NULL;
  char          *pszNewIconName;
  char          *pszNewIconMask;
  char          *pszOldIconName;
  char          *ptr;
  char          buffer[MAXFILENAME];

#define  CIF_ERROR    1
#define  CIF_SUCCESS  0

  if (ActionHasIcon()) {
     if ( !bHaveError && (CopyFilesFromWid(pAD, CA_LRG_IconGadget) != 0) ) {
        bHaveError = TRUE;
     }
     if ( !bHaveError && (CopyFilesFromWid(pAD, CA_MED_IconGadget) != 0) ) {
        bHaveError = TRUE;
     }
     if ( !bHaveError && (CopyFilesFromWid(pAD, CA_TINY_IconGadget) != 0) ) {
        bHaveError = TRUE;
     }
  }

  if (!bHaveError && (pAD->papFiletypes)) {
     pszDirName = CreateDirName();
     for (i=0; i < pAD->cFiletypes; i++) {
       pFtD = pAD->papFiletypes[i];
       if (FiletypeHasIcon(pFtD)) {
          FIND_ICONGADGET_ICON(pFtD->pszIcon, pszOldIconName, DtMEDIUM);
          if (pszOldIconName) {
             ptr = strstr(pszOldIconName, MEDIUM_EXT);
             if (ptr) *ptr = '\0';
          }

          /*****************************************************************/
          /* Medium Pixmap                                                 */
          /*****************************************************************/
          pszNewIconName = CreateIconName(pszDirName, pFtD->pszName, Medium_Icon, PIXMAP_EXT, FALSE);
          if (pszNewIconName) {
             if (pFtD->pszMedPmIcon) {
                CopyFile(pFtD->pszMedPmIcon, pszNewIconName);
             } else {
                ptr = CreateIconName((char *)NULL, pszOldIconName, Medium_Icon, PIXMAP_EXT, FALSE);
                if (ptr) {
                   CopyFile(ptr, pszNewIconName);
                   XtFree(ptr);
                }
             }
             if (pszNewIconName) XtFree(pszNewIconName);
          }

          /*****************************************************************/
          /* Medium Bitmap                                                 */
          /*****************************************************************/
          pszNewIconName = CreateIconName(pszDirName, pFtD->pszName, Medium_Icon, BITMAP_EXT, FALSE);
          pszNewIconMask = CreateIconName(pszDirName, pFtD->pszName, Medium_Icon, BITMAP_EXT, TRUE);
          if (pszNewIconName) {
             if (pFtD->pszMedBmIcon) {
                CopyFile(pFtD->pszMedBmIcon, pszNewIconName);
                /****************************/
                /* Now do the mask file.    */
                /****************************/
                sprintf(buffer, "%s_m", pFtD->pszMedBmIcon);
                if (pszNewIconMask && check_file_exists(buffer)) {
                   CopyFile(buffer, pszNewIconMask);
                }
             } else {
                ptr = CreateIconName((char *)NULL, pszOldIconName, Medium_Icon, BITMAP_EXT, FALSE);
                if (ptr) {
                   CopyFile(ptr, pszNewIconName);
                   XtFree(ptr);
                }
                /****************************/
                /* Now do the mask file.    */
                /****************************/
                ptr = CreateIconName((char *)NULL, pszOldIconName, Medium_Icon, BITMAP_EXT, TRUE);
                if (ptr && check_file_exists(ptr) && pszNewIconMask) {
                   CopyFile(ptr, pszNewIconMask);
                   XtFree(ptr);
                }
             }
             if (pszNewIconName) XtFree(pszNewIconName);
             if (pszNewIconMask) XtFree(pszNewIconMask);
          }

          /*****************************************************************/
          /* Tiny Pixmap                                                   */
          /*****************************************************************/
          pszNewIconName = CreateIconName(pszDirName, pFtD->pszName, Tiny_Icon, PIXMAP_EXT, FALSE);
          if (pszNewIconName) {
             if (pFtD->pszTinyPmIcon) {
                CopyFile(pFtD->pszTinyPmIcon, pszNewIconName);
             } else {
                ptr = CreateIconName((char *)NULL, pszOldIconName, Tiny_Icon, PIXMAP_EXT, FALSE);
                if (ptr) {
                   CopyFile(ptr, pszNewIconName);
                   XtFree(ptr);
                }
             }
             if (pszNewIconName) XtFree(pszNewIconName);
          }

          /*****************************************************************/
          /* Tiny Bitmap                                                   */
          /*****************************************************************/
          pszNewIconName = CreateIconName(pszDirName, pFtD->pszName, Tiny_Icon, BITMAP_EXT, FALSE);
          pszNewIconMask = CreateIconName(pszDirName, pFtD->pszName, Tiny_Icon, BITMAP_EXT, TRUE);
          if (pszNewIconName) {
             if (pFtD->pszTinyBmIcon) {
                CopyFile(pFtD->pszTinyBmIcon, pszNewIconName);
                /****************************/
                /* Now do the mask file.    */
                /****************************/
                sprintf(buffer, "%s_m", pFtD->pszTinyBmIcon);
                if (pszNewIconMask && check_file_exists(buffer)) {
                   CopyFile(buffer, pszNewIconMask);
                }
             } else {
                ptr = CreateIconName((char *)NULL, pszOldIconName, Tiny_Icon, BITMAP_EXT, FALSE);
                if (ptr) {
                   CopyFile(ptr, pszNewIconName);
                   XtFree(ptr);
                }
                /****************************/
                /* Now do the mask file.    */
                /****************************/
                ptr = CreateIconName((char *)NULL, pszOldIconName, Tiny_Icon, BITMAP_EXT, TRUE);
                if (ptr && check_file_exists(ptr) && pszNewIconMask) {
                   CopyFile(ptr, pszNewIconMask);
                   XtFree(ptr);
                }
             }
             if (pszNewIconName) XtFree(pszNewIconName);
             if (pszNewIconMask) XtFree(pszNewIconMask);
          }
       }
     }
  }

  return(CIF_SUCCESS);
}
