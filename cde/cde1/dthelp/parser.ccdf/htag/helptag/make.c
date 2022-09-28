/* $XConsortium: make.c /main/cde1_maint/2 1995/10/05 11:10:05 gtsang $ */
/*   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co. */
/* Functions for pseudo-make features of HP Tag/TeX translator, i.e.,
   for testing which generated files are current and which must be
   recreated.  */
/*
   NOTE : This file is used as a Microsoft Linker overlay, so it cannot
   have functions as pointers, eg. (proc *) f().  Routines that are
   or invoke function pointers should be in tex.c.
*/

#if defined(MSDOS)
#include <sys\types.h>
#include <sys\stat.h>
#endif

#if defined(hpux) || defined(_AIX) || defined(sun) || defined(USL) || defined(__uxp__)
#include <sys/types.h>
#include <sys/stat.h>
#endif

#include <math.h>

#if defined(hpux) || defined(_AIX) || defined(sun) || defined(USL) || defined(__uxp__)
#undef M_PI  /* M_PI is used by markup, we don't need the math PI */
#endif

#include "userinc.h"
#include "globdec.h"

/* Obsolete */
