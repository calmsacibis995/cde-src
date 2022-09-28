
/*
 *	$XConsortium: obj_motifdefs.h /main/cde1_maint/2 1995/10/16 10:35:07 rcs $
 *
 * @(#)obj_motifdefs.h	3.15 01/20/96	cde_app_builder/src/libABobj
 *
 * 	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement between
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel without
 *	Sun's specific written approval.  This document and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 */

/*
 * motifdefs.h - motif defines and stuff
 */
#ifndef _ABOBJ_MOTIFDEFS_H_
#define _ABOBJ_MOTIFDEFS_H_

/*************************************************************************
**									**
**		Automatically generated widgets				**
**									**
*************************************************************************/
#define AB_WRAP_TOP_LEVEL		"AB_Toplevel"
#define AB_WRAP_SCR_WIN			"AB_Scr_Win"
#define AB_WRAP_FOOTER			"AB_Footer"
#define AB_WRAP_FOOTER_CH		"AB_Footer_ch"
#define AB_WRAP_RUBBER			"AB_Rubber"
#define AB_WRAP_ABBREV			"AB_Abbrev"
#define AB_WRAP_LABEL			"AB_Label"
#define AB_WRAP_MAIN_WINDOW		"AB_Mainwin"
#define AB_WRAP_MENU_BAR		"AB_Menubar"
#define AB_WRAP_TEXT			"AB_Text"
#define AB_WRAP_EXCL			"AB_Excl"
#define AB_WRAP_NONEXCL			"AB_Nonexcl"
#define AB_WRAP_NUMERIC_FLD		"AB_Numfield"
#define AB_WRAP_CTL_AREA		"AB_Ctlarea"
#define AB_WRAP_BULLETIN		"AB_Bulletin"
#define AB_WRAP_PULLDOWN		"AB_Pulldown"
#define AB_WRAP_ROWCOL			"AB_Rowcol"


/*************************************************************************
**									**
**		Class Names						**
**									**
*************************************************************************/
#define _applicationShell	"applicationShellWidgetClass"
#define _group			"groupWidgetClass" 
#define _topLevelShell		"topLevelShellWidgetClass"
#define _xmArrowButton		"xmArrowButtonWidgetClass"
#define _xmBulletinBoard	"xmBulletinBoardWidgetClass"
#define _xmCascadeButton	"xmCascadeButtonWidgetClass"
#define _xmCommand		"xmCommandWidgetClass"
#define _xmDialogShell		"xmDialogShellWidgetClass"
#define _xmDrawingArea		"xmDrawingAreaWidgetClass"
#define _xmDrawnButton		"xmDrawButtonWidgetClass"
#define _xmFileSelectionBox	"xmFileSelectionBoxWidgetClass"
#define _xmForm			"xmFormWidgetClass"
#define _xmFrame		"xmFrameWidgetClass"
#define _xmLabel		"xmLabelWidgetClass"
#define _xmLabelGadget		"xmLabelGadgetWidgetClass"
#define _xmList			"xmListWidgetClass"
#define _xmMainWindow		"xmMainWindowWidgetClass"
#define _xmManager		"xmManagerWidgetClass"
#define _xmMenuShell		"xmMenuShellWidgetClass"
#define _xmMessageBox		"xmMessageBoxWidgetClass"
#define _xmPanedWindow		"xmPanedWindowWidgetClass"
#define _xmPrimitive		"xmPrimitiveWidgetClass"
#define _xmPushButton		"xmPushButtonWidgetClass"
#define _xmRowColumn		"xmRowColumnWidgetClass"
#define _xmScale		"xmScaleWidgetClass"
#define _xmScrollBar		"xmScrollBarWidgetClass"
#define _xmScrolledWindow	"xmScrolledWindowWidgetClass"
#define _xmSelectionBox		"xmSelectionBoxWidgetClass"
#define _xmSeparator		"xmSeparatorWidgetClass"
#define _xmSeparatorGadget	"xmSeparatorGadgetWidgetClass"
#define _xmText			"xmTextWidgetClass"
#define _xmTextField 		"xmTextFieldWidgetClass"
#define _xmToggleButton		"xmToggleButtonWidgetClass"
#define _xmToggleButtonGadget	"xmToggleButtonGadgetWidgetClass"

/*************************************************************************
**									**
**		Resources						**
**									**
*************************************************************************/

/*
 * Undefine a few commonly used strings that are defined in the
 * Xt files, so that we use the single-copy version.
 *
 * NOTE: Doing this can be dangerous, as the names have an opportunity
 * to get out of synch. with the Xt versions.
 */
#undef XmNeditType
#undef XmNfile
#undef XmNfont
#undef XmNjustify
#undef XmNlabel
#undef XmNorientation
#undef XmNresize
#undef XmNselection
#undef XmNselectionArray
#undef XmNtextSource

