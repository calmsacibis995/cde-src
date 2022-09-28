#include <SDtMail/Sdtmail.hh>

class PtResult {
public:

  PtResult();
  ~PtResult();

  void PtResult::operator=(const SdmBoolean s);

  void PtResult::operator=(const char* s);

  void PtResult::operator=(const SdmString& s);
  void PtResult::operator=(const SdmStringL& s);
  void PtResult::operator=(const SdmStringLL& s);

  void PtResult::operator=(SdmStrStr& s);
  void PtResult::operator=(SdmStrStrL& s);
  void PtResult::operator=(SdmStrStrLL& s);

  void PtResult::operator=(SdmIntStr& s);
  void PtResult::operator=(SdmIntStrL& s);
  void PtResult::operator=(SdmIntStrLL& s);

  void PtResult::Reset();
  void PtResult::Print();
  void PtResult::SetIntersperseOutput() { _intersperse = Sdm_True; }
  void PtResult::SetFileOutput()  { _intersperse = Sdm_False; }
  void PtResult::OutputResults()  { _displayResults = Sdm_True; }
  void PtResult::Validate(SdmString &testName, SdmString &validateString);

private:
  SdmString _result;
  SdmBoolean _intersperse;
  SdmBoolean _displayResults;
  int _outputFileNumber;
};
