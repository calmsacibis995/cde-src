/* $XConsortium: userinit.c /main/cde1_maint/3 1995/10/14 01:41:01 montyb $ */
#include "defs.h"
#include "name.h"
#include "variables.h"
#include <Dt/DtNlUtils.h>
#include <Dt/EnvControl.h>
#include <stdio.h>
#include <nl_types.h>
#include <X11/X.h>
#include <X11/Intrinsic.h>
#include <X11/IntrinsicP.h>
#include <X11/CoreP.h>
#include <X11/StringDefs.h>
#include <Xm/XmStrDefs.h>
#include <setjmp.h>
#include <string.h>
#include <ctype.h>
#include <Xm/Xm.h>
#include <Xm/Protocols.h>
#include "hash.h"
#include "stdio.h"
#define NO_AST
#include "dtksh.h"
#undef NO_AST
#include "xmksh.h"
#include "dtkcmds.h"
#include "xmcvt.h"
#include "widget.h"
#include "extra.h"
#include "xmwidgets.h"
#include "msgs.h"
#include <locale.h>


/*
 * LocaleChanged is defined in ksh93/src/cmd/ksh93/sh/init.c
 */
#ifdef _NO_PROTO

extern void LocaleChanged ();

#else

extern void LocaleChanged (
		Namval_t * np,
		char * val,
		int flags,
		Namfun_t * fp );

#endif /* _NO_PROTO */

static Namdisc_t localeDisc = { 0, LocaleChanged, NULL, NULL, NULL, NULL, NULL, NULL };
static Namfun_t localeFun = {NULL, NULL };

extern char *savedNlsPath; /* in ./ksh93/src/cmd/ksh93/sh/init.c */


void
#ifdef _NO_PROTO
SyncEnv(name)
    char *name;
#else
SyncEnv(
    char *name)
#endif /* _NO_PROTO */
{
  char *value, *buf;

  value = getenv(name);
  if(value != (char *)NULL)
  {
    buf = malloc(strlen(name) + strlen(value) + 2);
    strcpy(buf, name);
    strcat(buf, "=");
    strcat(buf, value);
    ksh_putenv(buf);
    free(buf);  /* I hope it's legal to free this! */
  }
}

/*
 *  This is a hook for an additional initialization routine
 *  A function of this name is called in main after sh_init().
 */

void
#ifdef _NO_PROTO
sh_userinit( )
#else
sh_userinit( void )
#endif /* _NO_PROTO */

{
   int * lockedFds;

   lockedFds = LockKshFileDescriptors();
   (void) XtSetLanguageProc((XtAppContext)NULL, (XtLanguageProc)NULL,
				(XtPointer)NULL);
   setlocale(LC_ALL, "");
   DtNlInitialize();
   _DtEnvControl(DT_ENV_SET);
   localeFun.disc = &localeDisc;
   nv_stack(LANGNOD, &localeFun);
   UnlockKshFileDescriptors(lockedFds);

   /*
    * Save the current setting of NLSPATH.  The user/script may want to
    * set its own NLSPATH to access its message catalog, so we need to
    * remember where to find our own catalog(s).  This saved path is used
    * in ksh93/src/cmd/ksh93/sh/init.c: _DtGetMessage().  We don't mess
    * with the user/script's setting of LANG as we want to track changes
    * in LANG.
    */
   savedNlsPath = strdup(getenv("NLSPATH"));

   /*
    * Sync the libc environment (set up by DtEnvControl) with our internal
    * hash table environment.
    */
  SyncEnv("NLSPATH");
  SyncEnv("LANG");
}
