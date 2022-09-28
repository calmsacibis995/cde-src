#include <SDtMail/Message.hh>
class SdmString;

class PmsgMessage : public SdmLinkedListElement {

public:
  PmsgMessage() : SdmLinkedListElement(this) {
  pmsg_message = (SdmMessage *)0;
  pmsg_messagename = (SdmString *)0;
  pmsg_messagestorename = (SdmString *)0;
  pmsg_messagenum = 0;
  };
  virtual ~PmsgMessage() {
    if (pmsg_message) {
      delete pmsg_message;
      pmsg_message = 0;
    }
    if (pmsg_messagename) {
      delete pmsg_messagename;
      pmsg_messagename = 0;
    }
    if (pmsg_messagestorename) {
      delete pmsg_messagestorename;
      pmsg_messagestorename = 0;
    }
    pmsg_messagenum = 0;
  };
  SdmMessage		*pmsg_message;
  SdmString		*pmsg_messagename;
  SdmString		*pmsg_messagestorename;
  SdmMessageNumber       pmsg_messagenum;
};

int Cmessage(int argc, char **argv);
extern char *Hmessage;
