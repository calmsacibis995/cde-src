/* $XConsortium: SearchCalls.c /main/cde1_maint/3 1995/10/05 09:22:14 lehors $ */
/**********************************<+>*************************************
***************************************************************************
**
**  File:        SearchCalls.c
**
**  Project:     DtEditor widget for editing services
**
**  Description: Spell and Find functions
**  -----------
**
*******************************************************************
*
* (c) Copyright 1993, 1994 Hewlett-Packard Company
* (c) Copyright 1993, 1994 International Business Machines Corp.
* (c) Copyright 1993, 1994 Sun Microsystems, Inc.
* (c) Copyright 1993, 1994 Unix System Labs, Inc., a subsidiary of Novell, Inc.
*
********************************************************************/


#include "EditorP.h"
#include <Xm/TextF.h>
#include <ctype.h>
#include <Dt/DtMsgsP.h>

static Boolean DoReplace(
	DtEditorWidget pPriv,
	char *replace_string,
	Time time);
static int SearchForString(
        DtEditorWidget pPriv,
	XmTextPosition startLocation,
	char *searchString);
static Boolean DoSearch(
	DtEditorWidget pPriv,
	char *search_string,
	Time time);

DtEditorErrorCode
DtEditorInvokeSpellDialog(
	Widget widget)
{
    DtEditorWidget pPriv = (DtEditorWidget) widget;
    char fileName[L_tmpnam], com[L_tmpnam + 7], *string, 
	 newline[1];
    char *line;
    FILE *fp;           /* pipe to read words from */
    int len = 0;        /* length of line read in */
    int maxLen = 0;     /* max length of the line buffer */
    XmString word;      /* processed word ready to add to list */
    DtEditorErrorCode error = DtEDITOR_NO_TMP_FILE;
    newline[0]='\n';

    _DtTurnOnHourGlass(M_topLevelShell(pPriv));

    /* 
     * Write out to a tmp file, getting the name back
     */
    (void)tmpnam(fileName);
    if((fp = fopen(fileName, "w")) != (FILE *)NULL) 
    {
       /* 
	* Temporary file created sucessfully so write out contents of
	* widget in preparation of feeding it to the 'spell' filter.
	*/
       string = (char *)XmTextGetString(M_text(pPriv));
       fwrite(string, sizeof(char), strlen(string), fp);
       XtFree(string);
       /* 
	* Tack on a final newline (\n) cuz spell(1) does not spell-check 
	* lines which do not terminate with a newline.
	*/
       fwrite(newline, sizeof(char), 1, fp);

       fclose(fp);

       /* start spell command */
       sprintf(com, "%s %s", M_spellFilter(pPriv), fileName);
       fp = popen(com, "r");

       if ( fp == (FILE *)NULL )
         error = DtEDITOR_SPELL_FILTER_FAILED;
       else {
         error = DtEDITOR_NO_ERRORS;

	 /* 
	  * The filter was started successfully. 
	  * Initialize the Spell dialog and get ready to receive 
	  * the list of mispelled words.
	  */
         _DtEditorSearch(pPriv, True, True);
         _DtTurnOnHourGlass(M_search_dialog(pPriv));
 	 /* needed for bug in list */
         XmListSetPos(M_search_spellList(pPriv), 1); 
         XmListDeleteAllItems(M_search_spellList(pPriv));

	 /*
	  * malloc the buffer
	  */
	 maxLen = 50;
	 line = (char *) XtMalloc (sizeof(char) * (maxLen + 1));
	 len  = 0;

         /* 
	  * Now, get each word and hand it to the list 
	  */
         while(fgets(&line[len], maxLen - len, fp)) 
         {
            len += strlen(&line[len]);
            if (len > 0)
	    {
               if (line[len - 1] == '\n')
 	       {
                  line[len - 1] = '\0';
                  word = XmStringCreateLtoR(line, XmFONTLIST_DEFAULT_TAG);
                  XmListAddItemUnselected(M_search_spellList(pPriv), word, 0);
                  XmStringFree(word);
		  len = 0;
               }
	       else
	       {
		  maxLen += 50;
		  line = (char *) XtRealloc (line, sizeof(char) * (maxLen + 1));
	       }
	    }
         }

         /* clean up and display the results */
	 XtFree(line);
         pclose(fp);
         _DtEditorSearch(pPriv, True, False);
         _DtTurnOffHourGlass(M_search_dialog(pPriv));

      } /* end start the spell filter */
      unlink(fileName);

    } /* end create temporary file */

    _DtTurnOffHourGlass(M_topLevelShell(pPriv));
    return( error );

} /* end DtEditorInvokeSpellDialog */


