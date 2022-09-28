/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */
 
// --> This file contains the test for SdmSearch object.
 
#pragma ident "@(#)SearchTest.cc	1.15 96/07/09 SMI"

#include <unistd.h>
#include <stropts.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Main.hh"
#include <SDtMail/Connection.hh>
#include <SDtMail/Session.hh>
#include <SDtMail/MessageStore.hh>
#include <SDtMail/Token.hh>

static  const char* kOutgoingStoreName = "/tmp/search.test";

class SdmSearchTest
{
public:
  SdmErrorCode Run();

protected:
  SdmErrorCode Setup(SdmError& err);
  SdmErrorCode Cleanup(SdmError& err);

  SdmErrorCode TestSearch(SdmError &err, 
           SdmSearch &search, int expected_count) ;
  SdmErrorCode TestBadSearch(SdmError &err, 
           SdmSearch &search, SdmErrorCode expected_error) ;

  void PrintMessageNumbers(SdmMessageNumberL &list);
  SdmErrorCode PrintSearchLeaf(SdmError &err, SdmSearch& search);

  SdmErrorCode TestSearchCalls(SdmError &err);
  SdmErrorCode TestAtomicSearchCalls(SdmError &err);
  SdmErrorCode TestNotCalls(SdmError &err);
  SdmErrorCode TestMessageCalls(SdmError &err);
  SdmErrorCode TestDynamicDateCalls(SdmError &err);
  SdmErrorCode TestBadCalls(SdmError &err);
  SdmErrorCode TestLeafPrinting(SdmError &err);

private:
  SdmConnection *mcon;
  SdmMessageStore *mbox;
  SdmSession *ises;
 
  int i_readFileDesc;
  SdmReplyQueue *i_frontEndQueue;
};

  

SdmErrorCode RunSearchTest()
{
  cout << endl << "************RunSearchTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;
  SdmSearchTest test;
  return test.Run();
}


SdmErrorCode
SdmSearchTest::Run()
{
  SdmError err;
  err = Sdm_EC_Success;

  if (Setup(err)) {
    return err;
  }
 
  if (TestSearchCalls(err)) {
    return err;
  }

  if (TestAtomicSearchCalls(err)) {
    return err;
  }

  if (TestNotCalls(err)) {
    return err;
  }
  
  if (TestMessageCalls(err)) {
    return err;
  }

  if (TestBadCalls(err)) {
    return err;
  }

  if (TestLeafPrinting(err)) {
    return err;
  }

  if (Cleanup(err)) {
    return err;
  }
  return err;
}

SdmErrorCode
SdmSearchTest::Setup(SdmError &err)
{
  SdmMessageNumber nmsgs;
  SdmBoolean readOnly; 

  // first, create out temp store.
  unlink(kOutgoingStoreName);
  char command[256];
  sprintf(command, "/bin/cp search.test %s", kOutgoingStoreName);
  system(command);
  mode_t mode = (S_IRUSR | S_IWUSR   | S_IWGRP  |  S_IRGRP);
  if (chmod(kOutgoingStoreName, mode)) {
    cout << "*** Error: can't set permissions on test mailbox.\n";
    err = Sdm_EC_Fail;
    return err;
  }


  SdmString kServiceOption("serviceoption");
  SdmString kServiceType("servicetype");
  SdmString kServiceClass("serviceclass");

  mcon  = new SdmConnection("TestSession");

  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }

  // create and start up incoming session.
  if (mcon->SdmSessionFactory(err, ises, Sdm_ST_InSession)) {
    cout << "*** Error: create incoming session failed\n";
    return err;
  }
  if (ises->StartUp(err)) {
    cout << "*** Error: Sdm Incoming Session StartUp Failed\n" ;
    return err;
  }

  // attach to session.
  void *vptr;
  if (SdmAttachSession(err, i_readFileDesc, vptr, *ises)) {
    cout << "*** Error: attach incoming session failed.\n";
    return err;
  }
  i_frontEndQueue = (SdmReplyQueue*)vptr;

  // create and start up message store.
  if (ises->SdmMessageStoreFactory(err, mbox)) {
     cout << "*** Error: create message store failed\n";
     return err;
  }
  if (mbox->StartUp(err)) {
    cout << "*** Error: Message store StartUp Failed\n" ;
    return err;
  }

  // open the message stores.
  SdmToken token;
  token.SetValue("serviceclass","messagestore");
  token.SetValue("servicetype","local");
  token.SetValue("serviceoption","messagestorename", kOutgoingStoreName);

  if (mbox->Open(err, nmsgs, readOnly, token)) {
    cout << "*** Error: Open failed.\n";
    return err;
  }

  return err;
}


