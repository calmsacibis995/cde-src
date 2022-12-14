//%%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
//%%  (c) Copyright 1993, 1994 International Business Machines Corp.	
//%%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
//%%  (c) Copyright 1993, 1994 Novell, Inc. 				
//%%  $Revision: $ 			 				
/*
 *
 * api_storage.cc
 *
 * Copyright (c) 1990 by Sun Microsystems, Inc.
 *
 * implementation of api storage classes
 *
 * NOTE: this implementation is rather over-abstract, using the
 * generic list classes to manage the stack.  If this turns out
 * to be too expensive, a very simple array-of-void* could
 * be used; the marks could just be special entries (since malloc()
 * always returns addresses on a doubleword boundary, any void* that's
 * not a multiple of 8 would be distinguishable.) The only problem would
 * be growing the array when out of room.
 */

#include <stdlib.h>
#include <stdio.h>
#include "api/c/api_storage.h"

_Tt_api_stg_stack::
_Tt_api_stg_stack(){
	mark_counter = 0;
	stack = new _Tt_api_stg_stack_elm_list;
}

_Tt_api_stg_stack::
~_Tt_api_stg_stack(){
}

int _Tt_api_stg_stack::
mark()
{
	int result = ++mark_counter;
	_Tt_api_stg_stack_elm_ptr e = new _Tt_api_stg_stack_elm;
	
	e->entry_type = _Tt_api_stg_stack_elm::STACK_MARK;
	e->addr = (caddr_t) result;
	stack->push(e);
	return result;
}

void _Tt_api_stg_stack::
release(int mark)
{
	// it's tempting to just pop and free until we get to the
	// matching mark, but that would be terrible if the mark
	// didn't exist (e.g. two releases for the same mark):
	// we'd free *everything*.

	_Tt_api_stg_stack_elm_list_cursor c(stack);
	int found = 0;

	while(!found && c.next()) {
		if (c->entry_type == _Tt_api_stg_stack_elm::STACK_MARK)
    		   if ((int)c->addr == mark) {
			   found = 1;
		}
	}
	if (!found) return;
	do {
		if (c->entry_type == _Tt_api_stg_stack_elm::STACK_STORAGE) {
			::free((MALLOCTYPE *)c->addr);
		}
		c.remove();
	} while(c.prev());
	return;
}

caddr_t _Tt_api_stg_stack::
#ifdef	_DEBUG_MALLOC_INC
debug_malloc(char *fname, int lnum, size_t size)
#else
malloc(size_t size)
#endif
{
	caddr_t p = (caddr_t)::malloc(size);
	if (p==0) return p;
	
	_Tt_api_stg_stack_elm_ptr e = new _Tt_api_stg_stack_elm;
	e->entry_type = _Tt_api_stg_stack_elm::STACK_STORAGE;
	e->addr = p;
	stack->push(e);
	return p;
}

caddr_t _Tt_api_stg_stack::
take(caddr_t addr)
{
	_Tt_api_stg_stack_elm_ptr e = new _Tt_api_stg_stack_elm;
	e->entry_type = _Tt_api_stg_stack_elm::STACK_STORAGE;
	e->addr = addr;
	stack->push(e);
	return addr;
}

void _Tt_api_stg_stack::
#ifdef	_DEBUG_MALLOC_INC
debug_free(char *fname, int lnum, caddr_t addr)
#else
free(caddr_t addr)
#endif
{
	_Tt_api_stg_stack_elm_list_cursor c(stack);

	while(c.prev()) {
		if (c->entry_type == _Tt_api_stg_stack_elm::STACK_STORAGE)
		  if (c->addr == addr) {
			::free((MALLOCTYPE *)c->addr);
			c.remove();
			return;
		}
	}
}

void 
_tt_api_stg_stack_elm_print(FILE *fs, const _Tt_object *obj)
{
	((_Tt_api_stg_stack_elm *)obj)->print(fs);
}

/*
 * Print a _Tt_api_stg_stack
void _Tt_api_stg_stack::
print(FILE *f) const
{
	(void)fprintf(f,"_Tt_api_stg_stack at %x <\n",(int)this);
	(void)fprintf(f,"mark_counter: %d\n",mark_counter);
	stack->print(_tt_api_stg_stack_elm_print);
	(void)fprintf(f,"> _Tt_api_stg_stack at %x\n",(int)this);
}
 */

_Tt_api_stg_stack_elm::
~_Tt_api_stg_stack_elm()
{
}

/*
 * Print a _Tt_api_stg_stack_elm
 */
void _Tt_api_stg_stack_elm::
print(FILE *f) const
{
	
	(void)fprintf(f,"_Tt_api_stg_stack_elm at %x <",(int)this);
	switch (entry_type) {
	      case STACK_MARK:
	        (void)fprintf(f,"mark %d>\n",(int)addr);
		break;
	      case STACK_STORAGE:
		(void)fprintf(f,"addr 0x%lx>\n",(long)addr);
		break;
	}
}

