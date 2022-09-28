/* $XConsortium: ModAttrP.c /main/cde1_maint/3 1995/10/09 14:52:14 pascale $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           ModAttrP.c
 *
 *   COMPONENT_NAME: Desktop File Manager (dtfile)
 *
 *   Description:    Processing functions for the modify file attribute dialog.
 *
 *   FUNCTIONS:	AttrChangeProcess
 *		ModAttrChange
 *		ModAttrClose
 *		ShowModAttrDialog
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifdef __osf__
#include <stdio.h>
#endif
  
#include <grp.h>
#include <pwd.h>
#include <errno.h>
#include <ctype.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/RowColumn.h>

#include <Dt/Connect.h>
#include <Dt/FileM.h>

#include <Tt/tttk.h>

#include "Encaps.h"
#include "SharedProcs.h"
#include "FileMgr.h"
#include "Desktop.h"
#include "Main.h"
#include "Help.h"
#include "ModAttr.h"


extern int mod_attr_dialog;


/* types of messages sent through the pipe */
#define PIPEMSG_DONE 1

/* callback data for pipe input handler */
typedef struct
{
   char *host;
   char *directory;
   char *file;
   int child;
} AttrChangeCBData;


/********    Static Function Declarations    ********/
#ifdef _NO_PROTO

void ModAttrChange() ;
void ModAttrClose() ;

#else

void ModAttrChange(
                        XtPointer client_data,
                        DialogData *old_dialog_data,
                        DialogData *new_dialog_data,
                        XtPointer call_data) ;
void ModAttrClose(
                        XtPointer client_data,
                        DialogData *old_dialog_data,
                        DialogData *new_dialog_data) ;

#endif /* _NO_PROTO */
/********    End Static Function Declarations    ********/