SdmErrorCode
SdmSearchTest::TestSearchCalls(SdmError &err)
{
  err = Sdm_EC_Success;
  SdmSearch search;
  SdmString search_str;

  // test no search criteria.  should return all messages.
  cout << "*** Info: searching with no criteria \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test empty string.  should return all messages.
  search_str = "";
  cout << "*** Info: searching with empty string\n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test ALL.  should return all messages.
  search_str = "ALL";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test FROM
  search_str = "FROM \"Sara Swanson\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 2)) {
    return err;
  }

  // test TO
  search_str = "TO xsun-perf";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 19)) {
    return err;
  }

  // test AND
  search_str = "TO xsun-perf AND FROM John";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 5)) {
    return err;
  }
 
  // test OR
  search_str = "TO xsun-perf OR FROM \"Robert Allen\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 21)) {
    return err;
  }

  // test for bug 4010144.  try different combinations or AND and OR.
  search_str = "subject kim or subject block or (from zittin and subject modem)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 4)) {
    return err;
  }

  search_str = "subject kim or (from zittin and subject modem) or subject block";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 4)) {
    return err;
  }

  search_str = "(from zittin and subject modem) or subject kim or subject block";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 4)) {
    return err;
  }

  // test CC
  search_str = "CC sandig@yoyo";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 2)) {
    return err;
  }

  // test BODY
  search_str = "body feedback";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 5)) {
    return err;
  }

  // test ON
  search_str = "on 01-DEC-95";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 21)) {
    return err;
  }

  // test DURING (with specific date.  should returnj same result as ON)
  search_str = "during 01-DEC-95";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 21)) {
    return err;
  }

  search_str = "during july";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  search_str = "during \"june\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  search_str = "during thismonth";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  // test BEFORE
  search_str = "before 30-nov-1995";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 38)) {
    return err;
  }

  // test SENTBEFORE
  search_str = "sentbefore 28-nov-1995";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 3)) {
    return err;
  }

  // test SENTON
  search_str = "senton 08-dec-1995";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 4)) {
    return err;
  }

  // test SENTSINCE
  search_str = "sentsince 06-dec-1995";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 48)) {
    return err;
  }

  // test LARGER
  search_str = "larger 10000";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 9)) {
    return err;
  }

  // test SMALLER
  search_str = "smaller 1000";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 10)) {
    return err;
  }


  // test TEXT
  search_str = "text \"Netscape\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 6)) {
    return err;
  }

  // test BODY
  search_str = "body \"car\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 36)) {
    return err;
  }

  // test HEADER
  search_str = "header subject assertions";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 1)) {
    return err;
  }

  // test HEADER with quoted value
  search_str = "header subject \"remove me\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 2)) {
    return err;
  }

  // test HEADER with quoted key and value
  search_str = "header \"subject\" \"homepage\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 1)) {
    return err;
  }

  // test HEADER with non-standard search header
  search_str = "header Return-Path \"<shriver\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 3)) {
    return err;
  }

   return err;
}

// 
// The test cases below are here to exercise the functionality of using dynamic
// fields for the date related tokens (eg. January, Today, ThisWeek, etc)
// The following calls ALL need better test cases because the mail in the
// test mail box are from last year.  using the dynamic fields (January) 
// will just give you all the messages as the search result.  it's difficult
// to get a good test case for this because with the current date changing,
// the results will vary.
//
SdmErrorCode
SdmSearchTest::TestDynamicDateCalls(SdmError &err)
{
  err = Sdm_EC_Success;
  SdmSearch search;
  SdmString search_str;

  // test months 
  // need better test case.
  search_str = "before june";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test months (borderline case)
  // need better test case.
  search_str = "before january";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test months (borderline case)
  // need better test case.
  search_str = "before december";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test month (last month)
  // need better test case.
  search_str = "before lastmonth";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test month (this month)
  // need better test case.
  search_str = "before thismonth";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test week (this week)
  // need better test case.
  search_str = "before thisweek";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test day (today)
  // need better test case.
  search_str = "before today";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test day (yesterday)
  // need better test case.
  search_str = "before yesterday";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test day (week day)
  // need better test case.
  search_str = "before tuesday";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test day (borderline case)
  // need better test case.
  search_str = "before saturday";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test day (borderline case)
  // need better test case.
  search_str = "before sunday";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }
  
  // test DURING (this month)
  // need better test case.
  search_str = "during thismonth";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test DURING (last month)
  // need better test case.
  search_str = "during lastmonth";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test DURING (month)
  // need better test case.
  search_str = "during march";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test DURING (borderline case)
  // need better test case.
  search_str = "during january";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test DURING (borderline case)
  // need better test case.
  search_str = "during december";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }
  
  return err;
}


