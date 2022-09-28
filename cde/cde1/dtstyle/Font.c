 /****************************************************************************
 **
 **   File:        Font.c
 **
 **   Project:     DT 3.0
 **
 **   Description: Controls the Dtstyle Font dialog
 **
 **
 **  (c) Copyright Hewlett-Packard Company, 1990.  
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/
/* $Revision: 1.19 $ */

/*+++++++++++++++++++++++++++++++++++++++*/
/* include files                         */
/*+++++++++++++++++++++++++++++++++++++++*/
#include <X11/Xlib.h>
#include <Xm/MwmUtil.h>

#include <Xm/Xm.h>
#include <Xm/XmP.h>
#include <Xm/DrawingA.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/Label.h>
#include <Xm/LabelG.h>
#include <Xm/List.h>
#include <Xm/Scale.h>
#include <Xm/TextF.h>
#include <Xm/VendorSEP.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SelectioB.h>
#include <Xm/DialogS.h>
#include <Xm/BulletinB.h>
#include <Xm/Separator.h>

#include <Dt/DialogBox.h>
#include <Dt/Icon.h>
#include <Dt/TitleBox.h>

#include <Dt/Message.h>
#include <Dt/SessionM.h>
#include <Dt/HourGlass.h>
#include <Dt/HourGlass.h>
#include <Dt/ComboBox.h>

#include <string.h>
#include "Help.h"
#include "Main.h"
#include "SaveRestore.h"

#include <Dt/UserMsg.h>

#include <dirent.h>

/*
 * Definition of the fields of XLFD names
 */
#define XLFD_FOUNDRY    0
#define XLFD_FAMILY     1
#define XLFD_WEIGHT     2
#define XLFD_SLANT      3
#define XLFD_SETWIDTH   4
#define XLFD_ADD_STYLE  5
#define XLFD_PIXEL_SIZE 6
#define XLFD_POINT_SIZE 7
#define XLFD_RES_X      8
#define XLFD_RES_Y      9
#define XLFD_SPACING    10
#define XLFD_AVG_WIDTH  11
#define XLFD_REGISTRY   12
#define XLFD_ENCODING   13
#define XLFD_NUM_FIELDS 14
 
/*+++++++++++++++++++++++++++++++++++++++*/
/* Local #defines                        */
/*+++++++++++++++++++++++++++++++++++++++*/

#define FONT_MSG   ((char *)GETMESSAGE(5, 23, "Style Manager - Font"))
#define PREVIEW    ((char *)GETMESSAGE(5, 17, "Preview"))
#define IMMEDIATE  ((char *)GETMESSAGE(5, 18, "The changes to fonts will show up in some\napplications the next time they are started.\nOther applications, such as file manager and\napplication manager, will not show the font\nchanges until you Exit the desktop and then log\nback in.")) 
#define LATER      ((char *)GETMESSAGE(5, 19, "The selected font will be used when\n you restart this session."))
#define SYSTEM_MSG ((char *)GETMESSAGE(5, 20, "AaBbCcDdEeFfGg0123456789"))
#define USER_MSG   ((char *)GETMESSAGE(5, 21, "AaBbCcDdEeFfGg0123456789"))
#define SIZE       ((char *)GETMESSAGE(5, 22, "Size"))
#define FONT_GROUP ((char *)GETMESSAGE(5, 74, "Font Group"))
#define XXSmall    ((char *)GETMESSAGE(5, 75, "XXSmall(1)"))
#define XSmall     ((char *)GETMESSAGE(5, 76, "XSmall(2)"))
#define Small      ((char *)GETMESSAGE(5, 77, "Small(3)"))
#define Medium     ((char *)GETMESSAGE(5, 78, "Medium(4)"))
#define Large      ((char *)GETMESSAGE(5, 79, "Large(5)"))
#define XLarge     ((char *)GETMESSAGE(5, 80, "XLarge(6)"))
#define XXLarge    ((char *)GETMESSAGE(5, 81, "XXLarge(7)"))

#define Attributes ((char *)GETMESSAGE(5, 82, "Attributes..."))

#define DUP_GRP ((char *)GETMESSAGE(5, 83, "A Font Group named '%s' already\
 exists.\nThis new group will overwrite the old one.\nIs this what you\
 want to do?"))
#define DEL_GRP ((char *)GETMESSAGE(5, 84, "Delete Font Group '%s'?\n"))

#define SPACING    ((char *)GETMESSAGE(5, 85, "Alias Family:"))
#define ALIAS      ((char *)GETMESSAGE(5, 86, "Alias:"))
#define ALIAS_XLFD ((char *)GETMESSAGE(5, 87, "Alias XLFD:"))
#define LOCATION   ((char *)GETMESSAGE(5, 88, "Location:"))
#define FONT_GRP_ATTRIBUTES  ((char *)GETMESSAGE(5, 89,"Font Group Attributes"))
#define FONT_XLFD  ((char *)GETMESSAGE(5, 90, "Font XLFD:"))
#define READ_ONLY_STR  ((char *)GETMESSAGE(5, 91, "[Read Only]"))
#define PUSH_BUTTON ((char *)GETMESSAGE(5, 92, "Push Button"))
#define ALIAS_NOT_FOUND ((char *)GETMESSAGE(5, 93, "Alias not found"))
#define PROPORTIONAL ((char *)GETMESSAGE(5, 94, "System"))
#define MONO         ((char *)GETMESSAGE(5, 95, "User"))
#define FONT_GRP_NAME ((char *)GETMESSAGE(5, 97, "New Font Group Name:"))
#define Default ((char *)GETMESSAGE(5, 99, "Default"))

#define  ERR1  ((char *)GETMESSAGE(16, 11, "Unable to delete '%s'.\n"))
#define  ERR3  ((char *)GETMESSAGE(16, 19, "Could not open directory %s."))
#define  ERR4  ((char *)GETMESSAGE(16, 20, "Could not open %s."))

#define CANT_DELETE ((char *)GETMESSAGE(5, 101, "The Font Group could not be\
 deleted.\nYou don't have permissions to delete this font\ngroup."))
#define BOLD         ((char *) GETMESSAGE(5, 103, "User Bold"))
#define ADD_FONT_GROUP ((char *)GETMESSAGE(5, 104, "Add Font Group"))
#define CHARSET ((char *)GETMESSAGE(5, 105, "Character set: %s (%d of %d)"))
#define CHSET    ((char *)GETMESSAGE(5, 106, "Charset:"))
#define PREV_MSG    ((char *)GETMESSAGE(5, 107, "Preview Window - Font"))


#define DEFAULT_FONT_PATH  "/usr/openwin/lib/X11/sdtfonts/"
#define SYS_FONT_PATH      "/etc/dt/sdtfonts/" 
#define USER_FONT_DIR      "/.dt/sdtfonts/" 
#define XSET               "/usr/openwin/bin/xset"




#define PREVIEW_WIN_WIDTH 440
#define PREVIEW_WIN_HEIGHT 200
#define BIGNUM 32767
#define ATTR_CANCEL_BUTTON 1
#define ATTR_HELP_BUTTON 2

static int pointSize[] = {
	10,
	12,
	14,
	17,
	18,
	20,
	24
};

static char *SizeStr[8] = {
	"xxs",
	"xs",
	"s",
	"m",
	"l",
	"xl",
	"xxl",
	"scalable",
};

#define scalable   (1 << 7)
#define xxlarge    (1 << 6)
#define xlarge     (1 << 5)
#define large      (1 << 4)
#define medium     (1 << 3)
#define small      (1 << 2)
#define xsmall     (1 << 1)
#define xxsmall    (1 << 0)


/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Variables                    */
/*+++++++++++++++++++++++++++++++++++++++*/

typedef struct {
    Widget fontWkarea;
    Widget sizeTB;
    Widget prevDrawWindow;
    Widget sizeList;
    Widget groupTB;
    Widget groupList;
    Widget attributes;
    Widget fontGroupForm;
    Widget addButton;
    Widget deleteButton;
    Widget addPropList;
    Widget addMonoList;
    Widget addForm;
    Widget addMonoTB;
    Widget addPropTB;
    Widget dupGroup;
    int    originalFontIndex;
    int    selectedFontIndex;
    String selectedFontStr;
    String originalGroupStr;
    String selectedGroupStr;
} FontData;

typedef struct {
    char *label;
    void (*callback)();
    caddr_t data;
} ActionAreaItem;

static int first_time = 0;
static FontData font;
static saveRestore save = {FALSE, 0, };

/*+++++++++++++++++++++++++++++++++++++++*/
/* Internal Functions                    */
/*+++++++++++++++++++++++++++++++++++++++*/

#ifdef _NO_PROTO

static void CreateFontDlg() ;
static void do_preview() ;
static void _DtmapCB_fontBB() ;
static void ButtonCB() ;
static void attr_ButtonCB() ;
static void add_ButtonCB() ;
static void deleteOkCB() ;
static void OverWriteGroupCB();
static void deleteCancelCB() ;
static void add_groupCB() ;
static void fontGrpDlg() ;
static char *AddFontGroup() ;
static Boolean CreateSdtFontDir() ;
static void changeSampleFontCB() ;
static void changeSampleGroupCB() ;
static FgroupRec *ScanFontGroups() ;
static XmString *FgroupList();
static Widget CreateSpacingOption ();
static void OptionChanged (); 
static void CharsetChanged (); 
static void create_attr_dlg() ;
static void dlg_event_handler (); 
static FgroupRec *gen_font_info_list();
static void split_line();
static void get_first_token_from_str();
static Boolean splitXLFDName();
static char *get_hrn();
static void set_read_only_str ();
static FgroupRec *find_group_selected();
static FgroupRec *find_group_prev();
static void set_attribute_fields();
static void set_font_preview();
static char *set_size();
static fontInfo *find_size_selected();
static FgroupRec *initDefaultFontInfo();
static void addButtonCB();
static void deleteButtonCB
static void initFontInfoList();
static FgroupRec *getFontInfo();
static FgroupRec *createFontRec();
static void construct_list();
static void AddGroupToList();
static void SameName();
static Boolean RemoveGroup();
static void select_def_items();
static void add_mono_to_list();
static void add_prop_to_list();
static char *get_alias();
static char *get_font_defn();
static void WriteFontInfo();
static fontInfo *get_final_list();
static void create_temp_fi_list();
static int  ScaleFromSize();
static void create_other_sizes();
static void get_sz_to_use();
static int SizeStrToPixelSize();
static void DeleteFiList();
static void DeleteFgroup();
static int find_size_to_map();
static char *FindFontGroup();
static void rem_fgrp_from_path();
static void set_fgrp_to_path();
static fontInfo *find_bolder_group();
static fontInfo *find_bold();
static void set_other_charset();
static char *tmpFontGroup();
static void writeTmpFontGrp();
static void appendToHome();
static void cleanUserTmp();
static void prepend_iso8859();


#else

static void CreateFontDlg( 
                        Widget parent) ;
static void do_preview( 
                        Widget parent) ;
static void _DtmapCB_fontBB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void ButtonCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void attr_ButtonCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void add_ButtonCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void deleteOkCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void OverWriteGroupCB(
        		Widget w,
        		XtPointer client_data,
        		XtPointer call_data);
static void deleteCancelCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void add_groupCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void fontGrpDlg(void); 
static char *AddFontGroup(
			char *name,
			char *fgrp);
static Boolean CreateSdtFontDir(
			char **home_group_dir);
static void changeSampleFontCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void changeSampleGroupCB( 
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static FgroupRec *ScanFontGroups(
			char *dir,
			int  *count);
static char      **FgroupList(
        		int  count_groups);

static Widget CreateSpacingOption ( Widget parent);
static void OptionChanged ( Widget    w,
                            int       which,
                            DtComboBoxCallbackStruct *state);
static void CharsetChanged ( Widget    w,
                            int       which,
                            DtComboBoxCallbackStruct *state);

static void create_attr_dlg(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;

static void dlg_event_handler (Widget w, 
                               XtPointer client_data, 
			       XEvent *event,
                               Boolean *dispatch);
static FgroupRec *gen_font_info_list( char     *group_name);

static void split_line(
		char *line, 
		char *alias, 
		char *font);
 
static void get_first_token_from_str(
		char *src, 
		char *dest, 
		char *separators);

static Boolean splitXLFDName(
    		char   *XLFDName,
    		char ***XLFDNameFields,
    		char  **freeMe);

static char *get_hrn(
		char **xlfd_field);

static void set_read_only_str ( 
		char *buf );

static FgroupRec *find_group_selected(
		char *group_name,
		FgroupRec *font_list);
static FgroupRec *find_group_prev(
		char *group_name,
		FgroupRec *font_list);

static void set_attribute_fields(void);

static void set_font_preview(void);


static char *set_size( char **xlfd_field);

static fontInfo *find_size_selected(
        	fontInfo *fontElement,
        	ScaleType sc);

static FgroupRec *initDefaultFontInfo(void);

static void addButtonCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void deleteButtonCB(
                        Widget w,
                        XtPointer client_data,
                        XtPointer call_data) ;
static void initFontInfoList(
			int i,
			char **charset_list);
static FgroupRec *getFontInfo( 
		        char	 *font_name,
			char     *familyLabel, 
			char     *familyLabel_charset, 
                	char **xlfd_field,
			Boolean   createInfo);
static FgroupRec *createFontRec(
		        char	 *font_name,
			char    **xlfd_field,
    			char     *familyLabel,
    			FgroupRec *parent,
    			FgroupRec *child);
static void construct_list(
			char	*newtext,
			list_type which_list,
			Widget list_w);
static void AddGroupToList(
		        char    *newGroup);
static void SameName(
        		Widget w,
        		char *name,
			char *fgrp);
static Boolean RemoveGroup(
        		char *group_name);
static void select_def_items(void);
static void add_mono_to_list(
        		FgroupRec *fi, 
			char *font_name, 
			char  **XLFDNameFields);
static void add_prop_to_list(
        		FgroupRec *fi, 
			char *font_name, 
			char **XLFDNameFields);
static char *get_alias(
        		char *font_name, 
			list_type which_type,
			char *sz_string);
static void WriteFontInfo(
        		char *user_group,
        		char *system_group,
        		char *name);
static fontInfo *get_final_list(
        		fontInfo *fi, list_type type_face);
static void create_temp_fi_list(
        		fontInfo *temp_fi, 
			char *sz_string, 
			list_type type_face);
static int  ScaleFromSize( 	
			char **XLFDNameFields);
static void create_other_sizes(
        		fontInfo *temp_fi,
        		fontInfo *orig_fi,
        		int size,
			list_type type_face);
static void get_sz_to_use(
      			int scale, 
			int sc, 
			int *new_diff, 
			int *cur_diff, 
			int *sz_to_use);
static int SizeStrToPixelSize(
			char *sz_string);
static char *get_font_defn( 
			char *font_name, 
			char *sz_string);
static void DeleteFiList(
			fontInfo *orig_fi); 
static void DeleteFgroup(
        		FgroupRec *prev, 
			FgroupRec *fgroup);
static int find_size_to_map(
        		int size, int sc);
static char *FindFontGroup(void);
static void rem_fgrp_from_path(char *fgrp);
static void set_fgrp_to_path(char *fgrp);
static fontInfo *find_bolder_group( FgroupRec *fgroup);
static fontInfo *find_bold (FgroupRec *fgroup, char *weight);
static void set_other_charset( int charset_num);
static char *tmpFontGroup( char *name);
static void writeTmpFontGrp(int charset_num);
static void appendToHome(
        		char *user_tmp,
        		char *home_group);
static void cleanUserTmp( void );
static void prepend_iso8859( void );

#endif /* _NO_PROTO */



static Widget prev_dialog = NULL;
static Widget prev_pane = NULL;
static Widget prev_label = NULL;
static Widget prev_text = NULL;
static Widget prev_push_button = NULL;
static Widget attr_dlg = NULL;
static Widget add_dlg = NULL;
static Widget fontGrpName_Dlg = NULL;
static Widget font_group;
static Widget font_size;
static Widget font_size_val;
static Widget font_group_name;
static Widget separator1;
static Widget alias_label;
static Widget alias_name;
static Widget alias_xlfd_name;
static Widget alias_xlfd_label;
static Widget alias_location_label;
static Widget alias_location_name;
static Widget separator2;
static Widget font_label;
static Widget font_name;
static Widget font_xlfd_label;
static Widget font_xlfd_name;
static Widget deleteDialog;
static Widget charset_label;

 
static char option_selected[10] = "prop";
FgroupRec	     *font_groups = NULL;
FgroupRec	     *new_font_groups = NULL;
FgroupRec            *cur_font_group = NULL;
fontInfo	     *new_font_list = NULL;
char 		     home_fontGroup_dir[MAXPATHLEN];
char 		     name[MAXNAMLEN];
char		     fgrp[MAXNAMLEN];
char 		     **charset_list;
int 	             charset_count = 0;
char		     *tmp_fgdir = NULL;
int		     charset_selected = 0;



/*+++++++++++++++++++++++++++++++++++++++*/
/* popup_fontBB                          */
/*+++++++++++++++++++++++++++++++++++++++*/
void 
#ifdef _NO_PROTO
popup_fontBB( shell )
        Widget shell ;
#else
popup_fontBB(
        Widget shell )
#endif /* _NO_PROTO */
{
Position x,y;

  if (style.fontDialog == NULL) {
    _DtTurnOnHourGlass(shell);  
    CreateFontDlg(shell); 
    XtManageChild(style.fontDialog);
    do_preview(style.fontDialog);
    raiseWindow(XtWindow(prev_dialog));

    _DtTurnOffHourGlass(shell);  
  }
  else { 
    XtManageChild(style.fontDialog);

    XtVaSetValues(prev_pane, 
		XmNwidth,  PREVIEW_WIN_WIDTH - 25,
        	XmNheight, PREVIEW_WIN_HEIGHT - 40,
		NULL);

    XtManageChild(prev_pane); 

    raiseWindow(XtWindow(XtParent(style.fontDialog)));
    raiseWindow(XtWindow(prev_dialog));
  }
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* CreateFontDlg                         */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
CreateFontDlg( parent )
        Widget parent ;
#else
CreateFontDlg(
        Widget parent )
#endif /* _NO_PROTO */
{
    register int     n;
    int              i, ch_num;
    Arg              args[MAX_ARGS];
    XmString         button_string[NUM_LABELS];
    XmString         string;
    int              count = 0, actual_count;
    Widget           widget_list[10];
    XmString         *sizeItems;
    XmStringTable    selectedSize;
    Dimension        fontheight;
    char             **groupList;
    int		     count_groups = 0;
    FgroupRec        *fgroup;
    char	     tmpGrpdir[MAXPATHLEN];
    char	     tmpStr[MAXNAMLEN], *def_str;
    char             *xlfd_name = "xyz-foobar"; 
    char             **tmp_charset;

    font.selectedFontStr = style.xrdb.systemFontStr;
    font.selectedGroupStr = style.xrdb.fontGroup;
    font.originalGroupStr = style.xrdb.fontGroup;

    /* Look for the selectedFont in the fontChoice array and set 
     * selectedFontIndex to that entry
     */

    for (i=0; i<style.xrdb.numFonts; i++)
        if (strcmp (font.selectedFontStr, 
                    style.xrdb.fontChoice[i].sysStr) == 0)
        {
            font.selectedFontIndex = i;        
            if (!style.xrdb.fontChoice[i].userFont)
                GetUserFontResource(i);
            if (!style.xrdb.fontChoice[i].sysFont)
                GetSysFontResource(i);
            break;    
        }            

/* The following test will never be true because 
   font.selectedFontStr is set = to 
   style.xrdb.systemFontStr above and nothing changes
   it before this point.  Therefore it is being blocked out.
*/
#if 0
    /* If the systemFontStr is different from the selectedFontStr,
    /* Set originalFontIndex for systemFontStr in the fontChoice array.
     * Else both indexes are the same.
     */

    if (strcmp (style.xrdb.systemFontStr, font.selectedFontStr) != 0)
    {
        for (i=0; i<style.xrdb.numFonts; i++)
            if (strcmp (style.xrdb.systemFontStr, 
                        style.xrdb.fontChoice[i].sysStr) == 0)
            {
                font.originalFontIndex = i;        
                break;    
            }            
    }
    else 
#endif

    font.originalFontIndex = font.selectedFontIndex;

    /* Set up button labels. */
    button_string[0] = CMPSTR(_DtOkString);
    button_string[1] = CMPSTR(_DtCancelString);
    button_string[2] = CMPSTR(_DtHelpString);

    /* Create toplevel DialogBox */
    /* saveRestore
     * Note that save.poscnt has been initialized elsewhere.  
     * save.posArgs may contain information from restoreFont().*/

    XtSetArg(save.posArgs[save.poscnt], XmNallowOverlap, False); save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNdefaultPosition, False); 
    save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNbuttonCount, NUM_LABELS);  
    save.poscnt++;
    XtSetArg(save.posArgs[save.poscnt], XmNbuttonLabelStrings, button_string); 
    save.poscnt++;
    style.fontDialog = 
        __DtCreateDialogBoxDialog(parent, "Fonts", save.posArgs, save.poscnt);
    XtAddCallback(style.fontDialog, XmNcallback, ButtonCB, NULL);
    XtAddCallback(style.fontDialog, XmNmapCallback, _DtmapCB_fontBB, 
                            (XtPointer)parent);
    XtAddCallback(style.fontDialog, XmNhelpCallback,
            (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_FONT_DIALOG);
    XtAddEventHandler(XtParent(style.fontDialog),
             StructureNotifyMask, False, dlg_event_handler, NULL);

    XmStringFree(button_string[0]);
    XmStringFree(button_string[1]);
    XmStringFree(button_string[2]);

    widget_list[0] = _DtDialogBoxGetButton(style.fontDialog,2);
    n=0;
    XtSetArg(args[n], XmNautoUnmanage, False); n++;
    XtSetArg(args[n], XmNcancelButton, widget_list[0]); n++;
    XtSetValues (style.fontDialog, args, n);

    n=0;
    XtSetArg(args[n], XmNtitle, FONT_MSG); n++;
    XtSetArg(args[n], XmNuseAsyncGeometry, True); n++;
    XtSetArg(args[n], XmNmwmFunctions, DIALOG_MWM_FUNC ); n++;

    XtSetValues (XtParent(style.fontDialog), args, n);

    n = 0;
    XtSetArg (args[n], XmNchildType, XmWORK_AREA);  n++;
    XtSetArg(args[n], XmNhorizontalSpacing, style.horizontalSpacing); n++;
    XtSetArg(args[n], XmNverticalSpacing, style.verticalSpacing); n++;
    XtSetArg(args[n], XmNallowOverlap, False); n++;
    font.fontWkarea = XmCreateForm(style.fontDialog, "fontWorkArea", args, n);


    /* Create a TitleBox and Scale/List to choose the font size */

	
    n = 0;
    string = CMPSTR(FONT_GROUP);
    XtSetArg(args[n], XmNtitleString, string);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.fontWkarea);  n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);       n++; 
    XtSetArg(args[n], XmNleftOffset,         style.horizontalSpacing);  n++; 
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_POSITION);       n++;
    XtSetArg(args[n], XmNrightPosition,      70);       n++;
    widget_list[count++] = font.groupTB =
        _DtCreateTitleBox(font.fontWkarea, "groupTB", args, n); 
    XmStringFree(string);

    /* calculate size for each of the fonts based on system font size */

    sizeItems = (XmString *) XtMalloc(sizeof(XmString) * style.xrdb.numFonts);
    for (n=0; n<style.xrdb.numFonts; n++)
      {
        switch(n) {
           case 0:
	         sizeItems[n] = CMPSTR(XXSmall); 
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(XXSmall); 
		 break;
	   case 1:
	         sizeItems[n] = CMPSTR(XSmall); 
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(XSmall); 
		 break;
	   case 2:
	         sizeItems[n] = CMPSTR(Small);
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(Small); 
		 break;
	   case 3:
	         sizeItems[n] = CMPSTR(Medium);
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(Medium); 
		 break;
	   case 4:
	         sizeItems[n] = CMPSTR(Large);
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(Large); 
		 break;
	   case 5:
	         sizeItems[n] = CMPSTR(XLarge);
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(XLarge); 
		 break;
	   case 6:
	         sizeItems[n] = CMPSTR(XXLarge);
	         style.xrdb.fontChoice[n].pointSize = CMPSTR(XXLarge); 
		 break;
	   default:
		 break;
	}

      }
  
    if((font_groups = initDefaultFontInfo()) == NULL)
	fprintf(stderr, "No default font group found. \n");
    else {
	count_groups = 1;
        cur_font_group = font_groups;
    }

    sprintf(home_fontGroup_dir, "%s%s%s/", 
		style.home, DT_FONT_GROUP_DIR, style.lang);

    font_groups = ScanFontGroups(home_fontGroup_dir, &count_groups);

    sprintf(tmpGrpdir, "%s%s/", SYS_FONT_PATH, style.lang);

    font_groups = ScanFontGroups(tmpGrpdir, &count_groups);

    sprintf(tmpGrpdir, "%s%s/", DEFAULT_FONT_PATH, style.lang);

    font_groups = ScanFontGroups(tmpGrpdir, &count_groups);


    groupList = FgroupList(count_groups); 

   
    /*
    **  Create a form inside font Group titlebox
    */
    n = 0;
    XtSetArg(args[n], XmNhorizontalSpacing, style.horizontalSpacing); n++;
    XtSetArg(args[n], XmNverticalSpacing, style.verticalSpacing); n++;
    font.fontGroupForm = XmCreateForm(font.groupTB, 
					"formForFontGroup", args, n);
    XtManageChild(font.fontGroupForm);


    n=0;
    XtSetArg (args[n], XmNtopAttachment,     XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNrightAttachment,   XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNleftAttachment,    XmATTACH_FORM);  n++;
    XtSetArg (args[n], XmNscrollBarDisplayPolicy,    XmSTATIC); n++;
    XtSetArg (args[n], XmNselectionPolicy,    XmBROWSE_SELECT); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    XtSetArg (args[n], XmNvisibleItemCount,   7); n++;
    /*  
     * if a font group is not found, select Default.
     */
    if((fgroup = gen_font_info_list(style.xrdb.fontGroup)) == NULL)
	string = CMPSTR(Default);
    else 
        string = CMPSTR(style.xrdb.fontGroup);

    selectedSize = &(string);
    XtSetArg (args[n], XmNselectedItems, selectedSize); n++;
    XtSetArg (args[n], XmNselectedItemCount, 1); n++;
    font.groupList = XmCreateScrolledList(font.fontGroupForm,
							"groupList",args,n);

    XtAddCallback(font.groupList, XmNbrowseSelectionCallback,
                                         changeSampleGroupCB, NULL);

    for(n = 0; n < count_groups ; ++n)
	AddGroupToList(groupList[n]);

    XtFree((char *)groupList);
    XmStringFree(string);

    /* Create Add button */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNtopWidget,          font.groupList);  n++;
    XtSetArg (args[n], XmNtopOffset, style.horizontalSpacing);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_FORM);  n++;

    string = CMPSTR(((char *)GETMESSAGE(14, 6, "Add...")));
    XtSetArg (args[n], XmNlabelString, string); n++;

    font.addButton =
            XmCreatePushButtonGadget(font.fontGroupForm, "add", args, n);

    XmStringFree(string);
    XtManageChild(font.addButton);

    XtAddCallback(font.addButton, XmNactivateCallback, 
		addButtonCB, (XtPointer) style.fontDialog);

    /* Create Delete button */
    n = 0;
    XtSetArg (args[n], XmNtopAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNtopWidget, font.groupList);  n++;
    XtSetArg (args[n], XmNleftAttachment, XmATTACH_WIDGET);  n++;
    XtSetArg (args[n], XmNleftWidget,     font.addButton);  n++;
    XtSetArg (args[n], XmNleftOffset, style.horizontalSpacing);  n++;
    string = CMPSTR(((char *)GETMESSAGE(14, 7, "Delete...")));
    XtSetArg (args[n], XmNlabelString, string); n++;
    font.deleteButton =
                XmCreatePushButtonGadget(font.fontGroupForm,"delete",args,n);

    XmStringFree(string);
    XtManageChild(font.deleteButton);

    XtAddCallback(font.deleteButton, XmNactivateCallback, 
				deleteButtonCB, (XtPointer) NULL);


    /* preview TitleBox */
    n = 0;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.fontWkarea);  n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_WIDGET);     n++; 
    XtSetArg(args[n], XmNleftWidget,         font.groupTB);         n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
