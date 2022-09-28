#ifndef _SDTFACL_UC_H_
#define _SDTFACL_UC_H_

#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/acl.h>
#include <sys/types.h>
#include <sys/param.h>

#define SDTFPROP_ICON		"SDtF.prop"
#define HELP_VOLUME		"Sdtfprop"
#define HELP_HOMETOPIC_ID	"_hometopic"

#define HELP_PROPERTIESDIALOG_ID	"FMPPropertiesDialogDE"
#define HELP_PERMISSSIONSDIALOG_ID	"FMPPermissionsDialogDE"
#define HELP_INFORMATIONDIALOG_ID	"FMPInformationDialogDE"
#define HELP_CHANGEACLDIALOG_ID		"FMPChangeACLDialogDE"
#define HELP_ADDACLDIALOG_ID		"FMPAddACLDialogDE"
#define HELP_CONFIRMCHANGESDIALOG_ID	"FMPConfirmChangesDialogDE"
#define HELP_CONFIRMEACHCHANGEDIALOG_ID	"FMPConfirmEachChangeDialogDE"

/* Logical file type defines */
#define LT_BROKEN_LINK		"BROKEN_LINK"
#define LT_RECURSIVE_LINK	"RECURSIVE_LINK"
#define LT_DOT_FOLDER		"DOT_FOLDER"
#define LT_CURRENT_FOLDER	"CURRENT_FOLDER"
#define LT_DIRECTORY		"FOLDER"
#define LT_DATA			"DATA"
#define LT_AGROUP		"AGROUP"
#define LT_AGROUP_SUBDIR	"APPGROUP"
#define LT_TRASH		"TRASH_FOLDER"
#define LT_FOLDER_LOCK		"FOLDER_LOCK"
#define LT_NON_WRITABLE_FOLDER	"NON_WRITABLE_FOLDER"

#define BUF_SIZ			1024

/*
 * Defines for the ACL entry field position in the list:
 * CLASS:	21 CHAR
 * NAME:	11 CHAR
 * PERMISSION:	13 CHAR
 * EFFECTIVE:	9 CHAR
 */
#define C_FIELD_SIZE		22
#define N_FIELD_SIZE		10
#define P_FIELD_SIZE		13
#define E_FIELD_SIZE		8
#define TOTAL_FIELD_SIZE	53

/* Checks for read, write, or execute access bit being set */
#define IS_RPERM(p)		(p & 4)
#define IS_WPERM(p)		(p & 2)
#define IS_XPERM(p)		(p & 1)

/* Gets User, Group, or Other permissions bits */
#define GET_UPERM(mode)		((mode & S_IRWXU) >> 6)
#define GET_GPERM(mode)		((mode & S_IRWXG) >> 3)
#define GET_OPERM(mode)		(mode & S_IRWXO)

#define GET_EPERM(perm, mask)	(perm & mask)

/* Defines for the type of pane that can be shown in the Properties Dialog */
enum {	INFO_PANE,
	PERM_PANE
};

/* Defines for the icon type on the Desktop */
enum {	DEFAULT,
	LARGE,
	SMALL
};

/* Defines for the operation type on the Modify Dialog */
enum {	ADD_OP,
	CHG_OP
};

/*
 * Variable "types" to be passed to the AlertPrompt function (that
 * function accepts a variable argument list with these attribute,
 * value pairs).
 *
 * The BUTTON_IDENT attribute can be used to identify 1 or more buttons
 * on the dialog.  The int id value will be returned if that button is
 * pressed and the char* name value will be the button label.  Button id
 * has to be greater than 0, and if the user close the window through the
 * window manager, a value of -1 will be return.
 *  
 */
enum {	DIALOG_TYPE = 1,	/* Followed by XmNdialogType */
	DIALOG_TITLE,		/* Followed by char* */
	DIALOG_STYLE,		/* Followed by XmNdialogStyle */
	DIALOG_TEXT,		/* Followed by char* */
	BUTTON_IDENT,		/* Followed by int, char* */
	HELP_IDENT		/* Followed by char* */
};

/*
 * Defines for Error severity to be passed to the ProcessErrorMsg function.
 */
enum {	FATAL = 1,		/* Display error msg and exit */
	ERROR,			/* Display error msg and return */
	WARNING,		/* Display warning msg and return */
	INFORMATION		/* Display informational msg and return */
};

/*
 * Defines button id values for Default Message Dialog.
 */
