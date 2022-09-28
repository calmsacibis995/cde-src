#include <dirent.h>
#include <libgen.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dlfcn.h>

#include <sys/mntent.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>
#include <Xm/MwmUtil.h>
#include <Xm/Protocols.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/RowColumn.h>
#include <Xm/ToggleB.h>
#include <Xm/LabelG.h>
#include <Xm/IconFile.h>

#include <Dt/Icon.h>
#include <Dt/HelpDialog.h>
#include <Dt/HelpQuickD.h>
#include <Dt/Dts.h>
#include <Dt/DtStrDefs.h>
#include <Dt/SharedProcs.h>
#include <Dt/Connect.h>

#include "sdtfprop.h"
#include "sdtfprop_ui.h"
#include "sdtfprop_uc.h"

/*
 * Define a sort table so that we can add the ACL entry to the list
 * accordingly.
 */ 
#define TOTAL_ENTRY_TYPES	9

struct sort_entry
{
    int order;
    int type;
};

static struct sort_entry sort_table[] =
{
    { 0, 0 },	/* UNUSED */
    { 1, CLASS_OBJ },
    { 2, DEF_CLASS_OBJ },
    { 3, USER },
    { 4, DEF_USER },
    { 5, DEF_USER_OBJ },
    { 6, GROUP },
    { 7, DEF_GROUP },
    { 8, DEF_GROUP_OBJ },
    { 9, DEF_OTHER_OBJ },
};

DtbSdtfpropFileSDialogInfoRec dtb_sdtfprop_fileseldialog =
    { False };

DtbSdtfpropRecurCDialogInfoRec dtb_sdtfprop_recurcdialog =
    { False };

DtbSdtfpropChangeCDialogInfoRec dtb_sdtfprop_changecdialog =
    { False };

DtbSdtfpropHelpDialogInfoRec dtb_sdtfprop_helpdialog =
    { False };

ProgramInfo proginfo =
    { True, False, False, False, False, False, True, True,
      0, PERM_PANE, 0, 0, 0, 1,
      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
      { 7, False, False, False, False, NULL } };

int (*acl_fptr)( char *, int, int, aclent_t* );
int (*aclcheck_fptr)( aclent_t*, int, int* );
int (*aclsort_fptr)( int, int, aclent_t* );


void
Usage()
{
    fprintf( stderr, catgets( Dtb_project_catd, 3, 49, "Usage: %s %s" ),
        "sdtfprop",
        "[-v info | perm] [-s filename] [filename]\n" );

    exit( -1 );
}


char *
ParseArgs(
    int argc,
    char *argv[] )
{
    Boolean	err_flag = False;
    char	*file_name = NULL;
    char	*option;
    char	*option_str;

    /* Process options */
    while ( ( --argc > 0 ) && ( (*++argv)[0] == '-' ) )
    {
        option = *argv;

        if ( !strcmp( option, "-v" ) )		/* View mode */
        {
            option_str = *++argv; argc--;

            if ( option_str == NULL )
            {
                err_flag = True;
                break;
            }

            if ( strncmp( option_str, "information", strlen(option_str) ) == 0 )
            {
                proginfo.current_pane = INFO_PANE;
            }
            else
            if ( strncmp( option_str, "permissions", strlen(option_str) ) == 0 )
            {
                proginfo.current_pane = PERM_PANE;
            }
            else
            {
                err_flag = True;
                break;
            }
        }
        else
        if ( !strcmp( option, "-s" ) )		/* Single file mode */
        {
            proginfo.is_browsable = False;

            option_str = *++argv; argc--;

            if ( option_str == NULL )
            {
                err_flag = True;
                break;
            }

            file_name = option_str;
        }
        else					/* Invalid option */
        {
            err_flag = True;
            break;
        }
    }

    if ( err_flag )
        Usage();

    /*
     * If the user has not specified the file name through the '-s' option, and
     * there are args left on the command line then assume the 1st arg is the
     * file name.
     */
    if ( ( argc > 0 ) && ( file_name == NULL ) )
        file_name = *argv;

    return( file_name );
}


void
CreateApplicationIcon()
{
    static Pixmap	app_icon = XmUNSPECIFIED_PIXMAP;
    static Pixmap	app_mask = XmUNSPECIFIED_PIXMAP;
    Pixel		bg, fg, top_shadow, bottom_shadow, select;
    Colormap		cmap;
    char		*icon_file_name;

    if ( app_icon == XmUNSPECIFIED_PIXMAP )
    {
        icon_file_name = XmGetIconFileName( proginfo.screen,
                                            NULL,
                                            SDTFPROP_ICON,
                                            NULL,
                                            XmLARGE_ICON_SIZE );

        if ( !icon_file_name )
            return;

        XtVaGetValues( dtb_sdtfprop_propdialog.propdialog_mainwin,
            XmNbackground, &bg,
            XmNcolormap, &cmap,
            NULL );
      
        XmGetColors( proginfo.screen,
            cmap, bg, &fg, &top_shadow, &bottom_shadow, &select );

        /* Get the application's icon and mask */
        app_icon = XmGetPixmap( proginfo.screen,
            icon_file_name, fg, bg );

        /* now let's get the mask for the File Manager */
        app_mask = _DtGetMask( proginfo.screen,
            icon_file_name );

        DtDtsFreeAttributeValue( icon_file_name ); 

        if ( app_icon != XmUNSPECIFIED_PIXMAP )
        {
            XtVaSetValues( proginfo.topLevel,
                XmNiconPixmap, app_icon,
                NULL );
        }

        if ( app_mask != XmUNSPECIFIED_PIXMAP )
        {
            XtVaSetValues( proginfo.topLevel,
                XmNiconMask, app_mask,
                NULL );
        }
    }

    /* Set icon name */
    XtVaSetValues( proginfo.topLevel,
        XmNiconName, proginfo.name,
        NULL );
}


/*
 * Set input focus explicitly to the specified widget. 
 */

void
SetFocus(
    Widget widget )
{
    XmProcessTraversal( widget, XmTRAVERSE_CURRENT );
}


void
InitRun(
    char *file_name )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    Widget widget;

    /*
     * Load the File info.  In case of error, just bring up the window.
     */
    if ( file_name != NULL )
    {
        if ( ( LoadNewFile( file_name ) != 0 ) && !proginfo.is_browsable ) 
            exit( proginfo.saved_errno );
    }
    else
    {
        if ( !proginfo.is_browsable )
            ProcessErrorMsg( FATAL, False, catgets( Dtb_project_catd, 4, 2, "Unable to load file properties.\n\nFile name was not specified." ) );
    }

    /*
     * Set the Category type
     */
    switch ( proginfo.current_pane )
    {
        case INFO_PANE:
            widget = 
                instance->categoryRbox_items.Information_item;
            break;

        case PERM_PANE:
            widget = 
                instance->categoryRbox_items.Permissions_item;
            break;
    }
    XmToggleButtonSetState( widget, True, True );

    /*
     * Set focus on the fileNameTfield.  We have to use XtAppAddTimeOut()
     * because this is the only way to do this when the topLevel widget
     * has not been realized.
     */
    XtAppAddTimeOut(
        XtWidgetToApplicationContext( proginfo.topLevel ),
        0, (XtTimerCallbackProc)SetFocus, instance->fileNameTfield );
}


/*
 * Clean up routine before closing.
 */

void
ShutdownProperties()
{
    if ( proginfo.appContext != NULL )
    {
        XtDestroyApplicationContext( proginfo.appContext );
        proginfo.appContext = NULL;
    }
    if ( proginfo.appContext != NULL )
    {
        XtCloseDisplay( proginfo.display );
        proginfo.display = NULL;
    }

    catclose( Dtb_project_catd );
}


void
ModDialogAddOK_CB(
    Widget	widget,
    int		position,
    XtPointer	callData )
{
    DtbSdtfpropModdialogInfo	instance = &dtb_sdtfprop_moddialog;

    Boolean		is_default_entry = False;
    aclent_t		*aclentry;
    ListItemRec		*curr = proginfo.list.items;

    /* Get the new ACL entry */
    if ( ( aclentry = GetModDialogValues() ) == NULL )
        return;		/* Let user re-enter the data */

    /* Check for duplicate entry */
    while ( curr != NULL )
    {
        if ( ( aclentry->a_type == (curr->acl_entry).a_type ) &&
             ( aclentry->a_id == (curr->acl_entry).a_id ) )
        {
            ProcessErrorMsg( ERROR, False,
                catgets( Dtb_project_catd, 4, 24, "Duplicate entries are not allowed.\n\nThe specified ACL entry already exists." ) );

            XtFree( (char *)aclentry );
            return; 
        }

        curr = curr->next;
    }

    /* Check to see if we are adding a default ACL entry */
    switch ( aclentry->a_type )
    {
        case DEF_USER: 
        case DEF_GROUP: 
            is_default_entry = True;
            break;

        case DEF_USER_OBJ:
            is_default_entry = proginfo.list.has_def_user_obj = True;
            break;

        case DEF_GROUP_OBJ:
            is_default_entry = proginfo.list.has_def_group_obj = True;
            break;

        case DEF_OTHER_OBJ:
            is_default_entry = proginfo.list.has_def_other_obj = True;
            break;

        case DEF_CLASS_OBJ:
            is_default_entry = proginfo.list.has_def_class_obj = True;
            break;
    }

    /*
     * If we are adding a default ACL entry, then we must add the required
     * default ACL entries to the list first.
     */
    if ( is_default_entry )
    {
        aclent_t entry;

        if ( !proginfo.list.has_def_user_obj )
        {
            proginfo.list.has_def_user_obj = True;

            entry.a_type = DEF_USER_OBJ;
            entry.a_id = -1;
            entry.a_perm = 7;	/* Give rwx access for owner only */

            proginfo.list.items =
                AddListItemsEntry( proginfo.list.items, entry, False );
        }
        if ( !proginfo.list.has_def_group_obj )
        {
            proginfo.list.has_def_group_obj = True;

            entry.a_type = DEF_GROUP_OBJ;
            entry.a_id = -1;
            entry.a_perm = 0;

            proginfo.list.items =
                AddListItemsEntry( proginfo.list.items, entry, False );
        }
        if ( !proginfo.list.has_def_other_obj )
        {
            proginfo.list.has_def_other_obj = True;

            entry.a_type = DEF_OTHER_OBJ;
            entry.a_id = -1;
            entry.a_perm = 0;

            proginfo.list.items =
                AddListItemsEntry( proginfo.list.items, entry, False );
        }
        if ( !proginfo.list.has_def_class_obj )
        {
            proginfo.list.has_def_class_obj = True;

            entry.a_type = DEF_CLASS_OBJ;
            entry.a_id = -1;
            entry.a_perm = 0;

            proginfo.list.items =
                AddListItemsEntry( proginfo.list.items, entry, False );
        }
    }

    /* Add the entry to the list */
    proginfo.list.items =
        AddListItemsEntry( proginfo.list.items, *aclentry, True );

    XtFree( (char *)aclentry );
    XtUnmanageChild( instance->moddialog_shellform );
}


void
ModDialogAddHelp_CB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData )
{
    ShowHelpDialog( HELP_VOLUME, HELP_ADDACLDIALOG_ID );
}


void
ModDialogChangeOK_CB(
    Widget	widget,
    int		position,
    XtPointer	callData )
{
    DtbSdtfpropModdialogInfo	instance = &dtb_sdtfprop_moddialog;

    register int	i;
    Boolean		change_perm_only = False;
    aclent_t		orig_aclentry = { 0, 0, 0 };
    aclent_t		*aclentry;
    ListItemRec 	*curr = proginfo.list.items;

    /* Get the new ACL entry */
    if ( ( aclentry = GetModDialogValues() ) == NULL )
        return;         /* Let user re-enter the data */

    /* Find the original ACL entry */
    for ( i = 1 ; i < position ; i++ )
    {
        if ( curr->next != NULL )
            curr = curr->next;		/* Keep going... */
    }
    orig_aclentry.a_type = (curr->acl_entry).a_type;
    orig_aclentry.a_id   = (curr->acl_entry).a_id;
    orig_aclentry.a_perm = (curr->acl_entry).a_perm;

    /*
     * We are changing the permissions for the ACL entry only if we are
     * unable to dermine the ACL type (i.e. a_type is set to '0').  If
     * this is the case, then the ACL type is the same as the original ACL
     * entry.
     */
    if ( aclentry->a_type == 0 )
    {
        change_perm_only = True;
        aclentry->a_type = orig_aclentry.a_type;
    }

    if ( ( aclentry->a_type == orig_aclentry.a_type ) && 
         ( aclentry->a_id == orig_aclentry.a_id ) ) 
        change_perm_only = True;

    if ( change_perm_only )
    {
        /*
         * We are changing the permissions for the ACL entry only, and the
         * permissions are the same so we do nothing.
         */
        if ( aclentry->a_perm != orig_aclentry.a_perm )
            proginfo.list.items =
               ChangeListItemsEntry( proginfo.list.items, *aclentry, position );
    }
    else
    {
        proginfo.list.items =
            DeleteListItemsEntry( proginfo.list.items, position );
        proginfo.list.items =
            AddListItemsEntry( proginfo.list.items, *aclentry, True );
    }

    XtFree( (char *)aclentry );
    XtUnmanageChild( instance->moddialog_shellform );
}


void
ModDialogChangeHelp_CB(
    Widget	widget,
    XtPointer	clientData,
    XtPointer	callData )
{
    ShowHelpDialog( HELP_VOLUME, HELP_CHANGEACLDIALOG_ID );
}


void
FileNameChanged_CB(
    Widget widget,
    XtPointer clientData,
    XtPointer callData )
{
    String file_name;
    String full_path;

    _DtTurnOnHourGlass( proginfo.topLevel );

    file_name = (String)XmTextFieldGetString( widget );

    if ( *file_name && ( strlen( file_name ) > 0 ) )
    {
        full_path = ProcessPathFromInput( proginfo.directory, file_name );

        if ( full_path != NULL )
        { 
            if ( strcmp( full_path, file_name ) )
            {
                XmTextFieldSetString( widget, full_path );
                XmTextFieldSetInsertionPosition( widget, MAXPATHLEN );
            }

            XtFree( file_name );
            file_name = full_path;
        }

        LoadNewFile( file_name );
    }
    else
        ProcessErrorMsg( ERROR, False,
            catgets( Dtb_project_catd, 4, 3, "Please specify file name." ) );

    XtFree( file_name );

    _DtTurnOffHourGlass( proginfo.topLevel );
}


/*
 * This function search the given string for the occurence of any characters
 * in the string set src and replace it with the string dest.  Caller is
 * responsible for using XtFree to free the return string.
 */

String
SearchAndReplace(
    char *str,
    char *src,
    char *dest )
{
    char *word;
    char work_str[BUF_SIZ];
    char new_str[BUF_SIZ];

    if ( src == NULL )
        return( ( char * )NULL );

    new_str[0] = '\0';

    strcpy( work_str, str );                  /* Make a copy of the string */

    word = strtok( work_str, src );           /* Find the 1st occurence */
    while ( word != NULL )
    {
        strcat( new_str, word );
        if ( dest != NULL )
            strcat( new_str, dest );
        word = strtok( NULL, src );           /* Find the next occurence */
    }

    /* Return the newly created string or the original string */
    if ( strlen( new_str ) > 0 )
        return( XtNewString( new_str ) );
    else
        return( XtNewString( str ) );
}


/*
 * This function takes a path name, and resolves any leading '~' to refer
 * to one of the following:
 *
 *   1) if ~ or ~/path, then it resolves to the user's home directory.
 *
 *   2) if ~user or ~user/path, then it resolves to the specified user's
 *      home directory.
 */

String
ChangeTildeToHome(
    char *input_string )
{
    char		full_path[MAXPATHLEN];
    char		*path;
    struct passwd	*passwdbufp;

    if ( ( input_string[1] != '\0' ) && ( input_string[1] != '/' ) )
    {
        /* ~user or ~user/path format */

        /* Is there a path? */
        if ( ( path = strchr( input_string, '/' ) ) != NULL )
            *path = '\0';

        /* Get specified user home directory */
        if ( ( passwdbufp = getpwnam( input_string + 1 ) ) == NULL )
        {
            /* Specified user doesn't exist */
            if ( path )
                *path = '/';

            return( NULL );
        }

        if ( path )
        {
            /* ~user/path format */

            *path = '/';

            if ( strcmp( passwdbufp->pw_dir, "/" ) == 0 )
            {
                /* We don't want to end up with double '/' in the path */
                strcpy( full_path, path );
            }
            else
            {
                full_path[0] = '\0';
                sprintf( full_path, "%s%s", passwdbufp->pw_dir, path );
            }
        }
        else
        {
            /* ~user format */

            strcpy( full_path, passwdbufp->pw_dir );
        }
    }
    else
    {
        /* Get user home directory */
        if ( ( passwdbufp = getpwuid( proginfo.user_id ) ) == NULL )
            return( NULL );

        if ( input_string[1] )
        {
            /* ~/path format */

            full_path[0] = '\0';
            sprintf( full_path, "%s%s", passwdbufp->pw_dir, input_string + 1 );
        }
        else
        {
           /* ~ format */

           strcpy( full_path, passwdbufp->pw_dir );
        }
    }
 
    return( XtNewString( full_path ) );
}


/*
 * This is a generic function for resolving a cannonical path from user input.
 */

String
ProcessPathFromInput(
    char *current_directory,
    char *input_string )
{
    FILE	*pfp;
    String	path;
    String	tmp_path;
    char	command[MAXPATHLEN];
    char	tmp_buf[MAXPATHLEN];

    if ( input_string == NULL )
        return( NULL );

    path = XtNewString( input_string );

    /* Strip any spaces from name -- input is overwritten */
    path = _DtStripSpaces( path );

    /* Resolve, if there're any, environement variables */
    command[0] = '\0';
    sprintf( command, "echo %s", path );

    if ( ( pfp = popen( command, "r" ) ) != NULL )
    {
        if ( fgets( tmp_buf, MAXPATHLEN, pfp ) != NULL )
        {
            tmp_buf[strlen( tmp_buf ) - 1] = '\0';
            XtFree( path );
            path = XtNewString( tmp_buf );
        }

        pclose( pfp );
    }

    /* Resolve '~' -- new memory is allocated, old memory is freed */
    if ( *path == '~' )
    {
        String new_path = ChangeTildeToHome( path );
        XtFree( path );
        path = new_path;
    }

    /* If current directory is provided, check for relative path */
    if ( ( path != NULL ) && ( current_directory != NULL ) )
    {
        if ( *path != '/' )
        {
            /* File contains relative path i.e. xyz/abc */
            tmp_buf[0] = '\0';
            if ( *( current_directory + strlen(current_directory) - 1 ) != '/' )
                sprintf( tmp_buf, "%s/%s", current_directory, path );
            else
                sprintf( tmp_buf, "%s%s", current_directory, path );

            XtFree( path );
            path = XtNewString( tmp_buf );
        } 
    }
    else if ( path == NULL )
        return( NULL ); 

    /* Resolve '.' or '..' -- input is overwritten */
    if ( ( tmp_path = DtEliminateDots( path ) ) == NULL )
    {
        XtFree( path );
        return( NULL );
    }

    /* Remove trailing '/' */
    tmp_path = path + strlen( path );
    while (( tmp_path != path ) && *--tmp_path == '/' )
        *tmp_path = '\0';

    return( path );
}


