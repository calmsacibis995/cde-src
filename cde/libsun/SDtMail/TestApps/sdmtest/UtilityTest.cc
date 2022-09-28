/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This contains the test for the expunging of deleted messages calls.

#include <unistd.h>
#include <stropts.h>
#include <poll.h>

#include "Main.hh"
#include <SDtMail/Connection.hh>
#include <SDtMail/Utility.hh>
#include <SDtMail/MailRc.hh>
#include <SDtMail/SystemUtility.hh>
#include <pwd.h>


SdmErrorCode RunUtilityTest()
{
  cout << endl << "************RunUtilityTest Invoked.*************" << endl << endl;
  cout << "======================================================" << endl;
  cout << endl;

  SdmError err;
  SdmString kServiceOption("serviceoption");
  SdmString kServiceType("servicetype");
  SdmString kServiceClass("serviceclass");

  SdmConnection *mcon  = new SdmConnection("TestSession");

  if (mcon == NULL) {
    cout << "*** Error: Sdm Connection Creation Failed\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (mcon->StartUp(err)) {
    cout << "*** Error: Sdm Connection StartUp Failed\n" ;
    return err;
  }

  SdmMailRc *mailRc;
  // create mailRc
  if (mcon->SdmMailRcFactory(err, mailRc)) {
    cout << "*** Error: create mailrc failed\n";
    return err;
  }

  // Test object key methods.
  SdmObjectKey key = SdmUtility::NewObjectKey();
  if (!SdmUtility::ValidObjectKey(key)) {
    cout << "*** Error: valid key not found.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmObjectKey key2 = SdmUtility::NewObjectKey();
  if (key == key2) {
    cout << "*** Error: NewObjectKey returns same key.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  SdmUtility::RemoveObjectKey(key);
  if (SdmUtility::ValidObjectKey(key)) {
    cout << "*** Error: key found after it's been removed.\n";
    err = Sdm_EC_Fail;
    return err;
  }

  // test LogError
  SdmUtility::LogError(Sdm_True, "This is a critical error. arg=%s\n", "foo");
  SdmUtility::LogError(Sdm_False, "This is a warning error. arg=%s\n", "bar");
  
  
  // test ExpandPath and GetRelativePath.
  //

  // setup mailrc resources.
  SdmMailRc *mailrc = SdmMailRc::GetMailRc();
  if (mailrc == NULL) {
    cout << "*** Error: can't get mailrc\n";
    err = Sdm_EC_Fail;
    return err;
  }
  mailrc->SetValue(err, SdmUtility::kFolderResourceName, "/home/foobar/MailboxFolder");
  if (err) {
    cout << "*** Error: can't set folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  mailrc->SetValue(err, SdmUtility::kImapFolderResourceName, "/home/foobar/ImapMailboxFolder");
  if (err) {
    cout << "*** Error: can't set imap folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }

  passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);
  char defaultPath[256];
  memset(&defaultPath, 0, 256);
  sprintf(defaultPath, "%s/Mail/mailFile", pw.pw_dir);
  cout << "running as " << pw.pw_name << "\n";
  
  char* r_path;
  if (SdmUtility::ExpandPath(err, r_path,
                        "+mailFile", *mailrc, SdmUtility::kFolderResourceName))
  {
    cout << "*** Error: ExpandPath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  
  if (strcmp(r_path, "/home/foobar/MailboxFolder/mailFile") != 0) {
    cout << "*** Error: ExpandPath returns wroing path using folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);
  

  if (SdmUtility::ExpandPath(err, r_path,
      "+mailFile", *mailrc, SdmUtility::kImapFolderResourceName))
  {
    cout << "*** Error: ExpandPath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (strcmp(r_path, "/home/foobar/ImapMailboxFolder/mailFile") != 0) {
    cout << "*** Error: ExpandPath returns wroing path using folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);

  if (SdmUtility::ExpandPath(err, r_path,
      "+mailFile", *mailrc, NULL))
  {
    cout << "*** Error: ExpandPath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (strcmp(r_path, defaultPath) != 0) {
    cout << "*** Error: ExpandPath returns wroing path using null resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);

  if (SdmUtility::GetRelativePath(err, r_path,
      "/home/foobar/MailboxFolder/mailFile", *mailrc, SdmUtility::kFolderResourceName))
  {
    cout << "*** Error: ExpandPath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (strcmp(r_path, "mailFile") != 0) {
    cout << "*** Error: GetRelativePath returns wroing path using folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);

  if (SdmUtility::GetRelativePath(err, r_path,
      "/home/foobar/ImapMailboxFolder/mailFile", *mailrc, SdmUtility::kImapFolderResourceName))
  {
    cout << "*** Error: GetRelativePath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (strcmp(r_path, "mailFile") != 0) {
    cout << "*** Error: GetRelativePath returns wroing path using folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);

  if (SdmUtility::GetRelativePath(err, r_path,
      defaultPath, *mailrc, NULL))
  {
    cout << "*** Error: GetRelativePath fail\n";
    err = Sdm_EC_Fail;
    return err;
  }
  if (strcmp(r_path, "mailFile") != 0) {
    cout << "*** Error: GetRelativePath returns wroing path using folder resource\n";
    err = Sdm_EC_Fail;
    return err;
  }
  free(r_path);


  return err;
}



