/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/
/*******************************************************************
 *
 *  File:              dtappgather.C
 *
 *  Purpose:           Create the Application Manager
 *                     the desktop components.
 *
 *  Description:       dtappgather is invoked by the Xsession file at
 *                     user login.  It collects the application groups
 *                     and subdirectories that are to appear in the
 *                     user's Application Manager view.
 *
 *                     It is also invoked by the "ReloadApps" action.
 *
 *  Product:           @(#)Common Desktop Environment 1.0          
 *
 *  Revision:          $XConsortium: dtappgather.C /main/cde1_maint/1 1995/07/14 22:59:13 drk $
 *
 ********************************************************************/

/********************************************************************
 *
 *  Environment variables used by this utility
 *
 *  DTMOUNTPOINT
 *     Default may be vendor specific:  /nfs
 *
 *  DTAPPSEARCHPATH
 *     Default:  $HOME/.dt,/etc/dt/appconfig,/usr/dt/appconfig
 *     Set at login by the dtsearchpath utility.
 *
 *  HOME
 *     No default.
 *     Set by the dtlogin program.
 *
 *  DISPLAY
 *     No default.
 *     Set by the dtlogin program.
 *
 *  TARGET_APPMAN_DIR
 *     Default:	/var/dt/appconfig/appmanager/$DTUSERSESSION
 *     Set within this utility.
 *
 ****************************************************************************/
#include "Options.h"
#include "dtappgather.h"
#include "DirIterator.h"
#include <stdlib.h>
#include <iostream.h>

Options * options;

void LegacyCleanUp
	(
	CDEEnvironment * user
	)
{

    // For now, remove the old ApplicationManager subdirectory
    // so that we don't leave old CDE users with extra baggage
    // under $HOME.

    CString targetAppmanDir (user->HOME() + ".dt/ApplicationManager");

    if (user->OS()->isDirectory(targetAppmanDir)) {
	user->OS()->changePermissions(targetAppmanDir,0777);
	user->OS()->removeDirectory(targetAppmanDir);
    }
}

AppManagerDirectory::AppManagerDirectory
	(
	CDEEnvironment * user,
	const CString &  app
	) : user_(user),
	    langVersionFound(0),
	    appsp_(app)
{
    // Set the users Application Manager subdirectory

    CString userhostdir = user->UserHostDir();

    if (userhostdir.isNull())
	userhostdir = "generic-display-0";

    dirname_ = "/var/dt/appconfig/appmanager/";
    dirname_ += userhostdir;

    if (user->OS()->isDirectory(dirname_)) {
	user->OS()->changePermissions(dirname_,0755);
	user->OS()->removeDeadLinks(dirname_);
	if (!options->Retain())
	    user->OS()->removeFiles(dirname_,"[.]*");
    }
    else {

	// Make /var/dt/appconfig/appmanger directories if not present 
        //
	// Make /var/dt/tmp directory if not present 
	//
        // Make user session subdirectory under /var/dt/appconfig/appmanager

	CString dir(dirname_);
	dir.replace("/" + userhostdir,"");
	if (!user->OS()->isDirectory(dir)) {	// does appmanager exist?
	    dir.replace("/appmanager","");
	    if (!user->OS()->isDirectory(dir)) {   // does appconfig exist?
		dir.replace("/appconfig","");
	        if (!user->OS()->isDirectory(dir)) {   // does dt exist?
		    dir.replace("/dt","");
		    if (!user->OS()->isDirectory(dir)) {  // does /var exist?
			user->OS()->MakeDirectory(dir,0775);
			user->OS()->changeOwnerGroup(dir,"root","other");
			user->OS()->changePermissions(dir,0775);
		    }
		    dir += "/dt";
		    user->OS()->MakeDirectory(dir,0755);
		    user->OS()->changeOwnerGroup(dir,"root","other");
		    user->OS()->changePermissions(dir,0755);
		}
		dir += "/appconfig";
		user->OS()->MakeDirectory(dir,0755);
		user->OS()->changeOwnerGroup(dir,"bin","bin");
		user->OS()->changePermissions(dir,0755);
	    }
	    dir += "/appmanager";
	    user->OS()->MakeDirectory(dir,0755);
	    user->OS()->changeOwnerGroup(dir,"bin","bin");
	    user->OS()->changePermissions(dir,0755);
	}
	user->OS()->MakeDirectory(dirname_,0755);
	user->OS()->changeOwnerGroup(dirname_,"","");
    }

    // Make /var/dt/tmp directory if not present 

    CString tmp("/var/dt/tmp/");
    if (!user->OS()->isDirectory(tmp)) {   // does tmp exist?
	user->OS()->MakeDirectory(tmp,0777);
	user->OS()->changeOwnerGroup(tmp,"root","other");
	user->OS()->changePermissions(tmp,0777);

        tmp += userhostdir;
        user->OS()->MakeDirectory(tmp, 0755);
        user->OS()->changeOwnerGroup(tmp,"","");
    }
}