/* ARGSUSED */
Boolean
DtEditorFind(
	Widget	widget,
	char	*find)
{
  DtEditorWidget editor = (DtEditorWidget) widget;
  Boolean foundIt = True;

  /*
   * If we were passed a string to find, then use it.  Otherwise,
   * use the last find string entered in the Find/Change dialog.
   */
  if ( find != (char *)NULL )
     foundIt = DoSearch(editor, find, CurrentTime);
  else
  {
     /*
      * If there is no value from the dialog, then post the Find/Change 
      * dialog to get one.
      */
     if (!M_search_string(editor)) 
     {
        _DtEditorSearch(editor, False, False);
        return( foundIt );
     }
     else 
        foundIt = DoSearch(editor, M_search_string(editor), CurrentTime);
  }

  return( foundIt );

} /* DtEditorFind */

/* Count the number of characters represented in the char* str.  
 * By definition, if MB_CUR_MAX == 1 then numBytes == number of characters.
 * Otherwise, use mblen to calculate. 
 */
int
_DtEditor_CountCharacters(
        char *str,
        int numBytes)
{
	char *bptr;
	int count = 0;
	int char_size = 0;
	int mbCurMax = MB_CUR_MAX; /* invoke the macro just once */

	if (mbCurMax <= 1)
	    return (numBytes < 0)? 0 : numBytes;
	if (numBytes <=0 || str == NULL || *str == '\0')
		return 0;

	for(bptr = str; numBytes > 0; count++, bptr+= char_size)
	{
	   char_size = mblen(bptr, mbCurMax);
	   if (char_size < 0)
		break; /* error */
	   numBytes -= char_size;
	}
	return count;
}

/*
 * SearchForString takes an Editor widget, a position at which
 * to begin its search, and the string for which it is to search.
 * It searches first from startLocation to the end of the file, and
 * then if necessary from the start of the file to startLocation.
 * It returns an integer indicating the location of the string, or
 * -1 if the string is not found.
 */
