#ifndef lint
static char sccsid[] = "@(#)deskset.c 1.4 95/01/06";
#endif

/*
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#include <pwd.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>

#define  GETHOSTNAME  (void) gethostname
#define  SPRINTF      (void) sprintf

#define  DS_MAX_STR   120          /* Maximum length for character strings. */




/*  Function:     ds_get_resource()
 *
 *  Purpose:      Get an X resource from the server.
 *
 *  Parameters:   rDB          X resources database.
 *
 *                appname      application name.
 *
 *                resource     X resource string to search for.
 *
 *  Returns:      resource string, or NULL if not found.
 *
 *  Note:         The first character of the appname and resource strings may
 *                be modified.
 */

char *
ds_get_resource(rDB, appname, resource)
XrmDatabase rDB ;                         /* Resources database. */
char *appname ;                           /* Application name. */
char *resource ;                          /* X resource to search for. */
{
  char cstr[DS_MAX_STR], nstr[DS_MAX_STR], str[DS_MAX_STR] ;
  char *str_type[20] ;
  XrmValue value ;

  if (isupper(appname[0])) appname[0] = tolower(appname[0]) ;
  SPRINTF(nstr, "deskset.%s.%s", appname, resource) ;

  if (islower(resource[0])) resource[0] = toupper(resource[0]) ;
  if (islower(appname[0])) appname[0] = toupper(appname[0]) ;
  SPRINTF(cstr, "Deskset.%s.%s", appname, resource) ;

  if (XrmGetResource(rDB, nstr, cstr, str_type, &value) == NULL)
    return((char *) NULL) ;
  else return(value.addr) ;
}


/*  Function:     ds_load_deskset_defs()
 *
 *  Purpose:      Load the Deskset resources database from the
 *                DESKSETDEFAULTS environment variable (if set), or
 *                from $HOME/.desksetdefaults.
 *
 *  Parameters:   None.
 *
 *  Returns:      X resources database.
 */

XrmDatabase
ds_load_deskset_defs()
{
  XrmDatabase rDB ;
  char name[MAXPATHLEN], *ptr ;

  if ((ptr = getenv("DESKSETDEFAULTS")) == NULL)
    {
      SPRINTF(name, "%s/.desksetdefaults", getenv("HOME")) ;
      rDB = XrmGetFileDatabase(name) ;
    }
  else rDB = XrmGetFileDatabase(ptr) ;
  return(rDB) ;
}


/*  Function:     ds_load_resources()
 *
 *  Purpose:      Get the resource databases. These are looked for in the
 *                following ways:
 *
 *                Classname file in the app-defaults directory. In this case,
 *                Classname is Deskset.
 *
 *                Classname file in the directory specified by the
 *                XUSERFILESEARCHPATH or XAPPLRESDIR environment variable.
 *
 *                Property set using xrdb, accessible through the
 *                XResourceManagerString macro or, if that is empty, the
 *                ~/.Xdefaults file.
 *
 *                XENVIRONMENT environment variable or, if not set,
 *                .Xdefaults-hostname file.
 *
 *                DESKSETDEFAULTS environment variable or, if not set, the
 *                ~/.desksetdefaults file
 *
 *  Parameters:   display    connection to the X server.
 *                           (returned from XOpenDisplay).
 *
 *  Returns:      X combined resources database.
 */

XrmDatabase
ds_load_resources(display)
Display *display ;
{
  XrmDatabase db, rDB ;
  char *home, name[MAXPATHLEN], *owhome, *ptr ;
  int len ;

  rDB  = NULL ;
  home = getenv("HOME") ;
  XrmInitialize() ;
  if ((owhome = getenv("OPENWINHOME")) != NULL)
    {
      SPRINTF(name, "%s/lib/app-defaults/Deskset", getenv("OPENWINHOME")) ;

/* Get applications defaults file, if any. */

      db = XrmGetFileDatabase(name) ;
      XrmMergeDatabases(db, &rDB) ;
    }

/* Merge server defaults, created by xrdb. If nor defined, use ~/.Xdefaults. */

  if (XResourceManagerString(display) != NULL)
    db = XrmGetStringDatabase(XResourceManagerString(display)) ;
  else
    { 
      SPRINTF(name, "%s/.Xdefaults", home) ;
      db = XrmGetFileDatabase(name) ;
    }
  XrmMergeDatabases(db, &rDB) ;

/*  Open XENVIRONMENT file or, if not defined, the .Xdefaults, and merge
 *  into existing database.
 */

  if ((ptr = getenv("XENVIRONMENT")) == NULL)
    {
      SPRINTF(name, "%s/.Xdefaults-", home) ;
      len = strlen(name) ;
      GETHOSTNAME(name+len, 1024-len) ;
      db = XrmGetFileDatabase(name) ;
    }
  else db = XrmGetFileDatabase(ptr) ;
  XrmMergeDatabases(db, &rDB) ;

/*  Finally merge in Deskset defaults via DESKSETDEFAULTS or, if not
 *  defined, the ~/.desksetdefaults file.
 */

  if ((ptr = getenv("DESKSETDEFAULTS")) == NULL)
    {
      SPRINTF(name, "%s/.desksetdefaults", home) ;
      db = XrmGetFileDatabase(name) ;
    }
  else db = XrmGetFileDatabase(ptr) ;
  XrmMergeDatabases(db, &rDB) ;
  return(rDB) ;
}


