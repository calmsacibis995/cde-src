#include <porttest.hh>
#include <ptcoverview.hh>

char *Hoverview = "Usage: overview\ngive quick overview of data port\n";

/*
** FORWARD DECLARATIONS
*/

void TestSdmErrorObject();
void TestSdmStringObject();


/*************************************************************************
 *
 * Function:	Coverview - give quick overview of data port
 *
 * Input:	int pargc - number of arguments passed to command
 *		char **pargv - array of pargc arguments passed
 *
 *		IF number of arguments == 0, assume we are being executed
 *		as a single command line argument.
 *
 * Output:	TRUE - user aborted quit, return to command mode
 *		FALSE - user confirmed quit, exit program
 *
 ************************************************************************/

int Coverview(int argc, char **argv)
{
  assert(argc >= 0);

  SdmError err;
  SdmToken tk;
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly;

  TestSdmErrorObject();

  TestSdmStringObject();
  
  printf(">> Overview of token object:\n");
  printf(">> Token object length = %d\n", sizeof(tk));

  printf(">> Overview of error object:\n");
  printf(">> Error object size = %d\n", sizeof(err));

  printf(">> Overview of current data port:\n");
  SdmDataPort dp(0);

  printf(">> Data port object length = %d\n", sizeof(dp));
  dp.StartUp(err);
  if (err) {
    printf(">> ERROR: could not start up data port object, err = %u\n", (SdmErrorCode) err);
    return(TRUE);
  }
  
  tk.SetValue("servicetype", "test");
  tk.SetValue("serviceoption", "messagestorename", "test.mbx");
  dp.Open(err, nmsgs, readOnly, tk);
  printf(">> open returned, err = %u, readonly = %u\n", (SdmErrorCode) err, (unsigned long) readOnly);
  
  SdmMessageStoreStatus storestatus;
  dp.GetMessageStoreStatus(err, storestatus, Sdm_MSS_ALL);
  if (err) {
    printf(">> ERROR: could not get store status, err = %u\n", (SdmErrorCode) err);
  }
  else {
  printf(">> Test mailbox successfully opened.\n\tflags = 0x%08lx, uidvalidity = 0x%08lx, checksum = %u, checksumbytes = %u\n"
	 "\tmessages = %d, recent = %d, unseen = %d, uidnext = %d\n",
	 storestatus.flags, storestatus.uidvalidity, storestatus.checksum, storestatus.checksum_bytes,
	 storestatus.messages, storestatus.recent, storestatus.unseen, storestatus.uidnext);
  }

  dp.ShutDown(err);
  if (err) {
    printf(">> ERROR: could not shut down data port object, err = %u\n", (SdmErrorCode) err);
    return(TRUE);
  }
  
  return(TRUE);
  
  /*NOTREACHED*/
}

void TellAboutError(SdmError& err)
{
  assert(err.ErrorCode() == (SdmErrorCode) err);
  printf("main code '%d' message '%s'\n", err.ErrorCode(), err.ErrorMessage());
  int nm = err.MinorCount();
  printf("minor count '%d'\n", nm);
  for (int i = 0; i < nm; i++) {
    assert(err.ErrorCode(i) == (SdmErrorCode) err[i]);
    printf("  minor error %d: code '%d' message '%s'\n", i, err.ErrorCode(i), err.ErrorMessage(i));
  }
}

