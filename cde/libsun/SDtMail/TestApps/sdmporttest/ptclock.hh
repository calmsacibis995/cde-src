class PlkLock : public SdmLinkedListElement {

public:
  PlkLock() : SdmLinkedListElement(this), plk_lock(0), plk_name(0),	
    plk_idobject_name("")
  {};
  virtual ~PlkLock() { 
    if (plk_lock) {
      delete plk_lock;
      plk_lock = 0;
    }
    if (plk_name) {
      delete plk_name;
      plk_name = 0;
    }
  }
  SdmLockUtility	*plk_lock;
  SdmString	*plk_name;
  SdmString	plk_idobject_name;
};

int Clock(int argc, char **argv);
extern char *Hlock;
PlkLock *lookupLock(char *Lock, int printMessage);
extern void DestroyAllLocks();