/*
    XtSetArg(args[n], XmNbottomAttachment,   XmATTACH_OPPOSITE_WIDGET);   n++;
    XtSetArg(args[n], XmNbottomWidget,       font.groupTB);       n++;
    XtSetArg(args[n], XmNbottomOffset,       style.verticalSpacing);    n++;
*/

    string = CMPSTR(SIZE); 
    XtSetArg(args[n], XmNtitleString, string); n++;

    widget_list[count++] = font.sizeTB =
        _DtCreateTitleBox(font.fontWkarea, "sizeTB", args, n);

    XmStringFree(string);

    n=0;
    XtSetArg (args[n], XmNselectionPolicy, XmBROWSE_SELECT); n++;
    XtSetArg (args[n], XmNautomaticSelection, True); n++;
    XtSetArg (args[n], XmNvisibleItemCount, 7); n++;
    XtSetArg (args[n], XmNitemCount, style.xrdb.numFonts); n++;
    XtSetArg (args[n], XmNitems, sizeItems); n++;
    selectedSize = &(style.xrdb.fontChoice[font.selectedFontIndex].pointSize);
    XtSetArg (args[n], XmNselectedItems, selectedSize); n++; 
    XtSetArg (args[n], XmNselectedItemCount, 1); n++; 
    font.sizeList = XmCreateScrolledList(font.sizeTB,"sizeList",args,n);
    XtAddCallback(font.sizeList, XmNbrowseSelectionCallback,
                                         changeSampleFontCB, NULL);

    XmListSetItem(font.sizeList,
        style.xrdb.fontChoice[font.selectedFontIndex].pointSize);

    XtFree((char *)sizeItems);


    /* Create the Attributes button */
    n = 0;
    XtSetArg(args[n], XmNmarginHeight, LB_MARGIN_HEIGHT);  n++;
    XtSetArg(args[n], XmNmarginWidth, LB_MARGIN_WIDTH);  n++;
    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.sizeTB);  n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_WIDGET); n++;
    XtSetArg(args[n], XmNleftWidget,         font.groupTB); n++;
    XtSetArg(args[n], XmNbottomOffset,       style.verticalSpacing);    n++;


    string = CMPSTR(Attributes);
    XtSetArg(args[n], XmNlabelString, string); n++;
    XtSetArg(args[n], XmNnavigationType, XmTAB_GROUP); n++;
    font.attributes = XmCreatePushButtonGadget(font.fontWkarea, "attributes", args, n);

    XtAddCallback(font.attributes, XmNactivateCallback, create_attr_dlg,
                               (XtPointer) style.fontDialog);

    XmStringFree(string);

    n = 0;

    XtSetArg(args[n], XmNtopAttachment,      XmATTACH_WIDGET);     n++;
    XtSetArg(args[n], XmNtopWidget,          font.attributes);  n++;
    XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
    XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
    XtSetArg(args[n], XmNbottomAttachment,   XmATTACH_FORM);       n++;
    XtSetArg(args[n], XmNbottomOffset,       style.verticalSpacing);    n++;
    XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);     n++;

    XtSetArg (args[n], XmNtraversalOn, False); n++;
    XtSetArg (args[n], XmNwidth, PREVIEW_WIN_WIDTH + 90); n++;
    XtSetArg (args[n], XmNheight, PREVIEW_WIN_HEIGHT + 50); n++;
    XtSetArg (args[n], XmNbackground,1); n++;
    XtSetArg (args[n], XmNforeground,0); n++;

    font.prevDrawWindow = XmCreateDrawingArea (font.fontWkarea,
                                                 "prevDrawWindow", args, n);

    XCreateFontSet (style.display, xlfd_name, &tmp_charset,
                        &charset_count, &def_str );
 
       actual_count = charset_count;
       for(i = 0; i < charset_count - 1; ++i)
         for(ch_num = i + 1; 
			ch_num < charset_count; ++ch_num) {
           if(strcmp(tmp_charset[i], tmp_charset[ch_num]) == 0) {
                strcpy(tmp_charset[ch_num], "NULL");
		--actual_count;  /* reduce count by removing dups */
         }
 
       }


     n = 0;
     charset_list = (char **) malloc (actual_count * sizeof (char *));

     for(i=0; i < charset_count; ++i) {
        if(strcmp(tmp_charset[i], "NULL") == 0)
		continue;
	else
	    charset_list[n++] = tmp_charset[i];
     }

     charset_count = actual_count;

     /* if charset list returned is null, disable the functionality
      of add and delete buttons. */

     if(!charset_count) {
       XtSetSensitive(font.deleteButton, False);
       XtSetSensitive(font.addButton, False);
     }
 

#ifdef DEBUG
        for(i=0; i < charset_count; ++i)
         fprintf(stderr, "Charset set %d. %s \n", i, charset_list[i]);
#endif


    XtManageChild(font.attributes);
    XtManageChild(font.prevDrawWindow);
    XtManageChild(font.sizeList);
    XtManageChild(font.groupList);
    XtManageChildren(widget_list,count);
    XtManageChild(font.fontWkarea);

}

/*+++++++++++++++++++++++++++++++++++++++*/
/* _DtmapCB_fontBB                          */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
_DtmapCB_fontBB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
_DtmapCB_fontBB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
Position x, y;
   
    DtWsmRemoveWorkspaceFunctions(style.display, XtWindow(XtParent(w)));

    if (!save.restoreFlag)
        putDialog ((Widget)client_data, w);

    first_time = 1;
    XtRemoveCallback(style.fontDialog, XmNmapCallback, _DtmapCB_fontBB, NULL);
}

    
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
/* ButtonCB                                        */
/* Process callback from PushButtons in DialogBox. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
ButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  DtDialogBoxCallbackStruct *cb     
           = (DtDialogBoxCallbackStruct *) call_data;
  int      n, len;
  char	   *str, *fntstr, *fntsetstr;
  Arg      args[MAX_ARGS];
  static char fontres[2048];
  XmStringTable    selectedSize;
  XmString	string;
  char     buf[MAXNAMLEN];
  FgroupRec *fgroup, *orig_group, *sel_group;

    switch (cb->button_position)
    {
        /* Set the xrdb or pass to dtsession and close the window */
      case OK_BUTTON:

         /*   Post an info dialog explaining when the new fonts will appear */
         if ((font.selectedFontIndex != font.originalFontIndex) ||
		(strcmp(font.selectedGroupStr, font.originalGroupStr) != 0))
         {
            XtUnmanageChild(style.fontDialog);  
            XtUnmanageChild(prev_pane);

            if(style.xrdb.writeXrdbImmediate) 
            { 
              InfoDialog(IMMEDIATE, style.shell, False); 
            }
            else 
            {
              InfoDialog(LATER, style.shell, False); 
            }

	    /* 
	       for *FontSet resource: find first font entry delimited by a ":" 
	       or an "=". 
	    */ 
            len = strcspn(style.xrdb.fontChoice[font.selectedFontIndex].userStr,":=");
            fntsetstr = (char *) XtCalloc(1, len + 1);
            memcpy(fntsetstr, style.xrdb.fontChoice[font.selectedFontIndex].userStr,len);

            /* 
	       Since the *Font and *FontSet resources may be used by old
	       X applications, a fontlist of multiple fonts must be converted 
               to Xt font set format (';'s converted to ','s since many old X 
               apps don't understand ';' syntax.)
	    */
            str = strstr(fntsetstr,";");
            while (str) {
   		*str = ',';
 		str = strstr(str,";");
	    }

	    /* 
	       for *Font resource: find first font entry delimited by a comma, 
               a colon or an = 
	    */
            len = strcspn(fntsetstr,",:=");
            fntstr = (char *) XtCalloc(1, len + 1);
            memcpy(fntstr, style.xrdb.fontChoice[font.selectedFontIndex].userStr,len);

            /* create the font resource specs with the selected font for xrdb */
            sprintf(fontres,
		 "*systemFont: %s\n*userFont: %s\n*FontList: %s\n*buttonFontList: %s\n*labelFontList: %s\n*textFontList: %s\n*XmText*FontList: %s\n*XmTextField*FontList: %s\n*DtEditor*textFontList: %s\n*Font: %s\n*FontSet: %s\n*fontGroup: %s\n",
                 style.xrdb.fontChoice[font.selectedFontIndex].sysStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].userStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].sysStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].sysStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].sysStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].userStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].userStr,
                 style.xrdb.fontChoice[font.selectedFontIndex].userStr,
	         style.xrdb.fontChoice[font.selectedFontIndex].userStr,
                 fntstr, fntsetstr,
	         font.selectedGroupStr);

	    XtFree(fntstr);
	    XtFree(fntsetstr);

         /*if writeXrdbImmediate true write to Xrdb else send to session mgr */
    	    if(style.xrdb.writeXrdbImmediate)
	        _DtAddToResource(style.display,fontres);

            SmNewFontSettings(fontres);

 	    if((orig_group = find_group_selected(font.originalGroupStr, 
							font_groups)) == NULL)
        	fprintf(stderr, "ButtonCB(): Font group %s not found ... \n", 
							font.originalGroupStr);

            font.originalFontIndex = font.selectedFontIndex;
            font.originalGroupStr  = font.selectedGroupStr;
            style.xrdb.systemFontStr = font.selectedFontStr;

 	    if((fgroup = find_group_selected(font.selectedGroupStr, 
						font_groups)) == NULL)
        	fprintf(stderr, "ButtonCB(): Font group %s not found ... \n", 
							font.selectedGroupStr);
	    else { 
		if(orig_group == NULL)
		  orig_group = fgroup;

                sprintf(buf,"%s %s %s %s %s %s", XSET,"-fp",
                               orig_group->location,
                               "+fp", 
			       fgroup->location,
			       "> /dev/null 2>/dev/null");
                system(buf);
	        prepend_iso8859();
#ifdef DEBUG
   fprintf(stderr,"ButtonCB(): %s\n", buf);
#endif
	    }
         }
	 else {
            XtUnmanageChild(style.fontDialog);  
            XtUnmanageChild(prev_pane);
         }
         break;

    case CANCEL_BUTTON:

      /* reset preview area fonts to original and close the window*/

      XtUnmanageChild(style.fontDialog);
      XtUnmanageChild(prev_pane);

      XmListSelectPos(font.sizeList, font.originalFontIndex+1, False);


      string = CMPSTR(font.originalGroupStr);
      /*
       * if the group doesn't exist, set it to default.
       */
      if(!XmListItemExists(font.groupList, string)) {
         XmStringFree(string);
	 string = CMPSTR(Default);
      }

      XmListSelectItem(font.groupList, string, False);

      set_font_preview();
      set_attribute_fields();

      XmStringFree(string);

      break;

    case HELP_BUTTON:
      XtCallCallbacks(style.fontDialog, XmNhelpCallback, (XtPointer)NULL);
      break;

    default:
      break;
    }
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* changSampleGroupCB                    */
/*  Change the font group in the sample areas  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
changeSampleGroupCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
changeSampleGroupCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
XmListCallbackStruct *cb = (XmListCallbackStruct *) call_data;
char *current_choice;
char buf[MAXNAMLEN];
static char *dir[] = {""};
FgroupRec *cur_group, *prev_group, *orig_group;
int 	  n = 0;
Arg       args[MAX_ARGS];
char	  *fgrp;
int 	  selected_pos;
int       *position_list, position_count;

    _DtTurnOnHourGlass(style.fontDialog);

    orig_group = gen_font_info_list(font.originalGroupStr);

    XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG , &current_choice);

/*  
 * remove the previous group and append current selection to the
 * font path.
 */

  if(strcmp(current_choice, "Default") != 0)   {
    if((cur_group = gen_font_info_list(current_choice)) == NULL)
       fprintf(stderr,"changeSampleGroupCB: Font group %s  not found \n",
                                                        current_choice);
    if(orig_group) {
       sprintf(buf,"%s %s %s %s %s %s", XSET,"-fp",
                               orig_group->location,
                               "+fp",
                               cur_group->location,
                               "> /dev/null 2>/dev/null");
 
#ifdef DEBUG
   fprintf(stderr,"changeSampleGroupCB: %s\n", buf);
#endif
 
       system(buf);
    }
 
    set_font_preview();

    if(orig_group) {
      sprintf(buf,"%s %s %s %s %s %s", XSET,"-fp",
                               cur_group->location,
                                "+fp", orig_group->location,
                               "> /dev/null 2>/dev/null");
      system(buf);
    }
#ifdef DEBUG
   fprintf(stderr,"changeSampleGroupCB(): %s\n", buf);
#endif


 } else {
    fgrp = FindFontGroup();

    if(fgrp) {
     XGrabServer (style.display);
     rem_fgrp_from_path(fgrp);
    }

    set_font_preview();

    if(fgrp) {
      set_fgrp_to_path(fgrp);
      XUngrabServer (style.display);
    }

 }
   font.selectedGroupStr = current_choice;
   select_def_items();
   set_attribute_fields();

   _DtTurnOffHourGlass(style.fontDialog);
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* changSampleFontCB                     */
/*  Change the font in the sample areas  */
/*+++++++++++++++++++++++++++++++++++++++*/
static void 
#ifdef _NO_PROTO
changeSampleFontCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
changeSampleFontCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    int       n;
    int       pos;
    int       hourGlassOn;
    Arg       args[MAX_ARGS];
    XmListCallbackStruct *cb = (XmListCallbackStruct *) call_data;
    char      *choice;
    XmString  *selectedSize;
    char      *size;
    char      *fgrp;

    pos = cb->item_position-1;

    font.selectedFontIndex = pos;
    font.selectedFontStr = style.xrdb.fontChoice[pos].sysStr;

if(strcmp(font.selectedGroupStr, "Default") != 0) {

    hourGlassOn = !style.xrdb.fontChoice[pos].userFont ||
                !style.xrdb.fontChoice[pos].sysFont;
 
    if (hourGlassOn)
      _DtTurnOnHourGlass(style.fontDialog);

    if (!style.xrdb.fontChoice[pos].userFont)
      GetUserFontResource(pos);
    if (!style.xrdb.fontChoice[pos].sysFont)
      GetSysFontResource(pos);

    set_font_preview();
    set_attribute_fields();
 
    if (hourGlassOn)
      _DtTurnOffHourGlass(style.fontDialog);

 } else {
    hourGlassOn = !style.xrdb.fontChoice[pos].userFont ||
                !style.xrdb.fontChoice[pos].sysFont;
 
    if (hourGlassOn)
      _DtTurnOnHourGlass(style.fontDialog);
 
    if (!style.xrdb.fontChoice[pos].userFont)
      GetUserFontResource(pos);
    if (!style.xrdb.fontChoice[pos].sysFont)
      GetSysFontResource(pos);
 
 
    fgrp = FindFontGroup();
 
    if(fgrp) {
     XGrabServer (style.display);
     rem_fgrp_from_path(fgrp);
    }
 
    n = 0;
    XtSetArg(args[n], XmNfontList, style.xrdb.fontChoice[pos].sysFont); n++;
    XtSetValues (prev_label, args, n);
 
    n = 0;
    XtSetArg(args[n], XmNfontList, style.xrdb.fontChoice[pos].userFont) ; n++;
    XtSetValues (prev_text, args, n);
    XmTextShowPosition(prev_text, 0);

    if(!style.xrdb.fontChoice[pos].userBoldFont)
      GetUserBoldFontResource(pos);
 
    n = 0;
    XtSetArg(args[n], XmNfontList, style.xrdb.fontChoice[pos].userBoldFont); n++;
    XtSetValues (prev_push_button, args, n);
 
   if(fgrp) {
    set_fgrp_to_path(fgrp);
    XUngrabServer (style.display);

   }
    if (hourGlassOn)
      _DtTurnOffHourGlass(style.fontDialog);
 }

}

/************************************************************************
 * restoreFonts()
 *
 * restore any state information saved with saveFonts.
 * This is called from restoreSession with the application
 * shell and the special xrm database retrieved for restore.
 ************************************************************************/
void 
#ifdef _NO_PROTO
restoreFonts( shell, db )
        Widget shell ;
        XrmDatabase db ;
#else
restoreFonts(
        Widget shell,
        XrmDatabase db )
