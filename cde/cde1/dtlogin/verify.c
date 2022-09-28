/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: verify.c /main/cde1_maint/7 1995/10/18 19:32:29 mgreess $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

/*
 * verify.c
 *
 * typical unix verification routine.
 */

# include	"dm.h"
# include	"vgmsg.h"
# include	"vgproto.h"

#include <X11/Xresource.h>
#include <sys/stat.h>

#ifdef _AIX
# include       <usersec.h>
#endif

#ifdef sun
#include "solaris.h" 
#endif

# include	<pwd.h>
# ifdef NGROUPS
# include	<grp.h>
# endif

#ifdef sun
# include	<deflt.h>
#endif

struct passwd nobody = {
	"Nobody", "***************"
};

static  char    *Pndefault      = "/etc/default/login";





#ifdef _NO_PROTO
extern char *_ExpandLang();
#else
extern char *_ExpandLang( char *string, char *lang );
#endif /* _NO_PROTO */

int 
#ifdef _NO_PROTO
Verify( d, greet, verify )
        struct display *d ;
        struct greet_info *greet ;
        struct verify_info *verify ;
#else
Verify(
        struct display *d,
        struct greet_info *greet,
        struct verify_info *verify )
#endif /* _NO_PROTO */
{
	struct passwd	*p;
	char		*shell, *home;
	char		**argv;
	FILE 		*lastsession;
	char 		last_sess[MAXPATHLEN];
	int 		notify_dt;
	int 		i;

	Debug ("Verify(): %s\n", greet->name);

	/*
	 * The user has already been verified by the Greeter. This is
	 * really just to look up the rest of his environment. Better
	 * performance would be realized by having the Greeter look this up
	 * and pass it back in the pipe (saving an additional getpwnam).
	 */
	 
	p = getpwnam (greet->name);
	if (!p || strlen (greet->name) == 0) {
	    Debug ("Can't get user information for %s\n", greet->name);
	    return 0;
        }

#ifdef BYPASSLOGIN
	Debug ("BypassLogin = %d\n",d->bypassLogin);
 	if ( d->bypassLogin ) {
	    char *s = NULL;
	    if ( !strcmp (p->pw_name, "Nobody" ))
		s = "Greetless login failed; user invalid\n";
	    else if ( access ( p->pw_dir, F_OK ))
		s = "Greetless login failed; bad user home dir access\n";
	    else if ( p->pw_uid == 0 )
		s = "Greetless login failed; root bypass disallowed\n";
	    if (s) {
		Debug(s); LogError((unsigned char *) s);
		bzero(greet->password, strlen(greet->password));
		return 0;
	    }
 	}
#endif /* BYPASSLOGIN */

#ifdef AthenaWidgets
#ifdef BYPASSLOGIN 
	if (strcmp(crypt(greet->password,p->pw_passwd),p->pw_passwd)
	    && !d->bypassLogin) {
#else
	if (strcmp(crypt(greet->password,p->pw_passwd),p->pw_passwd)) {
#endif /* BYPASSLOGIN */
	    Debug ("verify failed\n");
	    bzero(greet->password, strlen(greet->password));
	    return 0;
	}
	Debug ("verify succeeded\n");
	bzero(greet->password, strlen(greet->password));
#endif
	Debug ("User information for %s acquired\n", greet->name);


	verify->uid = p->pw_uid;

#ifdef __AFS
	if ( IsVerifyName(VN_AFS) ) {
	    (void)setgid(p->pw_gid);
	    verify->groups[2] = p->pw_gid;
	}
#else
#  ifdef NGROUPS
	getGroups (greet->name, verify, p->pw_gid);
#  else
	verify->gid = p->pw_gid;
#  endif
#endif /* __AFS */

#ifdef AUDIT
	verify->audid  = p->pw_audid;
	verify->audflg = p->pw_audflg;
#endif	

	home = p->pw_dir;
	shell = p->pw_shell;
	argv = 0;

	if(d->sessionType > NOTIFY_ALT_DTS)
	    notify_dt = NOTIFY_ALT_DTS;
	else
	    notify_dt = d->sessionType;

	switch (notify_dt) {
        case NOTIFY_DT:
        case NOTIFY_OK:
	    if(d->session)
	      argv = parseArgs (argv,d->session);
              break;
        case NOTIFY_ALT_DTS:
	    if((argv = setDt(d,argv,notify_dt,verify,NULL)) == NULL)
		/*if no startup file, startup the regular desktop*/
	       if(d->session) {
	          argv = parseArgs (argv,d->session);

               Debug ("No Startup file \n");
	       }
            break;
        case NOTIFY_LAST_DT:
	    strcpy(last_sess,home);
            strcat(last_sess,LAST_SESSION_FILE);

            if((lastsession = fopen(last_sess,"r")) == NULL)  {
	    /* 
	     * if no lastsession file, execute the logically 
             *		first alternate desktop
	     */
		if((argv = setDt(d,argv,notify_dt,verify,last_sess)) == NULL)
	          if(d->session) {
		    argv = parseArgs (argv,d->session);
		    /* No altdts, set the Default Regular session */
                    Debug ("No AltDestops in the resources file \n");
	          }
	    }
            else{
	       strcpy(last_sess,"\0");	
               fgets(last_sess,MAXPATHLEN,lastsession);

	       if(access(last_sess, X_OK) != 0) {
		  if((argv = setDt(d,argv,notify_dt,verify,NULL)) == NULL)
		    if(d->session){
		      argv = parseArgs (argv,d->session);
		      /* No altdts, set the Default Regular session */
                      Debug ("No AltDestops in the resources file setting to %s  \n",argv[0]);
		    }
	       }
	       else {
                  argv = parseArgs (argv, last_sess);
	       }
               fclose(lastsession);
            }
            break;
        default:
            break;
        }
	
	if (greet->string)
		argv = parseArgs (argv, greet->string);
	if (!argv)
		argv = parseArgs (argv, "xsession");
	verify->argv = argv;
	verify->userEnviron = userEnv (d, p->pw_name, home, shell,p);
	Debug ("User environment:\n");
	printEnv (verify->userEnviron);
	verify->systemEnviron = systemEnv (d, p->pw_name, home);
	Debug ("System environment:\n");
	printEnv (verify->systemEnviron);
	Debug ("End of environments\n");
#ifdef __osf__
        if (setlogin(greet->name) == -1) {
                Debug ("setlogin failed for %s, errno = %d\n", greet->name,
                errno);
        }
#endif /* __osf__ */
	return 1;
}

char **
#ifdef _NO_PROTO
setDt(d, argv, dt_type, verify, last_sess)
        struct display *d ;
        char **argv;
	int	dt_type;
	struct verify_info *verify;
	char *last_sess;
#else
setDt(
        struct display *d,
        char **argv,
	int	dt_type,
	struct verify_info *verify,
	char *last_sess)
#endif /* _NO_PROTO */
{
int		i;
char 		*resources = NULL;
char 		buf1[MAXPATHLEN];
char 		buf2[MAXPATHLEN];
char 		temp_file[MAXPATHLEN];
int 		adts;
XrmValue 	value_dts;
char 		*rmtype;
XrmValue 	rmvalue;
int 		altdtrequested;
FILE		*fp=NULL;
struct		stat statb;


   Debug("setDt: %d\n", dt_type);

   if(!XresourceDB) {
      Debug ("Verify - No resources \n");
      return 0;
   }


 
   if(dt_type == NOTIFY_LAST_DT) {
      /* 
       * If sdt_firstlogin exists, create temp file for sdt_firstlogin use
       */ 

      if ( stat(FIRST_LOGIN, &statb) != -1 &&
	   (statb.st_mode & S_IEXEC) == S_IEXEC ) { 

         strcpy(temp_file, "/tmp/sdt_flXXXXXX");
         mktemp(temp_file);
         Debug("first login tmp file: %s\n", temp_file);

         fp = fopen(temp_file, "w");
      }

      if (fp ) {
         strcpy(buf1,"Dtlogin*altDts");
         strcpy(buf2,"Dtlogin*AltDts");

         if( XrmGetResource(XresourceDB, buf1, buf2,
	     &rmtype, &value_dts) == True) {
           adts = atoi(value_dts.addr);

          Debug("num alt desktops = %d\n", adts);

           if (adts > 1) {
	      for(i = 0; i < adts; i++) {
                 /*
                  * Write out the alternate desktop info for FL use
                  */
                 sprintf(buf1,"%s%d","Dtlogin*altDtName",i+1);
                 sprintf(buf2,"%s%d","Dtlogin*AltDtName",i+1);

                 if(XrmGetResource(XresourceDB, buf1, buf2,
                     &rmtype, &rmvalue) == True){
                    Debug ("FL alt DT %d name is %s\n", i, rmvalue.addr);
	            sprintf(buf1,"%s\n",rmvalue.addr);
	            fputs(buf1, fp);
                 } else break;

                 sprintf(buf1,"%s%d","Dtlogin*altDtStart",i+1);
                 sprintf(buf2,"%s%d","Dtlogin*AltDtStart",i+1);

                 if(XrmGetResource(XresourceDB, buf1, buf2,
                     &rmtype, &rmvalue) == True){
                    Debug ("FL alt DT %d startup is %s\n", i, rmvalue.addr);
	            sprintf(buf1,"%s\n",rmvalue.addr);
	            fputs(buf1, fp);
                 } else fputs ("\n", fp);
	      }
	      fclose(fp);

	      sprintf(buf1,"%s -infile %s -outfile %s", 
				FIRST_LOGIN, temp_file, last_sess);
	      Debug("firstlogin cmdline: %s\n", buf1);

              argv = parseArgs (argv, buf1);
              return(argv);
           }
         }
         fclose(fp);
	 unlink(temp_file);
      } else Debug("fl temp file open: Failed\n");


      /* pick a desktop without using sdt_firstlogin */  

      strcpy(buf1,"Dtlogin*altDts");
      strcpy(buf2,"Dtlogin*AltDts");
 
      if(XrmGetResource(XresourceDB, buf1, buf2,
   	&rmtype, &value_dts) == True){
        adts = atoi(value_dts.addr);
        for(i = 0; i < adts; i++) {
           sprintf(buf1,"%s%d","Dtlogin*altDtStart",i+1);
           sprintf(buf2,"%s%d","Dtlogin*AltDtStart",i+1);
           if(XrmGetResource(XresourceDB, buf1, buf2,
               &rmtype, &rmvalue) == True){
              Debug ("Start up file is %s\n",rmvalue.addr);
              argv = parseArgs (argv, rmvalue.addr);
              return argv;
           }
        }
      }
   }
   else {
	    altdtrequested = d->sessionType - NOTIFY_ALT_DTS;
		
            sprintf(buf1,"%s%d", "Dtlogin*altDtStart",altdtrequested);
            sprintf(buf2,"%s%d", "Dtlogin*AltDtStart",altdtrequested);
 
            if(XrmGetResource(XresourceDB, buf1, buf2,
                                        &rmtype, &rmvalue) == True){
                Debug ("Start up file is %s\n",rmvalue.addr);
                argv = parseArgs (argv, rmvalue.addr);
                return argv;
            }
   }
 return 0;
}


char **
#ifdef _NO_PROTO
userEnv( d, user, home, shell, p )
        struct display *d ;
        char *user ;
        char *home ;
        char *shell ;
        struct passwd   *p;
#else
userEnv(
        struct display *d,
        char *user,
        char *home,
        char *shell,
	struct passwd   *p)

#endif /* _NO_PROTO */
{
	char	**env;
        char     *value;
        char    **exp;
        char     *Def_path;
	char 	*langlist;
	
#ifdef _AIX
       /*
        * For AIX, we must delimit protected variables. This is done by
        * adding special tags to the environment list. The setpenv()
        * routine serched the environent for these special tags.
        *
        * NOTE: If you change the location of PENV_SYSSTR or PENV_USRSTR
        *       in this environment list, you MUST change the index 
        *       in session.c prior to the call to setpenv().
        */
        env = 0;
        env = setEnv(env,PENV_SYSSTR,"");   /* protected vars */
        env = setEnv(env,"LOGNAME",user);
        env = setEnv(env,"LOGIN",user);
        env = setEnv(env,PENV_USRSTR,"");   /* regular vars */

#else
        env = 0;
        env = setEnv (env,"LOGNAME", user);
#endif

        for (exp = exportList; exp && *exp; ++exp)
        {
          value = getenv (*exp);
          if (value)
            env = setEnv (env, *exp, value);
        }

	env = setEnv (env, "DISPLAY", d->name);
	env = setEnv (env, "HOME", home);
	env = setEnv (env, "USER", user);

#ifdef sun
 	if ((Def_path = login_defaults(p,d)) != NULL) 
 	    env = setEnv (env, "PATH", Def_path);
#else
        if (d->userPath && strlen(d->userPath) > 0)
            env = setEnv (env, "PATH", d->userPath);
#endif

	env = setEnv (env, "SHELL", shell);

	if (timeZone && strlen(timeZone) > 0 )
	    env = setEnv(env, "TZ", timeZone);

            if ( d->langList && strlen(d->langList) > 0 )
		   langlist = d->langList;
#if !defined (ENABLE_DYNAMIC_LANGLIST)
            else
                if (languageList && strlen(languageList) > 0 )
		   langlist = languageList;
#endif /* ENABLE_DYNAMIC_LANGLIST */

        if ( d->language && strlen(d->language) > 0 )
		env = (char **) setLang(d, env, langlist);

#if 0 /* setting LC_ALL can cause undesirable side effects -- bf@hp */
/*
 * Don't set LC_MESSAGES to LANG since:
 * 1) if LC_MESSAGES is unset, it will by default take on the value of LANG
 * 2) if set, we don't want to override the user's setting
 */
	    env = setEnv (env, "LC_ALL",      d->language);
	    env = setEnv (env, "LC_MESSAGES", d->language);
#endif

#if 0 		/* setting XAUTHORITY here is a bug, needs further study... */
	if (d->authFile)
	    env = setEnv (env, "XAUTHORITY", d->authFile);
#endif

	if (d->displayType.location != Local) {
	    env = setEnv (env, XFORCEINTERNET, "True");
            env = setEnv (env, LOCATION, "remote");
 	} else {
            env = setEnv (env, LOCATION, "local");
	}

        if (d->environStr && strlen(d->environStr) > 0)
	    env = parseEnv(env, d->environStr);

	/*
	 *  set SESSIONTYPE environment variable if requested by user, or
	 *  if the "dtlite" resource is set...
	 *
	 *  Note: the dtlite resource takes precedence over xdmMode.
	 */

	if (d->dtlite == True) {
	    d->sessionType = NOTIFY_DTLITE;
	} else if (d->xdmMode == True) {
            d->sessionType = NOTIFY_XDM;
	}


	switch (d->sessionType) {

	case NOTIFY_DTLITE:
	    env = setEnv(env, SESSIONTYPE, "dtlite");
	    break;

	case NOTIFY_XDM:
	    env = setEnv(env, SESSIONTYPE, "xdm");
	    break;

	case NOTIFY_DT:
	    env = setEnv(env, SESSIONTYPE, "dt");
	    break;

	default:
	    break;

	}

	return env;
}

/*
 * read login defaults       
 */
#ifdef sun
char *
#ifdef _NO_PROTO
login_defaults( p, d)
        struct passwd *p ;
        struct display *d ;
#else
login_defaults(
        struct passwd *p,
	struct display *d )
#endif /* _NO_PROTO */
{
        extern int defcntl();
        register int  flags;
        register char *ptr;
	char *Def_path;
	char final_path[MAXPATHLEN];
	char *element;
 

         if (defopen(Pndefault) == 0) {
                /*
                 * ignore case
		 * code taken from login.c 
                 */
                flags = defcntl(DC_GETFLAGS, 0);
                TURNOFF(flags, DC_CASE);
                defcntl(DC_SETFLAGS, flags);
 
	      if ( p->pw_uid == 0 ) {
                if ((Def_path = defread("SUPATH=")) != NULL)
                        Def_path = strdup(Def_path);
		else
			Def_path = DEF_SUPATH;
	      }
	      else {
                if ((Def_path = defread("PATH=")) != NULL)
                        Def_path = strdup(Def_path);
		else
			Def_path = DEF_PATH;
	      }
	 }

	/*
	 * close the file..(/etc/default/login)
	 */
	(void) defopen((char *)NULL);

	Debug ("login_defaults():  \n");
 
     	 if (d->userPath && strlen(d->userPath) > 0) {
            if(Def_path && strlen(Def_path) > 0)
	   	element = strtok(Def_path,":");
	    else
		return d->userPath;

	   strcpy(final_path,d->userPath);

	   while(element) {
	     if((strstr(d->userPath,element)) == NULL) {
		strcat (final_path,":");
		strcat (final_path,element);
	     }
	     element = strtok(NULL,":");
	   }
	   return(strdup(final_path));
         }
         else  {
            if(Def_path && strlen(Def_path) > 0)
		return Def_path;	
	    else
		return 0;
	 }
}
#endif

char ** 
#ifdef _NO_PROTO
systemEnv( d, user, home )
        struct display *d ;
        char *user ;
        char *home ;
#else
systemEnv(
        struct display *d,
        char *user,
        char *home )
#endif /* _NO_PROTO */
{
        char    **env;
        char     *value;
        char    **exp;
	char 	*langlist;
	
        env = 0;

        for (exp = exportList; exp && *exp; ++exp)
        {
          value = getenv (*exp);
          if (value)
            env = setEnv (env, *exp, value);
        }
	
        {
          value = getenv("TZ");
          if(value)
                env = setEnv (env, "TZ", value);
	  else
		{
		   if (timeZone && strlen(timeZone) > 0 )
	    	   	 env = setEnv(env, "TZ", timeZone);
		}
        }

	env = setEnv (env, "DISPLAY", d->name);
	if(home)
	env = setEnv (env, "HOME", home);
	if(user)
	env = setEnv (env, "USER", user);
	env = setEnv (env, "PATH", d->systemPath);
	env = setEnv (env, "SHELL", d->systemShell);
	    
        if ( d->langList && strlen(d->langList) > 0 )
		   langlist = d->langList;
#if !defined (ENABLE_DYNAMIC_LANGLIST)
        else
             if (languageList && strlen(languageList) > 0 )
		   langlist = languageList;
#endif /* ENABLE_DYNAMIC_LANGLIST */
 
        if ( d->language && strlen(d->language) > 0 )
		env = (char **) setLang(d, env, langlist);

	if (d->displayType.location != Local)
	    env = setEnv (env, XFORCEINTERNET, "True");

#if defined (sun)
        if (getEnv (env, "OPENWINHOME") == NULL) 
            env = setEnv(env, "OPENWINHOME", "/usr/openwin");
#endif

	return env;
}

#ifdef NGROUPS

#ifndef __hpux

/*
 *  groupMember() not used in HP-UX
 */
 
int 
#ifdef _NO_PROTO
groupMember( name, members )
        char *name ;
        char **members ;
#else
groupMember(
        char *name,
        char **members )
#endif /* _NO_PROTO */
{
	while (*members) {
		if (!strcmp (name, *members))
			return 1;
		++members;
	}
	return 0;
}
#endif /* ! __hpux */


void
#ifdef _NO_PROTO
getGroups( name, verify, gid )
        char *name ;
        struct verify_info *verify ;
        int gid ;
#else
getGroups(
        char *name,
        struct verify_info *verify,
        int gid )
#endif /* _NO_PROTO */
{
	int		ngroups;
	struct group	*g;
	int		i;

	ngroups = 0;
	verify->groups[ngroups++] = gid;
	setgrent ();

        /*
	 * if your system does not support "initgroups(3C)", activate   
         * this code...
	 */

#if !(defined(__hpux) || defined(__osf__))
	while ( (g = getgrent()) != NULL ) {
		/*
		 * make the list unique
		 */
		for (i = 0; i < ngroups; i++)
			if (verify->groups[i] == g->gr_gid)
				break;
		if (i != ngroups)
			continue;
		if (groupMember (name, g->gr_mem)) {
			if (ngroups >= NGROUPS)
				LogError(ReadCatalog(MC_LOG_SET,
					MC_LOG_MORE_GRP,MC_DEF_LOG_MORE_GRP),
					name, NGROUPS, g->gr_name);
			else
				verify->groups[ngroups++] = g->gr_gid;
		}
	}
#endif
	verify->ngroups = ngroups;
	endgrent ();
}
#endif
