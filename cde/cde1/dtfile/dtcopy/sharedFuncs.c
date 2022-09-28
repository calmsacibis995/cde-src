/* $XConsortium: sharedFuncs.c /main/cde1_maint/3 1995/10/09 14:53:53 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           sharedFuncs.c
 *
 *
 *   DESCRIPTION:    Common functions for both dtfile and dtfile_copy
 *
 *   FUNCTIONS: CreateDefaultImage
 *		ImageInitialize
 *		auto_rename
 *		build_path
 *		generate_NewPath
 *		split_path
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/


#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ustat.h>
#include <dirent.h>


#include <Xm/XmP.h>
#include <Xm/Xm.h>

#include <Xm/Form.h>
#include <Xm/PushB.h>

#include "sharedFuncs.h"
#include "dtcopy.h"



/************************************************************************
 *  Bitmap Data for Default Symbol
 **********************************<->***********************************/

static unsigned char errorBits[] = {
   0x00, 0x00, 0x00, 0xc0, 0x0f, 0x00, 0xf0, 0x3a, 0x00, 0x58, 0x55, 0x00,
   0x2c, 0xa0, 0x00, 0x56, 0x40, 0x01, 0xaa, 0x80, 0x02, 0x46, 0x81, 0x01,
   0x8a, 0x82, 0x02, 0x06, 0x85, 0x01, 0x0a, 0x8a, 0x02, 0x06, 0x94, 0x01,
   0x0a, 0xe8, 0x02, 0x14, 0x50, 0x01, 0x28, 0xb0, 0x00, 0xd0, 0x5f, 0x00,
   0xa0, 0x2a, 0x00, 0x40, 0x15, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char infoBits[] = {
   0x00, 0x00, 0x78, 0x00, 0x54, 0x00, 0x2c, 0x00, 0x54, 0x00, 0x28, 0x00,
   0x00, 0x00, 0x00, 0x00, 0x7e, 0x00, 0x2a, 0x00, 0x5c, 0x00, 0x28, 0x00,
   0x58, 0x00, 0x28, 0x00, 0x58, 0x00, 0x28, 0x00, 0x58, 0x00, 0x28, 0x00,
   0x58, 0x00, 0xae, 0x01, 0x56, 0x01, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char questionBits[] = {
   0xf0, 0x3f, 0x00, 0x58, 0x55, 0x00, 0xac, 0xaa, 0x00, 0xd6, 0x5f, 0x01,
   0xea, 0xbf, 0x02, 0xf6, 0x7f, 0x01, 0xea, 0xba, 0x02, 0xf6, 0x7d, 0x05,
   0xea, 0xba, 0x0a, 0x56, 0x7d, 0x15, 0xaa, 0xbe, 0x1e, 0x56, 0x5f, 0x01,
   0xac, 0xaf, 0x02, 0x58, 0x57, 0x01, 0xb0, 0xaf, 0x00, 0x60, 0x55, 0x01,
   0xa0, 0xaa, 0x00, 0x60, 0x17, 0x00, 0xa0, 0x2f, 0x00, 0x60, 0x17, 0x00,
   0xb0, 0x2a, 0x00, 0x50, 0x55, 0x00};

static unsigned char warningBits[] = {
   0x00, 0x00, 0x18, 0x00, 0x2c, 0x00, 0x56, 0x00, 0x2a, 0x00, 0x56, 0x00,
   0x2a, 0x00, 0x56, 0x00, 0x2c, 0x00, 0x14, 0x00, 0x2c, 0x00, 0x14, 0x00,
   0x2c, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3c, 0x00, 0x14, 0x00,
   0x2c, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00};

static unsigned char workingBits[] = {
   0x00, 0x00, 0x00, 0xfe, 0xff, 0x0f, 0xaa, 0xaa, 0x0a, 0x44, 0x55, 0x06,
   0xcc, 0x2a, 0x02, 0x44, 0x55, 0x06, 0xcc, 0x2a, 0x02, 0x84, 0x15, 0x06,
   0x8c, 0x2a, 0x02, 0x04, 0x15, 0x06, 0x0c, 0x0a, 0x02, 0x04, 0x06, 0x06,
   0x0c, 0x0b, 0x02, 0x84, 0x15, 0x06, 0xcc, 0x2a, 0x02, 0x44, 0x55, 0x06,
   0xcc, 0x2a, 0x02, 0x44, 0x55, 0x06, 0xcc, 0x2a, 0x02, 0x44, 0x55, 0x06,
   0xfe, 0xff, 0x0f, 0x56, 0x55, 0x05, 0x00, 0x00, 0x00};

Widget G_toplevel;
int    G_dialog_closed = FALSE;

/*----------------------------------------------------------
 *
 *  split_path
 *
 *  Given a path, return a pointer to the directory (folder)
 *  and file (object). On error, both will be set to empty
 *  strings.
 *
 *  The calling routine is responsible for allocating space
 *  for object and folder.
 *
 *  This function simply searches for the last slash (/) in
 *  the path and returns the characters preceeding the slash
 *  as folder and the characters after the last slash as
 *  object. Thus, object could be a directory/folder or a
 *  file.
 *
 *  There is a complimentary function, build_path, to put
 *  object and folder back together to form a path.
 *
 *-----------------------------------------------------------*/

void
#ifdef _NO_PROTO
split_path(path, folder, object)
	const String  path;
	      String  folder;
	      String  object;
#else
split_path(const String path, String folder, String object)
#endif

{

   String lastSlash;

   if ( (lastSlash = strrchr(path,'/')) != NULL)
   {
      strcpy(object,lastSlash+1);
      if(lastSlash == path)  /* Must be root Folder */
        strcpy(folder,"/");
      else
      {
        *lastSlash = '\0';
        strcpy(folder,path);
        *lastSlash = '/';
      }
   }
   else
   {
      folder[0] = object[0] = '\0';
   }

}  /* end split_path */


/*-----------------------------------------------------------------
 *
 *  build_path
 *
 *  Given a directory (folder) and file (object), build a string
 *  with the complete path and return it.
 *
 *  The calling routine is responsible for freeing storage used
 *  for the returned string.
 *
 *  There is a complimentary function, split_path, to take
 *  path apart to form folder and object
 *
 *-----------------------------------------------------------------*/

String
#ifdef _NO_PROTO
build_path(folder, object)
	const String  folder;
	const String  object;
#else
build_path(const String folder, const String object)
#endif

{
   char    s[MAX_PATH];
   String  path;

   strncpy(s, folder, MAX_PATH);
   strncat(s, "/",    MAX_PATH-strlen(s));
   strncat(s, object, MAX_PATH-strlen(s));
   path = (String) malloc(strlen(s)+1);
   strcpy(path,s);

   return path;

}  /* end build_path */



/*--------------------------------------------------------------
 *
 *  auto_rename
 *
 *  Given a path, generate a new file name and rename the file.
 *
 *  The rc from the system call is returned and errno is set.
 *
 *--------------------------------------------------------------*/

int
#ifdef _NO_PROTO
auto_rename(path)
   const String path;
#else
auto_rename(const String path)
#endif

{
   char newPath[MAX_PATH];

   generate_NewPath(newPath,path);
   errno = 0;
   return (rename(path,newPath));

}  /* end auto_rename */


/*-----------------------------------------------------------------------
 *
 *  generate_NewPath
 *
 *  Given a path (and a complete path is required), append a
 *  sequence number to generate a new file name. The new file
 *  will not exist. The sequence number consists of a one-character
 *  delimiter and an integer. The function will start with
 *  1, if the file with 1 exists it will continue to 2, etc.
 *  newPath and oldPath can point to the same area. The sequence number
 *  is appended to the end of the name unless it contains a
 *  dot, in which case the sequence number precedes the dot. For example,
 *  /cde/dtfile/dtcopy/overwrtdialog.c becomes
 *  /cde/dtfile/dtcopy/overwrtdialog~1.c. However, if the dot
 *  is the first character in the file name, the sequence number is
 *  appended, (e.g. .profile --> .profile~1).
 *
 *-----------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
generate_NewPath(newPath, oldPath)
   String  oldPath;
   String  newPath;
#else
generate_NewPath(String newPath, String oldPath)
#endif

{

   const char   delim = '_';

   struct stat  buf;
   String       lastDot, lastSlash;
   char         firstPart[MAX_PATH], lastPart[MAX_PATH];
   int          i = 0;
   int          len;


   lastDot   = strrchr(oldPath,'.');
   lastSlash = strrchr(oldPath,'/');
   if (lastSlash == NULL)
      lastSlash = oldPath - 1;   /* allows for no path and first char is dot */
   len = lastDot - oldPath;
   if ( lastDot != NULL &&            /* no dot */
        lastDot > lastSlash+1 &&      /* dot is in filename, not directory name */
        len != strlen(oldPath)-1 )    /* dot is not last character of filename */
   {
      /* sequence number will be inserted before filename suffix */
      memcpy(firstPart,oldPath,len);
      firstPart[len] = '\0';
      strcpy(lastPart,lastDot);
   }
   else
   {
      /* sequence number will be appended to filename */
      strcpy(firstPart,oldPath);
      lastPart[0] = '\0';
   }


   do
   {
      i++;
      sprintf(newPath,"%s%c%d%s",firstPart,delim,i,lastPart);
   } while (lstat(newPath,&buf) == 0);

   return;


}  /* end generate_NewPath */


/****************************************************************
 * Create a default images for symbol... used in ClassInitialize.
 ****************/

XImage *
#ifdef _NO_PROTO
CreateDefaultImage( display, bits, width, height )
	Display *display ;
        char *bits ;
        unsigned int width ;
        unsigned int height ;
#else
CreateDefaultImage(
	Display *display,
        char *bits,
        unsigned int width,
        unsigned int height )
#endif /* _NO_PROTO */
{
    XImage *image ;

    _XmCreateImage(image, display, bits, width, height, LSBFirst);

    return( image) ;
}  /* end CreateDefaultImage */



void
#ifdef _NO_PROTO
ImageInitialize(display)
	Display *display ;
#else
ImageInitialize( Display *display )
#endif /* _NO_PROTO */
{
    XImage *image;

    /* create and install the default images for the symbol */

    image = CreateDefaultImage (display, (char *)errorBits, 20, 20);
    XmInstallImage (image, "default_xm_error");

    image = CreateDefaultImage (display, (char *)infoBits, 11, 24);
    XmInstallImage (image, "default_xm_information");

    image = CreateDefaultImage (display, (char *)questionBits, 22, 22);
    XmInstallImage (image, "default_xm_question");

    image = CreateDefaultImage (display, (char *)warningBits, 9, 22);
    XmInstallImage (image, "default_xm_warning");

    image = CreateDefaultImage (display, (char *)workingBits, 21, 23);
    XmInstallImage (image, "default_xm_working");

    return ;
}  /*  end ImageInitialize */

static ino_t
#ifdef _NO_PROTO
CopyFileSysType(dev)
   int dev;
#else
CopyFileSysType(
   int dev)
#endif
{
  struct ustat u1;
  if(ustat(dev,&u1) < 0)
     return -2;
  return u1.f_tinode;
}

static int
#ifdef _NO_PROTO
CopyCheckDeletePermissionRecur(destinationPath)
  char *destinationPath;
#else
CopyCheckDeletePermissionRecur(
  char *destinationPath)
#endif
{
  struct stat statbuf;
  DIR *dirp;
  struct dirent * dp;
  Boolean first_file;
  char *fnamep;

  DPRINTF(("CheckDeletePermissionRecur(\"%s\")\n", destinationPath));

  if (lstat(destinationPath, &statbuf) < 0)
    return -1;  /* probably does not exist */

  if (! S_ISDIR(statbuf.st_mode))
  {
    if(access(destinationPath,04) < 0)
       return -1;
    return 0;   /* no need to check anything more */
  }

  dirp = opendir (destinationPath);
  if (dirp == NULL)
    return -1;  /* could not read directory */


  first_file = True;

  while (dp = readdir (dirp))
  {
    if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0)
    {
      if (first_file)
      {
        /* check for write permission in this directory */
        if (access(destinationPath, 04|02|01) < 0)
          return -1;

        /* append a '/' to the end of directory name */
        fnamep = destinationPath + strlen(destinationPath);
        *fnamep++ = '/';

        first_file = False;
      }

      /* append file name to end of directory name */
      strcpy(fnamep, dp->d_name);

      /* recursively check permission on this file */
      if (CopyCheckDeletePermissionRecur(destinationPath))
        return -1;
    }
  }

  return 0;
}