void TestSdmErrorObject()
{
const char* successErrorMessage = "Operation successful.\n";
const SdmErrorCode firstError = Sdm_EC_Cancel;
const SdmErrorCode secondError = Sdm_EC_Closed;
const SdmErrorCode thirdError = Sdm_EC_Attached;
const SdmErrorCode fourthError = Sdm_EC_Open;

  printf(">> Testing SdmError object\n");

  assert(!Sdm_EC_Success);
  assert(Sdm_EC_Fail);

  SdmError e1;
  SdmError e2;
  int i1,i2;

  // First make sure the initial state of the error container is correct

  assert(e1 == Sdm_EC_Success); assert(e1.ErrorCode() == Sdm_EC_Success); assert(!::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 0); assert(e1.ErrorCode(0) == Sdm_EC_Fail); assert(e1[0] == Sdm_EC_Fail); 

  // Now set the major error code and see if it ripples only so far

  e1 = firstError;
  assert(e1 == firstError); assert(e1.ErrorCode() == firstError); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 0); assert(e1.ErrorCode(0) == Sdm_EC_Fail); assert(e1[0] == Sdm_EC_Fail);
  assert(e1.IsErrorContained(firstError));
  assert(!e1.IsErrorContained(secondError));

  // Add a minor error code and see if it gets set properly

  e1.AddMinorErrorCode(secondError);
  assert(e1 == firstError); assert(e1.ErrorCode() == firstError); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 1); assert(e1.ErrorCode(1) == Sdm_EC_Fail); assert(e1[1] == Sdm_EC_Fail);
  assert(e1.ErrorCode(0) == secondError); assert(e1[0] == secondError);

  // Add another minor error code and see if it gets set properly

  e1.AddMinorErrorCode(thirdError);
  assert(e1 == firstError); assert(e1.ErrorCode() == firstError); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 2); assert(e1.ErrorCode(2) == Sdm_EC_Fail); assert(e1[2] == Sdm_EC_Fail);
  assert(e1.ErrorCode(0) == secondError); assert(e1[0] == secondError);
  assert(e1.ErrorCode(1) == thirdError); assert(e1[1] == thirdError);

  // Now push down a new major error and see if everything shifts ok

  e1.AddMajorErrorCode(fourthError);
  assert(e1 == fourthError); assert(e1.ErrorCode() == fourthError); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 3); assert(e1.ErrorCode(3) == Sdm_EC_Fail); assert(e1[3] == Sdm_EC_Fail);
  assert(e1.ErrorCode(0) == secondError); assert(e1[0] == secondError);
  assert(e1.ErrorCode(1) == thirdError); assert(e1[1] == thirdError);
  assert(e1.ErrorCode(2) == firstError); assert(e1[2] == firstError);

  // Reset the error container and make sure it collapses down

  e1 = Sdm_EC_Success;
  assert(e1 == Sdm_EC_Success); assert(e1.ErrorCode() == Sdm_EC_Success); assert(!::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 0); assert(e1.ErrorCode(0) == Sdm_EC_Fail); assert(e1[0] == Sdm_EC_Fail); 

  // Test adding a minor error without setting the major error

  e1.AddMinorErrorCode(secondError);
  assert(e1 == Sdm_EC_Fail); assert(e1.ErrorCode() == Sdm_EC_Fail); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 1); assert(e1.ErrorCode(1) == Sdm_EC_Fail); assert(e1[1] == Sdm_EC_Fail);
  assert(e1.ErrorCode(0) == secondError); assert(e1[0] == secondError);

  // Reset the error container and make sure it collapses down

  e1.Reset();
  assert(e1 == Sdm_EC_Success); assert(e1.ErrorCode() == Sdm_EC_Success); assert(!::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 0); assert(e1.ErrorCode(0) == Sdm_EC_Fail); assert(e1[0] == Sdm_EC_Fail); 

  // Test adding major error codes to see if they push down

  e1.AddMajorErrorCode(firstError);
  e1.AddMajorErrorCode(secondError);
  e1.AddMajorErrorCode(thirdError);
  e1.AddMajorErrorCode(fourthError);
  assert(e1 == fourthError); assert(e1.ErrorCode() == fourthError); assert(::strcmp(e1.ErrorMessage(),successErrorMessage));
  assert(e1.MinorCount() == 3); assert(e1.ErrorCode(3) == Sdm_EC_Fail); assert(e1[3] == Sdm_EC_Fail); 
  assert(e1.ErrorCode(0) == firstError); assert(e1[0] == firstError);
  assert(e1.ErrorCode(1) == secondError); assert(e1[1] == secondError);
  assert(e1.ErrorCode(2) == thirdError); assert(e1[2] == thirdError);
  assert(e1.IsErrorContained(firstError));
  assert(e1.IsErrorContained(secondError));
  assert(e1.IsErrorContained(thirdError));
  assert(e1.IsErrorContained(fourthError));

  // Test copying an error container to another

  e2 = e1;
  assert((SdmErrorCode)e1 == (SdmErrorCode)e2); assert(e1.ErrorCode() == e2.ErrorCode()); assert(!::strcmp(e1.ErrorMessage(),e2.ErrorMessage()));
  assert(e1.MinorCount() == e2.MinorCount());
  i1 = e1.MinorCount();
  for (i2 = 0; i2 < i1; i2++) {
    assert(e1.ErrorCode(i2) == e2.ErrorCode(i2)); assert((SdmErrorCode)e1[i2] == (SdmErrorCode)e2[i2]);
  }

  // Test setting an error message as a major code
  // Has intimate knowledge of the error message returned

  e1.Reset();
  e1.SetMajorErrorCodeAndMessage(Sdm_EC_MRC_CannotRenameNewMailruleFile, "A1", "B2", "C3");
  assert(e1 == Sdm_EC_MRC_CannotRenameNewMailruleFile); assert(e1.ErrorCode() == Sdm_EC_MRC_CannotRenameNewMailruleFile); 
  assert(!::strcmp(e1.ErrorMessage(), "Cannot rename new mail rule file over existing mail rule file.\nNew Mailrc file: A1\nExisting Mailrc file: B2\n\nC3\n"));
  assert(e1.MinorCount() == 0); assert(e1.ErrorCode(0) == Sdm_EC_Fail); assert(e1[0] == Sdm_EC_Fail);

  // I can't believe the tests passed, no? :-)

  e1.Reset();	// reset container before destruction to errorlogging not done if enabled
  e2.Reset();	// reset container before destruction to errorlogging not done if enabled

  printf(">> SdmError object passed.\n");
}

