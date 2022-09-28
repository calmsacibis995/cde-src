/*
 * File:              dtsession_res
 *
 * Default Location:  /usr/dt/bin/dtsession_res
 *
 * Purpose:           Load the RESOURCE_MANAGER with desktop resources
 *
 * Description:       This is invoked to load or reload the 
 *                    RESOURCE_MANAGER from the desktop resource files.
 *
 * Invoked by:        The desktop Session Manager at session startup, 
 *                    or user by means of 'dtaction LoadResources'.
 *                    This should not be invoked directly.
 *
 * (c) Copyright 1997 Sun Microsystems, Inc.
 */

#include <stdio.h>
#include <stdlib.h>             
#include <string.h>            
#include <locale.h>            
#include <nl_types.h>            
#include <sys/param.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xresource.h>

#ifndef XRDB_PROGRAM
#ifdef sun
#define XRDB_PROGRAM "/usr/openwin/bin/xrdb"
#else
#define XRDB_PROGRAM "/usr/bin/X11/xrdb"
#endif /* sun */
#endif

#define needArgs 2
#define xrdbOptDone 3
#define needFile 4
#define unknownArg 5
#define arrayOver 6

#if !defined(NL_CAT_LOCALE)
#define NL_CAT_LOCALE 0
#endif
#define NLS_CATALOG "dtsession"
/* dtsession_res set number in dtsession.msg.src */
#define MC_DTSESSION_RES_SET 50
/*
 * Desktop message catalogs are in dt directory, so append desktop
 * search paths to current NLSPATH.
 */
#define NLS_PATH_STRING  CDE_INSTALLATION_TOP "/nls/msg/%L/%N.cat:" \
    CDE_INSTALLATION_TOP "/lib/nls/msg/%L/%N.cat:" \
    CDE_INSTALLATION_TOP "/lib/nls/msg/%l/%t/%c/%N.cat:" \
    CDE_INSTALLATION_TOP "/lib/nls/msg/%l/%c/%N.cat"

#define GETRESOURCE(x) \
    t = strstr(buf, x); \
    if (t) { \
        /* get to = */ \
        t = strstr(t, "="); \
        if (t) { \
            t++; \
            while (isspace(*t)) \
                t++; \
            /* check for control character */ \
            if (t[0] == '^') { \
                resourceString[strlen(resourceString)] = ' '; \
                strcat(resourceString, x); \
                resourceString[strlen(resourceString)] = ' '; \
                for (j = 0;!isspace(t[j]) && t[j] != ';';j++) \
                    resourceString[strlen(resourceString)] = t[j]; \
            } \
        } \
    } \


/* Program Globals */

char ttyModes[100];
nl_catd nl_fd;


/*
 * Message Catalog Read Procedure 
 */

unsigned char *
#ifdef _NO_PROTO
ReadCatalog(set_num, msg_num, def_str)
    int         set_num;        /* message catalog set number              */
    int         msg_num;        /* message catalog message number          */
    char        *def_str;       /* default string                          */
#else
ReadCatalog( int set_num, int msg_num, char *def_str )
#endif /* _NO_PROTO */
{
    static Bool alreadyopen = False;
    char *s;

    if (alreadyopen == False) {
        char *curNlsPath, *newNlsPath;
        int newNlsPathLen;

        alreadyopen = True;

        curNlsPath = getenv("NLSPATH");
        if (curNlsPath && strlen(curNlsPath) == 0)
            curNlsPath = NULL;
 
        /*
         * 7 is "NLSPATH"
         * 1 is "="
         * <length of NLS_PATH_STRING>
         * 1 for null byte
         */
        newNlsPathLen = 7 + 1 + strlen(NLS_PATH_STRING) + 1;

        if (curNlsPath != NULL) {
            /*
             * 1 is ":"
             * <length of curNlsPath>
             */
            newNlsPathLen += (1 + strlen(curNlsPath));
        }

        /* placed in environ, do not free */
        newNlsPath = malloc(newNlsPathLen); 
      
        if (curNlsPath != NULL) {
            sprintf(newNlsPath, "NLSPATH=%s:%s", curNlsPath, NLS_PATH_STRING);
        }
        else
            sprintf(newNlsPath, "NLSPATH=%s", NLS_PATH_STRING);

        /*
         * Store new NLSPATH in environment. Note this memory cannot be freed
         */
        putenv(newNlsPath);

       /*
        * Open message catalog. Note, if invalid descriptor returned (ie
        * msg catalog could not be opened), subsequent call to catgets() using
        * that descriptor will return 'def_str'.
        */
       nl_fd = catopen("dtsession", NL_CAT_LOCALE);
    }

    s = catgets(nl_fd, set_num, msg_num, def_str);

    return((unsigned char *) s);
}