#endif /* _NO_PROTO */
{
    XrmName xrm_name[5];
    XrmRepresentation rep_type;
    XrmValue value;

    xrm_name [0] = XrmStringToQuark ("Fonts");
    xrm_name [2] = NULL;

    /* get x position */
    xrm_name [1] = XrmStringToQuark ("x");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNx, atoi((char *)value.addr)); 
      save.poscnt++;
      save.restoreFlag = True;
    }

    /* get y position */
    xrm_name [1] = XrmStringToQuark ("y");
    if (XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value)){
      XtSetArg (save.posArgs[save.poscnt], XmNy, atoi((char *)value.addr)); 
      save.poscnt++;
    }

    xrm_name [1] = XrmStringToQuark ("ismapped");
    XrmQGetResource (db, xrm_name, xrm_name, &rep_type, &value);
    /* Are we supposed to be mapped? */
    if (strcmp(value.addr, "True") == 0) 
      popup_fontBB(shell);
}

/************************************************************************
 * saveFonts()
 *
 * This routine will write out to the passed file descriptor any state
 * information this dialog needs.  It is called from saveSessionCB with the
 * file already opened.
 * All information is saved in xrm format.  There is no restriction
 * on what can be saved.  It doesn't have to be defined or be part of any
 * widget or Xt definition.  Just name and save it here and recover it in
 * restoreFonts.  The suggested minimum is whether you are mapped, and your
 * location.
 ************************************************************************/
void 
#ifdef _NO_PROTO
saveFonts( fd )
        int fd ;
#else
saveFonts(
        int fd )
#endif /* _NO_PROTO */
{
    Position x,y;
    char *bufr = style.tmpBigStr;     /* size=[1024], make bigger if needed */
    XmVendorShellExtObject  vendorExt;
    XmWidgetExtData         extData;

    if (style.fontDialog != NULL) {
        if (XtIsManaged(style.fontDialog))
          sprintf(bufr, "*Fonts.ismapped: True\n");
        else
          sprintf(bufr, "*Fonts.ismapped: False\n");

        /* Get and write out the geometry info for our Window */

        x = XtX(XtParent(style.fontDialog));
        y = XtY(XtParent(style.fontDialog));

        /* Modify x & y to take into account window mgr frames
         * This is pretty bogus, but I don't know a better way to do it.
         */
        extData = _XmGetWidgetExtData(style.shell, XmSHELL_EXTENSION);
        vendorExt = (XmVendorShellExtObject)extData->widget;
        x -= vendorExt->vendor.xOffset;
        y -= vendorExt->vendor.yOffset;

        sprintf(bufr, "%s*Fonts.x: %d\n", bufr, x);
        sprintf(bufr, "%s*Fonts.y: %d\n", bufr, y);

        write (fd, bufr, strlen(bufr));
    }
}

static void
_DtFree(void * p)
{
    free(p);
}
 
static void *
_DtMalloc(int   size)
{
    void *      p = malloc(size);
 
    if (p == (void *) NULL) {
        fprintf(stderr, "Cannot allocate memory\n");
        exit(1);
    }
    return(p);
}
 
static void *
_DtRealloc(void *       p,
           int          size)
{
    p = realloc(p, size);
    if (p == (void *) NULL) {
        fprintf(stderr, "Cannot reallocate memory\n");
        exit(1);
    }
    return(p);
}
 
static char *
_DtNewString(char *     str)
{
    char *      p = (char *) NULL;
 
    if (str != (char *) NULL) {
        int len = strlen(str);
 
        p = (char *) _DtMalloc(len + 1);
        strcpy(p, str);
    }
    return(p);
}

FgroupRec *
#ifdef _NO_PROTO
ScanFontGroups( dir,  count )
        char      *dir;
	int       *count;
#else
ScanFontGroups(
        char *dir, int *count)
#endif /* _NO_PROTO */
{
    DIR *dirp;
    struct dirent *dp;
    DIR *group_dir;
    struct dirent *font_group_dir;
    char *name;
    char path[MAXPATHLEN];
    char fgdir[MAXPATHLEN];
    char *group_name;
    char Fgroup[MAXPATHLEN];
    FILE *fp;
    FgroupRec *fgroup;

    
    dirp = opendir(dir);
    if (dirp == NULL)
             return (font_groups);

    while ((dp = readdir(dirp)) != NULL) {
            name = dp->d_name;
            if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
                    continue;

	     sprintf(fgdir, "%s%s", dir,name);

	     if((group_dir = opendir(fgdir)) == NULL)
		    continue;
	    
             while ((font_group_dir = readdir(group_dir)) != NULL) {
              group_name = font_group_dir->d_name;

              if ((strcmp(group_name, ".") == 0) || 
                                       (strcmp(group_name, "..") == 0))
                    continue;
 
	      sprintf(path, "%s/%s/%s", dir, name, "sdtfonts.group");

              if ((fp = fopen(path, "r")) == NULL)
                    continue;
 
              if(fgets(Fgroup, MAXNAMLEN, fp)) {
		   Fgroup[strlen(Fgroup) - 1] = '\0';
		   /* 
		    * create fgroup only if it is not created.
		    */
   	       if((fgroup = find_group_selected(Fgroup, font_groups)) == NULL){
	        if(font_groups == NULL) {
                   font_groups = (FgroupRec *) _DtMalloc(sizeof(struct _FgroupRec));
                   font_groups->fgroup_name = _DtNewString(Fgroup);
                   font_groups->family_name = NULL;
                   font_groups->foundry = NULL;
                   font_groups->weight = NULL;
                   font_groups->location    = _DtNewString(fgdir);
     		   font_groups->mono_fonts = NULL;
     		   font_groups->prop_fonts = NULL;
     		   font_groups->bold_fonts = NULL;
	           cur_font_group = font_groups;
		   cur_font_group->next = NULL;
                }
	        else {
	           cur_font_group->next = 
                             (FgroupRec *)_DtMalloc(sizeof(struct _FgroupRec));
                   cur_font_group = cur_font_group->next;
                   cur_font_group->fgroup_name = _DtNewString(Fgroup);
                   cur_font_group->family_name = NULL;
                   cur_font_group->foundry = NULL;
                   cur_font_group->weight = NULL;
                   cur_font_group->location    = _DtNewString(fgdir);
     		   cur_font_group->mono_fonts = NULL;
     		   cur_font_group->prop_fonts = NULL;
     		   cur_font_group->bold_fonts = NULL;
  		   cur_font_group->next = NULL;
	        }
	       ++(*count);
	       }
              }
	      fclose(fp);
	      break; /* break after opening sdtfonts.group, no need to check other
		      * files.
		      */
             }
             (void) closedir(group_dir);
    }
    (void) closedir(dirp);

   return(font_groups);
}

