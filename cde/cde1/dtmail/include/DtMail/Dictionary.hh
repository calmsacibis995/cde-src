/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.3 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)Dictionary.hh	1.3 04/14/94"
#endif

#ifndef _DTM_DICTIONARY_HH
#define _DTM_DICTIONARY_HH

#include <DtMail/DtVirtArray.hh>

class DtMailDictionaryImpl : public DtCPlusPlusAllocator {
  public:
    // The hash_size must be <= 256. Setting to smaller sizes
    // will reduce the memory used by the dictionary, but may increase
    // the hash collisions resulting in slower performance.
    //
    DtMailDictionaryImpl(int hash_size = 256);
    ~DtMailDictionaryImpl(void);

    void set(const char * key, const void * value);
    const void * lookup(const char * key);
    void remove(const char * key);

  private:
    struct Entry : public DtCPlusPlusAllocator {
	char *			key;
	const void *		value;
    };

    typedef DtVirtArray<Entry *> * HashTable_t;

    HashTable_t		*_hash_table;
    int			_hash_size;
    void		*_obj_mutex;

    int hashValue(const char * key);
    void locate(int hash_value, const char * key, Entry ** entry);
};

template <class Element>
class DtMailDictionary : public DtCPlusPlusAllocator {
  public:
    DtMailDictionary(int hash_size = 256) : _dict(hash_size) { }
    ~DtMailDictionary(void) { }

    void set(const char * key, const Element value) { _dict.set(key, value); }

    const Element lookup(const char * key) {
	return((const Element)_dict.lookup(key));
    }

    void remove(const char * key) { _dict.remove(key); }

  private:
    DtMailDictionaryImpl	_dict;
    
};

#endif
