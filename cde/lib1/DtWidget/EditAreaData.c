/* $XConsortium: EditAreaData.c /main/cde1_maint/4 1995/11/07 11:44:55 mgreess $ */
/**********************************<+>*************************************
***************************************************************************
**
**  File:        EditAreaData.c
**
**  Project:     DtEditor widget for editing services
**
**  Description:  Contains functions for getting and setting the data
**                on which the editor operates.
**  -----------
**
*******************************************************************
*
* (c) Copyright 1993, 1994 Hewlett-Packard Company
* (c) Copyright 1993, 1994 International Business Machines Corp.
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
*
********************************************************************
**
**
**************************************************************************
**********************************<+>*************************************/
#include "EditorP.h"
#include "DtMsgsP.h"
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/TextP.h>
#include <Xm/MessageB.h>
#include <unistd.h>
#include <values.h>
#include <sys/types.h>
#include <sys/mman.h>


#ifdef README_SUN_CDE_PERF
  
  04/22/96 - Joel Gringorten

  Greatly Enhanced Interface to the Text Widget.
  ----------------------------------------------

  Summary:
  It was noted that the DtEditor Widget consumed large amounts of 
  heap space and CPU time in saving and loading files.  In the case
  of loading a file into DtEditor, 2 unnecessary malloc's of the entire 
  file size and memcopy operations were performed.  When saving files,
  DtEditor would make 4 malloc copies of the data.  However, when
  linewrap conversion was enabled, another huge buffer of 4 times the 
  size of the file was created as well.  Some of the other mallocs
  were a result of using a string interface into the text widget. 
  The text widget should also provide a block-data interface (count
  and data), so that applications dealing with large amount of data
  do not have to strip nulls and zero-terminate buffers just to pass
  them to the text widget.  The net result of these changes are that
  DtEditor now does not use appreciably more than the size of the 
  file in heap space.  Speedups between 5 and 50 times are realized
  depending on the operation.

  Loading files:  Unregistered the text modify callback before loading
  a file.  The callback data which was a malloc of the entire text file
  was unused anyway.  Provided a routine to write block data (ptr & count)
  to the text widget instead of having to convert it to a string.  Use
  mmap to provide a ptr to memory data, instead of buffering the entire
  file in memory.   (If mmap is deemed undesirable, the other alternative
  is to load the file into the text widget in reasonably sized chunks.)

  Saving files: Use the read_source method of the text widget to access
  the data positionally, rather than as one huge multibyte string.  This 
  saves a huge of amount of time in linewrap conversion to linefeed 
  operations, as a memory and cpu intensive solution was employed in the 
  old code. Do not buffer the data read from the text widget, but rather 
  write it directly to the file incrementally.  
#endif

typedef	enum _LoadActionType {
	  LOAD_DATA,
	  INSERT_DATA,
	  APPEND_DATA,
	  REPLACE_DATA
} LoadActionType;

static DtEditorErrorCode Check4EnoughMemory(
	int numBytes);

static DtEditorErrorCode StripEmbeddedNulls(
        DtEditorWidget w,
	char *stringData,
	int *length);

static DtEditorErrorCode LoadFile(
	Widget w,
        char *fileName,
        LoadActionType action,
	XmTextPosition	 startReplace,
	XmTextPosition	 endReplace );

#ifdef NEED_STRCASECMP

/*
 * in case strcasecmp is not provided by the system here is one
 * which does the trick
 */
static int
strcasecmp(s1, s2)
    register char *s1, *s2;
{
    register int c1, c2;

    while (*s1 && *s2) {
	c1 = isupper(*s1) ? tolower(*s1) : *s1;
	c2 = isupper(*s2) ? tolower(*s2) : *s2;
	if (c1 != c2)
	    return (1);
	s1++;
	s2++;
    }
    if (*s1 || *s2)
	return (1);
    return (0);
}

#endif

/*****************************************************************************
 * 
 * Check4EnoughMemory - estimates whether there is enough memory to malloc 
 * "numBytes" of memory.  This routine doubles the amount needed because the 
 * routines  that use it are putting data into the text widget & we must make
 * sure the widget will have room, too.
 * 
 * Returns DtEDITOR_NO_ERRORS 
 *         DtEDITOR_ILLEGAL_SIZE
 *         DtEDITOR_INSUFFICIENT_MEMORY
 * 
 *****************************************************************************/
static DtEditorErrorCode 
Check4EnoughMemory(
	int numBytes)
{

    DtEditorErrorCode returnVal = DtEDITOR_ILLEGAL_SIZE;

    if (numBytes > 0) {
       char *tmpString = (char *)malloc((2 * numBytes) + (numBytes/10));

       if(tmpString == (char *)NULL)
    	 returnVal = DtEDITOR_INSUFFICIENT_MEMORY;  
       else {
         returnVal = DtEDITOR_NO_ERRORS;
         free(tmpString);
       }
    }

    return( returnVal );

} /* end Check4EnoughMemory */


#define YES 1
#define NO 2
#define	charset			XmFONTLIST_DEFAULT_TAG

static void
response(Widget w, void *answer, void *cbs)
{
    XmAnyCallbackStruct *res = (XmAnyCallbackStruct *) cbs;
    
    if (res->reason == XmCR_OK)
	*(int *)answer = YES;
    else if (res->reason == XmCR_CANCEL)
	*(int *)answer = NO;
}


static Boolean
BinaryIncludeWarning(DtEditorWidget w)
{
    Widget dialog, helpBtn;
    Arg args[5];
    int n;
    static int answer = 0;
    Boolean ret;
    XmString tmpStr, titleStr;
    
    tmpStr = XmStringCreateLtoR(WARNING_1, charset);
    titleStr = XmStringCreateLtoR(WARNING_TITLE, charset);
    
    n=0;
    XtSetArg(args[n], XmNmessageString, tmpStr); n++;
    XtSetArg(args[n], XmNdialogTitle, titleStr); n++;
    XtSetArg(args[n], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL);n++;
    
    dialog = XmCreateQuestionDialog((Widget)w, "warning", args, n);

    XtAddCallback(dialog, XmNokCallback, response, &answer);
    XtAddCallback(dialog, XmNcancelCallback, response, &answer);
	
    XtUnmanageChild(XmMessageBoxGetChild(dialog, XmDIALOG_HELP_BUTTON));

    XtManageChild(dialog);

    while(answer == 0)
	XtAppProcessEvent(M_app_context(w), XtIMAll);

    XmStringFree(tmpStr);
    XmStringFree(titleStr);
    
    ret = (answer == YES ? True : False);
    answer = 0;

    return ret;
}


/*****************************************************************************
 * 
 * StripEmbeddedNulls - removes any embedded NULLs (\0) in a string of length 
 * 	'length'.  The removal occurs in place, with 'length' set to the
 *	new, stripped length.  The resulting string is terminated with a 
 *	trailing NULL.  
 * 
 * Returns DtEDITOR_NO_ERRORS - the string did not contain any embedded NULLs
 *         DtEDITOR_NULLS_REMOVED - the string did contain embedded
 *	   			    NULLs that were removed.
 * 
 *****************************************************************************/
