/*
 * File:         spc-env.c $XConsortium: spc-env.c /main/cde1_maint/2 1995/09/06 02:08:35 lehors $
 * Language:     C
 *
 * (c) Copyright 1989, Hewlett-Packard Company, all rights reserved.
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company			*
 * (c) Copyright 1993, 1994 International Business Machines Corp.	*
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.			*
 * (c) Copyright 1993, 1994 Novell, Inc.				*
 */

#include <stdlib.h>
#include <bms/sbport.h> /* NOTE: sbport.h must be the first include. */

#include "spcP.h"

#include <bms/MemoryMgr.h> 

/* External declarations */

extern XeString official_hostname;  /* from spc-net.c */

/*----------------------------------------------------------------------+*/
XeString SPC_Getenv(XeString var,
		    XeString *envp)
/*----------------------------------------------------------------------+*/
{

  int len;
  XeString *envidx;
  XeString idx;
  
  /* First, check that we have real values */
  
  if(!var || !envp || !*envp)
    return(XeString_NULL);

  /* Look for '=' */

  idx=strchr(var, Equal);
  
  if(idx==0)
    /* No '='.  Just use the entire var string */
    len=strlen(var);
  else
    /* found '='.  Diddle pointers to get index of var */
    len=(strchr(var, Equal))-var;
  
  for(envidx=envp; *envidx; envidx++) {
    if(!strncmp(*envidx, var, len))
      /* Found a match. Return value part */
      return(*envidx+len+1);
  }

  return(XeString_NULL);

}

/*
 **
 ** The purpose of SPC_Putenv is to maintain the environment pointers
 ** associated with a given channel.  It differs from the normal putenv
 ** routine in the following ways:
 **   1. It copies its arguments (and frees any arguments which get a "hit")
 **   2. It takes an environment pointer which will be used to store the
 **      new value.  This new pointer may change (via realloc).
 **
 ** Note: This routine makes a number of assumptions:
 **   1. The envp pointer was allocated via malloc.
 **   2. That it is okay to ignore error conditions.  In particular,
 **      it will ignore incorrect pointers (which is okay, as this
 **      guy is an internal routine which means that these pointers
 **      have been already checked), and it will ignore syntax errors
 **      in the passed value.  The latter ignore may be slightly
 **      problematical, but it happens.
 **   3. That memory allocation errors cause the program to bomb.
 **
*/

/*----------------------------------------------------------------------+*/
XeString *SPC_Putenv(XeString val,
		     XeString *envp)
/*----------------------------------------------------------------------+*/

{
  int len, newsize;
  XeString *envptr;
  char *pch;
  int remove_command = 0;
#ifdef NLS16
   int          is_multibyte = 0;
#endif /* NLS16 */

#ifdef NLS16
   if (MB_CUR_MAX > 1)
      is_multibyte = 1;
   else
      is_multibyte = 0;
#endif /* NLS16 */

  /* Again, check for real values */

  if(!val || !envp) {
    return(envp);
  }

  /* Look for '=' */

  len=(strchr(val, (XeChar)'='))-val;
  if(len<0) {
    /* 
     * This string doesn't contain an '='.  This may be OK if
     * the string contains a 'remove environment variable' keyword.  
     * If this keyword is found, treat it like a variable and
     * the SPC daemon will find it and remove it from the
     * environment before it exec's a subprocess.
     *
     * If the string contains some white space before the variable
     * or keyword, skip the white space.
     */
     pch = val;
     while (
#ifdef NLS16
            (!is_multibyte || (mblen (pch, MB_CUR_MAX) == 1)) &&
#endif
            isspace ((u_char)*pch))
              pch++;
     if (*pch == '\000')
       return (envp);
     if (strncmp (pch, SPC_REMOVE_VAR, strlen (SPC_REMOVE_VAR)))
       return(envp);
     remove_command = 1;
     val = pch;
     len = strlen (SPC_REMOVE_VAR);
  }
  
  for(envptr=envp; *envptr; envptr++) {
    if(!strncmp(*envptr, val, len) && !remove_command) {
      /* Found a match. Replace this value with the one passed */
      free(*envptr);
      *envptr=SPC_copy_string(val);
      return(envp);
    }
  }
  
  /* No match. We need to expand this env pointer, and stash the
     new value at the end */
  
  len=envptr-envp;
  
  /* calculate new size needed:
     len=ptr to null element
     newsize=len+2 (one for NULL, one for new element)
     */
  
  newsize=(len+2)*sizeof(XeString *);
  
  /* Expand envp.  This is a potentially expensive operation, if the
     realloc routine is not smart. */
  
  if(!(envp=(XeString *)realloc((char *)envp, newsize))) {
    SPC_Error(SPC_Out_Of_Memory);
    exit(1);
  }
  
  /* Okay.  We got the new memory.  Stash the new variable into it &
     return the new envp.*/
  
  envp[len]=SPC_copy_string(val);
  envp[len+1]=XeChar_NULL;

  return(envp);
  
}

