// @(#)SortCmd.C	1.14 03/27/97 20:12:27 SMI
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
  DtMailGenDialog *genDialog = theRoamApp.genDialog();
	MsgScrollingList	*displayList;
	SdmMessageStore		*mbox;
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
	SdmError mail_error;
	displayList->updateListItems(mail_error, current_msg);

  if (mail_error) {
    genDialog->post_error(mail_error, DTMAILHELPCANNOTUPDATEMSGHDRLIST,
			  catgets(DT_catd, 3, 133, "Mailer cannot update the Message Header List."));
  } 
  else {
    // if we are sorting by message number, just reset the last
    // sort command kept in the roam menu window.  otherwise,
    // same this sort command as the last method of sorting..
    if (_sortstyle == SortMsgNum) {
      _sortparent->setLastSortCmd(NULL);
    } else {
      _sortparent->setLastSortCmd(this);
    }
  }

  theRoamApp.unbusyAllWindows();
}