static DtEditorErrorCode 
StripEmbeddedNulls(
	DtEditorWidget w,
	char *stringData,
	int *length)
{
    DtEditorErrorCode returnVal = DtEDITOR_NO_ERRORS;
    int nulls=0,   /* how many nulls have we seen */
	i,
	nullChars=0, /* how many nulls in the current 'word' */
	nullWords=0; /* how many null 'words' so far */
    Boolean newWord=False;
    
    if (strlen(stringData) != *length)  /* go in for the nulls */
    {
       int firstNull, lastNull=0;
       
       returnVal = DtEDITOR_NULLS_REMOVED;

       /* Do a quick scan first. We use the worst case scenario, where
	* we're trying to detect nulls in a wide character locale, and
	* the characters are the size of the largest natural type. */
#define NULL_THRESHOLD 5
       
       for (i = 0; i< *length; i++)
       {
	   if (stringData[i] == '\0')
	   {
	       /* nulls++; */
	       nullChars++;
	       
	       if (!newWord)
	       {
		   newWord = True;
	       }
	       else if (nullChars == sizeof(long))
	       {
		   newWord = False;
		   nullWords++;
		   nullChars = 0;
	       }
	   }
	   
	   if (nullWords > NULL_THRESHOLD)
	   {
	       if (BinaryIncludeWarning(w))
		   break;
	       else
		   return DtEDITOR_UNREADABLE_FILE;
	   }
       }

       /*
        * The file contains NULL characters, so we strip them out and
        * report that we have done so.
        */
       while((firstNull = strlen(stringData)) != *length)
       {
	   int block, zeroes=1; /* at least one null */
	   
	   if (lastNull == 0)
	       lastNull = firstNull;

	   while((lastNull + 1 - nulls) < *length &&
		 stringData[lastNull + 1] == (char)'\0')
	   {
	       lastNull++;
	       zeroes++;
	   }
	   
	   /* find next block */
	   block = strlen(&stringData[lastNull+1])+1; /* copy term. null */
	   
	   memcpy(&stringData[firstNull], &stringData[lastNull + 1], block);
	   
	   nulls += zeroes;

	   *length -= zeroes;
	   lastNull += block;
       }
    }

    return( returnVal);

} /* end StripEmbeddedNulls */


/*****************************************************************************
 * 
 * Retrieves the current location of the insert cursor
 * 
 *****************************************************************************/

XmTextPosition
DtEditorGetInsertionPosition(
        Widget widget)
{
    DtEditorWidget editor = (DtEditorWidget) widget;

    return ( XmTextGetInsertionPosition(M_text(editor)) ) ;

} /* end DtEditorGetInsertionPosition */


/*****************************************************************************
 * 
 * Retrieves the current location of the last character in the widget
 * 
 *****************************************************************************/

XmTextPosition
DtEditorGetLastPosition(
        Widget widget)
{
    DtEditorWidget editor = (DtEditorWidget) widget;

    return ( XmTextGetLastPosition(M_text(editor)) ) ;

} /* end DtEditorGetLastPosition */


/*****************************************************************************
 * 
 * Changes the current location of the insert cursor
 * 
 *****************************************************************************/

void 
DtEditorSetInsertionPosition(
        Widget widget,
	XmTextPosition position)
{
    DtEditorWidget editor = (DtEditorWidget) widget;

    XmTextSetInsertionPosition(M_text(editor), position);

} /* end DtEditorSetInsertionPosition */

static DtEditorErrorCode 
setStringValue( 
        DtEditorWidget editor,
	char *data)
{
    /* 
     * Tell _DtEditorModifyVerifyCB() that we're replacing the entire
     * contents, so it doesn't try to save the current document in an
     * undo structure for a later undo.
     */
    M_loadingAllNewData(editor) = True;

    XmTextSetString( M_text(editor), data );


    /* 
     * If the _DtEditorModifyVerifyCB() did not get called, reset the
     * things which usually get reset there.  The modifyVerify callback
     * will not get called if the contents are being set to a null string 
     * and the widget is already empty.
     */
    if (M_loadingAllNewData(editor) == True) {
      M_loadingAllNewData(editor) = False;
      M_unreadChanges(editor) = False;
      _DtEditorResetUndo(editor);
    }

    return( DtEDITOR_NO_ERRORS );

} /* end setStringValue */


#ifdef SUN_CDE_PERF

/* The following routine should be called _XmStringSourceSetData and
 * should really exist in Xm/TextStrSo.c, but this would cause a 
 * dependency on a special libXm. So, we'll just make it local.
 */

void 
_DtStringSourceSetData(
        XmTextWidget widget,
        char *value,
	int count)
{
    XmTextSource source = widget->text.source;
    XmSourceData data = source->data;
    Boolean editable, freeBlock;
    int maxallowed;
    XmTextBlockRec block, newblock;
    XmTextPosition fromPos = 0; 
    XmTextPosition toPos = data->length;
    XmTextPosition cursorPos;

    (*source->SetSelection)(source, 1, 0,
			    XtLastTimestampProcessed(XtDisplay(widget)));
    block.format = XmFMT_8_BIT;
    block.length = count;
    block.ptr = value;
    editable = data->editable;
    maxallowed = data->maxallowed;
    data->editable = TRUE;
#ifndef OSF_v1_2_4
    data->maxallowed = MAXINT;
#else /* OSF_v1_2_4 */
    data->maxallowed = INT_MAX;
#endif /* OSF_v1_2_4 */
    XtFree((char*)(widget->text.url_highlight.list));
    widget->text.url_highlight.list = NULL;
    widget->text.url_highlight.number = 0;
    widget->text.url_highlight.maximum = 0;
    XmTextSetHighlight((Widget)widget, 0, widget->text.last_position, 
		       XmHIGHLIGHT_NORMAL);

    if (_XmTextModifyVerify(widget, NULL, &fromPos, &toPos,
		            &cursorPos, &block, &newblock, &freeBlock)) {
       (void)(source->Replace)(widget, NULL, &fromPos, &toPos,
			       &newblock, False);
       if (freeBlock && newblock.ptr) XtFree(newblock.ptr);
       _XmTextValueChanged(widget, NULL);
    }

    data->editable = editable;
    data->maxallowed = maxallowed;
}

#endif

static DtEditorErrorCode 
setDataValue( 
        DtEditorWidget  widget,
	void    *rawData,
	int	length)
{
   DtEditorErrorCode status = DtEDITOR_NULL_ITEM, tmpError;

   /*
    * Validate input
    */
   if (rawData != (void *)NULL)
#ifdef SUN_CDE_PERF
   {
     /* bug fix 4058477 let ModifyVerifyCB -called via _XmTextModifyVerify-
        know all new data has been loaded so don't query for save on quitting */
     M_loadingAllNewData(widget) = True;
     _DtStringSourceSetData((XmTextWidget)M_text(widget), rawData, length);
   }

   status = DtEDITOR_NO_ERRORS; 
#else
   {

      /* 
       * Check to see if we have a valid buffer size & enough memory to 
       * load the buffer into the text widget.  This is only an estimate 
       * of our needs.
       * Check4EnoughMemory() returns DtEDITOR_NO_ERRORS, 
       * DtEDITOR_ILLEGAL_SIZE, or  DtEDITOR_INSUFFICIENT_MEMORY.
       */

      status = Check4EnoughMemory( length );
      if (status == DtEDITOR_NO_ERRORS) 
      {

        /*
         * Convert the data buffer into a string & insert into the widget
         */
        char *textData = (char *)XtMalloc(length + 1);
        memcpy( textData, rawData, length );
        textData[length] = '\0';

        /* 
         * Strip out any embedded NULLs because the text widget will only 
	 * accept data up to the first NULL.
         * 
         * StripEmbeddedNulls() returns DtEDITOR_NO_ERRORS or 
         * DtEDITOR_NULLS_REMOVED
         */
        status = StripEmbeddedNulls( widget, textData, &length );

        /*
         * Now, insert the converted string into the text widget
         */
	if (status == DtEDITOR_NO_ERRORS || status == DtEDITOR_READ_ONLY_FILE
	    || status == DtEDITOR_NULLS_REMOVED)
	{
	    tmpError = setStringValue( widget, textData );
	    if (tmpError != DtEDITOR_NO_ERRORS)
		status = tmpError;
	}
	
        XtFree( (char *)textData );
      }
   }
#endif
   return ( status );
} /* end setDataValue */