/************************************************************************
 *
 *  ShowModAttrDialog
 *	Callback functions invoked from the Modify Attributes ... menu
 *	item.  This function displays the file attribute dialog.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
ShowModAttrDialog( w, client_data, callback )
        Widget w ;
        XtPointer client_data ;
        XtPointer callback ;
#else
ShowModAttrDialog(
        Widget w,
        XtPointer client_data,
        XtPointer callback )
#endif /* _NO_PROTO */
{
   FileMgrRec  * file_mgr_rec;
   DialogData  * dialog_data;
   FileMgrData * file_mgr_data;
   DialogData  * attr_dialog;
   DirectorySet * directory_set;
   FileViewData * file_view_data;
   ModAttrRec *modAttr_rec;
   int count, i;
   Boolean loadOk=True;
   char * errorMsg=NULL;
   char * title=NULL;
   Arg args[1];
   Widget mbar;
   char * tmpStr, *tempStr;
  
   XmUpdateDisplay (w);

   if((int)client_data != NULL)
      mbar = XtParent(w);
   else
      mbar = XmGetPostedFromWidget(XtParent(w));

   XtSetArg(args[0], XmNuserData, &file_mgr_rec);
   XtGetValues(mbar, args, 1);

   /* Ignore accelerators when we're insensitive */
   if (client_data == NULL)
      if ((file_mgr_rec->menuStates & MODIFY) == 0)
         return;

   /* Ignore accelerators received after we're unposted */
   if ((dialog_data = _DtGetInstanceData (file_mgr_rec)) == NULL)
      return;
   file_mgr_data = (FileMgrData *) dialog_data->data;

   /* get the file_view_data object from which the menu was invoked */
   if (client_data == NULL)
     file_view_data = NULL;
   else
   {
     file_view_data = file_mgr_data->popup_menu_icon;
     if (file_view_data == NULL)
       /* the object has gone away (probably deleted) */
       return;

     file_mgr_data->popup_menu_icon = NULL; /* Reset it, we don't need it */
   }


   attr_dialog = _DtGetDefaultDialogData (mod_attr_dialog);

   file_mgr_rec->menuStates &= ~MODIFY;
   if(attr_dialog && attr_dialog->data)
   {
     ModAttrData *modptr = (ModAttrData *) attr_dialog->data;
     modptr->IsToolBox = file_mgr_data->toolbox;
   }
   if (file_view_data == NULL)
   {
      DtActionArg args[1];

      directory_set =
         (DirectorySet *) file_mgr_data->selection_list[0]->directory_set;
      args[0].argClass = DtACTION_FILE;
      args[0].u.file.name =
		_DtGetSelectedFilePath( file_mgr_data->selection_list[0] );
      DtActionInvoke(file_mgr_rec->shell, "FileProperties",
		     args, 1, 0, 0, 0, 1, 0, 0 );
      XtFree( args[0].u.file.name );
      return;

      /* Determine which file is selected */
      loadOk =  LoadFileAttributes(file_mgr_data->host, directory_set->name,
                      file_mgr_data->selection_list[0]->file_data->file_name,
                      attr_dialog->data);
   }
   else
   {
      DtActionArg args[1];

      directory_set =
         (DirectorySet *)file_view_data->directory_set;
      args[0].argClass = DtACTION_FILE;
      args[0].u.file.name =
		_DtGetSelectedFilePath( file_view_data );
      DtActionInvoke(file_mgr_rec->shell, "FileProperties",
		     args, 1, 0, 0, 0, 1, 0, 0 );
      XtFree( args[0].u.file.name );
      return;

      /* Determine which file is selected */
      loadOk =  LoadFileAttributes(file_mgr_data->host, directory_set->name,
                      file_view_data->file_data->file_name,
                      attr_dialog->data);
   }

   if (loadOk)
   {
     /* Get the initial attribute values for seleceted file */
     _DtAddOneSubdialog (attr_dialog,
                      &file_mgr_data->attr_dialog_list,
                      &file_mgr_data->attr_dialog_count);
     /* We need the file_mgr_rec inside the create routine so
      * we attach it here for use later.
      */
     ((ModAttrData *) (attr_dialog->data))->main_widget = file_mgr_rec->main;

     _DtShowDialog (file_mgr_rec->shell, NULL, file_mgr_rec, attr_dialog,
                 ModAttrChange, file_mgr_rec, ModAttrClose,
                 file_mgr_rec,NULL, False, False, NULL, NULL,
                    False, 0, False, 0);

     for(i = 0; i < file_mgr_data->attr_dialog_count; i++)
     {
        modAttr_rec = (ModAttrRec *)_DtGetDialogInstance(
                                       file_mgr_data->attr_dialog_list[i]);

        if(file_mgr_data->title != NULL &&
                   strcmp(file_mgr_data->helpVol, DTFILE_HELP_NAME) != 0)
        {
           tmpStr = GETMESSAGE(21, 1, "File Permissions");
           tempStr = (char *)XtMalloc(strlen(tmpStr) +
                                 strlen(file_mgr_data->title) + 5);
           sprintf(tempStr, "%s - %s", file_mgr_data->title, tmpStr);
        }
        else
        {
           tmpStr = (GETMESSAGE(21,34, "File Manager - Permissions"));
           tempStr = XtNewString(tmpStr);
        }
        XtSetArg (args[0], XmNtitle, tempStr);
        XtSetValues (modAttr_rec->shell, args, 1);
        XtFree(tempStr);
      }
   }
   else
   {
     /* Clean up used structures and put up an error dialog */

     file_mgr_rec->menuStates |= MODIFY;
     _DtFreeDialogData (attr_dialog);
 
     tmpStr = GETMESSAGE(22, 4, "Cannot open object.");
     errorMsg = XtNewString(tmpStr);
     tmpStr = GETMESSAGE(22, 5, "Set Permissions Error");
     title = XtNewString(tmpStr);
     _DtMessage (file_mgr_rec->shell, title, errorMsg, NULL, HelpRequestCB);
     XtFree(title);
     XtFree(errorMsg);
   }
}




/************************************************************************
 *
 *  ModAttrChange
 *	Callback functions invoked from the file attribute dialog's
 *	Ok button being pressed.  This function updates the indicated
 *	file.
 *
 ************************************************************************/

/*--------------------------------------------------------------------
 * AttrChangeProcess
 *   Main routine of background process for AttrChange
 *------------------------------------------------------------------*/

static int
#ifdef _NO_PROTO
AttrChangeProcess(pipe_fd, host, directory, file, uid, gid, mode)
        int pipe_fd;
        char *host;
        char *directory;
        char *file;
        int uid;
        int gid;
        long mode;
#else /* _NO_PROTO */
AttrChangeProcess(
        int pipe_fd,
        char *host,
        char *directory,
        char *file,
        int uid,
        int gid,
        long mode)