#define XmNaccelerator			"XmNaccelerator"
#define XmNaccelerators			"XmNaccelerators"
#define XmNactivateCallback		"XmNactivateCallback"
#define XmNarmCallback			"XmNarmCallback"
#define XmNbackground			"XmNbackground"
#define XmNbackgroundPixmap		"XmNbackgroundPixmap"
#define XmNborderWidth			"XmNborderWidth"
#define XmNcolumns			"XmNcolumns"
#define XmNdestroyCallback		"XmNdestroyCallback"
#define XmNdisarmCallback		"XmNdisarmCallback"
#define XmNdragCallback			"XmNdragCallback"
#define XmNeditMode			"XmNeditMode"
#define XmNeditable			"XmNeditable"
#define XmNentryVerticalAlignment	"XmNentryVerticalAlignment"
#define XmNexposeCallback		"XmNexposeCallback"
#define XmNforeground			"XmNforeground"
#define XmNheight			"XmNheight"
#define XmNhighlightThickness		"XmNhighlightThickness"
#define XmNhorizontalSpacing		"XmNhorizontalSpacing"
#define XmNitemCount 			"XmNitemCount"
#define XmNmaximum			"XmNmaximum"
#define XmNmaxLength			"XmNmaxLength"
#define XmNmenuHistory			"XmNmenuHistory"
#define XmNminimum			"XmNminimum"
#define XmNmodifyVerifyCallback		"XmNmodifyVerifyCallback"
#define XmNnoResize			"XmNnoResize"
#define XmNnumColumns			"XmNnumColumns"
#define XmNorientation			"XmNorientation"
#define XmNpattern			"XmNpattern"
#define XmNpopupCallback		"XmNpopupCallback"
#define XmNpopdownCallback		"XmNpopdownCallback"
#define XmNresizeCallback		"XmNresizeCallback"
#define XmNresizePolicy			"XmNresizePolicy"
#define XmNrows				"XmNrows"
#define XmNrowColumnType		"XmNrowColumnType"
#define XmNselectionPolicy		"XmNselectionPolicy"
#define XmNshadowThickness		"XmNshadowThickness"
#define XmNshadowType			"XmNshadowType"
#define XmNlabelPixmap			"XmNlabelPixmap"
#define XmNtearOffModel			"XmNtearOffModel"
#define XmNvalue			"XmNvalue"
#define XmNvalueChangedCallback		"XmNvalueChangedCallback"
#define XmNvisibleItemCount		"XmNvisibleItemCount"
#define XmNvisualPolicy			"XmNvisualPolicy"
#define XmNverticalSpacing		"XmNverticalSpacing"
#define XmNwidth			"XmNwidth"
#define XmNx				"XmNx"
#define XmNy				"XmNy"
#define XmNiconName			"XmNiconName"
#define XmNiconPixmap			"XmNiconPixmap"
#define XmNiconWindow			"XmNiconWindow"
#define XmNiconMask			"XmNiconMask"
#define XmNiconNameEncoding		"XmNiconNameEncoding"
#define XmNiconX			"XmNiconX"
#define XmNiconY			"XmNiconY"
#define XmNiconic			"XmNiconic"
#define XmNwindowGroup			"XmNwindowGroup"
#define XtNvisual			"XtNvisual"
#define XtNsaveUnder			"XtNsaveUnder"
#define XmNtransient			"XmNtransient"
#define XtNoverrideRedirect		"XtNoverrideRedirect"
#define XtNtransientFor			"XtNtransientFor"
#define XmNallowShellResize		"XmNallowShellResize"
#define XtNcreatePopupChildProc		"XtNcreatePopupChildProc"
#define XmNtitle			"XmNtitle"
#define XmNtitleString			"XmNtitleString"
#define XmNtitleEncoding		"XmNtitleEncoding"
#define XtNargc				"XtNargc"
#define XtNargv				"XtNargv"
#define XtNinput			"XtNinput"
#define XtNinitialState 		"XtNinitialState"
#define XtNgeometry 			"XtNgeometry"
#define XtNbaseWidth 			"XtNbaseWidth"
#define XtNbaseHeight 			"XtNbaseHeight"
#define XtNwinGravity 			"XtNwinGravity"
#define XtNminWidth			"XtNminWidth"
#define XtNminHeight			"XtNminHeight"
#define XtNmaxWidth			"XtNmaxWidth"
#define XtNmaxHeight			"XtNmaxHeight"
#define XtNwidthInc			"XtNwidthInc"
#define XtNheightInc			"XtNheightInc"
#define XtNminAspectY			"XtNminAspectY"
#define XtNmaxAspectY			"XtNmaxAspectY"
#define XtNminAspectX			"XtNminAspectX"
#define XtNmaxAspectX			"XtNmaxAspectX"
#define XtNwmTimeout			"XtNwmTimeout"
#define XtNwaitForWm			"XtNwaitforwm"
#define XtNacceleratorsDoGrab "XtNacceleratorsDoGrab"
#define XmNacceleratorText		"XmNacceleratorText"
#define XtNacceptFocusFunc "XtNacceptFocusFunc"
#define XtNactivateFunc "XtNactivateFunc"
#define XtNadjustBtn "XtNadjustBtn"
#define XtNadjustKey "XtNadjustKey"
#define XtNalign "XtNalign"
#define XtNalignCaptions "XtNalignCaptions"
#define XtNalignHorizontal "XtNalignHorizontal"
#define XtNalignVertical "XtNalignVertical"
#define XtNalignment "XtNalignment"
#define XtNallowRootHelp "XtNallowRootHelp"
#define XtNapplAddItem "XtNapplAddItem"
#define XtNapplDeleteItem "XtNapplDeleteItem"
#define XtNapplEditClose "XtNapplEditClose"
#define XtNapplEditOpen "XtNapplEditOpen"
#define XtNapplTouchItem "XtNapplTouchItem"
#define XtNapplUpdateView "XtNapplUpdateView"
#define XtNapplViewItem "XtNapplViewItem"
#define XtNapply "XtNapply"
#define XtNautoAssertDropsiteRegistry "XtNautoAssertDropsiteRegistry"
#define XtNautoPopup "XtNautoPopup"
#define XtNbeep "XtNbeep"
#define XtNbeepVolume "XtNbeepVolume"
#define XtNblinkRate "XtNblinkRate"
#define XtNborderVisible "XtNborderVisible"
#define XtNbottomMargin "XtNbottomMargin"
#define XtNbtnDown "XtNbtnDown"
#define XtNbtnMotion "XtNbtnMotion"
#define XtNbtnUp "XtNbtnUp"
#define XtNbusy "XtNbusy"
#define XtNbuttonType "XtNbuttonType"
#define XtNbuttons "XtNbuttons"
#define XtNcancelKey "XtNcancelKey"
#define XtNcaption "XtNcaption"
#define XtNcaptionWidth "XtNcaptionWidth"
#define XtNcaptionLabel "XtNcaptionLabel"
#define XtNdeltaState "XtNdeltaState"
#define XtNcenter "XtNcenter"
#define XtNcenterLine "XtNcenterLine"
#define XtNcharBakKey "XtNcharBakKey"
#define XtNcharFwdKey "XtNcharFwdKey"
#define XtNcharsVisible "XtNcharsVisible"
#define XtNclientData "XtNclientData"
#define XtNcompression "XtNcompression"
#define XtNcomputeGeometries "XtNcomputeGeometries"
#define XtNconstrainBtn "XtNconstrainBtn"
#define XtNconsumeEvent "XtNconsumeEvent"
#define XtNcontainerType "XtNcontainerType"
#define XtNcontrolArea "XtNcontrolArea"
#define XtNcontrolName "XtNcontrolName"
#define XtNcopyKey "XtNcopyKey"
#define XtNcornerColor "XtNcornerColor"
#define XtNcurrentPage "XtNcurrentPage"
#define XtNcursorPosition "XtNcursorPosition"
#define XtNcutKey "XtNcutKey"
#define XtNdampingFactor "XtNdampingFactor"
#define XtNdata "XtNdata"
#define XtNdefault "XtNdefault"
#define XtNdefaultActionKey "XtNdefaultActionKey"
#define XtNdefaultData "XtNdefaultData"
#define XtNdefaultDropSiteID "XtNdefaultDropSiteID"
#define XtNdefaultProc "XtNdefaultProc"
#define XtNdelCharBakKey "XtNdelCharBakKey"
#define XtNdelCharFwdKey "XtNdelCharFwdKey"
#define XtNdelLineKey "XtNdelLineKey"
#define XtNdelLineBakKey "XtNdelLineBakKey"
#define XtNdelLineFwdKey "XtNdelLineFwdKey"
#define XtNdelWordBakKey "XtNdelWordBakKey"
#define XtNdelWordFwdKey "XtNdelWordFwdKey"
#define XtNdestroy "XtNdestroy"
#define XtNdim "XtNdim"
#define XtNdirectManipulation "XtNdirectManipulation"
#define XtNdirection "XtNdirection"
#define XtNdirty "XtNdirty"
#define XtNdiskSrc "XtNdiskSrc"
#define XtNdismiss "XtNdismiss"
#define XtNdisplayPosition "XtNdisplayPosition"
#define XtNdocEndKey "XtNdocEndKey"
#define XtNdocStartKey "XtNdocStartKey"
#define XtNdoingDrag "XtNdoingDrag"
#define XtNdontCare "XtNdontCare"
#define XtNdownKey "XtNdownKey"
#define XtNdragCBType "XtNdragCBType"
#define XtNdragCursor "XtNdragCursor"
#define XtNdragKey "XtNdragKey"
#define XtNdragRightDistance "XtNdragRightDistance"
#define XtNdropKey "XtNdropKey"
#define XtNdropSiteID "XtNdropSiteID"
#define XtNdsdmPresent "XtNdsdmPresent"
#define XtNduplicateBtn "XtNduplicateBtn"
#define XtNduplicateKey "XtNduplicateKey"
#define XtNeditMode "XtNeditMode"
#define XmNeditType			"XmNeditType"
#define XtNemanateWidget		"XtNemanateWidget"
#define XtNexecute			"XtNexecute"
#define XtNexpose			"XtNexpose"
#define XtNexposeCallback		"XtNexposeCallback"
#define XmNfile				"XmNfile"
#define XtNcurrentFolder		"XtNcurrentFolder"
#define XtNlastDocumentName		"XtNlastDocumentName"
#define XtNshowGlyphs			"XtNshowGlyphs"
#define XtNfilterString			"XtNfilterString"
#define XmNfilterLabelString		"XmNfilterLabelString"
#define XtNfilterProc			"XtNfilterProc"
#define XtNshowInactive			"XtNshowInactive"
#define XtNhideDotFiles			"XtNhideDotFiles"
#define XtNopenFolderCallback		"XtNopenFolderCallback"
#define XtNinputDocumentCallback	"XtNinputDocumentCallback"
#define XtNoutputDocumentCallback	"XtNoutputDocumentCallback"
#define XtNhomeFolder 			"XtNhomeFolder"
#define XtNapplicationFoldersMaxCount	"XtNapplicationFoldersMaxCount"
#define XtNapplicationFolders	 	"XtNapplicationFolders"
#define XtNuserFoldersMaxCount	 	"XtNuserFoldersMaxCount"
#define XtNuserFolders			"XtNuserFolders"
#define XtNhistoryFoldersMinCount	"XtNhistoryFoldersMinCount"
#define XtNhistoryFoldersMaxCount	"XtNhistoryFoldersMaxCount"
#define XtNhistoryFolders		"XtNhistoryFolders"
#define XtNexpandTilde			"XtNexpandTilde"
#define XtNsubstituteShellVariables	"XtNsubstituteShellVariables"
#define XmNsubMenuId			"XmNsubMenuId"
#define XtNcomparisonFunc 		"XtNcomparisonFunc"
#define XtNfocusModel "XtNfocusModel"
#define XtNfocusWidget "XtNfocusWidget"
#define XtNfont "XtNfont"
#define XtNfontColor "XtNfontColor"
#define XtNfontName "XtNfontName"

