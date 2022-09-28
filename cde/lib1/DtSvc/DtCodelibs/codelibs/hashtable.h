/*
 * File:	hashtable.h $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */

#ifndef __HASHTABLE_H_
#define __HASHTABLE_H_

#ifndef GENERICH
#include <generic.h>
#endif

#include <codelibs/boolean.h>
#include <stddef.h>

#define __hashnode(hashtable) name2(hashtable,hashnode)
#define __hashkey(hashtable) name2(hashtable,hashkey)

typedef unsigned __HASHTABLE_BUMP(unsigned);
extern unsigned __hashtable_bump(unsigned);


#define declare_hashtable(hashtable,iterator,symbol,key) \
typedef key __hashkey(hashtable); \
struct __hashnode(hashtable) \
{ \
	__hashnode(hashtable) *next; \
	symbol sym; \
	__hashnode(hashtable)(key k, __hashnode(hashtable) *tail) \
		: sym(k) { next = tail; } \
	__hashnode(hashtable)(symbol &s, __hashnode(hashtable) *tail) \
		: sym(s) { next = tail; } \
}; \
class hashtable \
{ \
	unsigned hashsize; \
	unsigned threshold; \
	unsigned numnode; \
	__hashnode(hashtable) **hashvec; \
	__hashnode(hashtable) *curr; \
	friend class iterator; \
	iterator *ilist; \
	unsigned (*bump)(unsigned); \
	void resize(unsigned =0); \
	void stomp(__hashnode(hashtable) **); \
	boolean insertsym(symbol&); \
public: \
	hashtable(unsigned hs =0, __HASHTABLE_BUMP *bump_function =NULL); \
	hashtable(hashtable&); \
	~hashtable(); \
	boolean find(key); \
	boolean insert(key); \
	boolean remove(key); \
	boolean operator+=(key k) { return insert(k); } \
	boolean operator-=(key k) { return remove(k); } \
	boolean operator==(key k) { return find(k); } \
	boolean operator!=(key k) { return !find(k); } \
	symbol &operator[](key); \
	symbol &get(); \
	symbol &operator()(); \
	symbol &operator*(); \
	symbol *operator->(); \
	hashtable& clear(); \
	hashtable& operator=(hashtable&); \
	boolean operator==(hashtable&); \
	hashtable& operator+=(hashtable&); \
	hashtable& operator-=(hashtable&); \
	hashtable& operator&=(hashtable&); \
	unsigned size(); \
}; \
class iterator \
{ \
	iterator *prev; \
	iterator *next; \
	hashtable *parent; \
	__hashnode(hashtable) *curr; \
	unsigned index; \
	void transfer(hashtable *); \
	friend class hashtable; \
public: \
	iterator(); \
	iterator(hashtable &); \
	~iterator(); \
	hashtable &operator=(hashtable &); \
	boolean operator==(iterator &); \
	boolean operator!=(iterator &); \
	operator boolean(); \
	symbol &get(); \
	symbol &operator()(); \
	symbol &operator*(); \
	symbol *operator->(); \
	symbol &operator++(); \
	void remove(); \
}; \
inline symbol &hashtable::operator[](key k) \
{ \
	void(insert(k)); \
	return curr->sym; \
} \
inline symbol &hashtable::get() \
{ \
	if (curr != NULL) \
		return curr->sym; \
	else \
		return *(symbol *)NULL; \
} \
inline symbol &hashtable::operator()() { return get(); } \
inline symbol &hashtable::operator*() { return get(); } \
inline symbol *hashtable::operator->() { return &get(); } \
inline unsigned hashtable::size() \
{ \
	return numnode; \
} \
inline iterator::iterator() \
{ \
	prev = next = this; \
	parent = NULL; \
	curr = NULL; \
} \
inline iterator::iterator(hashtable &t) \
{ \
	prev = next = this; \
	parent = NULL; \
	this->transfer(&t); \
} \
inline iterator::~iterator() \
{ \
	this->transfer(NULL); \
} \
inline hashtable &iterator::operator=(hashtable &t) \
{ \
	this->transfer(&t); \
	return t; \
} \
inline boolean iterator::operator==(iterator &i) \
{ \
	return (curr == i.curr); \
} \
inline boolean iterator::operator!=(iterator &i) \
{ \
	return (curr != i.curr); \
} \
inline iterator::operator boolean() \
{ \
	return (curr != NULL); \
} \
inline symbol &iterator::get() \
{ \
	if (curr != NULL) \
		return curr->sym; \
	else \
		return *(symbol *)NULL; \
} \
inline symbol &iterator::operator()() { return get(); } \
inline symbol &iterator::operator*() { return get(); } \
inline symbol *iterator::operator->() { return &get(); } \