/*
 * Get the parent directory name of a file path name.
 */

String
GetDirName(
    String path )
{
    char tmp_buf[MAXPATHLEN];
    char *ptr;

    if ( !path || !*path )			/* Zero or empty argument */
        return( XtNewString( "." ) );

    tmp_buf[0] = '\0';
    strcpy( tmp_buf, path );
    ptr = dirname( tmp_buf );

    /*
     * There is a bug in dirname(), in which an empty string is return
     * for the following path: /filename. 
     */
    if( !ptr || !*ptr )				/* Zero or empty argument */
        return( XtNewString( "/" ) );

    return( XtNewString( tmp_buf ) );
}


/*
 * Get the Mask value of the file.
 */

int
GetMaskValues()
{
    return( proginfo.list.mask );
}


/*
 * Set the Mask value of the file.
 */

void
SetMaskValues(
    int perm )
{
    proginfo.list.mask = perm;
}


/*
 * Get the number of groups and the group access list to which the calling
 * process belongs and store the result in the array of group IDs.
 * Caller is responsible for freeing the list.
 */

void
GetGroups(
    int *ngroups,
    gid_t **grouplist )
{
    int ngroups_max;

    /*
     * In case supplementary group IDs is not supported or we could not get
     * the configuration info then the default value will be 1.
     */
    if ( ( ngroups_max = sysconf( _SC_NGROUPS_MAX ) ) <= 0 )
        *ngroups = 1;

    *grouplist = ( gid_t * )XtMalloc( sizeof( gid_t ) * ngroups_max );

    if ( ( *ngroups = getgroups( ngroups_max, *grouplist ) ) == -1 )
    {
        *ngroups = 1;
        *grouplist[0] = getegid();
    }
}


/*
 * Get the user's id from the password file.
 * Return the user's id, -1 on error.
 */

uid_t
GetUserID(
    char *name )
{
    struct passwd *passwdbufp;

    passwdbufp = getpwnam( name );

    if ( passwdbufp != NULL )
        return( passwdbufp->pw_uid ); 
    else
        return( -1 ); 
}


/*
 * Get the group's id from the group file.
 * Return the group's id, -1 on error.
 */

gid_t
GetGroupID(
    char *name )
{
    struct group *groupbufp;

    groupbufp = getgrnam( name );

    if ( groupbufp != NULL )
        return( groupbufp->gr_gid ); 
    else
        return( -1 ); 
}


/*
 * Get the user's name from the password file.
 * Return the user's name, or string contains the user's id if no match found.
 * Caller is responsible for freeing the string.
 */

String
GetUserName(
    uid_t id )
{
    String name;
    struct passwd *passwdbufp;

    passwdbufp = getpwuid( (uid_t)id );

    if ( passwdbufp != NULL )
        name = XtNewString( passwdbufp->pw_name ); 
    else
    {
        char tmp_buf[32];

        tmp_buf[0] = '\0';
        sprintf( tmp_buf, "%d", id );
        name = XtNewString( tmp_buf );
    }

    return( name ); 
}


/*
 * Get the group's name from the group file.
 * Return the group's name, or string contains the group's id if no match found.
 * Caller is responsible for freeing the string.
 */

String
GetGroupName(
    gid_t id )
{
    String name;
    struct group *groupbufp;

    groupbufp = getgrgid( (gid_t)id );

    if ( groupbufp != NULL )
        name = XtNewString( groupbufp->gr_name ); 
    else
    {
        char tmp_buf[32];

        tmp_buf[0] = '\0';
        sprintf( tmp_buf, "%d", id );
        name = XtNewString( tmp_buf );
    }

    return( name );
}


/*
 * This function determines the number of pixels of a given string length
 * for a fixed width font. 
 */

Dimension
CvtStringLengthToPixel(
    XmFontList fixed_font,
    int length )
{
    char	tmp_buf[BUF_SIZ];
    XmString    label_xmstring = NULL;
    Dimension	pixel = 0;

    /*
     * Since we are dealing with fixed width font, we can just initialize
     * the temporary string with spaces.
     */
    memset( (void *)tmp_buf, 0x20, length );
    tmp_buf[length] = '\0';

    /* Now, create a temporary XmString */
    label_xmstring = XmStringCreateLocalized( tmp_buf );
    pixel = XmStringWidth( fixed_font, label_xmstring );
    XmStringFree( label_xmstring );

    return( pixel );
}


/*
 * Convert permission mode to a formated text string to inserted into the list. 
 * Caller is responsible for freeing the string.
 */

String
CvtPermModeToString(
    int perm )
{
    String perm_str = ( String )XtMalloc( 4 );

    if ( IS_RPERM( perm ) )
        perm_str[0] = 'r';
    else
        perm_str[0] = '-';

    if ( IS_WPERM( perm ) )
        perm_str[1] = 'w';
    else
        perm_str[1] = '-';

    if ( IS_XPERM( perm ) )
        perm_str[2] = 'x';
    else
        perm_str[2] = '-';

    perm_str[3] = '\0';

    return( perm_str );
}


/*
 * Convert each element of an ACL entry into a formatted text string.
 */

ListEntryData *
CvtAclEntryToListEntry(
    aclent_t acl_entry )
{
    int			mask;
    String		class = NULL;
    String		name = NULL;
    String		perm = NULL;
    String		eperm = NULL;
    ListEntryData	*listentry = NULL;

    mask = GetMaskValues();

    /* Get the memory for the list entry */
    listentry = XtNew( ListEntryData );

    switch ( acl_entry.a_type )
    {
        case USER:
            class = XtNewString( "User" );
            name  = GetUserName( acl_entry.a_id );
            eperm = CvtPermModeToString( acl_entry.a_perm & mask );
            break;

        case GROUP:
            class = XtNewString( "Group" );
            name  = GetGroupName( acl_entry.a_id );
            eperm = CvtPermModeToString( acl_entry.a_perm & mask );
            break;

        case CLASS_OBJ:
            class = XtNewString( "Mask" );
            break;

        case DEF_USER: 
            class = XtNewString( "Default User" );
            name  = GetUserName( acl_entry.a_id );
            break;

        case DEF_GROUP: 
            class = XtNewString( "Default Group" );
            name  = GetGroupName( acl_entry.a_id );
            break;

        case DEF_USER_OBJ:
            class = XtNewString( "Default Owning User" );
            break;

        case DEF_GROUP_OBJ:
            class = XtNewString( "Default Owning Group" );
            break;

        case DEF_OTHER_OBJ:
            class = XtNewString( "Default Other" );
            break;

        case DEF_CLASS_OBJ:
            class = XtNewString( "Default Mask" );
            break;
    }

    if ( name == NULL )
    {
        name = ( String )XtCalloc( N_FIELD_SIZE, sizeof( String ) );
        memset( ( void * )name, '*', ( N_FIELD_SIZE - 2 ) );
    }

    perm = CvtPermModeToString( acl_entry.a_perm );

    /*
     * For entry that does not have effective permissions use
     * permissions as effective permissions.
     */
    if ( eperm == NULL )
        eperm = XtNewString( perm );

    strcpy( listentry->class, class );
    strcpy( listentry->name, name );
    strcpy( listentry->perm, perm );
    strcpy( listentry->eperm, eperm );

    XtFree( class );
    XtFree( name );
    XtFree( perm );
    XtFree( eperm );

    return( listentry );
}


/*
 * Convert an ACL entry to a formated text string to inserted into the list. 
 */

String
CvtAclEntryToString(
    aclent_t aclentry )
{
    int			mask;
    String		acl_str;
    ListEntryData	*listentry;

    /* Get the memory for the result string */
    acl_str = ( String )XtMalloc( TOTAL_FIELD_SIZE + 1 );

    listentry = CvtAclEntryToListEntry( aclentry );

    acl_str[0] = '\0';
    sprintf( acl_str, "%-*s%-*s%-*s%-*s",
        C_FIELD_SIZE, listentry->class,
        N_FIELD_SIZE, listentry->name,
        P_FIELD_SIZE, listentry->perm,
        E_FIELD_SIZE, listentry->eperm );

    XtFree( (char *)listentry );

    return( acl_str );
}


/*
 * Select an item in the list.
 */

void
SelectListItemsEntry(
    int position )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    XmListSelectPos( instance->accessList, position, True );
}


/*
 * Change an item in the list with the specified ACL entry.
 */

ListItemRec *
ChangeListItemsEntry(
    ListItemRec *list,
    aclent_t aclentry,
    int position )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    register		int i;
    String		string;
    XmString		label_xmstring;
    ListItemRec		*curr = list;

    for ( i = 1 ; i < position ; i++ )
    {
        if ( curr->next != NULL )
            curr = curr->next;		/* Keep going... */
        else
            return( list );		/* Error - shouldn't be here! */
    }

    /*
     * Update the entry (permission only) in the linked-list.  
     */
    (curr->acl_entry).a_perm = aclentry.a_perm;

    /*
     * Update the entry in the list widget and selected the item
     */
    string = CvtAclEntryToString( curr->acl_entry );
    label_xmstring = XmStringCreateLocalized( string ); 
    XmListReplaceItemsPos(instance->accessList, &label_xmstring, 1, position);
    XmStringFree( label_xmstring );
    XtFree( string );

    /* Select the item in the list */
    XtAppAddTimeOut( XtWidgetToApplicationContext( proginfo.topLevel ),
        0, (XtTimerCallbackProc)SelectListItemsEntry, (XtPointer)position );

    /*
     * If we are modifying the Mask then we have to update all the entries
     * in the Access List widget.
     */
    if ( (curr->acl_entry).a_type == CLASS_OBJ )
    {
        /* Update the mask and the group effective permission */
        SetMaskValues( (curr->acl_entry).a_perm );
        UpdateGroupEffectPerm_CB( (instance->groupRead_items).nolabel_item,
            (XtPointer)&(*instance), NULL );

        curr = list;
        for ( i = 1 ; curr != NULL ; i++, curr = curr->next )
        {
            /* Update the entry in the list widget */
            string = CvtAclEntryToString( curr->acl_entry );
            label_xmstring = XmStringCreateLocalized( string ); 
            XmListReplaceItemsPos(
                instance->accessList, &label_xmstring, 1, i );
            XmStringFree( label_xmstring );
            XtFree( string );
        }
    }

    return( list );
}


/*
 * Add an ACL entry into the list.
 * If the selected variable is set to True, the item will be selected.
 */

ListItemRec *
AddListItemsEntry(
    ListItemRec *list,
    aclent_t aclentry,
    Boolean selected )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    register int	i, position;
    String		string;
    XmString		label_xmstring;
    ListItemRec		*new = NULL;
    ListItemRec		*prev = NULL;
    ListItemRec		*curr = list;
    XmString		xmstring;

    /* Create a new entry */
    new = XtNew( ListItemRec );
    new->next = NULL;

    (new->acl_entry).a_type = aclentry.a_type;
    (new->acl_entry).a_id   = aclentry.a_id;
    (new->acl_entry).a_perm = aclentry.a_perm;

    for ( i = 1 ; i <= TOTAL_ENTRY_TYPES ; i++ )
    {
        if ( (new->acl_entry).a_type == sort_table[i].type )
        {
            new->sort_order = sort_table[i].order;
            break;
        }
    }

    /* Add new entry to the linked-list */
    position = 1;

    if ( curr == NULL )
    {
        list = new;
    }
    else
    {
        /*
         * Walk down the linked-list until we found the place to insert
         * the entry.  The entry will be inserted between "prev" and "curr".
         * If "prev" is NULL then we will inserted in the front of the
         * linked-list.  We also assume that there are no duplicated entry.
         */
        while ( ( new->sort_order > curr->sort_order ) ||
              ( ( new->sort_order == curr->sort_order ) &&
                ( (new->acl_entry).a_id > (curr->acl_entry).a_id ) ) )
        {
            prev = curr;
            curr = curr->next;
            position++;

            if ( curr == NULL )
                break;
        }

        /* Insert at the front */ 
        if ( prev == NULL )
        {
            new->next = curr;
            list = new;
        }
        else
        {
            new->next = prev->next;
            prev->next = new;
        }
    }

    /* Insert new entry to the list widget */
    string = CvtAclEntryToString( aclentry );
    label_xmstring = XmStringCreateLocalized( string ); 
    XmListAddItem( instance->accessList, label_xmstring, position );
    XmStringFree( label_xmstring );
    XtFree( string );

    /* Select the inserted item in the list */
    if ( selected )
        XtAppAddTimeOut( XtWidgetToApplicationContext( proginfo.topLevel ),
            0, (XtTimerCallbackProc)SelectListItemsEntry, (XtPointer)position );

    return( list );
}


/*
 * Delete an item in the list.
 */

ListItemRec *
DeleteListItemsEntry(
    ListItemRec *list,
    int position )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    register int i;
    int		 nitems;
    ListItemRec *temp = list;
    ListItemRec *curr = NULL;		/* Entry we want to delete */

    /* Update our internal list */
    if ( temp == NULL )
        return( NULL );			/* Error - shouldn't be here! */

    if ( position == 1 )
    {
        curr = list;
        list = list->next;
    }
    else
    {
        /*
         * Walk down the linked-list until the next entry is the entry we
         * want to delete (i.e. curr->next is the entry we want to delete).
         */
        for ( i = 2 ; i < position ; i++ )
        {
            if ( temp->next != NULL )
                temp = temp->next;		/* Keep going... */
            else
                return( list );			/* Entry doesn't exist */
        }
        curr = temp->next;			/* This is the one to delete */
        temp->next = temp->next->next;
    }

    XtFree( (char *)curr );			/* Free this entry */

    /* Delete entry from the list widget */
    XmListDeletePos( instance->accessList, position );

    /* Select the next item in the list */
    XtVaGetValues( instance->accessList,
        XmNitemCount, &nitems,
        NULL );

    if ( position <= nitems )
        XtAppAddTimeOut( XtWidgetToApplicationContext( proginfo.topLevel ),
            0, (XtTimerCallbackProc)SelectListItemsEntry, (XtPointer)position );
    else
    {
        if ( nitems != 0 )
            XtAppAddTimeOut(
                XtWidgetToApplicationContext( proginfo.topLevel ),
                0, (XtTimerCallbackProc)SelectListItemsEntry, (XtPointer)(position - 1) );
        else
        {
            XtVaSetValues( instance->deleteButton,
                XmNsensitive, False,
                NULL );
            XtVaSetValues( instance->changeButton,
                XmNsensitive, False,
                NULL );
        }
    }

    return( list );
}


/*
 * Get an ACL entry from our internal list based on its position.
 */

aclent_t *
GetListItemsEntry(
    ListItemRec *list,
    int position )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    register int	i;
    int			nitems;
    aclent_t		*aclentry;
    ListItemRec		*curr = list;

    if ( curr == NULL )
        return( (aclent_t *)NULL );	/* Error - shouldn't be here! */

    aclentry = XtNew( aclent_t );

    /*
     * Walk down the linked-list until we found the entry.
     */
    for ( i = 1 ; i < position ; i++ )
    {
        if ( curr->next != NULL )
        {
            curr = curr->next;			/* Keep going... */
        }
        else
        {
            XtFree( (char *)aclentry );
            return( (aclent_t *)NULL );		/* Entry doesn't exist */
        }
    }

    aclentry->a_type = (curr->acl_entry).a_type;
    aclentry->a_id   = (curr->acl_entry).a_id;
    aclentry->a_perm = (curr->acl_entry).a_perm;

    return( aclentry );
}


/*
 * Initialize the ListInfo data structure.
 */

void
ClearListInfo()
{
    ListItemRec *curr = proginfo.list.items;
    ListItemRec *this = NULL;

    while ( curr != NULL )
    {
        this = curr;
        curr = curr->next;

        XtFree( (char *)this );			/* Free this Entry */
    }

    proginfo.list.items = NULL;

    proginfo.list.has_def_user_obj  = False;
    proginfo.list.has_def_group_obj = False;
    proginfo.list.has_def_other_obj = False;
    proginfo.list.has_def_class_obj = False;
}


/*
 * Initialize the list widget with the specified ACLData.
 */

void
InitListInfo(
    ACLData *adata )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    register int	i;
    XmString		xmstring;
    ListItemRec		*new = NULL;

    /* Clear the list widget */
    XmListDeleteAllItems( instance->accessList ); 

    /* Find the mask value, and check for default ACL entries */
    for ( i = 0 ; i < adata->nentries ; i++ )
    {
        switch ( (adata->bufp[i]).a_type )
        {
            case CLASS_OBJ:
                SetMaskValues( (adata->bufp[i]).a_perm );
                break;

            case DEF_USER_OBJ:
                proginfo.list.has_def_user_obj  = True;
                break;

            case DEF_GROUP_OBJ:
                proginfo.list.has_def_group_obj = True;
                break;

            case DEF_OTHER_OBJ:
                proginfo.list.has_def_other_obj = True;
                break;

            case DEF_CLASS_OBJ:
                proginfo.list.has_def_class_obj = True;
                break;
        }
    }

    for ( i = 0 ; i < adata->nentries ; i++ )
    {
        /* Don't want to add the user, group and other entry in the list */
        if ( ( (adata->bufp[i]).a_type != USER_OBJ ) &&
             ( (adata->bufp[i]).a_type != GROUP_OBJ ) &&
             ( (adata->bufp[i]).a_type != OTHER_OBJ ) )
        {
            new = AddListItemsEntry( new, adata->bufp[i], False );
        }
    }

    proginfo.list.items = new;

    XmUpdateDisplay( proginfo.topLevel );
}


FileData *
CreateFileData()
{
    FileData *fdata = NULL;

    /* Allocate and initialize the file attribute data. */
    fdata = XtNew( FileData );

    fdata->file_name     = NULL; 
    fdata->link_name     = NULL;
    fdata->owner         = NULL;
    fdata->group         = NULL;
    fdata->data_type     = NULL;
    fdata->stat          = NULL;
    fdata->acl           = NULL;
    fdata->pixmap        = NULL;

    return( fdata );
}