static int
SearchForString(
        DtEditorWidget pPriv,
	XmTextPosition startLocation,
	char *searchString)
{
    XmTextPosition pos, searchAnchor, topAnchor, cursorLocation,
    		   lastPosition = XmTextGetLastPosition(M_text(pPriv));

    topAnchor = 0;
    searchAnchor = cursorLocation = startLocation; 

    while((Boolean)_XmStringSourceFindString(M_text(pPriv), searchAnchor,
					     searchString,
					     &pos) ||
           ((Boolean)_XmStringSourceFindString(M_text(pPriv),
                                    topAnchor, searchString, &pos) && 
	    pos < cursorLocation)) 
    {
        char *word, leadingChar, trailingChar;
        XmTextPosition endPos;
	int length;
	/*
	 * Do some extra work for the Spell case, so we find only "words"
	 * and not strings.
	 * Get the word with the leading & trailing characters.
	 * It's a word if it's bounded by:
	 * a. 16-bit characters
	 * b. Spaces
	 * c. Punctuation
	 */

	/*
	 * Variables:
	 *
	 *      0 (constant) - first character position in the document.
	 *      lastPosition - last character position in the document.
	 *
	 * 	pos    - Position in the document of the first character
	 *		 of the word.  Does not include leading character.
	 * 	endPos - Position in the document of the last character
	 *		 of the word + 1. Points to the trailing character,
	 *		 if any.
	 *
	 *	word   - The string we have matched. Includes the leading
	 *		 & trailing characters, if any.
	 *	word[0] - Leading character, if any, otherwise first
	 *		 character of the matched string.
	 *	word[length] - Trailing character, if any, otherwise last
	 *		 character of the matched string.
	 *
	 */

	endPos = pos + 1 + _DtEditor_CountCharacters(searchString, 
					     strlen(searchString));
	if(pos < cursorLocation)
	    topAnchor = pos + 1;
	else
	    searchAnchor = pos + 1;

	/*
	 * If the first character of the word is the first character in
	 * the document there is no leading character.  Likewise, if the
	 * last character of the word is the last character in the document
	 * there is no trailing character.
	 */
	if( pos > 0 ) {
	  /* 
	   * There is a leading character.
	   */

	  if (endPos <= lastPosition) {
	     /* 
	      * There is a trailing character.
	      */
             word = (char *)_XmStringSourceGetString( 
					(XmTextWidget) M_text(pPriv), 
					pos - 1, endPos, False);
  	     length = strlen(word) - 1;
	     trailingChar = word[length];

	  }
	  else { 
	     /* 
	      * There is no trailing character.
	      */
             word = (char *)_XmStringSourceGetString( 
					(XmTextWidget) M_text(pPriv), 
					pos - 1, lastPosition, False);
  	     length = strlen(word);
	     trailingChar = ' ';
	  }
	  leadingChar = word[0];

	}
	else {
	  /* 
	   * There is no leading character.
	   */

	  if (endPos <= lastPosition) {
	     /* 
	      * There is a trailing character.
	      */
             word = (char *)_XmStringSourceGetString(
		       			(XmTextWidget) M_text(pPriv), 
					0, endPos, False);
  	     length = strlen(word) - 1;
	     trailingChar = word[length];
	  }
	  else { 
	     /* 
	      * There is no trailing character.
	      */
             word = (char *)_XmStringSourceGetString(
		       			(XmTextWidget) M_text(pPriv), 
					0, lastPosition, False);
  	     length = strlen(word);
	     trailingChar = ' ';
	  }
	  leadingChar = ' ';
	}

	if ((M_search_dialogMode(pPriv) != SPELL) ||
	    (
	      ((mblen(word, MB_CUR_MAX) > 1) || 
	       ( isascii(leadingChar) && 
	         (isspace(leadingChar) || ispunct(leadingChar))
	       )
	      ) &&
	      ((mblen(word+length-1, MB_CUR_MAX) > 1) ||
	       ( isascii(trailingChar) && 
		 (isspace(trailingChar) || ispunct(trailingChar))
	       )
	      )
	    )
	   )
	{
	    /*
	     * Either we are not in Spell mode or we have a word
	     * so return
	     */
	    if (word  != (char *)NULL)
	      XtFree(word);

	    return (int)pos;
	}

	if (word  != (char *)NULL)
	  XtFree(word);
    }

    return -1;
}

static Boolean
DoSearch(
	DtEditorWidget widget,
	char *search_string,
	Time time)
{
    int stringPosition;
    Boolean foundIt = False;

    stringPosition = SearchForString(widget, 
				     XmTextGetCursorPosition(M_text(widget)), 
		                     search_string);

    if(stringPosition == -1) {
	/* 
	 * If the string was not found unselect everything
	 */
        XmTextClearSelection(M_text(widget), time);
    } 
    else {
	/*
	 * The string was found so highlight the word and scroll the window 
	 * if it is not visible.
	 */
	XmTextPosition pos = (XmTextPosition) stringPosition;
	XmTextWidget tw = (XmTextWidget)M_text(widget);
	XmTextPosition top_pos = tw->text.top_character,
	               bot_pos = tw->text.bottom_position;
	
	foundIt = True;
        XmTextSetSelection( M_text(widget), pos, 
          pos+_DtEditor_CountCharacters(search_string, strlen(search_string)),
          XtLastTimestampProcessed(XtDisplay(M_text(widget))) );

#define TOP_PAD 2 /* number of lines to leave between top of the
		     working area and the found selection */

	if (pos < top_pos || pos >= bot_pos)
	{
           Arg al[1];
	   short rows;
	   
           XtSetArg(al[0], XmNrows, &rows);
           XtGetValues(M_text(widget), al, 1);

	   /* scroll found selection to the top of the working area,
	      leavingn a few lines of text above it if possible */
	   if (pos >= bot_pos)
	       XmTextScroll(M_text(widget),
			(rows>2*TOP_PAD ? rows-TOP_PAD : rows)-1);
	   else
	       XmTextScroll(M_text(widget),
			-(rows>2*TOP_PAD ? TOP_PAD+1 : 0));
	}
    }


    return ( foundIt );

} /* end DoSearch */