// Crude but effective test of string object
// Format: a < aa < ab < b
// [some unintelligible comment in French would be appropriate here]
// [How about: cest difficile a croire que cet algorithme marche non]
// [or how about: The French Language allows you to appear to be sophisticated,
// 		  when in fact vous avez des truffes a la place de cerveau!]
// 
void StringTester(const char *aA, const char *aaA, const char *abA, const char *bA)
{
  SdmString a0("");	assert(a0 == "");	assert(a0 == a0);	assert(!(a0 != a0));	assert(!(a0 != "")); assert(!a0); assert(a0.Length()==0);
  SdmString a(aA);	assert(a == aA);	assert(a == a);		assert(!(a != a));	assert(!(a != aA)); assert(a); assert(a.Length()!=0);
  SdmString a1(aA);	assert(a1 == aA);	assert(a1 == a1);	assert(!(a1 != a1));	assert(!(a1 != aA));
  SdmString aa(aaA);	assert(aa == aaA);	assert(aa == aa);	assert(!(aa != aa));	assert(!(aa != aaA));
  SdmString ab(abA);	assert(ab == abA);	assert(ab == ab);	assert(!(ab != ab));	assert(!(ab != abA));
  SdmString b(bA);	assert(b == bA);	assert(b == b);		assert(!(b != b));	assert(!(b != bA));
  SdmString bR(b);	assert(bR == bA);	assert(bR == bR);	assert(!(bR != bR));	assert(!(bR != bA)); assert(bR == b); assert(!(bR != b));
  SdmString aC(aA); aC+= aaA; assert(strncmp((const char *)aC, aA, strlen(aA))==0); assert(strcmp((const char *)aC+strlen(aA), aaA)==0);

  assert(a != "");	assert(a > "");		assert(a1 >= "");	assert(!(a <= ""));	assert(!(a < ""));
  assert(a1 != "");	assert(a1 > "");	assert(a1 >= "");	assert(!(a1 <= ""));	assert(!(a1 < ""));
  assert(aa != "");	assert(aa > "");	assert(aa >= "");	assert(!(aa <= ""));	assert(!(aa < ""));
  assert(ab != "");	assert(ab > "");	assert(ab > "");	assert(!(ab <= ""));	assert(!(ab < ""));
  assert(b != "");	assert(b > "");		assert(b > "");		assert(!(b <= ""));	assert(!(b < ""));
  assert(bR != "");	assert(bR > "");	assert(bR > "");	assert(!(bR <= ""));	assert(!(bR < ""));

  assert(a0 <= a0);	assert(a0 <= "");	assert(a0 >= a0);	assert(a0 >= "");
  assert(a <= a);	assert(a >= a);		assert(!(a < a));	assert(!(a > a));
  assert(a1 <= a1);	assert(a1 >= a1);	assert(!(a1 < a1));	assert(!(a1 > a1));
  assert(aa <= aa);	assert(aa >= aa);	assert(!(aa < aa));	assert(!(aa > aa));
  assert(ab <= ab);	assert(ab >= ab);	assert(!(ab < ab));	assert(!(ab > ab));
  assert(b <= b);	assert(b >= b);		assert(!(b < b));	assert(!(b > b));

  assert(a == a1);	assert(a1 == a);	assert(!(a != a1));	assert(!(a1 != a));
  assert(a != b);	assert(a != bA);	assert(!(a == b));	assert(!(a == bA));
  assert(a1 != ab);	assert(a1 != abA);	assert(!(a1 == ab));	assert(!(a1 == abA));
  assert(a1 != aa);	assert(a1 != aaA);	assert(!(a1 == aa));	assert(!(a1 == aaA));

  assert(a < b);	assert(a < bA);		assert(a <= b);		assert(a <= bA);	assert(!(a > b));	assert(!(a > bA));	assert(!(a >= b));	assert(!(a >= bA));
  assert(b > a);	assert(b > aA);		assert(b >= a);		assert(b >= aA);	assert(!(b < a));	assert(!(b < aA));	assert(!(b <= a));	assert(!(b <= aA));
  assert(a <= a1);	assert(a <= aA);	assert(a1 >= a);	assert(a1 >= aA);	assert(!(a < a1));	assert(!(a < aA));	assert(!(a > a1));	assert(!(a > aA));
}

