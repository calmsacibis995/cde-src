/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/* strdup - make duplicate of string s */
#include <string.h>
#include <malloc.h>

char *
strdup(char *s)
{
    char *p;

    p = (char *)malloc(strlen(s) + 1);  /* + 1 for '\0' character */
    if (p != NULL) 
        strcpy(p,s);
    return p;
}
