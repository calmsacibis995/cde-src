/* $XConsortium: ChangeDir.h /main/cde1_maint/1 1995/07/17 20:34:32 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           ChangeDir.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Public include file for the change directory dialog.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _ChangeDir_h
#define _ChangeDir_h


extern DialogClass * changeDirClass;


typedef struct
{
   Boolean       displayed;
   Position      x;
   Position      y;
   Dimension     width;
   Dimension     height;
   char        * host_name;
   XmString      string_path;
   XmStringTable history_list;
   int           visible_count;
   int           list_count;
   XtPointer     file_mgr_rec;
} ChangeDirData, * ChangeDirDataPtr;


typedef struct
{
   XtCallbackProc  callback;
   XtPointer       change_dir_rec;
   XtPointer       client_data;
} ChangeDirApply;


typedef struct
{
   Widget shell;
   Widget change_dir;
   ChangeDirApply * apply_data;
} ChangeDirRec;


#endif /* _ChangeDir_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