enum {	ANSWER_NONE = 0,
	ANSWER_ACTION_1,
	ANSWER_ACTION_2,
	ANSWER_ACTION_3,
	ANSWER_CANCEL,
	ANSWER_HELP
};

/* List Widget Entry record */
typedef struct _ListItemRec
{
    int			sort_order;
    aclent_t		acl_entry;
    struct _ListItemRec	*next;
} ListItemRec;

/* List Widget info record */
typedef struct
{
    int			mask;
    Boolean		has_def_user_obj;
    Boolean		has_def_group_obj;
    Boolean		has_def_other_obj;
    Boolean		has_def_class_obj;
    ListItemRec		*items;
} ListInfo;

/* List Entry Data structure */
typedef struct
{
    char	class[C_FIELD_SIZE];
    char	name[N_FIELD_SIZE];
    char	perm[P_FIELD_SIZE];
    char	eperm[E_FIELD_SIZE];
} ListEntryData;

/* Pixmap Data structure */
typedef struct
{
    int		size;
    char	*host_prefix;
    char	*instance_icon_name;
    char	*icon_name;
    char	*icon_file_name;
} PixmapData;

/* ACL Data structure */
typedef struct
{
    int		nentries;	/* Number of entries */
    aclent_t	*bufp;		/* File's ACL entries */
} ACLData;

/* Stat Data structure */
typedef struct stat StatData;

typedef struct
{
    String	file_name;	/* Name of the file */
    String	link_name;	/* Name of the linked-file */
    String	owner;		/* Name of the owner */
    String	group;		/* Name of the group */
    String	data_type;	/* Data type name of the file */
    StatData	*stat;		/* File info */
    ACLData	*acl;		/* File's ACL */ 
    PixmapData	*pixmap;	/* Icon image of the file */
} FileData;

typedef struct
{
    Boolean		is_browsable;	/* Browse button enable? */
    Boolean		is_acl_support;	/* OS supports ACL? */
    Boolean		is_root_user;	/* User is root? */
    Boolean		is_modifiable;	/* User can change file properties */
    Boolean		is_f_recursive;	/* Recursive option for files */
    Boolean		is_d_recursive;	/* Recursive option for directories */
    Boolean		is_confirm_chg; /* Ask for confirmation for changes */
    Boolean		is_merge_acl;	/* Merge ACL entries */
    int			recursive_depth;/* Level of sub-directory to process */
    int			current_pane;	/* Current pane being shown */
    int			saved_errno;	/* Saved errno value */
    uid_t		user_id;	/* Effective user ID */
    gid_t		group_id;	/* Effective group ID */
    int			ngroups;	/* Number of supplementary group */
    gid_t		*grouplist;	/* Supplementary group access list */ 
    String		name;		/* Name of this application */
    String		host;		/* Name of the host */
    String		directory;	/* Current working directory */
    Widget		iconType;
    Widget		topLevel;	/* Application top-level Widet */
    XtAppContext	appContext;	/* Application context */
    Display		*display;	/* Application display pointer */
    Screen		*screen;	/* Application screen pointer */
    FileData		*current_fdata;
    FileData		*new_fdata;
    ListInfo		list;		/* List Widget info */
} ProgramInfo;

typedef struct
{
    Boolean	initialized;
    Widget	fileSelDialogShell;
    Widget	fileSelDialog;
} DtbSdtfpropFileSDialogInfoRec, *DtbSdtfpropFileSDialogInfo;

typedef struct
{
    Boolean	initialized;
    Widget	dialogShell;
    Widget	dialog;
    Widget	form;
    Widget	label;
    Widget	okButton;
    Widget	cancelButton;
    Widget	helpButton;
    Widget      rbox1_rowcolumn;
    Widget	rbox1_label;
    Widget	rbox1;
    Widget	rbox1_toggleButton1;
    Widget	rbox1_toggleButton2;
    Widget      rbox2_rowcolumn;
    Widget	rbox2_label;
    Widget	rbox2;
    Widget	rbox2_toggleButton1;
    Widget	rbox2_toggleButton2;
} DtbSdtfpropRecurCDialogInfoRec, *DtbSdtfpropRecurCDialogInfo;

typedef struct
{
    Boolean	initialized;
    Widget	dialogShell;
    Widget	dialog;
    Widget	form;
    Widget	label;
    Widget	changeButton;
    Widget	doNotChangeButton;
    Widget	cancelButton;
    Widget	helpButton;
} DtbSdtfpropChangeCDialogInfoRec, *DtbSdtfpropChangeCDialogInfo;

