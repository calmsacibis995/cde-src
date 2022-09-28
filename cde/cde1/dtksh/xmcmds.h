/* $XConsortium: xmcmds.h /main/cde1_maint/1 1995/07/18 00:59:34 drk $ */
/************************************<+>*************************************
 ****************************************************************************
 **
 **   File:        xmcmds.h
 **
 **   Project:     CDE
 **
 **   Description: Public include file for xmcmds.c
 **
 **
 **   (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992
 **       by Hewlett-Packard Company
 **
 **
 **
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef _Dtksh_xmcmds_h
#define _Dtksh_xmcmds_h


#ifdef _NO_PROTO

extern int toolkit_initialize() ;
extern int toolkit_special_resource() ;
extern int do_XmCreateArrowButton() ;
extern int do_XmCreateArrowButtonGadget() ;
extern int do_XmCreateBulletinBoard() ;
extern int do_XmCreateBulletinBoardDialog() ;
extern int do_XmCreateCascadeButton() ;
extern int do_XmCreateCascadeButtonGadget() ;
extern int do_XmCreateCommand() ;
extern int do_XmCreateDialogShell() ;
extern int do_XmCreateDrawingArea() ;
extern int do_XmCreateDrawnButton() ;
extern int do_XmCreateErrorDialog() ;
extern int do_XmCreateFileSelectionBox() ;
extern int do_XmCreateFileSelectionDialog() ;
extern int do_XmCreateForm() ;
extern int do_XmCreateFormDialog() ;
extern int do_XmCreateFrame() ;
extern int do_XmCreateInformationDialog() ;
extern int do_XmCreateLabel() ;
extern int do_XmCreateLabelGadget() ;
extern int do_XmCreateList() ;
extern int do_XmCreateMainWindow() ;
extern int do_XmCreateMenuBar() ;
extern int do_XmCreateMenuShell() ;
extern int do_XmCreateMessageBox() ;
extern int do_XmCreateMessageDialog() ;
extern int do_XmCreateOptionMenu() ;
extern int do_XmCreatePanedWindow() ;
extern int do_XmCreatePopupMenu() ;
extern int do_XmCreatePromptDialog() ;
extern int do_XmCreatePulldownMenu() ;
extern int do_XmCreatePushButton() ;
extern int do_XmCreatePushButtonGadget() ;
extern int do_XmCreateQuestionDialog() ;
extern int do_XmCreateRadioBox() ;
extern int do_XmCreateRowColumn() ;
extern int do_XmCreateScale() ;
extern int do_XmCreateScrollBar() ;
extern int do_XmCreateScrolledList() ;
extern int do_XmCreateScrolledText() ;
extern int do_XmCreateScrolledWindow() ;
extern int do_XmCreateSelectionBox() ;
extern int do_XmCreateSelectionDialog() ;
extern int do_XmCreateSeparator() ;
extern int do_XmCreateSeparatorGadget() ;
extern int do_XmCreateText() ;
extern int do_XmCreateTextField() ;
extern int do_XmCreateToggleButton() ;
extern int do_XmCreateToggleButtonGadget() ;
extern int do_XmCreateWarningDialog() ;
extern int do_XmCreateWorkArea() ;
extern int do_XmCreateWorkingDialog() ;
extern int do_DtCreateHelpDialog() ;
extern int do_DtCreateHelpQuickDialog() ;
extern int do_DtHelpReturnSelectedWidgetId() ;
extern int do_DtHelpSetCatalogName() ;
extern int do_DtHelpQuickDialogGetChild() ;
extern int do_XmListAddItem() ;
extern int do_XmListAddItemUnselected() ;
extern int do_XmListAddItems() ;
extern int do_XmListAddItemsUnselected() ;
extern int do_XmListReplaceItemsPos() ;
extern int do_XmListReplaceItemsPosUnselected() ;
extern int do_XmListDeleteAllItems() ;
extern int do_XmListDeselectAllItems() ;
extern int do_XmListUpdateSelectedList() ;
extern int do_XmListDeleteItem() ;
extern int do_XmListDeselectItem() ;
extern int do_XmListSetBottomItem() ;
extern int do_XmListSetItem() ;
extern int do_XmListDeletePos() ;
extern int do_XmListDeselectPos() ;
extern int do_XmListSetBottomPos() ;
extern int do_XmListSetHorizPos() ;
extern int do_XmListSetPos() ;
extern int do_XmListDeleteItemsPos() ;
extern int do_XmListDeleteItems() ;
extern int do_XmListDeletePositions() ;
extern int do_XmListGetKbdItemPos() ;
extern int do_XmListGetSelectedPos();
extern int do_XmListGetMatchPos();
extern int do_XmListItemExists() ;
extern int do_XmListItemPos() ;
extern int do_XmListPosSelected() ;
extern int do_XmListPosToBounds() ;
extern int do_XmListSelectItem() ;
extern int do_XmListSelectPos() ;
extern int do_XmListSetAddMode() ;
extern int do_XmListSetKbdItemPos() ;
extern int do_XmMainWindowSetAreas() ;
extern int do_XmMainWindowSep1() ;
extern int do_XmMainWindowSep2() ;
extern int do_XmMainWindowSep3() ;
extern int do_XmProcessTraversal() ;
extern int do_XmInternAtom() ;
extern int do_XmGetAtomName() ;
extern int do_XmGetColors() ;
extern int do_XmUpdateDisplay() ;
extern int do_XmAddWMProtocols() ;
extern int do_XmRemoveWMProtocols() ;
extern int do_XmAddWMProtocolCallback() ;
extern int do_XmRemoveWMProtocolCallback() ;
extern int do_XmMenuPosition() ;
extern int do_XmCommandAppendValue() ;
extern int do_XmCommandError() ;
extern int do_XmCommandSetValue() ;
extern int do_XmCommandGetChild() ;
extern int do_XmMessageBoxGetChild() ;
extern int do_XmFileSelectionBoxGetChild() ;
extern int do_XmSelectionBoxGetChild() ;
extern int do_XmIsTraversable() ;
extern int do_XmScaleGetValue() ;
extern int do_XmScaleSetValue() ;
extern int do_XmScrollBarGetValues() ;
extern int do_XmScrollBarSetValues() ;
extern int do_XmScrollVisible() ;
extern int do_XmToggleButtonGetState() ;
extern int do_XmToggleButtonGadgetGetState() ;
extern int do_XmToggleButtonSetState() ;
extern int do_XmToggleButtonGadgetSetState() ;
extern int do_catopen() ;
extern int do_catclose() ;
extern int do_catgets() ;
extern int do_XmTextDisableRedisplay() ;
extern int do_XmTextEnableRedisplay() ;
extern int do_XmTextPaste() ;
extern int do_XmTextGetEditable() ;
extern int do_XmTextRemove() ;
extern int do_XmTextGetTopCharacter() ;
extern int do_XmTextGetBaseline() ;
extern int do_XmTextGetInsertionPosition() ;
extern int do_XmTextGetLastPosition() ;
extern int do_XmTextGetMaxLength() ;
extern int do_XmTextGetSelection() ;
extern int do_XmTextGetString() ;
extern int do_XmTextSetEditable() ;
extern int do_XmTextSetAddMode() ;
extern int do_XmTextScroll() ;
extern int do_XmTextSetInsertionPosition() ;
extern int do_XmTextSetTopCharacter() ;
extern int do_XmTextSetMaxLength() ;
extern int do_XmTextSetString() ;
extern int do_XmTextShowPosition() ;
extern int do_XmTextClearSelection() ;
extern int do_XmTextCopy() ;
extern int do_XmTextCut() ;
extern int do_XmTextGetSelectionPosition() ;
extern int do_XmTextInsert() ;
extern int do_XmTextPosToXY() ;
extern int do_XmTextReplace() ;
extern int do_XmTextSetSelection() ;
extern int do_XmTextXYToPos() ;
extern int do_XmTextSetHighlight() ;
extern int do_XmTextFindString() ;
extern int do_XmOptionButtonGadget() ;
extern int do_XmOptionLabelGadget() ;
extern int do_XmGetVisibility() ;
extern int do_XmGetTearOffControl() ;
extern int do_XmGetTabGroup() ;
extern int do_XmGetPostedFromWidget() ;
extern int do_XmGetFocusWidget() ;
extern int do_XmFileSelectionDoSearch() ;

#else

extern int toolkit_initialize( 
                        int argc,
                        char *argv[]) ;
extern int toolkit_special_resource( 
                        char *arg0,
                        XtResourceList res,
                        wtab_t *w,
                        wtab_t *parent,
                        classtab_t *class,
                        char *resource,
                        char *val,
                        XtArgVal *ret,
                        int *freeit,
                        Boolean postponePixmaps) ;
extern int do_XmCreateArrowButton( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateArrowButtonGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateBulletinBoard( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateBulletinBoardDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateCascadeButton( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateCascadeButtonGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateCommand( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateDialogShell( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateDrawingArea( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateDrawnButton( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateErrorDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateFileSelectionBox( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateFileSelectionDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateForm( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateFormDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateFrame( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateInformationDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateLabel( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateLabelGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateList( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateMainWindow( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateMenuBar( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateMenuShell( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateMessageBox( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateMessageDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateOptionMenu( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePanedWindow( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePopupMenu( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePromptDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePulldownMenu( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePushButton( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreatePushButtonGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateQuestionDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateRadioBox( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateRowColumn( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateScale( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateScrollBar( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateScrolledList( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateScrolledText( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateScrolledWindow( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateSelectionBox( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateSelectionDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateSeparator( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateSeparatorGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateText( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateTextField( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateToggleButton( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateToggleButtonGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateWarningDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateWorkArea( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCreateWorkingDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_DtCreateHelpDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_DtCreateHelpQuickDialog( 
                        int argc,
                        char *argv[]) ;
extern int do_DtHelpReturnSelectedWidgetId( 
                        int argc,
                        char *argv[]) ;
extern int do_DtHelpSetCatalogName( 
                        int argc,
                        char *argv[]) ;
extern int do_DtHelpQuickDialogGetChild( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListAddItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListAddItemUnselected( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListAddItems( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListAddItemsUnselected( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListReplaceItemsPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListReplaceItemsPosUnselected( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeleteAllItems( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeselectAllItems( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListUpdateSelectedList( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeleteItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeselectItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetBottomItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeletePos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeselectPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetBottomPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetHorizPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeleteItemsPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeleteItems( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListDeletePositions( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListGetKbdItemPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListGetSelectedPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListGetMatchPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListItemExists( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListItemPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListPosSelected( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListPosToBounds( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSelectItem( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSelectPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetAddMode( 
                        int argc,
                        char *argv[]) ;
extern int do_XmListSetKbdItemPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmMainWindowSetAreas( 
                        int argc,
                        char *argv[]) ;
extern int do_XmMainWindowSep1( 
                        int argc,
                        char *argv[]) ;
extern int do_XmMainWindowSep2( 
                        int argc,
                        char *argv[]) ;
extern int do_XmMainWindowSep3( 
                        int argc,
                        char *argv[]) ;
extern int do_XmProcessTraversal( 
                        int argc,
                        char *argv[]) ;
extern int do_XmInternAtom( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetAtomName( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetColors( 
                        int argc,
                        char *argv[]) ;
extern int do_XmUpdateDisplay( 
                        int argc,
                        char *argv[]) ;
extern int do_XmAddWMProtocols( 
                        int argc,
                        char **argv) ;
extern int do_XmRemoveWMProtocols( 
                        int argc,
                        char **argv) ;
extern int do_XmAddWMProtocolCallback( 
                        int argc,
                        char **argv) ;
extern int do_XmRemoveWMProtocolCallback( 
                        int argc,
                        char **argv) ;
extern int do_XmMenuPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCommandAppendValue( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCommandError( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCommandSetValue( 
                        int argc,
                        char *argv[]) ;
extern int do_XmCommandGetChild( 
                        int argc,
                        char *argv[]) ;
extern int do_XmMessageBoxGetChild( 
                        int argc,
                        char *argv[]) ;
extern int do_XmFileSelectionBoxGetChild( 
                        int argc,
                        char *argv[]) ;
extern int do_XmSelectionBoxGetChild( 
                        int argc,
                        char *argv[]) ;
extern int do_XmIsTraversable( 
                        int argc,
                        char *argv[]) ;
extern int do_XmScaleGetValue( 
                        int argc,
                        char *argv[]) ;
extern int do_XmScaleSetValue( 
                        int argc,
                        char *argv[]) ;
extern int do_XmScrollBarGetValues( 
                        int argc,
                        char *argv[]) ;
extern int do_XmScrollBarSetValues( 
                        int argc,
                        char *argv[]) ;
extern int do_XmScrollVisible( 
                        int argc,
                        char *argv[]) ;
extern int do_XmToggleButtonGetState( 
                        int argc,
                        char *argv[]) ;
extern int do_XmToggleButtonGadgetGetState( 
                        int argc,
                        char *argv[]) ;
extern int do_XmToggleButtonSetState( 
                        int argc,
                        char *argv[]) ;
extern int do_XmToggleButtonGadgetSetState( 
                        int argc,
                        char *argv[]) ;
extern int do_catopen( 
                        int argc,
                        char **argv) ;
extern int do_catclose( 
                        int argc,
                        char **argv) ;
extern int do_catgets( 
                        int argc,
                        char **argv) ;
extern int do_XmTextDisableRedisplay( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextEnableRedisplay( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextPaste( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetEditable( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextRemove( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetTopCharacter( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetBaseline( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetInsertionPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetLastPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetMaxLength( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetSelection( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetString( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetEditable( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetAddMode( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextScroll( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetInsertionPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetTopCharacter( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetMaxLength( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetString( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextShowPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextClearSelection( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextCopy( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextCut( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextGetSelectionPosition( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextInsert( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextPosToXY( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextReplace( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetSelection( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextXYToPos( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextSetHighlight( 
                        int argc,
                        char *argv[]) ;
extern int do_XmTextFindString( 
                        int argc,
                        char *argv[]) ;
extern int do_XmOptionButtonGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmOptionLabelGadget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetVisibility( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetTearOffControl( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetTabGroup( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetPostedFromWidget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmGetFocusWidget( 
                        int argc,
                        char *argv[]) ;
extern int do_XmFileSelectionDoSearch( 
                        int argc,
                        char *argv[]) ;

#endif /* _NO_PROTO */


#endif /* _Dtksh_xmcmds_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
