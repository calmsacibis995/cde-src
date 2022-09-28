/**********************************<+>*************************************
***************************************************************************
**
**  File:	 formatCB.c
**
**  Project:	 DT dtpad, a memo maker type editor based on the Dt Editor
**		 widget.
**
**  Description:
**  -----------
**
**	 This file contains the callbacks for the [Format] menu items.
**
*******************************************************************
**  (c) Copyright Hewlett-Packard Company, 1991, 1992.  All rights are
**  reserved.  Copying or other reproduction of this program
**  except for archival purposes is prohibited without prior
**  written consent of Hewlett-Packard Company.
********************************************************************
**
********************************************************************
**  (c) Copyright 1993, 1994 Hewlett-Packard Company
**  (c) Copyright 1993, 1994 International Business Machines Corp.
**  (c) Copyright 1993, 1994 Sun Microsystems, Inc.
**  (c) Copyright 1993, 1994 Novell, Inc.
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "dtpad.h"
/* #include <Dt/HourGlass.h> */

/************************************************************************
 * FormatCB - [Format] menu, [Settings...] button
 ************************************************************************/
/* ARGSUSED */
void
FormatCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Editor *pPad = (Editor *) client_data;
    DtEditorInvokeFormatDialog(pPad->editor);
}


/************************************************************************
 * FormatParaCB - [Format] menu, [Paragraph] button
 ************************************************************************/
/* ARGSUSED */
void
FormatParaCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Editor *pPad = (Editor *) client_data;
    DtEditorFormat(pPad->editor, (DtEditorFormatSettings *) NULL, DtEDITOR_FORMAT_PARAGRAPH);
}


/************************************************************************
 * FormatAllCB - [Format] menu, [All] button
 ************************************************************************/
/* ARGSUSED */
void
FormatAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data)
{
    Editor *pPad = (Editor *)client_data;
    DtEditorFormat(pPad->editor, (DtEditorFormatSettings *) NULL, DtEDITOR_FORMAT_ALL);
}
