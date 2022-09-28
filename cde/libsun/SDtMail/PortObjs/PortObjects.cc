/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Data Port Objects classes.
// --> Objects that are "part of the data port class" have their
// --> various methods in here

#pragma ident "@(#)PortObjects.cc	1.9 96/05/21 SMI"

// Include Files.
#include <iostream.h>
#include <assert.h>
#include <PortObjs/DataPort.hh>

// construct a new message structure 
// It should have a empty/null contents as it is to be filled in by the creator
//
SdmDpMessageStructure::SdmDpMessageStructure()
{
  // Note: SdmString fields are initialized by the SdmString constructor
  ms_type = Sdm_MSTYPE_none;		// Message type: no contents yet
  ms_encoding = Sdm_MSENC_other;	// Message encoding: unspecified
  ms_ms = (SdmDpMessageStructureL *)0;	// Message encapsulation: no encapsulated message
  ms_lines = 0;				// size in lines - none
  ms_bytes = 0; 			// size in bytes - none
  ms_disposition = Sdm_MSDISP_not_specified; // disposition of component: unspecified
  ms_unix_mode = 0;			// unix mode: unspecified
  ms_cached = Sdm_False;		// not cached
  ms_component = (SdmDpMessageComponent *)0;	// component identifier: unspecified
}

// destructor
SdmDpMessageStructure::~SdmDpMessageStructure()
{
  // Need to deallocate any data that is owned by this object; will recursively
  // decend the message structure tree if ms_ms is non-zero
  if (ms_component)
    delete ms_component;
  if (ms_ms) {
    ms_ms->ClearAndDestroyAllElements();
    delete ms_ms;
  }
}

// copy constructor
//
SdmDpMessageStructure::SdmDpMessageStructure(const SdmDpMessageStructure &msgstr)
{
  ms_type = msgstr.ms_type;
  ms_encoding = msgstr.ms_encoding;
  ms_subtype = msgstr.ms_subtype;
  ms_id = msgstr.ms_id;
  ms_description = msgstr.ms_description;
  ms_lines = msgstr.ms_lines;
  ms_bytes = msgstr.ms_bytes;
  ms_md5 = msgstr.ms_md5;
  ms_component = (msgstr.ms_component ? msgstr.ms_component->DeepCopy() : (SdmDpMessageComponent *)0);
  ms_disposition = msgstr.ms_disposition;
  ms_unix_mode = msgstr.ms_unix_mode;
  ms_attachment_name = msgstr.ms_attachment_name;
  ms_cached = msgstr.ms_cached;
  ms_ms = 0;

  // Dup any nested structure

  if (msgstr.ms_ms) {
    int numstrs = msgstr.ms_ms->ElementCount();
    ms_ms = new SdmDpMessageStructureL();
    for (int j = 0; j < numstrs; j++) {
      SdmDpMessageStructure *tmsgstr = (*msgstr.ms_ms)[j];
      assert(tmsgstr);
      SdmDpMessageStructure *dpms = new SdmDpMessageStructure(*tmsgstr);
      assert(dpms);
      ms_ms->AddElementToList(dpms);
    }
  }
}

// assignment operator
//
SdmDpMessageStructure& 
SdmDpMessageStructure::operator=(const SdmDpMessageStructure &rhs)
{
  // Check for assignment to self

  if (this == &rhs) return *this;

  // Copy the contents of the target object to this object

  ms_type = rhs.ms_type;
  ms_encoding = rhs.ms_encoding;
  ms_subtype = rhs.ms_subtype;
  ms_id = rhs.ms_id;
  ms_description = rhs.ms_description;
  ms_lines = rhs.ms_lines;
  ms_bytes = rhs.ms_bytes;
  ms_md5 = rhs.ms_md5;
  ms_disposition = rhs.ms_disposition;
  ms_unix_mode = rhs.ms_unix_mode;
  ms_attachment_name = rhs.ms_attachment_name;
  ms_cached = rhs.ms_cached;

  // Dup the component: existing one needs to be nuked first

  if (ms_component)
    delete ms_component;
  ms_component = (rhs.ms_component ? rhs.ms_component->DeepCopy() : (SdmDpMessageComponent *)0);

  // Dup any nested structure

  if (ms_ms) {
    // Nestest message nuke current contents
    delete ms_ms;
  }
  ms_ms = 0;

  if (rhs.ms_ms) {
    // Now make a copy of the existing 

    int numstrs = rhs.ms_ms->ElementCount();
    ms_ms = new SdmDpMessageStructureL();
    for (int j = 0; j < numstrs; j++) {
      SdmDpMessageStructure *tmsgstr = (*rhs.ms_ms)[j];
      assert(tmsgstr);
      SdmDpMessageStructure *dpms = new SdmDpMessageStructure(*tmsgstr);
      assert(dpms);
      ms_ms->AddElementToList(dpms);
    }
  }

  return *this;
}