/*
 * Exit Procedure 
 */

static void
dtsession_res_exit(int code) {

    if (ttyModes[0] != '\0' && unlink(ttyModes) == -1) {
        fprintf(stderr, "dtsession_res: unlink() on %s failed\n", ttyModes);
    }

    exit(code);
}

/*
 * Usage Statement Procedure 
 */

void
#ifdef _NO_PROTO
Usage(i)
int i;		/* set number */
#else
Usage(int i)
#endif
{
    fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET, 1,
        "dtsession_res: -load|-merge [-system] [-tty] [-xdefaults] [-file <name>]\n"));
    switch (i) { 
        case needArgs:
            fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET, 
               needArgs, "Options -load xxx or -merge xxx must be specified\n"));
            break;
        case xrdbOptDone:   
            fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET,
                xrdbOptDone, "Option -load or -merge already specified\n"));
            break;
        case needFile:      
            fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET,
                needFile, "Missing <filename> after -file option\n"));
            break;
        case unknownArg:    
            fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET,
                unknownArg, "Unknown option specified\n"));
            break;
        case arrayOver:    
            fprintf(stderr, "%s", ReadCatalog(MC_DTSESSION_RES_SET,
                arrayOver, "Array overflow\n"));
        default:     
            break;
    }

    dtsession_res_exit(1);
}

/*
 *  Main Program  
 */