static DtEditorErrorCode 
setWcharValue( 
        DtEditorWidget editor,
	wchar_t *data)
{
   DtEditorErrorCode status;
   wchar_t *tmp_wc;
   int result, num_chars=0;
   char *mb_value = (char *)NULL;


   /* 
    * Convert the wide char string to a multi-byte string & stick it in
    * the text widget.
    */
    
   /* 
    * Determine how big the resulting mb string may be 
    */
   for (num_chars = 0, tmp_wc = data; *tmp_wc != (wchar_t)0L; num_chars++) 
     tmp_wc++;

   /* 
    * Check to see if we have enough memory to load the string
    * into the text widget.  This is only an estimate of our needs.
    * status will be set to DtEDITOR_NO_ERRORS, DtEDITOR_ILLEGAL_SIZE, or 
    * DtEDITOR_INSUFFICIENT_MEMORY.
    */
   status = Check4EnoughMemory( (num_chars + 1) * MB_CUR_MAX );
   if(status != DtEDITOR_NO_ERRORS) 
       return( status );	  

   mb_value = XtMalloc( (unsigned)(num_chars + 1) * MB_CUR_MAX );

   /*
    * Convert the wchar string
    * If wcstombs fails it returns (size_t) -1, so pass in empty
    * string.
    */
   result = wcstombs( mb_value, data, (num_chars + 1) * MB_CUR_MAX );
   if (result == (size_t)-1) 
     result = 0;

   /*
    * wcstombs doesn't guarantee string is NULL terminated
    */
   mb_value[result] = 0;

   status = setStringValue( editor, mb_value );

   XtFree(mb_value);

   return( status );

} /* end setWcharValue */


static DtEditorErrorCode 
insertStringValue( 
        DtEditorWidget	 editor,
	char 		*data,
	LoadActionType   typeOfInsert,
	XmTextPosition	 beginInsert,
	XmTextPosition	 endInsert)
{
    int numInserted;

    switch( typeOfInsert ) 
    {
      case INSERT_DATA:
      {
        beginInsert = endInsert = XmTextGetInsertionPosition( M_text(editor) );
	break;
      }

      case APPEND_DATA:
      {
        beginInsert = endInsert = XmTextGetLastPosition( M_text(editor) );
	break;
      }

      case REPLACE_DATA:
      {
	break;
      }

      default:
      {
      }
    } /* end switch */

    /*
     * Insert/Replace/Append the data and move the insertion cursor to
     * the end of the inserted data.
     */
    numInserted = _DtEditor_CountCharacters( data, strlen(data) );
    XmTextReplace(M_text(editor), beginInsert, endInsert, data);
    XmTextSetInsertionPosition( M_text(editor), 
	  		        (XmTextPosition)(beginInsert + numInserted) );

    return( DtEDITOR_NO_ERRORS );
} /* insertStringValue */

static DtEditorErrorCode 
insertDataValue( 
        DtEditorWidget  widget,
	void    *rawData,
	int	length,
	LoadActionType   typeOfInsert,
	XmTextPosition	 beginInsert,
	XmTextPosition	 endInsert)
{
   char *textData;
   DtEditorErrorCode status = DtEDITOR_NULL_ITEM, loadError;

   /*
    * Validate input
    */
   if (rawData != (void *) NULL)
   {

      /* 
       * Check to see if we have a valid buffer size & enough memory to 
       * insert the buffer into the text widget.  This is only an estimate 
       * of our needs.
       * status will be set to DtEDITOR_NO_ERRORS, DtEDITOR_ILLEGAL_SIZE, or 
       * DtEDITOR_INSUFFICIENT_MEMORY.
       */
      status = Check4EnoughMemory( length );
      if (status == DtEDITOR_NO_ERRORS) 
      {

         /*
          * Convert the data buffer into a string & insert into the widget
          */
         char *textData = (char *)XtMalloc(length + 1);
         memcpy( textData, rawData, length );
         textData[length] = '\0';

	 /* 
          * Strip out any embedded NULLs because the text widget will only 
	  * accept data up to the first NULL.
	  *
          * StripEmbeddedNulls() returns DtEDITOR_NO_ERRORS or 
          * DtEDITOR_NULLS_REMOVED
          */
         status = StripEmbeddedNulls( widget, textData, &length );

	 /*
	  * Now, insert the converted string into the text widget
	  */
	 if (status == DtEDITOR_NO_ERRORS || status == DtEDITOR_READ_ONLY_FILE
	     || status == DtEDITOR_NULLS_REMOVED)
	 {
	     loadError = insertStringValue( widget, textData, typeOfInsert, 
					    beginInsert, endInsert );
	     if (loadError != DtEDITOR_NO_ERRORS)
		 status = loadError;
	 }
	
	 XtFree( (char *)textData );
      }
   }

   return( status );

} /* insertDataValue */


static DtEditorErrorCode 
insertWcharValue( 
        DtEditorWidget	 editor,
	wchar_t 	 *data,
	LoadActionType   typeOfInsert,
	XmTextPosition	 beginInsert,
	XmTextPosition	 endInsert)
{

   wchar_t *tmp_wc;
   int result, num_chars=0;
   char *mb_value = (char *)NULL;
   DtEditorErrorCode status;


   /* 
    * Convert the wide char string to a multi-byte string & insert it into
    * the text widget.
    */
    
   /* 
    * Determine how big the resulting mb string may be 
    */
   for (num_chars = 0, tmp_wc = data; *tmp_wc != (wchar_t)0L; num_chars++) 
     tmp_wc++;

   /* 
    * Check to see if we have enough memory to insert the string
    * into the text widget.  This is only an estimate of our needs.
    * status will be set to DtEDITOR_NO_ERRORS, DtEDITOR_ILLEGAL_SIZE, or 
    * DtEDITOR_INSUFFICIENT_MEMORY.
    */

   status = Check4EnoughMemory( (num_chars + 1) * MB_CUR_MAX );
   if(status != DtEDITOR_NO_ERRORS) 
       return( status );	  

   mb_value = XtMalloc( (unsigned)(num_chars + 1) * MB_CUR_MAX );

   /*
    * Convert the wchar string.
    * If wcstombs fails it returns (size_t) -1, so pass in empty
    * string.
    */
   result = wcstombs( mb_value, data, (num_chars + 1) * MB_CUR_MAX );
   if (result == (size_t)-1) 
     result = 0;

   /*
    * wcstombs doesn't guarantee string is NULL terminated
    */
   mb_value[result] = 0;

   status = insertStringValue( editor, mb_value, typeOfInsert, 
			       beginInsert, endInsert );
   XtFree( mb_value );

   return( status );

} /* insertWcharValue */


/***************************************************************************
 *
 * DtEditorSetContents - sets the contents of the DtEditor widget.
 *
 *      Inputs: widget to set the contents
 *
 *              a data structure containing the data to put into the
 *		widget.  Depending upon the type of data being set, this
 * 		structure will contain various fields:
 *
 *		string - \0-terminated string of characters
 *		data - the data, the size of the data
 *
 *      Returns  0 - contents were set sucessfully
 *		!0 - an error occured while setting the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorSetContents(
        Widget 			widget,
        DtEditorContentRec 	*data )
{
   DtEditorErrorCode	error = DtEDITOR_INVALID_TYPE;
   DtEditorWidget editor = (DtEditorWidget) widget;

   switch( data->type ) 
   {
     case DtEDITOR_TEXT:
     {
       error = setStringValue ( editor, data->value.string );
       break;
     }
     case DtEDITOR_DATA:
     {
       error = setDataValue ( editor, data->value.data.buf,
       			      data->value.data.length);
       break;
     }
     case DtEDITOR_WCHAR:
     {
       error = setWcharValue ( editor, data->value.wchar );
       break;
     }
     default :
     {
       error = DtEDITOR_INVALID_TYPE;
     }
   } /* end switch */

   /*
    * Update the current-line-display in the status line 
    */
   if (error == DtEDITOR_NO_ERRORS)
     _DtEditorUpdateLineDisplay(editor, 1, False );

   return( error );

} /* end DtEditorSetContents */