#define XtNgetValuesHook "XtNgetValuesHook"
#define XtNgrabPointer "XtNgrabPointer"
#define XtNgrabServer "XtNgrabServer"
#define XtNgranularity "XtNgranularity"
#define XtNgraphicsExposeCallback "XtNgraphicsExposeCallback"
#define XtNgravity "XtNgravity"
#define XtNgrow "XtNgrow"
#define XtNhAutoScroll "XtNhAutoScroll"
#define XtNhInitialDelay "XtNhInitialDelay"
#define XtNhMenuPane "XtNhMenuPane"
#define XtNhPad "XtNhPad"
#define XtNhRepeatRate "XtNhRepeatRate"
#define XtNhScrollbar "XtNhScrollbar"
#define XtNhSliderMoved "XtNhSliderMoved"
#define XtNhStepSize "XtNhStepSize"
#define XtNhelpInfo "XtNhelpInfo"
#define XtNhelpModel "XtNhelpModel"
#define XtNhelpKey "XtNhelpKey"
#define XtNhighlightHandlerProc "XtNhighlightHandlerProc"
#define XtNhorizSBMenuKey "XtNhorizSBMenuKey"
#define XtNhorizontalSB "XtNhorizontalSB"
#define XtNiconBorder "XtNiconBorder"
#define XtNiconGravity "XtNiconGravity"
#define XtNiconParking "XtNiconParking"
#define XtNimPreeditStyle "XtNimPreeditStyle"
#define XtNinitialDelay "XtNinitialDelay"
#define XtNinitialX  "XtNinitialX"
#define XtNinitialY  "XtNinitialY"
#define XtNinitialize "XtNinitialize"
#define XtNinitializeHook "XtNinitializeHook"
#define XtNinputFocusColor "XtNinputFocusColor"
#define XtNinsertReturn "XtNinsertReturn"
#define XtNinsertTab "XtNinsertTab"
#define XtNinterpolatePointSize "XtNinterpolatePointSize"
#define XtNitemCount "XtNitemCount"
#define XtNitemFields "XtNitemFields"
#define XtNitemGravity "XtNitemGravity"
#define XtNitemHeight "XtNitemHeight"
#define XtNitemMaxHeight "XtNitemMaxHeight"
#define XtNitemMaxWidth "XtNitemMaxWidth"
#define XtNitemMinHeight "XtNitemMinHeight"
#define XtNitemMinWidth "XtNitemMinWidth"
#define XtNitemState "XtNitemState"
#define XtNitemWidth "XtNitemWidth"
#define XtNitems "XtNitems"
#define XtNitemsTouched "XtNitemsTouched"
#define XtNitemsVisible "XtNitemsVisible"
#define XtNjustify "XtNjustify"
#define XtNkeys "XtNkeys"
#define XtNkeyRemapTimeOut		"XtNkeyRemapTimeOut"
#define XtNlabelJustify			"XtNlabelJustify"
#define XtNlabelPixmap			"XtNlabelPixmap"
#define XtNlabelProc			"XtNlabelProc"
#define XmNlabelString			"XmNlabelString"
#define XtNlabelTile			"XtNlabelTile"
#define XmNlabelType			"XmNlabelType"
#define XtNlayout 			"XtNlayout"
#define XtNlayoutHeight 		"XtNlayoutHeight"
#define XtNlayoutType 			"XtNlayoutType"
#define XtNlayoutWidth 			"XtNlayoutWidth"
#define XtNleaveVerification 		"XtNleaveVerification"
#define XtNleftKey 			"XtNleftKey"
#define XtNleftMargin 			"XtNleftMargin"
#define XtNlineEndKey 			"XtNlineEndKey"
#define XtNlineStartKey			"XtNlineStartKey"
#define XtNlineSpace			"XtNlineSpace"
#define XtNlinesVisible			"XtNlinesVisible"
#define XtNlockName			"XtNlockName"
#define XtNlowerControlArea		"XtNlowerControlArea"
#define XtNmanaged			"XtNmanaged"
#define XtNmargin			"XtNmargin"
#define XmNmarginHeight			"XmNmarginHeight"
#define XmNmarginRight			"XmNmarginRight"
#define XmNmarginTop			"XmNmarginTop"
#define XmNmarginWidth			"XmNmarginWidth"
#define XtNmaxLabel			"XtNmaxLabel"
#define XtNmaximumChars			"XtNmaximumChars"
#define XtNmenuAugment "XtNmenuAugment"
#define XtNmenuBtn "XtNmenuBtn"
#define XtNmenuButton "XtNmenuButton"
#define XtNmenuDefaultBtn "XtNmenuDefaultBtn"
#define XtNmenuDefaultKey "XtNmenuDefaultKey"
#define XtNmenuKey "XtNmenuKey"
#define XtNmenuMark "XtNmenuMark"
#define XtNmenuMarkRegion "XtNmenuMarkRegion"
#define XtNmenuName "XtNmenuName"
#define XtNmenuPane "XtNmenuPane"
#define XtNmenuPositioner "XtNmenuPositioner"
#define XtNmenuType "XtNmenuType"
#define XtNminLabel "XtNminLabel"
#define XtNmnemonic "XtNmnemonic"
#define XtNmnemonicPrefix "XtNmnemonicPrefix"
#define XtNmaxValue "XtNmaxValue"
#define XtNminValue "XtNminValue"
#define XtNmod1Name "XtNmod1Name"
#define XtNmod2Name "XtNmod2Name"
#define XtNmod3Name "XtNmod3Name"
#define XtNmod4Name "XtNmod4Name"
#define XtNmod5Name "XtNmod5Name"
#define XtNmodifyVerification "XtNmodifyVerification"
#define XtNmotionVerification "XtNmotionVerification"
#define XtNmouseDampingFactor "XtNmouseDampingFactor"
#define XtNmouseStatus "XtNmouseStatus"
#define XtNmouseX "XtNmouseX"
#define XtNmouseY "XtNmouseY"
#define XtNmultiClickTimeout "XtNmultiClickTimeout"
#define XtNmultiDownKey "XtNmultiDownKey"
#define XtNmultiLeftKey "XtNmultiLeftKey"
#define XtNmultiObjectCount		"XtNmultiObjectCount"
#define XtNmultiRightKey		"XtNmultiRightKey"
#define XtNmultiUpKey			"XtNmultiUpKey"
#define XmNmwmMenu			"XmNmwmMenu"
#define XtNnextAppKey "XtNnextAppKey"
#define XtNnextFieldKey "XtNnextFieldKey"
#define XtNnextWinKey "XtNnextWinKey"
#define XtNnoneSet "XtNnoneSet"
#define XtNnumItemFields "XtNnumItemFields"
#define XtNnumItems "XtNnumItems"
#define XtNnumberOfDropSites "XtNnumberOfDropSites"
#define XtNorientation 		"XtNorientation"
#define XmNpacking			"XmNpacking"
#define XtNpackedWidget			"XtNpackedWidget"
#define XtNpackedWidgetList		"XtNpackedWidgetList"
#define XtNpageDownKey "XtNpageDownKey"
#define XtNpageLeftKey "XtNpageLeftKey"
#define XtNpageRightKey "XtNpageRightKey"
#define XtNpageUpKey "XtNpageUpKey"
#define XtNpanBtn "XtNpanBtn"
#define XtNpaneBackground "XtNpaneBackground"
#define XtNpaneEndKey "XtNpaneEndKey"
#define XtNpaneForeground "XtNpaneForeground"
#define XtNpaneName "XtNpaneName"
#define XtNpaneStartKey "XtNpaneStartKey"
#define XtNparentReset "XtNparentReset"
#define XtNpasteKey "XtNpasteKey"
#define XtNpendingActionKey "XtNpendingActionKey"
#define XtNpendingDSDMInfo "XtNpendingDSDMInfo"
#define XtNpointerWarping "XtNpointerWarping"
#define XtNposition "XtNposition"
#define XtNpostModifyNotification "XtNpostModifyNotification"
#define XtNpostSelect			"XtNpostSelect"
#define XtNprefMaxWidth			"XtNprefMaxWidth"
#define XtNprefMinWidth			"XtNprefMinWidth"
#define XtNprevAppKey			"XtNprevAppKey"
#define XtNprevFieldKey			"XtNprevFieldKey"
#define XtNprevWinKey			"XtNprevWinKey"
#define XtNpreview			"XtNpreview"
#define XtNpreviewItem			"XtNpreviewItem"
#define XtNpreviewWidget		"XtNpreviewWidget"
#define XtNpropertiesKey		"XtNpropertiesKey"
#define XtNpropertyChange		"XtNpropertyChange"
#define XtNproportionLength		"XtNproportionLength"
#define XtNqueryGeometry		"XtNqueryGeometry"
#define XtNrealize			"XtNrealize"
#define XmNrecomputeSize		"XmNrecomputeSize"
#define XtNredrawKey			"XtNredrawKey"
#define XtNreferenceName		"XtNreferenceName"
#define XtNreferenceStub		"XtNreferenceStub"
#define XtNreferenceWidget		"XtNreferenceWidget"
#define XtNrefresh			"XtNrefresh"
#define XtNregisterFocusFunc		"XtNregisterFocusFunc"
#define XtNregistryUpdateTimestamp	 "XtNregistryUpdateTimestamp"
#define XtNrepeatRate			"XtNrepeatRate"
#define XtNreset			"XtNreset"
#define XtNresetDefault			"XtNresetDefault"
#define XtNresetFactory			"XtNresetFactory"
#define XtNresetSet			"XtNresetSet"
#define XtNresize			"XtNresize"
#define XtNresizeCallback		"XtNresizeCallback"
#define XtNresizeCorners		"XtNresizeCorners"
#define XtNreturnKey			"XtNreturnKey"
#define XtNrevertButton			"XtNrevertButton"
#define XtNrightKey			"XtNrightKey"
#define XtNrightMargin			"XtNrightMargin"
#define XtNrootX			"XtNrootX"
#define XtNrootY			"XtNrootY"
#define XtNrowDownKey			"XtNrowDownKey"
#define XtNrowUpKey			"XtNrowUpKey"
#define XtNsameHeight			"XtNsameHeight"
#define XtNsameSize			"XtNsameSize"
#define XtNsameWidth			"XtNsameWidth"
#define XtNscale			"XtNscale"
#define XtNscaleMap			"XtNscaleMap"
#define XmNscrollBarPlacement		"XmNscrollBarPlacement"
#define XmNhorizontalScrollBar		"XmNhorizontalScrollBar"
#define XmNverticalScrollBar		"XmNverticalScrollBar"
#define XtNscroll			"XtNscroll"
#define XtNscrollBottomKey		"XtNscrollBottomKey"
#define XtNscrollDocStart		"XtNscrollDocStart"
#define XtNscrollDocEnd			"XtNscrollDocEnd"
#define XtNscrollDownKey		"XtNscrollDownKey"
#define XtNscrollLeftKey		"XtNscrollLeftKey"
#define XtNscrollLeftEdgeKey		"XtNscrollLeftEdgeKey"
#define XtNscrollRightKey		"XtNscrollRightKey"
#define XtNscrollRightEdgeKey		"XtNscrollRightEdgeKey"
#define XtNscrollTopKey			"XtNscrollTopKey"
#define XtNscrollUpKey			"XtNscrollUpKey"
#define XtNselCharBakKey		"XtNselCharBakKey"
#define XtNselCharFwdKey		"XtNselCharFwdKey"
#define XtNselFlipEndsKey		"XtNselFlipEndsKey"
#define XtNselLineKey			"XtNselLineKey"
#define XtNselLineBakKey		"XtNselLineBakKey"
#define XtNselLineFwdKey		"XtNselLineFwdKey"
#define XtNselWordBakKey		"XtNselWordBakKey"
#define XtNselWordFwdKey		"XtNselWordFwdKey"
#define XtNselectBtn			"XtNselectBtn"
#define XtNselectKey			"XtNselectKey"
#define XtNselectDoesPreview		"XtNselectDoesPreview"
#define XtNselectEnd			"XtNselectEnd"
#define XtNselectProc			"XtNselectProc"
#define XtNselectStart			"XtNselectStart"
#define XtNselectable			"XtNselectable"
#define XtNselection			"XtNselection"
#define XmNselectionArray		"XmNselectionArray"
#define XtNselectionColor		"XtNselectionColor"
#define XtNset				"XtNset"
#define XtNsetDefaults			"XtNsetDefaults"
#define XtNsetValues			"XtNsetValues"
#define XtNsetValuesAlmost		"XtNsetValuesAlmost"
#define XtNsetValuesHook		"XtNsetValuesHook"
#define XtNshellBehavior		"XtNshellBehavior"
#define XtNshiftName "XtNshiftName"
#define XtNshowAccelerators "XtNshowAccelerators"
#define XtNshowMnemonics "XtNshowMnemonics"
#define XtNshowPage "XtNshowPage"
#define XtNsliderMax "XtNsliderMax"
#define XtNsliderMin "XtNsliderMin"
#define XtNsliderMoved "XtNsliderMoved"
#define XtNsliderValue "XtNsliderValue"
#define XtNsource "XtNsource"
#define XtNsourceType "XtNsourceType"
#define XtNspan "XtNspan"
#define XtNstateChange "XtNstateChange"
#define XtNstopKey "XtNstopKey"
#define XtNstopPosition "XtNstopPosition"
#define XmNstring		"XmNstring"
#define XtNstrip "XtNstrip"
#define XtNtabTable "XtNtabTable"
#define XtNtextArea "XtNtextArea"
#define XtNtextClearBuffer "XtNtextClearBuffer"
#define XtNtextCopyBuffer "XtNtextCopyBuffer"
#define XtNtextEditBackground "XtNtextEditBackground"
#define XtNtextEditFontColor "XtNtextEditFontColor"
#define XtNtextEditWidget "XtNtextEditWidget"
#define XtNtextField "XtNtextField"
#define XtNtextFormat "XtNtextFormat"
#define XtNtextGetInsertPoint "XtNtextGetInsertPoint"
#define XtNtextGetLastPos "XtNtextGetLastPos"
#define XtNtextInsert "XtNtextInsert"
#define XtNtextReadSubStr "XtNtextReadSubStr"
#define XtNtextRedraw "XtNtextRedraw"
#define XtNtextReplace "XtNtextReplace"
#define XtNtextSetInsertPoint "XtNtextSetInsertPoint"
#define XtNtextSetSource "XtNtextSetSource"
#define XtNtextSource "XtNtextSource"
#define XtNtextUpdate "XtNtextUpdate"
#define XtNthreeD "XtNthreeD"
#define XtNtickUnit "XtNtickUnit"
#define XtNticks "XtNticks"
#define XtNtogglePushpinKey "XtNtogglePushpinKey"
#define XtNtoggleState "XtNtoggleState"
#define XtNtopMargin "XtNtopMargin"
#define XtNtouchItems "XtNtouchItems"
#define XtNtraversalHandlerFunc "XtNtraversalHandlerFunc"
#define XtNtraversalManager "XtNtraversalManager"
#define XtNtraversalOn "XtNtraversalOn"
#define XtNtrigger "XtNtrigger"
#define XtNtype "XtNtype"
#define XtNunderline "XtNunderline"
#define XtNundoKey "XtNundoKey"
#define XtNunitType "XtNunitType"
#define XtNunselect "XtNunselect"
#define XtNunselectProc "XtNunselectProc"
#define XtNupKey "XtNupKey"
#define XtNupdateView "XtNupdateView"
#define XtNupperControlArea "XtNupperControlArea"
#define XtNuseSetValCallback "XtNuseSetValCallback"
#define XtNuseShortOLWinAttr "XtNuseShortOLWinAttr"
#define XtNuserAddItems "XtNuserAddItems"
#define XtNuserData "XtNuserData"
#define XtNuserDeleteItems "XtNuserDeleteItems"
#define XtNuserMakeCurrent "XtNuserMakeCurrent"
#define XtNvAutoScroll "XtNvAutoScroll"
#define XtNvInitialDelay "XtNvInitialDelay"
#define XtNvMenuPane "XtNvMenuPane"
#define XtNvPad "XtNvPad"
#define XtNvRepeatRate "XtNvRepeatRate"
#define XtNvScrollbar "XtNvScrollbar"
#define XtNvSliderMoved "XtNvSliderMoved"
#define XtNvStepSize "XtNvStepSize"
#define XtNverification "XtNverification"
#define XtNverify "XtNverify"
#define XtNvertSBMenuKey "XtNvertSBMenuKey"
#define XtNverticalSB "XtNverticalSB"
#define XtNverticalSBWidget "XtNverticalSBWidget"
#define XtNviewHeight "XtNviewHeight"
#define XtNviewWidth "XtNviewWidth"
#define XtNvisibleChildren "XtNvisibleChildren"
#define XtNvsb "XtNvsb"
#define XtNwinType "XtNwinType"
#define XtNwindowBackground "XtNwindowBackground"
#define XtNwindowForeground "XtNwindowForeground"
#define XtNwindowHeader "XtNwindowHeader"
#define XtNwindowLayering "XtNwindowLayering"
#define XtNwindowMenuKey "XtNwindowMenuKey"
#define XtNwmProtocol "XtNwmProtocol"
#define XtNwmProtocolInterested "XtNwmProtocolInterested"
#define XtNwordBakKey "XtNwordBakKey"
#define XtNwordFwdKey "XtNwordFwdKey"
#define XtNworkspaceMenuKey "XtNworkspaceMenuKey"
#define XtNwrap "XtNwrap"
#define XtNwrapBreak "XtNwrapBreak"
#define XtNwrapForm "XtNwrapForm"
#define XtNwrapMode "XtNwrapMode"
#define XtNxAddWidth "XtNxAddWidth"
#define XtNxAttachOffset "XtNxAttachOffset"
#define XtNxAttachRight "XtNxAttachRight"
#define XtNxOffset "XtNxOffset"
#define XtNxRefName "XtNxRefName"
#define XtNxRefWidget "XtNxRefWidget"
#define XtNxResizable "XtNxResizable"
#define XtNxVaryOffset "XtNxVaryOffset"
#define XtNyAddHeight 		"XtNyAddHeight"
#define XtNyAttachBottom 	"XtNyAttachBottom"
#define XtNyAttachOffset 	"XtNyAttachOffset"
#define XtNyOffset 		"XtNyOffset"
#define XtNyRefName 		"XtNyRefName"
#define XtNyRefWidget 		"XtNyRefWidget"
#define XtNyResizable 		"XtNyResizable"
#define XtNyVaryOffset 		"XtNyVaryOffset"
#define XtRAcceleratorTable "XtRAcceleratorTable"
#define XtRAtom "XtRAtom"
#define XtRBitmap "XtRBitmap"
#define XtRBool "XtRBool"
#define XtRBoolean "XtRBoolean"
#define XtRCallback "XtRCallback"
#define XtRCallProc "XtRCallProc"
#define XtRCardinal "XtRCardinal"
#define XtRColor "XtRColor"
#define XtRColormap "XtRColormap"
#define XtRCursor "XtRCursor"
#define XtRDimension "XtRDimension"
#define XtRDisplay "XtRDisplay"
#define XtREditMode "XtREditMode"
#define XtREnum "XtREnum"
#define XtRFile "XtRFile"
#define XtRFloat "XtRFloat"
#define XtRFont "XtRFont"
#define XtRFontStruct "XtRFontStruct"
#define XtRFunction "XtRFunction"
#define XtRGeometry "XtRGeometry"
#define XtRImmediate "XtRImmediate"
#define XtRInitialState "XtRInitialState"
#define XtRInt "XtRInt"
#define XtRJustify "XtRJustify"
#define XtRLongBoolean XtRBool
#define XtRObject "XtRObject"
#define XtROlStr "XtROlStr"
#define XtROrientation "XtROrientation"
#define XtRPixel "XtRPixel"
#define XtRPixmap "XtRPixmap"
#define XtRPointer "XtRPointer"
#define XtRPosition "XtRPosition"
#define XtRScreen "XtRScreen"
#define XtRShort "XtRShort"
#define XtRString "XtRString"
#define XtRStringArray "XtRStringArray"
#define XtRStringTable "XtRStringTable"
#define XtRUnsignedChar "XtRUnsignedChar"
#define XtRTranslationTable "XtRTranslationTable"
#define XtRVisual "XtRVisual"
#define XtRWidget "XtRWidget"
#define XtRWidgetClass "XtRWidgetClass"
#define XtRWidgetList "XtRWidgetList"
#define XtRWindow "XtRWindow"

