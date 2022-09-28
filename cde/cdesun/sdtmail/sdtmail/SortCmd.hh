// @(#)SortCmd.hh	1.6 10/22/96 01:28:28 SMI
#include "Sort.hh"

// commands for sort menu items

class SortCmd : public RoamCmd {
  public:
    virtual void doit();   
    //SortCmd( char *, int, RoamMenuWindow * );
    SortCmd( char *, char *, int, RoamMenuWindow *, enum sortBy);
    virtual ~SortCmd();
    virtual const char *const className () { return "SortCmd"; }
    Sort *getSorter() { return _sorter; }
    enum sortBy getSortStyle() { return _sortstyle; }
  private:
    RoamMenuWindow	*_sortparent;
    enum sortBy		_sortstyle;
    Sort		*_sorter;
};