static FgroupRec *
#ifdef _NO_PROTO
initDefaultFontInfo()
#else
initDefaultFontInfo(void)
#endif /* _NO_PROTO */
{
char buf_mono_str[256] = "-dt-interface user-medium";
char buf_prop_str[256] = "-dt-interface system-medium";
char buf_bold_str[256] = "-dt-interface user-bold";

char buf_str[256] = "";
char line_alias_name[256];
char     **XLFDNameFields, *freeMe;
char fonts_alias[MAXPATHLEN];
fontInfo *cur_entry = NULL, *mono_fonts = NULL;
fontInfo *cur_entry_prop = NULL, *prop_fonts = NULL;
fontInfo *cur_entry_bold = NULL, *bold_fonts = NULL;
char       **fontlist, *familyLabel;
FgroupRec  *fgroup = NULL;
int 	nfonts;
int 	i;
char    *fgrp;

    fgrp = FindFontGroup();
 
    if(fgrp) {
     XGrabServer (style.display);
     rem_fgrp_from_path(fgrp);
    }

   fontlist = XListFonts(style.display, "-dt-interface *-*-*-*-*-*-*-*-*-*-*-*-*", BIGNUM, &nfonts);


   if(fgrp) {
    set_fgrp_to_path(fgrp);
    XUngrabServer (style.display);
   }



   if(nfonts <= 0)
     return (fgroup);
   else {
       fgroup = (FgroupRec *) _DtMalloc(sizeof(struct _FgroupRec));
       fgroup->fgroup_name = _DtNewString("Default");
       fgroup->family_name = NULL; 
       fgroup->foundry = NULL; 
       fgroup->weight = NULL; 
       fgroup->location   = _DtNewString(DEFAULT_FONT_PATH);
       fgroup->mono_fonts = NULL;
       fgroup->prop_fonts = NULL;
       fgroup->bold_fonts = NULL;
       fgroup->next = NULL;
    }

   for(i = 0; i < nfonts; i++) {
    if (!splitXLFDName(fontlist[i], &XLFDNameFields, &freeMe))
	continue;

   sprintf(buf_str, "-%s-%s-%s",
     XLFDNameFields[XLFD_FOUNDRY], XLFDNameFields[XLFD_FAMILY],
     XLFDNameFields[XLFD_WEIGHT]);

   if(strcmp(buf_str, buf_mono_str) == 0) {
      if(mono_fonts == NULL) {
         mono_fonts =
         (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         mono_fonts->alias_name = _DtNewString(fontlist[i]);
         mono_fonts->font_defn =  NULL;
         mono_fonts->alias_hrn = get_hrn(XLFDNameFields);
         mono_fonts->font_hrn  = NULL;
         mono_fonts->size = set_size(XLFDNameFields);
         cur_entry = mono_fonts;
         cur_entry->next = NULL;
   }
      else {
         cur_entry->next =
         (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         cur_entry = cur_entry->next;
         cur_entry->alias_name = _DtNewString(fontlist[i]);
         cur_entry->font_defn =  NULL;
         cur_entry->alias_hrn = get_hrn(XLFDNameFields);
         cur_entry->font_hrn  = NULL;
         cur_entry->size = set_size(XLFDNameFields);
         cur_entry->next = NULL;
      }
   }

   if(strcmp(buf_str, buf_prop_str) == 0) {
      if(prop_fonts == NULL) {
         prop_fonts =
         (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         prop_fonts->alias_name = _DtNewString(fontlist[i]);
         prop_fonts->font_defn =  NULL;
         prop_fonts->alias_hrn = get_hrn(XLFDNameFields);
         prop_fonts->font_hrn  = NULL;
         prop_fonts->size = set_size(XLFDNameFields);
         cur_entry_prop = prop_fonts;
         cur_entry_prop->next = NULL;
      }
      else {
         cur_entry_prop->next =
         (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         cur_entry_prop = cur_entry_prop->next;
         cur_entry_prop->alias_name = _DtNewString(fontlist[i]);
         cur_entry_prop->font_defn =  NULL;
         cur_entry_prop->alias_hrn = get_hrn(XLFDNameFields);
         cur_entry_prop->font_hrn  = NULL;
         cur_entry_prop->size = set_size(XLFDNameFields);
         cur_entry_prop->next = NULL;
      }
   }
 
   if(strcmp(buf_str, buf_bold_str) == 0) {
      if(bold_fonts == NULL) {
         bold_fonts =
         (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         bold_fonts->alias_name = _DtNewString(fontlist[i]);
         bold_fonts->font_defn =  NULL;
         bold_fonts->alias_hrn = get_hrn(XLFDNameFields);
         bold_fonts->font_hrn  = NULL;
         bold_fonts->size = set_size(XLFDNameFields);
         cur_entry_bold = bold_fonts;
         cur_entry_bold->next = NULL;
      }
      else {
         cur_entry_bold->next =
                        (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         cur_entry_bold = cur_entry_bold->next;
         cur_entry_bold->alias_name = _DtNewString(fontlist[i]);
         cur_entry_bold->font_defn =  NULL;
         cur_entry_bold->alias_hrn = get_hrn(XLFDNameFields);
         cur_entry_bold->font_hrn  = NULL;
         cur_entry_bold->size = set_size(XLFDNameFields);
         cur_entry_bold->next = NULL;
      }
   }
 
  XtFree((char *)XLFDNameFields);
  XtFree(freeMe);
 
  } /* for */
 
  fgroup->mono_fonts = mono_fonts;
  fgroup->prop_fonts = prop_fonts;
  fgroup->bold_fonts = bold_fonts;
 
 return(fgroup);
}

static FgroupRec *
#ifdef _NO_PROTO
gen_font_info_list( group_name)
	char	     *group_name;

#else
gen_font_info_list(char *group_name)
#endif /* _NO_PROTO */
{
char **XLFD_fields;
char line_alias_name[256];
char line_font_name[256];
char buf_mono_str[256] = "-dt-interface user-medium";
char buf_prop_str[256] = "-dt-interface system-medium";
char buf_bold_str[256] = "-dt-interface user-bold";
char buf_str[256] = ""; 
char temp_str[MAXNAMLEN];
FILE *fp;
char     **XLFDNameFields, *freeMe1, *freeMe2;
char     **XLFDFontFields;
int i = 0;
int j = 0;
char fonts_alias[MAXPATHLEN];
fontInfo *cur_entry = NULL, *mono_fonts = NULL;
fontInfo *cur_entry_prop = NULL, *prop_fonts = NULL;
fontInfo *cur_entry_bold = NULL, *bold_fonts = NULL;
FgroupRec    *fgroup;

fontInfo	*mono_font;


 if((fgroup = find_group_selected(group_name, font_groups)) == NULL) {
#ifdef DEBUG
	fprintf(stderr,"gen_font_info_list(): Font group %s not found ... \n", group_name);
#endif
	return (fgroup);
 }
/* 
 * if the fonts list is already created, return group element 
 */

 if(fgroup->mono_fonts || fgroup->prop_fonts || fgroup->bold_fonts)
	return (fgroup);

 sprintf(fonts_alias,"%s%s/%s", DEFAULT_FONT_PATH, 
			group_name, "fonts.alias");

 if((fp = fopen(fonts_alias, "r")) == NULL){
        sprintf(fonts_alias, "%s/%s", fgroup->location,"fonts.alias");

        if((fp = fopen(fonts_alias, "r")) == NULL){
	   fprintf(stderr,"gen_font_info_list(): Unable to open %s \n",fonts_alias);
	   return (fgroup);
        }
 }
 
 if(fp != NULL)
   while(fgets(temp_str, 512, fp)) { 
	split_line(temp_str, line_alias_name, line_font_name);
    if (splitXLFDName(line_alias_name, &XLFDNameFields, &freeMe1))
      if (splitXLFDName(line_font_name, &XLFDFontFields, &freeMe2))
	sprintf(buf_str, "-%s-%s-%s",
		XLFDNameFields[XLFD_FOUNDRY], XLFDNameFields[XLFD_FAMILY],
                XLFDNameFields[XLFD_WEIGHT]);

	if(strcmp(buf_str, buf_mono_str) == 0) {
	   if(mono_fonts == NULL) {
             mono_fonts = 
			(fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
	     mono_fonts->alias_name = _DtNewString(line_alias_name);
	     mono_fonts->font_defn =  _DtNewString(line_font_name);
	     mono_fonts->alias_hrn = get_hrn(XLFDNameFields);
	     mono_fonts->font_hrn  = get_hrn(XLFDFontFields);
 	     mono_fonts->size = set_size(XLFDNameFields);
	     cur_entry = mono_fonts;
	     cur_entry->next = NULL;
	   }
           else {
  	     cur_entry->next = 
			(fontInfo *) _DtMalloc(sizeof(struct _fontInfo)); 
	     cur_entry = cur_entry->next;
             cur_entry->alias_name = _DtNewString(line_alias_name);
             cur_entry->font_defn =  _DtNewString(line_font_name);
	     cur_entry->alias_hrn = get_hrn(XLFDNameFields);
	     cur_entry->font_hrn  = get_hrn(XLFDFontFields);
 	     cur_entry->size = set_size(XLFDNameFields);
	     cur_entry->next = NULL; 
	   }
	}

	if(strcmp(buf_str, buf_prop_str) == 0) {
           if(prop_fonts == NULL) {
             prop_fonts =
                        (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
             prop_fonts->alias_name = _DtNewString(line_alias_name);
             prop_fonts->font_defn =  _DtNewString(line_font_name);
	     prop_fonts->alias_hrn = get_hrn(XLFDNameFields);
	     prop_fonts->font_hrn  = get_hrn(XLFDFontFields);
 	     prop_fonts->size = set_size(XLFDNameFields);
             cur_entry_prop = prop_fonts;
	     cur_entry_prop->next = NULL;
           }
           else {
             cur_entry_prop->next =
                        (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
             cur_entry_prop = cur_entry_prop->next;
             cur_entry_prop->alias_name = _DtNewString(line_alias_name);
             cur_entry_prop->font_defn =  _DtNewString(line_font_name);
	     cur_entry_prop->alias_hrn = get_hrn(XLFDNameFields);
	     cur_entry_prop->font_hrn  = get_hrn(XLFDFontFields);
 	     cur_entry_prop->size = set_size(XLFDNameFields);
	     cur_entry_prop->next = NULL;
           }
	}

	if(strcmp(buf_str, buf_bold_str) == 0) {
           if(bold_fonts == NULL) {
             bold_fonts =
                        (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
             bold_fonts->alias_name = _DtNewString(line_alias_name);
             bold_fonts->font_defn =  _DtNewString(line_font_name);
             bold_fonts->alias_hrn = get_hrn(XLFDNameFields);
             bold_fonts->font_hrn  = get_hrn(XLFDFontFields);
 	     bold_fonts->size = set_size(XLFDNameFields);
             cur_entry_bold = bold_fonts;
             cur_entry_bold->next = NULL;
           }
           else {
             cur_entry_bold->next =
                        (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
             cur_entry_bold = cur_entry_bold->next;
             cur_entry_bold->alias_name = _DtNewString(line_alias_name);
             cur_entry_bold->font_defn =  _DtNewString(line_font_name);
             cur_entry_bold->alias_hrn = get_hrn(XLFDNameFields);
             cur_entry_bold->font_hrn  = get_hrn(XLFDFontFields);
 	     cur_entry_bold->size = set_size(XLFDNameFields);
             cur_entry_bold->next = NULL;
           }
        }
   }  /* while */
   fclose(fp);

     fgroup->mono_fonts = mono_fonts;
     fgroup->prop_fonts = prop_fonts;
     fgroup->bold_fonts = bold_fonts;

  return (fgroup);
}

static Boolean
#ifdef _NO_PROTO
splitXLFDName(XLFDName, XLFDNameFields, freeMe)
    char   *XLFDName;
    char ***XLFDNameFields;
    char  **freeMe;
#else
splitXLFDName( char   *XLFDName, char ***XLFDNameFields, char  **freeMe)
#endif /* _NO_PROTO */
{
    int     i;
    char   *ch = XtNewString(XLFDName);
 
    *freeMe         = ch;

    *XLFDNameFields = (char **) XtCalloc(XLFD_NUM_FIELDS, (unsigned)sizeof(char
**));
 
    if (*ch != '-') {
        /* XLFD name is malformed */
        XtFree((char *)*XLFDNameFields);
        XtFree(*freeMe);
        return(FALSE);
    }
 
    for (i=0; i < XLFD_NUM_FIELDS; i++) {
        (*XLFDNameFields)[i] = ++ch;
        ch = strchr(ch, '-');
        if (ch == NULL) {
            break;
        } else {
            *ch = '\0';
        }
    }
 
    if ((ch != NULL) || (++i != XLFD_NUM_FIELDS)) {
        /* XLFD name is malformed */
        XtFree((char *)*XLFDNameFields);
        XtFree(*freeMe);
        return(FALSE);
    }
 
    return (TRUE);
}

static void
#ifdef _NO_PROTO
split_line(line, alias, font)
        char *line;
        char *alias;
	char *font;
#else
split_line(char *line, char *alias, char *font)
#endif /* _NO_PROTO */
{
        int len;
        char tmp_alias[256], tmp_font[256];
	char *str_ptr;
 
 
        if (line[0] == '"') {
                get_first_token_from_str(alias, &line[1], "\"");
                /*
                 *    strip space and
                 *    start two positions after len of alias to account
                 *    for the double quotes
                 */
                len = strlen(alias) + 3;
                get_first_token_from_str(font, &line[len], "\"");
        }
	else {
                get_first_token_from_str(alias, &line[1], " ");
                len = strlen(tmp_alias) + 2;
                get_first_token_from_str(font, &line[len], "\n");
 
        }
}

static char * 
#ifdef _NO_PROTO
set_size(xlfd_field)
        char **xlfd_field;
#else
set_size(char **xlfd_field)
#endif /* _NO_PROTO */
{
char buf[100];
char *ch;

    ch = strtok(xlfd_field[XLFD_ADD_STYLE], " ");
    strcpy(buf, ch);
    return strdup (buf);
}

static char * 
#ifdef _NO_PROTO
get_hrn(xlfd_field)
        char **xlfd_field; 
#else
get_hrn(char **xlfd_field)
#endif /* _NO_PROTO */
{
    char *field;
    char buf[2000];
    char *str;
    char *ptr;
    char type[100], slant[100];
    char family[100], set_width[100], add_style[100], weight[100],
         set_reg[100], set_enc[100], charset[200], hrn_charset[200];
 
    buf[0] = '\0';
    type[0] = '\0';
    slant[0] = '\0';
    family[0] = '\0';
    set_width[0] = '\0';
    add_style[0] = '\0';
    weight[0] = '\0';
    set_reg[0] = '\0';
    set_enc[0] = '\0';
    charset[0] = '\0';
    hrn_charset[0] = '\0';
 
    if (strcasecmp(xlfd_field[XLFD_SLANT], "i") == 0)
        strcpy (slant, " Italic");
    else if (strcasecmp(xlfd_field[XLFD_SLANT], "r") == 0)
        slant[0] = '\0';
    else if (strcasecmp(xlfd_field[XLFD_SLANT], "o") == 0)
        strcpy (slant, " Oblique");
    else if (strcasecmp (xlfd_field[XLFD_SLANT], "ro") == 0)
        strcpy (slant, " Reverse Oblique");
    else if (strcasecmp (xlfd_field[XLFD_SLANT], "ri") == 0)
        strcpy (slant, " Reverse Italic");
    else
    {
        str = slant + sizeof (char);
        strcpy (str, xlfd_field[XLFD_SLANT]);
        slant[0] = ' ';
    }
 
    strcpy (family, xlfd_field[XLFD_FAMILY]);
 
    if (!strcasecmp (xlfd_field[XLFD_SETWIDTH], "normal"))
	set_width[0] = '\0';
    else
    {
        str = set_width + sizeof (char);
        strcpy (str, xlfd_field[XLFD_SETWIDTH]);
        set_width[0] = ' ';
    }
 
    if (xlfd_field[XLFD_ADD_STYLE][0] == '\0' || xlfd_field[XLFD_ADD_STYLE][0] == ' ')
    {
        add_style[0] = '\0';
    }
    else
    {
        str = add_style + sizeof (char);
        if(strstr(xlfd_field[XLFD_ADD_STYLE], " ") != NULL) {
	   strtok(xlfd_field[XLFD_ADD_STYLE], " ");
	   strcpy(str, strtok(NULL, " ")); 
	}
	else
	   strcpy(str, xlfd_field[XLFD_ADD_STYLE]);
        add_style[0] = ' ';
    }

    if (!strcasecmp (xlfd_field[XLFD_WEIGHT], "medium"))
	weight[0] = '\0';
    else
    {
        str = weight + sizeof (char);
        strcpy (str, xlfd_field[XLFD_WEIGHT]);
        weight[0] = ' ';
    }
 
    family[0] = (char) toupper ((int)family[0]);
    if ((ptr = strstr(family, " ")) != NULL){
        while ( ptr != NULL && *ptr != '\0'){
        if ( *ptr == ' ' )
            *(ptr+1) = (char) toupper ((int)(*(ptr+1)));
            ptr++;
        }
    }

    if (set_width[0] != '\0')
      set_width[1] = (char) toupper ((int)set_width[1]);
    if (add_style[0] != '\0')
      add_style[1] = (char) toupper ((int)add_style[1]);
    weight[1] = (char) toupper ((int)weight[1]);
 
    if (family[0] != '\0');
        strcpy (buf, family);
    if (weight[0] != '\0');
        strcat (buf, weight);
    if (set_width[0] != '\0');
        strcat (buf, set_width);
    if (add_style[0] != '\0');
        strcat (buf, add_style);
    if (slant[0] != '\0');
        strcat (buf, slant);

    if (strcmp (xlfd_field[XLFD_FOUNDRY], "") != 0) {
	strcat (buf, " (");
	strcat (buf, xlfd_field[XLFD_FOUNDRY]);
	strcat (buf, ") ");
    }

    str = (char *) _DtMalloc(strlen(buf) + 1);
    strcpy(str,buf);
    return (str);
}

static void
#ifdef _NO_PROTO
set_read_only_str(buf)
        char *buf;
#else
set_read_only_str(char *buf)
#endif /* _NO_PROTO */
{
    int len;
    static int align_read_only_len = 46;
 
    len = strlen (buf);
    if (len < align_read_only_len)
    {
        int i;
 
        for ( i = len; i < align_read_only_len; i++ )
        {
            buf[i] = ' ';
        }
        sprintf ( buf+align_read_only_len, READ_ONLY_STR );
    }
    else
    {
        buf[len] = ' ';
        len++;
        buf[len] = ' ';
        len++;
        buf[len] = ' ';
        len++;
        sprintf ( buf+len, READ_ONLY_STR );
    }
}

static void
#ifdef _NO_PROTO
get_first_token_from_str(dest, src, separators)
	char *dest;
	char *src;
	char *separators;
#else
get_first_token_from_str(char *dest, char *src, char *separators)
#endif /* _NO_PROTO */
{
        char *ch_ptr;
 
        if (ch_ptr = strtok(src, separators)) {
                strcpy(dest, ch_ptr);
        }
}               



static char **
#ifdef _NO_PROTO
FgroupList(count_groups)
        int          count_groups;
#else
FgroupList( int count_groups)
#endif /* _NO_PROTO */
{
char **groupItems;
 int      i = 0;
 FgroupRec *temp = font_groups;

   groupItems = (char **) XtMalloc(sizeof(char *) * count_groups);

   while(temp != NULL) {
     groupItems[i++] = temp->fgroup_name;
     temp = temp->next;
   }

return (groupItems);
}
 
static Widget
#ifdef _NO_PROTO
CreateSpacingOption( parent)
        Widget parent;
#else
CreateSpacingOption(
        Widget parent)
#endif /* _NO_PROTO */
{

  Widget spacing_option, alias_family_label, ch_label, charset_option;
  Arg    args[24];
  register int n = 0;
  XmString valueXmstrings[3];
  XmString *charset_strings;
  int numStrings;
  XmString  string;
 
 
  valueXmstrings[0] = CMPSTR(PROPORTIONAL);
  valueXmstrings[1] = CMPSTR(MONO);
  valueXmstrings[2] = CMPSTR(BOLD);
 
  n = 0;
  XtSetArg(args[n], DtNorientation, DtLEFT); n++;
  XtSetArg(args[n], DtNitems, valueXmstrings); n++;
  XtSetArg(args[n], DtNitemCount, 3); n++;
  XtSetArg(args[n], DtNvisibleItemCount, 3); n++;
  XtSetArg(args[n], XmNtopAttachment,    XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition,      2); n++;
  XtSetArg(args[n], XmNtopOffset,        -5); n++;
  XtSetArg(args[n], XmNleftAttachment,   XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNleftPosition,     2); n++;
  spacing_option = DtCreateComboBox(parent, "Spacing_Option", args, n);
  XtManageChild(spacing_option);
 
  string = CMPSTR(PROPORTIONAL);
  XtVaSetValues(spacing_option, DtNselectedItem, string, NULL);
  XmStringFree(string);

  XtAddCallback(spacing_option, DtNselectionCallback, 
		(XtCallbackProc)OptionChanged, NULL);
 
 
  n = 0;
  string = CMPSTR(SPACING);
  XtSetArg(args[n], XmNlabelString,      string); n++;
  XtSetArg(args[n], XmNtopAttachment,    XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition,      2); n++;
  XtSetArg(args[n], XmNtopOffset,        0); n++;
  XtSetArg(args[n], XmNrightAttachment,  XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNrightWidget,      spacing_option); n++;
  XtSetArg(args[n], XmNeditable,         False); n++;
  alias_family_label  = XmCreateLabel(parent, "alias_family_label", args, n);

  XmStringFree(string);

  numStrings = charset_count;

  charset_strings = (XmString *)XtMalloc(numStrings * sizeof(XmString*));
  for (n = 0; n < charset_count; n++) {
        charset_strings[n] = XmStringCreate(charset_list[n], XmFONTLIST_DEFAULT_TAG);
  }

  n = 0;
  XtSetArg(args[n], DtNorientation,      DtLEFT); n++;
  XtSetArg(args[n], DtNitems,            charset_strings); n++;
  XtSetArg(args[n], DtNitemCount,        numStrings); n++;
  XtSetArg(args[n], DtNvisibleItemCount, numStrings); n++;
  XtSetArg(args[n], XmNtopAttachment,    XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition,      2); n++;
  XtSetArg(args[n], XmNtopOffset,        -5); n++;
  XtSetArg(args[n], DtNselectedItem, charset_strings[0]); n++;

  XtSetArg(args[n], XmNleftAttachment,   XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNleftPosition,     6); n++;

  charset_option = DtCreateComboBox(parent, "Charset_Option", args, n);
  XtManageChild(charset_option);
 
  XtAddCallback(charset_option, DtNselectionCallback,
                (XtCallbackProc)CharsetChanged, NULL);

  n = 0;
  string = CMPSTR(CHSET);
  XtSetArg(args[n], XmNlabelString,      string); n++;
  XtSetArg(args[n], XmNtopAttachment,    XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition,      2); n++;
  XtSetArg(args[n], XmNtopOffset,        0); n++;
  XtSetArg(args[n], XmNrightAttachment,   XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNrightWidget,      charset_option); n++; 
  XtSetArg(args[n], XmNeditable,         False); n++;
  ch_label  = XmCreateLabel(parent, "ch_label", args, n);

  XmStringFree(string);

  for (n = 0; n < 3; n++)
        XmStringFree(valueXmstrings[n]);

  for (n = 0; n < charset_count; n++)
        XmStringFree(charset_strings[n]);

  XtFree((char*)charset_strings);


  XtManageChild(ch_label);
  XtManageChild(alias_family_label);
}
 
static void
#ifdef _NO_PROTO
OptionChanged (w, which, state )
        Widget    w;
        int       which;
        DtComboBoxCallbackStruct *state;
#else
OptionChanged ( Widget    w, int which, DtComboBoxCallbackStruct *state)
#endif /* _NO_PROTO */
{

        switch(state->item_position) {
        case 0:
                strcpy(option_selected, (char *) "prop");
                break;
        case 1:
                strcpy(option_selected, (char *) "mono");
                break;
        case 2:
                strcpy(option_selected, (char *) "bold");
                break;
        default:
                break;
        }
        set_attribute_fields();
}

static void
#ifdef _NO_PROTO
CharsetChanged (w, which, state )
        Widget    w;
        int       which;
        DtComboBoxCallbackStruct *state;
#else
CharsetChanged ( Widget    w, int which, DtComboBoxCallbackStruct *state)
#endif /* _NO_PROTO */
{
    charset_selected = state->item_position;

#ifdef DEBUG
 fprintf(stderr, "CharsetChanged() -  charset_selected %d\n", charset_selected);
#endif 
    set_attribute_fields();
}



static void
#ifdef _NO_PROTO
create_attr_dlg( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
create_attr_dlg(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */

{
  Widget           parent = (Widget) client_data;
  Arg              args[24];
  XmString         string;
  Widget           children[16], form, longest_label;
  register         n;
  XmString         button_string[NUM_LABELS];
  Dimension        width1, width2, width3, width4, width5, 
                   width6, width7, longest;
  FgroupRec        *group_rec;
  

if(attr_dlg == NULL) {
  n = 0;
  button_string[0] = CMPSTR(_DtCloseString);
  button_string[1] = CMPSTR(_DtHelpString);

  XtSetArg (args[n], XmNbuttonCount, NUM_LABELS - 1);  n++;
  XtSetArg (args[n], XmNbuttonLabelStrings, button_string);  n++;

  XtSetArg (args[n], XmNchildType, XmWORK_AREA);  n++;
  XtSetArg (args[n], XmNdefaultPosition, False);  n++;

  attr_dlg = __DtCreateDialogBoxDialog(parent,
                             "attr_dlg", args, n);

  XtAddCallback(attr_dlg, XmNcallback, attr_ButtonCB, NULL);
  XtAddCallback(attr_dlg, XmNhelpCallback,
            (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_FONT_ATTRIB_DIALOG);

  XmStringFree(button_string[0]);
  XmStringFree(button_string[1]);

  n = 0;
  XtSetArg (args[n], XmNtitle, FONT_GRP_ATTRIBUTES); n++;
  XtSetArg (args[n], XmNmwmFunctions, DIALOG_MWM_FUNC); n++;
  XtSetValues (XtParent(attr_dlg), args, n);


  children[0] = _DtDialogBoxGetButton(attr_dlg,2);
  n=0;
  XtSetArg(args[n], XmNautoUnmanage, False); n++;
  XtSetArg(args[n], XmNcancelButton, children[0]); n++;
  XtSetValues (attr_dlg, args, n);
  

  n = 0;
  XtSetArg(args[n], XmNautoUnmanage, False); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightOffset, 10); n++;
  XtSetArg(args[n], XmNfractionBase, 10); n++;
  XtSetArg(args[n], XmNresizePolicy, XmRESIZE_NONE); n++;
  form = XmCreateForm (attr_dlg, "form", args, n);
  
  /* *************** Font Group : XmLabel *************** */
    string = CMPSTR(FONT_GROUP);
        string = XmStringConcat(string, XmStringCreate(":",
                                       XmFONTLIST_DEFAULT_TAG));
  n = 0;
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 0); n++;
  XtSetArg(args[n], XmNtopOffset,  10); n++;
  font_group =
    XmCreateLabel(form, "font_group", args, n);
  
  XmStringFree(string);

  /* *************** size value : XmLabel *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 0); n++;
  XtSetArg(args[n], XmNtopOffset,  10); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  font_size_val =
    XmCreateLabel(form, "font_size_val", args, n);
  
  /* *************** size : XmLabel *************** */
    string = CMPSTR(SIZE);
        string = XmStringConcat(string, XmStringCreate(":",
                                       XmFONTLIST_DEFAULT_TAG));
  n = 0;
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 0); n++;
  XtSetArg(args[n], XmNrightOffset, 10); n++;
  XtSetArg(args[n], XmNtopOffset,  10); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNrightWidget,     font_size_val); n++;
  font_size =
    XmCreateLabel(form, "font_size", args, n);
  
  XmStringFree(string);
  
  /* *************** separator1 : XmSeparator *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 1); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  separator1 =
    XmCreateSeparator(form, "separator1", args, n);

  /* *************** option menu : XmOptionMenu *************** */
     CreateSpacingOption(form);
  
  /* *************** separator2 : XmSeparator *************** */
  n = 0; 
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 3); n++;
  XtSetArg(args[n], XmNtopOffset, 5); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  separator2 =
    XmCreateSeparator(form, "separator2", args, n);

  /* *************** alias_label : XmLabel *************** */
  n = 0;
    string = CMPSTR(ALIAS);
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 4); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++; 
  alias_label =
    XmCreateLabel(form, "alias_label", args, n);

    XmStringFree(string);
  
  /* *************** alias_xlfd_label : XmLabel *************** */
  n = 0;
    string = CMPSTR(ALIAS_XLFD);
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 5); n++; 
  XtSetArg(args[n], XmNtopOffset, 10); n++; 
  alias_xlfd_label =
    XmCreateLabel (form, "alias_xlfd_label", args, n);
    XmStringFree(string);

 /* *************** location_label : XmLabel *************** */
  n = 0;
    string = CMPSTR(LOCATION);
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 6); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++; 
  alias_location_label =
    XmCreateLabel(form, "alias_location_label", args, n);
    XmStringFree(string);
  
  /* *************** font_label : XmLabel *************** */
    string = CMPSTR(FONT);
        string = XmStringConcat(string, XmStringCreate(":",
                                       XmFONTLIST_DEFAULT_TAG));
  n = 0;
  XtSetArg(args[n], XmNlabelString, string); n++; 
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 8); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  font_label =
    XmCreateLabel(form, "font_label", args, n);
  
    XmStringFree(string);
  
  /* *************** font_xlfd_label : XmLabel *************** */
  n = 0;
    string = CMPSTR(FONT_XLFD);
  XtSetArg(args[n], XmNlabelString, string); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 9); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  font_xlfd_label =
    XmCreateLabel(form, "font_xlfd_label", args, n);
    XmStringFree(string);

  /*
   * Find the longest label to line up colons ":".
   */

  XtVaGetValues (font_group, XmNwidth, &width1, NULL);
  XtVaGetValues (alias_label, XmNwidth, &width2, NULL);
  XtVaGetValues (alias_xlfd_label, XmNwidth, &width3, NULL);
  XtVaGetValues (font_label, XmNwidth, &width4, NULL);
  XtVaGetValues (font_xlfd_label, XmNwidth, &width5, NULL);
  XtVaGetValues (alias_location_label, XmNwidth, &width7, NULL);
  longest = width1;
  longest_label = font_group;
  if (width2 > longest) {
    longest = width2;
    longest_label = alias_label;
  }
  
  if (width3 > longest) {
    longest = width3;
    longest_label = alias_xlfd_label;
  }

  if (width4 > longest) {
    longest = width4;
    longest_label = font_label;
  }
  
  if (width5 > longest) {
    longest = width5;
    longest_label = font_xlfd_label;
  }
  
  if (width7 > longest) {
    longest = width7;
    longest_label = alias_location_label;
  }

  
  /*
   * Now that we know which label is longest,
   * align other label with this label.
   */
  if (font_group != longest_label) 
    XtVaSetValues (font_group,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);
  if (alias_label != longest_label) 
    XtVaSetValues (alias_label,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);
  if (alias_xlfd_label != longest_label) 
    XtVaSetValues (alias_xlfd_label,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);
  if (alias_location_label != longest_label) 
    XtVaSetValues (alias_location_label,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);
  if (font_label != longest_label) 
    XtVaSetValues (font_label,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);
  if (font_xlfd_label != longest_label) 
    XtVaSetValues (font_xlfd_label,
		   XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
		   XmNrightWidget, longest_label, NULL);

  /* *************** font_group : XmLabel *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 0); n++;
  XtSetArg(args[n], XmNtopOffset,  10); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, font_group); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_NONE); n++;
  font_group_name =
    XmCreateLabel(form, "font_group_name", args, n);

/* *************** alias_name : XmLabel *************** */
  n = 0;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_END); n++;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 4); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++; 
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, alias_label); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++; 
  alias_name =
    XmCreateLabel(form, "alias_name", args, n);
  
  /* *************** xlfd_name : XmTextField *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 5); n++;

  XtSetArg(args[n], XmNtopOffset, 10); n++;

  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, alias_xlfd_label); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++; 
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNeditable, False); n++;
  XtSetArg(args[n], XmNcolumns, 65); n++;
  /*  XtSetArg(args[n], XmNresizeWidth, True); n++;*/
  alias_xlfd_name =
    XmCreateTextField (form, "alias_xlfd_name", args, n);
  
  /* *************** loc_name : XmTextField *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 6); n++; 

  XtSetArg(args[n], XmNtopOffset, 10); n++;

  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, alias_location_label); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++; 
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNeditable, False); n++;
  XtSetArg(args[n], XmNcolumns, 65); n++;
  alias_location_name =
    XmCreateTextField (form, "alias_location_name", args, n);
  
  /* *************** font_name : XmLabel *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopOffset, 10); n++;
  XtSetArg(args[n], XmNtopPosition, 8); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, font_label); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++;
  font_name =
    XmCreateLabel (form, "font_name", args, n);
  
  /* *************** font_xlfd_name : XmTextField *************** */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_POSITION); n++;
  XtSetArg(args[n], XmNtopPosition, 9); n++;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); n++;
  XtSetArg(args[n], XmNleftWidget, font_xlfd_label); n++;
  XtSetArg(args[n], XmNleftOffset, 10); n++;

  XtSetArg(args[n], XmNtopOffset, 10); n++;

  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); n++;
  XtSetArg(args[n], XmNeditable, False); n++;
  XtSetArg(args[n], XmNcolumns, 65); n++;
  /*   XtSetArg(args[n], XmNresizeWidth, True); n++;*/

  font_xlfd_name =
    XmCreateTextField (form, "font_xlfd_name", args, n);
  
  n = 0;
  children[n++] = font_group;
  children[n++] = font_size;
  children[n++] = font_size_val;
  children[n++] = font_group_name;
  children[n++] = separator1;
  children[n++] = alias_label;
  children[n++] = alias_name;
  children[n++] = alias_xlfd_name;
  children[n++] = alias_xlfd_label;
  children[n++] = alias_location_label;
  children[n++] = alias_location_name;
  children[n++] = separator2;
  children[n++] = font_label;
  children[n++] = font_name;
  children[n++] = font_xlfd_label;
  children[n++] = font_xlfd_name;
  XtManageChildren (children, n);

  n = 0;
  children[n++] = form;
  XtManageChildren (children, n);
  putDialog (XtParent(style.fontDialog), attr_dlg);
}
  
  XtManageChild (attr_dlg);
  set_attribute_fields(); 
  XtPopup (XtParent(attr_dlg), XtGrabNone);

}

static void
#ifdef _NO_PROTO
addButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
addButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    Widget           parent = (Widget) client_data;
    register int     n;
    Arg              args[10];
    XmString         string;
    XmString         string1;
    XmString         button_string[NUM_LABELS];
    Widget           children[16];
    char	     tmpStr[MAXNAMLEN];
 

  _DtTurnOnHourGlass(style.fontDialog);

if(add_dlg == NULL) {
  n = 0;
  button_string[0] = CMPSTR(_DtOkString);
  button_string[1] = CMPSTR(_DtCancelString);
  button_string[2] = CMPSTR(_DtHelpString);
 
  XtSetArg (args[n], XmNbuttonCount, NUM_LABELS);  n++;
  XtSetArg (args[n], XmNbuttonLabelStrings, button_string);  n++;
 
  XtSetArg (args[n], XmNchildType, XmWORK_AREA);  n++;
  XtSetArg (args[n], XmNdefaultPosition, False);  n++;
  add_dlg = __DtCreateDialogBoxDialog(parent, "add_dlg", args, n);
 

  n=0;
  XtSetArg(args[n], XmNtitle, ADD_FONT_GROUP); n++;
  XtSetArg (args[n], XmNmwmFunctions, DIALOG_MWM_FUNC); n++;
  XtSetValues (XtParent(add_dlg), args, n);

  XtAddCallback(add_dlg, XmNcallback, add_ButtonCB, NULL);
  XtAddCallback(add_dlg, XmNhelpCallback,
            (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_FONT_ADD_GROUP_DLG);
 
  XmStringFree(button_string[0]);
  XmStringFree(button_string[1]);
  XmStringFree(button_string[2]);
 
  /*
  **  Create a form inside font Group add Dialog
  */
  n = 0;
  XtSetArg(args[n], XmNhorizontalSpacing, style.horizontalSpacing); n++;
  XtSetArg(args[n], XmNverticalSpacing, style.verticalSpacing); n++;
  font.addForm = XmCreateForm(add_dlg, "addForm", args, n);
  XtManageChild(font.addForm);

  /* TitleBox for Proportional font list */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM);     n++;
  XtSetArg(args[n], XmNleftAttachment,     XmATTACH_FORM);     n++;
  XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
  XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;
 
  string = CMPSTR(PROPORTIONAL);
  XtSetArg(args[n], XmNtitleString, string); n++;
  font.addPropTB =
        _DtCreateTitleBox(font.addForm, "addPropTB", args, n);
  XmStringFree(string);
  XtManageChild(font.addPropTB);

 
  n=0;
  XtSetArg (args[n], XmNscrollBarDisplayPolicy,    XmSTATIC); n++;
  XtSetArg (args[n], XmNselectionPolicy,    XmBROWSE_SELECT); n++;
  XtSetArg (args[n], XmNautomaticSelection, True); n++;
  XtSetArg (args[n], XmNvisibleItemCount,   7); n++;
  XtSetArg (args[n], XmNlistSizePolicy,   XmCONSTANT); n++;
  font.addPropList = XmCreateScrolledList(font.addPropTB, 
						"addPropList",args,n);

  /* TitleBox for mono font list */
  n = 0;
  XtSetArg(args[n], XmNtopAttachment,      XmATTACH_FORM);     n++;
  XtSetArg(args[n], XmNleftAttachment,     XmATTACH_WIDGET);     n++;
  XtSetArg(args[n], XmNleftWidget,         font.addPropTB);     n++;
  XtSetArg(args[n], XmNrightAttachment,    XmATTACH_FORM);     n++;
  XtSetArg(args[n], XmNtopOffset,          style.verticalSpacing+5);  n++;
  XtSetArg(args[n], XmNrightOffset,        style.horizontalSpacing);  n++;

  string = CMPSTR(MONO);
  XtSetArg(args[n], XmNtitleString, string); n++;
  font.addMonoTB =
        _DtCreateTitleBox(font.addForm, "addMonoTB", args, n);
  XmStringFree(string);
  XtManageChild(font.addMonoTB);

 
  n=0;
  XtSetArg (args[n], XmNscrollBarDisplayPolicy,    XmSTATIC); n++;
  XtSetArg (args[n], XmNselectionPolicy,    XmBROWSE_SELECT); n++;
  XtSetArg (args[n], XmNautomaticSelection, True); n++;
  XtSetArg (args[n], XmNvisibleItemCount,   7); n++;
  XtSetArg (args[n], XmNlistSizePolicy,   XmCONSTANT); n++;
  font.addMonoList = XmCreateScrolledList(font.addMonoTB, 
						"addMonoList",args,n);

  charset_label = XtVaCreateWidget("charset_label", xmLabelGadgetClass, 
	font.addForm,
        XmNleftAttachment,   XmATTACH_FORM,
  /*      XmNlabelString,      string, */
        XmNtopAttachment,    XmATTACH_WIDGET,
        XmNtopWidget,        font.addPropTB,
        NULL);


  children[0] = _DtDialogBoxGetButton(add_dlg,2);
  n=0;
  XtSetArg(args[n], XmNautoUnmanage, False); n++;
  XtSetArg(args[n], XmNcancelButton, children[0]); n++;
  XtSetValues (add_dlg, args, n);

  n = 1;
  XtManageChildren (children, n);

  n = 0;
  children[n++] = charset_label;
  XtManageChildren (children, n);

  n = 0;
  children[n++] = font.addPropList;
  XtManageChildren (children, n);

  n = 0;
  children[n++] = font.addMonoList;
  XtManageChildren (children, n);

  putDialog (XtParent(style.fontDialog), add_dlg);

}

  if(charset_count) {
    sprintf(tmpStr, CHARSET, charset_list[0], 1, charset_count);
    string = CMPSTR(tmpStr);
  }
 
  if(!charset_count)
   initFontInfoList( -1, NULL);
  else
   initFontInfoList( 0, charset_list);
 
  if(charset_count) {
    n=0;
    XtSetArg(args[n], XmNuserData, (int) 0); n++;
    XtSetArg(args[n], XmNlabelString, string); n++;
    XtSetValues (charset_label, args, n);
 
    XmStringFree(string);
  }
 
  select_def_items(); 

  _DtTurnOffHourGlass(style.fontDialog);

  XtManageChild (add_dlg);
  XtPopup (XtParent(add_dlg), XtGrabNone);
}

