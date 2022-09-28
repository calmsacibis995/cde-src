#ifdef REV_INFO
#ifndef lint
static char SCCSID[] = "OSF/Motif: @(#)MessagesP.h	1.7 93/10/12";
#endif /* lint */
#endif /* REV_INFO */
/******************************************************************************
*******************************************************************************
*
*  (c) Copyright 1989, 1990, OPEN SOFTWARE FOUNDATION, INC.
*  (c) Copyright 1989, DIGITAL EQUIPMENT CORPORATION, MAYNARD, MASS.
*  (c) Copyright 1987, 1988, 1989, 1990, HEWLETT-PACKARD COMPANY
*  ALL RIGHTS RESERVED
*  
*  	THIS SOFTWARE IS FURNISHED UNDER A LICENSE AND MAY BE USED
*  AND COPIED ONLY IN ACCORDANCE WITH THE TERMS OF SUCH LICENSE AND
*  WITH THE INCLUSION OF THE ABOVE COPYRIGHT NOTICE.  THIS SOFTWARE OR
*  ANY OTHER COPIES THEREOF MAY NOT BE PROVIDED OR OTHERWISE MADE
*  AVAILABLE TO ANY OTHER PERSON.  NO TITLE TO AND OWNERSHIP OF THE
*  SOFTWARE IS HEREBY TRANSFERRED.
*  
*  	THE INFORMATION IN THIS SOFTWARE IS SUBJECT TO CHANGE WITHOUT
*  NOTICE AND SHOULD NOT BE CONSTRUED AS A COMMITMENT BY OPEN SOFTWARE
*  FOUNDATION, INC. OR ITS THIRD PARTY SUPPLIERS  
*  
*  	OPEN SOFTWARE FOUNDATION, INC. AND ITS THIRD PARTY SUPPLIERS,
*  ASSUME NO RESPONSIBILITY FOR THE USE OR INABILITY TO USE ANY OF ITS
*  SOFTWARE .   OSF SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY
*  KIND, AND OSF EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES, INCLUDING
*  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE.
*  
*  Notice:  Notwithstanding any other lease or license that may pertain to,
*  or accompany the delivery of, this computer software, the rights of the
*  Government regarding its use, reproduction and disclosure are as set
*  forth in Section 52.227-19 of the FARS Computer Software-Restricted
*  Rights clause.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.  Unpublished - all
*  rights reserved under the Copyright laws of the United States.
*  
*  RESTRICTED RIGHTS NOTICE:  Use, duplication, or disclosure by the
*  Government is subject to the restrictions as set forth in subparagraph
*  (c)(1)(ii) of the Rights in Technical Data and Computer Software clause
*  at DFARS 52.227-7013.
*  
*  Open Software Foundation, Inc.
*  11 Cambridge Center
*  Cambridge, MA   02142
*  (617)621-8700
*  
*  RESTRICTED RIGHTS LEGEND:  This computer software is submitted with
*  "restricted rights."  Use, duplication or disclosure is subject to the
*  restrictions as set forth in NASA FAR SUP 18-52.227-79 (April 1985)
*  "Commercial Computer Software- Restricted Rights (April 1985)."  Open
*  Software Foundation, Inc., 11 Cambridge Center, Cambridge, MA  02142.  If
*  the contract contains the Clause at 18-52.227-74 "Rights in Data General"
*  then the "Alternate III" clause applies.
*  
*  (c) Copyright 1989, 1990, Open Software Foundation, Inc.
*  ALL RIGHTS RESERVED 
*  
*  
* Open Software Foundation is a trademark of The Open Software Foundation, Inc.
* OSF is a trademark of Open Software Foundation, Inc.
* OSF/Motif is a trademark of Open Software Foundation, Inc.
* Motif is a trademark of Open Software Foundation, Inc.
* DEC is a registered trademark of Digital Equipment Corporation
* DIGITAL is a registered trademark of Digital Equipment Corporation
* X Window System is a trademark of the Massachusetts Institute of Technology
*
*******************************************************************************
******************************************************************************/

#ifndef _XmMessagesP_h
#define _XmMessagesP_h

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/*** const causes the s800 8.0 compiler to barf.  Remove reference until ***
 *** the build and integration people can figure out how to get this to  ***
 *** work.                                                               ***/
