class PtkToken : public SdmLinkedListElement {

public:
  PtkToken() : SdmLinkedListElement(this), ptk_token(0), ptk_name(0) {};
  virtual ~PtkToken() { 
    if (ptk_token) {
      delete ptk_token;
      ptk_token = 0;
    }
    if (ptk_name) {
      delete ptk_name;
      ptk_name = 0;
    }
  }
  SdmToken	*ptk_token;
  SdmString	*ptk_name;
};

int Ctoken(int argc, char **argv);
extern char *Htoken;
PtkToken *lookupToken(char *token, int printMessage);