/*
 * Constants
 */
#define XmTEAR_OFF_ENABLED		"XmTEAR_OFF_ENABLED"
#define XmTEAR_OFF_DISABLED		"XmTEAR_OFF_DISABLED"
#define XmWORK_AREA			"XmWORK_AREA"
#define XmMENU_BAR			"XmMENU_BAR"
#define XmMENU_PULLDOWN			"XmMENU_PULLDOWN"
#define XmMENU_POPUP			"XmMENU_POPUP"
#define XmMENU_OPTION			"XmMENU_OPTION"
#define XmCR_ACTIVATE			"XmCR_ACTIVATE"
#define XmCR_ARM			"XmCR_ARM"
#define XmCR_DISARM			"XmCR_DISARM"
#define XmRES_PIXMAP			"XmPIXMAP"
#define XmSHADOW_IN			"XmSHADOW_IN"
#define XmSHADOW_OUT			"XmSHADOW_OUT"
#define XmSHADOW_ETCHED_IN		"XmSHADOW_ETCHED_IN"
#define XmSHADOW_ETCHED_OUT		"XmSHADOW_ETCHED_OUT"
#define XmRES_STRING			"XmSTRING"
#define XmRESIZE_ANY			"XmRESIZE_ANY"
#define XmVERTICAL			"XmVERTICAL"
#define XmHORIZONTAL			"XmHORIZONTAL"
#define XmSINGLE_SELECT			"XmSINGLE_SELECT"
#define XmMULTIPLE_SELECT		"XmMULTIPLE_SELECT"
#define XmEXTENDED_SELECT		"XmEXTENDED_SELECT"
#define XmBROWSE_SELECT			"XmBROWSE_SELECT"
#define XmMULTI_LINE_EDIT		"XmMULTI_LINE_EDIT"
#define XmTOP_LEFT			"XmTOP_LEFT"
#define XmBOTTOM_LEFT			"XmBOTTOM_LEFT"
#define XmTOP_RIGHT			"XmTOP_RIGHT"
#define XmBOTTOM_RIGHT			"XmBOTTOM_RIGHT"
#define XmVARIABLE			"XmVARIABLE"
#define XmCONSTANT			"XmCONSTANT"
#define	XmSTRING			"XmSTRING"
#define XmPIXMAP			"XmPIXMAP"
#define XmALIGNMENT_BASELINE_BOTTOM	"XmALIGNMENT_BASELINE_BOTTOM"
#define XmALIGNMENT_BASELINE_TOP	"XmALIGNMENT_BASELINE_TOP"
#define XmALIGNMENT_CONTENTS_BOTTOM	"XmALIGNMENT_CONTENTS_BOTTOM"
#define XmALIGNMENT_CONTENTS_TOP	"XmALIGNMENT_CONTENTS_TOP"
#define XmALIGNMENT_CENTER		"XmALIGNMENT_CENTER"
#define XmPACK_COLUMN			"XmPACK_COLUMN"
#define XmPACK_NONE			"XmPACK_NONE"
#define XmPACK_TIGHT			"XmPACK_TIGHT"

