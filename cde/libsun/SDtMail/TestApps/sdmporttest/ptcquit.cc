#include <porttest.hh>
#include <Utils/LockUtility.hh>
#include <ptcquit.hh>
#include <ptclock.hh>

char *Hquit = "Usage: quit\nExit program.\n";

/*
** FORWARD DECLARATIONS
*/


/*************************************************************************
 *
 * Function:	Cquit - save data and exit program
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 *		IF number of arguments == 0, assume we are being executed
 *		as a single command line argument.
 *
 * Output:	TRUE - user aborted quit, return to command mode
 *		FALSE - user confirmed quit, exit program
 *
 ************************************************************************/

int Cquit(int argc, char **argv)
{
  
  assert(argc >= 0);
  
  /* While asking questions, allow user to interrupt out
   */
  
  AllowInterrupts(TRUE);
  
  /* Determine if data base has been modified, and if so, does the
  ** user want the data to be written
  */

  DestroyAllLocks();

  AllowInterrupts(FALSE);
  return(FALSE);
  
  /*NOTREACHED*/
}
