/* $XConsortium: define.c /main/cde1_maint/1 1995/07/17 23:06:53 drk $ */
/*	Copyright (c) 1991, 1992 UNIX System Laboratories, Inc. */
/*	All Rights Reserved     */

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF    */
/*	UNIX System Laboratories, Inc.			*/
/*	The copyright notice above does not evidence any       */
/*	actual or intended publication of such source code.    */

#include "stdio.h"
#include "exksh.h" /* which includes sys/types.h */
#include <sys/param.h>
#include <string.h>
#include <search.h>
#include <ctype.h>
#include "misc.h"
#include "docall.h"
#include "basetbl.h"
#include "msgs.h"


#ifdef _NO_PROTO

static growdef() ;
static int add_deflist() ;
static def_init() ;

#else

static growdef( void ) ;
static int add_deflist( 
                        struct symarray *defptr,
                        char *prefix) ;
static def_init( void ) ;

#endif /* _NO_PROTO */


static struct symarray *Dyndef = NULL;
static int Ndyndef = 0;
static int Sdyndef = 0;
static char defInited = 0;

static char use[] = "0x%x";
static char use2[] = "%s=0x%x";

struct deflist {
	char *prefix;
	int size;
	struct symarray *defs;
};

struct deflist *Deflist = NULL;
int Ndeflist;


static
#ifdef _NO_PROTO
growdef()
#else
growdef( void )
#endif /* _NO_PROTO */
{
   int i;

   if (!defInited)
      def_init();

   if (!(Dyndef = (struct symarray *) realloc(Dyndef, (Sdyndef + 20) * 
                                              sizeof(struct symarray))))
   {
      return(SH_FAIL);
   }
   Deflist->defs = Dyndef;
   memset(((char *) Dyndef) + Sdyndef * sizeof(struct symarray), '\0', 
               20 * sizeof(struct symarray));
   Sdyndef += 20;
}

int
#ifdef _NO_PROTO
do_define( argc, argv )
        int argc ;
        char **argv ;
#else
do_define(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   int i, argstart, redo;
   char *name;
   struct symarray *found, dummy;

   if (!defInited)
      def_init();

   if (argc > 1 && C_PAIR(argv[1], '-', 'R')) 
   {
      redo = 0;
      argstart = 2;
   }
   else 
   {
      argstart = 1;
      redo = 1;
   }

   if ((argstart +1) >= argc)
      XK_USAGE(argv[0]);

   name = argv[argstart++];
   dummy.str = name;
   found = (struct symarray *) bsearch((char *) &dummy, Dyndef, Ndyndef, 
           sizeof(struct symarray), symcomp);

   if (found) 
   {
       if (!redo)
          return(SH_SUCC);
       i = found - Dyndef;
   }
   else 
   {
      if (Sdyndef == Ndyndef)
         growdef();
      Ndyndef++;
      if (Ndyndef > 1)
         for (i = Ndyndef - 1; i > 0; i--) 
         {
            if (strcmp(name, Dyndef[i - 1].str) >= 0)
               break;
            Dyndef[i] = Dyndef[i - 1];
         }
      else
         i = 0;
      Dyndef[i].str = strdup(name);
      Deflist->size++;
   }
   RIF(xk_par_int(argv + argstart, &Dyndef[i].addr, NULL));
   return(SH_SUCC);
}

int
#ifdef _NO_PROTO
fdef( str, val )
        char *str ;
        unsigned long *val ;
#else
fdef(
        char *str,
        unsigned long *val )
#endif /* _NO_PROTO */
{
   struct symarray *found, dummy;
   int i;

   dummy.str = str;
   if (!Deflist)
      return(0);

   for (i = 0; i < Ndeflist; i++) 
   {
      if (Deflist[i].defs) 
      {
         if (Deflist[i].size < 0)
         {
            found = (struct symarray *) lfind((char *) &dummy, Deflist[i].defs,
                    (unsigned int *) &Deflist[i].size, sizeof(struct symarray),
                     symcomp);
         }
         else
         {
            found = (struct symarray *) bsearch((char *) &dummy, 
                    Deflist[i].defs, Deflist[i].size, sizeof(struct symarray), 
                    symcomp);
         }

         if (found != NULL) 
         {
            *val = found->addr;
            return(1);
         }
      }
   }
   return(0);
}

int
#ifdef _NO_PROTO
do_deflist( argc, argv )
        int argc ;
        char **argv ;