SdmErrorCode
SdmSearchTest::TestAtomicSearchCalls(SdmError &err)
{
  err = Sdm_EC_Success;
  
  SdmSearch search;
  SdmString search_str;

  // test READ
  search_str = SdmSearch::Sdm_STK_Read;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 147)) {
    return err;
  }

  // test UNREAD
  search_str = SdmSearch::Sdm_STK_Unread;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 60)) {
    return err;
  }

  // test SEEN
  // this should be the same as READ.
  search_str = SdmSearch::Sdm_STK_Seen;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 147)) {
    return err;
  }

  // test UNSEEN
  // this should be the same as UNREAD.
  search_str = SdmSearch::Sdm_STK_Unseen;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 60)) {
    return err;
  }
  
  // test NEW
  // need better test case here.
  search_str = SdmSearch::Sdm_STK_New;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  // test OLD
  // need better test case here.
  search_str = SdmSearch::Sdm_STK_Old;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test RECENT
  // need better test case here.
  search_str = SdmSearch::Sdm_STK_Recent;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  // test FLAGGED
  search_str = SdmSearch::Sdm_STK_Flagged;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 7)) {
    return err;
  }

  // test UNFLAGGED
  search_str = SdmSearch::Sdm_STK_Unflagged;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 200)) {
    return err;
  }

  // test DRAFT
  search_str = SdmSearch::Sdm_STK_Draft;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 11)) {
    return err;
  }

  // test UNDRAFT
  search_str = SdmSearch::Sdm_STK_Undraft;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 196)) {
    return err;
  }

  // test ANSWERED
  // need better test case here.
  search_str = SdmSearch::Sdm_STK_Answered;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 0)) {
    return err;
  }

  // test UNANSWERED
  // need better test case here.
  search_str = SdmSearch::Sdm_STK_Unanswered;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 207)) {
    return err;
  }

  // test DELETED
  search_str = SdmSearch::Sdm_STK_Deleted;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 1)) {
    return err;
  }

  // test UNDELETED
  search_str = SdmSearch::Sdm_STK_Undeleted;
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 206)) {
    return err;
  }

  return err;
}

SdmErrorCode
SdmSearchTest::TestNotCalls(SdmError &err)
{
  SdmSearch search;
  SdmString search_str;

  // test BODY
  search_str = "body \"lobby\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 4)) {
    return err;
  }
  
  // test NOT
  search_str = "~body \"lobby\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 203)) {	// should equal total-4
    return err;
  }
 
  // more testing for NOT
  search_str = "subject \"dtmail\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 9)) {
    return err;
  }
 
  // the following cases tests a bug that was in the original Roam source.
  // it use to ignore the not and return the same results as 
  // "subject dtmail" (above case).
  search_str = "~(subject \"dtmail\")";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 198)) {
    return err;
  }

  search_str = "subject \"dtmail\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 9)) {
    return err;
  }

  // testing case using OR with two expressions
  search_str = "subject oneverylongstupidstring or ~(subject \"dtmail\")";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 198)) {
    return err;
  }


  search_str = "subject \"dtmail\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 9)) {
    return err;
  }

  // testing case using OR with three expressions
  search_str = "subject oneverylongstupidstring or subject twolongstring or ~(subject \"dtmail\")";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 198)) {
    return err;
  }

  search_str = "subject \"dtmail\"";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 9)) {
    return err;
  }

  // testing case using OR with four expressions
  search_str = "subject oneverylongstupidstring or subject twolongstring or subject threelongstring or ~(subject dtmail)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 198)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  search_str = "~(subject dtmail or subject lights)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 197)) {
    return err;
  }

  // testing case using NOT together with ANDed expressions
  search_str = "~(subject \"dtmail\" and subject \"digests\")";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 205)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  // (contains start/end expression, not bound to expression,
  // and not bound to second atom.
  search_str = "~(~subject dtmail or subject digests)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 7)) {
    return err;
  }

  // reverse predicates in expression.  result should be the same.
  search_str = "~(subject digests or ~subject dtmail)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 7)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  // (contains start/end expression, intermediate expression,
  // not bound to expression,
  search_str = "~(subject dtmail or subject first or subject lights)";
  cout << "*** Info: searching " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestSearch(err, search, 195)) {
    return err;
  }
   
  return err;
}