/*
 * DtEditorInvokeFindChangeDialog posts the "Find/Change" dialog.
 */
void
DtEditorInvokeFindChangeDialog(
	Widget	widget)
{
     DtEditorWidget pPriv = (DtEditorWidget) widget;
     _DtEditorSearch(pPriv, False, False);
}

/*
 * DoReplace checks that there is a non-null selection, and
 * if so, replaces the selection with the replace_string argument.
 */
static Boolean
DoReplace(
	DtEditorWidget pPriv,
	char *replace_string,
	Time time)
{
     XmTextPosition first, last;
     Boolean replaced = False;

    /*
     * Only do a replace if we have a non-null selection.
     * We could check that the selection == the Find string, but 
     * this allows a little more flexibility for the user.
     */
    if (XmTextGetSelectionPosition(M_text(pPriv), &first, &last) &&
	first != last) 
    {
       XmTextReplace(M_text(pPriv), first, last, replace_string);
       XmTextSetSelection(M_text(pPriv), first, 
	 first + 
	 _DtEditor_CountCharacters(replace_string, strlen(replace_string)),
         time);
       replaced = True;
    }

    return( replaced );
}

/* 
 * ReplaceAll replaces all occurrences of search_string with 
 * replacement_string in widget.
 */

static Boolean
ReplaceAll(
        DtEditorWidget widget,
        char 	*search_string, 
	char 	*replace_string ) 
{
    int replacementLength, searchLength, lastOccurrence, thisOccurrence;
    Boolean replaceOK = False;

    /* 
     * Make sure there is a string to find.  Null replacement strings
     * are OK.
     */
    if( search_string && *search_string )
    {
      /*
       * How long is the string we are searching for?
       */
      searchLength = _DtEditor_CountCharacters( search_string, 
					        strlen(search_string) );

      /*
       * How long is the replacement string?
       */
      replacementLength = _DtEditor_CountCharacters( replace_string, 
					 	     strlen(replace_string) );

      /*
       * Start at the beginning and search for the string
       */
      lastOccurrence = -1; 
      while( ((thisOccurrence = SearchForString(widget, 
	        (lastOccurrence > 0)? (XmTextPosition)lastOccurrence : 0,
	        search_string)
	      ) != -1 ) && 
	     thisOccurrence >= lastOccurrence )
      {
        XmTextReplace( M_text(widget), (XmTextPosition)thisOccurrence, 
	               (XmTextPosition) (thisOccurrence + searchLength), 
		       replace_string );
        lastOccurrence = thisOccurrence + replacementLength;
      } /* end while */

      if (lastOccurrence != -1)
	replaceOK = True;

    }

   return( replaceOK );

} /* end ReplaceAll */

/*
 * DtEditorChange replaces either the current selection, the next occurrence 
 * of a string, or all occurrences of the string with a replacement string.
 * If no find or change to strings are passed in, DtEditorFindChange uses 
 * the last find and change to strings from the Find/Change dialog.
 */
