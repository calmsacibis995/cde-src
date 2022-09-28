/* $XConsortium: Prefs.h /main/cde1_maint/1 1995/07/17 20:48:51 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           Prefs.h
 *
 *   COMPONENT_NAME: Desktop File Manager
 *
 *   DESCRIPTION:    Public include file for the preferences dialog.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Preferences_h
#define _Preferences_h


/*  Resource names  */

#define SELECTION_LIST		"selectionList"
#define SHOW_TYPE		"showType"
#define TREE_FILES		"treeFiles"
#define VIEW			"view"
#define ORDER			"order"
#define DIRECTION		"direction"
#define RANDOM   		"random"


/*  Defines used as values for the preferences  */

#define UNSET_VALUE		99

#define SINGLE_DIRECTORY	0
#define MULTIPLE_DIRECTORY	1

#define TREE_FILES_NEVER	0
#define TREE_FILES_CHOOSE	1
#define TREE_FILES_ALWAYS	2

#define BY_NAME			0
#define BY_NAME_AND_ICON	1
#define BY_NAME_AND_SMALL_ICON	2
#define BY_ATTRIBUTES		3

#define ORDER_BY_FILE_TYPE	0
#define ORDER_BY_ALPHABETICAL	1
#define ORDER_BY_DATE		2
#define ORDER_BY_SIZE		3

#define DIRECTION_ASCENDING	0
#define DIRECTION_DESCENDING	1

#define RANDOM_ON               0
#define RANDOM_OFF              1


extern DialogClass * preferencesClass;


typedef struct
{
   Boolean       displayed;
   Position      x;
   Position      y;
   Dimension     width;
   Dimension     height;

   Boolean show_iconic_path;
   Boolean show_current_dir;
   Boolean show_status_line;
   unsigned char show_type;
   unsigned char tree_files;
   unsigned char view_single;
   unsigned char view_tree;
   unsigned char order;
   unsigned char direction;
   unsigned char positionEnabled;

} PreferencesData, * PreferencesDataPtr;


typedef struct
{
   XtCallbackProc   callback;
   XtPointer        client_data;
   XtPointer        preferences_rec;
} PreferencesApply;


typedef struct
{
   Widget shell;
   Widget show_iconic_path;
   Widget show_current_dir;
   Widget show_status_line;
   Widget show_single;
   Widget show_multiple;
   Widget tree_files_never;
   Widget tree_files_choose;
   Widget tree_files_always;
   Widget random_on;
   Widget random_off;
   Widget by_name;
   Widget by_name_and_icon;
   Widget by_name_and_small_icon;
   Widget by_attributes;
   Widget order_file_type;
   Widget order_alphabetical;
   Widget order_date;
   Widget order_size;
   Widget direction_ascending;
   Widget direction_descending;
   Widget position;

   Widget ok;
   Widget apply;
   Widget reset;
   Widget close;
   Widget help;

   unsigned char view_single, view_tree;

   PreferencesApply * apply_data;
} PreferencesRec;

#endif /* _Preferences_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