void
FreeFileData(
    FileData *fdata )
{
    if ( fdata == NULL )
        return;

    /*
     * First, free all the memory that we have allocated
     */
    if ( fdata->file_name != NULL )
        XtFree( fdata->file_name );

    if ( fdata->link_name != NULL )
        XtFree( fdata->link_name );

    if ( fdata->owner != NULL )
        XtFree( fdata->owner );

    if ( fdata->group != NULL )
        XtFree( fdata->group );

    if ( fdata->data_type != NULL )
        XtFree( fdata->data_type );

    if ( fdata->stat != NULL )
        XtFree( (char *)fdata->stat );

    if ( fdata->acl != NULL )
    {
        if ( fdata->acl->bufp != NULL )
            XtFree( (char *)fdata->acl->bufp );

        XtFree( (char *)fdata->acl );
    }

    if ( fdata->pixmap != NULL )
    {
        DtDtsFreeAttributeValue( fdata->pixmap->host_prefix );
        DtDtsFreeAttributeValue( fdata->pixmap->instance_icon_name );
        DtDtsFreeAttributeValue( fdata->pixmap->icon_name );
        DtDtsFreeAttributeValue( fdata->pixmap->icon_file_name );

        XtFree( (char *)fdata->pixmap );
    }

    XtFree( (char *)fdata );
}


/*
 * Get all the information relevant for the file.
 */

FileData *
GetFileInfo(
    char *file )
{
    char	link_file_name[MAXPATHLEN];
    char	link_path[MAXPATHLEN];
    char	tmp_buf[MAXPATHLEN];
    char	*real_file_name;
    char	*tmpptr;
    char	**link_list;
    int		link_len;
    int		link_count;
    int		stat_result;
    int		stat_errno;
    int		i;
    StatData	stat_buf;
    StatData	stat_buf2;
    Boolean	recursive_link_found;
    FileData	*fdata;

    if ( file == NULL )
        return( NULL );

    /* Allocate memory for the new FileData structure */
    if ( ( fdata = CreateFileData() ) == NULL )
        return( NULL );

    fdata->file_name = XtNewString( file ); 

    /* Follow symbolic links to their ultimate destination */
    link_count = 0;
    link_list = NULL;
    link_file_name[0] = '\0';
    link_path[0] = '\0';
    recursive_link_found = False;

    strcpy( link_file_name, fdata->file_name );

    stat_result = lstat( link_file_name, &stat_buf );

    if ( stat_result == 0 && S_ISLNK( stat_buf.st_mode ) )
    {
        while ((link_len = readlink(link_file_name, link_path, MAXPATHLEN)) > 0)
        {
            link_path[link_len] = NULL;
            link_list = (char **)XtRealloc( (char *)link_list,
                sizeof(char *) * (link_count + 2) );

            /* Force the link to be an absolute path, if necessary */
            if ( link_path[0] != '/' )
            {
                /* Relative paths are relative to the current directory */
                if ( ( tmpptr = strrchr( link_file_name, '/' ) ) != NULL )
                    *++tmpptr = '\0';
                strcat( link_file_name, link_path );
            }
            else
                strcpy( link_file_name, link_path );

            /* Check for a recursive loop; abort if found */
            for ( i = 0; i < link_count; i++ )
            {
                if ( strcmp( link_file_name, link_list[i] ) == 0 )
                {
                    /* Back up to last non-recursive portion */
                    strcpy( link_file_name, link_list[link_count - 1] );
                    recursive_link_found = True;
                    break;
                }
            }

            if ( recursive_link_found )
                break;

            link_list[link_count++] = XtNewString( link_file_name );
            link_list[link_count] = NULL;
        }

        /* Try to stat the file that the link points to */
        if ( stat( link_file_name, &stat_buf2 ) == 0 )
        {
            /* Replace lstat result with the stat */
            memcpy( &stat_buf, &stat_buf2, sizeof( StatData ) );
        }
    }
    stat_errno = errno;

    if ( link_list )
    {
        fdata->link_name = XtNewString( link_list[link_count - 1] );

        /* Free all memory in link_list */
        for (i = 0; i < link_count; i++)
           XtFree( link_list[i] );
        XtFree( (char *)link_list );

        real_file_name = link_file_name;
    }
    else
        real_file_name = fdata->file_name;

    /* Fill in the FileData structure */ 
    if ( stat_result == 0 )
    {
        /* Find the logical type of the file */
        if ( S_ISLNK( stat_buf.st_mode ) )
        {
            /*
             * We should have located the stat buffer of the file links to
             * by now; if not then it must be a broken link.
             */  
            if ( recursive_link_found )
                fdata->data_type = XtNewString( LT_RECURSIVE_LINK );
            else
                fdata->data_type = XtNewString( LT_BROKEN_LINK );
        }
        else
        {
            if ( fdata->link_name )
                fdata->data_type = (char *)DtDtsDataToDataType(
                    fdata->file_name, NULL, 0, &stat_buf,
                    fdata->link_name, NULL, NULL );
            else
                fdata->data_type = (char *)DtDtsDataToDataType(
                    fdata->file_name, NULL, 0, &stat_buf,
                    NULL, NULL, NULL );

#if defined( DATATYPE_IS_FIXED )
#else
            /* The problem here is there isn't a way for user to mask
               only the OWNER READ bit in the MODE field of dtfile.dt file.
               If the MODE field set to d&!r Then all READ permission
               (S_IRUSR, S_IRGRP and S_IROTH)
               bits has to be off in order for the above data typing to work.
               The work around is manually checking it ourselves.
               When the data typing code is fixed, please remove this check.
             */
            if ( (strcmp( fdata->data_type, LT_DIRECTORY ) == 0 ) ||
                 (strcmp( fdata->data_type, LT_CURRENT_FOLDER ) == 0 ) ||
                 (strcmp( fdata->data_type, LT_DOT_FOLDER ) == 0 ) )
            {
                if( access( real_file_name, R_OK ) != 0 )
                {
                    XtFree( fdata->data_type );
                    fdata->data_type = XtNewString( LT_FOLDER_LOCK );
                }
                else if( access( real_file_name, W_OK ) != 0 )
                {
                    XtFree( fdata->data_type );
                    fdata->data_type = XtNewString( LT_NON_WRITABLE_FOLDER );
                }
            }
#endif
        }

        /* Set the pixmap data structure of the file */
        fdata->pixmap = RetrievePixmapData( fdata->data_type, LARGE );

        /* Set the owner name of the file */
        fdata->owner = GetUserName( stat_buf.st_uid );

        /* Set the group name of the file */
        fdata->group = GetGroupName( stat_buf.st_gid );

        /* Set the stat data structure of the file */
        fdata->stat = XtNew( StatData );
        memcpy( fdata->stat, &stat_buf, sizeof( StatData ) );
    }
    else
    {
        /* File did not exist or can't be accessed */
        proginfo.saved_errno = stat_errno; 

        FreeFileData( fdata );
        return( NULL );
    }

    /* Set the ACL data structure of the file */
    if ( proginfo.is_acl_support )
        fdata->acl = GetFileACL( real_file_name, fdata->stat );

    return( fdata );	/* Returns the FileData structure */
}


/*
 * Update the file ownership.
 * Return -1 if no changes, 0 if file has been updated, or errno on error.
 */

int
UpdateFileOwnership(
    char	*file,
    StatData	*o_sdata,
    StatData	*n_sdata )
{
    if ( file == NULL )
        return( -1 );

    if ( ( o_sdata->st_uid != n_sdata->st_uid ) ||
         ( o_sdata->st_gid != n_sdata->st_gid ) )
    {
        if ( chown( file, n_sdata->st_uid, n_sdata->st_gid ) != 0 )
        {
            proginfo.saved_errno = errno;
            return( errno );
        }
        else
            return( 0 );
    }

    return( -1 );
}


/*
 * Update the file permissions.
 */

int
UpdateFilePermission(
    char	*file,
    StatData	*o_sdata,
    ACLData	*o_adata,
    StatData	*n_sdata,
    ACLData	*n_adata,
    Boolean	is_merge )
{
    aclent_t	*wrk_bufp; 
    aclent_t	*tmp_bufp;
    Boolean	found;
    int		i, j;
    int		wrk_nentries;
    int		rc = 0;

    if ( file == NULL )
        return( -1 );

    /* Use ACL if it is defined otherwise use regular UNIX permissions */
    if ( o_adata != NULL )
    {
        /*
         * To merge the ACL entries, we create a temporary working buffer
         * and copy all of the original ACL entries into this buffer.  Next,
         * for each entry in the new ACL list, we will traverse down the old
         * ACL list, and check to see if we can found an entry with the same
         * class.  If there is a match, and the entry ID is the same, or for
         * all the entries that doesn't required an ID, we replace its
         * permissions with the new permissions.
         */
        if ( is_merge )
        {
            wrk_bufp = (aclent_t *)XtMalloc( sizeof( aclent_t ) *
                ( o_adata->nentries + n_adata->nentries ) );
            memcpy( wrk_bufp, o_adata->bufp,
                sizeof( aclent_t ) * o_adata->nentries );
            wrk_nentries = o_adata->nentries;

            for ( i = 0 ; i < n_adata->nentries ; i++ )
            { 
                found = False;

                for ( j = 0 ; j < o_adata->nentries ; j++ )
                {
                    if ( n_adata->bufp[i].a_type == o_adata->bufp[j].a_type )
                    {
                        if ( (n_adata->bufp[i].a_id == o_adata->bufp[j].a_id) ||
                             ( (n_adata->bufp[i].a_type != USER ) &&
                               (n_adata->bufp[i].a_type != GROUP ) &&
                               (n_adata->bufp[i].a_type != DEF_USER) &&
                               (n_adata->bufp[i].a_type != DEF_GROUP) ) ) 
                        {
                            found = True;
                            wrk_bufp[j].a_perm = n_adata->bufp[i].a_perm;
                            break;
                        }
                    }
                }

                /* Add it at the end */
                if ( !found )
                {
                    memcpy( wrk_bufp + wrk_nentries, &n_adata->bufp[i],
                        sizeof( aclent_t ) );
                    wrk_nentries++;
                }
            }

            if ( (*acl_fptr)( file, SETACL, wrk_nentries, wrk_bufp ) == -1 )
                rc = errno;

            XtFree( (char *)wrk_bufp );
        }
        else
        {
            if ( (*acl_fptr)
                 ( file, SETACL, n_adata->nentries, n_adata->bufp ) == -1 )
                rc = errno;
        }
    }
    else
    {
        if ( chmod( file, n_sdata->st_mode ) != 0 )
            rc = errno;
    }

    proginfo.saved_errno = rc;

    return( rc );
}


int
ConfirmAndUpdateFile(
    FileData	*fdata,
    Boolean	*quit )
{
    String	real_file_name;
    int		rc = 0;
    int		answer;

    if ( fdata == NULL )
        return( -1 );

    /* Get the real name */
    if ( fdata->link_name == NULL )
        real_file_name = fdata->file_name;
    else
        real_file_name = fdata->link_name;

    /* Check to see if we need to confirm each changes */
    if ( proginfo.is_confirm_chg )
    {
        answer = ChangeConfirmPrompt( real_file_name,
            S_ISDIR( fdata->stat->st_mode ),
            ( fdata->acl != NULL ) ? True : False );

        if ( answer == ANSWER_ACTION_2 )
        {
            return( 0 );	/* Do not change this file */
        }
        else
        if ( answer == ANSWER_CANCEL )
        {
            *quit = True;	/* Cancel recursive operation */
            return( 0 );
        }
    }

    rc = UpdateFilePermission( real_file_name,
                               fdata->stat,
                               fdata->acl,
                               (proginfo.new_fdata)->stat,
                               (proginfo.new_fdata)->acl,
                               proginfo.is_merge_acl );

    return( rc );
}


/*
 * This is a function for retrieving the pixmap data for a data type.
 */

PixmapData *
RetrievePixmapData(
    char *data_type,
    int size )
{
    PixmapData	*pixmap_data;
    char	*icon_name;

    pixmap_data = XtNew( PixmapData );

    if ( !pixmap_data )
        return( NULL );

    pixmap_data->size = size;

    /* Retrieve host name */
    pixmap_data->host_prefix =
        DtDtsDataTypeToAttributeValue( data_type, DtDTS_DA_DATA_HOST, NULL );

    pixmap_data->instance_icon_name = NULL;

    pixmap_data->icon_name =
        DtDtsDataTypeToAttributeValue( data_type, DtDTS_DA_ICON, NULL );

    /* Retrieve icon file name */
    if ( pixmap_data->size == LARGE )
        pixmap_data->icon_file_name = XmGetIconFileName( proginfo.screen,
                                          pixmap_data->instance_icon_name,
                                          pixmap_data->icon_name,
                                          pixmap_data->host_prefix,
                                          XmMEDIUM_ICON_SIZE );
    else
        pixmap_data->icon_file_name = XmGetIconFileName( proginfo.screen,
                                          pixmap_data->instance_icon_name,
                                          pixmap_data->icon_name,
                                          pixmap_data->host_prefix,
                                          XmTINY_ICON_SIZE );

    /* Return pixmap data */
    return( pixmap_data );
}


/*
 * This is a function for retrieving the ACL data for a file.
 */

ACLData *
GetFileACL(
    char *fname,
    StatData *stat )
{
    ACLData	*acl_data;

    if ( S_ISFIFO( stat->st_mode ) ||
        !strcmp( stat->st_fstype, MNTTYPE_SWAP ) ||
        !strcmp( stat->st_fstype, MNTTYPE_TMPFS ) )
    {
        return( NULL );
    }

    /* Allocate a new ACLData structure.  */
    acl_data = XtNew( ACLData );

    acl_data->nentries = (*acl_fptr)( fname, GETACLCNT, 0, NULL );

    /* Get file's ACL entries */
    if ( acl_data->nentries != -1 )
    {
        /*
         * Allocate memory for the ACL entries
         */
        acl_data->bufp = (aclent_t *)
            XtMalloc( (sizeof(aclent_t) * (acl_data->nentries + 1)) );

        if ( acl_data->bufp == NULL )
        {
            XtFree( (char *)acl_data );
            return( NULL );
        }

        if ( (*acl_fptr)( fname, GETACL, acl_data->nentries, acl_data->bufp ) == -1 )
        {
            XtFree( (char *)acl_data->bufp );
            XtFree( (char *)acl_data );
            return( NULL );
        }
        (*aclsort_fptr)( acl_data->nentries, 0, acl_data->bufp );

        /*
         * We need to verify that this file is ACL capable when the number of
         * of entries is 4 and the file is not a UNIX file system.  This is
         * because GETACLCNT command will return 4, i.e. user, group, other,
         * mask, even for file that are not ACL capable or on file system that
         * doesn't support ACL (Solaris 2.4). 
         *  
         * The only way to test this is to use the SETACL command and check
         * the result.
         */
        if ( acl_data->nentries == 4 && strcmp( stat->st_fstype, MNTTYPE_UFS ) )
        {
            acl_data->bufp[4].a_type = USER;
            acl_data->bufp[4].a_id = 0;
            acl_data->bufp[4].a_perm = 4;

            if ( (*acl_fptr)( fname, SETACL, 5 , acl_data->bufp ) == -1 )
            {
                errno = 0;
                XtFree( (char *)acl_data->bufp );
                XtFree( (char *)acl_data );
                return( NULL );
            }
            else
            {
                memset( (void*)(&acl_data->bufp[4]), 0, sizeof(aclent_t) );

                if ( (*acl_fptr)( fname, SETACL, 4 , acl_data->bufp ) == -1 )
                {
                    errno = 0;
                    XtFree( (char *)acl_data->bufp );
                    XtFree( (char *)acl_data );
                    return( NULL );
                }
            }
        }
    }
    else
    {
        XtFree( (char *)acl_data );
        return( NULL );
    }

    /* Return ACL data */
    return( acl_data );
}