Boolean
DtEditorChange(
	Widget			widget,
	DtEditorChangeValues    *findChangeStrings,
	unsigned int		instanceToChange)
{

      Boolean returnVal = False;
      DtEditorWidget editor = (DtEditorWidget) widget;

      switch( instanceToChange )
      {
        case DtEDITOR_NEXT_OCCURRENCE:
        {
	   /*
	    * Find the next occurrence and replace it (by treating it as 
	    * a current selection).
	    */

	   /* 
	    * If we were passed a Find string use it.  Otherwise, tell
	    * DtEditorFind to use the last search string value 
	    * (M_search_string).
	    */
	   if ( findChangeStrings != (DtEditorChangeValues *) NULL )
	     returnVal = DtEditorFind( widget, findChangeStrings->find );
	   else
	     returnVal = DtEditorFind( widget, (char *)NULL );
	   
	   if ( returnVal == False)
	     break;
        }

        case DtEDITOR_CURRENT_SELECTION:
        {
	   /*
	    * Replace whatever is selected.
	    */

	   /* 
	    * If we were passed a Change To string use it.  Otherwise, 
	    * use the last replace string value.
	    */
	   if ( findChangeStrings != (DtEditorChangeValues *) NULL )
	     returnVal = DoReplace( editor, findChangeStrings->changeTo, 
				    CurrentTime );
	   else
	     returnVal= DoReplace(editor,M_replace_string(editor),CurrentTime);

           break;
        }

        case DtEDITOR_ALL_OCCURRENCES:
        {
    	   _DtTurnOnHourGlass( M_topLevelShell(editor) );


	   if ( findChangeStrings != (DtEditorChangeValues *) NULL )
             returnVal = ReplaceAll( editor, findChangeStrings->find, 
				     findChangeStrings->changeTo );
	   else
             returnVal = ReplaceAll( editor, M_search_string(editor), 
		 	             M_replace_string(editor) );

    	   _DtTurnOffHourGlass( M_topLevelShell( editor ) );

           break;
        } /* replace all occurrences */

	default :
	{
	}

      } /* end switch */

      return( returnVal );

} /* end DtEditorChange */

/* ARGSUSED */
void
_DtEditorSearchMapCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    int         ac;
    Arg         al[4];
    Widget      parent;
    Position newX, newY, pY, pX;
    Dimension pHeight, myHeight, pWidth, myWidth;
    DtEditorWidget pPriv = (DtEditorWidget) client_data;

    parent = M_topLevelShell(pPriv);

    pX = XtX(parent);
    pY = XtY(parent);
    pHeight = XtHeight(parent);
    pWidth = XtWidth(parent);
    myHeight = XtHeight(w);
    myWidth = XtWidth(w);

    if ((newY = pY - (int)myHeight + 5) < 0)
        newY = pY + pHeight;
    newX = pX + pWidth/2 - ((int)myWidth)/2;

    ac = 0;
    XtSetArg(al[ac], XmNx, newX); ac++;
    XtSetArg(al[ac], XmNy, newY); ac++;
    XtSetValues(w,al,ac);

}

/* 
 * 
 * _DtEditorDialogSearchCB is called whenever the Find button is pressed
 * in the Find/Change dialog. If the dialog is displayed in Find/Change
 * mode, it updates the contents of M_search_string() with the contents 
 * of the "Find" text field, and then invokes DtEditorFind().  If the 
 * find is successful, the Change button is enabled, otherwise the
 * "String not found" dialog is displayed.  
 * 
 * When the dialog is in Spell mode, the selected misspelled word is merely
 * passed to DtEditorFind().  The Change (and Change All) buttons are left 
 * insentive until the user types something into the Change To field 
 * (too many users were replacing misspelled words with blanks).
 * 
 */