static int
#ifdef _NO_PROTO
CopyCheckDeletePermission(parentdir, destinationPath)
  char *parentdir,*destinationPath;
#else
CopyCheckDeletePermission(
  char *parentdir,
  char *destinationPath)
#endif
{
  struct stat statbuf;
  char fname[1024];

  if (lstat(parentdir,&statbuf) < 0)  /* does not exist */
    return -1;

  /* check if we are root */
  if (getuid() == 0)
  {
    /* if NFS, need to check if server trusts root */
    if (CopyFileSysType(statbuf.st_dev) < 0)  /* Root user and nfs */
    {
       char *tmpfile;
       tmpfile = tempnam(parentdir,"quang");
       if (creat(tmpfile,O_RDONLY)< 0)         /* Create a temporary file */
          return -1;
       if (remove(tmpfile) < 0)                /* Delete the created file */
          return -1;
    }

    /* root user can delete anything */
    return 0;
  }

  /* check for write and execute permisssion on parent dir */
  if (access(parentdir, 04|02 | 01) < 0)
      return -1;

  /* copy destinationPath to tmp buffer */
  strcpy(fname, destinationPath);

  return CopyCheckDeletePermissionRecur(fname);
}

void
#ifdef _NO_PROTO
CloseTopLevel(w,client_data,call_data)
     Widget w;
     void *client_data;
     void *call_data;
