#ifndef _SHUTDOWNACTIONS_HH
#define _SHUTDOWNACTIONS_HH
#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)ShutdownActions.hh	1.5 03/12/94"
#endif

class ShutdownActions {
  public:
    ShutdownActions(int num_actions = 32);
    ~ShutdownActions(void);

    typedef int (*ShutdownActionProc)(void *);

    void addAction(ShutdownActionProc, void * cb_data);
    void removeAction(ShutdownActionProc, void * cb_data);

    int doActions(void);

  private:
    struct ActionEntry {
	ShutdownActionProc	proc;
	void *			call_back_data;
    };

    ActionEntry		*_actions;
    int			_action_list_size;
    int			_action_count;

    void removeEntry(int);
};

#endif