/***************************************************************************
 *
 * DtEditorSetContentsFromFile - read a data file, putting the contents 
 *			  	   into a DtEditor widget.
 *
 *      Inputs: widget to load the file into 
 *
 *		to indicate the type of contents loaded from the file:
 *		  string - a \0-terminated string of characters
 *		  data - untyped data
 *
 *		filename - name of the file to read 
 *
 *      Returns  0 - contents were loaded sucessfully
 *		!0 - an error occured while loading the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorSetContentsFromFile(
        Widget 			widget,
	char			*fileName)
{

   return( LoadFile(widget, fileName, LOAD_DATA, 0, 0) );

} /* end DtEditorSetContentsFromFile */


/***************************************************************************
 *
 * DtEditorAppend - append data to the contents of the DtEditor widget.
 *
 *      Inputs: widget to add to the contents
 *
 *              a data structure containing the data to append to the
 *		widget.  Depending upon the type of data being set, this
 * 		structure will contain various fields:
 *
 *		string - \0-terminated string of characters
 *		data - the data, the size of the data
 *
 *      Returns  0 - contents were set sucessfully
 *		!0 - an error occured while setting the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorAppend(
        Widget 			widget,
        DtEditorContentRec 	*data )
{
   DtEditorErrorCode	error = DtEDITOR_INVALID_TYPE;
   DtEditorWidget editor = (DtEditorWidget) widget;

   switch( data->type ) 
   {
     case DtEDITOR_TEXT:
     {
       error = insertStringValue ( editor, data->value.string, 
				   APPEND_DATA, 0, 0 );
       break;
     }
     case DtEDITOR_DATA:
     {
       error = insertDataValue ( editor, data->value.data.buf, 
				 data->value.data.length, APPEND_DATA, 0,0);
       break;
     }
     case DtEDITOR_WCHAR:
     {
       error = insertWcharValue ( editor, data->value.wchar,
				   APPEND_DATA, 0, 0 );
       break;
     }
     default:
     {
       error = DtEDITOR_INVALID_TYPE;
     }
   } /* end switch */

   return( error );
} /* end DtEditorAppend */


/***************************************************************************
 *
 * DtEditorAppendFromFile - read a data file, appending the contents 
 *			  	   into a DtEditor widget.
 *
 *      Inputs: widget to append the file to 
 *
 *		to indicate the type of contents appended from the file:
 *		  string - a \0-terminated string of characters
 *		  data - untyped data
 *
 *		filename - name of the file to read 
 *
 *      Returns  0 - contents were appended sucessfully
 *		!0 - an error occured while appending the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorAppendFromFile(
        Widget 			widget,
	char			*fileName)
{

   return( LoadFile(widget, fileName, APPEND_DATA, 0, 0) );

} /* end DtEditorAppendFromFile */


/***************************************************************************
 *
 * DtEditorInsert - insert data into the contents of the DtEditor widget.
 *
 *      Inputs: widget to add to the contents
 *
 *              a data structure containing the data to insert into the
 *		widget.  Depending upon the type of data being set, this
 * 		structure will contain various fields:
 *
 *		string - \0-terminated string of characters
 *		data - the data, the size of the data
 *
 *      Returns  0 - contents were set sucessfully
 *		!0 - an error occured while setting the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorInsert(
        Widget 			widget,
        DtEditorContentRec 	*data )
{
   DtEditorErrorCode	error = DtEDITOR_INVALID_TYPE;
   DtEditorWidget editor = (DtEditorWidget) widget;

   switch( data->type ) 
   {
     case DtEDITOR_TEXT:
     {
       error = insertStringValue ( editor, data->value.string, 
				   INSERT_DATA, 0, 0 );
       break;
     }
     case DtEDITOR_DATA:
     {
       error = insertDataValue ( editor, data->value.data.buf, 
				 data->value.data.length, INSERT_DATA, 0,0);
       break;
     }
     case DtEDITOR_WCHAR:
     {
       error = insertWcharValue ( editor, data->value.wchar,
				   INSERT_DATA, 0, 0 );
       break;
     }
     default :
     {
       error = DtEDITOR_INVALID_TYPE;
     }
   } /* end switch */

   return( error );
} /* end DtEditorInsert */


/***************************************************************************
 *
 * DtEditorInsertFromFile - read a data file, inserting the contents 
 *			  	   into a DtEditor widget.
 *
 *      Inputs: widget to insert the file to 
 *
 *		to indicate the type of contents inserted from the file:
 *		  string - a \0-terminated string of characters
 *		  data - untyped data
 *
 *		filename - name of the file to read 
 *
 *      Returns  0 - contents were inserted sucessfully
 *		!0 - an error occured while inserting the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorInsertFromFile(
        Widget 			widget,
	char			*fileName)
{

   return( LoadFile(widget, fileName, INSERT_DATA, 0, 0) );

} /* end DtEditorInsertFromFile */


/***************************************************************************
 *
 * DtEditorReplace - replace a specified portion of the contents of the 
 *		     DtEditor widget with the supplied data.
 *
 *      Inputs: widget to replace a portion of its contents
 *
 *		starting character position of the portion to replace
 *
 *		ending character position of the portion to replace
 *
 *              a data structure containing the data to replace some data 
 *		in the widget.  Depending upon the type of data being set, 
 * 		this structure will contain various fields:
 *
 *		string - \0-terminated string of characters
 *		data - the data, the size of the data
 *
 *
 *      Returns  0 - the portion was replaced sucessfully
 *		!0 - an error occured while replacing the portion 
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorReplace(
        Widget 			widget,
        XmTextPosition 		startPos,
        XmTextPosition 		endPos,
        DtEditorContentRec 	*data)
{
   DtEditorErrorCode	error = DtEDITOR_INVALID_TYPE;
   DtEditorWidget editor = (DtEditorWidget) widget;
   XmTextWidget tw = (XmTextWidget) M_text(editor);

   if( startPos < 0 )
        startPos = 0;
   if( startPos > tw->text.last_position )
        startPos = tw->text.last_position;

   if( endPos < 0 )
        endPos = 0;
   if( endPos > tw->text.last_position )
        endPos = tw->text.last_position;

   if( startPos > endPos )
   {
        error = DtEDITOR_INVALID_RANGE;
   }
   else
   {
     switch( data->type ) 
     {
       case DtEDITOR_TEXT:
       {
         error = insertStringValue ( editor, data->value.string, 
				     REPLACE_DATA, startPos, endPos );
         break;
       }
       case DtEDITOR_DATA:
       {
         error = insertDataValue ( editor, data->value.data.buf, 
				   data->value.data.length, REPLACE_DATA, 
				   startPos, endPos );
         break;
       }
       case DtEDITOR_WCHAR:
       {
         error = insertWcharValue ( editor, data->value.wchar,
				    REPLACE_DATA, startPos, endPos );
         break;
       }
       default :
       {
         error = DtEDITOR_INVALID_TYPE;
       }
     } /* end switch */

   }

   return( error );

} /* end DtEditorReplace */


/***************************************************************************
 *
 * DtEditorReplaceFromFile - read a data file, using the contents to replace
 *			     a specified portion of the contntes of a 
 *			     DtEditor widget.
 *
 *      Inputs: widget to insert the file to 
 *
 *		starting character position of the portion to replace
 *
 *		ending character position of the portion to replace
 *
 *		to indicate the type of contents inserted from the file:
 *		  string - a \0-terminated string of characters
 *		  data - untyped data
 *
 *		filename - local name of the file to read 
 *
 *      Returns  0 - contents were inserted sucessfully
 *		!0 - an error occured while inserting the contents
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorReplaceFromFile(
        Widget 			widget,
        XmTextPosition 		startPos,
        XmTextPosition 		endPos,
	char			*fileName)
{

   DtEditorWidget editor = (DtEditorWidget) widget;
   XmTextWidget tw = (XmTextWidget) M_text(editor);

   if( startPos < 0)
        startPos = 0;
   if( startPos > tw->text.last_position )
        startPos = tw->text.last_position;

   if( endPos < 0 )
        endPos = 0;
   if( endPos > tw->text.last_position )
        endPos = tw->text.last_position;

   if(startPos > endPos)
   {
        return ( DtEDITOR_INVALID_RANGE );
   }
   else
   {
        return( LoadFile(widget, fileName, REPLACE_DATA, startPos, endPos) );
   }

} /* end DtEditorReplaceFromFile */