void
#ifdef _NO_PROTO
main(argc,argv)
int argc;		/* # of arguments */
char *argv[];		/* argument strings */
#else
main(int argc, char **argv)
#endif
{

    char cmd[PIPE_MAX];
    int unkArg = 1, i;
    int load = 0, merge = 0;
    char rOWsystem[MAXPATHLEN];
    char rOWdefaults[MAXPATHLEN];
    char rFactory[MAXPATHLEN];
    char rFactoryC[MAXPATHLEN];
    char rCustom[MAXPATHLEN];
    char rXdefaults[MAXPATHLEN];
    char *owhome, *home, *display, *lang;
    char tmpname[50];

    char *newNlsPath;


    struct stat statbuf;
    int results, fd, j;
    char *buf, *t, resourceString[BUFSIZ];

    int num_xrdb_files = 0;
    char *xrdb_files[100];

    ttyModes[0] = '\0';

    if (argc == 1)
        Usage(needArgs);

    if ((display = getenv("DISPLAY")) == NULL) {
          fprintf(stderr, "No display variable specified\n");
          display = (char *) malloc(3*sizeof(char));
          strcpy(display, ":0");
    }

    if ((owhome = getenv("OPENWINHOME")) != NULL)
          sprintf(rOWsystem, "%s%s", owhome, "/lib/Xdefaults");

    if ((home = getenv("HOME")) != NULL ) {
          sprintf(rOWdefaults, "%s%s", home,"/.OWdefaults");
          sprintf(rXdefaults, "%s%s", home, "/.Xdefaults");
    }

    if ((lang = getenv("LANG")) != NULL ){
          sprintf(rFactory,
              "%s%s%s", "/usr/dt/config/", lang, "/sys.resources");
          sprintf(rCustom, "%s%s%s", "/etc/dt/config/", lang, "/sys.resources");
    }
    
    if (access("/usr/dt/config/C/sys.resources", R_OK) == NULL )
          sprintf(rFactoryC, "%s", "/usr/dt/config/C/sys.resources");


    i = 1;

    while (i != argc) {
        if ((strcmp(argv[i], "-tty") == NULL)) {

            
            /*
             * populate _sttyFile in /tmp with output of stty -a
             * read it into a buffer
             * look for intr, quit, erase, kill, eof, eol,
             *     swtch, start, stop, susp, dsusp, rprnt, flush,
             *     weras and lnext which dtterm understands
             * set up *ttyMode with name value pair
             * For example, from /usr/dt/config/C/sys.resources,
             *     *ttyModes:erase ^H intr ^C kill ^U start ^Q stop ^S
             *         swtch ^@ susp ^Z
             */

            strcpy(tmpname,"/tmp/_sttyFile_XXXXXX");
            (void) mktemp(tmpname);
            sprintf (cmd, "/usr/bin/stty -a > %s", tmpname);
            system(cmd);
                results = stat(tmpname, &statbuf);
                if (results != 0) {
                    fprintf(stderr, "Couldn't open %s\n", tmpname);
                    dtsession_res_exit(1);
                }
                        buf = (char *) malloc(statbuf.st_size);
                fd = open(tmpname, O_RDWR);
                read(fd, buf, statbuf.st_size);
		close(fd);

                /*
                 * parse the buffer
                 */

                memset(resourceString, '\0', BUFSIZ);
                strcat(resourceString, "*ttyModes:");
                GETRESOURCE("intr");
                GETRESOURCE("quit");
                GETRESOURCE("erase");
                GETRESOURCE("kill");
                GETRESOURCE("eof");
                GETRESOURCE("eol");
                GETRESOURCE("swtch");
                GETRESOURCE("start");
                GETRESOURCE("stop");
                GETRESOURCE("susp");
                GETRESOURCE("dsusp");
                GETRESOURCE("rprnt");
                GETRESOURCE("flush");
                GETRESOURCE("weras");
                GETRESOURCE("lnext");

            unlink(tmpname);

            strcpy(ttyModes,"/tmp/ttyModes_XXXXXX");
            (void) mktemp(ttyModes);

            fd = open(ttyModes, O_CREAT | O_RDWR);
            sprintf(cmd, "chmod +rw %s", ttyModes);
            system(cmd);
            resourceString[strlen(resourceString)] = '\n';
            write(fd, resourceString, strlen(resourceString));
	    close(fd);

	    xrdb_files[num_xrdb_files++] = strdup(ttyModes);

            unkArg = 0;
        }

        if ((strcmp(argv[i],"-load") == NULL)) {
                 if (!load)
                      load = True; 
                 else
                      Usage(xrdbOptDone);

              unkArg = 0;
          }

        if ((strcmp(argv[i],"-merge") == NULL)) {
            if (!merge)
                  merge = True;
            else
                  Usage(xrdbOptDone);
    
              unkArg = 0;
        }

        if (strcmp(argv[i],"-system") == NULL)  {
            if (access(rOWsystem, R_OK) == NULL) {
	        xrdb_files[num_xrdb_files++] = strdup(rOWsystem);
            }

            if (access(rFactory, R_OK) == NULL) {
	        xrdb_files[num_xrdb_files++] = strdup(rFactory);
            }
            else if(access(rFactoryC,R_OK) == NULL){
	        xrdb_files[num_xrdb_files++] = strdup(rFactoryC);
            }

            if (access(rCustom, R_OK) == NULL){
	        xrdb_files[num_xrdb_files++] = strdup(rCustom);
            }
            unkArg = 0;
        }

        if (strcmp(argv[i],"-xdefaults") == NULL)  {
            if (access(rXdefaults,R_OK) == NULL){
	        xrdb_files[num_xrdb_files++] = strdup(rXdefaults);
            }

            if (access(rOWdefaults,R_OK)){
	        xrdb_files[num_xrdb_files++] = strdup(rOWdefaults);
            }
            unkArg = 0;
        }

        if (strcmp(argv[i],"-file") == NULL) {
            ++i;
            if (access(argv[i], R_OK) == NULL) {
	        xrdb_files[num_xrdb_files++] = strdup(argv[i]);
            }
            else
                Usage(needFile);
            unkArg = 0;
        }

        ++i;
    }

    if(unkArg || (!load && !merge) || num_xrdb_files == 0 ) {
      Usage(needArgs); /* usage statement */
    }

    {
	char* lm_opt; 

	if (load) {
	    lm_opt = strdup("-load");
	}
	else {
	    lm_opt = strdup("-merge");
	}

	cmd[0] = '\0';

	strcat(cmd, "/usr/bin/cat ");

	for (i=0; i<num_xrdb_files; i++) {

	    if (strlen(cmd) + strlen(xrdb_files[i]) + 2 > sizeof(cmd)) {
		Usage(arrayOver);
	    }

	    strcat(cmd, xrdb_files[i]);
	    strcat(cmd, " ");
	}

        snprintf (&cmd[strlen(cmd)], sizeof(cmd)-strlen(cmd)-1, 
		 " | %s -quiet -display %s %s", 
		 XRDB_PROGRAM, display, lm_opt);

	/* Execute the actual xrdb call */

        system(cmd);
    }

    dtsession_res_exit(0);
}