#endif /* _NO_PROTO */
{
  char *path;
  struct stat stat_buf;
  long modify_time;
  short pipe_msg;
  int rc = -1;
  Tt_status tt_status;

  /* get directory timestamp */
  path = ResolveLocalPathName(host, directory, NULL,home_host_name, &tt_status);
  if( TT_OK == tt_status )
  {
    if (stat(path, &stat_buf) == 0)
      modify_time = stat_buf.st_mtime;
    XtFree(path);

    /* change the file attributes */
    path = ResolveLocalPathName(host, directory, file, home_host_name, &tt_status);
    if( TT_OK == tt_status )
    {
      if (chmod(path, mode) == 0)
      {
        rc = 0;
        chown(path, uid, gid);
      }
      else
        rc = errno;
      XtFree (path);
    }
  }

  /* send a 'done' msg through the pipe */
  DPRINTF(("AttrChangeProcess: sending DONE, rc %d\n", rc));
  pipe_msg = PIPEMSG_DONE;
  write(pipe_fd, &pipe_msg, sizeof(short));
  write(pipe_fd, &rc, sizeof(int));
  write(pipe_fd, &modify_time, sizeof(long));

  return rc;
}


/*--------------------------------------------------------------------
 * AttrChangePipeCB:
 *   Read and process data sent through the pipe.
 *------------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
AttrChangePipeCB(client_data, fd, id)
   XtPointer client_data;
   int *fd;
   XtInputId *id;
#else
AttrChangePipeCB(
   XtPointer client_data,
   int *fd,
   XtInputId *id)
#endif /* _NO_PROTO */
{
   AttrChangeCBData *cb_data = (AttrChangeCBData *)client_data;
   short pipe_msg;
   char *title, *err_msg, *err_arg;
   long modify_time;
   int i, n, rc;

   /* read the msg from the pipe */
   pipe_msg = -1;
   n = PipeRead(*fd, &pipe_msg, sizeof(short));

   if (pipe_msg == PIPEMSG_DONE)
   {
      PipeRead(*fd, &rc, sizeof(int));
      PipeRead(*fd, &modify_time, sizeof(long));
   }
   else
   {
      fprintf(stderr, "Internal error in AttrChangePipeCB: bad pipe_msg %d\n",
              pipe_msg);
      rc = 0;
      modify_time = 0;
   }

   DPRINTF(("AttrChangePipeCB: n %d, pipe_msg %d, rc %d\n", n, pipe_msg, rc));

   /* if error, post error message */
   if (rc)
   {
      err_msg = XtNewString(GETMESSAGE(22,3,"Cannot change properties for %s"));
      FileOperationError(toplevel, err_msg, cb_data->file);
      XtFree(err_msg);
   }

   /* arrange for modified directory to be updated */
   if (modify_time != 0)
      DirectoryModifyTime(cb_data->host, cb_data->directory, modify_time);
   DirectoryFileModified(cb_data->host, cb_data->directory, cb_data->file);
   DirectoryEndModify(cb_data->host, cb_data->directory);
   if(!strcmp(cb_data->file,".") || !strcmp(cb_data->file,".."))
   {
      char *dir;
      char *tmpptr = strrchr(cb_data->directory,'/');
      if(tmpptr)
      {
	if(tmpptr != cb_data->directory)
	   *tmpptr = '\0',dir = cb_data->directory;
        else
	   dir = "/";
      DirectoryBeginModify(cb_data->host,dir);
      DirectoryFileModified(cb_data->host,dir, tmpptr+1);
      DirectoryEndModify(cb_data->host,dir);

      if(tmpptr)
	*tmpptr='/';
      }
   }
   else
   {
     char subdir_name[MAX_PATH + 1];
     char *p;

     strcpy(subdir_name, cb_data->directory);
     p = subdir_name + strlen(subdir_name);
     if (p[-1] != '/')
       *p++ = '/';

     strcpy(p, cb_data->file);
     UpdateDirectory(NULL,cb_data->host, subdir_name);
   }
   /* close the pipe and cancel the callback */
   close(*fd);
   XtRemoveInput(*id);

   /* free callback data */
   XtFree(cb_data->host);
   XtFree(cb_data->directory);
   XtFree(cb_data->file);
   XtFree((char *)cb_data);
}