/***************************************************************************
 *
 * _DtEditorValidateFileAccess - check to see if file exists, whether we
 *			           can get to it, and whether it is readable
 *				   or writable.
 *
 *	Note: does not check whether files for reading are read only.
 *
 *      Inputs: filename - name of the local file to read 
 *			   flag indicating whether we want to read or write
 *			   the file.
 *		
 *      Returns  0  file exists & we have read or write permissions.
 *
 *		>0  if file cannot be read from/written to.
 *		    errno is set to one of the following values:
 *
 *	General errors:	
 *		DtEDITOR_INVALID_FILENAME - 0 length filename
 *		DtEDITOR_NONEXISTENT_FILE - file does not exist
 * 		  (Note: this may not be considered an error when saving 
 *		   to a file.  The file may just need to be created.)
 *		DtEDITOR_NO_FILE_ACCESS - cannot stat existing file 
 *		DtEDITOR_DIRECTORY - file is a directory
 *		DtEDITOR_CHAR_SPECIAL_FILE - file is a device special file
 *		DtEDITOR_BLOCK_MODE_FILE - file is a block mode file
 *
 *	additional READ_ACCESS errors:	
 *		DtEDITOR_UNREADABLE_FILE - 
 *
 *	additional WRITE_ACCESS errors:	
 *		DtEDITOR_UNWRITABLE_FILE - 
 *				file or directory is write protected for 
 *				another reason
 *
 ***************************************************************************/

extern DtEditorErrorCode
_DtEditorValidateFileAccess(
	char	*fileName,
	int	accessType )
{
   struct stat statbuf;	/* Information on a file. */
   unsigned short tmpMode;

   DtEditorErrorCode error = DtEDITOR_INVALID_FILENAME;

   /*
    * First, make sure we were given a name 
    */
   if (fileName && *fileName ) 
   { 

     /*
      * Does the file already exist?
      */
     if ( access(fileName, F_OK) != 0 )
        error = DtEDITOR_NONEXISTENT_FILE; 
     else
     { 
        error = DtEDITOR_NO_ERRORS;

	/*
 	 * The file exists, so lets do some type checking 
	 */

	if( stat(fileName, &statbuf) != 0 ) 
	  error = DtEDITOR_NO_FILE_ACCESS;
	else
	{ 

	  /* if its a directory - can't save */ 
	  if( (statbuf.st_mode & S_IFMT) == S_IFDIR ) 
	  { 
	     error = DtEDITOR_DIRECTORY;
	     return( error );
          }

          /* if its a character special device - can't save */
          if( (statbuf.st_mode & S_IFMT) == S_IFCHR ) 
	  {
             error = DtEDITOR_CHAR_SPECIAL_FILE;
	     return( error );
          }

          /* if its a block mode device - can't save */
          if((statbuf.st_mode & S_IFMT) == S_IFBLK) 
	  {
             error = DtEDITOR_BLOCK_MODE_FILE;
	     return( error );
          }

	  /* 
	   * We now know that it's a regular file so check to whether we
	   * can read or write to it, as appropriate.  
	   */

	  switch( accessType )
	  {
	    case READ_ACCESS:
	    {

	      if( access(fileName, R_OK) != 0 ) 
	         error = DtEDITOR_UNREADABLE_FILE;

	      break;
	    }

	    case WRITE_ACCESS:
	    {

	      if( access(fileName, W_OK) == 0 )
	      {
	    	/*
	     	 * Can write to it. 
	     	 */
		error = DtEDITOR_WRITABLE_FILE;
	      }
	      else
	      {
	    	/*
	     	 * Can't write to it.  
	     	 */
                 error = DtEDITOR_UNWRITABLE_FILE;

	      } /* end no write permission */ 

	      break;
	    }

	    default:
	    {
	      break;
	    }

	  } /* end switch */
	} /* end stat suceeded */

      } /* end file exists */

   } /* end filename passed in */

   return( error );

} /* end _DtEditorValidateFileAccess */


/************************************************************************
 *
 *  LoadFile - Check if file exists, whether we can get to it, etc.  
 *	       If so, type and read its contents.
 *
 *	Inputs: widget to set, add, or insert contents of file into
 *
 *		name of file to read
 *
 *		type of file (NULL). This will be set by LoadFile
 *
 *		action to perform with the data (load, append, insert, 
 *		  replace a portion, attach)
 *
 *		The following information will be used if the file
 *		  contents will replace a portion of the widget's contents:
 *
 * 		starting character position of the portion to replace
 *
 *		ending character position of the portion to replace
 *
 *	Returns: DtEDITOR_NO_ERRORS - file was read sucessfully
 *		 DtEDITOR_READ_ONLY_FILE - file was read sucessfully but 
 *					     is read only
 *		 DtEDITOR_DIRECTORY - the file is a directory
 *		 DtEDITOR_CHAR_SPECIAL_FILE - the file is a character
 *					   special device
 *		 DtEDITOR_BLOCK_MODE_FILE - the file is a block mode device
 *		 DtEDITOR_NONEXISTENT_FILE - file does not exist
 *		 DtEDITOR_NULLS_REMOVED - file contained embedded NULLs
 *					  that were removed
 *		 DtEDITOR_INSUFFICIENT_MEMORY - unable to allocate
 *					  enough memory for contents of file
 *
 ************************************************************************/

static DtEditorErrorCode 
LoadFile(
	Widget 		 w,
        char 		 *fileName,
        LoadActionType 	 action,
	XmTextPosition	 startReplace,
	XmTextPosition	 endReplace )