/*
 ************** GMF group widget ***********
 */
#define XmNgroupType		"XmNgroupType"
#define XmNgroupUseConstraints	"XmNgroupUseConstraints"
#define XmNgroupColumnAlignment	"XmNgroupColumnAlignment"
#define XmNgroupRowAlignment	"XmNgroupRowAlignment"
#define XmNgroupMinX		"XmNgroupMinX"
#define XmNgroupMinY		"XmNgroupMinY"
#define XmNgroupDenominator	"XmNgroupDenominator"
#define XmNgroupVOffset		"XmNgroupVOffset"
#define XmNgroupHOffset		"XmNgroupHOffset"
#define XmNgroupColumns		"XmNgroupColumns"
#define XmNgroupRows		"XmNgroupRows"
#define XmNgroupColumn		"XmNgroupColumn"
#define XmNgroupRow		"XmNgroupRow"
#define XmNgroupAnchor		"XmNgroupAnchor"
#define XmNgroupAnchorName	"XmNgroupAnchorName"
#define XmNgroupAnchorPoint	"XmNgroupAnchorPoint"
#define XmNgroupReferencePoint	"XmNgroupReferencePoint"
#define XmNgroupColumnAlignment "XmNgroupColumnAlignment"
#define XmNgroupRowAlignment 	"XmNgroupRowAlignment"

