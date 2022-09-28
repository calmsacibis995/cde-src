
/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)defaults.c	1.3 96/06/19 SMI"


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>

#include "engine.h"
#include "defaults.h"

#define	RES_FILE		"sdtspell.res"
#define	RES_DFLT_DICTIONARY	"sdtspell.defaultDictionary"
#define	RES_USE_PERSONAL	"sdtspell.usePersonal"
#define RES_FOLLOW_LOCALE	"sdtspell.followLocale"


#define EXT_SIZE	7

char *_pd_filename_extensions[] = {
	LOCALE_CA,
	LOCALE_EN_AU,
	LOCALE_EN_UK,
	LOCALE_EN_US,
	LOCALE_DE,
	LOCALE_DE_CH,
	LOCALE_ES,
	LOCALE_FR,
	LOCALE_IT,
	LOCALE_SV,
	LOCALE_EN_US
};


/* Function	:	_SDtSpellGetDefaultsDir()
 | 
 | Objective	:	Get the default directory where the spell checker
 |			will store its personal dictionaries and resource
 |			database.
 |
 | Arguments	:	none.
 |
 | Return value	:	char *dir    - the default directory path; since
 |				       this variable's space is allocated,
 |				       the calling function should
 |				       explicitly free the memory
 */

char *
_SDtSpellGetDefaultsDir(void)
{
	char	*home = (char *) NULL;
	char	*directory = (char *) NULL;


	/* Default directory is $HOME/PD_PATH.  If $HOME isn't define, 
	 | then assume the current working directory.
	 */
	home = getenv("HOME");
	if (home) {

		directory = (char *) calloc(strlen(home) + strlen(PD_PATH) + 5,
					    sizeof(char));
		sprintf(directory, "%s%s", home, PD_PATH);

	} else {

		directory = (char *) calloc(strlen(PD_PATH) + 5, sizeof(char));
		sprintf(directory, ".%s", PD_PATH);

	}
	return(directory);
}


/* Function	:	_SDtSpellGetDesksetDB()
 |
 | Objective	:	Establish an X resource database based on the
 |			.desksetdefaults file.
 |
 | Arguments	:	XrmDatabase *	- placeholder for the X resource
 |					  database.
 |			char *		- filename of .desksetdefaults (space
 |					  should be allocated by the calling
 |					  function)
 |
 | Return value	:	None.
 */

void
_SDtSpellGetDesksetResFile(XrmDatabase *db, char *file)
{
	char           *home;	  /* $HOME environment variable. */
	char           *deskset;  /* $DESKSETDEFAULTS env variable. */
	char           *path;


	/* Get the directory path to where the personal dictionaries
	 | are kept - this directory also contains the resource file.
	 */
	path = _SDtSpellGetDefaultsDir();


	/* Construct the filename. */
	sprintf(file, "%s/%s", path, RES_FILE);
	free(path);


	/* Get the resources from the .desksetdefaults file. */
	XrmInitialize();
	*db = XrmGetFileDatabase(file);
}


/* Function	:	_SDtSpellGetDefaults()
 | 
 | Objective	:	Retrieve the usePersonal, followLocale, and 
 |			defaultDictionary resources.
 |
 | Arguments	:	SDtSpellResP	- resource variable that will take
 |					  in the resources read from the
 |					  $HOME/.desksetdefaults or
 |					  $DESKSETDEFAULTS file.
 |
 | Return value	:	None.
 */

