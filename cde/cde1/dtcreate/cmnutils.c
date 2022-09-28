/* $XConsortium: cmnutils.c /main/cde1_maint/1 1995/07/17 20:29:59 drk $ */
/***************************************************************************/
/*                                                                         */
/*  Utility Functions                                                      */
/*                                                                         */
/***************************************************************************/

#include <stdio.h>
#include <stdarg.h>
#include <Xm/Xm.h>
#include <Xm/RowColumnP.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include "cmnutils.h"


/********************************************************************************/
/* countItems - counts the number of items in a null terminated array           */
/* INPUT: char **items - null terminated array                                  */
/* OUTPUT: int lcv - number of items in array                                   */
/********************************************************************************/
#ifdef _NO_PROTO
int countItems (items)
     char **items;
#else /* _NO_PROTO */
int countItems (char **items)
#endif /* _NO_PROTO */
{
  int lcv = 0;
  /*
  while (items[lcv]) {
     lcv++;
  }
  */
  if (items) {
     for (lcv = 0; items[lcv]; lcv++);
  }
  return (lcv);
}

/********************************************************************************/
/* TextStringsToXmStrings - Given an array of C text strings returns an         */
/*                          array of XmStrings.                                 */
/* INPUT:  char **text_strings - array of C style strings                       */
/* OUTPUT: XmStringTable  xmstrings - an array Motif compound strings           */
/********************************************************************************/
#ifdef _NO_PROTO
XmStringTable TextStringsToXmStrings (text_strings)
     char **text_strings;
#else /* _NO_PROTO */
XmStringTable TextStringsToXmStrings (char **text_strings)
#endif /* _NO_PROTO */
{
XmStringTable xmstrings = NULL;
int count, lcv;

if (text_strings) {
    count = countItems (text_strings);
    xmstrings = (XmStringTable) calloc (sizeof(XmString), (count));
    for (lcv = 0; lcv < count; lcv++)
        xmstrings[lcv] = (XmString) XmStringCreateSimple (text_strings[lcv]);
}
return ((XmStringTable)xmstrings);
}

/********************************************************************************/
/* XmStringToText - Given an XmString returns a C character text string.        */
/* INPUT:  XmString  xmstring - a Motif compound string                         */
/*  OUTPUT: char *text_string - C style string                                  */
/********************************************************************************/
#ifdef _NO_PROTO
char *XmStringToText (xmstring)
     XmString xmstring;
#else /* _NO_PROTO */
char *XmStringToText (XmString xmstring)
#endif /* _NO_PROTO */
{
XmStringContext   context;
XmStringCharSet   charset;
XmStringDirection direction;
Boolean           separator;
char              *text_string = NULL, *temp = NULL;

text_string = (char *)calloc (1, sizeof (char));

if (xmstring) {
    if (!XmStringInitContext (&context, xmstring)) {
      printf("Can't convert compound string.\n");
      return (NULL);
    }
    while  (XmStringGetNextSegment (context, &temp, &charset,
                                    &direction, &separator)) {
      text_string = (char *)realloc (text_string, strlen (temp)+1);
      if (text_string == NULL) {
        printf("Can't allocate space for file name.\n");
        return (NULL);
      }
      text_string = strcpy(text_string, temp);
    }

    XmStringFreeContext(context);
    }

return (text_string);
}

/********************************************************************************/
/* delete_all_list_items - removes all items from a list box                    */
/* INPUT: Widget list - id of list widget                                       */
/* OUTPUT: none                                                                 */
/********************************************************************************/
#ifdef _NO_PROTO
void delete_all_list_items (list)
     Widget list;
#else /* _NO_PROTO */
void delete_all_list_items (Widget list)
#endif /* _NO_PROTO */
{
int item_count = 0;

XtVaGetValues (list, XmNitemCount, &item_count, NULL);
if (item_count > 0) {
  XmListDeleteItemsPos (list, item_count, 1);
}
return;
}

/********************************************************************************/
/* clear_text_field - removes any text from a text field                        */
/* INPUT: Widget textfield - id of text widget                                  */
/* OUTPUT: none                                                                 */
/********************************************************************************/
#ifdef _NO_PROTO
void clear_text_field (textfield)
     Widget textfield;
#else /* _NO_PROTO */
void clear_text_field (Widget textfield)
#endif /* _NO_PROTO */
{
XmTextPosition last_pos;
char *empty = "";

last_pos = XmTextGetLastPosition (textfield);
XmTextReplace (textfield, 0, last_pos, empty);

return;
}

/********************************************************************************/
/* clear_text - removes any text from a text widget                             */
/* INPUT: Widget textwid - id of text widget                                    */
/* OUTPUT: none                                                                 */
/********************************************************************************/
#ifdef _NO_PROTO
void clear_text (textwid)
     Widget textwid;
#else /* _NO_PROTO */
void clear_text (Widget textwid)
#endif /* _NO_PROTO */
{
XmTextPosition last_pos;
char *empty = "";

last_pos = XmTextGetLastPosition (textwid);
XmTextReplace (textwid, 0, last_pos, empty);

return;
}