FileData *
GetPropDialogValues()
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    FileData    	*fdata;
    ListItemRec 	*curr;
    aclent_t		work_bufp[MAX_ACL_ENTRIES];
    Widget		omenu_widget;
    char		*err_msg = NULL;
    register int	i;
    int			acl_result;

    /* Allocate memory for the new FileData structure */
    if ( ( fdata = CreateFileData() ) == NULL )
        return( NULL );

    /* Allocate a new StatData structure.  */
    if ( ( fdata->stat = XtNew( StatData ) ) == NULL )
    {
        FreeFileData( fdata );
        return( NULL );
    }

    /* Get file ownership */
    fdata->stat->st_uid = (proginfo.current_fdata)->stat->st_uid;
    fdata->stat->st_gid = (proginfo.current_fdata)->stat->st_gid;

    /* Get and check if owner name is valid (only for root user) */
    if ( XtIsManaged( instance->ownerNameTfield ) )
    {
        fdata->owner = (String)XmTextFieldGetString(instance->ownerNameTfield);

        if ( *fdata->owner && ( strlen( fdata->owner ) > 0 ) )
        {
            if ( ( fdata->stat->st_uid = GetUserID( fdata->owner ) ) == -1 )
            {
                if ( isdigit( fdata->owner[0] ) )
                    fdata->stat->st_uid = atoi( fdata->owner );
                else
                    err_msg = catgets( Dtb_project_catd, 4, 4, "Owner Name is invalid.\n\nThe specified user does not exist." );
            }
        }
        else
            err_msg = catgets( Dtb_project_catd, 4, 5, "Owner Name is invalid.\n\nYou must specify a User Name or identification number." ) ;

        if ( err_msg != NULL )
        {
            ProcessErrorMsg( ERROR, False, err_msg );
            FreeFileData( fdata );
            return( NULL );
        }
    }

    /* Get and check if group name is valid */
    if ( XtIsManaged( instance->groupNameTfield ) )
    {
        fdata->group = (String)XmTextFieldGetString(instance->groupNameTfield);

        if ( *fdata->group && ( strlen( fdata->group ) > 0 ) )
        {
            if ( ( fdata->stat->st_gid = GetGroupID( fdata->group ) ) == -1 )
            {
                if ( isdigit( fdata->group[0] ) )
                    fdata->stat->st_gid = atoi( fdata->group );
                else
                    err_msg = catgets( Dtb_project_catd, 4, 6, "Group Name is invalid.\n\nThe specified group does not exist.  Use your system admnistration\n\nsoftware if you want to create a user group with this name." );
            }

            /* Can only change to a group to which the owner belongs */
            if ( ( err_msg == NULL ) &&
                 ( !proginfo.is_root_user ) &&
                 ( fdata->stat->st_gid !=
                     (proginfo.current_fdata)->stat->st_gid ) )
            {
                Boolean found = False;

                for ( i = 0 ; i < proginfo.ngroups ; i++ )    
                {
                    if ( fdata->stat->st_gid == proginfo.grouplist[i] )
                    {
                        found = True;
                        break;
                    }
                }

                if ( !found )
                    err_msg = catgets( Dtb_project_catd, 4, 8, "Group Name is invalid.\n\nYou can only change the Group Name to the name\nof another group to which you belong." );

            }
        }
        else
            err_msg = catgets( Dtb_project_catd, 4, 7, "Group Name is invalid.\n\nYou must specify a Group Name or identification number." );

        if ( err_msg != NULL )
        {
            ProcessErrorMsg( ERROR, False, err_msg );
            FreeFileData( fdata );
            return( NULL );
        }
    }

    /* Get file basic permissions */
    fdata->stat->st_mode = 0;

    if ( XmToggleButtonGetState( (instance->ownerRead_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IRUSR;
    if ( XmToggleButtonGetState( (instance->ownerWrite_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IWUSR;
    if ( XmToggleButtonGetState( (instance->ownerExecute_items).nolabel_item ) )
    {
        /* Preserve setuid bits from the original file */
        fdata->stat->st_mode |=
            S_IXUSR | ((proginfo.current_fdata)->stat->st_mode & S_ISUID );
    }

    if ( XmToggleButtonGetState( (instance->groupRead_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IRGRP;
    if ( XmToggleButtonGetState( (instance->groupWrite_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IWGRP;
    if ( XmToggleButtonGetState( (instance->groupExecute_items).nolabel_item ) )
    {
        /* Preserve setgid bits from the original file */
        fdata->stat->st_mode |=
            S_IXGRP | ((proginfo.current_fdata)->stat->st_mode & S_ISGID );
    }

    if ( XmToggleButtonGetState( (instance->otherRead_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IROTH;
    if ( XmToggleButtonGetState( (instance->otherWrite_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IWOTH;
    if ( XmToggleButtonGetState( (instance->otherExecute_items).nolabel_item ) )
        fdata->stat->st_mode |= S_IXOTH;

    /* Get file's ACL if applicable */
    if ( (proginfo.current_fdata)->acl != NULL )
    {
        /* Get the ACL entries from the list widget in the UI */ 
        i = 0;
        curr = proginfo.list.items;
        while ( curr != NULL )
        {
            work_bufp[i].a_type = (curr->acl_entry).a_type;
            work_bufp[i].a_id   = (curr->acl_entry).a_id;
            work_bufp[i].a_perm = (curr->acl_entry).a_perm;

            i++;
            curr = curr->next;
        }

        /* We also need to include entries for Owner, Group, and Other */
        work_bufp[i].a_type = USER_OBJ;
        work_bufp[i].a_id   = fdata->stat->st_uid;
        work_bufp[i].a_perm = GET_UPERM( fdata->stat->st_mode ); 
        i++;

        work_bufp[i].a_type = GROUP_OBJ;
        work_bufp[i].a_id   = fdata->stat->st_gid;
        work_bufp[i].a_perm = GET_GPERM( fdata->stat->st_mode ); 
        i++;

        work_bufp[i].a_type = OTHER_OBJ;
        work_bufp[i].a_id   = -1;
        work_bufp[i].a_perm = GET_OPERM( fdata->stat->st_mode ); 
        i++;

        /* Allocate a new ACLData structure.  */
        if ( ( fdata->acl = XtNew( ACLData ) ) == NULL )
        {
            FreeFileData( fdata );
            return( NULL );
        }

        fdata->acl->nentries = i;
        fdata->acl->bufp = (aclent_t *)XtMalloc( sizeof(aclent_t) * i );
        memcpy( fdata->acl->bufp, work_bufp, sizeof( aclent_t ) * i );

        /* Now, call the aclsort() function to check and sort the ACLs */
        acl_result = (*aclsort_fptr)( fdata->acl->nentries, 0, fdata->acl->bufp );

        if ( acl_result != 0 )
        {
            ProcessErrorMsg( ERROR, False, catgets( Dtb_project_catd, 4, 23, "An error has occurred while checking the validity of the ACL\nfor this file or folder.\n\nPlease check and make sure that entries of type User, Group,\nDefault User, or Default Group do not contain duplicate entries." ) );
            FreeFileData( fdata );
            return( NULL );
        }
    }

    /* Get recursive value */
    proginfo.is_f_recursive = False;
    proginfo.is_d_recursive = False;
    proginfo.recursive_depth = 0;

    if ( S_ISDIR( proginfo.current_fdata->stat->st_mode ) )
    {
        XtVaGetValues( instance->applyFolderOmenu,
            XmNmenuHistory, &omenu_widget,
            NULL );

        if ( omenu_widget == instance->applyFolderOmenu_items.This_Folder_and_its_Subfolders_item ) 
        {
            proginfo.is_d_recursive = True;
            proginfo.recursive_depth = 1024;
        }
    }
    else
    {
        XtVaGetValues( instance->applyFileOmenu,
            XmNmenuHistory, &omenu_widget,
            NULL );

        if ( omenu_widget == instance->applyFileOmenu_items.All_Files_in_Parent_Folder_item )
        {
            proginfo.is_f_recursive = True;
            proginfo.recursive_depth = 1;
        }
        else
        if ( omenu_widget == instance->applyFileOmenu_items.All_Files_in_Parent_Folder_and_its_Subfolders_item )
        {
            proginfo.is_f_recursive = True;
            proginfo.recursive_depth = 1024;
        }
    }

    return( fdata );
}


void
SetPropDialogValues(
    FileData * fdata )
{
    DtbSdtfpropPropdialogInfo	instance = &dtb_sdtfprop_propdialog;

    Boolean	sensitivity;
    uid_t	owner_id;
    char	tmp_buf[BUF_SIZ];
    char	*base_name;
    XmString	label_xmstring, label_xmstring2;
    XmString	owner_xmstring, group_xmstring;
    Dimension	max_width, width;
    XmFontList	font_list = NULL;
    struct tm	*tms;

    /*
     * Verify that we are allowed to change the attributes
     */
    if ( ( owner_id = GetUserID( fdata->owner ) ) == -1 )
    {
        if ( isdigit( fdata->owner[0] ) )
            owner_id = atoi( fdata->owner );
    }
    if ( ( proginfo.user_id == owner_id ) || proginfo.is_root_user )
    {
        /* Check for access rights (e.g. NFS mounted filesystem)  */
        if ( proginfo.is_root_user && strcmp( fdata->stat->st_fstype, MNTTYPE_UFS ) )
        {
            int rc;

            /* Get the real name */
            if ( fdata->link_name == NULL )
                rc = chmod( fdata->file_name, fdata->stat->st_mode & 07777 );
            else
                rc = chmod( fdata->link_name, fdata->stat->st_mode & 07777 );

            if ( rc == 0 )
                sensitivity = True;
            else
                sensitivity = False;
        }
        else
            sensitivity = True;
    }
    else
        sensitivity = False;

    if ( fdata->data_type &&
        ( strcmp( fdata->data_type, LT_RECURSIVE_LINK) == 0 ||
          strcmp( fdata->data_type, LT_BROKEN_LINK) == 0 ) )
        sensitivity = False;

    proginfo.is_modifiable = sensitivity;

    /*
     * Change dialog title and application icon name to include the file name
     */
    tmp_buf[0] = '\0';
    base_name = basename( fdata->file_name );
    if ( sensitivity )
        sprintf( tmp_buf,
            catgets( Dtb_project_catd, 3, 11, "Properties - %s" ), base_name );
    else
        sprintf( tmp_buf,
            catgets( Dtb_project_catd, 3, 12, "Properties - %s [Read Only]" ), 
            base_name );
    XtVaSetValues( instance->propdialog,
        XmNtitle, tmp_buf,
        NULL );
    
    XtVaSetValues( proginfo.topLevel,
        XmNiconName, base_name,
        NULL );

    /*
     * Set all the relevant info in the information pane, i.e. onwer name,
     * group name, file size, access time, modified time... 
     */
    if ( fdata->owner )
    {
        if ( isdigit( fdata->owner[0] ) )
        {
            tmp_buf[0] = '\0';
            sprintf( tmp_buf, "%s ( %s )", fdata->owner,
                catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
            owner_xmstring = XmStringCreateLocalized( tmp_buf );
        }
        else
            owner_xmstring = XmStringCreateLocalized( fdata->owner );
    }
    else
    {
        owner_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
    }
    XtVaSetValues( instance->infoOwnerName,
        XmNlabelString, owner_xmstring,
        NULL );

    if ( fdata->group )
    {
        if ( isdigit( fdata->group[0] ) )
        {
            tmp_buf[0] = '\0';
            sprintf( tmp_buf, "%s ( %s )", fdata->group,
                catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
            group_xmstring = XmStringCreateLocalized( tmp_buf );
        }
        else
            group_xmstring = XmStringCreateLocalized( fdata->group );
    }
    else
    {
        group_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
    }
    XtVaSetValues( instance->infoGroupName,
        XmNlabelString, group_xmstring,
        NULL );

    tmp_buf[0] = '\0';
    sprintf( tmp_buf,
        catgets( Dtb_project_catd, 3, 43, "%lld bytes" ), (long long)fdata->stat->st_size );

    label_xmstring = XmStringCreateLocalized( tmp_buf );
    XtVaSetValues( instance->infoSize,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    tmp_buf[0] = '\0';
    tms = localtime( &fdata->stat->st_atime );
    strftime( tmp_buf, BUF_SIZ, 0, tms );
    label_xmstring = XmStringCreateLocalized( tmp_buf );
    XtVaSetValues( instance->infoAccessTime,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    tmp_buf[0] = '\0';
    tms = localtime( &fdata->stat->st_mtime );
    strftime( tmp_buf, BUF_SIZ, 0, tms );
    label_xmstring = XmStringCreateLocalized( tmp_buf );
    XtVaSetValues( instance->infoModifyTime,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    /* Only root user can change ownership */
    if ( sensitivity && proginfo.is_root_user )
    { 
        XtManageChild( instance->ownerNameTfield );
        XtUnmanageChild( instance->ownerName );

        if ( fdata->owner )
            XmTextFieldSetString( instance->ownerNameTfield, fdata->owner );
        else
            XmTextFieldSetString( instance->ownerNameTfield,
                catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
    }
    else
    {
        XtUnmanageChild( instance->ownerNameTfield );
        XtManageChild( instance->ownerName );

        XtVaSetValues( instance->ownerName,
            XmNlabelString, owner_xmstring,
            NULL );
    }
    XmStringFree( owner_xmstring );

    /*
     * There are 3 cases in which a user allow to change the group ownership
     * of a file:
     *    1. A root user.
     *    2. User belongs to more than 1 group.
     *    3. Group ownership of the file doesn't match the group ID of the
     *       user, so user is allow to change the group ownership of the
     *       file back to the user's group ID.
     */
    if ( sensitivity &&
         ( proginfo.is_root_user || 
         ( proginfo.ngroups > 1 ) ||
         ( proginfo.group_id != fdata->stat->st_gid ) ) )
    {
        XtManageChild( instance->groupNameTfield );
        XtUnmanageChild( instance->groupName );

        if ( fdata->group )
            XmTextFieldSetString( instance->groupNameTfield, fdata->group );
        else
            XmTextFieldSetString( instance->groupNameTfield,
                catgets( Dtb_project_catd, 2, 42, "Unknown" ) );
    }
    else
    {
        XtUnmanageChild( instance->groupNameTfield );
        XtManageChild( instance->groupName );

        XtVaSetValues( instance->groupName,
            XmNlabelString, group_xmstring,
            NULL );
    }
    XmStringFree( group_xmstring );

    /*
     * Set the file name
     */
    XmTextFieldSetString( instance->fileNameTfield, fdata->file_name );
    XmTextFieldSetInsertionPosition( instance->fileNameTfield, MAXPATHLEN );

    /*
     * Set the linked file name text field
     */
    if ( fdata->link_name )
    {
        XmTextFieldSetString( instance->linkNameTfield, fdata->link_name );
        XmTextFieldSetInsertionPosition( instance->linkNameTfield, MAXPATHLEN );

        XtSetMappedWhenManaged( instance->linkNameGroup, True );
    }
    else
    {
        XtSetMappedWhenManaged( instance->linkNameGroup, False );
    }

    /*
     * Set the file icon
     */
    if ( fdata->data_type )
    {
        char *ptr = NULL;

        /*
         * Get the Attribute name from the database and note that we don't
         * need to create a localized string for the name.
         */
        if ( ptr = (char *)
            DtDtsDataTypeToAttributeValue( fdata->data_type,
                                           DtDTS_DA_NAME,
                                           NULL) )
        {
            label_xmstring =
                XmStringCreateLtoR( ptr, XmFONTLIST_DEFAULT_TAG ); 
            DtDtsFreeAttributeValue( ptr );
        }
        else
            label_xmstring =
                XmStringCreateLtoR( fdata->data_type, XmFONTLIST_DEFAULT_TAG); 
    }
    else
    {
        label_xmstring = XmStringCreateLocalized( " " ); 
    }
    XtVaSetValues( proginfo.iconType,
        XmNstring, label_xmstring,
        XmNimageName,
            ( fdata->pixmap ? fdata->pixmap->icon_file_name : NULL ),
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Set the basic permissions 
     */
    XmToggleButtonSetState( (instance->ownerRead_items).nolabel_item,
        ( fdata->stat->st_mode & S_IRUSR ) ? True : False, False );
    XmToggleButtonSetState( (instance->ownerWrite_items).nolabel_item,
        ( fdata->stat->st_mode & S_IWUSR ) ? True : False, False );
    XmToggleButtonSetState( (instance->ownerExecute_items).nolabel_item,
        ( fdata->stat->st_mode & S_IXUSR ) ? True : False , False );
    XmToggleButtonSetState( (instance->groupRead_items).nolabel_item,
        ( fdata->stat->st_mode & S_IRGRP ) ? True : False, False );
    XmToggleButtonSetState( (instance->groupWrite_items).nolabel_item,
        ( fdata->stat->st_mode & S_IWGRP ) ? True : False, False );
    XmToggleButtonSetState( (instance->groupExecute_items).nolabel_item,
        ( fdata->stat->st_mode & S_IXGRP ) ? True : False, False );
    XmToggleButtonSetState( (instance->otherRead_items).nolabel_item,
        ( fdata->stat->st_mode & S_IROTH ) ? True : False, False );
    XmToggleButtonSetState( (instance->otherWrite_items).nolabel_item,
        ( fdata->stat->st_mode & S_IWOTH ) ? True : False, False );
    XmToggleButtonSetState( (instance->otherExecute_items).nolabel_item,
        ( fdata->stat->st_mode & S_IXOTH ) ? True : False, False );

    XtSetSensitive( (instance->ownerRead_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->ownerWrite_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->ownerExecute_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->groupRead_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->groupWrite_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->groupExecute_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->otherRead_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->otherWrite_items).nolabel_item, sensitivity );
    XtSetSensitive( (instance->otherExecute_items).nolabel_item, sensitivity );

    /*
     * Set file's ACL 
     */
    SetMaskValues( 7 );		/* Reset file mask to default */
    if ( proginfo.is_acl_support && ( fdata->acl != NULL ) )
    {
        if ( fdata->acl->nentries >= 4 )
        {
            register int i;
            register aclent_t *tmp_bufp;

            ClearListInfo();
            InitListInfo( fdata->acl );

            if ( fdata->acl->nentries == 4 )
                HideACL();
            else
                ShowACL();

            XtSetSensitive( instance->aclButton, True );
            XtSetSensitive( instance->deleteButton, False );
            XtSetSensitive( instance->changeButton, False );

            /*
             * We need to use the group permissions (i.e., GROUP_OBJ) from
             * the ACL list for the basic permissions pane.  This is because
             * the value in st_mode is the effective permissions. 
             */
            tmp_bufp = fdata->acl->bufp;
            for ( i = 0 ; i < fdata->acl->nentries ; i++ )
                if ( (tmp_bufp[i]).a_type == GROUP_OBJ )
                {
                    XmToggleButtonSetState(
                        (instance->groupRead_items).nolabel_item,
                        IS_RPERM( tmp_bufp[i].a_perm ) ? True : False, False );
                    XmToggleButtonSetState(
                        (instance->groupWrite_items).nolabel_item,
                        IS_WPERM( tmp_bufp[i].a_perm ) ? True : False, False );
                    XmToggleButtonSetState(
                        (instance->groupExecute_items).nolabel_item,
                        IS_XPERM( tmp_bufp[i].a_perm ) ? True : False, False );
                }
        }
    }
    else
    {
        HideACL();
        XtSetSensitive( instance->aclButton, False );
    }

    /*
     * Initialize recursive options depending on the file type, update
     * the fileName label to reflect the its type, and align the nameLabel
     * object with the linkLabel's object.
     */
    if ( S_ISDIR( proginfo.current_fdata->stat->st_mode ) )
    {
        label_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 3, 3, "Folder Name:" ) );
        XtSetMappedWhenManaged( instance->applyFileOmenu, False );
        XtSetMappedWhenManaged( instance->applyFolderOmenu, True );
    }
    else
    {
        label_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 2, 27, "File Name:" ) );
        XtSetMappedWhenManaged( instance->applyFileOmenu, True );
        XtSetMappedWhenManaged( instance->applyFolderOmenu, False );
    }

    XtVaGetValues( instance->linkLabel,
        XmNfontList, &font_list,
        XmNlabelString, &label_xmstring2,
        NULL );

    max_width = XmStringWidth( font_list, label_xmstring2 );
    if ( ( width = XmStringWidth( font_list, label_xmstring ) ) > max_width )
        max_width = width;
    max_width += 3;	/* Just to be sure that we have enough room... */ 

    XtVaSetValues( instance->linkLabel,
        XmNwidth, max_width,
        NULL );
    XtVaSetValues( instance->nameLabel,
        XmNlabelString, label_xmstring,
        XmNwidth, max_width,
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Update effective permissions for basic permissions
     */
    UpdateOwnerEffectPerm_CB( (instance->ownerRead_items).nolabel_item,
        (XtPointer)&(*instance), NULL );
    UpdateGroupEffectPerm_CB( (instance->groupRead_items).nolabel_item,
        (XtPointer)&(*instance), NULL );
    UpdateOtherEffectPerm_CB( (instance->otherRead_items).nolabel_item,
        (XtPointer)&(*instance), NULL );

    /*
     * Reset button should be active if there is a file loaded
     */
    XtSetSensitive( instance->resetButton, True );

    /*
     * User doesn't have modification privileges
     */
    XtSetSensitive( instance->addButton, sensitivity );
    XtSetSensitive( instance->applyToLabel, sensitivity );
    XtSetSensitive( instance->applyFileOmenu, sensitivity );
    XtSetSensitive( instance->applyFolderOmenu, sensitivity );
    XtSetSensitive( instance->okButton, sensitivity );
    XtSetSensitive( instance->applyButton,
        sensitivity && ( proginfo.current_pane == PERM_PANE ) );

    XtRemoveAllCallbacks( instance->accessList, XmNdefaultActionCallback );
    if ( sensitivity )
        XtAddCallback( instance->accessList,
            XmNdefaultActionCallback, PropDialogChange_CB,
            (XtPointer)&(*instance) );
}


int
GetModDialogClassType()
{
    DtbSdtfpropModdialogInfo instance = &dtb_sdtfprop_moddialog;

    int		class;
    Boolean	map_when_managed;
    Widget	omenu_widget = NULL;

    XtVaGetValues( instance->mod_classOmenu,
        XtNmappedWhenManaged, &map_when_managed,
        NULL );

    /*
     * If the Class option menu is not mapped then we must be changing the
     * permission only.
     */
    if ( map_when_managed == False )
        return( 0 );

    XtVaGetValues( instance->mod_classOmenu,
        XmNmenuHistory, &omenu_widget,
        NULL );

    /* Determine the type of ACL entry */
    if ( omenu_widget == instance->mod_classOmenu_items.User_item )
        class = USER;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Group_item )
        class = GROUP;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Default_User_item )
        class = DEF_USER;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Default_Group_item )
        class = DEF_GROUP;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Default_Owning_User_item )
        class = DEF_USER_OBJ;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Default_Owning_Group_item )
        class = DEF_GROUP_OBJ;
    else
    if ( omenu_widget == instance->mod_classOmenu_items.Default_Other_item )
        class = DEF_OTHER_OBJ;
    else
        class = DEF_CLASS_OBJ;

    return( class );
}


int
GetModDialogPermValues()
{
    DtbSdtfpropModdialogInfo	instance = &dtb_sdtfprop_moddialog;

    int perm = 0;

    if ( XmToggleButtonGetState( (instance->mod_permCbox_items).r_item ) )
        perm += 4;
    if ( XmToggleButtonGetState( (instance->mod_permCbox_items).w_item ) )
        perm += 2;
    if ( XmToggleButtonGetState( (instance->mod_permCbox_items).x_item ) )
        perm += 1;

    return( perm );
}


void
SetModDialogPermValues(
    int perm )
{
    DtbSdtfpropModdialogInfo	instance = &dtb_sdtfprop_moddialog;

    String	eperm;
    XmString	label_xmstring;

    if ( IS_RPERM( perm ) )
        XmToggleButtonSetState( (instance->mod_permCbox_items).r_item,
            True, False );
    else
        XmToggleButtonSetState( (instance->mod_permCbox_items).r_item,
            False, False );
    if ( IS_WPERM( perm ) )
        XmToggleButtonSetState( (instance->mod_permCbox_items).w_item,
            True, False );
    else
        XmToggleButtonSetState( (instance->mod_permCbox_items).w_item,
            False, False );
    if ( IS_XPERM( perm ) )
        XmToggleButtonSetState( (instance->mod_permCbox_items).x_item,
            True, False );
    else
        XmToggleButtonSetState( (instance->mod_permCbox_items).x_item,
            False, False );

    eperm  = CvtPermModeToString( perm & GetMaskValues() );
    label_xmstring = XmStringCreateLocalized( eperm );
    XtVaSetValues( instance->mod_effPermLabel,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );
    XtFree( eperm );
}


aclent_t *
GetModDialogValues()
{
    DtbSdtfpropModdialogInfo    instance = &dtb_sdtfprop_moddialog;

    aclent_t	*aclentry;
    char	*name;
    char	*err_msg = NULL;

    aclentry = XtNew( aclent_t );

    /* Get ACL type */
    if ( ( aclentry->a_type = GetModDialogClassType() ) != 0 )
    {
        /* Get entry uid or gid */
        if (( aclentry->a_type == USER ) || ( aclentry->a_type == DEF_USER ))
        {
            name = (char *)XmTextFieldGetString( instance->mod_nameTfield );

            if ( *name && ( strlen( name ) > 0 ) )
            {
                if ( ( aclentry->a_id = GetUserID( name ) ) == -1 )
                {
                    if ( isdigit( name[0] ) )
                        aclentry->a_id = atoi( name );
                    else
                        err_msg = catgets( Dtb_project_catd, 4, 9, "User Name is invalid.\n\nThe specified user does not exist." );
                }

                XtFree( name );
            }
            else
                err_msg = catgets( Dtb_project_catd, 4, 10, "User Name is invalid.\n\nYou must specify a User Name or identification number." );
        }
        else
        if (( aclentry->a_type == GROUP ) || ( aclentry->a_type == DEF_GROUP ))
        {
            name = (char *)XmTextFieldGetString( instance->mod_nameTfield );

            if ( *name && ( strlen( name ) > 0 ) )
            {
                if ( ( aclentry->a_id = GetGroupID( name ) ) == -1 )
                {
                    if ( isdigit( name[0] ) )
                        aclentry->a_id = atoi( name );
                    else
                        err_msg = catgets( Dtb_project_catd, 4, 6, "Group Name is invalid.\n\nThe specified group does not exist.  Use your system administration\n\nsoftware if you want to create a user group with this name." );
                }

                XtFree( name );
            }
            else
                err_msg = catgets( Dtb_project_catd, 4, 7, "Group Name is invalid.\n\nYou must specify a Group Name or identification number." );
        }

        /* Check for error */
        if ( err_msg != NULL )
        {
            ProcessErrorMsg( ERROR, False, err_msg );
            XtFree( (char *)aclentry );
            return( (aclent_t *)NULL );
        }
    }

    /* Get ACL permissions */
    aclentry->a_perm = GetModDialogPermValues();

    return( aclentry );
}


void
SetModDialogValues(
    int operation,
    int position )
{
    DtbSdtfpropModdialogInfo     instance = &dtb_sdtfprop_moddialog;

    Boolean		change_perm_only = False;
    static String	add_title_str = NULL;
    static String	chg_title_str = NULL;
    static XmString	add_button_xmstring = NULL;
    static XmString	chg_button_xmstring = NULL;
    String		title_str = NULL;
    String		user_name = NULL;
    String		class_name = NULL;
    XmString		ok_xmstring = NULL;
    XmString		label_xmstring;
    int			permission = 0;
    register int	i;
    ListItemRec		*curr = proginfo.list.items;
    void		(*help_func_ptr)();
    void		(*ok_func_ptr)();

    /*
     * Create the dialog title if we haven't created already
     */
    if ( ( add_title_str == NULL ) || ( chg_title_str == NULL ) )
    {
        /* Make sure that the dialog is initialized */
        if (!(instance->initialized))
            dtb_sdtfprop_moddialog_initialize( instance,
                dtb_sdtfprop_propdialog.propdialog );

        /*
         * Handle the mapping and unmapping of the following objects
         * dynamically because they depend on the ACL entry type.
         */
        XtSetMappedWhenManaged( instance->mod_classLabel, False );
        XtSetMappedWhenManaged( instance->mod_classOmenu, False );

        add_title_str = XtNewString( catgets( Dtb_project_catd, 3, 13,
            "Properties : Add Access List Entry" ) );
        chg_title_str = XtNewString( catgets( Dtb_project_catd, 3, 14,
            "Properties : Change Access List Entry" ) );
        add_button_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 3, 6, "Add" ) );
        chg_button_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 3, 7, "Change" ) );
    }

    SetModDialogFooter( " " );          /* Clear the footer first */

    if ( operation == ADD_OP )
    {
        ok_func_ptr = ModDialogAddOK_CB;
        help_func_ptr = ModDialogAddHelp_CB;

        title_str = add_title_str;
        ok_xmstring = add_button_xmstring;

        /*
         * Set default items in Class option menu accordingly
         */
        XtVaSetValues( instance->mod_classOmenu,
            XmNmenuHistory, instance->mod_classOmenu_items.User_item,
            NULL );
    }
    else
    {
        ok_func_ptr = ModDialogChangeOK_CB;
        help_func_ptr = ModDialogChangeHelp_CB;

        title_str = chg_title_str;
        ok_xmstring = chg_button_xmstring;

        /*
         * Walk down the linked-list until we found the entry
         */
        for ( i = 1 ; i < position ; i++ )
            curr = curr->next;
   
        /*
         * Get the user/group name and initialize the Class option menu
         */
        switch ( (curr->acl_entry).a_type )
        {
            case USER:
                user_name  = GetUserName( (curr->acl_entry).a_id );
                XtVaSetValues( instance->mod_classOmenu,
                    XmNmenuHistory, instance->mod_classOmenu_items.User_item,
                    NULL );
                break;

            case GROUP:
                user_name  = GetGroupName( (curr->acl_entry).a_id );
                XtVaSetValues( instance->mod_classOmenu,
                    XmNmenuHistory, instance->mod_classOmenu_items.Group_item,
                    NULL );
                break;

            case CLASS_OBJ:
                class_name = XtNewString( "Mask" ); 
                change_perm_only = True;
                break;

            case DEF_USER:
                user_name  = GetUserName( (curr->acl_entry).a_id );
                XtVaSetValues( instance->mod_classOmenu,
                    XmNmenuHistory,
                    instance->mod_classOmenu_items.Default_User_item,
                    NULL );
                break;

            case DEF_GROUP:
                user_name  = GetGroupName( (curr->acl_entry).a_id );
                XtVaSetValues( instance->mod_classOmenu,
                    XmNmenuHistory,
                    instance->mod_classOmenu_items.Default_Group_item,
                    NULL );
                break;

            case DEF_USER_OBJ:
                class_name = XtNewString( "Default Owning User" ); 
                change_perm_only = True;
                break;

            case DEF_GROUP_OBJ:
                class_name = XtNewString( "Default Owning Group" ); 
                change_perm_only = True;
                break;

            case DEF_OTHER_OBJ:
                class_name = XtNewString( "Default Other" ); 
                change_perm_only = True;
                break;

            case DEF_CLASS_OBJ:
                class_name = XtNewString( "Default Mask" ); 
                change_perm_only = True;
                break;

            default:
                break;
        }

        permission = (curr->acl_entry).a_perm;
    }

    if ( change_perm_only )
    {
        XtSetMappedWhenManaged( instance->mod_classLabel, True );
        XtSetMappedWhenManaged( instance->mod_classOmenu, False );

        XtSetSensitive( instance->mod_nameLabel, False );
        XtSetSensitive( instance->mod_nameTfield, False );

        label_xmstring = XmStringCreateLocalized( class_name );
        XtVaSetValues( instance->mod_classLabel,
            XmNlabelString, label_xmstring,
            NULL );
        XmStringFree( label_xmstring );
        XtFree( class_name );
    }
    else
    {
        XtSetMappedWhenManaged( instance->mod_classOmenu, True );
        XtSetMappedWhenManaged( instance->mod_classLabel, False );

        XtSetSensitive( instance->mod_nameLabel, True );
        XtSetSensitive( instance->mod_nameTfield, True );

        /*
         * Set default items in Class option menu accordingly
         */
        if ( S_ISDIR( proginfo.current_fdata->stat->st_mode ) )
        {
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_User_item,
                True );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Group_item,
                True );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Owning_User_item,
                !proginfo.list.has_def_user_obj );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Owning_Group_item,
                !proginfo.list.has_def_group_obj );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Other_item,
                !proginfo.list.has_def_other_obj );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Mask_item,
                !proginfo.list.has_def_class_obj );
        }
        else
        {
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_User_item,
                False );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Group_item,
                False );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Owning_User_item,
                False );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Owning_Group_item,
                False );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Other_item,
                False );
            XtSetSensitive(
                instance->mod_classOmenu_items.Default_Mask_item,
                False );
        }

    }

    /*
     * Set the name of the user/group for the text field object
     */
    if ( user_name == NULL )
        user_name  = XtNewString( "" ); 

    XmTextFieldSetString( instance->mod_nameTfield, user_name );
    XtFree( user_name );

    /*
     * Set the Permission and Effective Permission objects
     */
    SetModDialogPermValues( permission );

    /*
     * Set the title of dialog and the label of the OK button
     */
    XtVaSetValues( dtb_sdtfprop_moddialog.moddialog,
        XmNtitle, title_str,
        NULL );
    XtVaSetValues( instance->mod_okButton,
        XmNlabelString, ok_xmstring,
        NULL );

    /*
     * Set the OK and Help callback
     */
    XtRemoveAllCallbacks( instance->mod_okButton, XmNactivateCallback );
    XtRemoveAllCallbacks( instance->mod_helpButton, XmNactivateCallback );
    XtAddCallback( instance->mod_okButton,
        XmNactivateCallback, (XtCallbackProc)ok_func_ptr, (XtPointer)position );
    XtAddCallback( instance->mod_helpButton,
        XmNactivateCallback, (XtCallbackProc)help_func_ptr, NULL );
}


/*
 * The user made some sort of response to the file browser.
 * Set the answer (client_data) accordingly.
 */

void
FileBrowserResponse_CB(
    Widget	w,
    char	**answer,
    XmFileSelectionBoxCallbackStruct *cbs )
{
    char *file;

    /* Set answer to an empty string */
    if ( cbs-> reason == XmCR_CANCEL )
    {
        *answer = ( char * )XtMalloc( 1 );
        *answer[0] = '\0';
        return;
    }

    if ( !XmStringGetLtoR( cbs->value, XmSTRING_DEFAULT_CHARSET, &file ) )
        return;

    if ( strlen( file ) == 0 )
        return;

    /*
     * If it's not a directory, determine the full pathname of the selection
     * by concatenating it to the "dir" part.
     */
    if ( *file != '/' )
    {
        char *dir, *newfile;

        if ( XmStringGetLtoR( cbs->dir, XmSTRING_DEFAULT_CHARSET, &dir ))
        {
            newfile = XtMalloc( strlen(dir)+1+strlen(file)+1 );
            sprintf( newfile, "%s/%s", dir, file );
            XtFree( dir );
            XtFree( file );
            file = newfile;
        }
    }

    *answer = file;	/* Set answer to the file selection */
}


int
CreateFileSDialog(
    DtbSdtfpropFileSDialogInfo instance,
    Widget parent )
{
    XmString	label_xmstring = NULL;
    XmString	oklabel_xmstring = NULL;
    int		n = 0;
    Arg		args[10];

    if ( instance->initialized )
    {
        return ( 0 );
    }
    instance->initialized = True;

    oklabel_xmstring =
        XmStringCreateLocalized( "Ok" );

    n = 0;
    XtSetArg( args[n], XmNfileTypeMask, XmFILE_ANY_TYPE );	++n;
    XtSetArg( args[n], XmNautoUnmanage, False ) ;		++n;
    XtSetArg( args[n], XmNokLabelString, oklabel_xmstring );	++n;
    XtSetArg( args[n], "pathMode", 1 );				++n;
    XtSetArg( args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL );++n;
    instance->fileSelDialog =
        XmCreateFileSelectionDialog( parent, "fileSelDialog", args, n );

    XmStringFree( oklabel_xmstring );

    instance->fileSelDialogShell = XtParent( instance->fileSelDialog );
    XtVaSetValues( instance->fileSelDialogShell,
        XmNtitle, catgets( Dtb_project_catd, 3, 25, "File Selection Dialog" ),
        NULL );

    return ( 0 );
}


String
GetFileName(
    char *title,
    char *directory,
    char *filter,
    char *selection )
{
    DtbSdtfpropFileSDialogInfo instance = &dtb_sdtfprop_fileseldialog;

    static char	*file_name;
    XmString	dir_xmstring;
    XmString	filter_xmstring;

    if ( !instance->initialized )
    {
        CreateFileSDialog( instance, dtb_sdtfprop_propdialog.propdialog );
        XtAddCallback( instance->fileSelDialog,
            XmNokCallback, (XtCallbackProc)FileBrowserResponse_CB,
            &file_name );
        XtAddCallback( instance->fileSelDialog,
            XmNcancelCallback, (XtCallbackProc)FileBrowserResponse_CB,
            &file_name );
        XtUnmanageChild(
            XmFileSelectionBoxGetChild( instance->fileSelDialog,
                XmDIALOG_HELP_BUTTON) );
    }

    /* Set the title for the file browser dialog */
    if ( title != NULL )
        XtVaSetValues( instance->fileSelDialogShell,
            XmNtitle, title, NULL );
    else
        XtVaSetValues( instance->fileSelDialogShell,
            XmNtitle,
            catgets( Dtb_project_catd, 3, 25, "File Selection Dialog" ), NULL );

    /* Set the directory */
    if ( directory != NULL )
    {
        dir_xmstring = XmStringCreateSimple( directory );
        XtVaSetValues( instance->fileSelDialog,
            XmNdirectory, dir_xmstring, NULL );
        XmStringFree( dir_xmstring );
    }

    /* Set the file filter */
    if ( filter != NULL )
        filter_xmstring = XmStringCreateSimple( filter );
    else
        filter_xmstring = XmStringCreateSimple( "*" );
    XtVaSetValues( instance->fileSelDialog,
        XmNpattern, filter_xmstring, NULL );
    XmStringFree( filter_xmstring );

    /* Set the selection if specified */
    if ( selection != NULL)
    {
        XmTextFieldSetString(
            XmFileSelectionBoxGetChild(
                instance->fileSelDialog, XmDIALOG_TEXT ), selection );
    }

    XtManageChild( instance->fileSelDialog );

    /* Wait for user to make a selection */
    file_name = NULL;
    while ( file_name == NULL )
        XtAppProcessEvent( proginfo.appContext, XtIMAll );

    XtUnmanageChild( instance->fileSelDialog );

    return( file_name );
}


/*
 * Close button was selected from the window manager, so treated it
 * as the equivalent of Cancel. 
 */

void
MessageDialogQuitHandler(
    Widget	w,
    XtPointer	clientData,
    XtPointer	callData )
{
    int *result = (int *)clientData;
    *result = ANSWER_CANCEL;
}


void
MessageDialogResponse_CB(
    Widget	w,
    XtPointer	clientData,
    XtPointer	cbs )
{
    XtVaGetValues( w,
        XmNuserData, (int *)clientData,
        NULL );
}


void
AlertPromptHelp_CB(
    Widget	w,
    XtPointer	clientData,
    XtPointer	cbs )
{
    static Widget	quickHelpDialog = (Widget)NULL;

    /* Create shared quick help dialog */
    if( quickHelpDialog == (Widget)NULL )
    {
        quickHelpDialog = DtCreateHelpQuickDialog( proginfo.topLevel,
            "QuickHelp", NULL, 0 );

        XtVaSetValues( XtParent( quickHelpDialog ),
            XmNtitle, catgets( Dtb_project_catd, 3, 24, "Properties Help" ) ,
            NULL );

        XtUnmanageChild( DtHelpQuickDialogGetChild( quickHelpDialog,
            DtHELP_QUICK_BACK_BUTTON ) );
        XtUnmanageChild( DtHelpQuickDialogGetChild( quickHelpDialog,
            DtHELP_QUICK_PRINT_BUTTON ) );
        XtUnmanageChild( DtHelpQuickDialogGetChild( quickHelpDialog,
            DtHELP_QUICK_HELP_BUTTON ) );
    }

    XtVaSetValues( quickHelpDialog,
        DtNhelpType, DtHELP_TYPE_DYNAMIC_STRING,
        DtNstringData, (char *)clientData,
        NULL );

    /* Now display the help dialog */
    XtManageChild( quickHelpDialog );
}


int
AlertPrompt(
    Widget frame,
    ... )
{
    static int		answer;
    int			dialog_type;
    int			button_cnt = 0;
    int			msg_cnt = 0;
    int			button_id = 0;
    int			modality = XmDIALOG_MODELESS;
    char		*str, *ptr;
    char		*help_str = NULL;
    Widget		pshell, dialog, button;
    XmString		xmstring;
    XmFontList		font_list = NULL;
    Dimension		width;
    Atom		delete_window; 
    va_list		ap;
    int			op;

    va_start( ap, p );
    op = va_arg( ap, int );
    while ( op )
    {
        switch ( op )
        {
            case DIALOG_TYPE:
                dialog_type = va_arg( ap, int );
                dialog = XmCreateMessageDialog( frame, "AlertDialog", NULL, 0 );
                XtVaSetValues( dialog,
                    XmNdialogType, dialog_type,
                    XmNnoResize, True,
                    XmNverticalSpacing, 0,
                    NULL );
                pshell = XtParent( dialog );
                delete_window = XmInternAtom( proginfo.display,
                    "WM_DELETE_WINDOW", False );
                XmAddWMProtocolCallback( pshell,
                    delete_window, MessageDialogQuitHandler, &answer );
                break;

            case DIALOG_TITLE:
                str = va_arg( ap, char * );
                if ( str && pshell )
                    XtVaSetValues( pshell,
                        XtNtitle, str,
                        NULL );
                break;

            case DIALOG_STYLE:
                modality = va_arg( ap, int );
                break;

            case DIALOG_TEXT:
                str = va_arg( ap, char * );
                if ( str && dialog )
                {
                    xmstring = XmStringCreateLocalized( str );
                    XtVaSetValues( dialog, XmNmessageString, xmstring, NULL );
                    XmStringFree( xmstring );
                }
                break;

            case HELP_IDENT:
                help_str = va_arg( ap, char * );
                break;

            case BUTTON_IDENT:
                button_id = va_arg( ap, int );
                str = va_arg( ap, char * );
                ++button_cnt;
                button = XtVaCreateManagedWidget( str,
                             xmPushButtonGadgetClass, dialog,
                             XmNuserData, button_id,
                             NULL);
                XtAddCallback( button,
                    XmNactivateCallback, MessageDialogResponse_CB, &answer );
                break;

            default:
                break;
        }
        op = va_arg( ap, int );
    }
    va_end( ap );

    /*
     * Unmanage buttons so we can display our own.
     */
    button = XmMessageBoxGetChild( dialog, XmDIALOG_OK_BUTTON );
    if ( button )
        XtUnmanageChild( button );
    button = XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON );
    if ( button )
        XtUnmanageChild( button );
    button = XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON );
    if ( button )
    {
        if ( help_str != NULL )
        {
            XtAddCallback( button,
                XmNactivateCallback, AlertPromptHelp_CB, help_str );
        }
        else
            XtUnmanageChild( button );
    }

    /*
     * Set dialog minimum width
     */
    XtVaSetValues( pshell,
        XmNminWidth, 300,
        NULL );

    /*
     * Set modality
     */
    XtVaSetValues( dialog,
        XmNdialogStyle, modality,
        NULL );

    XtManageChild( dialog );
    XBell( proginfo.display, 0 );

    XtPopup( pshell, XtGrabNone );

    answer = ANSWER_NONE;
    while( answer == ANSWER_NONE )
        XtAppProcessEvent( proginfo.appContext, XtIMXEvent | XtIMAll );

    XtDestroyWidget( pshell );
    XSync( proginfo.display, 0 );

    return( answer );
}


