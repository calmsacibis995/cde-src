#ifdef apollo
#ifndef	_Apollo_INCLUDED	/* to keep this from happening twice */
#define	_Apollo_INCLUDED

/* Apollo sys5 stuff needed to compile this code on a BSD Apollo machine */
/* !!!XXX This is BOGUS!  But since the BSD and Sys5 <types.h> conflict I
 *   don't think that I have a choice.  If the kernal/type.h ever changes, 
 *   this stuff better be updated!  --CD
 *
 * Files depending on this header include:
 *    hp/grm/grmd/CheckSys.c
 *    ddx/hp/hp/mergefload.c
 *    a variety of vue clients :-(
 */

        /* HPUX /usr/include/sys/ioctl.h */
#define FIOSNBIO        FIONBIO

#if 0		/* these were added to the 10.3 bsd sys/types.h */
        /* /sys5/usr/include/sys/types.h */
typedef long            key_t;          /* IPC key type */
typedef int             pid_t;        /* process id/process group id*/
#endif

        /* /sys5/usr/include/sys/stat.h */
#define S_IRUSR  00400           /* read permission: owner  */
#define S_IRGRP  00040           /* read permission: group  */
#define S_IWUSR  00200           /* write permission: owner */
#define S_IWGRP  00020           /* write permission: group */
#define S_IXUSR  00100           /* execute permission: owner */
#define S_IXGRP  00010           /* execute permission: group */
#define S_IROTH  00004           /* read permission: other    */
#define S_IWOTH  00002           /* write permission: other   */
#define S_IXOTH  00001           /* execute permission: other */

#define S_ISLNK(_M)  ((_M & S_IFMT)==S_IFLNK)     /* test for symbolic link */
#define S_ISDIR(_M)  ((_M & S_IFMT)==S_IFDIR)     /* test for directory     */


        /* /sys5/usr/include/stdio.h */
#define L_cuserid	32

#ifndef NULL
#define NULL 0
#endif

#endif	/* _Apollo_INCLUDED */
#endif	/* apollo */
