/*
 * $XConsortium: TermPrimOSDepI.h /main/cde1_maint/3 1995/10/09 11:49:05 pascale $";
 */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/******************************************************************************

    this is a list of OS dependent defines that are available...

    XOR_CAPS_LOCK		xor caps lock and shift so that lock+shift
				acts as a noop for alpha keys
    MESSAGE_CAT			enables the message catalog code
    USE_TIOCCONS		use TIOCCONS iocto for -C option
    USE_SRIOCSREDIR		use SRIOCSREDIR ioctl for -C option
    SETENV_LINES_AND_COLS	set $LINES and $COLUMNS
    HAS_SETRESUID		os supports setresuid(2) - first choice
    HAS_SETEUID			os supports seteuid(2) - second choice
    HAS_SETREUID		os supports setreuid(2) - second choice
    USE_STREAMS			use streams as opposed to ptys - first choice
    USE_STREAMS_TTCOMPAT	use ttcompat streams mod
    USE_STREAMS_BUFMOD		use bofmod streams mod
    USE_PTYS			use ptys - second choice
    USE_CSWIDTH			use the csWidth resource to initialize
				multi-byte processing in ldterm
    USE_SETCSMAP		use setcsmap() for multi-byte processing
    USE_SUN_WCWIDTH_PATCH	use SUN wcwidth() workaround
    USE_TIOCBREAK		use TIOCBREAK ioctl() to send RS232 break
    USE_TCSBRK			use TCBRK ioctl() to send RS232 break
    USE_TCSBREAK		use TCBREAK ioctl() to send RS232 break
    USE_TCSENDBREAK		use tiocbreak() to send RS232 break

******************************************************************************/

#ifndef	_Dt_TermPrimOSDepI_h
#define	_Dt_TermPrimOSDepI_h

#ifdef  ALPHA_ARCHITECTURE
#ifndef         MESSAGE_CAT
#define         MESSAGE_CAT             /* use message catalogs         */
#endif
#define         USE_TIOCCONS            /* use tioccons for -C          */
#define         HAS_SETEUID             /* seteuid available            */
#define         HAS_SETREUID            /* setreuid available           */
#define         USE_PTYS                /* use ptys                     */
#define         USE_TCSBRK              /* use TCSBRK ioctl()           */
#endif /* ALPHA_ARCHITECTURE */

#ifdef	HP_ARCHITECTURE
#define		XOR_CAPS_LOCK		/* xor caps lock and shift	*/
#define		MESSAGE_CAT		/* use message catalogs		*/
#define		USE_TIOCCONS		/* use tioccons for -C		*/
#define		SETENV_LINES_AND_COLS	/* set $LINES and $COLUMNS	*/
#define		HAS_SETRESUID		/* setresuid available		*/

#if	OSMAJORVERSION > 9
#define		USE_STREAMS		/* use streams			    */
#define		USE_CSWIDTH		/* use the csWidth resource to      */
				        /* initialize multi-byte processing */
                                        /* in ldterm                        */
#define		USE_TCSENDBREAK		/* use tiocbreak()		*/
#else	/* OSMAJORVERSION > 9 */
#define		USE_PTYS		/* use ptys			*/
#define		USE_TIOCBREAK		/* use TIOCBREAK ioctl()	*/
#endif	/* OSMAJORVERSION > 9 */
#endif	/* HP_ARCHITECTURE */

#ifdef	SUN_ARCHITECTURE
#define		XOR_CAPS_LOCK		/* xor caps lock and shift	*/
#define		MESSAGE_CAT		/* use message catalogs		*/
#define		USE_SRIOCSREDIR		/* use SRIOCSREDIR ioctl for -C	*/
#define		USE_STREAMS		/* use streams			*/
#define		USE_STREAMS_TTCOMPAT	/* use ttcompat streams mod	*/
#define		USE_STREAMS_BUFMOD	/* use bofmod streams mod	*/
#define		USE_TCSENDBREAK		/* use tiocbreak()		*/
#if        (_XOPEN_VERSION == 3)
#define         USE_SUN_WCWIDTH_PATCH
#define         wcwidth(w)  sun_wcwidth(w)
#endif  /* (_XOPEN_VERSION == 3) */
#endif	/* SUN_ARCHITECTURE */


#ifdef	IBM_ARCHITECTURE
/* this seems to be #define'ed in the world of imake... */
#ifndef		MESSAGE_CAT
#define		MESSAGE_CAT		/* use message catalogs		*/
#endif		/* MESSAGE_CAT */
#define		USE_TIOCCONS		/* use tioccons for -C		*/
#define		HAS_SETEUID		/* seteuid available		*/
#define		HAS_SETREUID		/* setreuid available		*/
#define		USE_PTYS		/* use ptys			*/
#define		USE_SETCSMAP		/* use setcsmap() for multi-byte*/
#define		USE_TCSENDBREAK		/* use tiocbreak()		*/
#endif	/* IBM_ARCHITECTURE */

#ifdef	USL_ARCHITECTURE
#define		XOR_CAPS_LOCK		/* xor caps lock and shift	*/
#define		MESSAGE_CAT		/* use message catalogs		*/
#define		USE_STREAMS		/* use streams			*/
#define		USE_STREAMS_TTCOMPAT	/* use ttcompat streams mod	*/
#define         USE_USL_WCWIDTH_PATCH
#define         wcwidth(w)  usl_wcwidth(w)
#define		USE_TCSENDBREAK		/* use tiocbreak()		*/
#endif	/* USL_ARCHITECTURE */

#ifdef	UXP_ARCHITECTURE
#define		XOR_CAPS_LOCK		/* xor caps lock and shift	*/
#define		MESSAGE_CAT		/* use message catalogs		*/
#define		USE_STREAMS		/* use streams			*/
#define		USE_STREAMS_TTCOMPAT	/* use ttcompat streams mod	*/
#define		USE_TCSENDBREAK		/* use tiocbreak()		*/
#endif	/* UXP_ARCHITECTURE */
#endif	/* _Dt_TermPrimOSDepI_h */
/* DON'T ADD ANYTHING AFTER THIS #endif... */
