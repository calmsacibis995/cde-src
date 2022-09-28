/* $XConsortium: text.c /main/cde1_maint/1 1995/07/17 16:43:40 drk $ */
/*****************************************************************************
 *****************************************************************************
 **
 **   File:         text.c
 **
 **   Description:  Text transfer functions for the CDE Drag & Drop Demo.
 **
 **  (c) Copyright 1993, 1994 Hewlett-Packard Company
 **  (c) Copyright 1993, 1994 International Business Machines Corp.
 **  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 **  (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of
 **      Novell, Inc.
 **
 ****************************************************************************
 ************************************<+>*************************************/

#include <X11/Intrinsic.h>

#include <Xm/Xm.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>

#include <Dt/Dt.h>
#include <Dt/Dnd.h>

#include "demo.h"
#include "text.h"

 /*************************************************************************
 *
 *       Data Structures & Private Declarations For Text Transfers
 *
 **************************************************************************/

/*
 * Data for text list of fruit
 */

char *todaysFruit[] = {
        "Oranges",
        "Peaches",
        "Lemons",
        "Watermelons",
        "Apples",
        "Bananas",
        "Plums",
        "Limes",
        "Cantaloupes",
        "Nectarines",
        "Papayas",
        "Mangos",
        NULL
};

 /*************************************************************************
 *
 *      Text Drag & Drop
 *
 *************************************************************************/

/*
 * textConvertCallback
 *
 * Sets the text object's text to the text in the fruit list based on where
 * the pointer was when the drag started.
 */
void
textConvertCallback(
        Widget          dragContext,
        XtPointer       clientData,
	XtPointer       callData)
{
        DtDndConvertCallbackStruct *convertInfo =
                                        (DtDndConvertCallbackStruct *) callData;
        Widget          fruitList = (Widget) clientData;
        int             selectedPos;
        XmString       *items;
        Cardinal        itemCount;

	if (convertInfo == NULL) {
		return;
	}

	/*
	 * Verify protocol and callback reason
	 */

        if (convertInfo->dragData->protocol != DtDND_TEXT_TRANSFER ||
	    (convertInfo->reason != DtCR_DND_CONVERT_DATA &&
	     convertInfo->reason != DtCR_DND_CONVERT_DELETE) ||
	    fruitList == NULL) {
                return;
	}

        switch (convertInfo->reason) {
        case DtCR_DND_CONVERT_DATA:

		/*
	 	 * Provide the text from the fruit list
		 */

                XtVaGetValues(fruitList,
                        XmNuserData,    &selectedPos,
                        XmNitems,       &items,
                        XmNitemCount,   &itemCount,
                        NULL);

                if (itemCount > 0 && selectedPos < itemCount) {
                        convertInfo->dragData->data.strings[0] =
                                        items[selectedPos-1];
                } else {
                        convertInfo->status = DtDND_FAILURE;
                }
                break;
        DtCR_DND_CONVERT_DELETE:

		/*
		 * Delete the text from the fruit list. If the fruit list
		 * were set up to be dynamic, deletion from the list would
		 * occur here.
		 */

                printf("Delete fruit item #%d\n",
                        convertInfo->dragData->data.strings[0]);
                break;
        }
}

/*
 * textDragFinishCallback
 *
 * Normally would free any memory allocated by textConvertCallback
 * but none was allocated so this is just a placeholder.
 */
void
textDragFinishCallback(
        Widget          widget,
        XtPointer       clientData,
        XtPointer       callData)
{
}

/*
 * textTransferCallback
 *
 * Handles transfer of files or text to the text edit. Files are transfered
 * by placing their name in the field, text by inserting the text into the
 * field.
 */
void
textTransferCallback(
        Widget          widget,
        XtPointer       clientData,
        XtPointer       callData)
{
        DtDndTransferCallbackStruct *transferInfo =
				(DtDndTransferCallbackStruct *) callData;
        String  	text;

	/*
	 * Verify callback reason
	 */

	if (transferInfo == NULL || 
	    transferInfo->reason != DtCR_DND_TRANSFER_DATA) {
		return;
	}

        switch (transferInfo->dropData->protocol) {

        case DtDND_FILENAME_TRANSFER:

		/*
		 * Copy the file name into the text field
		 */

                XtVaSetValues(widget,
                        XmNvalue, transferInfo->dropData->data.files[0],
                        NULL);

                break;

        case DtDND_TEXT_TRANSFER:

		/*
		 * Copy the fruit name into the text field
		 */

                XmStringGetLtoR(transferInfo->dropData->data.strings[0],
                        XmFONTLIST_DEFAULT_TAG, &text);

                XtVaSetValues(widget, XmNvalue, text, NULL);

                break;
        }
}

