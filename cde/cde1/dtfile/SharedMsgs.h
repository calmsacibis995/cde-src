/* $XConsortium: SharedMsgs.h /main/cde1_maint/1 1995/07/17 20:49:44 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           SharedMsgs.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Functions and defines for accessing shared message
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Shared_Msgs_h
#define _Shared_Msgs_h

#ifdef _NO_PROTO
extern char * GetSharedMessage();
#else
extern char * GetSharedMessage(
                      int id) ;
#endif /* _NO_PROTO */


#define CANNOT_READ_DIRECTORY_ERROR   1
#define DIRECTORY_DATE_FORMAT         2
#define FILE_RENAME_ERROR_TITLE       3
#define FILE_MANIPULATION_ERROR_TITLE 4
#define CANNOT_CONNECT_ERROR_1        5
#define CANNOT_CONNECT_ERROR_2        6
#define CANT_READ_ERROR               7
#define CANT_WRITE_ERROR              8
#define CANT_CREATE_ERROR             9
#define CANT_DELETE_ERROR            10
#define CANT_OVERWRITE_ERROR         11
#define ACTION_FT_HELP_TITLE         12
#define FILES_FOUND_LABEL            13
#define LOCAL_RENAME_ONLY_ERROR      14
#define NO_DIR_ACCESS_ERROR          15
#define NO_FILES_FOUND_ERROR         16
#define FIND_ERROR_TITLE             17
#define FILE_COPY_ERROR_TITLE        20
#define NO_SPACES_ALLOWED_ERROR      21
#define MENU_OVERVIEW_LABEL          22
#define MENU_TUTORIAL_LABEL          23
#define MENU_ABOUT_LABEL             24
#define FILE_MGR_MENU_NAME           25
#define CHANGE_DIR_ERROR_TITLE       26
#define TRASH_ERROR_TITLE            27
#define FILTER_EDIT_ID_STRING        28
#define FILTER_ACTIVE_ID_STRING      29
#define UP_ONE_LEVEL_LABEL           30
#define ITEM_HELP_ERROR_TITLE        31
#define ITEM_HELP_ERROR              32
#define FILE_MOVE_ERROR_TITLE        33
#define FILE_LINK_ERROR_TITLE        34
#define AMITEM_HELP_ERROR            35

#endif /* _Shared_Msgs_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
