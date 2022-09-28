/*%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			 */
/*%%  (c) Copyright 1993, 1994 International Business Machines Corp.	 */
/*%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			 */
/*%%  (c) Copyright 1993, 1994 Novell, Inc. 				 */
/*%%  $Revision: $ 			 				 */
/*
 * tt_client_isam_file.h - Defines the TT ISAM file class.  This class
 * 	simplifies opening, closing, reading and writing an ISAM file.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#ifndef  _TT_CLIENT_ISAM_FILE_H
#define  _TT_CLIENT_ISAM_FILE_H

#include "util/tt_object.h"
#include "db/tt_db_client_utils.h"
#include "db/tt_client_isam_key_descriptor_utils.h"
#include "db/tt_client_isam_record_utils.h"
#include <sys/param.h>

class _Tt_client_isam_file : public _Tt_object {
public:
  // Dummy constructor needed to make _tt_client_isam_file_utils.cc happy
  _Tt_client_isam_file () {}

  // Real constructors
  _Tt_client_isam_file (const _Tt_string        &file,
		 int                      mode,
		 const _Tt_db_client_ptr &db_connection);
  _Tt_client_isam_file (const _Tt_string            &file,
		 int                          max_record_length,
		 int                          min_record_length,
		 _Tt_client_isam_key_descriptor_ptr  primary_key_descriptor,
		 int                          mode,
		 const _Tt_db_client_ptr     &db_connection);
  ~_Tt_client_isam_file ();

  void setErase (bool_t);

  int addIndex (_Tt_client_isam_key_descriptor_ptr);
  _Tt_client_isam_record_ptr getEmptyRecord ();

  int findStartRecord (const _Tt_client_isam_key_descriptor_ptr &key_descriptor,
		       int                                length,
		       const _Tt_client_isam_record_ptr         &record,
		       int                                mode);

  _Tt_client_isam_record_ptr readRecord (int mode);
  int                 readRecord (int mode, const _Tt_client_isam_record_ptr&);
  int                 updateRecord (long recnum, const _Tt_client_isam_record_ptr&);
  int                 writeRecord (const _Tt_client_isam_record_ptr&);
  int                 deleteRecord (long                              recnum,
				    const _Tt_client_isam_record_ptr &record);

  int        writeMagicString (const _Tt_string&);
  _Tt_string readMagicString ();

  long getLastRecordNumber () const
    {
      return currentRecordNumber;
    }

  int getFileDescriptor () const
    {
      return fileDescriptor;
    }

  const _Tt_string &getName () const
    {
      return fileName;
    }

  int getErrorStatus () const
    {
      return errorStatus;
    }

  bool_t isNew () const
    {
      return newFlag;
    }

private:
  int                              currentRecordLength;
  long                             currentRecordNumber;
  _Tt_db_client_ptr                dbConnection;
  bool_t		           eraseFlag;
  int                              errorStatus;
  int                              fileDescriptor;
  _Tt_string                       fileName;
  int                              fileMode;
  _Tt_client_isam_key_descriptor_list_ptr keyDescriptorList;
  int                              maxRecordLength;
  int                              minRecordLength;
  bool_t		           newFlag;

  void		      setTtISAMFileDefaults ();
  _Tt_client_isam_record_ptr getFullRecord (const _Tt_string&);
  void                getISAMFileInfo ();

  void		      getStatusInfo ()
    {
      currentRecordLength = dbConnection->isreclen;
      currentRecordNumber = dbConnection->isrecnum;
      errorStatus = dbConnection->iserrno;
    }
};

#endif  /* _TT_CLIENT_ISAM_FILE_H */
