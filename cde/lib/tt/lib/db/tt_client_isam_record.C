//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 * tt_client_isam_record.cc - Defines the TT ISAM record class.
 *
 * Copyright (c) 1992 by Sun Microsystems, Inc.
 */

#include "db/tt_client_isam_record.h"

_Tt_client_isam_record::
_Tt_client_isam_record (const _Tt_client_isam_key_descriptor_list_ptr &key_descriptor_list,
	         int                                     max_record_length,
		 int                                     min_record_length)
{
  _Tt_string new_buffer(max_record_length);
  memset((char *)new_buffer, '\0', max_record_length);

  buffer = new_buffer;
  keyDescriptorList = key_descriptor_list;
  currentLength = min_record_length;
  maxLength = max_record_length;
  minLength = min_record_length;
}

_Tt_client_isam_record::~_Tt_client_isam_record ()
{
}

int _Tt_client_isam_record::getNumberOfKeys () const
{
  return keyDescriptorList->count();
}

_Tt_client_isam_key_descriptor_ptr _Tt_client_isam_record::getKeyDescriptor (int index) const
{
  _Tt_client_isam_key_descriptor_ptr descriptor_ptr = (_Tt_client_isam_key_descriptor *)NULL;
  
  if ((index > -1) && (index < keyDescriptorList->count())) {
    descriptor_ptr = (*keyDescriptorList) [index];
  }

  return descriptor_ptr;
}

_Tt_string _Tt_client_isam_record::getKeyPartValue (int index, int part) const
{
  _Tt_string value;

  short key_part_start;
  short key_part_length;
  short key_part_type;

  if ((index > -1) && (index < keyDescriptorList->count())) {
    _Tt_client_isam_key_descriptor_ptr descriptor_ptr = (*keyDescriptorList) [index];
    
    if ((part > -1) && (part < descriptor_ptr->getNumberOfParts())) {
      descriptor_ptr->getKeyPart(part,
				 key_part_start,
				 key_part_length,
				 key_part_type);
      
      _Tt_string key_part(key_part_length);
      (void)memcpy((char *)key_part,
		   (char *)buffer+key_part_start,
		   key_part_length);
      value = key_part;
    }
    else {
      _Tt_string null_string((char *)NULL);
      value = null_string;
    }
  }
  else {
    _Tt_string null_string((char *)NULL);
    value = null_string;
  }

  return value;
}

void _Tt_client_isam_record::setKeyPartValue (int               index,
				       int               part,
				       const _Tt_string &value)
{
  short key_part_start;
  short key_part_length;
  short key_part_type;

  if ((index > -1) && (index < keyDescriptorList->count())) {
    _Tt_client_isam_key_descriptor_ptr descriptor_ptr = (*keyDescriptorList) [index];

    descriptor_ptr->getKeyPart(part,
			       key_part_start,
			       key_part_length,
			       key_part_type);

    int length = ((value.len() < key_part_length) ?
		  value.len() : key_part_length);
    (void)memcpy((char *)buffer+key_part_start, (char *)value, length);
  }
}

_Tt_string _Tt_client_isam_record::getBytes (int start, int length) const
{
  if (length < 1) {
    length = currentLength - start;
  }

  _Tt_string value(length);
  (void)memcpy((char *)value, (char *)buffer+start, length);
  return value;
}
     
void _Tt_client_isam_record::setBytes (int start, const _Tt_string &value)
{
  (void)memcpy((char *)buffer+start, (char *)value, value.len());
}

void _Tt_client_isam_record::setBytes (int               start,
				int               length,
				const _Tt_string &value)
{
  (void)memcpy((char *)buffer+start, (char *)value, length);
}
