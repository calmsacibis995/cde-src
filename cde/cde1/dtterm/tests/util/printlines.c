/* $XConsortium: printlines.c /main/cde1_maint/1 1995/07/18 01:50:01 drk $ */
#include <stdio.h>


main(argc, argv)
int argc;
char *argv[];
{
  
   int i;
     for (i=0; i < 100; i++)
       printf("Testing SaveLines: This line number is %d \n", i);
}