{
    DtEditorWidget editor = (DtEditorWidget) w;
    DtEditorContentRec cr;	/* Structure for passing data to widget */
    struct stat statbuf;	/* Information on a file. */
    off_t file_length;            /* Length of file. */
    FILE *fp = NULL;		/* Pointer to open file */
    DtEditorErrorCode returnVal = DtEDITOR_NONEXISTENT_FILE; 
		                /* Error accessing file & reading contents */
    DtEditorErrorCode loadError=DtEDITOR_NO_ERRORS;
		                /* Error from placing bits into text widget */

    /*
     * First, make sure we were given a name 
     */

    if (fileName && *fileName ) 
    { 

      /*
       * Can we read the file?
       */
      returnVal = _DtEditorValidateFileAccess( fileName, READ_ACCESS );

      if( returnVal == DtEDITOR_NO_ERRORS )
      {
	/*
	 * Open the file for reading.  If we can read/write, then we're
	 * cool, otherwise we might need to tell the user that the
	 * file's read-only, or that we can't even read from it.
	 */
	if( (fp = fopen(fileName, "r+")) == NULL ) 
	{
	  /*
	   * We can't update (read/write) the file so try opening read-
	   * only
	   */
	  if( (fp = fopen(fileName, "r")) == NULL )
	  {
	    /*
	     * We can't read from the file.
	     */
	    return ( DtEDITOR_UNREADABLE_FILE );
	  }
	  else
	  {
	    /*
	     * Tell the application that the file's read-only.
	     * Becareful not to overwrite this value with one of the calls 
	     * to set the widget's contents.
	     */
	    returnVal = DtEDITOR_READ_ONLY_FILE;
	  }

	} /* end open for read/write */

      } /* end try to read the file */ 


    } /* end if no filename */

    /* If a file is open, get the bytes */
    if ( fp )
    {
	stat( fileName, &statbuf );
	file_length = statbuf.st_size;

#ifndef SUN_CDE_PERF
	/* 
	 * Check to see if we have enough memory to load the file contents
	 * into the text widget.  This is only an estimate of our needs.
         * Check4EnoughMemory() returns DtEDITOR_NO_ERRORS, 
	 * DtEDITOR_ILLEGAL_SIZE, or DtEDITOR_INSUFFICIENT_MEMORY.
	 */
        loadError = Check4EnoughMemory( file_length );
	if( loadError == DtEDITOR_INSUFFICIENT_MEMORY ) {
	    returnVal == loadError;
	}
#else
	if(0) ;
#endif
	else {

	  /* 
	   * Read the file contents (with room for null) & convert to a
	   * string.  We want to use a string because the 
	   * DtEditorSetContents/Append/Insert/... functions create another 
	   * copy of the data before actually putting it into the widget.
	   */
#ifndef SUN_CDE_PERF
	  char *file_string = (char *)XtMalloc(file_length + 1); 
	  file_length = fread(file_string, sizeof(char), file_length, fp);

	  if (!file_length)
	  {
	      fclose(fp);
	      return ( DtEDITOR_UNREADABLE_FILE );
	  }
#else
	  char *ptr;
     	  ptr = mmap((caddr_t) 0, file_length,
				   (PROT_READ | PROT_WRITE),
				   MAP_PRIVATE, fileno(fp), 0);

	  if (ptr == MAP_FAILED)
	  {
	      fclose(fp);
	      return ( DtEDITOR_UNREADABLE_FILE );
	  }
#endif
#ifndef SUN_CDE_PERF

	  file_string[file_length] = '\0';

	  /* 
           * Strip out any embedded NULLs because the text widget will only 
	   * accept data up to the first NULL.
	   *
           * StripEmbeddedNulls() returns DtEDITOR_NO_ERRORS or 
           * DtEDITOR_NULLS_REMOVED
           */
          loadError = StripEmbeddedNulls( editor, file_string, &file_length );
          if ( loadError != DtEDITOR_NO_ERRORS )
            returnVal = loadError;
#endif

#ifdef SUN_CDE_PERF
	  /* 
	   * Insert it as DATA, otherwise the following DtEditor*()
	   * functions will make another copy of the data.
	   */
          cr.type = DtEDITOR_DATA;
          cr.value.data.buf = ptr;
          cr.value.data.length = file_length;
#else
	  /* 
	   * Insert it as a string, otherwise the following DtEditor*()
	   * functions will make another copy of the data.
	   */
          cr.type = DtEDITOR_TEXT;
          cr.value.string = file_string;

#endif
          /*
           * Load, insert, append, or attach the file, as specified
           */
          switch( action )
          {
	      case LOAD_DATA:
	      {
		DtEditorWidget dw = (DtEditorWidget) w;
#ifdef SUN_CDE_PERF
		/* inhibit the modify callback for the load
		 * as it causes the file to be malloced and copied
		 * just so it can be fed to the callback, where its
		 * ignored anyhow.
		 */
		XtRemoveCallback(M_text(dw),
				 XmNmodifyVerifyCallback,
				 (XtCallbackProc) _DtEditorModifyVerifyCB,
				 (XtPointer) w);

#endif
     	        loadError = DtEditorSetContents ( w, &cr);
#ifdef SUN_CDE_PERF

		XtAddCallback(M_text(dw), 
			      XmNmodifyVerifyCallback,
			      (XtCallbackProc) _DtEditorModifyVerifyCB,
			      (XtPointer) w);

		_DtEditorResetUndo(dw);
		M_unreadChanges(dw) = False;
		M_loadingAllNewData(dw) = False;
#endif
	        break;
	      }

	      case INSERT_DATA:
	      {
   	        loadError = DtEditorInsert ( w, &cr );
	        break;
	      }

	      case APPEND_DATA:
	      {
   	        loadError = DtEditorAppend ( w, &cr );
	        break;
	      }

	      case REPLACE_DATA:
	      {
   	        loadError = DtEditorReplace(w, startReplace, endReplace, &cr);
	        break;
	      }

	      default:
	      {
	      }
          } /* end switch */
	    
          if ( loadError != DtEDITOR_NO_ERRORS )
            returnVal = loadError;

          /*
           * The file is loaded, clean up.
           */
#ifndef SUN_CDE_PERF
	             XtFree( file_string ); 
#endif
	  munmap( ptr, file_length );
	} /* end there is enough memory */
 
        /* Close the file */
        fclose(fp);

    } /* end if a file is open */

    return( returnVal );

} /* end LoadFile */


static char *
StringAdd(
	char *destination,
	char *source,
	int number)
{
    memcpy(destination, source, number);
    destination[number] = (char)'\0';
    destination += number;
    return destination;
}

/***************************************************************************
 *
 * CopySubstring - copies out a portion of the text, optionally
 *                 adding newlines at any and all wordwrap-caused
 *                 "virtual" lines.
 *
 *      Inputs: widget from which we get the data to write;
 *              startPos determines the first character to write out;
 *              endPos determines the last character to write out;
 *              buf is the character buffer into which we write. It
 *                  is assumed to be large enough - be careful.
 *              addNewlines specifies whether to add '/n' to "virtual" lines.
 *      Returns Nuthin' (...yes it does)
 *
 *
 ***************************************************************************/

#ifndef SUN_CDE_PERF

static char *
CopySubstring(
        XmTextWidget widget,
        XmTextPosition startPos,
        XmTextPosition endPos,
        char *buf,
        Boolean addNewlines)
{
    register XmTextLineTable line_table = widget->text.line_table;
    int last_line_index, currLine, firstLine;
    char *pString, *pCurrChar, *pLastChar;
    int numToCopy;
    int size;

    if(startPos < 0)
        startPos = 0;
    if(startPos > widget->text.last_position)
        startPos = widget->text.last_position;

    if(endPos < 0)
        endPos = 0;
    if(endPos > widget->text.last_position)
        endPos = widget->text.last_position;

    if(startPos > endPos)
        return buf;

    pString = XmTextGetString((Widget)widget);

    if(addNewlines == False)
    {

        pCurrChar = _DtEditorGetPointer(pString, startPos);
        pLastChar = _DtEditorGetPointer(pString, endPos);

	/* mblen can return -1 ... if it does size s/b 1 */
        if ( (size = mblen(pLastChar, MB_CUR_MAX)) < 0) size = 1;
	numToCopy = pLastChar - pCurrChar + size;

	buf = StringAdd(buf, pCurrChar, numToCopy);
    }
    else
    {
	int *mb_str_loc, total, z, siz;
	char *bptr;

	mb_str_loc = (int *) XtMalloc((sizeof(int) *
			((endPos - startPos) + 1)));

    /*
     * mb_str_loc[] is being used to replace the call
     * to _DtEditorGetPointer.  That function used
     * mbtowc() to count the number of chars between the
     * beginning of pString and startChar.  The problem
     * was that it sat in a loop and was also called for
     * every line, so it was SLOW.  Now, we count once
     * and store the results in mb_str_loc[].
     */

	/* Because startPos may not always == 0:	*/
	/*     mb_str_loc[0] = startPos 		*/
	/*     mb_str_loc[endPos - startPos] = endPos 	*/
	/*						*/
	/* So when accessing items, dereference off of  */
	/*  startPos.					*/

	mb_str_loc[0] = 0;
	for(total=0, bptr=pString, z=1;
		z <= (endPos - startPos); bptr += siz, z++)
	{
	   if (MB_CUR_MAX > 1)
	   {
	      if ( (siz = mblen(bptr, MB_CUR_MAX)) < 0)
	      {
		siz = 1;
		total += 1;
	      }
	      else
		total += siz;
	   }
	   else
	   {
		siz = 1;
		total += 1;
	   }

	     mb_str_loc[z] = total;
	}


        firstLine = currLine = _DtEditorGetLineIndex(widget, startPos);
        do
        {
            if(startPos > (XmTextPosition)line_table[currLine].start_pos)
		pCurrChar = pString + mb_str_loc[0];
            else
	    {
		z = line_table[currLine].start_pos;
		pCurrChar = pString +
			mb_str_loc[z - startPos];
	    }

            if(addNewlines == True && currLine > firstLine &&
               line_table[currLine].virt_line != 0)
	    {
		buf[0] = (char)'\n';
		buf[1] = (char)'\0';
		buf++;
	    }

            if(currLine >= (widget->text.total_lines - 1))
		  pLastChar = pString +
			mb_str_loc[endPos - startPos];
            else if((XmTextPosition)line_table[currLine + 1].start_pos <= endPos)
	    {
		  z = line_table[currLine+1].start_pos - 1;
		  pLastChar = pString +
			mb_str_loc[z - startPos];
	    }
            else
		  pLastChar = pString +
			mb_str_loc[endPos - startPos];

            if ( (size = mblen(pLastChar, MB_CUR_MAX)) < 0) size = 1;
	    numToCopy = pLastChar - pCurrChar + size;

	    buf = StringAdd(buf, pCurrChar, numToCopy);

            currLine++;
        } while(currLine < widget->text.total_lines &&
                (XmTextPosition)line_table[currLine].start_pos <= endPos);
	XtFree(mb_str_loc);
    }

    XtFree(pString);
    return buf;
}