#else
do_deflist(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   int i, j;
   char *prefix = NULL;
   struct symarray *defptr = NULL;
   char * errmsg;

   for (i = 1; (i < argc) && argv[i]; i++) 
   {
      if (argv[i][0] == '-') 
      {
         for (j = 1; argv[i][j]; j++) 
         {
            switch(argv[i][j]) 
            {
               case 'p': 
               {
                  if (argv[i][j + 1]) 
                  {
                     prefix = argv[i] + j;
                     j += strlen(prefix) - 2;
                  }
                  else 
                  {
                     prefix = argv[++i];
                     j = strlen(prefix) - 1;
                  }
               }
            }
         }
      }
      else 
      {
         if ((defptr = (struct symarray *) getaddr(argv[i])) == NULL) 
         {
            errmsg=strdup(GETMESSAGE(3,1, 
                          "Unable to locate the definition list '%s'"));
            printerrf(argv[0], errmsg, argv[i], NULL, NULL,
                      NULL, NULL, NULL, NULL, NULL);
            free(errmsg);
            return(SH_FAIL);
         }
      }
   }

   if (defptr == NULL)
   {
      XK_USAGE(argv[0]);
   }

   for (i = 0; i < Ndeflist; i++)
      if ((Deflist[i].defs == defptr) && 
          (!prefix || (strcmp(Deflist[i].prefix, prefix) == 0)))
      {
         return(SH_SUCC);
      }

   return(add_deflist(defptr, prefix));
}

static int
#ifdef _NO_PROTO
add_deflist( defptr, prefix )
        struct symarray *defptr ;
        char *prefix ;
#else
add_deflist(
        struct symarray *defptr,
        char *prefix )
#endif /* _NO_PROTO */
{
   int i;

   if (!Deflist)
   {
      Deflist = (struct deflist *) malloc((Ndeflist + 1) * 
                                           sizeof(struct deflist));
   }
   else
   {
      Deflist = (struct deflist *) realloc(Deflist, (Ndeflist + 1) * 
                                           sizeof(struct deflist));
   }

   if (!Deflist)
      return(SH_FAIL);

   Deflist[Ndeflist].defs = defptr;
   Deflist[Ndeflist].prefix = strdup(prefix);
   if (!defptr[0].str)
      Deflist[Ndeflist].size = 0;
   else 
   {
      for (i = 1; defptr[i].str && defptr[i].str[0]; i++)
         if (symcomp((void *) (defptr + i), (void *) (defptr + i - 1)) < 0)
            break;

      if (!(defptr[i].str && defptr[i].str[0]))
         Deflist[Ndeflist].size = i;
      else
         Deflist[Ndeflist].size = -1;
   }
   Ndeflist++;
   return(SH_SUCC);
}

int
#ifdef _NO_PROTO
do_finddef( argc, argv )
        int argc ;
        char **argv ;
#else
do_finddef(
        int argc,
        char **argv )
#endif /* _NO_PROTO */
{
   unsigned long found;
   struct symarray dummy;
   char * errmsg;

   if (argc < 2) 
      XK_USAGE(argv[0]);

   if (fdef(argv[1], &found)) 
   {
      if (argc >= 3) 
      {
         char buf[50];

         sprintf(buf, use2, argv[2], found);
         env_set(buf);
      }
      else 
      {
         sprintf(xk_ret_buffer, use, found);
         xk_ret_buf = xk_ret_buffer;
      }
      return(SH_SUCC);
   }
   errmsg = strdup(GETMESSAGE(3, 2, "Unable to locate the define '%s'"));
   printerrf(argv[0], errmsg, argv[1], NULL, NULL, NULL,
             NULL, NULL, NULL, NULL);
   free(errmsg);
   return(SH_FAIL);
}

static
#ifdef _NO_PROTO
def_init()
#else
def_init( void )
#endif /* _NO_PROTO */
{
   char * errhdr;
   char * errmsg;

   defInited = 1;
   if (!(Dyndef = (struct symarray *) malloc(20 * sizeof(struct symarray)))) 
   {
      errhdr = strdup(GetSharedMsg(DT_ERROR));
      errmsg =  strdup(GetSharedMsg(DT_ALLOC_FAILURE));
      printerr(errhdr, errmsg, NULL);
      free(errhdr);
      free(errmsg);
      exit(1);
   }
   Dyndef[0].str = NULL;
   Sdyndef = 20;
   Ndyndef = 0;
   add_deflist(Dyndef, "dynamic");
   add_deflist(basedefs, "base");
}