/*  Function:     ds_put_resource()
 *
 *  Purpose:      Adds an X resource string (name and value) to a resources
 *                database.
 *
 *  Parameters:   rDB          X resources database.
 *
 *                appname      application name.
 *
 *                rstr         X resource string name.
 *
 *                rval         X resource string value.
 *
 *  Returns:      None.
 *
 *  Note:         The first character of the appname and resource strings may
 *                be modified.
 */

void
ds_put_resource(rDB, appname, rstr, rval)
XrmDatabase *rDB ;
char *appname, *rstr, *rval ;
{
  char resource[DS_MAX_STR] ;

  if (isupper(appname[0])) appname[0] = tolower(appname[0]) ;
  SPRINTF(resource, "deskset.%s.%s", appname, rstr) ;
  XrmPutStringResource(rDB, resource, rval) ;
}



/*  Function:     ds_save_resources()
 *
 *  Purpose:      Save away the resources database to the file given by the
 *                DESKSETDEFAULTS environment variable (if set), or
 *                to $HOME/.desksetdefaults.
 *
 *  Parameters:   rDB        X resources database to save.
 *
 *  Returns:      XV_ERROR   if cannot access resource database to write.
 *                XV_OK      on successful completion.
 */


int
ds_save_resources(rDB)
XrmDatabase rDB ;
{
	char *home, *filename;
	struct  stat    statbuf;

	if ((filename = getenv("DESKSETDEFAULTS")) == NULL) {
		home = getenv("HOME");
		filename = (char*)calloc(1, strlen(home)+18);
      		sprintf(filename, "%s/.desksetdefaults", home);
	}
	/* if file exists but user does not have access*/
	if (stat(filename, &statbuf) != -1 && access(filename, W_OK) != 0) { 
		free(filename);
		return(1);
	}
	/* if file does not exist this call will create it*/
	XrmPutFileDatabase(rDB, filename);
	free(filename);
  	return(0);
}


/******************************************************************************
**
**  Function:		ds_expand_pathname
**
**  Description:	Expand ~'s and environment variables in a path.
**		
**			This routine was stolen from filemgr. I've made a
**			couple of small changes to better handle "~user"
**			at the start of a path.
**
**  Parameters:		path	Unexpanded path
**			bug	Returned expanded path.  Caller is responsible
**				allocating enough space.
**
**  Returns:		Nothing
**
******************************************************************************/
extern void
ds_expand_pathname(char *path, char *buf)
{
	register char *p, *b_p, *e_p;	/* String pointers */
	char *save_p;			/* Point in path before env var */
	char env[255];			/* Environment variable expansion */
	struct passwd *pw;		/* Password file entry */

	p = path;
	if (*p == '~')
	{
		p++;
		if (*p && *p != '/')
		{
			/* Somebody else's home directory? */
			if (b_p = (char *) strchr(p, '/'))
				*b_p = '\0';
			if (pw = getpwnam(p)) {
				(void) strcpy(buf, pw->pw_dir);
			} else {
				*buf = '~';
				(void) strcpy(buf + 1, p);
			}

			if (b_p)  {
				*b_p = '/';
				p = b_p;
			} else
				return;

		}
		else
			(void) strcpy(buf, (char *) getenv("HOME"));
		b_p = buf + strlen(buf);
	}
	else
		b_p = buf;

	while (*p)
		if (*p == '$')
		{
			/* Expand environment variable */
			save_p = p;
			e_p = env;
			p++;
			while ((isalnum(*p) || *p == '_') && e_p - env < 255)
				*e_p++ = *p++;
			*e_p = NULL;
			if (e_p = (char *) getenv(env))
				while (*e_p)
					*b_p++ = *e_p++;
			else
			{
				p = save_p;
				*b_p++ = *p++;
			}
		}
		else 
			*b_p++ = *p++;
	*b_p = NULL;
}

/*
 * Detects if double click event occurred
 */
int
ds_is_double_click(last_event, event )
XEvent           *last_event;
XEvent           *event;
{
        static  int             time_threshold;
        static  int             dist_threshold;
        static  short           first_time      = True;
        short                   ret_value       = False;
        int                     delta_time;
        int                     delta_x, delta_y;

        if (last_event == NULL || event == NULL)
                return ret_value;

        /* first time this is called init the thresholds */
        if( first_time ) {
                /* Get time threshold in miliseconds */
		time_threshold = 800;
		dist_threshold = 4;
                first_time      = False;
        }
        /* only deal with the down events */
	if (event->type == ButtonRelease)
		return ret_value;

	if ((event->xbutton.type == ButtonPress) &&
	    (last_event->xbutton.type == ButtonPress)) {
 
		delta_time = event->xbutton.time - last_event->xbutton.time;
		delta_time += event->xbutton.time;
		delta_time -= last_event->xbutton.time;
 
                /* is the time within bounds? */
                if( delta_time <= time_threshold ) {
 
                        /* check to see if the distance is ok */
			delta_x = (last_event->xbutton.x > event->xbutton.x ?
	 				   last_event->xbutton.x - event->xbutton.x :
		 			   event->xbutton.x - last_event->xbutton.x);

			delta_y = (last_event->xbutton.y > event->xbutton.y ?
	 				   last_event->xbutton.y - event->xbutton.y :
		 			   event->xbutton.y - last_event->xbutton.y);
                        if( delta_x <= dist_threshold &&
                            delta_y <= dist_threshold )
                                ret_value       = True;
                }
        }
        return ret_value;
}        

