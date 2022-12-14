/*
 *+SNOTICE
 *
 *	$Revision: 1.2 $
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
#pragma ident "@(#)Node.C	1.8 02/17/94"
#endif

#include <stdio.h>

#include	"Node.h"

// In the design, we had abstracted out the DOMF related activities
// (insertion in lookup table, creation of the objref corresponding to
// servant...) into <>Impl classes.
// The <>Servant classes derive from the <>Impl classes.
// Note how, in the constructor of the NodeServant class, we call the 
// constructor of the parent NodeImpl class passing in "this".
// The NodeImpl constructor uses "this" to insert into the lookup table...


Node::Node()
{

    my_message_handle    = NULL;
    my_message_header    = NULL;

    homep = DTM_FALSE;
    deleted_p = DTM_FALSE;
    my_previous_node = NULL;
    my_next_node  = NULL;
}

Node::Node(
    DtMailMessageHandle msg_num,
    char*     hdr
)
{
    my_message_handle    = msg_num;
    my_message_header    = hdr;

    homep = DTM_FALSE;
    deleted_p = DTM_FALSE;
    my_previous_node = NULL;
    my_next_node  = NULL;
}

Node::~Node() {}

void 
Node::set_homep(
)
{
    homep = DTM_TRUE;
}

boolean
Node::is_home(
)
{
    return(homep);
}

// Starting with the current node "this", traverse nodes until
// home node is reached.

Node*
Node::get_home(
)
{
    Node* a_node;
    unsigned char  is_home_p;


    a_node = this;

    is_home_p = a_node->is_home();

    if (is_home_p) {
	return (this);
    }
    else {
	Node* n;
	
	n = a_node->next();
	n->get_home();
    }
}

void
Node::set_message_handle(
    DtMailMessageHandle a_hndl
)
{
    my_message_handle = a_hndl;
}


DtMailMessageHandle
Node::get_message_handle()
{
    return(my_message_handle);
}

void
Node::set_message_header(
    char* hdr
)
{
    my_message_header = hdr;
}

char* 
Node::get_message_header()
{
    return(my_message_header);
}

Node* 
Node::next(
)
{

    return(my_next_node);

}

// Don't attempt duplicating a nil objref!

Node* 
Node::prev(
)
{

    return(my_previous_node);

}

// Don't attempt duplicating a nil objref!

void
Node::set_previous_node(
	Node* a_node
)
{
    my_previous_node = a_node;

}

// Don't attempt duplicating a nil objref!

void
Node::set_next_node(
	Node* a_node
)
{
    my_next_node = a_node;
}

// append anyNode to self

void 
Node::append(
	Node* anyNode
)
{

    Node* tmpNext;
    Node* tmpSelf;
    Node* inNode;

    if (anyNode == NULL) {
	printf("Cannot append NIL node!");
	return;
    }

    inNode = anyNode;

    tmpSelf = this;

    if (my_next_node)
      {
	  tmpNext = my_next_node;
	  my_next_node = inNode;
	  inNode->set_previous_node(tmpSelf);
	  inNode->set_next_node(my_next_node);
	  tmpNext->set_previous_node(inNode);
      }
    else
      {
	  my_next_node = inNode;
	  my_previous_node      = inNode;
	  inNode->set_previous_node(tmpSelf);
	  inNode->set_next_node(tmpSelf);
      }
}

// prepend anyNode to self

void
Node::prepend(
	Node* anyNode
)
{
    
    Node* tmpPrev;
    Node* inNode;
    Node* tmpSelf;

    if (anyNode == NULL) {
	printf("Cannot prepend NIL node!");
	return;
    }

    inNode  = anyNode;
    tmpSelf = this;

    if (my_previous_node)
      {
	  tmpPrev = my_previous_node;
	  my_previous_node = inNode;
	  inNode->set_next_node(tmpSelf);
	  inNode->set_previous_node(tmpPrev);
	  tmpPrev->set_next_node(inNode);
      }
    else
      {
	  my_next_node = inNode;
	  my_previous_node = inNode;
	  inNode->set_previous_node(tmpSelf);
	  inNode->set_next_node(tmpSelf);
      }

}

// Good etiquette requires that clients remove() a node
// before destroy()-ing it.
    
void 
Node::remove(
)
{

    my_previous_node->set_next_node(my_next_node);

    my_next_node->set_previous_node(my_previous_node);

}


void
Node::set_number(int i)
{
    bogus_number = i;

}

int
Node::get_number()
{
    return(bogus_number);
}




