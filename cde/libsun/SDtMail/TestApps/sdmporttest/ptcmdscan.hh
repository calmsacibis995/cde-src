/*
** Global Includes
*/

/*
** Global Defines
*/

int cmdScanner();
int oneCommand(char *pcmd);
CMD *getcmd(char *name, CMD *cmdtab);
SUBCMD *getsubcmd(char *cmdname, SUBCMD *subcmdtab);
int oneSubCommand(char *cmdname, char *subcmdname, SUBCMD *subcmdtab, int argc, char **argv);
char *getCommandString(char *s);

extern CMD maincmdtab[];

/*
** Global Structs
*/