void
RecursiveConfirmPromptHelp_CB(
    Widget      w,
    XtPointer   clientData,
    XtPointer   cbs )
{
    ShowHelpDialog( HELP_VOLUME, HELP_CONFIRMCHANGESDIALOG_ID );
}


int
CreateRecursiveCDialog(
    DtbSdtfpropRecurCDialogInfo instance,
    Widget parent )
{
    WidgetList	children = NULL;
    int		numChildren = 0;
    XmString	label_xmstring;

    if ( instance->initialized )
    {
        return ( 0 );
    }
    instance->initialized = True;

    /*
     * Create Shell, Dialog, Forms.
     */
    instance->dialogShell = XmCreateDialogShell( parent,
        "ConfirmDialog", NULL, 0 );
    XtVaSetValues( instance->dialogShell,
        XmNtitle,
        catgets( Dtb_project_catd, 3, 15, "Properties : Confirm Changes" ),
        XmNallowShellResize, True,
        NULL );

    instance->dialog = XtVaCreateWidget( "MessageDialog",
        xmMessageBoxWidgetClass,
        instance->dialogShell,
        XmNautoUnmanage, False,
        XmNdialogType, XmDIALOG_TEMPLATE,
        XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
        XmNnoResize, True,
        NULL );
    
    instance->form = XtVaCreateManagedWidget( "MessageForm",
        xmFormWidgetClass, instance->dialog,
        XmNallowResize, True,
        XmNresizePolicy, XmRESIZE_ANY,
        NULL );

    /*
     * Create Message Label 
     */
    label_xmstring = XmStringCreateLocalized( catgets( Dtb_project_catd, 3, 26,
        "This operation will change the permissions on multiple files." ) );
    instance->label = XtVaCreateManagedWidget( "MessageLabel",
        xmLabelWidgetClass, instance->form,
        XmNtopAttachment, XmATTACH_FORM,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNlabelString, label_xmstring,
        XmNalignment, XmALIGNMENT_BEGINNING,
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Create Radiobox1 for confirmation options
     */
    instance->rbox1_rowcolumn = XtVaCreateManagedWidget( "Radiobox1Rowcolumn",
        xmRowColumnWidgetClass, instance->form,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, instance->label,
        XmNtopOffset, 15,
        XmNbottomAttachment, XmATTACH_NONE,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        XmNentryVerticalAlignment, XmALIGNMENT_BASELINE_BOTTOM,
        XmNpacking, XmPACK_TIGHT,
        XmNisHomogeneous, False,
        XmNadjustLast, True,
        XmNentryAlignment, XmALIGNMENT_BEGINNING,
        XmNorientation, XmVERTICAL,
        NULL);

    label_xmstring = XmStringCreateLocalized(
        catgets( Dtb_project_catd, 3, 28, "Do you want to:" ) );
    instance->rbox1_label = XtVaCreateManagedWidget( "Radiobox1Label",
        xmLabelWidgetClass, instance->rbox1_rowcolumn,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    instance->rbox1 = XmCreateRadioBox( instance->rbox1_rowcolumn, "Radiobox1", NULL, 0 ); 
    XtVaSetValues( instance->rbox1,
        XmNradioBehavior, True,
        XmNentryClass, xmToggleButtonWidgetClass,
        XmNnumColumns, 1,
        XmNisHomogeneous, True,
        XmNspacing, 0,
        XmNrowColumnType, XmWORK_AREA,
        XmNpacking, XmPACK_TIGHT,
        XmNorientation, XmVERTICAL,
        NULL );

    label_xmstring = XmStringCreateLocalized( catgets( Dtb_project_catd, 3, 29,
        "Ask for confirmation before each change?" ) );
    instance->rbox1_toggleButton1 = XtVaCreateWidget( "Radiobox1ToggleButton1",
        xmToggleButtonWidgetClass, instance->rbox1,
        XmNset, True,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized( catgets( Dtb_project_catd, 3, 30,
        "Change all files and do not ask for confirmation?" ) );
    instance->rbox1_toggleButton2 = XtVaCreateWidget( "Radiobox1ToggleButton2",
        xmToggleButtonWidgetClass, instance->rbox1,
        XmNset, False,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Create Radiobox2 for ACL options
     */
    instance->rbox2_rowcolumn = XtVaCreateManagedWidget( "Radiobox2Rowcolumn",
        xmRowColumnWidgetClass, instance->form,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, instance->rbox1_rowcolumn,
        XmNtopOffset, 10,
        XmNbottomAttachment, XmATTACH_NONE,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        XmNentryVerticalAlignment, XmALIGNMENT_BASELINE_BOTTOM,
        XmNpacking, XmPACK_TIGHT,
        XmNisHomogeneous, False,
        XmNadjustLast, True,
        XmNentryAlignment, XmALIGNMENT_BEGINNING,
        XmNorientation, XmVERTICAL,
        NULL);

    label_xmstring = XmStringCreateLocalized( catgets( Dtb_project_catd, 3, 32,
     "When an Access Control List already exists on a file, do you want to:" ));
    instance->rbox2_label = XtVaCreateManagedWidget( "Radiobox2Label",
        xmLabelWidgetClass, instance->rbox2_rowcolumn,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    instance->rbox2 = XmCreateRadioBox( instance->rbox2_rowcolumn, "Radiobox2", NULL, 0 ); 
    XtVaSetValues( instance->rbox2,
        XmNradioBehavior, True,
        XmNentryClass, xmToggleButtonWidgetClass,
        XmNnumColumns, 1,
        XmNisHomogeneous, True,
        XmNspacing, 0,
        XmNrowColumnType, XmWORK_AREA,
        XmNpacking, XmPACK_TIGHT,
        XmNorientation, XmVERTICAL,
        NULL );

    label_xmstring = XmStringCreateLocalized(catgets( Dtb_project_catd, 3, 34,
        "Merge the new ACL entries with the existing ACL entries?" ) );
    instance->rbox2_toggleButton1 = XtVaCreateWidget( "Radiobox2ToggleButton1",
        xmToggleButtonWidgetClass, instance->rbox2,
        XmNset, True,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized( catgets( Dtb_project_catd, 3, 35,
        "Remove the existing ACL entries and replace with the new ACL entries?" ) );
    instance->rbox2_toggleButton2 = XtVaCreateWidget( "Radiobox2ToggleButton2",
        xmToggleButtonWidgetClass, instance->rbox2,
        XmNset, False,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Create the okButton, cancelButton, and helpButton
     */
    label_xmstring = XmStringCreateLocalized(
        catgets( Dtb_project_catd, 2, 20, "OK" ) );
    instance->okButton = XtVaCreateManagedWidget( "okButton",
        xmPushButtonWidgetClass,
        instance->dialog,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_ACTION_1,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized(
        catgets( Dtb_project_catd, 2, 21, "Cancel" ) );
    instance->cancelButton = XtVaCreateManagedWidget( "cancelButton",
        xmPushButtonWidgetClass,
        instance->dialog,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_CANCEL,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized(
        catgets( Dtb_project_catd, 2, 22, "Help" ) );
    instance->helpButton = XtVaCreateManagedWidget( "helpButton",
        xmPushButtonWidgetClass,
        instance->dialog,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_HELP,
        NULL );
    XmStringFree( label_xmstring );

    /*
     * Set OK to be default button
     */
    XtVaSetValues( instance->dialog,
        XmNdefaultButton, instance->okButton,
        NULL);

    /*
     * Manage the RadioBox widgets
     */
    XtVaGetValues( instance->rbox1,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL );
    XtManageChildren( children, numChildren );
    XtVaGetValues( instance->rbox1_rowcolumn,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL );
    XtManageChildren( children, numChildren );

    XtVaGetValues( instance->rbox2,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL );
    XtManageChildren( children, numChildren );
    XtVaGetValues( instance->rbox2_rowcolumn,
        XmNchildren, &children, XmNnumChildren, &numChildren, NULL );
    XtManageChildren( children, numChildren );

    return( 0 );
}


int
RecursiveConfirmPrompt()
{
    DtbSdtfpropRecurCDialogInfo instance = &dtb_sdtfprop_recurcdialog;

    static int	answer;
    XmString	label_xmstring1;
    XmString	label_xmstring2;
    XmString	label_xmstring3;
    Atom	delete_window;

    if ( !instance->initialized )
    {
        CreateRecursiveCDialog( instance, dtb_sdtfprop_propdialog.propdialog );
        XtAddCallback( instance->okButton,
            XmNactivateCallback, (XtCallbackProc)MessageDialogResponse_CB,
            &answer );
        XtAddCallback( instance->cancelButton,
            XmNactivateCallback, (XtCallbackProc)MessageDialogResponse_CB,
            &answer );
        XtAddCallback( instance->helpButton,
            XmNactivateCallback, (XtCallbackProc)RecursiveConfirmPromptHelp_CB,
            &answer );
        delete_window = XmInternAtom( proginfo.display,
            "WM_DELETE_WINDOW", False );
        XmAddWMProtocolCallback( instance->dialogShell,
            delete_window, MessageDialogQuitHandler, &answer );
    }

    /* Change the labels according to the object type */
    if ( S_ISDIR( (proginfo.current_fdata)->stat->st_mode ) )
    {
        label_xmstring1 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 27,
          "This operation will change the permissions on multiple folders." ) );
        label_xmstring2 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 31,
          "Change all folders and do not ask for confirmation?" ) );
        label_xmstring3 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 33,
          "When an Access Control List already exists on a folder, do you want to:" ) );
    }
    else
    {
        label_xmstring1 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 26,
          "This operation will change the permissions on multiple files." ) );
        label_xmstring2 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 30,
          "Change all files and do not ask for confirmation?" ) );
        label_xmstring3 = XmStringCreateLocalized(
          catgets( Dtb_project_catd, 3, 32,
          "When an Access Control List already exists on a file, do you want to:" ) );
    }

    /*
     * Modify the main message to warn user that ownership will only be
     * apply to the selected file or folder only.
     */
    if ( XtIsManaged( dtb_sdtfprop_propdialog.ownerNameTfield ) ||
         XtIsManaged( dtb_sdtfprop_propdialog.groupNameTfield ) )
    {
        char		tmp_buf[BUF_SIZ];
        XmString	label_xmstring;
        XmString	label_xmstring1b;

        tmp_buf[0] = '\0';
        sprintf( tmp_buf, catgets( Dtb_project_catd, 3, 48,
            "\nIt will changed ownership only for \"%s\"."  ),
            basename( (proginfo.current_fdata)->file_name ) );

        label_xmstring1b = XmStringCreateLocalized( tmp_buf );
        label_xmstring   = XmStringConcat( label_xmstring1, label_xmstring1b );

        XmStringFree( label_xmstring1b );
        XmStringFree( label_xmstring1 );

        label_xmstring1 = label_xmstring;
    }

    XtVaSetValues( instance->label, 
        XmNlabelString, label_xmstring1,
        NULL );
    XtVaSetValues( instance->rbox1_toggleButton2, 
        XmNlabelString, label_xmstring2,
        NULL );
    XtVaSetValues( instance->rbox2_label, 
        XmNlabelString, label_xmstring3,
        NULL );

    XmStringFree( label_xmstring1 );
    XmStringFree( label_xmstring2 );
    XmStringFree( label_xmstring3 );

    if ( !proginfo.is_acl_support || ( (proginfo.current_fdata)->acl == NULL ) )
        XtUnmanageChild( instance->rbox2_rowcolumn );

    XtManageChild( instance->dialog );

    answer = 0;
    while( answer == 0 )
        XtAppProcessEvent( proginfo.appContext, XtIMXEvent | XtIMAll );

    /* Update the value if user hit OK */
    if ( answer == ANSWER_ACTION_1 )
    {
        proginfo.is_confirm_chg =
            XmToggleButtonGetState( instance->rbox1_toggleButton1 );
        proginfo.is_merge_acl =
            XmToggleButtonGetState( instance->rbox2_toggleButton1 );
    }

    XtUnmanageChild( instance->dialog );
    XSync( proginfo.display, 0 );

    return( answer );
}