static void
#ifdef _NO_PROTO
select_def_items()
#else
select_def_items()
#endif /* _NO_PROTO */
{
    FgroupRec      *temp_fg;
    char           *group_name;
    XmStringTable  selectedItem;
    fontInfo       *mono_font;
    fontInfo       *prop_font;
    char	   tmpStr[MAXNAMLEN];
    char     **XLFDNameFields, *freeMe;
    int            ch_num = 0;


  /* 
   * charset_label is created only when add group dialog is
   * if it is not created, no need to select default items
   */

  if(charset_label)
     XtVaGetValues(charset_label,
                           XmNuserData, &ch_num,
                           NULL);
  else
	return;

  XtVaGetValues(font.groupList,
                           XmNselectedItems, &selectedItem,
                           NULL);

  XmStringGetLtoR(selectedItem[0], XmFONTLIST_DEFAULT_TAG, &group_name);

  if((strcmp(group_name, "Default") == 0) 
		&& font.addPropList && font.addMonoList) {
	XtVaSetValues(font.addPropList, XmNselectedItemCount, 0, 0);
	XtVaSetValues(font.addMonoList, XmNselectedItemCount, 0, 0);
        return;
  }
 
  if((temp_fg = find_group_selected(group_name, font_groups)) == NULL) {
        fprintf(stderr, "select_def_items(): Font group %s not found ... \n", 
								group_name);
        return;
  }

  mono_font = temp_fg->mono_fonts;
  prop_font = temp_fg->prop_fonts;

  if((mono_font == NULL) || (prop_font == NULL))
    return;
 
  if(font.addMonoList == NULL || font.addPropList == NULL)
	return;
  else {
    if(charset_count > 1) {

	/* 
	 * select default items in add font group dialog 
	 */

	while(mono_font) {
        if (splitXLFDName(mono_font->font_defn, &XLFDNameFields, &freeMe)) {
	    sprintf(tmpStr, "%s-%s", XLFDNameFields[XLFD_REGISTRY],
          				XLFDNameFields[XLFD_ENCODING]);
	   XtFree((char *)XLFDNameFields);
           XtFree(freeMe);

           if(!strcasecmp(tmpStr, charset_list[ch_num])) {
              XmListSetItem(font.addMonoList, 
		XmStringCreateSimple(mono_font->font_hrn));
              XmListSelectItem(font.addMonoList,
                XmStringCreateSimple(mono_font->font_hrn), False);
	      break;
	   } 
	 }
	    mono_font = mono_font->next;
	}

	while(prop_font) {
        if (splitXLFDName(prop_font->font_defn, &XLFDNameFields, &freeMe)) {
            sprintf(tmpStr, "%s-%s", XLFDNameFields[XLFD_REGISTRY],
                                        XLFDNameFields[XLFD_ENCODING]);
           XtFree((char *)XLFDNameFields);
           XtFree(freeMe);
 
           if(!strcasecmp(tmpStr, charset_list[ch_num])) {
              XmListSetItem(font.addPropList,
                XmStringCreateSimple(prop_font->font_hrn));
              XmListSelectItem(font.addPropList,
                XmStringCreateSimple(prop_font->font_hrn), False);
	      break;
	   }
         }
         prop_font = prop_font->next;
        }

    }else {
        XmListSetItem(font.addMonoList, 
			XmStringCreateSimple(mono_font->font_hrn));
        XmListSelectItem(font.addMonoList,
                        XmStringCreateSimple(mono_font->font_hrn), False);
        XmListSetItem(font.addPropList, 
			XmStringCreateSimple(prop_font->font_hrn));
        XmListSelectItem(font.addPropList,
                        XmStringCreateSimple(prop_font->font_hrn), False);
    }

  }

  XtFree(group_name);

}

static void
#ifdef _NO_PROTO
deleteButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
deleteButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
    XmStringTable selectedItem;
    char *current_choice;
    register int     n;
    Arg              args[10];
    char            *tmpStr;
 
    XtVaGetValues(font.groupList,
                           XmNselectedItems, &selectedItem,
                           NULL);

    XmStringGetLtoR(selectedItem[0],
                        XmFONTLIST_DEFAULT_TAG, &current_choice);

    if(strcmp(current_choice, "Default") == 0) {
        InfoDialog(((char *)GETMESSAGE(5, 96,
            "Can't delete Default font group.\n")),style.fontDialog, False); 
	return;
    }

    if(deleteDialog == NULL) {
        n=0;
        XtSetArg(args[n], XmNokLabelString, CMPSTR(_DtOkString)); n++;
        XtSetArg(args[n], XmNcancelLabelString, CMPSTR(_DtCancelString)); n++;
        XtSetArg(args[n], XmNhelpLabelString, CMPSTR(_DtHelpString)); n++;
        XtSetArg(args[n], XmNdialogType, XmDIALOG_INFORMATION);             n++;
        XtSetArg(args[n], XmNborderWidth, 3);                               n++;
        XtSetArg(args[n], XmNdefaultPosition, False);                      n++;
        deleteDialog = XmCreateQuestionDialog(style.fontDialog,
                                              "deleteDialog", args, n);
        XtAddCallback(deleteDialog, XmNmapCallback, CenterMsgCB,
                      style.fontDialog);
        
        XtAddCallback(deleteDialog, XmNcancelCallback, deleteCancelCB, NULL);

        XtAddCallback(deleteDialog, XmNokCallback, deleteOkCB, 
				(XtPointer) current_choice);
	
	XtAddCallback(deleteDialog, XmNhelpCallback,
            (XtCallbackProc)HelpRequestCB, (XtPointer)HELP_FONT_DEL_GROUP_DLG);


        n = 0;
        XtSetArg (args[n], XmNmwmInputMode,
                        MWM_INPUT_PRIMARY_APPLICATION_MODAL); n++;
        XtSetArg (args[n], XmNuseAsyncGeometry, True); n++;
        XtSetArg (args[n], XmNtitle, ((char *)GETMESSAGE(5, 98, 
				"Delete Font Group"))); n++;
        XtSetArg (args[n], XmNmwmFunctions, DIALOG_MWM_FUNC); n++;
        XtSetValues (XtParent (deleteDialog), args, n);
 
    }
    n = 0;
    tmpStr = XtMalloc(strlen(DEL_GRP) + strlen(current_choice) + 1);
    sprintf(tmpStr, DEL_GRP, current_choice);
    XtSetArg(args[n], XmNmessageString, CMPSTR(tmpStr)); n++;
    XtSetValues(deleteDialog, args, n);

    XtFree (tmpStr);
    XtFree(current_choice);
  
    XtManageChild(deleteDialog);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
/* attr_ButtonCB                                   */
/* Process callback from PushButtons in DialogBox. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
attr_ButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
attr_ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  DtDialogBoxCallbackStruct *cb
           = (DtDialogBoxCallbackStruct *) call_data;

    switch (cb->button_position)
    {
      case ATTR_CANCEL_BUTTON:
            XtUnmanageChild(attr_dlg);
	    break;
      case ATTR_HELP_BUTTON:
	    XtCallCallbacks(attr_dlg, XmNhelpCallback, (XtPointer) HELP_FONT_ATTRIB_DIALOG);
            break;
      default:
            break;
    }
}
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
deleteOkCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
deleteOkCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  int pos;
  XmStringTable selectedItem;
  char *current_choice;
 
    XtVaGetValues(font.groupList,
                           XmNselectedItems, &selectedItem,
                           NULL);
 
    XmStringGetLtoR(selectedItem[0],
                        XmFONTLIST_DEFAULT_TAG, &current_choice);

    if(RemoveGroup(current_choice) == True) {
       pos = XmListItemPos(font.groupList, selectedItem[0]);
       XmListDeletePos (font.groupList, pos);  
       XmListSelectPos(font.groupList,   pos - 1, TRUE); 
    }
    else
            InfoDialog(CANT_DELETE, style.fontDialog, False);

    XtUnmanageChild(deleteDialog);
    XtFree(current_choice);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
deleteCancelCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
deleteCancelCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
	    XtUnmanageChild(deleteDialog);
}

static Boolean
#ifdef _NO_PROTO
RemoveGroup(group_name)
	char *group_name;
#else
RemoveGroup( char *group_name)
#endif /* _NO_PROTO */
{
    int  result = -1;
    char *fonts_group;
    char *tmpStr;
    FgroupRec *fgroup;
    FgroupRec *fgroup_prev;
    char buf[MAXPATHLEN];
    int status = -1;
 
    if((fgroup = find_group_selected(group_name, font_groups)) == NULL) {
        fprintf(stderr, "RemoveGroup(): Font group %s not found ... \n", group_name);
        return (FALSE);
    }

    if((fgroup_prev = find_group_prev(group_name, font_groups)) == NULL) {
        fprintf(stderr, "Previous group of %s not found ... \n", group_name);
        return (FALSE);
    }

        fonts_group = (char *)XtMalloc(strlen(fgroup->location) + 
			strlen(group_name) + strlen("sdtfonts.group") + 3);

        sprintf(fonts_group, "%s/%s", fgroup->location, "sdtfonts.group");
 
        status = unlink(fonts_group);
 
        if(status != 0)
        {
            tmpStr = (char *)XtMalloc(strlen(ERR1) + strlen(fonts_group) + 1);
            sprintf (tmpStr, ERR1, fonts_group);
            _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
            _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
            XtFree(tmpStr);
        }

        sprintf(fonts_group, "%s/%s", fgroup->location, "fonts.alias");
        result = unlink(fonts_group);
 
        if(result != 0)
        {
            tmpStr = (char *)XtMalloc(strlen(ERR1) + strlen(fonts_group) + 1);
            sprintf (tmpStr, ERR1, fonts_group);
            _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL); 
            _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
            XtFree(tmpStr);
        }

        sprintf(fonts_group, "%s", fgroup->location);

	if((result = rmdir(fonts_group)) == -1)
	{
	    tmpStr = (char *)XtMalloc(strlen(ERR1) + strlen(fonts_group) + 1);
            sprintf (tmpStr, ERR1, fonts_group);
            _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
            _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
            XtFree(tmpStr);
	}


    	XtFree(fonts_group);

        sprintf(buf,"%s %s %s %s", XSET,
                               "-fp",
                               fgroup->location,
                               "> /dev/null 2>/dev/null");
        system(buf);

	if(status != 0)
            return(False);
	else {
	   DeleteFgroup(fgroup_prev, fgroup);
	   return (True);
	}
}


/*+++++++++++++++++++++++++++++++++++++++++++++++++*
/* add_ButtonCB                                   */
/* Process callback from PushButtons in DialogBox. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
add_ButtonCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
add_ButtonCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  DtDialogBoxCallbackStruct *cb
           = (DtDialogBoxCallbackStruct *) call_data;
  int items_selected = 0;
  static int charset_num = 1;
  int ch_num;



    switch (cb->button_position)
    {
      case OK_BUTTON:
             XtVaGetValues(font.addPropList,
                           XmNselectedItemCount, &items_selected,
                           NULL);
 
             if(items_selected == 0) {
              ErrDialog(((char *)GETMESSAGE(5, 102,
              "Items in User and System font group lists should be selected.")),
 w);
              return;
             }
 
             XtVaGetValues(font.addMonoList,
                           XmNselectedItemCount, &items_selected,
                           NULL);
 
             if(items_selected == 0) {
              ErrDialog(((char *)GETMESSAGE(5, 102,
              "Items in User and System font group lists should be selected.")),
 w);
              return;
             }
	    
             XtVaGetValues(charset_label,
                           XmNuserData, &ch_num,
                           NULL);
	    if(!ch_num)
		charset_num = 1; 

	    if(charset_num != charset_count) {
	      writeTmpFontGrp(charset_num - 1);

	      if(strcmp(charset_list[charset_num], "NULL") == 0)
	        set_other_charset(++charset_num);
	      else
	        set_other_charset(charset_num);

	      ++charset_num;

	      return;
  	    }

	    fontGrpDlg();
            break;
      case CANCEL_BUTTON:
	    if(charset_count > 1)
	       cleanUserTmp();
            XtUnmanageChild(add_dlg);
            break;
      case HELP_BUTTON:
	    XtCallCallbacks(add_dlg, XmNhelpCallback, (XtPointer) HELP_FONT_ADD_GROUP_DLG);
            break;
      default:
            break;
    }
  
}

static void
#ifdef _NO_PROTO
writeTmpFontGrp(charset_num)
  int charset_num;
#else
writeTmpFontGrp(int charset_num)
#endif /*_NO_PROTO */
{
char *system_group, *user_group;
char tmpStr[MAXNAMLEN];
char usr_font[MAXNAMLEN];
char sys_font[MAXNAMLEN];
XmString string;
char *ch, *name;
XmStringTable  selectedItem;
int i;


           XtVaGetValues(font.addPropList,
                           XmNselectedItems, &selectedItem,
                           NULL);
 
           XmStringGetLtoR(selectedItem[0],
                        XmFONTLIST_DEFAULT_TAG, &system_group);
 
           name = (char *)XtMalloc(strlen(system_group) + 1);
           strcpy(name, system_group);
 
           ch = name;
 
           for(i = 0;i <= strlen(name); i++) {
 
              if(*ch == ')') {
                *ch = '\0';
                break;
              }
 
              if(*ch == ' ' || *ch == '(')
                *ch = '_';
 
                ++ch;
           }
 
             XtVaGetValues(font.addMonoList,
                           XmNselectedItems, &selectedItem,
                           NULL);
 
             XmStringGetLtoR(selectedItem[0],
                                XmFONTLIST_DEFAULT_TAG, &user_group);


	    if(tmp_fgdir == NULL) /* create tmp_fgdir if it is not created */
                tmp_fgdir = tmpFontGroup(name);

	    sprintf(usr_font, "%s %s", user_group,charset_list[charset_num]);
	    sprintf(sys_font, "%s %s", system_group,charset_list[charset_num]);

	    WriteFontInfo(usr_font, sys_font, tmp_fgdir);

	    XtFree(system_group);
	    XtFree(user_group);
}

static void
#ifdef _NO_PROTO
set_other_charset(charset_num)
  int charset_num;
#else
set_other_charset( int charset_num)
#endif /*_NO_PROTO */
{
char tmpStr[MAXNAMLEN];
XmString string;

            if(!charset_count)
               return;

            sprintf(tmpStr, CHARSET, charset_list[charset_num], charset_num + 1, charset_count);
            string = CMPSTR(tmpStr);

            XtVaSetValues(charset_label,
              XmNuserData, (int) charset_num, 
              XmNlabelString,      string,
              NULL);
 
            XmStringFree(string);

	    XmListDeleteAllItems(font.addMonoList);
	    XmListDeleteAllItems(font.addPropList);

            initFontInfoList(charset_num, charset_list);

	    select_def_items();

	
}

/*
 * initFontInfoList 
 *
 */
 
static void
#ifdef _NO_PROTO
 initFontInfoList(i, charset_list)
 int i;
 char **charset_list;
#else
 initFontInfoList(
		int i, char **charset_list)
#endif /* _NO_PROTO */
{
    char       **fontlist, *familyLabel;
    char	familyLabel_charset[MAXNAMLEN];
    int          ii,nfonts;
    char     **XLFDNameFields, *freeMe;
    FgroupRec *fi;
    char list_of_fonts[100];
    XmString str;

    if(i >= 0)
      sprintf(list_of_fonts, "%s%s","-*-*-*-*-*-*-*-*-*-*-*-*-", 
							charset_list[i]);
    else
      strcpy(list_of_fonts, "-*-*-*-*-*-*-*-*-*-*-*-*-*-*"); 

    fontlist = XListFonts(style.display, list_of_fonts, BIGNUM, &nfonts);

    for(ii = 0; ii < nfonts; ii++) {
 

        if (!splitXLFDName(fontlist[ii], &XLFDNameFields, &freeMe))
            continue;

	if(strcmp(XLFDNameFields[XLFD_FOUNDRY], "dt") != 0) 
        {

          familyLabel = get_hrn(XLFDNameFields);

	/* 
	 * if same family name exists in different charsets, continue to 
	 * construct list ...
 	 */ 
	  if(charset_count > 1) {

           sprintf(familyLabel_charset,"%s %s", familyLabel, charset_list[i]);

           if((fi = find_group_selected(familyLabel_charset, 
					new_font_groups)) == NULL)
              fi = getFontInfo(fontlist[ii], familyLabel, familyLabel_charset,
						XLFDNameFields, TRUE);
	   else {
    		str = XmStringCreateSimple(familyLabel);
             if((!XmListItemExists(font.addMonoList, str)) && 
			(!XmListItemExists(font.addPropList, str))){
               switch (XLFDNameFields[XLFD_SPACING][0]) {
	        case 'p':
	          construct_list(familyLabel, PROP_FONT, font.addPropList);
                  break;
                case 'm': case 'c':
  	          construct_list(familyLabel, MONO_FONT, font.addMonoList);
	          construct_list(familyLabel, PROP_FONT, font.addPropList);
                  break;
               default:
                  break;
	     }
            }
		XmStringFree(str);
	   }
	  }
	  else  {
	   if((fi = find_group_selected(familyLabel, 
					new_font_groups)) == NULL)
             fi = getFontInfo(fontlist[ii], familyLabel, (char *) NULL,
						XLFDNameFields, TRUE);

	   }


          switch (XLFDNameFields[XLFD_SPACING][0]) {
	    case 'p':
	       add_prop_to_list(fi, fontlist[ii], XLFDNameFields);
	       break;
	    case 'm': case 'c':
	       add_mono_to_list(fi, fontlist[ii], XLFDNameFields);
	       add_prop_to_list(fi, fontlist[ii], XLFDNameFields);
	       break;
	    default:
	       break;
	  }

          XtFree(familyLabel);
 	}
        XtFree((char *)XLFDNameFields);
        XtFree(freeMe);
    }

}

/* 
 * Add mono font to the list.
 */

static void
#ifdef _NO_PROTO
add_mono_to_list(fi, font_name, XLFDNameFields)
FgroupRec *fi;
char	*font_name;
char	**XLFDNameFields;
#else
add_mono_to_list( 
	FgroupRec *fi, char *font_name, char **XLFDNameFields)
