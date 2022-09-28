/* $XConsortium: DtNlUtils.h /main/cde1_maint/1 1995/07/17 18:10:09 drk $ */
/*
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/**************************************************************************/
/*                                                                        */
/* Public include file for Dt localization functions.                    */
/*                                                                        */
/**************************************************************************/

#ifdef NLS16

#include <stdlib.h>
#include <locale.h>
#include <string.h>
#include <limits.h>
#include <nl_types.h>
#include <langinfo.h>
#endif

#include <X11/Intrinsic.h>


#ifdef NLS16

#define is_multibyte	_DtNl_is_multibyte
extern Boolean _DtNl_is_multibyte;


extern void Dt_nlInit( void ) ;
extern char * Dt_strtok( 
                        char *s1,
                        char *s2) ;
extern int Dt_strspn( 
                        char *s1,
                        char *s2) ;
extern int Dt_strcspn( 
                        char *s1,
                        char *s2) ;
extern char * Dt_strchr( 
                        char *s,
                        char c) ;
extern char * Dt_strrchr( 
                        char *s,
                        char c) ;
extern void Dt_lastChar( 
                        char *s,
                        char **cptr,
                        int *lenptr) ;
extern int Dt_charCount( 
                        char *s) ;

extern char * _Dt_NextChar(char *s);
extern char * _Dt_PrevChar(const char *start,char *s);
extern int _Dt_isspace(char *s);
extern int _Dt_isdigit(char *s);

#define DtNlInitialize()     (Dt_nlInit())
#define DtStrtok(s1, s2)     (Dt_strtok(s1, s2))
#define DtStrspn(s1, s2)     (Dt_strspn(s1, s2))
#define DtStrcspn(s1, s2)    (Dt_strcspn(s1, s2))
#define DtStrchr(s1, c)      (Dt_strchr(s1, c))
#define DtStrrchr(s1, c)     (Dt_strrchr(s1, c))
#define DtLastChar(s1, cp, lp)   (Dt_lastChar(s1, cp, lp))
#define DtCharCount(s1)      (Dt_charCount(s1))
#define DtNextChar(s)	     (is_multibyte?_Dt_NextChar(s):((s)+1))
#define DtPrevChar(st,s)     (is_multibyte?_Dt_PrevChar(st,s):((s)-1))
#define DtIsspace(s)	     (is_multibyte?_Dt_isspace(s):isspace(*(s)))
#define DtIsdigit(s)	     (is_multibyte?_Dt_isdigit(s):isdigit(*(s)))

#else	/* NLS16 */

#define DtNlInitialize()
#define DtStrtok(s1, s2)     (strtok(s1, s2))
#define DtStrspn(s1, s2)     (strspn(s1, s2))
#define DtStrcspn(s1, s2)    (strcspn(s1, s2))
#define DtStrchr(s1, c)      (strchr(s1, c))
#define DtStrrchr(s1, c)     (strrchr(s1, c))
#define DtLastChar(s1, cp, lp)   {(*cp = s1 + strlen(s1) - 1); *lp = 1;}
#define DtCharCount(s1)      (strlen(s1))
#define DtNextChar(s)	     ((s)+1)
#define DtPrevChar(st,s)     ((s)-1)
#define DtIsspace(s)	     (isspace(*s))
#define DtIsdigit(s)	     (isdigit(*s))
#endif /* NLS16 */

extern char * _DtGetNthChar( 
                        char *s,
                        int n) ;
extern char * _dt_strpbrk(
			char *cs,
			char *ct);
extern int _is_previous_single(
			char *s1,
			char *s2);