void
ChangeConfirmPromptHelp_CB(
    Widget      w,
    XtPointer   clientData,
    XtPointer   cbs )
{
    ShowHelpDialog( HELP_VOLUME, HELP_CONFIRMEACHCHANGEDIALOG_ID );
}


int
CreateChangeCDialog(
    DtbSdtfpropChangeCDialogInfo instance,
    Widget parent )
{
    WidgetList  children = NULL;
    int         numChildren = 0;
    XmString    label_xmstring;

    if ( instance->initialized )
    {
        return ( 0 );
    }
    instance->initialized = True;

    /*
     * Create Shell, Dialog
     */
    instance->dialogShell = XmCreateDialogShell( parent,
        "ConfirmEachDialog", NULL, 0 );
    XtVaSetValues( instance->dialogShell,
        XmNtitle,
        catgets( Dtb_project_catd, 3, 16, "Properties : Confirm Each Change" ),
        XmNallowShellResize, True,
        NULL );

    instance->dialog = XtVaCreateWidget( "MessageDialog",
        xmMessageBoxWidgetClass,
        instance->dialogShell,
        XmNautoUnmanage, False,
        XmNdialogType, XmDIALOG_QUESTION,
        XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL,
        XmNnoResize, True,
        NULL );

    /*
     * Assign value of the label, changeButton, doNotChange, cancelButton
     * and helpButton.
     */
    instance->label =
        XmMessageBoxGetChild( instance->dialog, XmDIALOG_MESSAGE_LABEL );

    instance->changeButton =
        XmMessageBoxGetChild( instance->dialog, XmDIALOG_OK_BUTTON );
    instance->cancelButton =
        XmMessageBoxGetChild( instance->dialog, XmDIALOG_CANCEL_BUTTON );
    instance->helpButton =
        XmMessageBoxGetChild( instance->dialog, XmDIALOG_HELP_BUTTON );

    label_xmstring = XmStringCreateLocalized (
        catgets( Dtb_project_catd, 3, 7, "Change" ) );
    XtVaSetValues( instance->changeButton,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_ACTION_1,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized (
        catgets( Dtb_project_catd, 3, 8, "Do Not Change" ) );
    instance->doNotChangeButton = XtVaCreateManagedWidget( "doNotChangeButton",
        xmPushButtonWidgetClass,
        instance->dialog,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_ACTION_2,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized (
        catgets( Dtb_project_catd, 2, 21, "Cancel" ) );
    XtVaSetValues( instance->cancelButton,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_CANCEL,
        NULL );
    XmStringFree( label_xmstring );

    label_xmstring = XmStringCreateLocalized (
        catgets( Dtb_project_catd, 2, 22, "Help" ) );
    XtVaSetValues( instance->helpButton,
        XmNlabelString, label_xmstring,
        XmNuserData, ANSWER_HELP,
        NULL );
    XmStringFree( label_xmstring );

    return( 0 );
}


int
ChangeConfirmPrompt(
    char *file,
    Boolean is_dir,
    Boolean has_acl )
{
    DtbSdtfpropChangeCDialogInfo instance = &dtb_sdtfprop_changecdialog;

    static int  answer;
    char	msg_str[BUF_SIZ];
    char	acl_str[BUF_SIZ];
    XmString    label_xmstring;
    Atom	delete_window;

    if ( !instance->initialized )
    {
        CreateChangeCDialog( instance, dtb_sdtfprop_propdialog.propdialog );
        XtAddCallback( instance->changeButton,
            XmNactivateCallback, (XtCallbackProc)MessageDialogResponse_CB,
            &answer );
        XtAddCallback( instance->doNotChangeButton,
            XmNactivateCallback, (XtCallbackProc)MessageDialogResponse_CB,
            &answer );
        XtAddCallback( instance->cancelButton,
            XmNactivateCallback, (XtCallbackProc)MessageDialogResponse_CB,
            &answer );
        XtAddCallback( instance->helpButton,
            XmNactivateCallback, (XtCallbackProc)ChangeConfirmPromptHelp_CB,
            &answer );
        delete_window = XmInternAtom( proginfo.display,
            "WM_DELETE_WINDOW", False );
        XmAddWMProtocolCallback( instance->dialogShell,
            delete_window, MessageDialogQuitHandler, &answer );
    }

    /* Change the labels according to the object type */
    acl_str[0] = '\0';
    if ( has_acl )
    {
        if ( proginfo.is_merge_acl )
            sprintf( acl_str, "%s\n\n", catgets( Dtb_project_catd, 3, 36, "Change will merge the new ACL entries with the existing ACL entries." ) );
        else
            sprintf( acl_str, "%s\n\n", catgets( Dtb_project_catd, 3, 37, "Change will remove the existing ACL entries and replace with the new ACL entries." ) );
    }
    else
        strcpy( acl_str, "" );

    msg_str[0] = '\0';
    if ( is_dir ) 
    {
        sprintf( msg_str, "%s %s\n\n",
            catgets( Dtb_project_catd, 3, 2, "Folder:" ), file );
        strcat( msg_str, acl_str );
        strcat( msg_str, catgets( Dtb_project_catd, 3, 39,
            "Do you want to change the permissions for this folder?" ) );
    }
    else
    {
        sprintf( msg_str, "%s %s\n\n",
            catgets( Dtb_project_catd, 3, 1, "File:" ), file );
        strcat( msg_str, acl_str );
        strcat( msg_str, catgets( Dtb_project_catd, 3, 38,
            "Do you want to change the permissions for this file?" ) );
    }

    label_xmstring = XmStringCreateLocalized( msg_str );
    XtVaSetValues( instance->label,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );

    XtManageChild( instance->dialog );

    answer = ANSWER_NONE;
    while( answer == ANSWER_NONE )
        XtAppProcessEvent( proginfo.appContext, XtIMXEvent | XtIMAll );

    XtUnmanageChild( instance->dialog );
    XSync( proginfo.display, 0 );

    return( answer );
}


/*
 * Open the necessary shared objects to initialize the ACL support funcions,
 * i.e. acl(), aclcheck(), aclsort().
 * Return True if successfull, False on error.
 */

Boolean
LoadDynLibForACL()
{
    void *libc_handle;		/* Object contains ACL routine */
    void *libsec_handle;	/* Object contains ACL supplement routines */

    /*
     * Open the needed library, if we are unable to open the libraries then
     * assume that ACL is not supported on this system.
     */
    if ( ( libc_handle = dlopen( "libc.so", RTLD_LAZY ) ) == 0 )
    {
        return( False );
    }
    if ( ( libsec_handle = dlopen( "libsec.so", RTLD_LAZY ) ) == 0 )
    {
        return( False );
    }

    /* Get the address of the ACL functions */
    if ( ( acl_fptr = ( int (*)( char *, int, int, aclent_t* ) )
        dlsym( libc_handle, "acl" ) ) == 0 )
    {
        return( False );
    }

    if ( ( aclcheck_fptr = ( int (*)( aclent_t*, int, int* ) )
        dlsym( libsec_handle, "aclcheck" ) ) == 0 )
    {
        return( False );
    }

    if ( ( aclsort_fptr = ( int (*)( int, int, aclent_t* ) )
        dlsym( libsec_handle, "aclsort" ) ) == 0 )
    {
        return( False );
    }

    return( True );
}


int
LoadNewFile(
    char *file_name )
{
    Boolean	is_successful = False;
    FileData	*new = NULL;

    /*
     * First check to see if the file is valid before doing any works...
     */
    proginfo.saved_errno = 0;

    if ( ( new = GetFileInfo( file_name ) ) != NULL )
    {
        /* Free the current data and assign it to the new data */
        FreeFileData( proginfo.current_fdata );
        proginfo.current_fdata = new;

        if ( proginfo.directory != NULL )
            XtFree( proginfo.directory );
        proginfo.directory = GetDirName( new->file_name ); 

        /* Initialize the UI */
        SetPropDialogValues( proginfo.current_fdata );

        is_successful = True;
    }

    if ( !is_successful )
    {
        ProcessErrorMsg( ERROR, True, "%s\n\n%s",
            file_name,
            catgets( Dtb_project_catd, 4, 1, "Unable to load file properties." ) );
        return( -1 );
    }

    return( 0 );
}


/*
 * Get the relevant error message based on errno value.
 */

char *
GetErrnoErrorMsg(
    int err )
{
    char *err_msg = NULL;

    switch ( err )
    {
        case EPERM:
        case EINVAL:
            err_msg = catgets( Dtb_project_catd, 4, 11, "Only the owner of this file or folder can change its permissions." );
            break;

        case ENOENT:
        case ENOTDIR:
            err_msg = catgets( Dtb_project_catd, 4, 17, "The specified path name does not exist." );
            break;

        case EIO:
            err_msg = catgets( Dtb_project_catd, 4, 21, "A disk I/O error has occurred while storing\nor retrieving the ACL for this file or folder." ); 
            break;

        case EACCES:
            err_msg = catgets( Dtb_project_catd, 4, 16, "Access permission is denied for the specified path name." );
            break;

        case ENOSPC:
            err_msg = catgets( Dtb_project_catd, 4, 22, "There is insufficient space in the file system to store the ACLs." );
            break;

        case EROFS:
            err_msg = catgets( Dtb_project_catd, 4, 19, "The specified file or folder resides on a\nfile system that is mounted read-only." );
            break;

        case ENOSYS:
            err_msg = catgets( Dtb_project_catd, 4, 20, "The specified file or folder resides on a\nfile system that does not support ACLs." );
            break;

        case ELOOP:
            err_msg = catgets( Dtb_project_catd, 4, 18, "The specified path name contains a loop of symbolic links." );
            break;

        default:
            err_msg = strerror( err );
    }

    return( err_msg );
}


/*
 * Display error message in the message dialog box if top level window
 * has been initialized otherwise dump it to stderr.
 * At a minimum, this function expects 3 arguments:
 *	o Severity of the error
 *	o A boolean value to indicate whether if the error is a system error 
 *	o Error message (can be NULL if the error is a system error)
 */

void
ProcessErrorMsg(
    int severity,
    Boolean is_errno,
    ... )
{
    va_list	ap;
    int		dialog_type;
    char	tmp_str[BUF_SIZ];
    char	err_str[BUF_SIZ];
    String	sys_string = NULL;
    String	msg_string = NULL;
    String	title_string = NULL;
    Boolean	is_graphical = XtIsRealized( proginfo.topLevel );

    va_start( ap, is_errno );

    msg_string = va_arg( ap, char * );

    /* Get the relevant error message based on errno value */
    if ( is_errno )
    {
        sys_string = GetErrnoErrorMsg( proginfo.saved_errno );
        
        /* Create a new message string to include the system error message */
        if ( msg_string != NULL )
        {
            tmp_str[0] = '\0';
            sprintf( tmp_str, "%s\n\n%s", msg_string, sys_string ); 
            msg_string = tmp_str;
        }
        else
            msg_string = sys_string;
    }

    if ( msg_string == NULL )
        msg_string = "Internal error!"; 

    err_str[0] = '\0';
    vsprintf( err_str, msg_string, ap );

    /*
     * If top level window has been initialized then we can display the
     * error message in the message dialog else dump it to stderr.
     */
    if ( is_graphical )
    {
        switch( severity )
        {
            case FATAL:
            case ERROR:
                dialog_type = XmDIALOG_ERROR;
                title_string = catgets( Dtb_project_catd, 3, 21, "Properties : Error" );
                break;

            case WARNING:
                title_string = catgets( Dtb_project_catd, 3, 20, "Properties : Warning" );
                dialog_type = XmDIALOG_WARNING;
                break;

            case INFORMATION:
            default:
                title_string = catgets( Dtb_project_catd, 3, 19, "Properties : Information" );
                dialog_type = XmDIALOG_INFORMATION;
        }

        AlertPrompt( proginfo.topLevel,
                     DIALOG_TYPE, dialog_type,
                     DIALOG_TITLE, title_string,
                     DIALOG_STYLE, XmDIALOG_PRIMARY_APPLICATION_MODAL,
                     DIALOG_TEXT, err_str,
                     BUTTON_IDENT, ANSWER_ACTION_1,
                     catgets( Dtb_project_catd, 2, 20, "OK" ),
                     NULL );
    }
    else
    {
#ifdef DEBUG
        fflush( stdout );	/* In case stdout and stderr are the same */
        fputs( err_str, stderr );
        fflush( NULL );		/* Flushes all stdio output streams */
#endif
        /*
         * We will use dterror.ds to output the error message, but we will
         * need to subtitute all occurance of '\n' to '\\n' inorder for it
         * to work correctly.
         */
        msg_string = SearchAndReplace( err_str, "\n", "\\n" );

        tmp_str[0] = '\0';
        sprintf( tmp_str,
            "dterror.ds \"%s\" \"%s\" \"%s\"",
            msg_string,
            catgets( Dtb_project_catd, 3, 21, "Properties : Error" ),
            catgets( Dtb_project_catd, 2, 20, "OK" ) );
        system( tmp_str ); 

        XtFree( msg_string );
    }

    va_end( ap );

    if ( severity == FATAL )
        exit( proginfo.saved_errno );
    else
        return;
}


void
SetModDialogFooter(
    char *message )
{
    DtbSdtfpropModdialogInfo instance = &dtb_sdtfprop_moddialog;

    XmString label_xmstring = XmStringCreateLocalized( message );

    XtVaSetValues( instance->mod_footerLabel,
        XmNlabelString, label_xmstring,
        NULL );
    XmStringFree( label_xmstring );
}


/*
 * Do any tailoring to the propdialog's UI which we can't do in the builder. 
 */

void 
PropDialogPostCreateProc(
    DtbSdtfpropPropdialogInfo instance )
{
    static	first_time = True;
    char	tmp_buf[TOTAL_FIELD_SIZE + 1];
    XmString	label_xmstring = NULL;
    XmFontList	font_list = NULL;
    Dimension	width, max_width, height, offset;

    if ( first_time )
    {
        first_time = False;

        /*
         * Align the nameLabel width with the linkLabel's width
         */
        XtVaGetValues( instance->nameLabel,
            XmNwidth, &max_width,
            NULL );
        XtVaGetValues( instance->linkLabel,
            XmNwidth, &width,
            NULL );
        if ( width > max_width )
            max_width = width;

        XtVaSetValues( instance->nameLabel,
            XmNwidth, max_width,
            NULL );
        XtVaSetValues( instance->linkLabel,
            XmNwidth, max_width,
            NULL );

        /*
         * Create an icon image for the file 
         */
        proginfo.iconType =
            _DtCreateIcon( instance->objectNameCpane, "", NULL, 0 );
        XtVaSetValues( proginfo.iconType,
            XmNbottomAttachment, XmATTACH_NONE,
            XmNrightOffset, 0,
            XmNrightAttachment, XmATTACH_FORM,
            XmNleftAttachment, XmATTACH_NONE,
            XmNtopOffset, 0,
            XmNtopAttachment, XmATTACH_FORM,
            XmNalignment, XmALIGNMENT_CENTER,
            XmNshadowThickness, 2,
            XmNshadowType, XmSHADOW_ETCHED_IN,
            XmNhighlightThickness, 5,
            XmNmarginWidth, 4,
            XmNmarginHeight, 4,
            XmNtraversalOn, False,
            XmNpixmapPosition, XmPIXMAP_TOP,
            XmNbehavior, XmICON_LABEL,
            NULL );
        XtManageChild( proginfo.iconType );

        /* Set menu functions and frame decor. */
        XtVaSetValues( instance->propdialog,
            XmNmwmDecorations, MWM_DECOR_BORDER |
                               MWM_DECOR_TITLE |
                               MWM_DECOR_MENU |
                               MWM_DECOR_MINIMIZE,
            XmNmwmFunctions,   MWM_FUNC_MOVE |
                               MWM_FUNC_CLOSE |
                               MWM_FUNC_MINIMIZE,
            NULL );

        if ( proginfo.is_browsable )
        {
            /* Add callback to respond to new file name being entered. */
            XtAddCallback( instance->fileNameTfield,
                XmNactivateCallback, FileNameChanged_CB, (XtPointer)&(*instance) );
        }
        else
        {
            /* Unmanage browseButton and adjust layout */
            XtUnmanageChild( instance->browseButton );
            XtVaSetValues( instance->fileNameTfield,
                XmNrightAttachment, XmATTACH_FORM,
                XmNrightOffset, 0,
                NULL );

            /* The file name text filed should not be editable */
            XmTextFieldSetEditable( instance->fileNameTfield, False );
        }

        /*
         * Attatch the fileNameGroup & linkNameGroup object to iconType object
         */
        XtVaSetValues( instance->fileNameGroup,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, proginfo.iconType,
            XmNrightOffset, 5,
            NULL );
        XtVaSetValues( instance->linkNameGroup,
            XmNrightAttachment, XmATTACH_WIDGET,
            XmNrightWidget, proginfo.iconType,
            XmNrightOffset, 5,
            NULL );

        /*
         * The linkNameGroup object should not be visible and editable 
         */
        XtSetMappedWhenManaged( instance->linkNameGroup, False );
        XmTextFieldSetEditable( instance->linkNameTfield, False );

        /*
         * Re-do layout adjustment for Basic Permissions section by
         * align items in the center of the column.
         */
        XtVaGetValues( instance->ownerRead_items.nolabel_item,
            XmNwidth, &width,
            NULL );
        XtVaGetValues( instance->readLabel,
            XmNwidth, &max_width,
            NULL );
        offset = (int)( max_width - width ) / 2;

        XtVaSetValues( instance->ownerRead,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->groupRead,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->otherRead,
            XmNleftOffset, offset,
            NULL );

        XtVaGetValues( instance->writeLabel,
            XmNwidth, &max_width,
            NULL );
        offset = (int)( max_width - width ) / 2;

        XtVaSetValues( instance->ownerWrite,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->groupWrite,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->otherWrite,
            XmNleftOffset, offset,
            NULL );

        XtVaGetValues( instance->executeLabel,
            XmNwidth, &max_width,
            NULL );
        offset = (int)( max_width - width ) / 2;

        XtVaSetValues( instance->ownerExecute,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->groupExecute,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->otherExecute,
            XmNleftOffset, offset,
            NULL );

        XtVaGetValues( instance->ownerEffectLabel,
            XmNwidth, &width,
            NULL );
        XtVaGetValues( instance->effectLabel,
            XmNwidth, &max_width,
            NULL );
        offset = (int)( max_width - width ) / 2;

        XtVaSetValues( instance->ownerEffectLabel,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->groupEffectLabel,
            XmNleftOffset, offset,
            NULL );
        XtVaSetValues( instance->otherEffectLabel,
            XmNleftOffset, offset,
            NULL );

        /*
         * Change XmNscrollBarDisplayPolicy to display the horizontal
         * scrollbar only when needed for the accessList object
         */
        XtVaSetValues( instance->accessList,
            XmNscrollBarDisplayPolicy, XmSTATIC, 
            NULL );

        /*
         * Change to fixed font for the accessList object and align
         * all the label objects for the accessList
         */
        XtVaSetValues( instance->accessList,
            XmNfontList, dtb_app_resource_rec.user_font, 
            NULL );
        XtVaGetValues( instance->accessList,
            XmNfontList, &font_list, 
            NULL );

        width = 5;
        width += CvtStringLengthToPixel(
            font_list, C_FIELD_SIZE );
        XtVaSetValues( instance->nameListLabel,
            XmNleftOffset, width, 
            NULL );
       
        width += CvtStringLengthToPixel(
            font_list, N_FIELD_SIZE );
        XtVaSetValues( instance->permListLabel,
            XmNleftOffset, width, 
            NULL );

        width += CvtStringLengthToPixel(
            font_list, P_FIELD_SIZE );
        XtVaSetValues( instance->effectListLabel,
            XmNleftOffset, width, 
            NULL );

        /*
         * Re-compute the width for all buttons in the ACL Permissions section
         */
        XtVaGetValues( instance->addButton,
            XmNwidth, &max_width,
            NULL );
        XtVaGetValues( instance->deleteButton,
            XmNwidth, &width,
            NULL );
        if ( width > max_width )
            max_width = width;
        XtVaGetValues( instance->changeButton,
            XmNwidth, &width,
            NULL );
        if ( width > max_width )
            max_width = width;

        max_width += 10;	/* Add some padding */
        XtVaSetValues( instance->addButton,
            XmNwidth, max_width,
            NULL );
        XtVaSetValues( instance->deleteButton,
            XmNwidth, max_width,
            NULL );
        XtVaSetValues( instance->changeButton,
            XmNwidth, max_width,
            NULL );

        /*
         * Set dialog minimum width size based on estimate of the length of
         * the string in the list, the width of the button, and some padding.
         */
        width = CvtStringLengthToPixel(
            font_list, TOTAL_FIELD_SIZE ) + max_width + 50;
        XtVaSetValues( instance->propLayers,
            XmNwidth, width,
            NULL );

        /*
         * Set the action area attachment.
         */
        XtVaSetValues( instance->propDialogBpanel,
            XmNbottomAttachment, XmATTACH_NONE,
            XmNrightOffset, 0,
            XmNrightPosition, 100,
            XmNrightAttachment, XmATTACH_POSITION,
            XmNleftOffset, 0,
            XmNleftPosition, 0,
            XmNleftAttachment, XmATTACH_POSITION,
            XmNtopOffset, 0,
            XmNtopAttachment, XmATTACH_WIDGET,
            XmNtopWidget, instance->propLayers,
            NULL);

        /* Don't show the ACL permissions section be default */
        HideACL();
    }
}


void
ShowACL()
{
    DtbSdtfpropPropdialogInfo instance = &dtb_sdtfprop_propdialog;

    static XmString label_xmstring = NULL;

    /*
     * Create a label for the aclButton if we haven't done so
     */
    if ( label_xmstring == NULL )
        label_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 2, 45, "Hide Access List" ) );

    /*
     * To show the ACL permission pane, we need to change the attatchment
     * of the dialog button panel to the ACL permission pane.
     */
    XtVaSetValues( instance->applyCpane,
        XmNtopWidget, instance->aclPermCpane,
        NULL );

    XtManageChild( instance->aclPermCpane );
    XtManageChild( instance->aclLabel );

    /*
     * If we are showing the ACL pane then label should be set to hide
     */
    XtVaSetValues( instance->aclButton,
        XmNlabelString, label_xmstring,
        NULL );
}