/*
 * textDragSetup
 *
 * Prepares the fruit list to source drags of text with button 1.
 */
void
textDragSetup(
	Widget		fruitList)
{
        XtAddEventHandler(fruitList, Button1MotionMask, False,
                (XtEventHandler)demoDragMotionHandler,
                (XtPointer)DtDND_TEXT_TRANSFER);
}

/*
 * textDropSetup
 *
 * Registers text field to accept drops of files.
 */
void
textDropSetup(
        Widget          textField)
{
        static XtCallbackRec transferCBRec[] = { {textTransferCallback, NULL},
                                                 {NULL, NULL} };

        DtDndDropRegister(textField, DtDND_FILENAME_TRANSFER,
                XmDROP_COPY, transferCBRec, NULL, 0);
}

/*
 * textDragStart
 *
 * Initiates a drag of a text item from the fruit list provided the pointer
 * is over an item in the list.
 */
void
textDragStart(
        Widget          widget,
        XEvent         *event)
{
        int             itemCount,
			selectedPos;

        static XtCallbackRec convertCBRec[] = { {textConvertCallback, NULL},
                                                {NULL, NULL} };
        static XtCallbackRec dragFinishCBRec[] =
					      { {demoDragFinishCallback, NULL},
					        {textDragFinishCallback, NULL},
                                                {NULL, NULL} };

	/*
	 * Determine which item to drag from the text list
	 */

        XtVaGetValues(widget, XmNitemCount, &itemCount, NULL);

        selectedPos = XmListYToPos(widget, event->xmotion.y);

        if (selectedPos == 0 || selectedPos > itemCount) {
                return;
        }

        XtVaSetValues(widget, XmNuserData, selectedPos, NULL);

        convertCBRec[0].closure = (XtPointer)widget;

	/*
	 * Start the drag
	 */

        if (DtDndDragStart(widget, event, DtDND_TEXT_TRANSFER, 1,
            XmDROP_COPY, convertCBRec, dragFinishCBRec, NULL, 0)
            == NULL) {

                printf("DragStart returned NULL.\n");
        }
}

 /*************************************************************************
 *
 *      Text Creation & Initialization
 *
 *************************************************************************/

/*
 * textCreateDragSource
 *
 * Creates a scrolling list filled with fruit names.
 */
Widget
textCreateDragSource(
        Widget          parent)
{
        Widget          fruitList;
        XmString       *fruits;
        Arg             args[2];
        int             ii, fruitCount;

        for (ii = 0; todaysFruit[ii] != NULL; ii++)
		;
        fruitCount = ii;

        fruits = (XmString *) XtMalloc(sizeof(XmString) * fruitCount);

        for (ii = 0; ii < fruitCount; ii++) {
                fruits[ii] = XmStringCreate(todaysFruit[ii],
                                            XmFONTLIST_DEFAULT_TAG);
        }

        ii = 0;
        XtSetArg(args[ii], XmNitems,      fruits);     ii++;
        XtSetArg(args[ii], XmNitemCount,  fruitCount); ii++;

        fruitList = XmCreateScrolledList(parent, "fruitList", args, ii);
        XtManageChild(fruitList);

        for (ii = 0; ii < fruitCount; ii++) {
                XmStringFree(fruits[ii]);
        }
	XtFree((char *)fruits);

        return fruitList;
}

/*
 * textCreateDropSite
 *
 * Creates a text field with a label.
 */
Widget
textCreateDropSite(
	Widget		parent)
{
	Widget		textRowColumn,
			textLabel,
			textField;

        textRowColumn = XtVaCreateManagedWidget("textRowColumn",
                xmRowColumnWidgetClass, parent,
                NULL);

        textLabel = XtVaCreateManagedWidget("textLabel",
                xmLabelWidgetClass, textRowColumn,
                NULL);

        textField = XtVaCreateManagedWidget("textField",
                xmTextWidgetClass, textRowColumn,
                NULL);

	return textField;
}