/********************************************************************
 * 
 *  TraversePath()
 *
 * 	Parse a given search path, using comma (,) and colon (:) as
 *      delimiters.  Pass each path element to another function.
 ********************************************************************/

void AppManagerDirectory::TraversePath()
{
    if (!appsp_.isNull()) {
	CTokenizedString subpath(appsp_,":");
	CString dir = subpath.next();
	while (!dir.isNull()) {
	    GatherAppsFromASearchElement (dir);
	    dir = subpath.next();
	    if (langVersionFound && dir == "/usr/dt/appconfig/appmanager/C")
		dir = subpath.next();
	}
    }
}

/********************************************************************
 * 
 *  goodFile()
 *
 * 	Ignore '.' and '..' and make sure file doesn't exist.
 *
 ********************************************************************/

int AppManagerDirectory::goodFile
	(
	const CString & path,
	const CString & fname
	) const
{
    if (fname == "." || fname == "..")
	return 0;

    CString filnam(path + "/" + fname);

    if (user_->OS()->FileExists(filnam))
	return 0;

    return 1;
}


/*********************************************************************
 *
 *  GatherAppsFromASearchElement()
 *
 * 	Given search path element (host name or path name), construct
 *      the appropriate path for the various desktop subsystems.
 *
 *      A path is constructed so that each element is appended to
 *      the tail.
 *********************************************************************/
void AppManagerDirectory::GatherAppsFromASearchElement
	(
	const CString & path
	) 
{
    if (path.length()) {
	CString source(path);
	if (source.contains("/%L")) {
	    if (user_->OS()->LANG() != "C") {
		source.replace("%L",user_->OS()->LANG());
		if (!user_->OS()->isDirectory(source))
		     return;
		if (source.contains("/usr/dt/appconfig/appmanager/"))
		    langVersionFound = 1;
	    }
	    else
		return;
	}

	else if (!user_->OS()->isDirectory(source))
	    return;

	DirectoryIterator iter (source);
	struct dirent * direntry;

	user_->OS()->setUserId();

	while (direntry = iter()) {
	    CString dname(direntry->d_name);
	    if (user_->OS()->isDirectory(source + "/" + dname)
	     || user_->OS()->isFile(source + "/" + dname))
		if (goodFile(dirname_, dname))
		    user_->OS()->symbolicLink (source + "/" + dname, 
					       dirname_ + "/" + dname);
	}

	user_->OS()->setUserId();
    }
}

int main (int argc, char **argv)
{
    options = new Options(argc, argv);

    UnixEnvironment * os = new UnixEnvironment;

    CDEEnvironment * user = new CDEEnvironment(0, os);

    LegacyCleanUp(user);

    CString app(getenv("DTAPPSEARCHPATH"));

    AppManagerDirectory * appman = new AppManagerDirectory(user, app);
    appman->TraversePath();

    user->OS()->changePermissions((*appman)(),0555);

    return 0;
}