/*--------------------------------------------------------------------
 * ModAttrChange:
 *    Start the background process and set up callback for the pipe.
 *------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
ModAttrChange( client_data, old_dialog_data, new_dialog_data, call_data )
        XtPointer client_data ;
        DialogData *old_dialog_data ;
        DialogData *new_dialog_data ;
        XtPointer call_data ;
#else
ModAttrChange(
        XtPointer client_data,
        DialogData *old_dialog_data,
        DialogData *new_dialog_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
   AttrChangeCBData *cb_data;
   FileMgrRec  * file_mgr_rec = (FileMgrRec *) client_data;
   ModAttrData * current;
   ModAttrData * old;
   struct group * gr;
   struct passwd * pw;
   int uid, gid;
   long mode;
   int pipe_fd[2];
   int pid;
   int rc;

   /* Get current data */
   current = (ModAttrData *) new_dialog_data->data;
   old = (ModAttrData *) old_dialog_data->data;

   /* see if anything has changed */
   if (strcmp(current->owner, old->owner) == 0 &&
       strcmp(current->group, old->group) == 0 &&
       current->accessBits == old->accessBits &&
       current->setuidBits == old->setuidBits)
   {
      /* nothing changed: close the dialog and return */
       ModAttrClose (file_mgr_rec, old_dialog_data, new_dialog_data);
       return;
   }

   /* get new owner uid */
   pw = getpwnam(current->owner);
   if (pw)
      uid = pw->pw_uid;
   else if (isdigit(current->owner[0]))
      uid = atoi(current->owner);
   else
      uid = -1;

   /* get new group gid */
   gr = getgrnam(current->group);
   if (gr)
      gid = gr->gr_gid;
   else if (isdigit(current->group[0]))
      gid = atoi(current->group);
   else
      gid = -1;

   /* get new file mode */
   mode = current->accessBits + current->setuidBits;

   /* set up callback data */
   cb_data = XtNew(AttrChangeCBData);
   cb_data->host = XtNewString(current->host);
   cb_data->directory = XtNewString(current->directory);
   cb_data->file = XtNewString(current->name);

   /* mark the directory as being modified in the directory cache */
   DirectoryBeginModify(cb_data->host, cb_data->directory);

   /* create a pipe */
   pipe(pipe_fd);

   /* fork the process that does the actual work */
   pid = fork();
   if (pid == 0)
   {
      /* child process */
      DPRINTF(("AttrChange: child started\n"));
      close(pipe_fd[0]);  /* child won't read from the pipe */

      rc = AttrChangeProcess(pipe_fd[1], cb_data->host, cb_data->directory,
                             cb_data->file, uid, gid, mode);
      close(pipe_fd[1]);

      DPRINTF(("AttrChange: child ends (rc %d)\n", rc));
      exit(rc);
   }

   /* parent: set up callback to get the pipe data */
   close(pipe_fd[1]);  /* parent won't write the pipe */

   cb_data->child = pid;

   XtAppAddInput(XtWidgetToApplicationContext(toplevel),
                 pipe_fd[0], (XtPointer)XtInputReadMask,
                 AttrChangePipeCB, (XtPointer)cb_data);

   ModAttrClose (file_mgr_rec, old_dialog_data, new_dialog_data);
}




/************************************************************************
 *
 *  ModAttrClose
 *	Callback function invoked from the file attribute dialog's close
 *	button.
 *
 ************************************************************************/

void
#ifdef _NO_PROTO
ModAttrClose( client_data, old_dialog_data, new_dialog_data )
        XtPointer client_data ;
        DialogData *old_dialog_data ;
        DialogData *new_dialog_data ;
#else
ModAttrClose(
        XtPointer client_data,
        DialogData *old_dialog_data,
        DialogData *new_dialog_data )
#endif /* _NO_PROTO */
{
   FileMgrRec  * file_mgr_rec = (FileMgrRec *) client_data;
   FileMgrData * file_mgr_data;
   DialogData  * dialog_data;
   int count;
   int i, j;



   dialog_data = _DtGetInstanceData (file_mgr_rec);
   file_mgr_data = (FileMgrData *) dialog_data->data;

   if(file_mgr_data->selected_file_count > 0)
      file_mgr_rec->menuStates |= MODIFY;


   /*  Remove the dialog data from the file managers list  */
   _DtHideOneSubdialog( old_dialog_data,
                     &file_mgr_data->attr_dialog_list,
                     &file_mgr_data->attr_dialog_count);

   _DtHideDialog (old_dialog_data, False);
   _DtFreeDialogData (old_dialog_data);
   _DtFreeDialogData (new_dialog_data);
}
