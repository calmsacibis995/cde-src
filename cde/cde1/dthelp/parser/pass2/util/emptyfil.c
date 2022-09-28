/* $XConsortium: emptyfil.c /main/cde1_maint/1 1995/07/17 22:24:14 drk $ */
/* Copyright (c) 1988, 1989 Hewlett-Packard Co. */
/* Creates a file with nothing in it */
#include <stdio.h>
#if defined(MSDOS)
#include <process.h>
#endif
#include "basic.h"

void main(
#if defined(M_PROTO)
  int argc, char **argv
#endif
  ) ;

void main(argc, argv)
  int argc ;
  char **argv ;
  {
    if (argc != 2) {
      fprintf(stderr, "Usage: emptyfil filename\n") ;
      exit(1) ;
      }
    if (! fopen(argv[1], "w"))
      fprintf(stderr, "Unable to open %s\n", argv[1]) ;
    return 0;
    }