/* ARGSUSED */
void
_DtEditorDialogSearchCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{

    DtEditorWidget pPriv = (DtEditorWidget) client_data;

    /*
     * Is the dialog in Find/Change or Spell mode?
     */
    if (M_search_dialogMode(pPriv) == REPLACE) {
       /*
        * Find/Change mode
        * Free the existing search string and get the new one.
        */
       if (M_search_string(pPriv)) 
           XtFree(M_search_string(pPriv));
       M_search_string(pPriv) = XmTextFieldGetString( M_findText(pPriv) );

       /*
        * Find the string
        */
       if( DtEditorFind((Widget)pPriv, M_search_string(pPriv)) ) {
         /* 
          * If the string was found then enable the Change button.
          * It will be disabled when it is pressed or a new Find string is 
	  * entered.
          */
         _DtEditorSetReplaceSensitivity( pPriv, True );
       }
       else {
	 /* 
	  * Post a dialog informing the user the string was not found.
	  */

         char *tempStr = (char *)XtMalloc(strlen(NO_FIND) +
                          	    strlen(M_search_string(pPriv)) + 1);
         sprintf(tempStr, NO_FIND, M_search_string(pPriv));
         _DtEditorWarning(pPriv, tempStr, XmDIALOG_INFORMATION);
         XtFree(tempStr);
       }

    }
    else {
       /* 
        * Spell mode.
        */
       char *pString;

       M_misspelled_found(pPriv) = DtEditorFind((Widget)pPriv, 
						M_misspelled_string(pPriv) );
       /* 
        * If the word was found & there is a Change To string then enable 
	* the Change button.  If there is no Change To string, Change will 
	* be enabled when a string is entered if M_misspelled_found is True
	* (see _DtEditorReplaceTextChangedCB) 
	*
	* Change All is enabled in _DtEditorReplaceTextChangedCB() 
	* (ie. anytime there is a Change To string).  It is not
	* dependent upon a sucessful Find because it initiates its own
	* find.
        */
       if ( M_misspelled_found(pPriv) == True ) {

          /*
           * Is there a Change To string?
           */
          pString = XmTextFieldGetString(M_replaceText(pPriv));

          if( pString != (char *)NULL && *pString != (char)'\0' ) 
             _DtEditorSetReplaceSensitivity( pPriv, True );

          if(pString != (char *)NULL)
   	     XtFree(pString);
       }
       else {
	 /* 
	  * Post a dialog informing the user the string was not found.
	  */

         char *tempStr = (char *)XtMalloc(strlen(NO_FIND) +
                          	    strlen(M_misspelled_string(pPriv)) + 1);
         sprintf(tempStr, NO_FIND, M_misspelled_string(pPriv));
         _DtEditorWarning(pPriv, tempStr, XmDIALOG_INFORMATION);
         XtFree(tempStr);
       }

    }

} /* end _DtEditorDialogSearchCB */

/*
 * _DtEditorDialogReplaceCB is called whenever the Change button is pressed
 * in the Find/Change dialog. If the dialog is displayed in Find/Change
 * mode, it updates the contents of M_replace_string() with the contents 
 * of the "Change To" text field, and then invokes DtEditorChange(). 
 * 
 * When the dialog is in Spell mode, the contents of the "Change To" text 
 * field is passed to DtEditorChange() without updating M_replace_string().
 * 
 * In both cases, the Change button is disabled after the change is
 * complete.  
 */

/* ARGSUSED */
void
_DtEditorDialogReplaceCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Arg al[10];                 /* arg list */
    DtEditorWidget pPriv = (DtEditorWidget) client_data;

    /*
     * Is the dialog in Find/Change or Spell mode?
     */
    if (M_search_dialogMode(pPriv) == REPLACE) {
       /*
        * Find/Change mode
        * Free the existing Change To string and get the new one.
        */
       if (M_replace_string(pPriv)) 
           XtFree(M_replace_string(pPriv));
       M_replace_string(pPriv) = XmTextFieldGetString(M_replaceText(pPriv));

       DtEditorChange( (Widget)pPriv, (DtEditorChangeValues *)NULL, 
		       DtEDITOR_CURRENT_SELECTION );
    }
    else {
       /* 
        * Spell mode.
        */
       DtEditorChangeValues newWord;

       newWord.changeTo = XmTextFieldGetString(M_replaceText(pPriv));
       if (newWord.changeTo != (char *)NULL) {
         /* This field ignored when changing the current selection */
         newWord.find = (char *)NULL; 
         DtEditorChange( (Widget)pPriv, &newWord, DtEDITOR_CURRENT_SELECTION );

         XtFree(newWord.changeTo);
       }
    }

    /*
     * Disable the Change button.  In Find/Change mode, it will be enabled 
     * when the Find button is pressed and the Find text is successfully 
     * found.  In Spell mode, there must also be a value in the Change To 
     * field.
     */
    _DtEditorSetReplaceSensitivity(pPriv, False );

    /*
     * Want the traversal to be on the Find button, so that the user
     * can initiate another search.
     */
    XmProcessTraversal(M_search_findBtn(pPriv), XmTRAVERSE_CURRENT);
}

/*
 * _DtEditorDialogReplaceAllCB is attached to the "Change All" button
 * in the Find/Change dialog.  It replaces all occurances of the "Find"
 * string with the "Change To" string.
 */