typedef struct
{
    Boolean	initialized;
    Widget	dialogShell;
    Widget	dialog;
} DtbSdtfpropHelpDialogInfoRec, *DtbSdtfpropHelpDialogInfo;

extern DtbSdtfpropFileSDialogInfoRec dtb_sdtfprop_fileseldialog;
extern DtbSdtfpropRecurCDialogInfoRec dtb_sdtfprop_recurcdialog;
extern DtbSdtfpropChangeCDialogInfoRec dtb_sdtfprop_changecdialog;
extern DtbSdtfpropHelpDialogInfoRec dtb_sdtfprop_helpdialog;
extern ProgramInfo proginfo;

extern int (*acl_fptr)( char *, int, int, aclent_t* );
extern int (*aclcheck_fptr)( aclent_t*, int, int* );
extern int (*aclsort_fptr)( int, int, aclent_t* );

void Usage();
void ShowHelpDialog( char *, char * );
void CreateApplicationIcon();
void InitRun( char * );
void ShutdownProperties();
void ProcessErrorMsg( int, Boolean, ... );
void SetModDialogFooter( char * );
void ShowACL();
void HideACL();
void FreeFileData( FileData * );
void FreeListItems( ListItemRec * );
void GetGroups( int *, gid_t ** );
void SetPropDialogValues( FileData * );
void SetModDialogValues( int, int );
void SetModDialogPermValues( int );
void SetMaskValues( int );
void SelectListItemsEntry( int );
void ProcessPermissionChange();
void FileBrowserResponse_CB( Widget, char **,
                             XmFileSelectionBoxCallbackStruct * );
void MessageDialogQuitHandler( Widget, XtPointer, XtPointer );
void MessageDialogResponse_CB( Widget, XtPointer, XtPointer );
void AlertPromptHelp_CB( Widget, XtPointer, XtPointer );
void RecursiveConfirmPromptHelp_CB( Widget, XtPointer, XtPointer );
void FileNameChanged_CB( Widget, XtPointer, XtPointer );

Boolean LoadDynLibForACL();
Boolean ComparePermissionData( FileData *, FileData * );
Boolean ProcessPermisisonPaneExit( Boolean );

uid_t GetUserID( char * );
gid_t GetGroupID( char * );

int WalkFileTree( char*, int, Boolean * );
int LoadNewFile( char * );
int GetModDialogPermValues();
int GetModDialogClassType();
int GetMaskValues();
int UpdateFileOwnership( char *, StatData *, StatData * );
int UpdateFilePermission( char *, StatData *, ACLData *, StatData *, ACLData *,
                          Boolean );
int ConfirmAndUpdateFile( FileData *, Boolean * );
int CreateFileSDialog( DtbSdtfpropFileSDialogInfo, Widget );
int CreateRecursiveCDialog( DtbSdtfpropRecurCDialogInfo, Widget );
int RecursiveConfirmPrompt();
int ChangeConfirmPrompt( char *, Boolean, Boolean );
int AlertPrompt( Widget, ... );

char *ParseArgs( int, char ** );
char *GetFileName( char *, char *, char *, char * );

String ProcessPathFromInput( char *, char * );
String SearchAndReplace( char *, char *, char * );
String GetUserName( uid_t );
String GetGroupName( gid_t );
String CvtPermModeToString( int );
String CvtAclEntryToString( aclent_t );
Dimension CvtStringLengthToPixel( XmFontList, int );
ListEntryData *CvtAclEntryToListEntry( aclent_t );

ACLData *GetFileACL( char *, StatData * );
FileData *CreateFileData();
FileData *GetFileInfo( char * );
FileData *GetPropDialogValues();
aclent_t *GetModDialogValues();
aclent_t *GetListItemsEntry( ListItemRec *, int );
PixmapData *RetrievePixmapData( char *, int );
ListItemRec *CreateListItems( ACLData * );
ListItemRec *AddListItemsEntry( ListItemRec *, aclent_t, Boolean );
ListItemRec *ChangeListItemsEntry( ListItemRec *, aclent_t, int );
ListItemRec *DeleteListItemsEntry( ListItemRec *, int );
ListItemRec *UpdateListItems( ListItemRec * );

#endif	/* _SDTFACL_UC_H_ */
