/* $XConsortium: cd_pwd.c /main/cde1_maint/3 1995/10/13 23:59:39 montyb $ */
/***************************************************************
*                                                              *
*                      AT&T - PROPRIETARY                      *
*                                                              *
*        THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF        *
*                    AT&T BELL LABORATORIES                    *
*         AND IS NOT TO BE DISCLOSED OR USED EXCEPT IN         *
*            ACCORDANCE WITH APPLICABLE AGREEMENTS             *
*                                                              *
*                Copyright (c) 1995 AT&T Corp.                 *
*              Unpublished & Not for Publication               *
*                     All Rights Reserved                      *
*                                                              *
*       The copyright notice above does not evidence any       *
*      actual or intended publication of such source code      *
*                                                              *
*               This software was created by the               *
*           Advanced Software Technology Department            *
*                    AT&T Bell Laboratories                    *
*                                                              *
*               For further information contact                *
*                    {research,attmail}!dgk                    *
*                                                              *
***************************************************************/

/* : : generated by proto : : */

#if !defined(__PROTO__)
#if defined(__STDC__) || defined(__cplusplus) || defined(_proto) || defined(c_plusplus)
#if defined(__cplusplus)
#define __MANGLE__	"C"
#else
#define __MANGLE__
#endif
#define __STDARG__
#define __PROTO__(x)	x
#define __OTORP__(x)
#define __PARAM__(n,o)	n
#if !defined(__STDC__) && !defined(__cplusplus)
#if !defined(c_plusplus)
#define const
#endif
#define signed
#define void		int
#define volatile
#define __V_		char
#else
#define __V_		void
#endif
#else
#define __PROTO__(x)	()
#define __OTORP__(x)	x
#define __PARAM__(n,o)	o
#define __MANGLE__
#define __V_		char
#define const
#define signed
#define void		int
#define volatile
#endif
#if defined(__cplusplus) || defined(c_plusplus)
#define __VARARG__	...
#else
#define __VARARG__
#endif
#if defined(__STDARG__)
#define __VA_START__(p,a)	va_start(p,a)
#else
#define __VA_START__(p,a)	va_start(p)
#endif
#endif
#include	"defs.h"
#include	<stak.h>
#include	<error.h>
#include	"variables.h"
#include	"path.h"
#include	"name.h"
#include	"builtins.h"
#include	<ls.h>

