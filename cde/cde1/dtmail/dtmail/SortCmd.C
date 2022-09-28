// @(#)SortCmd.C	1.7 11/30/95 01:16:40 SMI
#include "SortCmd.hh"
#include <stdlib.h>
#include <unistd.h>
#include "RoamApp.h"
#include "MailMsg.h"

SortCmd::SortCmd( 
		  char *name, 
		  char *label,
		  int active, 
		  RoamMenuWindow *window,
		  enum sortBy sortstyle 
		  ) : RoamCmd ( name, label, active, window )
{
	// initialize the internal state
	_sortparent = window;
	_sortstyle = sortstyle;
	_sorter = new Sort ();
}

SortCmd::~SortCmd() 
{
	delete _sorter;
}

void
SortCmd::doit()
{
	MsgScrollingList	*displayList;
	DtMail::MailBox		*mbox;
	int			current_msg;

    	theRoamApp.busyAllWindows(catgets(DT_catd, 1, 219,
			"Sorting..."));

	// Get Mailbox
	mbox = _sortparent->mailbox();

	// Get array of message handles from scrolling list
	displayList = _sortparent->list();

	// Sort array of message handles
	current_msg = _sorter->sortMessages (displayList, mbox, _sortstyle);

	// The array of message handles is sorted. Now we need to update
	// the display
	displayList->updateListItems(current_msg);

        theRoamApp.unbusyAllWindows();
}
