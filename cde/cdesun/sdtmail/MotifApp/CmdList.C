/*
 *+SNOTICE
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)CmdList.C	1.23 05/14/97"
#endif

///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//         This example code is from the book:
//
//           Object-Oriented Programming with C++ and OSF/Motif
//         by
//           Douglas Young
//           Prentice Hall, 1992
//           ISBN 0-13-630252-1	
//
//         Copyright 1991 by Prentice Hall
//         All Rights Reserved
//
//  Permission to use, copy, modify, and distribute this software for 
//  any purpose except publication and without fee is hereby granted, provided 
//  that the above copyright notice appear in all copies of the software.
///////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
// CmdList.C: Maintain a list of Cmd objects
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// MODIFIED TO INHERIT FROM CMD - not described in Book
///////////////////////////////////////////////////////////


#include "CmdList.h"

CmdList::CmdList() : Cmd("CmdList", NULL, 1)
{
    _contents = 0;
    _numElements = 0;
    _pane = NULL;
    _data = NULL;
    _more_data = NULL;
    _cascade = NULL;
    _callback = NULL;
    _menu_pending = FALSE;
    _destroyed = FALSE;
}

CmdList::CmdList(char *name, char *label ) : Cmd(name, label, 1)
{
    _contents    = 0;
    _numElements = 0;
    _pane = NULL;
    _data = NULL;
    _more_data = NULL;
    _cascade = NULL;
    _callback = NULL;
    _menu_pending = FALSE;
    _destroyed = FALSE;
}

CmdList::~CmdList()
{
  // free the list
  delete[] _contents;
}

// deleteMe will delete 'this' if _menu_pending is not TRUE, otherwise
// it will just set the _destroyed flag to TRUE.
void CmdList::deleteMe()
{
  // GL - To be on the safe side lets make sure that the "this" pointer
  // is not nill.  Trust me, this is a good thing to do here.
  if (this == NULL)
    return;
  if (isPending()) {
    setDestroyed(TRUE);
  } else {
    delete this;
  }
}  

void CmdList::add ( Cmd *cmd )
{
    int i;
    Cmd **newList;

    // CmdList can only be undone if all Cmds it contains can be undone
    if(!cmd->hasUndo())
	_hasUndo = 0;

    if ((_numElements % 6) == 0) {
	newList = new Cmd*[_numElements + 6];

	for( i = 0; i < _numElements; i++)
	    newList[i] = _contents[i];

	if (_contents)
	    delete []_contents;

	_contents = newList;
    }
    _contents[_numElements++] = cmd;
}
// This inserts alphabetically based on the label field
void CmdList::insert ( Cmd *cmd )
{
    int i;
    Cmd **newList;

    // CmdList can only be undone if all Cmds it contains can be undone
    if(!cmd->hasUndo())
        _hasUndo = 0;

    if ((_numElements % 6) == 0) {
        newList = new Cmd*[_numElements + 6];

        for( i = 0; i < _numElements; i++)
            newList[i] = _contents[i];

        if (_contents)
            delete []_contents;

        _contents = newList;
    }   
    _contents[_numElements++] = cmd;

    // Sort
    char *label, *label2;
    for( i = _numElements-1; i > 0; i--) {
        label = _contents[i-1]->getLabel();
        label2 = _contents[i]->getLabel();
        if (strcasecmp(label, label2) > 0) {
                Cmd *save_cmd = _contents[i-1];
                _contents[i-1] = _contents[i];
                _contents[i] = save_cmd;
        }
    }
}
void CmdList::collInsert ( Cmd *cmd )
{
    int i;
    Cmd **newList;

    // CmdList can only be undone if all Cmds it contains can be undone
    if(!cmd->hasUndo())
        _hasUndo = 0;

    if ((_numElements % 6) == 0) {
        newList = new Cmd*[_numElements + 6];

        for( i = 0; i < _numElements; i++)
            newList[i] = _contents[i];

        if (_contents)
            delete []_contents;

        _contents = newList;
    }   
    _contents[_numElements++] = cmd;

    // Sort
    char *label, *label2;
    for( i = _numElements-1; i > 0; i--) {
        label = _contents[i-1]->getLabel();
        label2 = _contents[i]->getLabel();
        if (strcoll(label, label2) > 0) {
                Cmd *save_cmd = _contents[i-1];
                _contents[i-1] = _contents[i];
                _contents[i] = save_cmd;
        }
    }
}

void CmdList::remove ( int index )
{
    int i;

    for (i=index; i<(_numElements-1); i++) {
	_contents[i] = _contents[i+1];
    }

    _numElements--;
}    

Cmd *CmdList::operator[] ( int index )
{
    // Return the indexed element
    
    return _contents[index];
}


void CmdList::doit()
{
    for( int i = 0; i < _numElements; i++)
	_contents[i]->execute();
}

void CmdList::undoit()
{
    if(_hasUndo)
	for( int i = _numElements - 1; i >=0; i--)
	    _contents[i]->undo();
}

void CmdList::addMenuCompleteCallback(MenuCompleteCallback cb,
				      void *client_data)
{
    _callback = cb;
    _more_data = client_data;
}

void CmdList::menuComplete()
{
    if (_callback)
        _callback(_more_data);
}