/* ARGSUSED */
void
_DtEditorDialogReplaceAllCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget pPriv = (DtEditorWidget) client_data;

    /*
     * Is the dialog in Find/Change or Spell mode?
     */
    if (M_search_dialogMode(pPriv) == REPLACE) {
       /*
        * Find/Change mode
        * Free any existing search string before getting the current one.
        */
       if (M_search_string(pPriv)) 
           XtFree(M_search_string(pPriv));
       M_search_string(pPriv) = XmTextFieldGetString(M_findText(pPriv));

       /*
        * Free the existing Change To string and get the new one.
        */
       if (M_replace_string(pPriv)) 
           XtFree(M_replace_string(pPriv));
       M_replace_string(pPriv) = XmTextFieldGetString(M_replaceText(pPriv));

       /* 
        * Make the change with the current values (set above).
        */
       if( !DtEditorChange((Widget)pPriv, (DtEditorChangeValues *)NULL, 
		       DtEDITOR_ALL_OCCURRENCES) ) {
	 /* 
	  * If the replace failed, post a dialog informing the user 
	  * the string was not found.
	  */

         char *tempStr = (char *)XtMalloc(strlen(NO_FIND) +
                          	    strlen(M_search_string(pPriv)) + 1);
         sprintf(tempStr, NO_FIND, M_search_string(pPriv));
         _DtEditorWarning(pPriv, tempStr, XmDIALOG_INFORMATION);
         XtFree(tempStr);
       }

    }
    else {
       /* 
        * Spell mode.
        */
       DtEditorChangeValues changeValues;

       changeValues.find = M_misspelled_string(pPriv);
       changeValues.changeTo = XmTextFieldGetString(M_replaceText(pPriv));
       DtEditorChange((Widget)pPriv, &changeValues, DtEDITOR_ALL_OCCURRENCES);

       if( changeValues.changeTo != (char *)NULL )
	  XtFree( changeValues.changeTo );

    }

    /*
     * Disable the Change button.  It will be enabled when the Find
     * button is pressed and the Find text is successfully found.
     * In Spell mode, there must also be a value in the Change To field.
     */
    _DtEditorSetReplaceSensitivity(pPriv, False );

} /* _DtEditorDialogReplaceAllCB */

/* ARGSUSED */
void
_DtEditorDialogFindCancelCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    DtEditorWidget pPriv = (DtEditorWidget)client_data;
    XtUnmanageChild(M_search_dialog(pPriv));
}

/*
 * _DtEditorMisspelledSelectCB is called when a new word has been selected 
 * from the list of misspelled words.
 */
/* ARGSUSED */
void
_DtEditorMisspelledSelectCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    XmListCallbackStruct *cb = (XmListCallbackStruct *)call_data;
    DtEditorWidget editor = (DtEditorWidget)client_data;

    /* 
     * Get the selected word for use when the Find or Replace All button
     * is pressed.
     */
    if (M_misspelled_string(editor)) 
      XtFree(M_misspelled_string(editor));

    XmStringGetLtoR(cb->item, XmFONTLIST_DEFAULT_TAG, 
		    &M_misspelled_string(editor));

    /*
     * Mark that it has not been found
     */

    M_misspelled_found(editor) = False;

    /*
     * Enable the Find button
     */
    _DtEditorSetFindSensitivity(editor, True );

    /*
     * Clear the "Change To" text field.
     */
    XmTextFieldSetString(M_replaceText(editor), (char *)NULL);

} /* end _DtEditorMisspelledSelectCB */

/*
 * _DtEditorMisspelledDblClickCB is called when a word has been double-
 * clicked from the list of misspelled words.  First, the word will become 
 * the new misspelled string and, then, a find will be initiated for it.
 */
/* ARGSUSED */
void
_DtEditorMisspelledDblClickCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    XmListCallbackStruct *cb = (XmListCallbackStruct *)call_data;

    _DtEditorMisspelledSelectCB(w, client_data, call_data );
    _DtEditorDialogSearchCB(w, client_data, call_data );
} /* end _DtEditorMisspelledDblClickCB */