#define XmGROUP_NONE			"XmGROUP_NONE"
#define XmGROUP_ROWS			"XmGROUP_ROWS"
#define XmGROUP_COLUMNS			"XmGROUP_COLUMNS"
#define	XmGROUP_ROWCOLUMN		"XmGROUP_ROWCOLUMN"

/* column alignments */

#define	XmGROUP_COLUMN_ALIGN_V_CENTER	"XmGROUP_COLUMN_ALIGN_V_CENTER"
#define	XmGROUP_COLUMN_ALIGN_LEFT	"XmGROUP_COLUMN_ALIGN_LEFT"
#define	XmGROUP_COLUMN_ALIGN_RIGHT	"XmGROUP_COLUMN_ALIGN_RIGHT"
#define	XmGROUP_COLUMN_ALIGN_LABELS	"XmGROUP_COLUMN_ALIGN_LABELS"

/* row alignments */

#define	XmGROUP_ROW_ALIGN_TOP_EDGES	"XmGROUP_ROW_ALIGN_TOP_EDGES"
#define	XmGROUP_ROW_ALIGN_BOTTOM_EDGES	"XmGROUP_ROW_ALIGN_BOTTOM_EDGES"
#define	XmGROUP_ROW_ALIGN_H_CENTER	"XmGROUP_ROW_ALIGN_H_CENTER"