#define implement_hashtable(hashtable,iterator,symbol,key,hash) \
hashtable::hashtable(unsigned hs, __HASHTABLE_BUMP *bump_function) \
{ \
	hashsize = 0; \
	numnode = 0; \
	hashvec = NULL; \
	curr = NULL; \
	ilist = NULL; \
	bump = (bump_function == NULL) ? &__hashtable_bump : bump_function; \
	resize(hs); \
} \
hashtable::hashtable(hashtable& h) \
{ \
	hashsize = 0; \
	numnode = 0; \
	hashvec = NULL; \
	curr = NULL; \
	ilist = NULL; \
	bump = h.bump; \
	resize(h.hashsize); \
	*this = h; \
} \
hashtable::~hashtable() \
{ \
	clear(); \
	delete hashvec; \
} \
boolean hashtable::find(key k) \
{ \
	register __hashnode(hashtable) *p = hashvec[hash(k) % hashsize]; \
	for (; p != NULL; p = p->next) \
		if (p->sym == k) \
		{ \
			curr = p; \
			return TRUE; \
		} \
	curr = NULL; \
	return FALSE; \
} \
boolean hashtable::insert(key k) \
{ \
	if (numnode >= threshold && ilist == NULL) \
		resize(); \
	register __hashnode(hashtable) **q = hashvec + hash(k) % hashsize; \
	register __hashnode(hashtable) *p = *q; \
	for (; p != NULL; p = p->next) \
		if (p->sym == k) \
		{ \
			curr = p; \
			return FALSE; \
		} \
	curr = *q = new __hashnode(hashtable)(k, *q); \
	numnode++; \
	return TRUE; \
} \
boolean hashtable::insertsym(symbol &s) \
{ \
	if (numnode >= threshold && ilist == NULL) \
		resize(); \
	register __hashnode(hashtable) **q = hashvec + \
		hash((__hashkey(hashtable))s) % hashsize; \
	register __hashnode(hashtable) *p = *q; \
	for (; p != NULL; p = p->next) \
		if (p->sym == (__hashkey(hashtable))s) \
		{ \
			curr = p; \
			return FALSE; \
		} \
	curr = *q = new __hashnode(hashtable)(s, *q); \
	numnode++; \
	return TRUE; \
} \
boolean hashtable::remove(key k) \
{ \
	register __hashnode(hashtable) **p = hashvec + hash(k) % hashsize; \
	curr = NULL; \
	for (; (*p) != NULL; p = &((**p).next)) \
		if ((**p).sym == k) \
		{ \
			stomp(p); \
			return TRUE; \
		} \
	return FALSE; \
} \
void hashtable::stomp(__hashnode(hashtable) **p) \
{ \
	if (ilist != NULL) \
	{ \
		iterator *i = ilist; \
		do \
		{ \
			if (i->curr == *p) \
				void((*i)++); \
		} \
		while ((i = i->next) != ilist); \
	} \
	__hashnode(hashtable) *next = (**p).next; \
	delete *p; \
	*p = next; \
	--numnode; \
} \
void hashtable::resize(unsigned hs) \
{ \
	register __hashnode(hashtable) **pos, **endpos; \
	__hashnode(hashtable) *chain = NULL; \
	unsigned new_hashsize = (hs == 0) ? bump(hashsize) : hs; \
	if (new_hashsize == 0) \
		new_hashsize = 1; \
	if (new_hashsize <= hashsize) \
		return; \
	if (hashvec != NULL) \
	{ \
		pos = hashvec; \
		endpos = pos + hashsize; \
		for (; pos < endpos; pos++) \
		{ \
			__hashnode(hashtable) *p = *pos; \
			if (p != NULL) \
			{ \
				while (p->next != NULL) \
					p = p->next; \
				p->next = chain; \
				chain = *pos; \
			} \
		} \
	} \
	hashsize = new_hashsize; \
	threshold = hashsize + (hashsize >> 2); \
	delete hashvec; \
	pos = hashvec = new __hashnode(hashtable) *[hashsize]; \
	endpos = pos + hashsize; \
	while (pos < endpos) \
		*(pos++) = NULL; \
	while (chain != NULL) \
	{ \
		__hashnode(hashtable) *entry = chain; \
		chain = entry->next; \
		pos = hashvec + \
				hash((__hashkey(hashtable))entry->sym) \
				% hashsize; \
		entry->next = *pos; \
		*pos = entry; \
	} \
} \
hashtable& hashtable::clear() \
{ \
	for (int i = 0; i < hashsize; i++) \
	{ \
		__hashnode(hashtable) *p = hashvec[i]; \
		while (p != NULL) \
		{ \
			__hashnode(hashtable) *next = p->next; \
			delete p; \
			p = next; \
		} \
		hashvec[i] = NULL; \
	} \
	while (ilist != NULL) \
		ilist->transfer(NULL); \
	numnode = 0; \
	curr = NULL; \
	return *this; \
} \
hashtable& hashtable::operator=(hashtable& h) \
{ \
	if (this == &h) \
	    return *this; \
	clear(); \
	for (iterator i = h; i; i++) \
		insertsym(i()); \
	return *this; \
} \
boolean hashtable::operator==(hashtable& h) \
{ \
	if (this == &h) \
	    return TRUE; \
	if (size() != h.size()) \
	    return FALSE; \
	for (iterator i = h; i; i++) \
	    if (!find((__hashkey(hashtable))i())) \
		return FALSE; \
	return TRUE; \
} \
hashtable& hashtable::operator+=(hashtable& h) \
{ \
	if (this == &h) \
	    return *this; \
	for (iterator i = h; i; i++) \
		insert((__hashkey(hashtable))i()); \
	return *this; \
} \
hashtable& hashtable::operator-=(hashtable& h) \
{ \
	if (this == &h) \
	    return clear(); \
	for (iterator i = h; i; i++) \
		remove((__hashkey(hashtable))i()); \
	return *this; \
} \
hashtable& hashtable::operator&=(hashtable& h) \
{ \
	if (this == &h) \
	    return *this; \
	for (iterator i = *this; i; ) \
	    if (h.find((__hashkey(hashtable))i())) \
		i++; \
	    else \
		i.remove(); \
	return *this; \
} \
symbol &iterator::operator++() \
{ \
	if (curr != NULL) \
	{ \
		__hashnode(hashtable) *prev = curr; \
		for (curr = curr->next; \
				curr == NULL && ++index < parent->hashsize; \
				curr = parent->hashvec[index]) \
			; \
		return prev->sym; \
	} \
	else \
	{ \
		return *(symbol *)NULL; \
	} \
} \
void iterator::remove() \
{ \
	if (curr != NULL) \
	{ \
		register __hashnode(hashtable) **p = parent->hashvec + index; \
		for (; (*p) != curr; p = &((**p).next)) \
			; \
		parent->stomp(p); \
	} \
} \
void iterator::transfer(hashtable *newparent) \
{ \
	if (parent != NULL) \
	{ \
		next->prev = prev; \
		prev->next = next; \
		if (parent->ilist == this) \
			parent->ilist = (next == this) ? NULL : next; \
	} \
	if ((parent = newparent) != NULL) \
	{ \
		if (parent->ilist == NULL) \
			parent->ilist = prev = next = this; \
		else \
		{ \
			next = parent->ilist; \
			prev = next->prev; \
			parent->ilist = next->prev = prev->next = this; \
		} \
		for (index = 0, curr = parent->hashvec[0]; \
				curr == NULL && ++index < parent->hashsize; \
				curr = parent->hashvec[index]) \
			; \
	} \
	else \
		curr = NULL; \
} \


#endif /* __HASHTABLE_H_ */
/*
@(#)REV: 2.18 90/12/13                                               
*/
