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
#pragma ident "@(#)Editor.C	1.39 04/05/96"
#else
static char *sccs__FILE__ = "@(#)Editor.C	1.39 04/05/96";
#endif

#include <Xm/RowColumn.h>
#include <EUSCompat.h>
#include <assert.h>
#include <stdio.h>

#include <Dt/Dts.h>

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

DtMailBoolean
Editor::set_message(DtMail::Message * msg,
		    char ** status_string,
		    HeaderFormat header_format,
		    InsertFormat format,
		    BracketFormat brackets)
{
    DtMailEnv error;
    DtMail::Session *m_session = theRoamApp.session()->session(); 
    DtMail::MailRc * mail_rc = m_session->mailRc(error);

    *status_string = NULL;
    DtMailBoolean firstBPHandled = DTM_FALSE;

    DtMail::Envelope *env = msg->getEnvelope(error);

    DtMailHeaderHandle hdr_hnd;
    char *name;
    DtMailValueSeq value;
	int status;

    // Here is not the place for getting the indent string 
    // but it'll do for now.
    const char *indent_str;

    mail_rc->getValue(error, "indentprefix", &indent_str);
    if ( error.isSet() ) 
	    indent_str = strdup("> ");

    disable_redisplay();

    int indent = 0;

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

	status = append_to_contents_and_cache(ins_bracket, strlen(ins_bracket));
	if (status != 0) {
    		status = update_contents ();
		enable_redisplay();
		return DTM_TRUE;
	}
    }

    // Code from MsgScrollingList - display_message().
    // We're trying to reduce heap size by not allocating and 
    // deleting space in every loop iteration.  So just have a 
    // fixed size buffer initially.
    // 

    // Initial line size.  When not enough, allocate more.
    int line_size = 1024;   
    int tmp_count = 0;
    char *line = new char[line_size];
    int hdr_num = 0;

    if (header_format != HF_NONE) {
	for ( hdr_hnd = env->getFirstHeader(
					    error, 
					    &name, 
					    value);
	      hdr_hnd && !error.isSet();
	      hdr_hnd = env->getNextHeader(
					   error, 
					   hdr_hnd, 
					   &name, 
					   value), hdr_num++ ) {
	    
	    
	    if ((header_format == HF_ABBREV)
			&& (hdr_num != 0 || strcmp(name, "From") != 0)) {
		DtMailEnv ierror;
		if (mail_rc->ignore(ierror, name)) {
		    free(name);
		    value.clear();
		    continue;
		}
	    }
	    
	    for ( int val = 0;  val < value.length();  val++ ) {
		tmp_count = strlen(name) + 
		    strlen(*(value[val])) +
		    strlen(indent_str) + 
		    5;
		if ( tmp_count > line_size ) { // Need to increase line size.
		    delete line;
		    line_size = tmp_count;
		    line = new char[line_size];
		}
		memset(line, 0, line_size);
		if (format == IF_INDENTED) {
		    strcpy(line, indent_str);
		    strcat(line, name);
		} else {
		    strcpy(line, name);
		}
		
		if (hdr_num != 0 || strcmp(name, "From") != 0) {
		    strcat(line, ": ");
		}
		else {
		    strcat(line, " ");
		}
		
		strcat(line, *(value[val]));
		strcat(line, "\n");
		status = append_to_contents_and_cache(line, strlen(line));
		if (status != 0) {
    			status = update_contents ();
			enable_redisplay();
			return DTM_TRUE;
		}
		}
	    value.clear();
	    free(name);
	}
    }
    // Don't delete line yet, because it's used below.
   
    if (format == IF_INDENTED) {
	status = append_to_contents_and_cache(indent_str, strlen(indent_str));
	if (status != 0) {
    		status = update_contents ();
		enable_redisplay();
		return DTM_TRUE;
	}
	}

    if (header_format != HF_NONE) {
	status = append_to_contents_and_cache("\n", 1);
	if (status != 0) {
    		status = update_contents ();
		enable_redisplay();
		return DTM_TRUE;
	}
	}

    DtMail::BodyPart *bp = msg->getFirstBodyPart(error);
   
    char *type;

    bp->getContents(error, NULL, NULL, &type, NULL, NULL, NULL);
    if ( type ) {
	char *attr = DtDtsDataTypeToAttributeValue(
					type, 
					DtDTS_DA_IS_TEXT, 
					NULL);
	if ((attr && strcasecmp(attr, "true") == 0)
	    || strcoll(type, DtDTS_DT_UNKNOWN) == 0 ) {
	    const void *contents;
	    unsigned long size;
	    bp->lockContents(error, DtMailLockRead);
	    bp->getContents(error, &contents, &size, NULL, NULL, NULL, NULL);
	    if (format == IF_INDENTED) {
		int byte_count = 0;
		int content_count = size;
		const char *last = NULL, *content_ptr = NULL;
		// Parse the result of getContents().
		// Spit out indent string with each line.
		// Is contents NULL terminated???
		for ( last = (const char *)contents, 
		    content_ptr = (const char *)contents;
		    content_count > 0;
		    content_ptr++, 
		    byte_count++, 
		    content_count-- ) {
		    if ((*content_ptr == '\n') || 
			(content_count == 1) ) {
			// 2 for null terminator and new line.
			tmp_count = strlen(indent_str) + byte_count + 2;
			if ( tmp_count > line_size ) { 
			   // Need to increase line size.
			   delete line;
			   line_size = tmp_count;
			   line = new char[line_size];
			}
			memset(line, 0, line_size);
			strcpy(line, indent_str);
			strncat(line, last, byte_count + 1);   
			// Copy the '\n' also
			status = append_to_contents_and_cache(line, strlen(line));
			if (status != 0) {
    				status = update_contents ();
				enable_redisplay();
				return DTM_TRUE;
			}
			last = content_ptr + 1;
			byte_count = -1;
		    }
		}  // end of for loop
	    } else {
	       status = append_to_contents_and_cache((const char *)contents, size);
		   if (status != 0) {
    			status = update_contents ();
		  	enable_redisplay();
		   	return DTM_TRUE;
		   }
	    }
	    bp->unlockContents(error);
	    firstBPHandled = DTM_TRUE;
	    if (attr) {
		DtDtsFreeAttributeValue(attr);
		attr = NULL;
	    }
	}
	free(type);

	// DLP: We will turn off this test for now. We need to study the problem
	// of text checksums more.
	//
	//if (bp->checksum(error) == DtMailCheckBad) {
	//    *status_string = strdup(catgets(DT_catd, 1, -1, "Digital signature did not match."));
	//}
    }
    delete line;

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

	status = append_to_contents_and_cache(ins_bracket, strlen(ins_bracket));
	if (status != 0) {
    		status = update_contents ();
		enable_redisplay();
		return DTM_TRUE;
	}
	}

    /* Add the contents to the DtEditor widget */
    status = update_contents ();

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