/* compass points */

#define XmGROUP_CENTER			"XmGROUP_CENTER"
#define	XmGROUP_NORTH			"XmGROUP_NORTH"
#define	XmGROUP_NORTHEAST		"XmGROUP_NORTHEAST"
#define	XmGROUP_EAST			"XmGROUP_EAST"
#define	XmGROUP_SOUTHEAST		"XmGROUP_SOUTHEAST"
#define	XmGROUP_SOUTH			"XmGROUP_SOUTH"
#define	XmGROUP_SOUTHWEST		"XmGROUP_SOUTHWEST"
#define	XmGROUP_WEST			"XmGROUP_WEST"
#define	XmGROUP_NORTHWEST		"XmGROUP_NORTHWEST"

/*
 * XtN stuff
 */

#define XtNallowHoriz			"XtNallowHoriz"
#define XtNNallowVert			"XtNallowVert"
#define XtNancestorSensitive		"XtNancestorSensitive"
#define XtNbitmap			"XtNbitmap"
#define XtNborderColor			"XtNborderColor"
#define XtNborder			"XtNborderColor"
#define XtNborderPixmap			"XtNborderPixmap"
#define XtNcallback			"XtNcallback"
#define XtNchildren			"XtNchildren"
#define XtNcolormap			"XtNcolormap"
#define XtNdepth			"XtNdepth"
#define XtNfont				"XtNfont"
#define XtNforceBars			"XtNforceBars"
#define XtNfunction			"XtNfunction"
#define XtNhighlight			"XtNhighlight"
#define XtNindex			"XtNindex"
#define XtNinitialResourcesPersistent	"XtNinitialResourcesPersistent"
#define XtNinnerHeight			"XtNinnerHeight"
#define XtNinnerWidth			"XtNinnerWidth"
#define XtNinnerWindow			"XtNinnerWindow"
#define XtNinsertPosition		"XtNinsertPosition"
#define XtNinternalHeight		"XtNinternalHeight"
#define XtNinternalWidth		"XtNinternalWidth"
#define XtNjumpProc			"XtNjumpProc"
#define XtNjustify			"XtNjustify"
#define XtNknobHeight 			"XtNknobHeight"
#define XtNknobIndent 			"XtNknobIndent"
#define XtNknobPixel 			"XtNknobPixel"
#define XtNknobWidth 			"XtNknobWidth"
#define XtNlength			"XtNlength"
#define XtNlowerRight			"XtNlowerRight"
#define XtNmappedWhenManaged		"XtNmappedWhenManaged"
#define XtNmenuEntry			"XtNmenuEntry"
#define XtNname				"XtNname"
#define XtNnotify			"XtNnotify"
#define XtNnumChildren			"XtNnumChildren"
#define XtNparameter			"XtNparameter"
#define XtNpixmap			"XtNpixmap"
#define XtNresize			"XtNresize"
#define XtNreverseVideo			"XtNreverseVideo"
#define XtNscreen			"XtNscreen"
#define XtNscrollingListMode		"XtNscrollingListMode"
#define XtNscrollProc			"XtNscrollProc"
#define XtNscrollDCursor		"XtNscrollDCursor"
#define XtNscrollHCursor		"XtNscrollHCursor"
#define XtNscrollLCursor		"XtNscrollLCursor"
#define XtNscrollRCursor		"XtNscrollRCursor"
#define XtNscrollUCursor		"XtNscrollUCursor"
#define XtNscrollVCursor		"XtNscrollVCursor"
#define XtNselection			"XtNselection"
#define XtNsensitive			"XtNsensitive"
#define XtNshown			"XtNshown"
#define XtNspace			"XtNspace"
#define XtNtextOptions 			"XtNtextOptions"
#define XtNtextSink			"XtNtextSink"
#define XtNtextSource			"XtNtextSource"
#define XtNthickness			"XtNthickness"
#define XtNthumb			"XtNthumb"
#define XtNthumbProc			"XtNthumbProc"
#define XtNtop				"XtNtop"
#define XtNtranslations			"XtNtranslations"
#define XtNunrealizeCallback		"XtNunrealizeCallback"
#define XtNupdate			"XtNupdate"
#define XtNuseBottom			"XtNuseBottom"
#define XtNuseRight			"XtNuseRight"
#define XtNvalue			"XtNvalue"
#define XmNwindow			"XmNwindow"

#endif /* _ABOBJ_MOTIFDEFS_H_ */
