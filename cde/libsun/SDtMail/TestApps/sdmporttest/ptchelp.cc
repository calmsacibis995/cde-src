#include <porttest.hh>
#include <ptchelp.hh>
#include <ptcmdscan.hh>

/*
** EXTERNALS
*/

/*
** GLOBALS
*/
char *Hhelp =
"Usage: help [ command-name ]\nGives help on specified command, or lists valid commands if none specified.\n";

/*
** FORWARD DECLARATIONS
*/
static int	generalHelp(CMD* cmdtab);

/*************************************************************************
 *
 * Function:	Chelp - give general or specific help as requested
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 * Output:	TRUE - help given
 *
 ************************************************************************/

int Chelp(int argc, char **argv)
{
  register CMD *c;
  
  if (argc == 1)		/* Only help given on command line? */
    return(generalHelp(maincmdtab));
  
  /* Help on a specific command or commands requested
   */
  
  while (--argc > 0) 
    {
      register char *arg;
      arg = *++argv;
      c = getcmd(arg, maincmdtab);
      if (c == (CMD *)-1)
	printf("?Ambiguous help command %s\n", arg);
      else if (c == (CMD *)0)
	printf("?Invalid help command %s\n", arg);
      else
	printf("Command name: %s\n%s\n",c->c_name, c->c_help);
    }
  return(TRUE);
}

/*************************************************************************
 *
 * Function:	generalHelp - give list of valid commands and hints
 *
 * Input:	NONE
 *
 * Output:	NONE - general help given
 *
 ************************************************************************/

static int generalHelp(CMD* cmdtab)
{
  register int i, j, w;
  int columns, width = 0, lines, ncmds = 0;
  CMD *c;
  
  printf("Commands may be abbreviated to the leftmost unique subset. Commands are:\n\n");
  for (c = cmdtab; c->c_name != NULL; c++)
    {
      int len;
      len = strlen(c->c_name);
      ncmds++;
      if (len > width)
	width = len;
    }
  width = (width + 8) &~ 7;
  columns = 80 / width;
  if (columns == 0)
    columns = 1;
  lines = (ncmds + columns - 1) / columns;
  for (i = 0; i < lines; i++) 
    {
      for (j = 0; j < columns; j++) 
	{
	  c = cmdtab + j * lines + i;
	  printf("%s%c", c->c_name, c->c_nocmdline ? ' ':'*');
	  if (c + lines >= &cmdtab[ncmds])
	    {
	      printf("\n");
	      break;
	    }
	  w = strlen(c->c_name);
	  while (w < width)
	    {
	      w = (w + 8) &~ 7;
	      (void) putchar('\t');
	    }
	}
    }
  
  printf("\n"
	 "A command may be issued to the shell by typing '!' followed by the command\n"
	 "\n"
	 "Commands followed by an '*' may be issued on the command line.\n"
	 "\n"
	 "You may type 'help' followed by a command name for specific information\n"
	 "on that particular command (e.g. help help)\n"
	 "\n"
	 "You may type '%s -help' at the UNIX command level for information about\n"
	 "startup options.\n"
	 ,Command);
  return(TRUE);
}