SdmErrorCode
SdmSearchTest::TestMessageCalls(SdmError &err)
{
  SdmSearch search;
  SdmString search_str;
  SdmMessageNumberL search_results;

  err = Sdm_EC_Success;

  // test PerformSearch given start and end msgnums.
  search_str = "subject \"dtmail\"";
  cout << "*** Info: searching with range \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  
  SdmMessageNumberL  correctResults;
  correctResults.AddElementToList(119);  // should find this one
  correctResults.AddElementToList(18);   // should find this one
  correctResults.AddElementToList(128);   // should find this one
  correctResults.AddElementToList(131);   // should find this one
  correctResults.AddElementToList(137);   // should find this one

  cout << "*** Info: executing SdmMessageStore search.\n";
  if (mbox->PerformSearch(err, search_results, search, 1, 150)) {
    cout << "*** Error: PerformSearch2 failed.\n";
    return err;
  }
 
  int count = search_results.ElementCount();
  if (count != correctResults.ElementCount()) {
    cout << "*** Error: search returned invalid number of messages.\n";
    err = Sdm_EC_Fail;
    PrintMessageNumbers(search_results);
    return err;
  }
  
  for (int i= 0; i< count; i++) {
    if (search_results.FindElement(correctResults[i]) < 0) {
      cout << "*** Error: Incorrect results from search.\n";
      err = Sdm_EC_Fail;
      PrintMessageNumbers(search_results);
      return err;
    }
  }

  search_results.ClearAllElements();
  correctResults.ClearAllElements();

  // test PerformSearch given list of numbers.
  cout << "*** Info: searching with list of numbers \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  
  SdmMessageNumberL  msgsToCheck;
  msgsToCheck.AddElementToList(192);  // should find this one
  msgsToCheck.AddElementToList(198);
  msgsToCheck.AddElementToList(200);  // should find this one
  msgsToCheck.AddElementToList(207);
  msgsToCheck.AddElementToList(100);
  msgsToCheck.AddElementToList(119);  // should find this one
  msgsToCheck.AddElementToList(18);   // should find this one
  
  correctResults.AddElementToList(192);  // should find this one
  correctResults.AddElementToList(200);  // should find this one
  correctResults.AddElementToList(119);  // should find this one
  correctResults.AddElementToList(18);   // should find this one
  
  cout << "*** Info: executing SdmMessageStore search.\n";
  if (mbox->PerformSearch(err, search_results, search, msgsToCheck)) {
    cout << "*** Error: PerformSearch3 failed.\n";
    return err;
  }
 
  count = search_results.ElementCount();
  if (count != correctResults.ElementCount()) {
    cout << "*** Error: search returned invalid number of messages.\n";
    err = Sdm_EC_Fail;
    PrintMessageNumbers(search_results);
    return err;
  }

  for (i= 0; i< count; i++) {
    if (search_results.FindElement(correctResults[i]) < 0) {
      cout << "*** Error: Incorrect results from search.\n";
      err = Sdm_EC_Fail;
      PrintMessageNumbers(search_results);
      return err;
    }
  }  

  SdmBoolean found = Sdm_False;
  
  // test PerformSearch that takes single msgno.
  if (mbox->PerformSearch(err, found, search, 154)) {
    cout << "*** Error: PerformSearch4 failed.\n";
    return err;
  }
  
  if (!found) {
    cout << "*** Error: PerformSearch could not find single message.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // test case when message not found.
  if (mbox->PerformSearch(err, found, search, 100)) {
    cout << "*** Error: PerformSearch4 failed.\n";
    return err;
  }
  
  if (found) {
    cout << "*** Error: PerformSearch could not find single message.\n";
    err = Sdm_EC_Fail;
    return err;
  }
     
  return err;
}




SdmErrorCode
SdmSearchTest::TestBadCalls(SdmError &err)
{
  SdmSearch search;
  SdmString search_str;
  SdmMessageNumberL search_results;

  err = Sdm_EC_Success;
  
  // test bad token
  search_str = "badtoken foo";
  cout << "*** Info: searching with bad token \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalSearchToken)) {
    return err;
  }

  // test missing boolean
  search_str = "subject foo subject bar";
  cout << "*** Info: searching with missing boolean op \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_ExpectedBoolean)) {
    return err;
  }

  // test missing predicate after boolean op
  search_str = "subject foo and";
  cout << "*** Info: searching with missing predicate \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_MissingPredicate)) {
    return err;
  }

  // test missing field
  search_str = "subject";
  cout << "*** Info: searching with missing field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_MissingField)) {
    return err;
  }

  // test missing double quote
  search_str = "subject \"hi";
  cout << "*** Info: searching with missing double quote \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_MissingDoubleQuote)) {
    return err;
  }

  // test illegal left parenthesis field
  search_str = "subject ( hi bert how are you )";
  cout << "*** Info: searching with illegal left paren field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalLeftParenthesisField)) {
    return err;
  }

  // test illegal right parenthesis token
  search_str = "(subject x and subject) hi";
  cout << "*** Info: searching with illegal right paren token \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalRightParenthesisToken)) {
    return err;
  }

  // test illegal  key field
  search_str = "header \"two words\" assertions";
  cout << "*** Info: searching with illegal key field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalKeyField)) {
    return err;
  }

  // test illegal left parenthesis key field
  search_str = "header (subject assertions)";
  cout << "*** Info: searching with illegal left paren key field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalLeftParenthesisKeyField)) {
    return err;
  }

  // test illegal right parenthesis key field
  search_str = "(header subject) assertions";
  cout << "*** Info: searching with illegal right paren key field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalRightParenthesisKeyField)) {
    return err;
  }

  // test illegal numeric field
  search_str = "smaller abc";
  cout << "*** Info: searching with illegal numeric field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalNumericField)) {
    return err;
  }

  // test illegal numeric field
  search_str = "smaller 123abc";
  cout << "*** Info: searching with illegal numeric field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalNumericField)) {
    return err;
  }

  // test illegal date field
  search_str = "senton abc";
  cout << "*** Info: searching with illegal date field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalDateField)) {
    return err;
  }

  // test illegal date field
  search_str = "senton thisweek";
  cout << "*** Info: searching with illegal date field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalDateField)) {
    return err;
  }

  // test illegal not operator before field.
  search_str = "from ~ foobar";
  cout << "*** Info: searching with invalid not operator before field \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (TestBadSearch(err, search, Sdm_EC_PRS_IllegalNotBeforeField)) {
    return err;
  }
  return err;
}

