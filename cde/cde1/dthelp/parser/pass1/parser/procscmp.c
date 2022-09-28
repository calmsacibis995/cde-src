/* $XConsortium: procscmp.c /main/cde1_maint/1 1995/07/17 21:52:46 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Procscmp.c prefixes the interface's procs.c file with appropriate
   declarations so it can be compiled. */

#include "userinc.h"
#include "globdec.h"

/* Include any procedures specified by the interface designer */
#include "procs.c"


