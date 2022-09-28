/* $XConsortium: Utils.c /main/cde1_maint/2 1995/08/29 19:42:22 gtsang $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Utils.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Contains utility routines.
 *
 *   FUNCTIONS: ResolveLocalPathName
 *		ResolveTranslationString
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <Dt/Connect.h>
#include <Tt/tttk.h>


/***********************************************************************
* FUNCTION NAME: ResolveLocalPathName
* 
* PURPOSE: This function takes the given parameters and returns a path
*          (owned by the caller) suitable for passing to open(2). It
*          takes the internal cannonical file name kept by the File
*          Manager and remaps them into the standard CDE file name
*          mapping calls (tt_host_file_netfile, tt_netfile_file). The
*          returned pathname is valid only on the local host. See
*          ResolveRemotePathName if you need a pathname which is valid
*          on a different host.
*
* SYNOPSIS: path = ResolveLocalPathName(hostname, directory_name,  
*                                       file_name,local_hostname,
*                                       tt_status)
*
*       char *path;           The returned pathname. The memory is owned
*                             by the caller. NULL is returned on failure.
*  
*       char *hostname;       The host where the file is located.
*
*       char *directory_name; The directory portion of the pathname;
*
*       char *file_name;      The name of the file.          
*
*       char *local_hostname; The current host.
*
*       Tt_status *tt_status; Upon error, the tool talk error status will
*                             be returned for use by the caller. Use
*                             tt_status_message to get a printable string.
* 
********************************************************************************/

#ifdef _NO_PROTO
char *
ResolveLocalPathName(hostname, 
                     directory_name, 
                     file_name, 
                     local_hostname,
                     tt_status)
char *hostname;
char *directory_name;
char *file_name;
char *local_hostname;
Tt_status *tt_status;
#else
char *
ResolveLocalPathName(
                     char *hostname,
                     char *directory_name,
                     char *file_name,
                     char *local_hostname,
                     Tt_status *tt_status)
#endif /* _NO_PROTO */
{
  char * fully_qualified_name;
  char * cannon_name;
  char * path;
  char * tmp;
  int len = strlen( directory_name );


#ifdef DEBUG
  printf ("Home host name is %s  actual hostname is %s\n",
          local_hostname, hostname);
#endif

  /* construct full qualified filename
  */
  if( file_name )
  {
    if( len == 1 && *directory_name == '/' )
    {
      fully_qualified_name = (char *)XtCalloc ( 1, ( strlen (file_name) + 2 ) );
      sprintf( fully_qualified_name, "/%s", file_name );
    }
    else
    {
      fully_qualified_name = (char *)XtCalloc ( 1, ( len + strlen (file_name) + 2 ) );
      sprintf( fully_qualified_name, "%s/%s", directory_name, file_name );
    }
  }
  else
  {
    if( len > 0 )
    {
      fully_qualified_name = (char *)XtMalloc ( len + 1 );
      strcpy( fully_qualified_name, directory_name );
    }
    else
      fully_qualified_name = NULL;
  }

  *tt_status=TT_OK;

#if defined(FILE_MAP_OPTIMIZE)
  /* check if local host name = hostname  */
  /* Do not do a tooltalk call if that is */ 
  /* the case                             */
  if (strcmp(hostname, local_hostname) == 0 )
  {
#ifdef DEBUG
    printf ("fully qualified name is %s\n", path);
#endif
    return (fully_qualified_name);    
  }

  /* not on local host, make an rpc trip */
#endif

  /* What about no hostname, then just send the file */
  if (!hostname || (hostname[0] == '\0'))
     return fully_qualified_name;

  /* convert to network canonical name
  */
  cannon_name = (char *)tt_host_file_netfile (hostname, fully_qualified_name);
  XtFree (fully_qualified_name);
  if ( (*tt_status = tt_pointer_error(cannon_name)) != TT_OK)
  {
#ifdef DEBUG
    printf( "Tooltalk error message: %s\n", tt_status_message(*tt_status));
#endif
    return (NULL);
  }


  /* resolve canonical name on local host
  */
  tmp = (char *) tt_netfile_file(cannon_name);
  tt_free (cannon_name);

  if ( (*tt_status = tt_pointer_error (tmp)) != TT_OK)
  {
#ifdef DEBUG
    printf( "Tooltalk error message: %s\n", tt_status_message(*tt_status));
#endif
    return (NULL);
  }

  path = XtNewString( tmp );
  tt_free( tmp );

  return (path);
}



char *
#ifdef _NO_PROTO
ResolveTranslationString( originalString, address )
                          char * originalString;
                          char * address;
#else
ResolveTranslationString( char * originalString,
                          char * address )
#endif /* _NO_PROTO */

{
  char   addressStr[20];
  char * resolvedString = NULL;
  int    i, j, k, total, length;

  sprintf( addressStr, "%x", address );

  for( i = 0, total = 0; originalString[i] != '\0'; ++i )
    if( originalString[i] == '@' )
      ++total;

  length = strlen( originalString );

  if( total == 0 )
  {
    resolvedString = XtNewString( originalString );
  }
  else
  {
    resolvedString = (char *)XtCalloc( 1, length + ( ( strlen( addressStr ) )
                                                     * total ) + 1 );
    if( resolvedString != NULL )
    {
      i = 0;
      j = 0;
      while( originalString[i] != '\0' )
      {
        if( originalString[i] == '@' )
        {
          for( k = 0; addressStr[k] != '\0'; ++j, ++k )
            resolvedString[ j ] = addressStr[k];
          ++i;
        }
        else
          resolvedString[ j++ ] = originalString[ i++ ];
      }
    }
  }

  return resolvedString;
}

