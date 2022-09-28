/* -*-C-*-
****************************************************************
*
/* The following macro should be invoked once by each of the X11 components
   to provide standard revision information. */
#ifndef	lint
#define	vueversion() \
static char _what_vue_version[] = "@(#)HP Visual User Environment Version 3.0      A.04.00";\
char *_vue_version = _what_vue_version+4 ;
#else	/* lint */
#define	hpversion
#endif	/* lint */

/* Any revision information other than the standard 'hpversion' macro shoud
   be conditionally compiled using the VERBOSE_REV_INFO constant so that it
   can be excluded for official X11 releases. To set VERBOSE_REV_INFO, comment
   out one of the following 2 blocks. */

/* This block turns on VERBOSE_REV_INFO */
/*
#ifndef lint
#define VERBOSE_REV_INFO  1
#else
#ifdef VERBOSE_REV_INFO
#undef VERBOSE_REV_INFO
#endif
#endif
*/

/* This block turns off VERBOSE_REV_INFO */
#ifdef VERBOSE_REV_INFO		
#undef VERBOSE_REV_INFO
#endif