void
HideACL()
{
    DtbSdtfpropPropdialogInfo instance = &dtb_sdtfprop_propdialog;

    static XmString label_xmstring = NULL;

    /*
     * Create a label for the aclButton if we haven't done so
     */
    if ( label_xmstring == NULL )
        label_xmstring = XmStringCreateLocalized(
            catgets( Dtb_project_catd, 3, 10, "Show Access Control List" ) );

    /*
     * To hide the ACL permission pane, we need to change the attatchment
     * of the dialog button panel to the basic permission pane.
     */
    XtVaSetValues( instance->applyCpane,
        XmNtopWidget, instance->basicPermCpane,
        NULL );

    XtUnmanageChild( instance->aclPermCpane );
    XtUnmanageChild( instance->aclLabel );

    /*
     * If we are hiding the ACL pane then label should be set to show
     */
    XtVaSetValues( instance->aclButton,
        XmNlabelString, label_xmstring,
        NULL );
}


void
ProcessRecursiveError(
    Boolean *quit,
    ... )
{
    va_list	ap;
    char	err_str[BUF_SIZ];
    char	*msg_string = NULL;
    int		answer;

    va_start( ap, quit );

    msg_string = va_arg( ap, char * );

    if ( msg_string == NULL )
        msg_string = "Internal error!";

    err_str[0] = '\0';
    vsprintf( err_str, msg_string, ap );

    answer = AlertPrompt( proginfo.topLevel,
        DIALOG_TYPE, XmDIALOG_ERROR,
        DIALOG_TITLE,
        catgets( Dtb_project_catd, 3, 21, "Properties : Error" ),
        DIALOG_STYLE, XmDIALOG_PRIMARY_APPLICATION_MODAL,
        DIALOG_TEXT, err_str,
        BUTTON_IDENT, ANSWER_ACTION_1,
        catgets( Dtb_project_catd, 3, 45, "Continue" ),
        BUTTON_IDENT, ANSWER_CANCEL,
        catgets( Dtb_project_catd, 2, 21, "Cancel" ),
        HELP_IDENT,
        catgets( Dtb_project_catd, 3, 101, "This dialog box appears when an error has occurred while\nprocessing the Apply Changes To operation.\n\nContinue\n\tCloses the dialog box, and continues with the Apply\n\tChanges To operation.\n\nCancel\n\tCloses the dialog box, terminates the Apply Changes\n\tTo operation, and returns to the Permissions dialog box.\n\nHelp\n\tDisplays help about this dialog box." ),
        NULL );

    if ( answer == ANSWER_CANCEL )
        *quit = True;	/* Cancel recursive operation */
}


