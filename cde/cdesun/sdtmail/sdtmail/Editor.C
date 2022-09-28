/*
 *+SNOTICE
 *
 *	$Revision: 1.4 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
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
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Editor.C	1.75 06/12/97"
#else
static char *sccs__FILE__ = "@(#)Editor.C	1.75 06/12/97";
#endif

#include <Xm/RowColumn.h>
#include <assert.h>
#include <stdio.h>

#include <Dt/Dts.h>
#include <SDtMail/Session.hh>
#include <SDtMail/Error.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/String.hh>
#include <SDtMail/SimpleTuples.hh>
#include <SDtMail/MessageBody.hh>
#include <SDtMail/ContentBuffer.hh>

#include "RoamApp.h"
#include "MailMsg.h"
#include "Editor.hh"

Editor::Editor()
  : UIComponent("Editor")
{
}

Editor::~Editor() 
{
}


AbstractEditorParent::AbstractEditorParent() {
  _attachmentPopupMenu = NULL;
  _textPopupMenu = NULL;
  _menuPopupAtt = NULL;
  _menuPopupText = NULL;
}

AbstractEditorParent::~AbstractEditorParent() 
{
  delete _menuPopupAtt;
  delete _menuPopupText;
}   

SdmBoolean
Editor::set_message(SdmError & error,
		    SdmMessage * msg,
		    char ** status_string,
		    HeaderFormat header_format,
		    InsertFormat format,
		    BracketFormat brackets)
{
  SdmMailRc *mail_rc;
  SdmMessageEnvelope *env;
  SdmConnection *con = theRoamApp.connection()->connection();
  assert (con != NULL);
  SdmBoolean firstBPHandled = Sdm_False;
  
  // initialize error.
  error.Reset();
    
  if ( (con->SdmMailRcFactory(error, mail_rc) != Sdm_EC_Success) 
       || (msg->SdmMessageEnvelopeFactory(error, env) != Sdm_EC_Success) ) {
    return (firstBPHandled);
  }

  *status_string = NULL;

  // Here is not the place for getting the indent string 
  // but it'll do for now.
  char *t_indent_str = NULL;
  SdmString indent_str;

  mail_rc->GetValue(error, "indentprefix", &t_indent_str);
  if (t_indent_str && error == Sdm_EC_Success) {
    indent_str = t_indent_str;
    free(t_indent_str);
  }
  else {
    indent_str = "> ";
    error = Sdm_EC_Success;
  }

  disable_redisplay();

  int indent = 0;

  // Before any data is placed in the contents cache, attempt to preallocate 
  // some space to avoid needless copying of the data - ignore an error in case
  // an even smaller allocation might do if done piece meal

  (void) preallocate_contents_cache(1024);	// 1k for headers and brackets

  if (format == IF_BRACKETED) {
    char * ins_bracket;
    switch (brackets) {
    case BF_FORWARD:
      ins_bracket = catgets(DT_catd, 1, 195, "------------- Begin Forwarded Message -------------\n\n");
      break;

    case BF_INCLUDE:
    default:
      ins_bracket = catgets(DT_catd, 1, 196, "------------- Begin Included Message -------------\n\n");
      break;
    }

    if (append_to_contents_cache(ins_bracket, strlen(ins_bracket), 1) != 0) {
      error = Sdm_EC_NoMemory;
      (void) insert_contents_cache_at_cursor ();
      enable_redisplay();
      return (firstBPHandled);
    }
  }

  // Code adapted from MsgScrollingList - display_message().  Need to
  // duplicate headers from the message first, then the message body.

  int line_size = 1024;   
  int hdr_num = 0;

  // We're trying to reduce heap size by not allocating and deleting
  // space in every loop iteration.  So just have a fixed size buffer
  // initially.  Initial line size.  When not enough, allocate more.

  char *line = new char[line_size];
  if (!line) {
    error = Sdm_EC_NoMemory;
    return(firstBPHandled);
  }

  if (header_format != HF_NONE) {
    SdmStrStrL headers;
    env->GetHeaders(error, headers);
    // if there are no headers (which would be very weird but it could happen)
    // then we just reset the error and continue.
    if (error == Sdm_EC_RequestedDataNotFound) {
      error = Sdm_EC_Success;
    } 
    else if (error != Sdm_EC_Success) {
      delete [] line;
      return (firstBPHandled);
    }
      
    for (hdr_num=0; hdr_num<headers.ElementCount(); hdr_num++) {

      SdmString value;
      SdmString name(headers[hdr_num].GetFirstString());

      // I18N - decode only the From, Subject, To, and Cc headers
      if ( strcasecmp((const char *)name,"from")==0 
	   || strcasecmp((const char *)name,"subject")==0
	   || strcasecmp((const char *)name,"to")==0
	   || strcasecmp((const char *)name,"cc")==0 ) {
        // I18N Decode the header and concatenate the segments
        // that are returned, then append them to the line.
        SdmString charset;
        if ( (env->GetCharacterSet(error, charset) != Sdm_EC_Success)
	     || (env->DecodeHeader (error, value,
				    (const char *)headers[hdr_num].GetSecondString(), charset) != Sdm_EC_Success)) {
	  delete [] line;
          return (firstBPHandled);
	}

      }
      else {
        value = headers[hdr_num].GetSecondString();
      }
      // I18N - end 
          
      // Check to see if user wants us to ignore this particular
      // header.

      if ((header_format == HF_ABBREV) && (hdr_num != 0 || strcasecmp((const char *)name, "from") != 0)) {
        SdmError ierror;
        if (mail_rc->Ignore(ierror, name)) {
          continue;
        }
      }

      int tmp_count = name.Length() +  value.Length() + indent_str.Length() + 5;
          
      if ( tmp_count > line_size ) { // Need to increase line size.
        delete [] line;
	line = NULL;
        line_size = tmp_count;
        line = new char[line_size];
	if (!line) {
	  error = Sdm_EC_NoMemory;
	  return(firstBPHandled);
	}
      }

      assert(line);
      line[0] = '\0';
      int prefix_length;
      if (format == IF_INDENTED) {
        strcpy(line, (const char *)indent_str);
        strcat(line, name);
	prefix_length = indent_str.Length() + name.Length();
      } 
      else {
        strcpy(line, name);
	prefix_length = name.Length();
      }

      // If this is the first header or it is specifically the
      // "From" line then don't tag on a ":".
      //
      if (hdr_num != 0 || strcasecmp((const char *)name, "from") != 0) {
        strcat(line, ": ");
	prefix_length += 2;	// length of strcat'd text
      }
      else {
        strcat(line, " ");
	prefix_length += 1;	// length of strcat'd text
      }

      strcat(line, value);
      strcat(line, "\n");
      if (append_to_contents_cache(line, prefix_length + value.Length() + 1, 1) != 0) {
	error = Sdm_EC_NoMemory;
        (void) insert_contents_cache_at_cursor();
        enable_redisplay();
	delete [] line;
        return (firstBPHandled);
      }
    }
  }

  // Don't delete line yet, because it's used below.
   
  if (format == IF_INDENTED) {
    if (append_to_contents_cache((const char *)indent_str, indent_str.Length(), 1) != 0) {
      error = Sdm_EC_NoMemory;
      (void) insert_contents_cache_at_cursor ();
      enable_redisplay();
      delete [] line;
      return (firstBPHandled);
    }
  }

  if (header_format != HF_NONE) {
    if (append_to_contents_cache("\n", 1, 1) != 0) {
      error = Sdm_EC_NoMemory;
      (void) insert_contents_cache_at_cursor ();
      enable_redisplay();
      delete [] line;
      return (firstBPHandled);
    }
  }

  // OK, all of the headers have been safely constructed and placed in
  // the editor contents cache - time to deal with the "message body"

  SdmMessageBody *bp;
  SdmMessageBodyStructure mbstruct;
  if ( (msg->SdmMessageBodyFactory(error, bp, 1) != Sdm_EC_Success)
       || bp->GetStructure(error, mbstruct) != Sdm_EC_Success) {
    delete [] line;
    return (firstBPHandled);
  }
    
  // 8/5/96 - esthert
  // if this is a multipart mixed, use the first body part
  // in the multipart body.
  if ( mbstruct.mbs_type == Sdm_MSTYPE_multipart ) {
    SdmMessage *newMsg;
    bp->SdmMessageFactory(error, newMsg);
    if (error) {
      // we can't get the parts of the nested message so 
      // we just continue below using the body structure of
      // the message.
      error = Sdm_EC_Success;    // reset error.
    } 
    else {
      if ( (newMsg->SdmMessageBodyFactory(error, bp, 1) != Sdm_EC_Success)
	   || bp->GetStructure(error, mbstruct) != Sdm_EC_Success) {
      delete [] line;
      return (firstBPHandled);
      }
    }
  }

  // 8/5/96 - esthert
  // this is how we determine if we can display the contents of the
  // body in the message area.  The body must be:
  //  a.  text/plain AND
  //  b.  disposition is not attachment.
  //
  // note that we do not check for the attachment name != empty string
  // because some mailers automatically set the attachment name even
  // though it is not an attachment.
  //
  if ((mbstruct.mbs_mime_content_type == "text/plain") &&
      (mbstruct.mbs_disposition != Sdm_MSDISP_attachment))
    {
      // Let's assume that the first body part is of type text
      SdmString contents;
      SdmContentBuffer content_buf;

      bp->GetContents(error, content_buf);
      if (error == Sdm_EC_RequestedDataNotFound) {
        error = Sdm_EC_Success;
      } else if (error != Sdm_EC_Success) {
	delete [] line;
        return (firstBPHandled);
      } else if (content_buf.Length() == 0) {
	insert_contents_cache_at_cursor();
	delete [] line;
	enable_redisplay();
	return (Sdm_True);
      }

      content_buf.GetContents(error, contents);
      if (error) {
	delete [] line;
        return (firstBPHandled);
      }

      // I18N - decode the first body part's contents
      // assume it's text
      char *newData;
      int newLength;
      SdmString charset;

      // Get the user chosen charset, if there isn't one, get the one
      // specified by the MIME charset parameter
      content_buf.GetCharacterSet(error, charset);
      if (charset.Length() == 0)
        content_buf.GetDataTypeCharSet(error, charset);

      SdmContentBuffer::DecodeCharset (error, newData, newLength,
				       (const char *)contents, contents.Length(), charset);
      if (!error) {
        // Note: we shouldn't null terminate the data because it is not
        // necessarily a string.
        // Let the contents SdmString "own" newData
        contents = SdmString(&newData, (SdmStringLength)newLength, (size_t)newLength);
      }

      error = Sdm_EC_Success;

      if (format == IF_INDENTED) {
        int byte_count = 0;
        int content_count;
        const char *last = NULL, *content_ptr = NULL;
	unsigned long final_result_size;

	// Predetermine the size of the final output and have the editor widget
	// do the preallocation for us so we avoid copying the result to death.

	for (content_ptr = (const char *)contents, content_count = contents.Length(), final_result_size = 0; 
	     content_count;
	     content_count--, content_ptr++) {
	  if (*content_ptr == '\n')
	    final_result_size++;
	  else if (*content_ptr == '\r' && content_count > 1 && content_ptr[1] == '\n')
	    final_result_size--;
	}

	if (preallocate_contents_cache(final_result_size) != 0) {
	  error = Sdm_EC_NoMemory;
	  (void) insert_contents_cache_at_cursor();
	  enable_redisplay();
	  delete [] line;
	  return (firstBPHandled);
	}

        // Parse the result of getContents().
        // Spit out indent string with each line.
        // Is contents NULL terminated???

	int crFound = 0;	// used to control append_to_contents_cache behavior

        for ( last = (const char *)contents, content_ptr = (const char *)contents, content_count = contents.Length();
	      content_count > 0;
	      content_ptr++, byte_count++, content_count-- ) 
	  {
	    if (*content_ptr == '\r') {
	      crFound++;
	    }
	    else if ((*content_ptr == '\n') ||  (content_count == 1) ) {
              // 2 for null terminator and new line.
              int tmp_count = indent_str.Length() + byte_count + 2;
	      if (*content_ptr == '\r')
		crFound++;
              if ( tmp_count > line_size ) { 
                // Need to increase line size.
                delete [] line;
		line = NULL;
                line_size = tmp_count;
                line = new char[line_size];
		if (!line) {
		  error = Sdm_EC_NoMemory;
		  return(firstBPHandled);
		}
              }
	      assert(line);
              strcpy(line, (const char *)indent_str);
              strncpy(line+indent_str.Length(), last, byte_count + 1);	// +1 - Copy the '\n'/last byte also
              if (append_to_contents_cache(line, indent_str.Length() + byte_count + 1, !crFound) != 0) {
		error = Sdm_EC_NoMemory;
                (void) insert_contents_cache_at_cursor ();
                enable_redisplay();
		delete [] line;
                return (firstBPHandled);
              }
              last = content_ptr + 1;
              byte_count = -1;
	      crFound = 0;
            }
	  }  // end of for loop
      }
      else {
	// We have so far cached headers and some other information but now we need
	// to place the message contents into the window - if we were to call the
	// append to contents and cache method, if would grow the cache by the size
	// of the message contents and copy it. This is really wasteful of memory and
	// cpu, so instead we cause the current cached info to be output and then
	// directly append the contents of the message.

	insert_contents_cache_at_cursor();	// causes insertion point to be at end of buffer
	if (insert_at_cursor((const char *)contents, contents.Length()) != 0) {
	  error = Sdm_EC_NoMemory;
          (void) insert_contents_cache_at_cursor ();
          enable_redisplay();
	  delete [] line;
          return (firstBPHandled);
        }
      }
      firstBPHandled = Sdm_True;
    }
	
  delete [] line;

  if (format == IF_BRACKETED) {
    char * ins_bracket;
    switch (brackets) {
    case BF_FORWARD:
      ins_bracket = catgets(DT_catd, 1, 197, "\n------------- End Forwarded Message -------------\n\n");
      break;

    case BF_INCLUDE:
    default:
      ins_bracket = catgets(DT_catd, 1, 198, "\n------------- End Included Message -------------\n\n");
      break;
    }

    if (append_to_contents_cache(ins_bracket, strlen(ins_bracket)) != 0) {
      error = Sdm_EC_NoMemory;
      insert_contents_cache_at_cursor ();
      enable_redisplay();
      return (firstBPHandled);
    }
  }

  /* Add the contents to the DtEditor widget */
  (void) insert_contents_cache_at_cursor ();

  enable_redisplay();

  return(firstBPHandled);
}

void 
AbstractEditorParent::postAttachmentPopup(XEvent *event)
{
  XmMenuPosition(_attachmentPopupMenu, (XButtonEvent *)event);
  XtManageChild(_attachmentPopupMenu);
}

void 
AbstractEditorParent::postTextPopup(XEvent *event)
{
  if (_textPopupMenu == NULL)
    return;

  XmMenuPosition(_textPopupMenu, (XButtonEvent *)event);
  XtManageChild(_textPopupMenu);
}