#endif /* _NO_PROTO */
{
fontInfo *mono_fonts = NULL;
fontInfo *temp = NULL;
char **XLFDAliasNameFields;
char *freeMe;
ScaleType sc;

        if((sc = ScaleFromSize(XLFDNameFields)) == -1)
           return;

         mono_fonts = (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         mono_fonts->alias_name =  get_alias(font_name, MONO_FONT, SizeStr[sc]);
         mono_fonts->font_defn =  _DtNewString(font_name);

        if (splitXLFDName(mono_fonts->alias_name, 
				&XLFDAliasNameFields, &freeMe)) {

         mono_fonts->alias_hrn =  get_hrn(XLFDAliasNameFields);;
         XtFree((char *)XLFDAliasNameFields);
         XtFree(freeMe);
	}
        else
         mono_fonts->alias_hrn =  NULL;
	
         mono_fonts->font_hrn  =  get_hrn(XLFDNameFields); 
         mono_fonts->size = SizeStr[sc];
         mono_fonts->next = NULL;


	if(fi->mono_fonts == NULL)
	  fi->mono_fonts = mono_fonts;
	else {
           temp = fi->mono_fonts;
           while(temp->next)
               temp = temp->next;
	    temp->next = mono_fonts;
	}

}

/* 
 * Add prop font to the list.
 */

static void
#ifdef _NO_PROTO
add_prop_to_list(fi, font_name, XLFDNameFields)
FgroupRec *fi;
char 	*font_name;
char 	**XLFDNameFields;
#else
add_prop_to_list( 
	FgroupRec *fi, char *font_name, char **XLFDNameFields)
#endif /* _NO_PROTO */
{
fontInfo *prop_fonts = NULL;
fontInfo *temp = NULL;
char **XLFDAliasNameFields;
char *freeMe;
ScaleType sc;

 


	/* if there is a point_size that does not fall under xxs .. xxl and
	 * it is not scalable don't put it in the list 
	 */

        if((sc = ScaleFromSize(XLFDNameFields)) == -1)
	   return;
       
         prop_fonts = (fontInfo *) _DtMalloc(sizeof(struct _fontInfo));
         prop_fonts->alias_name =  get_alias(font_name, PROP_FONT, SizeStr[sc]);
         prop_fonts->font_defn =  _DtNewString(font_name);

        if (splitXLFDName(prop_fonts->alias_name,
                                &XLFDAliasNameFields, &freeMe)) {

         prop_fonts->alias_hrn =  get_hrn(XLFDAliasNameFields);;
         XtFree((char *)XLFDAliasNameFields);
         XtFree(freeMe);
        }
        else
         prop_fonts->alias_hrn =  NULL;

         prop_fonts->font_hrn  =  get_hrn(XLFDNameFields); 
         prop_fonts->size = SizeStr[sc];
         prop_fonts->next = NULL;
 
        if(fi->prop_fonts == NULL)
          fi->prop_fonts = prop_fonts;
        else {
           temp = fi->prop_fonts;
           while(temp->next)
               temp = temp->next;
           temp->next = prop_fonts;
        }
}
/*
 * get alias name from the font definition
 */
 
static char *
#ifdef _NO_PROTO
get_alias(font_name, which_type, sz_string)
char    *font_name;
list_type which_list;
char    *sz_string;
#else
get_alias(
        char *font_name, list_type which_type, char *sz_string)
#endif /* _NO_PROTO */
{
char alias_name[256];
char buf[256];
char **XLFDNameFields;
char *freeMe;
char buf_mono_str[256] = "-dt-interface user-medium";
char buf_prop_str[256] = "-dt-interface system-medium";
char buf_bold_str[256] = "-dt-interface user-bold";
int pixel_size;

   if (!splitXLFDName(font_name, &XLFDNameFields, &freeMe))
       return (NULL);

   switch(which_type) {
     case MONO_FONT:
             sprintf(buf, "%s", buf_mono_str); 
	        break;
     case BOLD_FONT:
             sprintf(buf, "%s", buf_bold_str); 
	        break;
     case PROP_FONT:
             sprintf(buf, "%s", buf_prop_str); 
		break;
     default:
		break;
   }
   
   pixel_size = SizeStrToPixelSize(sz_string);

   (void) sprintf(alias_name,
          "%s-%s-%s-%s %s-%d-%s-%s-%s-%s-%s-%s-%s",
	  buf,
          XLFDNameFields[XLFD_SLANT],
          XLFDNameFields[XLFD_SETWIDTH],
          sz_string,
	  XLFDNameFields[XLFD_ADD_STYLE],
	  pixel_size,
          XLFDNameFields[XLFD_POINT_SIZE],
          XLFDNameFields[XLFD_RES_X],
          XLFDNameFields[XLFD_RES_Y],
          XLFDNameFields[XLFD_SPACING],
          XLFDNameFields[XLFD_AVG_WIDTH],
          XLFDNameFields[XLFD_REGISTRY],
          XLFDNameFields[XLFD_ENCODING]);

 
  XtFree((char *)XLFDNameFields);
  XtFree(freeMe);

  return(XtNewString(alias_name));


}


/* 
 * Add item to the list in alphabetical order.  Perform binary
 * search to find the correct location for the new item position.
 */

static void
#ifdef _NO_PROTO
construct_list(newtext, which_list, list_w)
char *newtext;
list_type which_list;
Widget list_w;
#else
construct_list(
    char     *newtext, list_type which_list, Widget list_w)
#endif /* _NO_PROTO */
{
    char *text;
    XmString str, *strlist;
    int u_bound, l_bound = 0;
 
    XtVaGetValues(list_w,
        XmNitemCount, &u_bound,
        XmNitems,     &strlist,
        NULL);
    u_bound--;

    while (u_bound >= l_bound) {
        int i = l_bound + (u_bound - l_bound)/2;

        if (!XmStringGetLtoR(strlist[i], XmFONTLIST_DEFAULT_TAG, &text))
            break;
        if (strcmp(text, newtext) > 0)
            u_bound = i-1; /* newtext comes before item */
        else
            l_bound = i+1; /* newtext comes after item */
        XtFree(text); /* XmStringGetLtoR() allocates memory ... yuk */
    }
    str = XmStringCreateSimple(newtext);

      switch(which_list) 
      {
	case MONO_FONT:
                     XmListAddItemUnselected(font.addMonoList, str, l_bound+1);
		     break;
	case PROP_FONT:
       		     XmListAddItemUnselected(font.addPropList, str, l_bound+1);
		     break;
	default:
		     break;
      }
    XmStringFree(str);
}


static FgroupRec *
#ifdef _NO_PROTO
getFontInfo(font_name, familyLabel, familyLabel_charset, xlfd_field, createInfo)
    char     *font_name;
    char     *familyLabel;
    char     *familyLabel_charset;
    char **xlfd_field;
    Boolean   createInfo;
#else
getFontInfo(
    char     *font_name,
    char     *familyLabel,
    char     *familyLabel_charset,
    char **xlfd_field,
    Boolean   createInfo)
#endif /* _NO_PROTO */
{
    FgroupRec *ptr  = new_font_groups,
             *prev = NULL;
    int       comparison;
 

    if (familyLabel == NULL) {
        return(NULL);
    }
 
    while (ptr != NULL) {
	if(familyLabel_charset)
          comparison = strcmp(ptr->fgroup_name, familyLabel_charset);
	else
          comparison = strcmp(ptr->fgroup_name, familyLabel);
        if (comparison > 0) {                           /* not found */
            break;
        } else if (comparison == 0) {                   /* found */
            return(ptr);
        } else { /* (comparison < 0) */                 /* keep looking */
            prev = ptr;
            ptr = ptr->next;
        }
    }
 
    /*
     * need to add the new one
     */

     switch (xlfd_field[XLFD_SPACING][0]) {
	    case 'p':
	       construct_list(familyLabel, PROP_FONT, font.addPropList);
               break;
            case 'm': case 'c':
  	       construct_list(familyLabel, MONO_FONT, font.addMonoList);
	       construct_list(familyLabel, PROP_FONT, font.addPropList);
               break;
            default:
               break;
     }


    if (createInfo) {
	if(familyLabel_charset)
          return(createFontRec(font_name, xlfd_field, familyLabel_charset, 
								prev, ptr));
	else
          return(createFontRec(font_name, xlfd_field, familyLabel, prev, ptr));
    } else {
        return(NULL);
    }
}

/*
 * createFontRec is used by getFontInfo to create a new FgroupRec struct and
 * insert it into new_font_groups list
 */
static FgroupRec *
#ifdef _NO_PROTO
createFontRec(font_name, xlfd_field,  familyLabel, parent, child)
    char     *font_name;
    char    **xlfd_field;
    char     *familyLabel;
    FgroupRec *parent;
    FgroupRec *child;
#else
createFontRec(
    char     *font_name,
    char    **xlfd_field,
    char     *familyLabel,
    FgroupRec *parent,
    FgroupRec *child)
#endif /* _NO_PROTO */
{
    FgroupRec *newPtr = (FgroupRec*) XtCalloc(1, (unsigned)sizeof(FgroupRec));
 
    newPtr->fgroup_name = XtNewString(familyLabel);
    newPtr->family_name = XtNewString(xlfd_field[XLFD_FAMILY]);
    newPtr->foundry     = XtNewString(xlfd_field[XLFD_FOUNDRY]);
    newPtr->weight      = XtNewString(xlfd_field[XLFD_WEIGHT]);
    newPtr->next       = child;
    newPtr->mono_fonts = NULL;
    newPtr->prop_fonts = NULL;
    newPtr->bold_fonts = NULL;
    if (parent == NULL) {
        new_font_groups   = newPtr;
    } else {
        parent->next   = newPtr;
    }
 
    return(newPtr);
}



static void
#ifdef _NO_PROTO
fontGrpDlg()
#else
fontGrpDlg(void)
#endif /* _NO_PROTO */
{
 
    register int     n;
    Arg              args[10];
    XmString         string;
    XmString         string1;
 
    if (fontGrpName_Dlg == NULL)
    {
        n = 0;
 
        XtSetArg(args[n], XmNokLabelString, CMPSTR(_DtOkString)); n++;
        XtSetArg(args[n], XmNcancelLabelString, CMPSTR(_DtCancelString)); n++;
        XtSetArg(args[n], XmNhelpLabelString, CMPSTR(_DtHelpString)); n++;
        string =  CMPSTR(FONT_GRP_NAME);
        XtSetArg(args[n], XmNselectionLabelString, string); n++;
        string1 =  CMPSTR("");
        XtSetArg(args[n], XmNtextString, string1); n++;
        XtSetArg(args[n], XmNborderWidth, 3); n++;
        XtSetArg(args[n], XmNautoUnmanage, False); n++;
        fontGrpName_Dlg = XmCreatePromptDialog(add_dlg,"fontGrpName_Dlg",args, n);
        XmStringFree(string);
        XmStringFree(string1);

        XtAddCallback(fontGrpName_Dlg, XmNokCallback, add_groupCB,
                                 (XtPointer) NULL);
        XtAddCallback(fontGrpName_Dlg, XmNcancelCallback, add_groupCB,
                                 (XtPointer) NULL);
	XtAddCallback(fontGrpName_Dlg, XmNhelpCallback,
          (XtCallbackProc)HelpRequestCB,(XtPointer)HELP_FONT_GROUP_NAME_DLG);

 
        n = 0;
        XtSetArg (args[n], XmNmwmInputMode,
                        MWM_INPUT_PRIMARY_APPLICATION_MODAL); n++;
        XtSetArg (args[n], XmNuseAsyncGeometry, True); n++;
        XtSetArg (args[n], XmNtitle, 
			((char *)GETMESSAGE(5, 48, "Font Group Name"))); n++;
        XtSetArg (args[n], XmNmwmFunctions, DIALOG_MWM_FUNC); n++;
        XtSetValues (XtParent (fontGrpName_Dlg), args, n);
    }
    n=0;
    string =  CMPSTR("");
    XtSetArg(args[n], XmNtextString, string); n++;
    XtSetValues(fontGrpName_Dlg, args, n);
    XmStringFree(string);
 
    XtManageChild(fontGrpName_Dlg);
 
}
static char *
#ifdef _NO_PROTO
tmpFontGroup(name)
        char *name;
#else
tmpFontGroup(
        char *name)
#endif /* _NO_PROTO */
{
 char buf[MAXPATHLEN];
 int result;
 char *tmpStr;
 DIR *dirp;
 FILE *fp_fgroup;
 char tmpname[MAXPATHLEN];
 
 
/* if $HOME/.dt/tmp doesn't exist, create it. */
 sprintf(buf, "%s%s", style.home, DT_TMP_DIR);

   if( (dirp = opendir(buf)) == NULL)
   {
       result = mkdir(buf, 511);
       if(result == -1)
       {
           tmpStr = (char *)XtMalloc(strlen(ERR3) + strlen(buf) +1);
           sprintf(tmpStr, ERR4, buf);
           _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
           XtFree(tmpStr);
           return(FALSE);
       }
   }else
       closedir(dirp);

 sprintf(tmpname, "%s%s%s_%s", style.home, DT_TMP_DIR, name, "XXXXXX");

 (void) mktemp(tmpname);
 
 strcpy(buf, tmpname);
 
  /*
   * create $HOME/.dt/tmp/<group>
   */
 
   if( (dirp = opendir(buf)) == NULL)
   {
       result = mkdir(buf, 511);
       if(result == -1)
       {
           tmpStr = (char *)XtMalloc(strlen(ERR3) + strlen(buf) +1);
           sprintf(tmpStr, ERR4, buf);
           _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
           XtFree(tmpStr);
           return(FALSE);
       }
   }else
       closedir(dirp);
 

   sprintf(buf,"%s/%s", tmpname, "fonts.alias");

   if((fp_fgroup = fopen (buf, "a")) == NULL) {
       tmpStr = (char *)XtMalloc(strlen(ERR4) + strlen(buf) + 1);
       sprintf (tmpStr, ERR4, buf);
       _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
       XtFree(tmpStr);
       XtFree(buf);
       return(FALSE);
 
   } else
       fclose(fp_fgroup);
 
 
 return(strdup(tmpname));
}



static char *
#ifdef _NO_PROTO
AddFontGroup(name, fgrp)
	char *name;
	char *fgrp;
#else
AddFontGroup(
	char *name, char *fgrp)
#endif /* _NO_PROTO */
{
 char *home_dir;
 char buf[MAXPATHLEN];
 int result;
 char *tmpStr;
 DIR *dirp;
 FILE *fp_fgroup;
 char tmpname[MAXPATHLEN];
 
/* create font directory in user's home directory */
 CreateSdtFontDir(&home_dir);

 if((result = strlen(home_dir) + strlen(name)) > MAXPATHLEN)
  return (FALSE);

 sprintf(tmpname, "%s%s_%s", home_dir, name, "XXXXXX");
 (void) mktemp(tmpname);

 strcpy(buf, tmpname);

  /* 
   * create $HOME/.dt/sdtfonts/<locale>/<group> 
   * if it does not exist 
   */

   if( (dirp = opendir(buf)) == NULL)
   {
       result = mkdir(buf, 511);
       if(result == -1)
       {
           tmpStr = (char *)XtMalloc(strlen(ERR3) + strlen(buf) +1);
           sprintf(tmpStr, ERR4, buf);
           _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
           XtFree(tmpStr);
           return(FALSE);
       }
   }else
       closedir(dirp);

   sprintf(buf,"%s/%s", tmpname, "sdtfonts.group");

   if((fp_fgroup = fopen (buf, "w")) == NULL) {
       tmpStr = (char *)XtMalloc(strlen(ERR4) + strlen(buf) + 1);
       sprintf (tmpStr, ERR4, buf);
       _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
       XtFree(tmpStr);
       XtFree(buf);
       return(FALSE);
   } else
	fprintf(fp_fgroup, "%s\n", fgrp);

   fclose(fp_fgroup);

   sprintf(buf,"%s/%s", tmpname, "fonts.alias");

   if((fp_fgroup = fopen (buf, "w")) == NULL) {
       tmpStr = (char *)XtMalloc(strlen(ERR4) + strlen(buf) + 1);
       sprintf (tmpStr, ERR4, buf);
       _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
       XtFree(tmpStr);
       XtFree(buf);
       return(FALSE);

   } else
       fclose(fp_fgroup);


 XtFree(home_dir);
 return(strdup(tmpname));
}

static Boolean
#ifdef _NO_PROTO
CreateSdtFontDir(home_group_dir)
	char **home_group_dir;
#else
CreateSdtFontDir(
	char **home_group_dir)
#endif /* _NO_PROTO */
{
   int  result;
   char *tmpStr;
   DIR  *dirp;
   
  /* get the $HOME environment varible and constuct the full path */
   *home_group_dir = (char *)XtMalloc(strlen(style.home)
        + strlen(DT_FONT_GROUP_DIR) + strlen(style.lang) + 3);

  /* create the full path name plus file name */
   strcpy(home_group_dir[0], style.home);
   strcat(home_group_dir[0], DT_FONT_GROUP_DIR);
 
  /* create $HOME/.dt/sdtfonts/ if it does not exist */
   if( (dirp = opendir(home_group_dir[0])) == NULL)
   {
       result = mkdir(home_group_dir[0], 511);
       if(result == -1)
       {
           tmpStr = (char *)XtMalloc(strlen(ERR3) +
                strlen(home_group_dir[0]) +1);
           sprintf(tmpStr, ERR4, home_group_dir[0]);
           _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
           XtFree(tmpStr);
           return (FALSE);
       }
   }
   else
	closedir(dirp);
	
   /*  create .dt/sdtfonts/<locale> if it does not exist */
   strcat(home_group_dir[0], style.lang);
   strcat(home_group_dir[0], "/");

   if( (dirp = opendir(home_group_dir[0])) == NULL)
   {
       result = mkdir(home_group_dir[0], 511);
       if(result == -1)
       {
           tmpStr = (char *)XtMalloc(strlen(ERR3) +
           strlen(home_group_dir[0]) +1);
           sprintf(tmpStr, ERR4, home_group_dir[0]);
           _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
           XtFree(tmpStr);
           return (FALSE);
       }
   }
   else
	closedir(dirp);

   return (TRUE);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
/* add_groupCB                                 */
/* Process callback from PushButtons in DialogBox. */
/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
add_groupCB( w, client_data, call_data )
        Widget w ;
        XtPointer client_data ;
        XtPointer call_data ;
#else
add_groupCB(
        Widget w,
        XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
  char 	*user_group, *system_group;
  XmStringTable  selectedItem;
  char  *name;
  int      reason_code;
  XmString str;
  int count_groups = 0;
  int items_selected = 0;
  int i = 0;
  char *token;
  char *tokenSep = " ";
  char *tmpStr, *ch, *fgdir, *fgrp;
  char usr_font[MAXNAMLEN], sys_font[MAXNAMLEN];
  

    reason_code = ((XmAnyCallbackStruct *)call_data)->reason;
 
    switch (reason_code)
    {
      case XmCR_OK:

  	   _DtTurnOnHourGlass(fontGrpName_Dlg);
            /* Get the text from the prompt dialog */
            name = XmTextFieldGetString( XmSelectionBoxGetChild(fontGrpName_Dlg, XmDIALOG_TEXT));

	   if(strlen(name) > MAXNAMLEN)
		name[MAXNAMLEN] = '\0';

   	   tmpStr = (char *)XtCalloc(1, strlen(name) + 1);
   	   strcpy(tmpStr, name);

   	   fgrp = (char *)XtCalloc(1, strlen(name) + 1);
   	   strcpy(fgrp, name);

	   token = strtok (tmpStr, tokenSep);

	   while(token != NULL) {
       	      ++(i);
       	      token = strtok(NULL, tokenSep);
     	   }

	    /* see if the user typed in a valid group name */
           if(i == 0) {
             ErrDialog(((char *)GETMESSAGE(5, 100,
                "The Font Group name cannot contain blank(s).")), w);
           XtFree(name);
  	   _DtTurnOffHourGlass(fontGrpName_Dlg);
           return;
           }

    	   XtVaGetValues(font.addPropList,
                           XmNselectedItems, &selectedItem,
                           NULL);

    	   XmStringGetLtoR(selectedItem[0], 
			XmFONTLIST_DEFAULT_TAG, &system_group);

   	   name = (char *)XtMalloc(strlen(system_group) + 1);
	   strcpy(name, system_group);

	   ch = name;

           for(i = 0;i <= strlen(name); i++) {

	      if(*ch == ')') {
		*ch = '\0';
		break;
	      }

	      if(*ch == ' ' || *ch == '(')
		*ch = '_';

		++ch;
	   }

	   str = XmStringCreateSimple(fgrp);

	   /* add the new group to list if it doesn't exist. */
	   if(!XmListItemExists(font.groupList, str)){


             XtVaGetValues(font.addMonoList,
                           XmNselectedItems, &selectedItem,
                           NULL);

             XmStringGetLtoR(selectedItem[0], 
				XmFONTLIST_DEFAULT_TAG, &user_group);
 
              fgdir = AddFontGroup(name, fgrp);
           /* add the group to the linked list of groups */
              ScanFontGroups(home_fontGroup_dir, &count_groups);
           /* add the name to the scrolled window list */
  	      AddGroupToList(fgrp);
	   /* write the font names and alias names to the fonts.alias */
	    if(charset_count > 1) {
              sprintf(usr_font, "%s %s", user_group,
			charset_list[charset_count - 1]);
              sprintf(sys_font, "%s %s", system_group,
			charset_list[charset_count - 1]);

              WriteFontInfo(usr_font, sys_font, fgdir);

	      /* append $HOME/.dt/tmp/<tmp_fgdir>/fonts.alias  
		 to $HOME/.dt/sdtfonts/<locale>/<fgroup>/fonts.alias */

	      appendToHome(tmp_fgdir, fgdir);
	    }
	    else
	      WriteFontInfo(user_group, system_group, fgdir);

      	      XmListSelectItem(font.groupList, str, True);

	      XtFree(system_group);
	      XtFree(user_group);
 	    }
  	    else 
	      SameName(w, name, fgrp);

  	   _DtTurnOffHourGlass(fontGrpName_Dlg);

            XtUnmanageChild(fontGrpName_Dlg);
            XtUnmanageChild(add_dlg);

	    XmStringFree(str);
            XtFree(name);

            break;
      case XmCR_CANCEL:
            XtUnmanageChild(fontGrpName_Dlg);
            break;
      case XmCR_NO_MATCH:
            XtUnmanageChild(fontGrpName_Dlg);
            break;
      default:
            break;
    }
}

void
#ifdef _NO_PROTO
appendToHome(user_tmp, home_group)
	char *user_tmp;
	char *home_group;
#else
appendToHome(
	char *user_tmp,
	char *home_group)
#endif /* _NO_PROTO */
{
char fonts_alias[MAXPATHLEN];
FILE *fp_tmp;
FILE *fp_home;
char temp_str[MAXNAMLEN];

        sprintf(fonts_alias, "%s/%s", home_group, "fonts.alias");

        if((fp_home = fopen(fonts_alias, "a")) == NULL){
           fprintf(stderr,"Unable to open %s for \
                                saving font aliases.\n",fonts_alias);
           return;
        }

        sprintf(fonts_alias, "%s/%s", user_tmp, "fonts.alias");

        if((fp_tmp = fopen(fonts_alias, "r")) == NULL){
           fprintf(stderr,"Unable to open %s for \
                                saving font aliases.\n",fonts_alias);
           return;
        }

   	while(fgets(temp_str, 512, fp_tmp))
              fputs (temp_str, fp_home);

	fclose(fp_tmp);
	fclose(fp_home);

	/* clean tmp font group directory */

	cleanUserTmp();

	
}

void
#ifdef _NO_PROTO
cleanUserTmp()
#else
cleanUserTmp(void)
#endif /* _NO_PROTO */
{
int status;
char fonts_alias[MAXPATHLEN];
char *tmpStr;

    if(tmp_fgdir) {
        sprintf(fonts_alias, "%s/%s", tmp_fgdir, "fonts.alias");

        if((status = unlink(fonts_alias)) == -1)
        {
            tmpStr = (char *)XtMalloc(strlen(ERR1) + strlen(fonts_alias) + 1);
            sprintf (tmpStr, ERR1, fonts_alias);
            _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
            _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
            XtFree(tmpStr);
        }
 
        if((status = rmdir(tmp_fgdir)) == -1)
        {
            tmpStr = (char *)XtMalloc(strlen(ERR1) + strlen(tmp_fgdir) + 1);
            sprintf (tmpStr, ERR1, tmp_fgdir);
            _DtSimpleErrnoError (progName, DtWarning, NULL, tmpStr, NULL);
            _DtSimpleError (progName, DtWarning, NULL, tmpStr, NULL);
            XtFree(tmpStr);
        }

	/* reset tmp fontgrp dir for next tmp fgroup dirs */
	tmp_fgdir = NULL;
    }

}

void
#ifdef _NO_PROTO
WriteFontInfo(user_group, system_group, name )
	char *user_group;
	char *system_group;
        char *name;
#else
WriteFontInfo(
	char *user_group,
	char *system_group,
        char *name )
#endif /* _NO_PROTO */
{
FgroupRec *fgroup = NULL;
fontInfo *mono_fonts, *prop_fonts, *bold_fonts;
char fonts_alias[MAXPATHLEN];
char buf[MAXPATHLEN];
FILE *fp;


        sprintf(fonts_alias, "%s/%s", name, "fonts.alias");

        if((fp = fopen(fonts_alias, "a")) == NULL){
           fprintf(stderr,"Unable to open %s for \
				saving font aliases.\n",fonts_alias);
	   return;
        }


        if((fgroup = 
	    find_group_selected(user_group, new_font_groups)) == NULL){
            fprintf(stderr,"WriteFontInfo(): Font Group %s not found.\n", user_group);
	
        } else {

	  new_font_list = NULL;

	  mono_fonts = get_final_list(fgroup->mono_fonts, MONO_FONT);

          while(mono_fonts) {
	      sprintf(buf,"%s%s%s%s%s",
				"\"", mono_fonts->alias_name,
				"\" \"", mono_fonts->font_defn,
				"\"\n");
	      mono_fonts = mono_fonts->next;
	      fputs (buf, fp);
	  }
        }


      if((bold_fonts = find_bolder_group(fgroup)) != NULL) {
	  new_font_list = NULL;

	  bold_fonts = get_final_list(bold_fonts, BOLD_FONT);

          while(bold_fonts) {
            sprintf(buf,"%s%s%s%s%s",
                                "\"", bold_fonts->alias_name,
                                "\" \"", bold_fonts->font_defn, 
                                "\"\n");
            bold_fonts = bold_fonts->next;
            fputs (buf, fp);
          }

      }
	

      if((fgroup = 
            find_group_selected(system_group, new_font_groups)) == NULL){
            fprintf(stderr,"WriteFontInfo(): Font Group %s not found.\n", system_group);
	
      } else {
	  new_font_list = NULL;
	  prop_fonts = get_final_list(fgroup->prop_fonts, PROP_FONT);

          while(prop_fonts) {
            sprintf(buf,"%s%s%s%s%s",
                                "\"", prop_fonts->alias_name,
                                "\" \"", prop_fonts->font_defn, 
                                "\"\n");
            prop_fonts = prop_fonts->next;
            fputs (buf, fp);
          }
      }



	
    fclose(fp);

}

static fontInfo *
#ifdef _NO_PROTO
find_bolder_group( fgroup)
        FgroupRec *fgroup;
#else
find_bolder_group(
        FgroupRec *fgroup)
#endif /* _NO_PROTO */
{
fontInfo *bold_font = 0;

#ifdef DEBUG
fprintf(stderr,"find_bolder_group (): group name '%s' \n", fgroup->fgroup_name);
#endif

        if(fgroup == (FgroupRec *)NULL)
                return ((fontInfo *)NULL);


	if(strcmp(fgroup->weight, "bold") == 0)
		return ((fontInfo *)NULL);

	if(strcmp(fgroup->weight, "light") == 0)
	  if((bold_font = find_bold(fgroup, "medium")) == NULL) {
	      if((bold_font = find_bold(fgroup, "demi")) == NULL) {
	        if((bold_font = find_bold(fgroup, "bold")) == NULL)
		 return ((fontInfo *) NULL);
	      }
	  }

	if(strcmp(fgroup->weight, "medium") == 0)
	     if((bold_font = find_bold(fgroup,"demi")) == NULL) {
	        if((bold_font = find_bold(fgroup, "bold")) == NULL)
		 return ((fontInfo *) NULL); 
	     }

	if(strcmp(fgroup->weight, "demi") == 0)
	     if((bold_font = find_bold(fgroup, "bold")) == NULL)
		 return ((fontInfo *) NULL);


	return (bold_font);
	

}

static fontInfo *
#ifdef _NO_PROTO
find_bold( fgroup, weight)
	FgroupRec *fgroup;
	char *weight;
#else
find_bold(
        FgroupRec *fgroup, char *weight)
#endif /* _NO_PROTO */
{
FgroupRec *font_group = new_font_groups;

while(font_group) {
	if((strcmp(fgroup->family_name, font_group->family_name) == 0) &&
		(strcmp(fgroup->foundry, font_group->foundry) == 0)) {
	    if(strcmp(font_group->weight, weight) == 0)
	         break;  /* found the bolder group */
	}
      font_group = font_group->next;
}


 if(font_group)
  return (font_group->mono_fonts);
 else
  return ((fontInfo *) NULL);

}

static fontInfo *
#ifdef _NO_PROTO
get_final_list( fi, type_face)
        fontInfo *fi;
	list_type type_face;
#else
get_final_list(
        fontInfo *fi, list_type type_face)
#endif /* _NO_PROTO */
{
	int all_sizes = 127; /* sum of all sizes */
        fontInfo *final_font_list;
	int size = 0;
	int dup_size = 0;
	fontInfo *temp_fi = fi;

        while(fi) {

 	 if(strcmp(fi->size,"scalable") == 0)
        	size |= scalable;
	 else {

 	   if(strcmp(fi->size,"xxs") == 0)
        	size |= xxsmall;
 	   if(strcmp(fi->size,"xs") == 0)
        	size |= xsmall;
 	   if(strcmp(fi->size,"s") == 0)
        	size |= small;
 	   if(strcmp(fi->size,"m") == 0)
        	size |= medium;
 	   if(strcmp(fi->size,"l") == 0)
        	size |= large;
 	   if(strcmp(fi->size,"xl") == 0)
        	size |= xlarge;
 	   if(strcmp(fi->size,"xxl") == 0)
        	size |= xxlarge;


	   /* ignore duplicate fonts sizes in the list */

	   if(!(dup_size & size)) 
	      create_temp_fi_list(fi, (char *) NULL, type_face);
	 }

	 dup_size |= size;
	 size = 0;

         fi = fi->next;
	}

	 if(dup_size == all_sizes) {
	   DeleteFiList(new_font_list);
	   return(temp_fi); 
	 }
	 else {
	   create_other_sizes(new_font_list, temp_fi, dup_size, type_face);
	  /*  DeleteFiList(temp_fi); */
	 }
		
	return(new_font_list);
}

static void
#ifdef _NO_PROTO
DeleteFiList(orig_fi)
       fontInfo *orig_fi; 
#else
DeleteFiList(
        fontInfo *orig_fi)
#endif /* _NO_PROTO */
{
fontInfo *temp = orig_fi;

  while(temp) {
	temp = temp->next;


	if(orig_fi->alias_name)
	  _DtFree(orig_fi->alias_name);

	if(orig_fi->alias_hrn)
	  _DtFree(orig_fi->alias_hrn);

	if(orig_fi->font_defn)
	  _DtFree(orig_fi->font_defn);

	if(orig_fi->font_hrn)
	  _DtFree(orig_fi->font_hrn);

	if(orig_fi)
	  _DtFree(orig_fi);

	orig_fi = temp;
  }

}

static void
#ifdef _NO_PROTO
DeleteFgroup(prev, fgroup)
       FgroupRec *prev;
       FgroupRec *fgroup;
#else
DeleteFgroup(
        FgroupRec *prev, FgroupRec *fgroup)
#endif /* _NO_PROTO */
{
 
	if(prev == NULL)
	  font_groups = fgroup->next;
	else
	  prev->next = fgroup->next;

	/*
         * point tail of the font_groups list to the last element
         */
        if(cur_font_group == fgroup)
            cur_font_group = prev;


        if(fgroup->fgroup_name)
          _DtFree(fgroup->fgroup_name);

        if(fgroup->family_name)
          _DtFree(fgroup->family_name);

        if(fgroup->foundry)
          _DtFree(fgroup->foundry);
 
        if(fgroup->location)
          _DtFree(fgroup->location);
 
        if(fgroup->mono_fonts)
	  DeleteFiList(fgroup->mono_fonts);

        if(fgroup->prop_fonts)
	  DeleteFiList(fgroup->prop_fonts);

        if(fgroup->mono_fonts)
	  DeleteFiList(fgroup->bold_fonts);
 
        if(fgroup)
          _DtFree(fgroup);
}


static int
#ifdef _NO_PROTO
ScaleFromSize(XLFDNameFields)
	char **XLFDNameFields;
#else
ScaleFromSize(
	char **XLFDNameFields)
#endif /* _NO_PROTO */
{
int point_size;

       point_size = (int) strtol(XLFDNameFields[XLFD_POINT_SIZE], NULL, 10);
 
        point_size = point_size/10;
 
        if (point_size >= 8 && point_size < 10){
		return(xxs);
        }
        if (point_size >= 10 && point_size < 12){
		return(xs);
        }
        if (point_size >= 12 && point_size < 14){
		return(s);
        }
        if (point_size >= 14 && point_size < 17){
		return(m);
        }
        if (point_size >= 17 && point_size < 20){
		return(l);
        }
        if (point_size >= 20 && point_size < 23){
		return(xl);
        }
        if (point_size >= 23 && point_size <= 26){
		return(xxl);
        }
        if (point_size == 0)
		return(Scalable);
     
     return (-1);
}

static void
#ifdef _NO_PROTO
create_temp_fi_list(temp_fi, sz_string, type_face)
     fontInfo *temp_fi;
     char *sz_string;
list_type type_face;
#else
create_temp_fi_list(
     fontInfo *temp_fi, char *sz_string, list_type type_face)
#endif /* _NO_PROTO */
{
  fontInfo *newPtr = (fontInfo*) XtCalloc(1, (unsigned)sizeof(fontInfo));
  fontInfo *cur_entry = new_font_list;


    if(sz_string == NULL) {

       if(type_face == BOLD_FONT)
         newPtr->alias_name = get_alias(temp_fi->font_defn, 
						type_face, temp_fi->size);
       else
         newPtr->alias_name  =  _DtNewString(temp_fi->alias_name);

       newPtr->size        =  _DtNewString(temp_fi->size);
    }else {
       newPtr->alias_name = get_alias(temp_fi->font_defn, type_face, sz_string);
       newPtr->size       =  _DtNewString(sz_string);
    }

    if((strcmp(temp_fi->size, "scalable") == 0) && (sz_string != NULL))
       newPtr->font_defn   =  get_font_defn(temp_fi->font_defn, sz_string);
    else
       newPtr->font_defn   =  _DtNewString(temp_fi->font_defn);

    newPtr->font_hrn  	=  _DtNewString(temp_fi->font_hrn); 
    newPtr->alias_hrn  	=  _DtNewString(temp_fi->alias_hrn); 
    newPtr->next       	=  NULL;

	

    if (new_font_list == NULL) {
        new_font_list   = newPtr;
    } else {
	while(cur_entry->next)
	  cur_entry = cur_entry->next;
        cur_entry->next = newPtr;
    }

}

/*
 * Put pixel size to the font definition, if it is a scalable font
 */

static char *
#ifdef _NO_PROTO
get_font_defn(font_name, sz_string)
char    *font_name;
char	*sz_string;
#else
get_font_defn( char *font_name, char *sz_string)
#endif /* _NO_PROTO */
{
char dest_font_defn[256];
char **XLFDNameFields;
char *freeMe;
int pixel_size;
 
   if (!splitXLFDName(font_name, &XLFDNameFields, &freeMe))
       return (NULL);
 
   pixel_size = SizeStrToPixelSize(sz_string);

   (void) sprintf(dest_font_defn, 
	  "-%s-%s-%s-%s-%s-%s-%d-%s-%s-%s-%s-%s-%s-%s",
          XLFDNameFields[XLFD_FOUNDRY],
          XLFDNameFields[XLFD_FAMILY],
          XLFDNameFields[XLFD_WEIGHT],
          XLFDNameFields[XLFD_SLANT],
          XLFDNameFields[XLFD_SETWIDTH],
          XLFDNameFields[XLFD_ADD_STYLE],
          pixel_size,
          XLFDNameFields[XLFD_POINT_SIZE],
          XLFDNameFields[XLFD_RES_X],
          XLFDNameFields[XLFD_RES_Y],
          XLFDNameFields[XLFD_SPACING],
          XLFDNameFields[XLFD_AVG_WIDTH],
          XLFDNameFields[XLFD_REGISTRY],
          XLFDNameFields[XLFD_ENCODING]);

 
  XtFree((char *)XLFDNameFields);
  XtFree(freeMe);
 
  return(XtNewString(dest_font_defn));
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* create_other_sizes                                   */
/* Creates all sizes in a group. 			*/
/* For example, If small, medium and large are present, */
/* it maps xsmall and xxsmall to 			*/
/* small. xxlarge to large                              */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static void
#ifdef _NO_PROTO
create_other_sizes(temp_fi, orig_fi, size, type_face)
        fontInfo *temp_fi;
	fontInfo *orig_fi;
        int size;
	list_type type_face;
#else
create_other_sizes(
        fontInfo *temp_fi, fontInfo *orig_fi, int size, list_type type_face)
#endif /* _NO_PROTO */
{
fontInfo *fi;
register int i = 0;
int map_to_size = 0;


    if(size & scalable) {
       for(i = 1 ; i <= 7; i++) {
      	if((fi = find_size_selected(new_font_list, i)) == NULL) {
      	   if((fi = find_size_selected(orig_fi, Scalable + 1)) == NULL)
        	fprintf(stderr,"Scalable Font Info not found ... \n");
	}
	else
	   continue;

	create_temp_fi_list(fi, SizeStr[i - 1], type_face);
       }
    }else {
	for(i = 1 ; i <= 7; i++) {    /* i is the size to map */
	  map_to_size = find_size_to_map(size, i);

	  if(i == map_to_size)
		continue; /* this size exists */
	  else {
	      /* 
	       * retrieve size (map_to_size) so size(i) that is not present
	       * can be mapped to it
	       */
	      if((fi = find_size_selected(new_font_list, map_to_size)) != NULL){
		/* size (i) will contain same information as fontInfo (fi)*/
	        create_temp_fi_list(fi, SizeStr[i - 1], type_face);

	      }
	  }

	} /* for */
    }
}

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
/* find_size_to_map                                     */
/* Called by create_other_sizes 			*/
/* Maps existing size fonts to non existing ones.      */
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

static int
#ifdef _NO_PROTO
find_size_to_map(size, sc)
	int size;
	int sc;
#else
find_size_to_map(
	int size, int sc)
#endif /* _NO_PROTO */
{
ScaleType scale;
int cur_diff = 100, new_diff = 100;
int sz_to_use = 0;
 
        for(scale = 1; scale <= 7; ++scale)
          switch(scale)
                {
            case 1:
                 if(size & xxsmall) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 2:
                 if(size & xsmall) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 3:
                 if(size & small) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 4:
                 if(size & medium) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 5:
                 if(size & large) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 6:
                 if(size & xlarge)  {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            case 7:
                 if(size & xxlarge) {
                   if(sc == scale)
                    return(scale);
	           else
		    get_sz_to_use(scale, sc, &new_diff, &cur_diff, &sz_to_use);
		 }
                 break;
            default:
                 break;
           }
 
        return(sz_to_use);
}

static void
#ifdef _NO_PROTO
get_sz_to_use(scale, sc, new_diff, cur_diff, sz_to_use)
 int scale; 
 int sc; 
 int *new_diff; 
 int *cur_diff; 
 int *sz_to_use;
#else
get_sz_to_use(
      int scale, int sc, int *new_diff, int *cur_diff, int *sz_to_use)
#endif /* _NO_PROTO */
{

      *new_diff = abs(scale - sc);

      if(*cur_diff >=  *new_diff) {
        *cur_diff = *new_diff;
        *sz_to_use = scale;
      }
}

void
#ifdef _NO_PROTO
SameName( w, Name, fgrp_name )
        Widget w ;
        char *Name ;
        char *fgrp_name ;
#else
SameName(
        Widget w,
        char *Name,
	char *fgrp_name)
#endif /* _NO_PROTO */
{
   char    *tmpStr;
   int n=0;
   Arg args[10];
 
    tmpStr = (char *)XtMalloc(strlen(DUP_GRP) + strlen(name) + 1);
    sprintf(tmpStr, DUP_GRP, name);

    strcpy(name, Name);
    strcpy(fgrp, fgrp_name); 

   if (font.dupGroup == NULL) {
     XtSetArg(args[n], XmNokLabelString, CMPSTR(_DtOkString)); n++;
     XtSetArg(args[n], XmNcancelLabelString, CMPSTR(_DtCancelString)); n++;
     XtSetArg(args[n], XmNhelpLabelString, CMPSTR(_DtHelpString)); n++;
     XtSetArg(args[n], XmNmwmFunctions, DIALOG_MWM_FUNC ); n++;
     XtSetArg(args[n], XmNdialogTitle, 
		CMPSTR(((char *)GETMESSAGE(14, 21, "Warning")))); n++;
     font.dupGroup = XmCreateWarningDialog(style.fontDialog, "QNotice", 
						args, n);


     XtAddCallback(font.dupGroup, XmNokCallback, 
					OverWriteGroupCB, (XtPointer) NULL);

      n=0;
      XtSetArg (args[n], XmNmwmInputMode,
                        MWM_INPUT_PRIMARY_APPLICATION_MODAL); n++;
      XtSetValues (XtParent(style.fontDialog), args, n);
   }
     n = 0;
     XtSetArg(args[n], XmNmessageString, CMPSTR(tmpStr)); n++;
     XtSetValues (font.dupGroup, args, n);
     XtFree(tmpStr);

   XtManageChild(font.dupGroup);
}

/*+++++++++++++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
OverWriteGroupCB( w, client_data, call_data )
        Widget w;
	XtPointer client_data;
        XtPointer call_data ;
#else
OverWriteGroupCB(
        Widget w,
	XtPointer client_data,
        XtPointer call_data )
#endif /* _NO_PROTO */
{
char  *user_group, *system_group, *fgdir;
XmStringTable  selectedItem;
int count_groups = 0;

       if(RemoveGroup(fgrp) == False) {
            InfoDialog(CANT_DELETE, style.fontDialog, False);
	    return;
       }


       XtVaGetValues(font.addPropList,
                           XmNselectedItems, &selectedItem,
                           NULL);
       XmStringGetLtoR(selectedItem[0],
                        XmFONTLIST_DEFAULT_TAG, &system_group);
 
       XtVaGetValues(font.addMonoList,
                           XmNselectedItems, &selectedItem,
                           NULL);
 
       XmStringGetLtoR(selectedItem[0],
                                XmFONTLIST_DEFAULT_TAG, &user_group);
 
       fgdir = AddFontGroup(name, fgrp);
       /* add the group to the linked list of groups */
       ScanFontGroups(home_fontGroup_dir, &count_groups);
       /* write the font names and alias names to the fonts.alias */
       WriteFontInfo(user_group, system_group, fgdir);

       XmListSelectItem(font.groupList, XmStringCreateSimple(fgrp), True);


	
       XtFree(system_group);
       XtFree(user_group);
}
 

static void
#ifdef _NO_PROTO
AddGroupToList( newGroup )
        char *newGroup ;
#else
AddGroupToList(
        char *newGroup )
#endif /* _NO_PROTO */
{
    int u_bound, l_bound = 0;
    char *text;
    XmString string, *strlist;

/*
**  Add the Group Name to the list in alphabetical order.
*/

    string = CMPSTR(newGroup);
 
    XtVaGetValues(font.groupList,
        XmNitemCount, &u_bound,
        XmNitems,     &strlist,
        NULL);

    u_bound--;
 
    while (u_bound >= l_bound) {
        int i = l_bound + (u_bound - l_bound)/2;
 
        if (!XmStringGetLtoR(strlist[i], XmFONTLIST_DEFAULT_TAG, &text))
            break;
        if (strcmp(text, newGroup) > 0)
            u_bound = i-1;
        else
            l_bound = i+1;
        XtFree(text); 
    }

    XmListAddItem(font.groupList, string, l_bound + 1);
    XmListSetBottomItem(font.groupList, string);

    XmStringFree(string);
}


void
#ifdef _NO_PROTO
set_attribute_fields()
#else
set_attribute_fields(void)
#endif /* _NO_PROTO */
{
    XmStringTable  selectedItem;
    fontInfo *font_info = NULL, *mono_font, *prop_font, *bold_font;
    FgroupRec    *fgroup;
    char 	*group_name;
    XmString	string;
    int 	*position_list, position_count;
    int 	items_selected = 0;
    char	buf[2000];
    Boolean     nobold = False;
    char	tmpStr[MAXNAMLEN];
    char     **XLFDNameFields, *freeMe;

if(attr_dlg != NULL) {
    XtVaGetValues(font.groupList,
                           XmNselectedItems, &selectedItem,
                           XmNselectedItemCount, &items_selected,
                           NULL);

    if(!items_selected)
	return;

    XtVaSetValues(font_group_name, 
			XmNlabelString, selectedItem[0],
			NULL);

    XmStringGetLtoR(selectedItem[0], XmFONTLIST_DEFAULT_TAG, &group_name);

    if((fgroup = find_group_selected(group_name, font_groups)) == NULL) {
        fprintf(stderr, "set_attribute_fields(): Font group %s not found ... \n", group_name);
	return;
    }

	mono_font = fgroup->mono_fonts;
	prop_font = fgroup->prop_fonts;
	bold_font = fgroup->bold_fonts;

    if(charset_count > 1 && strcmp(group_name, "Default")) {
        /*
         * select default items in add font group dialog
         */
     if(strcmp(option_selected, "mono") == 0) {
        while(mono_font) {
        if (splitXLFDName(mono_font->font_defn, &XLFDNameFields, &freeMe)) {
            sprintf(tmpStr, "%s-%s", XLFDNameFields[XLFD_REGISTRY],
                                        XLFDNameFields[XLFD_ENCODING]);
           XtFree((char *)XLFDNameFields);
           XtFree(freeMe);
 
           if(!strcasecmp(tmpStr, charset_list[charset_selected]))
              break;
         }
            mono_font = mono_font->next;
        }
     }
     else {
	if(strcmp(option_selected, "prop") == 0) {
          while(prop_font) {
           if (splitXLFDName(prop_font->font_defn, &XLFDNameFields, &freeMe)) {
              sprintf(tmpStr, "%s-%s", XLFDNameFields[XLFD_REGISTRY],
                                        XLFDNameFields[XLFD_ENCODING]);
              XtFree((char *)XLFDNameFields);
              XtFree(freeMe);
 
              if(!strcasecmp(tmpStr, charset_list[charset_selected]))
               break;
           }
           prop_font = prop_font->next;
          }
	}else {
          while(bold_font) {
           if (splitXLFDName(bold_font->font_defn, &XLFDNameFields, &freeMe)) {
              sprintf(tmpStr, "%s-%s", XLFDNameFields[XLFD_REGISTRY],
                                        XLFDNameFields[XLFD_ENCODING]);
              XtFree((char *)XLFDNameFields);
              XtFree(freeMe);
 
              if(!strcasecmp(tmpStr, charset_list[charset_selected]))
               break;
           }
           bold_font = bold_font->next;
          }
         }
      }
    }

    XtVaGetValues(font.sizeList,
                           XmNselectedItems, &selectedItem,
                           NULL);

    XtVaSetValues(font_size_val, 
			XmNlabelString, selectedItem[0],
			NULL);

    XtVaSetValues(alias_location_name, 
			XmNvalue, fgroup->location,
			NULL);

    XmListGetSelectedPos(font.sizeList, &position_list, &position_count);

    
    if(strcmp(option_selected, "mono") == 0)
       font_info = find_size_selected(mono_font, position_list[0]); 
    else{
      if(strcmp(option_selected, "prop") == 0)
          font_info = find_size_selected(prop_font, position_list[0]); 
      else {
       if(strcmp(option_selected, "bold") == 0) 
          font_info = find_size_selected(bold_font, position_list[0]); 

       if(!font_info) {
	  fgroup = find_group_selected("Default", font_groups);
          font_info = find_size_selected(fgroup->bold_fonts, position_list[0]); 
          nobold = True;
       }
      }
	 
    }
    

    if((strcmp(group_name, "Default") == 0) ||  nobold){
           XtUnmanageChild(font_xlfd_name);
           XtUnmanageChild(font_xlfd_label);
           XtUnmanageChild(font_name);
           XtUnmanageChild(font_label);
           XtUnmanageChild(alias_location_name);
           XtUnmanageChild(alias_location_label);
    }
    else {
	   XtManageChild(font_xlfd_name);
           XtManageChild(font_xlfd_label);
           XtManageChild(font_name);
           XtManageChild(font_label);
           XtManageChild(alias_location_name);
           XtManageChild(alias_location_label);
    }

    if(!font_info) {
        string = CMPSTR(ALIAS_NOT_FOUND);
        XtVaSetValues(alias_name,
                        XmNlabelString, string,
                        NULL);
        XmStringFree(string);

        string = XmStringCreateLocalized (" ");
        XtVaSetValues(alias_xlfd_name,
                        XmNvalue, string,
                        NULL);
        XtVaSetValues(font_name,
                        XmNlabelString, string,
                        NULL);
        XtVaSetValues(font_xlfd_name,
                        XmNvalue, string,
                        NULL);
        XtVaSetValues(alias_location_name,
                        XmNvalue, string,
                        NULL);
        XmStringFree(string);
    }
    else {
    
    strcpy(buf, font_info->alias_hrn);
    set_read_only_str(buf);

    string = XmStringCreateLocalized (buf);

    XtVaSetValues(alias_name, 
			XmNlabelString, string,
			NULL);
    XtVaSetValues(alias_xlfd_name, 
			XmNvalue, font_info->alias_name,
			NULL);

    XmStringFree(string);

    strcpy(buf, font_info->font_hrn);
    set_read_only_str(buf);

    string = XmStringCreateLocalized (buf);

    XtVaSetValues(font_name, 
			XmNlabelString, string,
			NULL);

	
    XtVaSetValues(font_xlfd_name, 
			XmNvalue, font_info->font_defn,
			NULL);
    XmStringFree(string);
    }

    XtFree(group_name);
}

}


static fontInfo *
#ifdef _NO_PROTO
find_size_selected(fontElement, sc)
	fontInfo *fontElement;
        ScaleType sc ;
#else
find_size_selected(
	fontInfo *fontElement,
        ScaleType sc)
#endif /* _NO_PROTO */
{
   char     buf[10];
   fontInfo *temp_fi;
 
   strcpy(buf, SizeStr[sc - 1]); 

   temp_fi = fontElement;
   while(temp_fi)
     if((strcmp(temp_fi->size,buf)) == 0)
       break;
     else
       temp_fi = temp_fi->next;
 
        return (temp_fi);
}

static FgroupRec *
#ifdef _NO_PROTO
find_group_selected(group_name, font_group)
        char *group_name;
	FgroupRec *font_group;
#else
find_group_selected(
        char *group_name,
	FgroupRec *font_group)
#endif /* _NO_PROTO */
{

   /* find group element */
   while(font_group) {
     if((strcmp(font_group->fgroup_name,group_name)) == 0)
       break;
     else
       font_group = font_group->next;
   }

   return (font_group);
}

static FgroupRec *
#ifdef _NO_PROTO
find_group_prev(group_name, font_group)
        char *group_name;
        FgroupRec *font_group;
#else
find_group_prev(
        char *group_name,
        FgroupRec *font_group)
#endif /* _NO_PROTO */
{
FgroupRec *prev_ptr = NULL;
 
   /* find group element */
   while(font_group)
     if((strcmp(font_group->fgroup_name,group_name)) == 0)
       break;
     else {
       prev_ptr = font_group;
       font_group= font_group->next;
     }

   return (prev_ptr);
}


static void
#ifdef _NO_PROTO
dlg_event_handler( w, client_data, event, dispatch )
        Widget w ;
        XtPointer client_data ;
        XEvent    *event ;
	Boolean   *dispatch;
#else
dlg_event_handler(
        Widget w,
        XtPointer client_data,
        XEvent    *event,
	Boolean   *dispatch )
#endif /* _NO_PROTO */
{
Position x,y;
Position x1,y1;

   if (event->type == MapNotify) {
         XtVaGetValues(XtParent(style.fontDialog), XmNx, &x, XmNy, &y, NULL);
         XtVaGetValues(font.prevDrawWindow, XmNx, &x1, XmNy, &y1, NULL);
         XtVaSetValues(prev_dialog, XmNx, x1+x+40, XmNy, y1+y+45, NULL);
         first_time = 0;
         XtManageChild(prev_pane);
   }
   else if (event->type == UnmapNotify) 
         XtUnmanageChild(prev_pane);
        else {  
         if(!first_time) {
           XtVaGetValues(XtParent(style.fontDialog), XmNx, &x, XmNy, &y, NULL);
           XtVaGetValues(font.prevDrawWindow, XmNx, &x1, XmNy, &y1, NULL);
           XtVaSetValues(prev_dialog, XmNx, x1+x+40, XmNy, y1+y+45, NULL);
 	 }
        }
         raiseWindow(XtWindow(prev_dialog));
}

/*+++++++++++++++++++++++++++++++++++++++*/
/* do_preview                             */
/*+++++++++++++++++++++++++++++++++++++++*/
static void
#ifdef _NO_PROTO
do_preview(w)
        Widget w; 
#else
do_preview(
        Widget w)
#endif /* _NO_PROTO */
{
XmString string;

  if(prev_dialog == NULL) {

    first_time = 0;


    prev_dialog = XtVaCreatePopupShell("prev_dialog",
        xmDialogShellWidgetClass, w,
        XmNtitle, PREV_MSG,
        XmNmwmFunctions, 0,
        NULL);

    prev_pane = XtVaCreateWidget("prev_pane", xmFormWidgetClass, prev_dialog,
	   XmNwidth,  PREVIEW_WIN_WIDTH - 25,
           XmNheight, PREVIEW_WIN_HEIGHT - 40,
	   XmNautoUnmanage, False,
	   NULL);

    string = CMPSTR(SYSTEM_MSG);

    prev_label = XtVaCreateWidget("label", xmLabelGadgetClass, prev_pane,
        XmNleftAttachment, XmATTACH_FORM,
        XmNtopAttachment,  XmATTACH_FORM,
        XmNlabelString, string,
        XmNtopOffset, 2 * style.verticalSpacing,
    	XmNleftOffset,         style.horizontalSpacing,
        NULL);

    XmStringFree(string);

    string = CMPSTR(USER_MSG);

    prev_text = XtVaCreateWidget("text_w",
        xmTextFieldWidgetClass, prev_pane,
        XmNtraversalOn,      True,
        XmNtopAttachment,    XmATTACH_WIDGET,
        XmNtopWidget,  	     prev_label,
        XmNtopOffset,        2 * style.verticalSpacing,
        XmNlabelString,      string,
        XmNcolumns, 	     26,
        XmNleftAttachment,   XmATTACH_FORM,
	XmNvalue,	     USER_MSG,
        XmNbottomOffset,     2 * style.verticalSpacing,
    	XmNleftOffset,         style.horizontalSpacing,
        NULL);

    XmStringFree(string);

    string = CMPSTR(PUSH_BUTTON);

    prev_push_button = XtVaCreateWidget("push_button",
        xmPushButtonWidgetClass, prev_pane, 
        XmNtopAttachment,    XmATTACH_WIDGET,
        XmNtopWidget,  	     prev_text,
        XmNlabelString,      string,
        XmNrightAttachment,  XmATTACH_NONE,
        XmNleftAttachment,   XmATTACH_FORM,
        XmNtopOffset,        2 * style.verticalSpacing,
    	XmNleftOffset,         style.horizontalSpacing,
	NULL);

    XmStringFree(string);
    set_font_preview();
 
  }
  XtManageChild (prev_label); 
  XtManageChild (prev_text); 
  XtManageChild (prev_push_button); 
  XtManageChild (prev_pane); 
  XtPopup (prev_dialog, XtGrabNone);

}


void
#ifdef _NO_PROTO
set_font_preview()
#else
set_font_preview(void)
#endif /* _NO_PROTO */
{
    XmStringTable  selectedItem;
    char        *group_name;
    int         *position_list, position_count;
    Arg args[10];
    int n = 0, items_selected = 0;
    XmFontList  fontlist_mono, fontlist_prop, fontlist_bold;
    int       selected_pos;
    XFontSet	     xfontset;
    char	     *def_str;
    XmFontListEntry fl_entry;
    char                 **ch_list;
    int                  ch_count;
    char              tmp_buf[512];




      XtVaGetValues(font.groupList,
                           XmNselectedItems, &selectedItem,
                           XmNselectedItemCount, &items_selected,
                           NULL);
 
      if(!items_selected)
         return;

      XmStringGetLtoR(selectedItem[0], XmFONTLIST_DEFAULT_TAG, &group_name);

      XmListGetSelectedPos(font.sizeList, &position_list, &position_count);

      if(position_count > 0)
        selected_pos = position_list[0] - 1;
      else
	/* this should never happen, if it does select 3rd item */
        selected_pos = 3;

      if(strcmp(group_name, "Default") == 0) {

         if (!style.xrdb.fontChoice[selected_pos].userFont)
           GetUserFontResource(selected_pos);
         if (!style.xrdb.fontChoice[selected_pos].sysFont)
           GetSysFontResource(selected_pos);
         if(!style.xrdb.fontChoice[selected_pos].userBoldFont)
           GetUserBoldFontResource(selected_pos);
 
        /* set system font for preview */

    	 n = 0;
    	 XtSetArg(args[n], XmNfontList, 
			style.xrdb.fontChoice[selected_pos].sysFont); n++;
    	 XtSetValues (prev_label, args, n);

         n = 0;
         XtSetArg(args[n], XmNfontList, 
			style.xrdb.fontChoice[selected_pos].userFont); n++;
         XtSetValues (prev_text, args, n);

         XmTextShowPosition(prev_text, 0);

         n = 0;
         XtSetArg(args[n], XmNfontList, 
			style.xrdb.fontChoice[selected_pos].userBoldFont); n++;
         XtSetValues (prev_push_button, args, n);

      }else {

	/* set user font for preview*/
        strcpy(tmp_buf, style.xrdb.fontChoice[selected_pos].userStr);
        xfontset = XCreateFontSet (style.display, 
			strtok(tmp_buf, ":"),
			&ch_list, &ch_count, &def_str );

        if (xfontset)
          fl_entry = XmFontListEntryCreate ( XmFONTLIST_DEFAULT_TAG,
                                             XmFONT_IS_FONTSET, xfontset );

	fontlist_mono = XmFontListAppendEntry ( NULL, fl_entry );
	

        n = 0;
        XtSetArg(args[n], XmNfontList, fontlist_mono); n++;
        XtSetValues(prev_text, args, n);
        XmTextShowPosition(prev_text, 0);

        XmFontListEntryFree(&fl_entry);
        XmFontListFree(fontlist_mono);

	/* set system font for preview */
        strcpy(tmp_buf, style.xrdb.fontChoice[selected_pos].sysStr);
        xfontset = XCreateFontSet (style.display,
			strtok(tmp_buf, ":"),
                        &ch_list, &ch_count, &def_str );

        if (xfontset)
          fl_entry = XmFontListEntryCreate ( XmFONTLIST_DEFAULT_TAG,
                                             XmFONT_IS_FONTSET, xfontset );
 
        fontlist_prop = XmFontListAppendEntry ( NULL, fl_entry );

        n = 0;
        XtSetArg(args[n], XmNfontList, fontlist_prop); n++;
        XtSetValues(prev_label, args, n);

        XmFontListEntryFree(&fl_entry);
        XmFontListFree(fontlist_prop);

	/* set user bold font on pushbutton in preview */
        strcpy(tmp_buf, style.xrdb.fontChoice[selected_pos].userBoldStr);
        xfontset = XCreateFontSet (style.display,
			strtok(tmp_buf, ":"),
                        &ch_list, &ch_count, &def_str );

        if (xfontset)
          fl_entry = XmFontListEntryCreate ( XmFONTLIST_DEFAULT_TAG,
                                             XmFONT_IS_FONTSET, xfontset );
 
        fontlist_bold = XmFontListAppendEntry ( NULL, fl_entry );

        n = 0;
        XtSetArg(args[n], XmNfontList, fontlist_bold); n++;
        XtSetValues(prev_push_button, args, n);

        XmFontListEntryFree(&fl_entry);
        XmFontListFree(fontlist_bold);

      }

  XtFree(group_name);

}
 
static int
#ifdef _NO_PROTO
SizeStrToPixelSize(sz_string)
char *sz_string;
#else
SizeStrToPixelSize(char *sz_string)
#endif /* _NO_PROTO */
{
 
           if(strcmp(sz_string, "xxs") == 0)
		return(8);
           if(strcmp(sz_string, "xs") == 0)
		return(10);
           if(strcmp(sz_string, "s") == 0)
		return(12);
           if(strcmp(sz_string, "m") == 0)
		return(14);
           if(strcmp(sz_string, "l") == 0)
		return(17);
           if(strcmp(sz_string, "xl") == 0)
		return(20);
           if(strcmp(sz_string, "xxl") == 0)
		return(24);
           if(strcmp(sz_string, "scalable") == 0)
		return(0);
}


static char *
#ifdef _NO_PROTO
FindFontGroup()
#else
FindFontGroup( void )
#endif /* _NO_PROTO */
{
DIR *dirp;
struct dirent *dp;
DIR *group_dir;
struct dirent *font_group_dir;
char buf[MAXPATHLEN];
char **fgdirs;
char path[MAXPATHLEN];
char fgdir[MAXPATHLEN];
char *name;
char *group_name;
char Fgroup[MAXPATHLEN];
char fgrp_loc[MAXNAMLEN];
FILE *fp;
int found_loc = -1;
int i;
 
 
         /*
          * if it is Default, don't do anything. It is already in the
          * fontpath.
          */
        if(strcmp("Default", style.xrdb.fontGroup) == 0)
                return ((char *) NULL);
 
        fgdirs = (char **) XtMalloc(sizeof(char *) * 3);
 
        fgdirs[0] = (char *)XtMalloc(strlen(DEFAULT_FONT_PATH)
                                                + strlen(style.lang) + 2);
        sprintf(fgdirs[0], "%s%s/", DEFAULT_FONT_PATH, style.lang);
 
        fgdirs[1] = (char *)XtMalloc(strlen(SYS_FONT_PATH)
                                                + strlen(style.lang) + 2);
 
        sprintf(fgdirs[1], "%s%s/", SYS_FONT_PATH, style.lang);
 
        fgdirs[2] = (char *)XtMalloc(strlen(style.home)
                        + strlen(USER_FONT_DIR) + strlen(style.lang) + 2);
        sprintf(fgdirs[2], "%s%s%s/", style.home, USER_FONT_DIR, style.lang);
        for(i = 0 ; i < 3 ; ++i) {
 
          if((dirp = opendir(fgdirs[i])) == NULL)
                continue;
 
          while((dp = readdir(dirp)) != NULL) {
             name = dp->d_name;
             if ((strcmp(name, ".") == 0) || (strcmp(name, "..") == 0))
                    continue;
 
             sprintf(fgdir, "%s%s", fgdirs[i], name);
 
             if((group_dir = opendir(fgdir)) == NULL)
                    continue;
 
             while ((font_group_dir = readdir(group_dir)) != NULL) {
              group_name = font_group_dir->d_name;
 
              if ((strcmp(group_name, ".") == 0) ||
                                       (strcmp(group_name, "..") == 0))
                    continue;
 
              sprintf(path, "%s%s/%s", fgdirs[i], name, "sdtfonts.group");
 
              if ((fp = fopen(path, "r")) == NULL)
                   break;     /* go to next directory */
              else  {
                if(fgets(Fgroup, MAXNAMLEN, fp)) {
                   Fgroup[strlen(Fgroup) - 1] = '\0';
                   if(strcmp(Fgroup, font.originalGroupStr) == 0) {
                        found_loc = i;
                        strcpy(fgrp_loc, name);
                        break;
                   }
                }
                fclose(fp);
              }
 
            } /* while */
            closedir(group_dir);
 
          } /* while */
          closedir(dirp);
        }  /* for */
 
 
        if(found_loc != -1) {
           sprintf(buf, "%s%s", fgdirs[found_loc], fgrp_loc);
           XtFree((char *) *fgdirs);
           XtFree((char *) fgdirs);

#ifdef DEBUG
        fprintf(stderr,"Font group is %s \n", buf);
#endif
	   return(strdup(buf));
        }
	else
	   return ((char *) NULL);

 
 
}

static void
#ifdef _NO_PROTO
rem_fgrp_from_path(fgrp)
char *fgrp;
#else
rem_fgrp_from_path(char *fgrp)
#endif /* _NO_PROTO */
{
    int ndirs = 0;
    char **currentList = NULL; int ncurrent = 0;
    int i, j;
    int nnew = 0;
    char **newList;

    currentList = XGetFontPath (style.display, &ncurrent);

    if (!currentList) {
        fprintf (stderr, "%s:  unable to get current font path.\n", progName);
        return;
    }

    newList = (char **) malloc (ncurrent * sizeof (char *));
 
    if (!newList)         
		fprintf(stderr, "Cannot allocate memory\n");

        for (i = 0; i < ncurrent; i++) {
            if (strcmp (currentList[i], fgrp) == 0)
	       continue;

            newList[nnew++] = currentList[i];
        }

        XSetFontPath (style.display, newList, nnew);
        free ((char *) newList);

    if (currentList) XFreeFontPath (currentList);

}

static void
#ifdef _NO_PROTO
set_fgrp_to_path(fgrp)
char *fgrp;
#else
set_fgrp_to_path(char *fgrp)
#endif /* _NO_PROTO */
{
char **directoryList = NULL;
char **newList = NULL;
char **currentList = NULL;
int ncurrent = 0;
int ndirs = 1;

    directoryList = (char **) malloc (ndirs*sizeof (char *));

    if (!directoryList)  {
	fprintf (stderr, "out of memory for font path directory list");
	return;
    }
    directoryList[0] = fgrp;
 
    currentList = XGetFontPath (style.display, &ncurrent);
    if (!currentList) {
        fprintf (stderr, "%s:  unable to get current font path.\n", progName);
        return;
    }

    newList = (char **) malloc ((ncurrent + ndirs) * sizeof (char *));
 
    memcpy ((char *) newList, (char *) directoryList,
                   (unsigned) (ndirs*sizeof (char *)));
 
    memcpy ((char *) (newList + ndirs), (char *) currentList,
                        (unsigned) (ncurrent*sizeof (char *)));

    XSetFontPath (style.display, newList, ndirs + ncurrent);
 
 
    free ((char *) newList);
    free ((char *) directoryList);

    if (currentList) XFreeFontPath (currentList);

}

/*++++++++++++++++++++++++++++++++++++++++++++++*/
/* prepend_iso8859                       	*/
/*++++++++++++++++++++++++++++++++++++++++++++++*/

static void
#ifdef _NO_PROTO
prepend_iso8859()
#else
prepend_iso8859()
#endif /* _NO_PROTO */
{
Boolean iso8859_found = False;
int     i = 0;
char    buf[MAXNAMLEN];

     for(i=0; i < charset_count; ++i) {
        if(strncasecmp(charset_list[i], "iso8859", strlen("iso8859")) == 0) {
	  iso8859_found = True;
          break;
	}
     }

     if(!iso8859_found)  {
          sprintf(buf,"%s %s %s %s %s %s", 
		       XSET,
                       "-fp",
                       "/usr/openwin/lib/X11/fonts/F3/,/usr/openwin/lib/X11/fonts/F3bitmaps/",
                       "+fp",
                       "/usr/openwin/lib/X11/fonts/F3/,/usr/openwin/lib/X11/fonts/F3bitmaps/",
                       "> /dev/null 2>/dev/null");
          system(buf);
     }

}
