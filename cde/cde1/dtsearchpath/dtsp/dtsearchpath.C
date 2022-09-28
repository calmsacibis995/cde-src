/*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1990, 1991, 1992, 1993.
**  All rights are reserved.  Copying or other reproduction of this
**  program except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
****************************<+>*************************************/

/*****************************************************************************
 *  File:              dtsearchpath
 *
 *  Purpose:           Set up the application search paths for 
 *                     the desktop components.
 *
 *  Description:       This client is invoked by the Xsession file
 *                     at user login.  It sets the local and remote locations
 *                     that the desktop components should look, for application
 *                     icons, help files, and other application configuration
 *                     information on behalf of the user.
 *
 *  Invoked by:        Xsession after the scripts in /etc/dt/Xsession.d
 *                     are sourced in.
 *
 *  Product:           @(#)Common Desktop Environment 1.0          
 *
 *  Revision:          $XConsortium: dtsearchpath.C /main/cde1_maint/3 1995/10/05 10:46:01 lehors $
 *
 ****************************************************************************/

/****************************************************************************
 *
 *  DTMOUNTPOINT 
 *     Description:  The directory where this system mounts remote discs.
 *                   Used for constructing pathnames to remote files, for
 *                   systems that are not using a global file name space.
 *     Default:      Vendor specific:  /nfs
 *
 *  DTAPPSEARCHPATH
 *     Description:  The directories where we will look for applications.
 *     Default:      $HOME/.dt/appmanager,
 *                   /etc/dt/appconfig/appmanager,
 *                   /usr/dt/appconfig/appmanager
 *
 *  DTDATABASESEARCHPATH
 *     Description:  The directories where we will look for actions/filetypes.
 *     Default:      Derived from DTAPPSEARCHPATH:
 *                       $HOME/.dt/types,
 *                       /etc/dt/appconfig/types/%L,
 *                       /usr/dt/appconfig/types/%L
 *
 *  DTHELPSEARCHPATH
 *     Description:  The directories where we will look for online help.
 *     Default:      Derived from $DTAPPSEARCHPATH:
 *                       $HOME/.dt/help/[%H | %H.hv]
 *                       /etc/dt/appconfig/help/%L/[%H | %H.sdl | %H.hv],
 *                       /usr/dt/appconfig/help/%L/[%H | %H.sdl | %H.hv]
 *
 *  DTICONSEARCHPATH
 *     Description:  The directories where we will look for icons.
 *     Default:      Derived from $DTAPPSEARCHPATH:
 *                       $HOME/.dt/icons/%B,
 *                       /etc/dt/appconfig/icons/%L/[%B | %B.pm | %B.bm]
 *                       /usr/dt/appconfig/icons/%L/[%B | %B.pm | %B.bm]
 ****************************************************************************/

/****************************************************************************
 *    If you specify a value for one of the search paths, then that value
 *    will be added to the default search path after the sys-admin config-
 *    uration location but before the factory-default location.  For ex-
 *    ample, if you specify a value for the DTSPSYSAPPHOSTS of /foo, then
 *    the search path will be:
 *
 *      $HOME/.dt:/etc/dt/appconfig:/foo:/usr/dt/appconfig
 *
 *    Remember that you can also set the USER equivalents of these
 *    search paths.  They are useful for specifying locations other
 *    than the defaults.   Their names are:
 *
 *  DTSPUSERAPPHOSTS
 *     Default:  <null>
 *
 *  DTSPUSERDATABASEHOSTS
 *     Default:  <null>
 *
 *  DTSPUSERHELP
 *     Default:  <null>
 *
 *  DTSPUSERICON
 *     Default:  <null>
 *
 *    These values will be prepended to the system paths, and hence will
 *    will take precedence.
 *
 **************************************************************************/

#include <stdlib.h>
#include "SearchPath.h"
#include "Options.h"
#include "Environ.h"

Options * options;

int main (int argc, char **argv)
{
  options = new Options(argc, argv);

  if ( options->useCshEnv() ) {
     putenv("SHELL=/usr/bin/csh");
  }
  else {
     putenv("SHELL=/usr/bin/ksh");
  }

  UnixEnvironment * os = new UnixEnvironment;

  CDEEnvironment * user = new CDEEnvironment(options->getHomeDir(), os);
  user->CreateHomeAppconfigDir();

  if (options->removeAutoMountPoint())
    options->setAutoMountPoint(os->getEnvironmentVariable("DTAUTOMOUNTPOINT"));

  AppSearchPath app (user);
  DatabaseSearchPath database (user);

  user->setDTAPPSP(app.GetSearchPath());

  IconSearchPath icon (user);
  IconSearchPath iconbm (user, "XMICONBM", ".bm", ".pm");

  HelpSearchPath help (user);
  ManSearchPath man(user);

  app.FixUp();

  if (options->Verbose() || options->CheckingUser()) {
    CString mp(user->OS()->MountPoint());
#ifdef IOSTREAMSWORKS
    cout << "DTMOUNTPOINT: " << mp << "\n" << endl;
    cout << app << endl;
    cout << database << endl;
    cout << icon << endl;
    cout << iconbm << endl;
    cout << help << endl;
#else
    printf("DTMOUNTPOINT: %s\n",mp.data());
    app.Print();
    database.Print();
    icon.Print();
    iconbm.Print();
    help.Print();
#endif
    man.Print();
  }

  if (!options->CheckingUser() && !options->Verbose()) {
    app.ExportPath();
    database.ExportPath();
    icon.ExportPath();
    iconbm.ExportPath();
    help.ExportPath();
    man.ExportPath();
  }

  return 0;
}