void
_SDtSpellGetDefaults(SDtSpellResP resource)
{
	XrmDatabase     db = (XrmDatabase) NULL;  /* X resource databse. */
	char		file[MAXPATHLEN];
	char		class[MAXPATHLEN];
	char           *str_type;
        XrmValue	value;


	/* Get standard spell resource file in order to initialize
	 | an X resource database.  If it doesn't exist, then
	 | assume the following defaults:
         |      XmNusePersonal	      = True
	 |	followLocale          = True
	 | 	XmNdefaultDictionary  = NULL
	 */
	_SDtSpellGetDesksetResFile(&db, file);
	if (!db) {
		resource->usePersonal = True;
		resource->followLocale = True;
		resource->defaultDictionary = (char *) NULL;
		return;
	}


	/* Get the defaultDictionary resource. */
	XrmGetResource(db, RES_DFLT_DICTIONARY, class, &str_type, &value);
	if (value.addr) 
		resource->defaultDictionary = strdup(value.addr);
	else
		resource->defaultDictionary = (char *) NULL;


	/* Get the usePersonal resource. */
	XrmGetResource(db, RES_USE_PERSONAL, class, &str_type, &value);
	if (value.addr) {
		if (strcmp(value.addr, "True") == 0 ||
		    strcmp(value.addr, "true") == 0 ||
		    strcmp(value.addr, "TRUE") == 0)
			resource->usePersonal = True;
		else
			resource->usePersonal = False;
	}
	else
		resource->usePersonal = True;


	/* Get the followLocale resource. */
	XrmGetResource(db, RES_FOLLOW_LOCALE, class, &str_type, &value);
	if (value.addr) {
		if (strcmp(value.addr, "True") == 0 ||
		    strcmp(value.addr, "true") == 0 ||
		    strcmp(value.addr, "TRUE") == 0)
			resource->followLocale = True;
		else
			resource->followLocale = False;
	}
	else
		resource->followLocale = True;


	/* Destroy the database. */
	XrmDestroyDatabase(db);
}


/* Function	:	_SDtSpellSaveDefaults()
 | 
 | Objective	:	Save the XmNusePersonal and XmNdictionary resource
 |			values into the user's $HOME/.desksetdefaults
 |			file.
 |
 | Arguments	:	SDtSpellResR	- variable with resources that are
 |					  to be saved in the user's
 |					  $HOME/.desksetdefaults or
 |					  $DESKSETDEFAULTS file.
 | Return value	:	None.
 */

void
_SDtSpellSaveDefaults(SDtSpellResR resource)
{
	XrmDatabase     db = (XrmDatabase) NULL;  /* X resource databse. */
	char		file[MAXPATHLEN];


	/* Get the .desksetdefaults file. */
	_SDtSpellGetDesksetResFile(&db, file);


	/* Put the defaultDictionary resource into the database. */
	if (resource.defaultDictionary != (char *) NULL)
		XrmPutStringResource(&db, RES_DFLT_DICTIONARY, 
				     resource.defaultDictionary);


	/* Put the usePersonal resource into the database. */
	if (resource.usePersonal)
		XrmPutStringResource(&db, RES_USE_PERSONAL, "True");
	else
		XrmPutStringResource(&db, RES_USE_PERSONAL, "False");


	/* Put the followLocale resource into the database. */
	if (resource.followLocale)
		XrmPutStringResource(&db, RES_FOLLOW_LOCALE, "True");
	else
		XrmPutStringResource(&db, RES_FOLLOW_LOCALE, "False");


	/* Save the database to the .desksetdefaults file. */
	XrmPutFileDatabase(db, file);


	/* Destroy the database. */
	XrmDestroyDatabase(db);
}


/* Function	:	_SDtSpellGetDefaultPDFilename()
 | 
 | Objective	:	Get the filename of the user's personal dictionary
 |			that will be used.  The personal dictionary file
 |			name comes in the form
 |				sdtspell_personal.<LANGUAGE>
 |			where <LANGUAGE> refers to the language dictionary
 |			extension associated with the language argument.
 |
 | Arguments	:	char *language	- language string that determines
 |					  what language dictionary to use
 |
 | Return value	:	char *filename	- the personal dictionary filename;
 |					  space for this has been allocated;
 |					  calling function must explicitly
 |					  free this return value
 */

void
_SDtSpellGetDefaultPD(char *language, char **path, char **file)
{
	int	index;


	/* Get the language type. */
	index = _getLanguage(language);


	/* Get the directory path to the personal dictionary. */
	*path = _SDtSpellGetDefaultsDir();


	/* Create the personal dictionary filename.  The personal
	 | dictionary filename is of the form
	 |	PD_BASENAME.language
	 */
	*file = (char *) calloc(strlen(PD_BASENAME) + 
				strlen(_pd_filename_extensions[index]) + 5,
				sizeof(char));
	sprintf(*file, "%s.%s", PD_BASENAME, _pd_filename_extensions[index]);
}