void TestSdmStringObject()
{
  printf(">> Testing SdmString object\n");

  // Simple concatenation test

  SdmString xct1("dtmail ");
  SdmString xct2(" number");
  xct1 += "test ";
  xct1 += "version";
  xct1 += xct2;
  assert(strcmp((const char *)xct1,"dtmail test version number")==0);

  StringTester("a", "aa", "ab", "b");
  StringTester("aa", "aaa", "aab", "ba");
  StringTester("aaa", "aaaa", "aaab", "baa");
  StringTester("aaaa", "aaaaa", "aaaab", "baaa");
  StringTester("aaaaa", "aaaaaa", "aaaaab", "baaaa");
  StringTester("aaaaaaaaaa", "aaaaaaaaaaa", "aaaaaaaaab", "b");
  StringTester("a", "aa", "ab", "aaaaaaaaaaaaaaaaaab");
  StringTester("a", "aa", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaab", "b");
  StringTester("amock", "amuck", "amvck", "amwck");
  StringTester("0", "1", "2", "3");
  StringTester("a", "b", "c", "d");
  StringTester("xxxxxxxxxxxxxxxxxxxxa", "xxxxxxxxxxxxxxxxxxxxb", "xxxxxxxxxxxxxxxxxxxxc", "xxxxxxxxxxxxxxxxxxxxd");

  // Lets test the string find operator

  SdmString firstString;
  SdmString secondString;

  firstString = "xxxx";
  secondString = "y";

  assert(firstString.Find(secondString, 1) == -1);

  firstString = "xxxx";
  secondString = "x";

  assert(firstString.Find(secondString, 1) == 0);
  assert(firstString.Find(secondString, 2) == 1);
  assert(firstString.Find(secondString, 3) == 2);
  assert(firstString.Find(secondString, 4) == 3);
  assert(firstString.Find(secondString, 5) == -1);

  firstString = "xyxyxyxy";
  secondString = "x";
  assert(firstString.Find(secondString, 1) == 0);
  assert(firstString.Find(secondString, 2) == 2);
  assert(firstString.Find(secondString, 3) == 4);
  assert(firstString.Find(secondString, 4) == 6);
  assert(firstString.Find(secondString, 5) == -1);

  firstString = "xyxyxyxy";
  secondString = "xy";
  assert(firstString.Find(secondString, 1) == 0);
  assert(firstString.Find(secondString, 2) == 2);
  assert(firstString.Find(secondString, 3) == 4);
  assert(firstString.Find(secondString, 4) == 6);
  assert(firstString.Find(secondString, 5) == -1);

  firstString = "xyxyxyxy";
  secondString = "xyz";
  assert(firstString.Find(secondString, 1) == -1);

  firstString = "xyzxyxyzxy";
  secondString = "xyz";

  assert(firstString.Find(secondString, 1) == 0);
  assert(firstString.Find(secondString, 2) == 5);
  assert(firstString.Find(secondString, 3) == -1);

  firstString = "xyzxyxyxzyzxyz";
  secondString = "xyz";

  assert(firstString.Find(secondString, 1) == 0);
  assert(firstString.Find(secondString, 2) == 11);
  assert(firstString.Find(secondString, 3) == -1);

  firstString = "gggxyzgggxyzggg";
  secondString = "xyz";

  assert(firstString.Find(secondString, 1) == 3);
  assert(firstString.Find(secondString, 2) == 9);
  assert(firstString.Find(secondString, 3) == -1);

  firstString = "gggxyzgggxyzgggx";
  secondString = "xyz";

  assert(firstString.Find(secondString, 1) == 3);
  assert(firstString.Find(secondString, 2) == 9);
  assert(firstString.Find(secondString, 3) == -1);

  firstString = "gggxyzgggxyzgggxy";
  secondString = "xyz";

  assert(firstString.Find(secondString, 1) == 3);
  assert(firstString.Find(secondString, 2) == 9);
  assert(firstString.Find(secondString, 3) == -1);

  firstString = "xxxxABCDExxxx";
  secondString = "ABCDE";

  assert(firstString.Find(secondString, 1) == 4);
  assert(firstString.Find(secondString, 2) == -1);

  // Test various null string creations
  char *nsx = 0;
  SdmString ns0((char *)""), ns1((const char *)""), ns2, ns3((char *)0), ns4('\0', 0), ns5((const char *)"", 0), ns6(&nsx, 0, 0);
  assert(ns0.Length() == 0); assert(*(const char *)ns0 == '\0');
  assert(ns1.Length() == 0); assert(*(const char *)ns1 == '\0');
  assert(ns2.Length() == 0); assert(*(const char *)ns2 == '\0');
  assert(ns3.Length() == 0); assert(*(const char *)ns3 == '\0');
  assert(ns4.Length() == 0); assert(*(const char *)ns4 == '\0');
  assert(ns5.Length() == 0); assert(*(const char *)ns5 == '\0');
  assert(ns6.Length() == 0); assert(*(const char *)ns6 == '\0');

  // Lets test the US-ASCII collating sequence
  //
  const int maxlen = 2048;

  char a[maxlen+2];
  char b[maxlen+2];
  char c[maxlen+2];
  char d[maxlen+2];
  for (int i = 1; i < 127-4; i++) {
    if (UserInterrupt) {
      printf(">> Test interrupted - give up\n");
      break;
    }
    for (int j = 0; j < maxlen; j++) {
      printf("%03d %04d\r", i, j); fflush(stdout);
      ::memset(a, 'a', j);
      ::memset(b, 'a', j);
      ::memset(c, 'a', j);
      ::memset(d, 'a', j);
      a[j] = i;
      b[j] = i+1;
      c[j] = i+2;
      d[j] = i+3;
      a[j+1] = 0;
      b[j+1] = 0;
      c[j+1] = 0;
      d[j+1] = 0;
      StringTester(a, b, c, d);
      SdmString a0(a), a1(b), a2(c), a3(d);
#if 0
      a0.StripCharacter('a');
      a1.StripCharacter('a');
      a2.StripCharacter('a');
      a3.StripCharacter('a');
#endif
      StringTester((const char *)a0, (const char *)a1, (const char *)a2, (const char *)a3);
    }
  }

  printf(">> SdmString object passed.\n");
}
