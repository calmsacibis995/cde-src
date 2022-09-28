/* $XConsortium: DtPrtJobIcon.h /main/cde1_maint/2 1995/10/23 09:58:02 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef DTPRTJOBICON_H
#define DTPRTJOBICON_H

#include "IconObj.h"
#include "PrintJob.h"

class DtMainW;
class DtProps;

extern const char *PROPS_PRINTJOB_ID;
extern const char *PRINTJOB_ID;

class DtPrtJobIcon : public IconObj 
{

   DtMainW *mainw;
   DtProps *props;
   PrintJob *print_job;
   char job_number[10];

   boolean HandleHelpRequest();
   void NotifyVisiblity(BaseUI *);

 public:
   DtPrtJobIcon(DtMainW *, AnyUI *parent, PrintJob *printJob,
		int SequenceNumber);
   ~DtPrtJobIcon();

   void DisplayProps();
   void PrintJobObj(PrintJob *printJob);
   PrintJob *PrintJobObj() { return print_job; }
   const char *JobNumber() { return job_number; }

};

#endif // DTPRTJOBICON_H
