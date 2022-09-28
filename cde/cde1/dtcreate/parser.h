/* $XConsortium: parser.h /main/cde1_maint/2 1995/10/08 00:12:02 pascale $ */
/***************************************************************************/
/*                                                                         */
/*  parser.h                                                               */
/*                                                                         */
/***************************************************************************/

/* Field id Definitions for Actiondata Datastructure */
enum {
               DATA_ATTRIBUTES=1 ,
               DATA_CRITERIA     ,
               ACTION_OPEN       ,
               ACTION_PRINT_FTYPE,
               ACTION_PRINT
    };

/* Field id Definitions for Filetypedata Datastructure */
enum {
              ACTION_NAME=1,
              ACTION_CMD   ,
              ACTION_ICO   ,
              ACTION_HELP
    };
/* Tabletype definitions */
#define ACTION_TABLE       1
#define FILETYPE_TABLE     2
#define HELP_TEXT          3

/* Function prototype definitions */
#ifdef _NO_PROTO
int GetActionData();
#else
int GetActionData(FILE *, ActionData *);
#endif


/* Error code Defines */

#define ACTION_FILEOPEN_ERROR          1
#define ACTION_FILE_MUCKED             2
#define ACTION_DATA_ERROR              3

/***************************************************************************/
/* Platform specifics                                                      */
/***************************************************************************/
#ifdef NOT_ILS
#ifndef _AIX
#define mbslen strlen
#define mbsncpy strncpy
#endif
#endif /* NOT_ILS */

#ifdef SVR4
#define bzero(pbuffer, size)  memset(pbuffer, 0, size)
#endif