#else

/*
 * New and Improved CopySubstring, now writes data either to "buf"
 * or directly out to a file using the passed-in FILE* if it's non-zero.
 * This routine returns True upon success, False upon any error.
 */
static Boolean
CopySubstring(
        XmTextWidget widget,
        XmTextPosition startPos,
        XmTextPosition endPos,
        char *buf,
        Boolean addNewlines,
	FILE *fp)
{
    register XmTextLineTable line_table = widget->text.line_table;
    int last_line_index, currLine, firstLine;

    XmTextPosition pos;
    XmTextBlockRec block;
    char *ptr = buf;

    if(startPos < 0)
        startPos = 0;
    if(startPos > widget->text.last_position)
        startPos = widget->text.last_position;

    if(endPos < 0)
        endPos = 0;
    if(endPos > widget->text.last_position)
        endPos = widget->text.last_position;

    if(startPos > endPos)
        return False;

    if(addNewlines == False)
    {
      for (pos = startPos; pos < endPos; ){
          pos = (*widget->text.source->ReadSource)
	    (widget->text.source, pos, endPos, &block);
          if (block.length == 0)
             break;
	  if(fp) 
	  {
	      int ret = fwrite(block.ptr, 1, block.length, fp);
	      if(ret != block.length) 
		  return False;
	  }
	  else
	  {
	      (void)memcpy((void*)ptr, (void*)block.ptr, block.length);
	      ptr += block.length;
	  }
      }
      if(!fp) 
	*ptr = (char) '\0';
    }
    else
    {
      XmTextPosition pos, fromPos, toPos;

        firstLine = currLine = _DtEditorGetLineIndex(widget, startPos);
        do
        {
	    if(startPos > (XmTextPosition)line_table[currLine].start_pos) 
	    {
	      fromPos = startPos;
	    }
	    else
	    {
	      fromPos = line_table[currLine].start_pos;
	    }
            if(currLine >= (widget->text.total_lines - 1))
	        toPos = endPos;
            else if((XmTextPosition)line_table[currLine + 1].start_pos <= endPos)
	    {
	        toPos = line_table[currLine+1].start_pos;
	    }
            else
	        toPos = endPos;

            if(currLine > firstLine && line_table[currLine].virt_line != 0) 
	    {
	      if(fp)
	      {
	          char c = '\n';
		  int ret = fwrite(&c, 1, 1, fp);
		  if(ret != 1)
		      return False;
	      }
	      else
	      {
	          *ptr = (char)'\n';
	          ptr++;
	      }
	    }
	    for (pos = fromPos; pos < toPos; )
	    {
	        pos = (*widget->text.source->ReadSource)
		    (widget->text.source, pos, toPos, &block);
	        if (block.length == 0)
		    break;
		if(fp) 
		{
		    int ret = fwrite(block.ptr, 1, block.length, fp);
		    if(ret != block.length) 
		      return False;
		}
		else
		{
		    (void)memcpy((void*)ptr, (void*)block.ptr, block.length);
		    ptr += block.length;
		}
	    }
            currLine++;
        } while(currLine < widget->text.total_lines &&
                (XmTextPosition)line_table[currLine].start_pos <= endPos);
    }
    if(!fp)
        *ptr = (char)'\0';
    return True;
}

#endif



/*************************************************************************
 *
 * _DtEditorCopyDataOut - Writes the entire text editor buffer contents to 
 *               	  the specified character array.
 *
 * Inputs: tw, to supply the data.
 *         buf, specifying the array to which to write the data.
 *
 *************************************************************************/

static char *
_DtEditorCopyDataOut(
        XmTextWidget tw,
        char *buf,
	Boolean addNewlines)
{
    XmTextPosition curCharNum;
    DtEditorWidget editor = M_editor(tw);

#ifdef SUN_CDE_PERF
    CopySubstring(tw, 0, tw->text.last_position, buf, addNewlines, (FILE*)0);
#else
    buf = CopySubstring(tw, 0, tw->text.last_position, buf, addNewlines);
#endif

    return buf;
}

static DtEditorErrorCode
getStringValue(
        DtEditorWidget	editor,
	char 		**buf,
	Boolean 	insertNewlines)
{
    XmTextWidget tw = (XmTextWidget) M_text(editor);
    int 	bufSize;
    char 	*lastChar;

    /*
     * Calculate the size of the buffer we need for the data.
     * 1. Start with MB_CUR_MAX for each char in the text.
     * 3. Add in 1 char for each line, if we have to insert newlines.
     * 4. Add 1 for a terminating NULL.
     */
    bufSize = tw->text.last_position * MB_CUR_MAX;
    if(insertNewlines == True)
        bufSize += tw->text.total_lines;
    bufSize += 1;

    *buf = (char *) XtMalloc(bufSize);
    lastChar = _DtEditorCopyDataOut(tw, *buf, insertNewlines);

    return( DtEDITOR_NO_ERRORS );

} /* end getStringValue */


static DtEditorErrorCode
getDataValue(
        DtEditorWidget	editor,
	void 		**buf,
	unsigned int 	*size,
	Boolean 	insertNewlines)
{
    DtEditorErrorCode error;

    error = getStringValue(editor, (char **)buf, insertNewlines);
    *size = strlen( *buf ); /* remember, strlen doesn't count \0 at end */

    return( error );

} /* end getDataValue */


static DtEditorErrorCode
getWcharValue(
        DtEditorWidget	editor,
	wchar_t         **data,
	Boolean 	insertNewlines)
{
    DtEditorErrorCode error;
    char *mb_value;
    wchar_t *pWchar_value;
    int	num_char, result;

    error = getStringValue(editor, &mb_value, insertNewlines);

    if (error == DtEDITOR_NO_ERRORS)
    {
       /*
	* Allocate space for the wide character string
	*/
       num_char = _DtEditor_CountCharacters(mb_value, strlen(mb_value)) + 1;
       pWchar_value = (wchar_t *)XtMalloc( (unsigned)num_char*sizeof(wchar_t) );

       /*
	* Convert the multi-byte string to wide character
	*/
       result = mbstowcs(pWchar_value, mb_value, num_char*sizeof(wchar_t) );
       if (result < 0)
         pWchar_value[0] = 0L;
       *data = pWchar_value;

       XtFree( mb_value );
    }

    return( error );

} /* end getWcharValue */