#define const

extern const char _XmMsgArrowB_0000[]; 
extern const char _XmMsgBaseClass_0000[];
extern const char _XmMsgBaseClass_0001[];
extern const char _XmMsgBaseClass_0002[];
extern const char _XmMsgBulletinB_0000[];
extern const char _XmMsgBulletinB_0001[];
extern const char _XmMsgBulletinB_0002[];
extern const char _XmMsgCascadeB_0000[];
extern const char _XmMsgCascadeB_0001[];
extern const char _XmMsgCascadeB_0002[];
extern const char _XmMsgCascadeB_0003[];
extern const char _XmMsgCommand_0000[];
extern const char _XmMsgCommand_0001[];
extern const char _XmMsgCommand_0002[];
extern const char _XmMsgCommand_0003[];
extern const char _XmMsgCommand_0004[];
extern const char _XmMsgCommand_0005[];
extern const char _XmMsgCutPaste_0000[];
extern const char _XmMsgCutPaste_0001[];
extern const char _XmMsgCutPaste_0002[];
extern const char _XmMsgCutPaste_0003[];
extern const char _XmMsgCutPaste_0004[];
extern const char _XmMsgCutPaste_0005[];
extern const char _XmMsgCutPaste_0006[];
extern const char _XmMsgCutPaste_0007[];
extern const char _XmMsgCutPaste_0008[];
extern const char _XmMsgCutPaste_0009[];
extern const char _XmMsgDialogS_0000[];
extern const char _XmMsgDrawnB_0000[];
extern const char _XmMsgForm_0000[];
extern const char _XmMsgForm_0001[];
extern const char _XmMsgForm_0002[];
extern const char _XmMsgForm_0003[];
extern const char _XmMsgForm_0004[];
extern const char _XmMsgFrame_0000[];
extern const char _XmMsgGadget_0000[];
extern const char _XmMsgGadget_0001[];
extern const char _XmMsgGetSecRes_0000[];
extern const char _XmMsgLabel_0000[];
extern const char _XmMsgLabel_0001[];
extern const char _XmMsgLabel_0002[];
extern const char _XmMsgLabel_0003[];
extern const char _XmMsgLabel_0004[];
extern const char _XmMsgList_0000[];
extern const char _XmMsgList_0001[];
extern const char _XmMsgList_0002[];
extern const char _XmMsgList_0003[];
extern const char _XmMsgList_0004[];
extern const char _XmMsgList_0005[];
extern const char _XmMsgList_0006[];
extern const char _XmMsgList_0007[];
extern const char _XmMsgList_0008[];
extern const char _XmMsgList_0009[];
extern const char _XmMsgList_0010[];
extern const char _XmMsgList_0011[];
extern const char _XmMsgList_0012[];
extern const char _XmMsgList_0013[];
extern const char _XmMsgMainW_0000[];
extern const char _XmMsgMainW_0001[];
extern const char _XmMsgManager_0000[];
extern const char _XmMsgManager_0001[];
extern const char _XmMsgMenuShell_0000[];
extern const char _XmMsgMenuShell_0001[];
extern const char _XmMsgMessageB_0000[];
extern const char _XmMsgMessageB_0001[];
extern const char _XmMsgMessageB_0002[];
extern const char _XmMsgMessageB_0003[];
extern const char _XmMsgMessageB_0004[];
extern const char _XmMsgNavigMap_0000[];
extern const char _XmMsgPanedW_0000[];
extern const char _XmMsgPanedW_0001[];
extern const char _XmMsgPanedW_0002[];
extern const char _XmMsgPanedW_0003[];
extern const char _XmMsgPanedW_0004[];
extern const char _XmMsgPanedW_0005[];
extern const char _XmMsgPrimitive_0000[];
extern const char _XmMsgProtocols_0000[];
extern const char _XmMsgProtocols_0001[];
extern const char _XmMsgProtocols_0002[];
extern const char _XmMsgPushB_0000[];
extern const char _XmMsgResConvert_0000[];
extern const char _XmMsgRowColumn_0000[];
extern const char _XmMsgRowColumn_0001[];
extern const char _XmMsgRowColumn_0002[];
extern const char _XmMsgRowColumn_0003[];
extern const char _XmMsgRowColumn_0004[];
extern const char _XmMsgRowColumn_0005[];
extern const char _XmMsgRowColumn_0006[];
extern const char _XmMsgRowColumn_0007[];
extern const char _XmMsgRowColumn_0008[];
extern const char _XmMsgRowColumn_0009[];
extern const char _XmMsgRowColumn_0010[];
extern const char _XmMsgRowColumn_0011[];
extern const char _XmMsgRowColumn_0012[];
extern const char _XmMsgRowColumn_0013[];
extern const char _XmMsgRowColumn_0014[];
extern const char _XmMsgRowColumn_0015[];
extern const char _XmMsgRowColumn_0016[];
extern const char _XmMsgRowColumn_0017[];
extern const char _XmMsgRowColumn_0018[];
extern const char _XmMsgRowColumn_0019[];
extern const char _XmMsgRowColumn_0020[];
extern const char _XmMsgRowColumn_0021[];
extern const char _XmMsgRowColumn_0022[];
extern const char _XmMsgRowColumn_0023[];
extern const char _XmMsgRowColumn_0024[];
extern const char _XmMsgScale_0000[];
extern const char _XmMsgScale_0001[];
extern const char _XmMsgScale_0002[];
extern const char _XmMsgScale_0003[];
extern const char _XmMsgScale_0004[];
extern const char _XmMsgScale_0005[];
extern const char _XmMsgScale_0006[];
extern const char _XmMsgScale_0007[];
extern const char _XmMsgScrollBar_0000[];
extern const char _XmMsgScrollBar_0001[];
extern const char _XmMsgScrollBar_0002[];
extern const char _XmMsgScrollBar_0003[];
extern const char _XmMsgScrollBar_0004[];
extern const char _XmMsgScrollBar_0005[];
extern const char _XmMsgScrollBar_0006[];
extern const char _XmMsgScrollBar_0007[];
extern const char _XmMsgScrollBar_0008[];
extern const char _XmMsgScrolledW_0000[];
extern const char _XmMsgScrolledW_0001[];
extern const char _XmMsgScrolledW_0002[];
extern const char _XmMsgScrolledW_0003[];
extern const char _XmMsgScrolledW_0004[];
extern const char _XmMsgScrolledW_0005[];
extern const char _XmMsgScrolledW_0006[];
extern const char _XmMsgScrolledW_0007[];
extern const char _XmMsgScrolledW_0008[];
extern const char _XmMsgScrolledW_0009[];
extern const char _XmMsgSelectioB_0000[];
extern const char _XmMsgSelectioB_0001[];
extern const char _XmMsgSelectioB_0002[];
extern const char _XmMsgSelectioB_0003[];
extern const char _XmMsgSeparatoG_0000[];
extern const char _XmMsgSeparatoG_0001[];
extern const char _XmMsgText_0000[];
extern const char _XmMsgText_0001[];
extern const char _XmMsgText_0002[];
extern const char _XmMsgTextF_0000[];
extern const char _XmMsgTextF_0001[];
extern const char _XmMsgTextF_0002[];
extern const char _XmMsgTextF_0003[];
extern const char _XmMsgTextF_0004[];
extern const char _XmMsgTextF_0005[];
extern const char _XmMsgTextF_0006[];
extern const char _XmMsgTextIn_0000[];
extern const char _XmMsgTextOut_0000[];
extern const char _XmMsgToggleB_0000[];
extern const char _XmMsgTraversal_0000[];
extern const char _XmMsgTraversal_0001[];
extern const char _XmMsgTraversal_0002[];
extern const char _XmMsgVendor_0000[];
extern const char _XmMsgVendorE_0000[];
extern const char _XmMsgVendorE_0001[];
extern const char _XmMsgVendorE_0002[];
extern const char _XmMsgVendorE_0003[];
extern const char _XmMsgVendorE_0004[];
extern const char _XmMsgVendorE_0005[];
extern const char _XmMsgVirtKeys_0000[];
extern const char _XmMsgVirtKeys_0001[];
extern const char _XmMsgVirtKeys_0002[];
extern const char _XmMsgVisual_0000[];
extern const char _XmMsgVisual_0001[];
extern const char _XmMsgVisual_0002[];

#if defined(__cplusplus) || defined(c_plusplus)
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmMessagesP_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */
