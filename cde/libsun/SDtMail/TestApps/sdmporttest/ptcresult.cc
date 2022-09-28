#include <ptcresult.hh>
#include <stdio.h>
#include <errno.h>

PtResult::PtResult() : _intersperse(Sdm_True), _displayResults(Sdm_False),
  _outputFileNumber(0)
{
  _result = NULL;
}

PtResult::~PtResult()
{
  _result = NULL;
}

void PtResult::operator=(const SdmBoolean s)
{
  if (!_displayResults)
    return;

  _result = (s == Sdm_True ? "true" : "false");
}

void PtResult::operator=(const SdmString& s)
{
  if (!_displayResults)
    return;

  _result = s;
}

void PtResult::operator=(const char* s)
{
  if (!_displayResults)
    return;

  _result = s;
}

void PtResult::operator=(const SdmStringL& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    _result += s[i];
    _result += "\n";
  }
}

void PtResult::operator=(const SdmStringLL& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    SdmStringL *&p = s[i];
    for (int j = 0; j < (*p).ElementCount(); j++) {
      _result += (*p)[j];
      _result += "\n";
    }
  }
}

void PtResult::operator=(SdmStrStr& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  _result = s.GetFirstString();
  _result += "< >";
  _result += s.GetSecondString();
}

void PtResult::operator=(SdmStrStrL& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    _result += s[i].GetFirstString();
    _result += "<->";
    _result += s[i].GetSecondString();
    _result += "|";
  }
}

void PtResult::operator=(SdmStrStrLL& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    SdmStrStrL *&sl = s[i];

    for (int j = 0; j < (*sl).ElementCount(); j++) {
      _result += (*sl)[j].GetFirstString();
      _result += "<=>";
      _result += (*sl)[j].GetSecondString();
      _result += "|";
    }
    _result += "<!>";
  }
}

void PtResult::operator=(SdmIntStr& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  char buf[32];
  int n = s.GetNumber();
  sprintf(buf, "%d", n);
  _result += buf;
  _result += "< >";
  _result += s.GetString();
}

void PtResult::operator=(SdmIntStrL& s)
{
  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    char buf[32];
    int n = s[i].GetNumber();
    sprintf(buf, "%d", n);
    _result += buf;
    _result += "<->";
    _result += s[i].GetString();
    _result += "|";
  }
}

void PtResult::operator=(SdmIntStrLL& s)
{
  if (!_displayResults)
    return;

  _result = NULL;

  for (int i = 0; i < s.ElementCount(); i++) {
    SdmIntStrL *&sl = s[i];

    for (int j = 0; j < (*sl).ElementCount(); j++) {
      char buf[32];
      int n = (*sl)[j].GetNumber();
      sprintf(buf, "%d", n);
      _result += buf;
      _result += "<=>";
      _result += (*sl)[j].GetString();
      _result += "|";
    }
    _result += "<!>";
  }
}

void PtResult::Reset()
{
  _result = "(No Result)";
}

void PtResult::Print()
{
  char filename[64];
  FILE *outputFile;

  if (_displayResults == Sdm_False)
    return;

  if (_intersperse || _result == "(No Result)") {
    printf("Result: %s\n", (const char*)_result);
    if (_result != "(No Result)") {
      // Display the command that would validate the above result
      ++_outputFileNumber;
      printf("port validate -d test %d -F golden%d.out\n",
             _outputFileNumber, _outputFileNumber);
    }
  }
  else {
    sprintf(filename, "golden%d.out", ++_outputFileNumber);
    outputFile = fopen(filename, "w");
    if (outputFile == NULL) {
      printf("?Cant create -F output file '%s': %s\n", outputFile, strerror(errno));
      return;
    }
    fwrite((const char *)_result, _result.Length(), 1, outputFile);
    fclose(outputFile);
  }
}

void PtResult::Validate(SdmString &testName, SdmString &validateString)
{
  const char *s = (const char*)testName;

  fprintf(stderr, "%s:\t%s\n", s, _result == validateString ? "Pass" : "Fail");
}