/***************************************************************************
 *
 * DtEditorGetContents - gets the contents of the DtEditor widget.
 *
 *      Inputs: widget to retrieve the contents
 *
 *              pointer to a data structure indicating how the retrieved 
 *		data should be formatted.  Depending upon the type of format, 
 *		this structure will contain various fields:
 * 		  string - a NULL pointer (char *) to hold the data
 *		 	      a new container will be created.
 *		  data - void pointer to hold the data, unsigned int for the
 *			 size of the data,
 *		a Boolean indicating whether Newline characters should be
 *              inserted at the end of each line, in string format.
 *		a Boolean indicating whether the the unsaved changes
 *		  flag should be cleared.  There may be times when an 
 *		  application will want to request a copy of the contents 
 *		  without effecting whether DtEditorCheckForUnsavedChanges 
 *		  reports there are unsaved changes.
 *
 *      Returns  0 - contents were retrieved sucessfully
 *		!0 - an error occured while retrieving the contents
 *
 *		The structure passed in will be set according to the
 *		  requested format:
 *		string - a \0-terminated string of characters with
 *			 optional Newlines
 *		container - handle to a Bento container
 *		data - the data, the size of the data
 *
 *		The application is responsible for free'ing any data in the
 * 		above structure.
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorGetContents(
        Widget 			widget,
        DtEditorContentRec 	*data,
	Boolean			hardCarriageReturns,
	Boolean			markContentsAsSaved )
{
   DtEditorErrorCode	error = DtEDITOR_INVALID_TYPE;
   DtEditorWidget editor = (DtEditorWidget) widget;

   switch( data->type ) 
   {
     case DtEDITOR_TEXT:
     {
       error = getStringValue( editor, &(data->value.string), 
			       hardCarriageReturns );
       break;
     }

     case DtEDITOR_DATA:
     {
       error = getDataValue( editor, &(data->value.data.buf),
       			     &(data->value.data.length), 
			     hardCarriageReturns );
       break;
     }

     case DtEDITOR_WCHAR:
     {
       error = getWcharValue( editor, &(data->value.wchar),
			      hardCarriageReturns );
       break;
     }

     default :
     {
       error = DtEDITOR_INVALID_TYPE;
     }
   } /* end switch */

   /*
    * If there were no errors, mark there are now no unsaved changes (unless
    * we were told not to).
    */
   if ( error == DtEDITOR_NO_ERRORS && markContentsAsSaved == True )
     M_unreadChanges( editor ) = False;

   return( error );

} /* end DtEditorGetContents */


/***************************************************************************
 *
 * DtEditorSaveContentsToFile - saves the contents of the DtEditor 
 *				  widget to a disc file as string/data
 *				  or a CDE Document (Bento container).
 *
 *      Inputs: widget to retrieve the contents
 *
 *		filename - name of the file to read 
 *		a Boolean indicating whether the file should be
 *		   overwritten if it currently exists.
 *		a Boolean indicating whether Newline characters should be
 *                inserted at the end of each line (string format only).
 *		a Boolean indicating whether the the unsaved changes
 *		  flag should be cleared.  There may be times when an 
 *		  application will want to request a copy of the contents 
 *		  without effecting whether DtEditorCheckForUnsavedChanges 
 *		  reports there are unsaved changes.
 *		
 *      Returns  DtEDITOR_NO_ERRORS - contents were saved sucessfully
 *		 DtEDITOR_UNWRITABLE_FILE - file is write protected
 *		 DtEDITOR_WRITABLE_FILE - file exists and the 
 *		   overwriteIfExists parameter is False.
 *		 DtEDITOR_SAVE_FAILED - write to the file failed; check 
 *					  disk space, etc.
 *		 OR any errors from DtEditorGetContents 
 *
 ***************************************************************************/

extern DtEditorErrorCode
DtEditorSaveContentsToFile(
        Widget 			widget,
	char			*fileName,
	Boolean			overwriteIfExists,
	Boolean			hardCarriageReturns,
	Boolean			markContentsAsSaved )
{

   struct stat statbuf;		/* Information on a file. */
   FILE *pFile;
   DtEditorContentRec cr;  /* Structure for retrieving contents of widget */
   DtEditorWidget editor = (DtEditorWidget) widget;

   DtEditorErrorCode error = DtEDITOR_INVALID_FILENAME; 

   /*
    * First, make sure we were given a name 
    */

   if (fileName && *fileName ) 
   { 
 
      /*
       * Can we save to the file?
       */
      error = _DtEditorValidateFileAccess( fileName, WRITE_ACCESS );

      if( error == DtEDITOR_NO_ERRORS || 
	  error == DtEDITOR_NONEXISTENT_FILE ||
	  error == DtEDITOR_WRITABLE_FILE )
      {
	/*
	 * Don't overwrite an existing file if we've been told not to
	 */
        if( error == DtEDITOR_WRITABLE_FILE && overwriteIfExists == False )
        {
	   return( error );
        }

	/*
	 * Open the file for writing
	 */
        if ( (pFile = fopen(fileName, "w")) == NULL ) 
          return( DtEDITOR_UNWRITABLE_FILE );
        else
        {

          /* 
           * Save the unsaved changes flag so we can restore it if the write
           * to the file fails.
           */
          Boolean saved_state =  M_unreadChanges( editor );

          /*
           * Now, get the contents of the widget and write it to the file,
           * depending upon the format requested.
           */

#ifdef SUN_CDE_PERF
	  if(True == CopySubstring((XmTextWidget)(M_text(editor)), 0, 
		((XmTextWidget)M_text(editor))->text.last_position, 
				   NULL, hardCarriageReturns, (FILE*)pFile))
	      error = DtEDITOR_NO_ERRORS;
	  else
	      error = DtEDITOR_SAVE_FAILED;
	  /*
	   * If there were no errors, mark there are now no unsaved 
	   * changes (unless we were told not to).
	   */
	  if ( error == DtEDITOR_NO_ERRORS && markContentsAsSaved == True )
	      M_unreadChanges( editor ) = False;

#else        
	  cr.type = DtEDITOR_DATA;
          error = DtEditorGetContents( widget, &cr, hardCarriageReturns,
				       markContentsAsSaved );

	  if ( error == DtEDITOR_NO_ERRORS )
	  {

	    /*
	     * Write it to the file
	     */
	    size_t size_written = fwrite( cr.value.data.buf, 1, 
     				  cr.value.data.length, pFile );

	    if( cr.value.data.length != size_written ) 
	      error = DtEDITOR_SAVE_FAILED;

	    XtFree( cr.value.data.buf );
	  }
#endif
          fclose(pFile);

          if( error == DtEDITOR_SAVE_FAILED )
          {
            /* 
	     * Restore the unsaved changes flag since the save failed
	     */
            M_unreadChanges( editor ) = saved_state;
          }

        } /* end file is writable */

      } /* end filename is valid */

   } 
   return( error );

} /* end DtEditorSaveContentsToFile */


/*
 * _DtEditorGetPointer returns a pointer to the _character_ 
 * numbered by startChar within the string pString. 
 * It accounts for possible multibyte chars.
 */
char *
_DtEditorGetPointer(
	char *pString,
	int startChar)
{
    char *bptr;
    int curChar, char_size;

    if(MB_CUR_MAX > 1)
    {
        for(bptr = pString, curChar = 0;
	    curChar < startChar && *bptr != (char)'\0';
            curChar++, bptr += char_size)
        {
	    if ( (char_size = mblen(bptr, MB_CUR_MAX)) < 0)
	        char_size = 1;
        }
    }
    else
    {
	bptr = pString + startChar;
    }
    return bptr;
} /* end _DtEditorGetPointer */