SdmErrorCode
SdmSearchTest::TestLeafPrinting(SdmError &err)
{
  SdmSearch search;
  SdmString search_str;

  // test FROM (simple quoted field)
  search_str = "FROM \"Sara Swanson\"";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

 // test TO (simple non-quoted field)
  search_str = "TO xsun-perf";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // test AND (logic_op = AND)
  search_str = "TO xsun-perf AND FROM John";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
 
  // test OR  (logic_op = OR)
  search_str = "TO xsun-perf OR FROM \"Robert Allen\"";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
  
  // test date  (dynamic field)
  search_str = "before 30-nov-1995";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
  
  // test SMALLER (numeric field)
  search_str = "smaller 1000";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // test READ (simple field)
  search_str = "read";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // testing case using NOT with simple expression
  // (contains simple expression, not bound to atom.
  search_str = "~subject dtmail";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
  
  // testing case using NOT with simple expression
  // (contains simple expression, not bound to expression.
  search_str = "~(subject dtmail)";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
    
  
  // testing case using NOT together with ORed expressions
  // (contains start/end expression, not bound to expression.
  search_str = "~(subject dtmail or subject lights)";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  // (contains start/end expression, not bound to expression, 
  // and not bound to first atom.
  search_str = "~(subject dtmail or ~subject lights)";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  // (contains start/end expression, not bound to expression,
  // and not bound to second atom.
  search_str = "~(~subject dtmail or subject digests)";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }

  // testing case using NOT together with ORed expressions
  // (contains start/end expression, not bound to expression,
  // intermediate expression.
  search_str = "~(subject dtmail or subject car or subject lights)";
  cout << "*** Info: printing " << (const char*)search_str << " \n";
  if (search.SetSearchString(err, search_str)) {
    cout << "*** Error: can't set search string.\n";
    return err;
  }
  if (PrintSearchLeaf(err, search)) {
    return err;
  }
    
  return err;
}
  