#else
CloseTopLevel(
     Widget w,
     void *client_data,
     void *call_data)
#endif
{
  XtDestroyWidget((Widget)G_toplevel);
  exit(-1);
}

void
#ifdef _NO_PROTO
CheckDeleteAccess(app_context,delay,checkPerms,move,source_name)
     XtAppContext app_context;
     int delay;
     Boolean checkPerms;
     Boolean move;
     char *source_name;
#else
CheckDeleteAccess(
     XtAppContext app_context,
     int delay,
     Boolean checkPerms,
     Boolean move,
     char *source_name)
#endif
{
  if(checkPerms && move)
  {
     char title[200],*msg,*tmpmsg;
     char *tmpstring = strdup(source_name),*tmpptr;
     XEvent event;
     int perm_status;

     delay = 10000;
     tmpptr = strrchr(tmpstring,'/');
     if(!tmpptr)                               /* Error */
       perm_status = 1;
     else
     {
       if(tmpptr == tmpstring)
          tmpptr = "/";
       else
       {
         *tmpptr = '\0';
         tmpptr = tmpstring;
       }
       perm_status = CopyCheckDeletePermission(tmpptr,source_name);
       free(tmpstring);
     }
     if(!perm_status)  /* Everything is fine just return */
          return;

     strcpy(title,GETMESSAGE(4,7,"Object Trash - Error"));
     tmpmsg =  GETMESSAGE(4,8,"You do not have permission to put the object \n\n%s\n\ninto trash.\n\nUse the Change Permissions choice from the object's\npopup menu or from the Selected menu to turn on your\nRead permission on the object.\n\nNote: If this object is a folder, you must also have\nRead permission for each of the objects inside the\nfolder before you can put the folder in the trash.");

     msg = XtMalloc(strlen(tmpmsg)+strlen(source_name)+2);
     sprintf(msg,tmpmsg,source_name);
     _DtMessageDialog (G_toplevel, title, msg, 0, FALSE, NULL,
         CloseTopLevel, NULL, NULL, False, ERROR_DIALOG);
/*
     XtAppAddTimeOut(app_context, delay, TimeoutHandler, NULL);
*/

    /* wait for user to close the dialog before exiting */
     XtFree(msg);
     while (!G_dialog_closed)
     {
         XtAppNextEvent(app_context, &event);
         XtDispatchEvent(&event);
     }
  }
}

void
#ifdef _NO_PROTO
TimeoutHandler(client_data, id)
        XtPointer client_data;
        XtIntervalId *id;
#else
TimeoutHandler(XtPointer client_data, XtIntervalId *id)
#endif
{
  exit(0);
}

