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
#pragma ident "@(#)HashTable.hh	1.6 02/14/95"
#endif

#ifndef _HASHTABLE_HH
#define _HASHTABLE_HH

#include <DtMail/ObjectKey.hh>

class HashTableImpl : public DtCPlusPlusAllocator {
  public:
    HashTableImpl(int table_size);
    virtual ~HashTableImpl(void);

    virtual void * lookup(ObjectKey &);
    virtual void set(ObjectKey & key, void * value);
    virtual void * remove(ObjectKey & key);

// HP can not support this construct.
//
#if !defined(HPUX)
#define HASHTABLE_HASFOREACH
    typedef int (*HashImplIterator)(ObjectKey &, void * value, void * client_data);
    virtual void forEach(HashImplIterator, void * client_data);
#endif

  protected:
    struct HashEntry : public DtCPlusPlusAllocator {
	ObjectKey *	key;
	void *		value;
	HashEntry	*next;
    };

    HashEntry *		_hash_table;
    int			_table_size;
};

template <class Object>
class HashTable : public DtCPlusPlusAllocator {
  public:
    HashTable(int table_size) : _table(table_size) { }

    Object lookup(ObjectKey & key) { return((Object)_table.lookup(key)); }

    void set(ObjectKey & key, Object obj) {
	_table.set(key, (void *)obj);
    }

    Object remove(ObjectKey & key) { return((Object)_table.remove(key)); }

#if !defined(HPUX)
    typedef int (*HashIterator)(ObjectKey &, Object, void * client_data);
    void forEach(HashTableImpl::HashImplIterator iterator, void * client_data) {
	_table.forEach((HashTableImpl::HashImplIterator)iterator, client_data);
    }
#endif

  protected:
    HashTableImpl	_table;
};

#endif
