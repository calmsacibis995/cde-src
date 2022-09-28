/*
 * @(#)VALID.c	1.1 97/03/08
 *
 * Test for valid header.  Valid formats are:
 * 		From user Wed Dec  2 05:53 1992
 * BSD		From user Wed Dec  2 05:53:22 1992
 * SysV		From user Wed Dec  2 05:53 PST 1992
 * rn		From user Wed Dec  2 05:53:22 PST 1992
 *		From user Wed Dec  2 05:53 -0700 1992
 *		From user Wed Dec  2 05:53:22 -0700 1992
 *		From user Wed Dec  2 05:53 1992 PST
 *		From user Wed Dec  2 05:53:22 1992 PST
 *		From user Wed Dec  2 05:53 1992 -0700
 * SUN-OS	From user Wed Dec  2 05:53:22 1992 -0700
 * Other        From user Fri May 5 (see other_is_valid(char *s))
 * rfc822_valid From user Fri, 6 Mar 1996 10:00:00 PST
 *              From user Fri, 6 Mar 1996 10:00 PST
 *              From user Fri, 6 Mar 1996 10:00:00 -0800
 *              From user Fri, 6 Mar 1996 10:00 +0100
 *
 * It tries to validate all From headers with all date formats known to
 * be used (and several possible others).  After validating the "From "
 * part, it scans from the end of the string trying different formats in
 * the order shown above.  ti is left as the offset from end of the string
 * of the time, tz is the offset from the end of the string of the timezone
 * (if present, else zero).  Matters are  made hairier because there is
 * no deterministic way to parse the "user" field, which may contain
 * unquoted spaces!
 *
 * Validate line
 * Accepts: pointer to candidate string to validate as a From header (startPtr)
 *	    returns:
 *
 *		eol -		pointer to end of date/time field.
 *
 *	    	timeOffset -	pointer to offset from eol of
 *				time (hours of "mmm dd hh:mm")
 *
 *	   	zoneOffset -	pointer to offset from eol of time
 *				zone (if non-zero)
 *
 *	eol, timOffset, zoneOffset set if valid From string,
 *	else timeOffset is 0
 *
 * Why does it scan backwards from the end of the line, instead of doing the
 * much easier forward scan?  There is no deterministic way to parse the
 * "user" field, because it may contain unquoted spaces!  Yes, I tested it to
 * see if unquoted spaces were possible.  They are, and I've encountered enough * evil mail to be totally unwilling to trust that "it will never happen".
 */

#include "VALID.h"
#include <string.h>

extern int	other_is_valid(char *s, int *zn, int *ti);
extern int	rfc822_valid(char *s, char *x, int *ti, int *zn, int *rv);

void
VALID(char *	startPtr,
      char ** 	eol,
      int  *	timeOffset,	/* Returns ZERO (0) on failure */
      int  *	zoneOffset,
      int  *	rfc822Valid)
{
  /* Initializes the return timeOffset value to failure (0); */
  *timeOffset = 0;
  *rfc822Valid = 0;

  /* Validates that the 1st-5th characters are "From ". */
  if (strncmp("From ", startPtr, 5) == 0) {

    /* Validates that there is an end of line and points EOL at it. */
    *eol = strchr(startPtr + 5, '\n');
    if (*eol != NULL) {
      if (!other_is_valid(*eol, zoneOffset, timeOffset)
	  && !rfc822_valid(startPtr, *eol,timeOffset, zoneOffset,rfc822Valid)){

	/*
	 * First checks to see if the line is at least 41 characters long.
	 * If so, it scans backwards to find the rightmost space.  From
	 * that point, it scans backwards to see if the string matches
	 * " remote from".  If so, it sets EOL to point to the space at
	 * the start of the string.
	 */
	if ((*eol)[0]) {
	  if ((*eol) - startPtr >= 41) {
	    for (*zoneOffset = -1; (*eol)[*zoneOffset]!= ' '; (*zoneOffset)--);
	    if (strncmp(" remote from", &(*eol)[(*zoneOffset)-12], 12) == 0) {
	      *eol += (*zoneOffset) - 12;
	    }
	  }

	  /* Makes sure that there are at least 27 characters in the line. */
	  if ((*eol) - startPtr >= 27) {

	    /*
	     * Checks if the date/time ends with the year (there is a space
	     * five characters back).  If there is a colon three characters
	     * further back, there is no timezone field, so zoneOffset is set
	     * to 0 and timeOffset is set in front of the year.  Otherwise,
	     * there must either to be a space four characters back for a
	     * three-letter timezone, or a space six characters back followed
	     * by a + or - for a numeric timezone; in either case, zoneOffset
	     * and timeOffset become the offset of the space immediately
	     * before it.
	     */
	    if ((*eol)[-5] == ' ') {
	      if ((*eol)[-8] == ':') {
		*zoneOffset = 0;
		*timeOffset = -5;
	      } else if ((*eol)[-9] == ' ') {
		*timeOffset = *zoneOffset = -9;
	      } else if (((*eol)[-11] == ' ')
			 && (((*eol)[-10]=='+')  || ((*eol)[-10]=='-'))) {
		*timeOffset = *zoneOffset = -11;
	      }

	      /* 
	       * If there is a space four characters back, it is a
	       * three-letter timezone; there must be a space for
	       * the year nine characters back.  zoneOffset is the zone
	       * offset; timeOffset is the offset of the space.
	       */
	    } else if ((*eol)[-4] == ' ') {
	      if ((*eol)[-9] == ' ') {
		*zoneOffset = -4;
		*timeOffset = -9;
	      }

	      /*
	       * If there is a space six characters back, it is a numeric
	       * timezone; there must be a space eleven characters back
	       * and a + or - five characters back. zoneOffset is the zone
	       * offset; timeOffset is the offset of the space.
	       */
	    } else if ((*eol)[-6] == ' ') {
	      if (((*eol)[-11] == ' ')
		  && (((*eol)[-5] == '+') || ((*eol)[-5] == '-'))) {
		*zoneOffset = -6;	
		*timeOffset = -11;
	      }
	    }

	    /*
	     * If timeOffset is valid, make sure that the string before
	     * timeOffset is of the form
	     *
	     * 	www mmm dd hh:mm or www mmm dd hh:mm:ss
	     *
	     * otherwise invalidate timeOffset. There must be a colon
	     * three characters back and a space six or nine characters
	     * back (depending upon whether or not the character six
	     * characters back is a colon). There must be a space three
	     * characters further back (in front of the day), one seven
	     * characters back (in front of the month), and one eleven
	     * characters back (in front of the day of week).
	     * timeOffset is set to be the offset of the space before the time.
	     */
	    if (*timeOffset) {

	      if (((*eol)[(*timeOffset) - 3] != ':')
		  || ((*eol)[(*timeOffset)
			 -= (((*eol)[(*timeOffset) - 6] == ':') ? 9 : 6)]
		      != ' ')) (*timeOffset) = 0;

	      if ((*eol)[(*timeOffset) - 3] == ' ') {
		if ((*eol)[(*timeOffset)-7] != ' '
		    ||(*eol)[(*timeOffset)-11] !=' ') {
		  *timeOffset = 0;
		}
	      } else if ((*eol)[(*timeOffset) - 2] == ' ') {
		if ((*eol)[(*timeOffset)-6] != ' '||(*eol)[(*timeOffset)-10]!= ' ') {
		  *timeOffset = 0;
		}
	      } else {
		*timeOffset = 0;
	      }
	    }
	  }
	}
      }
    }
  }
  return;
}