int
WalkFileTree(
    char *path,
    int depth,
    Boolean *quit )
{
    char		full_path[MAXPATHLEN];
    char		*real_file_name;
    char		*ptr;
    struct dirent	*dirp;
    DIR			*dp;
    FileData		*d_fdata = NULL;
    FileData		*e_fdata = NULL;
    int			rc;

    proginfo.saved_errno = 0;

    if ( ( d_fdata = GetFileInfo( path ) ) == NULL )
    {
        ProcessRecursiveError( quit, "%s %s\n\n%s\n\n%s",
            catgets( Dtb_project_catd, 3, 2, "Folder:" ),
            path,
            catgets( Dtb_project_catd, 4, 13, "Files or folders contains in this folder could not be changed." ),
            GetErrnoErrorMsg( proginfo.saved_errno ) );

        return( -1 );
    }

    /*
     * Not a directory
     */
    if ( S_ISDIR( d_fdata->stat->st_mode ) == 0 )
    {
        rc = -1;
        goto quit;
    }

    /*
     * It's a directory, so check to see if we are supposed to do any work...
     */
    if ( d_fdata->link_name == NULL )
        real_file_name = d_fdata->file_name;
    else
        real_file_name = d_fdata->link_name;

    if ( proginfo.is_d_recursive )
    {
        if ( ( rc = ConfirmAndUpdateFile( d_fdata, quit ) ) != 0 )
        {
            ProcessRecursiveError( quit, "%s %s\n\n%s\n\n%s",
                catgets( Dtb_project_catd, 3, 2, "Folder:" ),
                real_file_name,
                catgets( Dtb_project_catd, 4, 12, "The permissions on this file or folder could not be changed." ),
                GetErrnoErrorMsg( proginfo.saved_errno ) );
        }
    }

    /*
     * Check to see if we are supposed to process this directory,
     * and don't follow the link.
     */
    if ( ( depth <= 0 ) || ( d_fdata->link_name != NULL ) || ( *quit == True ) )
    {
        rc = 0;
        goto quit;
    }

    /*
     * Now, let process each filename in this directory. 
     */
    strcpy( full_path, path );
    ptr = full_path + strlen( full_path );
    *ptr++ = '/'; 
    *ptr = '\0'; 

    /*
     * Cannot read or search directory
     */
    dp = opendir( path );
    if ( ( dp == NULL ) || ( access( path, X_OK ) != 0 ) )
    {
        proginfo.saved_errno = errno;
        ProcessRecursiveError( quit, "%s %s\n\n%s\n\n%s",
            catgets( Dtb_project_catd, 3, 2, "Folder:" ),
            real_file_name,
            catgets( Dtb_project_catd, 4, 13, "Files or folders contains in this folder could not be changed." ),
            GetErrnoErrorMsg( proginfo.saved_errno ) );
        rc = -1;
        goto quit;
    }
     
    while ( ( dirp = readdir( dp ) ) != NULL )
    {
        if ( ( strcmp( dirp->d_name, "." ) == 0 ) ||
             ( strcmp( dirp->d_name, ".." ) == 0 ) )
            continue;			/* Ignore "." and ".." */

        strcpy( ptr, dirp->d_name );	/* Append name after "/" */

        proginfo.saved_errno = 0;

        if ( ( e_fdata = GetFileInfo( full_path ) ) == NULL )
        {
            ProcessRecursiveError( quit, "%s %s\n\n%s\n\n%s",
                catgets( Dtb_project_catd, 3, 1, "File:" ),
                full_path,
                catgets( Dtb_project_catd, 4, 12, "The permissions on this file or folder could not be changed." ),
                GetErrnoErrorMsg( proginfo.saved_errno ) );

            if ( *quit == True )
                break;		/* Time to leave */

            continue;	/* Process next entry */
        }

        if ( S_ISDIR( e_fdata->stat->st_mode ) )
        {
            rc = WalkFileTree( full_path, depth - 1, quit );
        }
        else
        {
            if ( proginfo.is_f_recursive )
            {
                if ( e_fdata->link_name == NULL )
                    real_file_name = e_fdata->file_name;
                else
                    real_file_name = e_fdata->link_name;

                if ( ( rc = ConfirmAndUpdateFile( e_fdata, quit ) ) != 0 )
                {
                    ProcessRecursiveError( quit, "%s %s\n\n%s\n\n%s",
                        catgets( Dtb_project_catd, 3, 1, "File:" ),
                        real_file_name,
                        catgets( Dtb_project_catd, 4, 12, "The permissions on this file or folder could not be changed." ),
                        GetErrnoErrorMsg( proginfo.saved_errno ) );
                }
            }
        }
        
        FreeFileData( e_fdata );	/* Free the data for this entry */

        if ( *quit == True )
            break;			/* Time to leave */
    }

    if ( closedir( dp ) < 0 )
    {
        proginfo.saved_errno = errno;
        ProcessRecursiveError( quit, "%s %s\n\n%s",
            catgets( Dtb_project_catd, 3, 2, "Folder:" ),
            path,
            GetErrnoErrorMsg( proginfo.saved_errno ) );
        rc = -1;
    }

quit:
    FreeFileData( d_fdata );	/* Free the data for this directory entry */

    return( rc );
}


void
ProcessPermissionChange()
{
    Boolean	quit = False;
    int		rc, update_o_rc;
    int		answer;
    char	*real_file_name;
    FileData	*new = NULL;
    
    /* Put up busy cursor */
    _DtTurnOnHourGlass( proginfo.topLevel );

    /* Get the real name */
    if ( (proginfo.current_fdata)->link_name == NULL )
        real_file_name = (proginfo.current_fdata)->file_name;
    else
    {
        real_file_name = (proginfo.current_fdata)->link_name;

        /* Make sure the source and target file exists before doing any work */
        if ( access( (proginfo.current_fdata)->link_name, F_OK ) != 0 ) 
        {
            ProcessErrorMsg( ERROR, False, "%s\n\n%s\n\n%s",
                (proginfo.current_fdata)->file_name,
                catgets( Dtb_project_catd, 4, 12,
                    "The permissions on this file or folder could not be changed." ),
                catgets( Dtb_project_catd, 4, 25,
                    "Symbolic link points to a file or folder that is no longer exist.\nSomeone has deleted it from your system." ) );

            goto permission_change_done;
        }
        else if ( access( (proginfo.current_fdata)->file_name, F_OK ) != 0 )
        {
            ProcessErrorMsg( ERROR, False, "%s\n\n%s\n\n%s",
                (proginfo.current_fdata)->file_name,
                catgets( Dtb_project_catd, 4, 12,
                    "The permissions on this file or folder could not be changed." ),
                catgets( Dtb_project_catd, 4, 26,
                    "Symbolic link no longer exists.\nSomeone has deleted it from your system." ) );

            goto permission_change_done;
        }
    }

    proginfo.saved_errno = 0;

    if ( proginfo.is_f_recursive || proginfo.is_d_recursive )
    {
        /* Cancel is pressed */
        if ( answer = RecursiveConfirmPrompt() != 1 )
            goto permission_change_done;

        /* Update file ownership */
        update_o_rc = UpdateFileOwnership( real_file_name,
                                           (proginfo.current_fdata)->stat,
                                           (proginfo.new_fdata)->stat );

        if ( update_o_rc > 0 )
        {
            ProcessErrorMsg( ERROR, True, "%s %s\n\n%s",
                S_ISDIR( (proginfo.current_fdata)->stat->st_mode ) ?
                    catgets( Dtb_project_catd, 3, 2, "Folder:" ) :
                    catgets( Dtb_project_catd, 3, 1, "File:" ),
                real_file_name,
                catgets( Dtb_project_catd, 4, 14,
                    "The ownership could not be changed." ) );

            goto permission_change_done;
        }

        if ( proginfo.is_f_recursive )
        {
            /* Start from the current directory of this file */
            if ( proginfo.directory != NULL )
                WalkFileTree( proginfo.directory, proginfo.recursive_depth, &quit );
            else
                WalkFileTree( ".", proginfo.recursive_depth, &quit );
        }
        else
            WalkFileTree( real_file_name, proginfo.recursive_depth, &quit );

        /* Put up a dialog to inform the process has completed */
        if ( !quit )
        {
            AlertPrompt( proginfo.topLevel,
                         DIALOG_TYPE, XmDIALOG_INFORMATION,
                         DIALOG_TITLE,
                         catgets( Dtb_project_catd, 3, 23,
                             "Properties : Permissions" ),
                         DIALOG_STYLE, XmDIALOG_PRIMARY_APPLICATION_MODAL,
                         DIALOG_TEXT,
                         catgets( Dtb_project_catd, 3, 42, "Changes complete" ),
                         BUTTON_IDENT, ANSWER_ACTION_1,
                         catgets( Dtb_project_catd, 2, 20, "OK" ),
                         NULL );
        }
    }
    else
    {
        /* Update file ownership */
        update_o_rc = UpdateFileOwnership( real_file_name,
                                           (proginfo.current_fdata)->stat,
                                           (proginfo.new_fdata)->stat );

        if ( update_o_rc > 0 )
        {
            ProcessErrorMsg( ERROR, True, "%s %s\n\n%s",
                S_ISDIR( (proginfo.current_fdata)->stat->st_mode ) ?
                    catgets( Dtb_project_catd, 3, 2, "Folder:" ) :
                    catgets( Dtb_project_catd, 3, 1, "File:" ),
                real_file_name,
                catgets( Dtb_project_catd, 4, 14,
                    "The ownership could not be changed." ) );

            goto permission_change_done;
        }

        /* Update file permissions */
        rc = UpdateFilePermission( real_file_name,
                                   (proginfo.current_fdata)->stat,
                                   (proginfo.current_fdata)->acl,
                                   (proginfo.new_fdata)->stat,
                                   (proginfo.new_fdata)->acl,
                                   False );

        if ( rc != 0 )
        {
            if ( update_o_rc == -1 )
            {
                ProcessErrorMsg( ERROR, True, "%s %s\n\n%s",
                    S_ISDIR( (proginfo.current_fdata)->stat->st_mode ) ?
                        catgets( Dtb_project_catd, 3, 2, "Folder:" ) :
                        catgets( Dtb_project_catd, 3, 1, "File:" ),
                    real_file_name,
                    catgets( Dtb_project_catd, 4, 12,
                        "The permissions on this file or folder could not be changed." ) );
            }
            else
            {
                ProcessErrorMsg( ERROR, True, "%s %s\n\n%s",
                    S_ISDIR( (proginfo.current_fdata)->stat->st_mode ) ?
                        catgets( Dtb_project_catd, 3, 2, "Folder:" ) :
                        catgets( Dtb_project_catd, 3, 1, "File:" ),
                    real_file_name,
                    catgets( Dtb_project_catd, 4, 15,
                    "The ownership has been changed but the permissions could not be changed." ) );
            }
        }
        else
        {
            /* Reload the file info from disk */
            new = GetFileInfo( (proginfo.current_fdata)->file_name );
            if ( new != NULL )
            {
                /* Free the current data and assign it to the new data */
                FreeFileData( proginfo.current_fdata );
                proginfo.current_fdata = new;
            }
            else
            {
                ProcessErrorMsg( ERROR, True, "%s\n\n%s",
                    (proginfo.current_fdata)->file_name,
                    catgets( Dtb_project_catd, 4, 1, "Unable to load file properties." ) );
            }
        }
    }

permission_change_done:

    /* Remove the busy cursor */
    _DtTurnOffHourGlass( proginfo.topLevel );

    return;
}


/*
 * Compare the current FileData info with the new FileData info.
 * Return True if they are the same and False if they are not. 
 */

Boolean
ComparePermissionData(
    FileData *old,
    FileData *new )
{
    register int	i;

    if ( ( old == NULL ) || ( new == NULL ) )
        return( False );

    /* Check for changes in file ownership */
    if ( ( old->stat->st_uid != new->stat->st_uid ) ||
         ( old->stat->st_gid != new->stat->st_gid ) )
        return( True );

    /*
     * Check for changes in file permissions; use ACL if it is defined
     * otherwise use regular UNIX permissions.
     */
    if ( old->acl != NULL )
    {
        aclent_t *old_bufp = old->acl->bufp;
        aclent_t *new_bufp = new->acl->bufp;

        /* First check for the obvious changes like number of ACL entries */
        if ( old->acl->nentries == new->acl->nentries )
        {
            /* Do the comparision */
            for ( i = 0 ; i < new->acl->nentries ; i++ )
            {
                if ( ( old_bufp[i].a_type == USER ) || 
                     ( old_bufp[i].a_type == GROUP ) || 
                     ( old_bufp[i].a_type == DEF_USER ) || 
                     ( old_bufp[i].a_type == DEF_GROUP ) )
                {
                    if ( ( old_bufp[i].a_type != new_bufp[i].a_type )||
                         ( old_bufp[i].a_id   != new_bufp[i].a_id ) ||
                         ( old_bufp[i].a_perm != new_bufp[i].a_perm ) )
                        return( True );
                }
                else
                {
                    if ( ( old_bufp[i].a_type != new_bufp[i].a_type )||
                         ( old_bufp[i].a_perm != new_bufp[i].a_perm ) )
                        return( True );
                }
            }
            return( False );
        }
        else
            return( True );
    }
    else
    {
        if ( ( old->stat->st_mode & 0777 ) != ( new->stat->st_mode & 0777 ) )
            return( True );
    }

    return( False );
}


/*
 * Check for any changes in the permission pane and ask user for their
 * respond to save the changes or to continue with the process only
 * if verify is True, otherwise just save the changes.
 * Return True to signal caller to continue with the process and False
 * to cancel the process.
 */

Boolean
ProcessPermissionPaneExit(
    Boolean verify )
{
    int answer;

    /* Before we doing any work check to see if there is a file loaded */
    if ( proginfo.current_fdata == NULL )
        return( True );

    /* Get the new FileData structure from the UI */
    if ( proginfo.new_fdata != NULL )
        FreeFileData( proginfo.new_fdata );
    if ( ( proginfo.new_fdata = GetPropDialogValues() ) == NULL )
        return( True );

    /* Need to check if there is any changes */
    if ( verify )
    {
        if ( ComparePermissionData(proginfo.current_fdata, proginfo.new_fdata) )
        {
            answer = AlertPrompt( proginfo.topLevel,
                DIALOG_TYPE, XmDIALOG_QUESTION,
                DIALOG_TITLE,
                catgets( Dtb_project_catd, 3, 17, "Properties : Unsaved Changes" ),
                DIALOG_STYLE, XmDIALOG_PRIMARY_APPLICATION_MODAL,
                DIALOG_TEXT,
                catgets( Dtb_project_catd, 3, 41, "You have unsaved changes.\n\nDo you want to save changes before leaving Permissions dialog box?" ),
                BUTTON_IDENT, ANSWER_ACTION_1,
                catgets( Dtb_project_catd, 3, 4, "Yes" ),
                BUTTON_IDENT, ANSWER_ACTION_2,
                catgets( Dtb_project_catd, 3, 5, "No" ),
                BUTTON_IDENT, ANSWER_CANCEL,
                catgets( Dtb_project_catd, 2, 21, "Cancel" ),
                HELP_IDENT,
                catgets( Dtb_project_catd, 3, 100, "Changes, which have not been saved, have been made to\nthe currently loaded file.  If you exit Permissions dialog\nbox, those changes will be lost.  You have the option of\ncontinuing with the exit operation or cancelling it.\n\nYes\n\tSaves the current settings.\n\nNo\n\tDo not save the current settings.\n\nCancel\n\tDo not save the current settings and cancel the exit operation.\n\nHelp\n\tDisplays help about this dialog box." ),
                NULL );

            if ( answer == ANSWER_ACTION_1 )
                ProcessPermissionChange();
            else
            if ( answer == ANSWER_CANCEL )
                return( False );
        }
    }
    else
        ProcessPermissionChange();

    return( True );
}


/*
 * Generic callback when asking for Help on any widget.
 */
void
ShowHelpDialog(
    char *volume_name,
    char *location_id )
{
    DtbSdtfpropHelpDialogInfo instance = &dtb_sdtfprop_helpdialog;

    if ( !instance->initialized )
    {
        instance->initialized = True;

        instance->dialog = DtCreateHelpDialog( proginfo.topLevel,
                                               "MainHelpDialog", NULL, 0 );
        instance->dialogShell = XtParent( instance->dialog );

        XtVaSetValues( instance->dialogShell,
            XmNtitle, catgets( Dtb_project_catd, 3, 24, "Properties Help" ),
            NULL );
    }

    XtVaSetValues( instance->dialog,
        DtNhelpType, DtHELP_TYPE_TOPIC,
        DtNhelpVolume, HELP_VOLUME,
        DtNlocationId, location_id,
        NULL );

    XtManageChild( instance->dialog );
}