/*----------------------------------------------------------------------+*/
XeString *SPC_Add_Env_File(XeString filename,
			   XeString *envp)
/*----------------------------------------------------------------------+*/
{

  FILE *f;
  int n;
  XeChar fbuffer[BUFSIZ];

  if(!filename)
    return(envp);

  /* Open stream */

  if(!(f=fopen(filename, "r")))
    return(envp);

  while(fgets(fbuffer, BUFSIZ, f)) {
    n=strlen(fbuffer);
    if(fbuffer[n-1]==Newline)
      fbuffer[--n]=Pad;  /* get rid of the extra newline */

    /* Should we skip this line? */

    if(n==XeChar_NULL || fbuffer[0]==Pad || fbuffer[0]==Pound) continue;

    envp=SPC_Putenv(fbuffer, envp);
  }

  fclose(f);

  return(envp);
}
  
/*----------------------------------------------------------------------+*/
XeString *SPC_Create_Default_Envp(XeString *old_envp)
/*----------------------------------------------------------------------+*/
{

  XeString *envp;
  XeChar display[MAXHOSTNAMELEN+sizeof((XeString)"DISPLAY=:0.0")];
  XeString dispvar;
  XeString sys_env_path = NULL;
  
  if(old_envp)
    envp=old_envp;
  else {
    envp=(XeString *)XeMalloc(sizeof(XeString) * DEFAULT_ENVP_SIZE);
    envp[0]=XeChar_NULL;
  }
  
  if(!(dispvar=getenv("DISPLAY")))
    sprintf(display, "DISPLAY=%s:0", official_hostname);
  else
    sprintf(display, "DISPLAY=%s", dispvar);
  
  envp=SPC_Putenv(display, envp);
  
  /* 
   * First add the installed environment file.
   */
  sys_env_path = (XeString) malloc (strlen(SPCD_ENV_INSTALL_DIRECTORY) +
                                    strlen(SPCD_ENV_FILE) + 3);
  (void) sprintf (sys_env_path, "%s/%s", 
		  SPCD_ENV_INSTALL_DIRECTORY,
                  SPCD_ENV_FILE);
  envp=SPC_Add_Env_File(sys_env_path, envp);

  /*
   * Next add the configured environment file.
   */
  sys_env_path = (XeString) realloc (sys_env_path,
  				     strlen(SPCD_ENV_CONFIG_DIRECTORY) +
                                     strlen(SPCD_ENV_FILE) + 3);
  (void) sprintf (sys_env_path, "%s/%s", 
		  SPCD_ENV_CONFIG_DIRECTORY,
                  SPCD_ENV_FILE);
  envp=SPC_Add_Env_File(sys_env_path, envp);

  /*
   * Now add the user environment file
   */
  envp=SPC_Add_Env_File(spc_user_environment_file, envp);

  free(sys_env_path);
  
  return(envp);
}

/* Final cleanup of environment pointer */

/*----------------------------------------------------------------------+*/
XeString *SPC_Fixup_Environment(XeString *envp,
				SPC_Channel_Ptr channel)
/*----------------------------------------------------------------------+*/
{
  
  XeString disp;
  XeString screenptr;
  XeChar buffer[MAXHOSTNAMELEN+sizeof((XeString)"DISPLAY=:0.0")];
  XeChar null=XeChar_NULL;
  
  if(!envp)
    return(envp);
  
  disp=SPC_Getenv((XeString)"DISPLAY", envp);
  
  /* I don't particularly like hard coding these values here.
     I have been bitten before by doing so, and probably will again... */
  
  if(IS_REMOTE(channel) &&
     (!strncmp(disp, (XeString)"unix", 4) ||
      !strncmp(disp, (XeString)"local", 5) ||
      !strncmp(disp, (XeString)":", 1)
      )
     ) {

    screenptr=strchr(disp, Colon);
    sprintf(buffer, "DISPLAY=%s%s",
	    official_hostname,
	    screenptr ? screenptr : &null);

    envp=SPC_Putenv(buffer, envp);
  }

  return(envp);
}

/*----------------------------------------------------------------------+*/
XeString *SPC_Merge_Envp(XeString *dest_envp,
			 XeString *source_envp)
/*----------------------------------------------------------------------+*/
{

  if(!dest_envp || !source_envp)
    return(dest_envp);
  for(; *source_envp; source_envp++)
    dest_envp=SPC_Putenv(*source_envp, dest_envp);
  
  return(dest_envp);
}

/*----------------------------------------------------------------------+*/
void SPC_Free_Envp(XeString *envp)
/*----------------------------------------------------------------------+*/
{
  XeString *envptr=envp;

  if(!envptr)
    return;
  
  while (*envptr)
    free(*envptr++);

  free((char *)envp);
}