SdmErrorCode
SdmSearchTest::Cleanup(SdmError &err)
{
  if (mbox->Close(err)) {
    cout << "*** Error: SdmMessageStore close failed.\n";
    return err;
  }

  if (mbox->ShutDown(err)) {
    cout << "*** Error: SdmMessageStore shutdown failed.\n";
    return err;
  }

  if (ises->ShutDown(err)) {
    cout << "*** Error: SdmSession shutdown failed.\n";
    return err;
  }

  if (mcon->ShutDown(err)) {
    cout << "*** Error: SdmConnection shutdown failed.\n";
    return err;
  }

  // clean up objects.
  // note: don't delete envelope or body.  they got deleted by message.
  // note: don't delete message.  it got deleted by session.
  // note: don't delete session.  it got deleted by connection.
  delete mcon;

  // Sessions should all be shut down now.  Call SdmHandleSessionEvent
  // one more time to cause the clean up of the front end queue and
  // closing of read file descriptor.
  //
  if (SdmHandleSessionEvent(i_frontEndQueue, &i_readFileDesc, NULL) >= 0) {
     cout <<  "*** SdmHandleSessionEvent does not fail after shutdown.\n";
     err = Sdm_EC_Fail;
     return err;
  }

  // clean up temp outgoing store.
  unlink(kOutgoingStoreName);
 
  return err;
}



SdmErrorCode
SdmSearchTest::TestSearch(SdmError &err, 
   SdmSearch &search, int expectedCnt) 
{
  err = Sdm_EC_Success;
  SdmMessageNumberL search_results;
 
  cout << "*** Info: executing SdmMessageStore search.\n";
  if (mbox->PerformSearch(err, search_results, search)) {
    
    cout << "*** Error: PerformSearch failed.\n";
    return err;
  }
 
  int count = search_results.ElementCount();
  if (count != expectedCnt) {
    cout << "*** Error: search returned invalid number of messages.\n";
    err = Sdm_EC_Fail;
    PrintMessageNumbers(search_results);
    cout << "   Message list count: " << search_results.ElementCount() << "\n";
    return err;
  }

  return err;
}


SdmErrorCode
SdmSearchTest::TestBadSearch(SdmError &err, 
   SdmSearch &search, SdmErrorCode expectedError) 
{
  err = Sdm_EC_Success;
  SdmMessageNumberL search_results;
 
  cout << "*** Info: executing SdmMessageStore search.\n";
  if (mbox->PerformSearch(err, search_results, search)) {
    if ( (!err.MinorCount() && err == expectedError) 
	 || (err.MinorCount() == 1 && err == Sdm_EC_SearchStringParseFailure && err[0] == expectedError) ) {
      err = Sdm_EC_Success;
    } else {
      cout << "*** Error: search returned invalid error: (" << (SdmErrorCode) err << ") " << err.ErrorMessage() << "\n";
      if (err.MinorCount())
	for (int i = 0; i < err.MinorCount(); i++)
	  cout << "*** minor error code (" << (SdmErrorCode)err[i] << ") " << err.ErrorMessage(i) << "\n";
    }
  } else {
    cout << "*** Error: sesarch did not return error for bad search.\n";
    err = Sdm_EC_Fail;
  }
  return err;
}
 

void
SdmSearchTest::PrintMessageNumbers(SdmMessageNumberL &list)
{
  int count = list.ElementCount();
  SdmMessageNumber msgno;
  cout << "*** Info: search results:\n";
  for (int i=0; i<count; i++) {
    msgno = list[i];
    cout << msgno << "\n";
  }
}


SdmErrorCode
SdmSearchTest::PrintSearchLeaf(SdmError &err, SdmSearch& search)
{
  SdmSearchLeaf *leaf;
  err = Sdm_EC_Success;
  
  if (search.ParseSearch(err, leaf)) {
    cout << "*** Error: ParseSearch failed.\n"; 
    return err;
  } else if (leaf == NULL) {
    cout << "*** Error: ParseSearch returns null leaf.\n"; 
    err = Sdm_EC_Fail;
    return err;
  } else {
    search.PrintLeaf(leaf);
    search.FreeLeaf(leaf);
  }
  return err;
}