/* 
 * The following functions effectively track whether the user has
 * entered or changed the "Find" text. This information is used to make 
 * the "Find" and "Change All" buttons sensitive/desensitive.  The
 * "Find" button must be pressed and the Find text found before the
 * "Change" button is sensitive.  These functions also set the default 
 * button to either the "Find" or "Close" button. 
 */
 
void
_DtEditorSetFindSensitivity(
        DtEditorWidget widget,
	Boolean sensitivity)
{
        XtSetSensitive(M_search_findBtn(widget), sensitivity);
}

void
_DtEditorSetReplaceSensitivity(
        DtEditorWidget editor,
	Boolean sensitivity)
{
  /*
   * Cannot enable Change button if widget is read only
   */
  if ( M_editable(editor) || !sensitivity )
    XtSetSensitive(M_search_replaceBtn(editor), sensitivity);
}

void
_DtEditorSetReplaceAllSensitivity(
        DtEditorWidget editor,
	Boolean sensitivity)
{
  /*
   * Cannot enable Change All button if widget is read only
   */
  if ( M_editable(editor) || !sensitivity )
        XtSetSensitive(M_search_replaceAllBtn(editor), sensitivity);
}

/* ARGSUSED */
void
_DtEditorFindTextChangedCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    Arg al[10];                 /* arg list */
    Widget defaultButton;
    char *pString;

    DtEditorWidget editor = (DtEditorWidget)client_data;

    /*
     * Is there a Find string?
     */
    pString = XmTextFieldGetString(M_findText(editor));

    /*
     * Only enable the Find & Change All buttons if there is a 
     * string to search for (i.e. a Find string).
     */
    if(pString == (char *)NULL || *pString == (char)'\0') {
	_DtEditorSetFindSensitivity(editor, False );
	_DtEditorSetReplaceAllSensitivity(editor, False );
        /*
         * Make the Close button the default
         */
        defaultButton = M_search_closeBtn(editor);
    }
    else {
	_DtEditorSetFindSensitivity( editor, True );
	_DtEditorSetReplaceAllSensitivity(editor, True );
        /*
         * Make the Find button the default
         */
        defaultButton = M_search_findBtn(editor);
    }
    if(pString != (char *)NULL)
	XtFree(pString);

    /*
     * Set the default button 
     */
    XtSetArg(al[0], XmNdefaultButton, defaultButton); 
    XtSetValues(M_search_dialog(editor), al, 1);

    /*
     * Disable the Change button.  It will be enabled when the Find
     * button is pressed and the Find text is successfully found.
     */
    _DtEditorSetReplaceSensitivity(editor, False );

} /* end _DtEditorFindTextChangedCB */

/* 
 * The following functions effectively track whether the user has
 * entered or changed the Change To text.  This information is used 
 * in the Spell dialog to make the Change and Change All buttons 
 * sensitive/desensitive so users cannot replace a misspelled word with
 * a null string.
 */

/* ARGSUSED */
void
_DtEditorReplaceTextChangedCB(
        Widget w,
        caddr_t client_data,
        caddr_t call_data )
{
    char *pString;

    DtEditorWidget editor = (DtEditorWidget)client_data;

    /*
     * Ignore this callback if it is not being called from the Spell
     * dialog.
     */

    if( M_search_dialogMode(editor) == SPELL ) {
       /*
        * Is there a Change To string?
        */
       pString = XmTextFieldGetString(M_replaceText(editor));

       /*
        * Disable the Change & Change All buttons if there is 
        * no Change To string.
        */
       if( pString == (char *)NULL || *pString == (char)'\0' ) {
          _DtEditorSetReplaceSensitivity(editor, False );
          _DtEditorSetReplaceAllSensitivity(editor, False );
       }
       else {
	  /* 
	   * If there is a Change To string enable the Change 
	   * All button, but only enable the Change button if.  
	   * the Find button has been pressed & the misspelled 
	   * word found (see _DtEditorDialogSearchCB()
	   */
          _DtEditorSetReplaceAllSensitivity(editor, True );
	  if ( M_misspelled_found(editor) )
            _DtEditorSetReplaceSensitivity(editor, True );

       if(pString != (char *)NULL)
   	  XtFree(pString);

      }

    }

} /* end _DtEditorReplaceTextChangedCB */
