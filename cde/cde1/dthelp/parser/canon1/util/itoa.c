/* $XConsortium: itoa.c /main/cde1_maint/1 1995/07/17 21:29:04 drk $ */
/* From example in Kernighan and Ritchie, The C Programming Language,
   Prentice-Hall, 1978 */

#include <string.h>
#include "basic.h"

char *m_itoa(
#if defined(M_PROTO)
  int n, char *s
#endif
  ) ;

void reverse(
#if defined(M_PROTO)
  char *s
#endif
  ) ;

char *m_itoa(n, s)  /* convert n to characters in s */
char s[]; 
int n;
{   
    int sign ;
    char *p = s ;

    if ( (sign = n) < 0 ) /* record sign */
        n = -n;
    do {    /* generate digits in reverse order */
        *p++ = (char) (n % 10 + '0') ;
    }  while (( n/= 10) > 0);
    if (sign < 0)
        *p++ = '-';
    *p = '\0';

    reverse(s);
    return(s) ;
}

void reverse(s)
char s[];
{
    int c, i, j;

    for (i=0, j=strlen(s)-1; i < j ; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = (char) c;
    }
}
