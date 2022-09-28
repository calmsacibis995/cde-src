// @(#)SortCmd.hh	1.5 11/30/95 01:16:49 SMI
#include "Sort.hh"

// commands for sort menu items

class SortCmd : public RoamCmd {
  public:
    virtual void doit();   
    //SortCmd( char *, int, RoamMenuWindow * );
    SortCmd( char *, char *, int, RoamMenuWindow *, enum sortBy);
    virtual ~SortCmd();
    virtual const char *const className () { return "SortCmd"; }
  private:
    RoamMenuWindow	*_sortparent;
    enum sortBy		_sortstyle;
    Sort		*_sorter;
};
