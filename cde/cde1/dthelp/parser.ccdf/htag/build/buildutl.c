/* $XConsortium: buildutl.c /main/cde1_maint/1 1995/07/17 22:31:55 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Buildult.c contains the main supporting utilities for program BUILD */

#include <malloc.h>
#include "context.h"
#include "build.h"
#include "delim.h"

#include "version.h"

/* Verify that <PARAM>, <MIN> or <USEMAP> has not previously occurred in
   this rule */
void found(flag, delim)
LOGICAL *flag;
char *delim;
{
M_WCHAR wcbuff[129];
size_t  length;

wcbuff[128] = 0;
length = mbstowcs(wcbuff,delim,128);
if (length < 0)
    {
    m_error("An invalid multibyte character was seen");
    wcbuff[0] = 0;
    }

if (*flag)
    {
    m_err1("A rule can contain only one %s clause", wcbuff) ;
    return ;
    }
*flag = TRUE ;
}

/* Program initialization */
void initialize(M_NOPAR)
{
char    **mb_delims;
M_WCHAR **wc_delims;

ifile = stdin ;
m_openchk(&m_errfile, "error", "w") ;

fprintf(stderr, "MARKUP System - BUILD %s\n", M_VERSION) ;
fprintf(stderr, "Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.\n") ;
if (! standard)
    warning("Warning: optional enhancements of SGML enabled.") ;

mb_delims = mb_dlmptr;
wc_delims = m_dlmptr;

while (*mb_delims)
    {
    *wc_delims++ = MakeWideCharString(*mb_delims);
    mb_delims++;
    }
*wc_delims = 0;

ruleinit() ;
}


/* Initialization before starting a new rule */
void ruleinit(M_NOPAR)
  {
    /* Reinitialize for next rule */
    lhs = NULL ;
    nextlhs = &lhs ;
    eltsinrule = 0 ;
    plist = NULL ;
    pcount = 0 ;
    dellist(&bot.finals) ;
    dellist(&bot.allfinal) ;
    dellist(&bot.newfinal) ;
    poccur = uoccur = moccur = FALSE ;
    smin = emin = FALSE ;
    srefp = 0 ;
    useoradd = TRUE ;
    egensuf = 1 ;
    idoccur = FALSE ;
    }

/* Rule end processing */
void rulend(M_NOPAR)
  {
    STATE *fsa ;
    LOGICAL canbenull ;
    LHS *lhsp ;
    LHS *discard ;
    int inptr ;
    int exptr ;

    if (contype == GRPO) fsa = startfsa(ruletree, &canbenull) ;
    if (pcount > maxpar) maxpar = pcount ;
    inptr = lhs->elt->inptr ;
    exptr = lhs->elt->exptr ;
    for (lhsp = lhs ; lhsp ; ) {
      lhsp->elt->model = contype == GRPO ? fsa : (STATE *) M_NULLVAL ;
      lhsp->elt->content = contype ;
      lhsp->elt->inptr = inptr ;
      lhsp->elt->exptr = exptr ;
      if (plist) {
        if (lhsp->elt->parptr)
          warning1("Redefining parameter list for element %s",
            lhsp->elt->enptr) ;
        lhsp->elt->parptr = plist ;
        lhsp->elt->paramcount = pcount ;
        lhsp->elt->parindex = parcount - pcount + 1 ;
        }
      lhsp->elt->stmin = smin ;
      lhsp->elt->etmin = emin ;
      lhsp->elt->srefptr = srefp ;
      lhsp->elt->useoradd = useoradd ;
      discard = lhsp ;
      lhsp = lhsp->next ;
      m_free((M_POINTER) discard, "lhs") ;
      }
    if (ruletree) freetree(ruletree) ;
    ruletree = NULL ;
    ruleinit() ;
    }

/* Skip rest of statement after an error */
void skiptoend(M_NOPAR)
  {
    int i ;
    static errlev = 0 ;

    if (! errlev++) {
      curcon = ERROR ;
      for (i = scan() ; ; i = scan())
        if ((i == REND && ! entrule) ||
            (i == TAGC && entrule) ||
            i == ENDFILE) break ;
      curcon = RULE ;
      entrule = FALSE ;
      }
    errlev-- ;
    ruleinit() ;
  }

