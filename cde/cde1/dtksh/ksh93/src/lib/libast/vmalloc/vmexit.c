/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*         THIS IS PROPRIETARY SOURCE CODE LICENSED BY          *
*                          AT&T CORP.                          *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*                     All Rights Reserved                      *
*                                                              *
*           This software is licensed by AT&T Corp.            *
*       under the terms and conditions of the license in       *
*       http://www.research.att.com/orgs/ssr/book/reuse        *
*                                                              *
*               This software was created by the               *
*           Software Engineering Research Department           *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                     gsf@research.att.com                     *
*                                                              *
***************************************************************/
#include	"vmhdr.h"

/*
**	Any required functions for process exiting.
**	Written by Kiem-Phong Vo (05/25/93).
*/
#if PACKAGE_ast || _lib_atexit
int	Vm_atexit_already_defined;
#else

_BEGIN_EXTERNS_
extern int	onexit _ARG_(( void(*)() ));
extern int	on_exit _ARG_(( void(*)(), char* ));
extern void	_exit _ARG_((int));
extern void	_cleanup();
_END_EXTERNS_

#if _lib_onexit || _lib_on_exit

#if __STD_C
atexit(void (*exitf)(void))
#else
atexit(exitf)
void	(*exitf)();
#endif
{
#if _lib_onexit
	return onexit(exitf);
#else
#if _lib_on_exit
	return on_exit(exitf,NIL(char*));
#endif
#endif
}

#else /*!(_lib_onexit || _lib_onexit)*/

typedef struct _exit_s
{	struct _exit_s*	next;
	void(*		exitf)_ARG_((void));
} Exit_t;
static Exit_t*	Exit;

#if __STD_C
atexit(void (*exitf)(void))
#else
atexit(exitf)
void	(*exitf)();
#endif
{	Exit_t*	e;

	if(!(e = (Exit_t*)malloc(sizeof(Exit_t))) )
		return -1;
	e->exitf = exitf;
	e->next = Exit;
	Exit = e;
	return 0;
}

#if __STD_C
exit(int type)
#else
exit(type)
int	type;
#endif
{
	Exit_t*	e;

	for(e = Exit; e; e = e->next)
		(*e->exitf)();

#if _exit_cleanup
	_cleanup();
#endif

	_exit(type);
	return type;
}

#endif	/* _lib_onexit || _lib_on_exit */

#endif /*!PACKAGE_ast*/
