class SdmDataPort;
class SdmString;

class PdpDataPort : public SdmLinkedListElement {

public:
  PdpDataPort() : SdmLinkedListElement(this) {};
  virtual ~PdpDataPort() {
    if (pdp_dataport) {
      delete pdp_dataport;
      pdp_dataport = 0;
    }
    if (pdp_name) {
      delete pdp_name;
      pdp_name = 0;
    }
  };
  SdmDataPort	*pdp_dataport;
  SdmString	*pdp_name;
};

int decodeHeaderAbstractValue(SdmMessageHeaderAbstractFlags &headerAbstractFlags, char *sequence, int printMessage);
char *printAbstractHeaderFlag(SdmMessageHeaderAbstractFlags flags);
void printFlags(SdmMessageFlagAbstractFlags &flags);
int decodeFlagAbstractValue(SdmMessageFlagAbstractFlags &flagAbstractFlags, char *sequence, int printMessage);
void printFlags(SdmMessageFlagAbstractFlags &flags);
int Cport(int argc, char **argv);
extern char *Hport;