int	b_cd __PARAM__((int argc, char *argv[],__V_ *extra), (argc, argv, extra)) __OTORP__(int argc; char *argv[];__V_ *extra;){
	register char *dir, *cdpath="";
	register const char *dp;
	int saverrno=0;
	int rval,flag = (sh_isoption(SH_PHYSICAL)!=0);
	char *oldpwd;
	Namval_t *opwdnod, *pwdnod;
	NOT_USED(extra);
	if(sh_isoption(SH_RESTRICTED))
		error(ERROR_exit(1),e_restricted+4);
	while((rval = optget(argv,sh_optcd))) switch(rval)
	{
		case 'L':
			flag = 0;
			break;
		case 'P':
			flag = 1;
			break;
		case ':':
			error(2, opt_arg);
			break;
		case '?':
			error(ERROR_usage(2), opt_arg);
			break;
	}
	argv += opt_index;
	argc -= opt_index;
	dir =  argv[0];
	if(error_info.errors>0 || argc >2)
		error(ERROR_usage(2),optusage((char*)0));
	oldpwd = (char*)sh.pwd;
	opwdnod = (sh.subshell?sh_assignok(OLDPWDNOD,1):OLDPWDNOD); 
	pwdnod = (sh.subshell?sh_assignok(PWDNOD,1):PWDNOD); 
	if(argc==2)
		dir = sh_substitute(oldpwd,dir,argv[1]);
	else if(!dir || *dir==0)
		dir = nv_getval(HOME);
	else if(*dir == '-' && dir[1]==0)
		dir = nv_getval(opwdnod);
	if(!dir || *dir==0)
		error(ERROR_exit(1),argc==2?e_subst+4:e_direct);
	if(*dir != '/')
	{
		cdpath = nv_getval(nv_scoped(CDPNOD));
		if(!oldpwd)
			oldpwd = path_pwd(1);
	}
	if(!cdpath)
		cdpath = "";
	if(*dir=='.')
	{
		/* test for pathname . ./ .. or ../ */
		if(*(dp=dir+1) == '.')
			dp++;
		if(*dp==0 || *dp=='/')
			cdpath = "";
	}
	rval = -1;
	do
	{
		dp = cdpath;
		cdpath=path_join(cdpath,dir);
		if(*stakptr(PATH_OFFSET)!='/')
		{
			char *last=(char*)stakfreeze(1);
			stakseek(PATH_OFFSET);
			stakputs(oldpwd);
			stakputc('/');
			stakputs(last+PATH_OFFSET);
			stakputc(0);
		}
		if(!flag)
		{
			register char *cp;
			stakseek(PATH_MAX+PATH_OFFSET);
#ifdef SHOPT_FS_3D
			if(!(cp = pathcanon(stakptr(PATH_OFFSET),PATH_DOTDOT)))
				continue;
			/* eliminate trailing '/' */
			while(*--cp == '/' && cp>stakptr(PATH_OFFSET))
				*cp = 0;
#else
			if(*(cp=stakptr(PATH_OFFSET))=='/')
				if(!pathcanon(cp,PATH_DOTDOT))
					continue;
#endif /* SHOPT_FS_3D */
		}
		if((rval=chdir(path_relative(stakptr(PATH_OFFSET)))) >= 0)
			goto success;
		if(errno!=ENOENT && saverrno==0)
			saverrno=errno;
	}
	while(cdpath);
	if(rval<0 && *dir=='/' && *(path_relative(stakptr(PATH_OFFSET)))!='/')
		rval = chdir(dir);
	/* use absolute chdir() if relative chdir() fails */
	if(rval<0)
	{
		if(saverrno)
			errno = saverrno;
		error(ERROR_system(1),"%s:",dir);
	}
success:
	if(dir == nv_getval(opwdnod) || argc==2)
		dp = dir;	/* print out directory for cd - */
	if(flag)
	{
		dir = stakptr(PATH_OFFSET);
		if (!(dir=pathcanon(dir,PATH_PHYSICAL)))
		{
			dir = stakptr(PATH_OFFSET);
			error(ERROR_system(1),"%s:",dir);
		}
		stakseek(dir-stakptr(0));
	}
	dir = (char*)stakfreeze(1)+PATH_OFFSET;
	if(*dp && *dp!= ':' && strchr(dir,'/'))
		sfputr(sfstdout,dir,'\n');
	if(*dir != '/')
		return(0);
	nv_putval(opwdnod,oldpwd,NV_RDONLY);
	if(oldpwd)
		free(oldpwd);
	flag = strlen(dir);
	/* delete trailing '/' */
	while(--flag>0 && dir[flag]=='/')
		dir[flag] = 0;
	nv_putval(pwdnod,dir,NV_RDONLY);
	nv_onattr(pwdnod,NV_NOFREE|NV_EXPORT);
	sh.pwd = pwdnod->nvalue.cp;
	return(0);
}

int	b_pwd __PARAM__((int argc, char *argv[],__V_ *extra), (argc, argv, extra)) __OTORP__(int argc; char *argv[];__V_ *extra;){
	register int n, flag = (sh_isoption(SH_PHYSICAL)!=0);
	register char *cp;
	NOT_USED(extra);
	NOT_USED(argc);
	while((n = optget(argv,sh_optpwd))) switch(n)
	{
		case 'L':
			flag = 0;
			break;
		case 'P':
			flag = 1;
			break;
		case ':':
			error(2, opt_arg);
			break;
		case '?':
			error(ERROR_usage(2), opt_arg);
			break;
	}
	if(error_info.errors)
		error(ERROR_usage(2),optusage((char*)0));
	if(*(cp = path_pwd(0)) != '/')
		error(ERROR_system(1), e_pwd);
	if(flag)
	{
#ifdef SHOPT_FS_3D
		if((flag = mount(e_dot,NIL(char*),FS3D_GET|FS3D_VIEW,0))>=0)
		{
			cp = (char*)stakseek(++flag+PATH_MAX);
			mount(e_dot,cp,FS3D_GET|FS3D_VIEW|FS3D_SIZE(flag),0);
		}
		else
#endif /* SHOPT_FS_3D */
			cp = strcpy(stakseek(strlen(cp)+PATH_MAX),cp);
		pathcanon(cp,PATH_PHYSICAL);
	}
	sfputr(sfstdout,cp,'\n');
	return(0);
}

