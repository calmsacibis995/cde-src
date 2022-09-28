#include <SDtMail/MessageStore.hh>
class SdmString;

class PmstMessageStore : public SdmLinkedListElement {

public:
  PmstMessageStore() : SdmLinkedListElement(this) {};
  virtual ~PmstMessageStore() {
    if (pmst_messagestore) {
      delete pmst_messagestore;
      pmst_messagestore = 0;
    }
    if (pmst_storename) {
      delete pmst_storename;
      pmst_storename = 0;
    }
  };
  SdmMessageStore	*pmst_messagestore;
  SdmString		*pmst_storename;
};

int Cstore(int argc, char **argv);
extern char *Hstore;
PmstMessageStore *lookupStore(char *storeName, int printMessage);
