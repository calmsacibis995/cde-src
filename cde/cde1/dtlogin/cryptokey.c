/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */
/*
 * xdm - display manager daemon
 *
 * $XConsortium: cryptokey.c /main/cde1_maint/4 1995/11/28 09:38:05 rcs $
 *
 * Copyright 1988 Massachusetts Institute of Technology
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose and without fee is hereby granted, provided
 * that the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  M.I.T. makes no representations about the
 * suitability of this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 *
 * Author:  Keith Packard, MIT X Consortium
 */

# include   <X11/Xauth.h>
# include   <X11/Xos.h>

/*
 * crypto key.  Generate cryptographically secure random numbers
 */

static long	key[2];

#ifndef DONT_USE_DES
# ifndef USE_CRYPT
#  ifdef AIXV3
#   define USE_CRYPT
#  endif
#  ifdef ultrix
#   define USE_CRYPT
#  endif
#  ifdef hpux
#   define USE_CRYPT
#  endif
#  ifdef macII
#   define USE_CRYPT
#  endif
#  ifdef sun
#   define USE_CRYPT
#   if (OSMAJORVERSION >= 4)
     /* avoid strange sun crypt hackery */
#    define crypt _crypt
#   endif
#  endif
# endif
#endif

#if !defined (DONT_USE_DES) && !defined (USE_CRYPT)
# define USE_ENCRYPT
#endif

#ifdef USE_ENCRYPT
static
bitsToBytes (bits, bytes)
unsigned long	bits[2];
char	bytes[64];
{
    int	bit, byte;
    int	i;

    i = 0;
    for (byte = 0; byte < 2; byte++)
	for (bit = 0; bit < 32; bit++)
	    bytes[i++] = ((bits[byte] & (1 << bit)) != 0);
}
#endif

void
#ifdef _NO_PROTO
GenerateCryptoKey( auth, len )
        char *auth ;
        int len ;
#else
GenerateCryptoKey( char *auth, int len )
#endif /* _NO_PROTO */
{
    long    data[2];

#ifdef ITIMER_REAL
    {
	struct timeval  now;
	struct timezone zone;
	gettimeofday (&now, &zone);
	data[0] = now.tv_sec;
	data[1] = now.tv_usec;
    }
#else
    {
	long    time ();

	data[0] = time ((long *) 0);
	data[1] = getpid ();
    }
#endif
#ifdef DONT_USE_DES
    {
    	int	    seed;
    	int	    value;
    	int	    i;
    
    	seed = (data[0] + key[0]) +
	      ((data[1] + key[1]) << 16);
    	srand (seed);
    	for (i = 0; i < len; i++)
    	{
	    value = rand ();
	    auth[i] = value & 0xff;
    	}
	value = len;
	if (value > sizeof (key))
	    value = sizeof (key);
    	bcopy (auth, (char *) key, value);
    }
#else
#ifndef USE_ENCRYPT
    {
    	int	    i, j, k;
    	char    *result, *crypt ();
	char	cdata[9];
	long	sdata;
    
	for (j = 0; j < 2; j++)
	{
	    sdata = data[j];
	    for (i = 0; i < 4; i++)
	    {
		k = j * 4 + i;
		cdata[k] = sdata & 0xff;
		if (cdata[k] == 0)
		    cdata[k] = 1;
		sdata >>= 8;
	    }
	}
	cdata[8] = '\0';
	for (i = 0; i < len; i += 4)
	{
	    result = crypt (cdata, (const char *) key);
	    k = 4;
	    if (i + k > len)
		k = len - i;
	    for (j = 0; j < k; j++)
		auth[i + j] = result[2 + j];
	    for (j = 0; j < 8; j++)
		cdata[j] = result[2 + j];
	}
    }
#else
    {
    	char    key_bits[64];
    	char    data_bits[64];
    	int	    bit;
    	int	    i;
    
    	bitsToBytes (key, key_bits);
    	bitsToBytes (data, data_bits);
    	setkey (key_bits);
    	for (i = 0; i < len; i++) {
	    auth[i] = 0;
	    for (bit = 1; bit < 256; bit <<= 1) {
	    	encrypt (data_bits, 0);
	    	if (data_bits[bit])
		    auth[i] |= bit;
	    }
    	}
    }
#endif
#endif
}

# define FILE_LIMIT	1024	/* no more than this many buffers */

static
sumFile (name, sum)
char	*name;
long	sum[2];
{
    long    data[2];
    long    buf[1024*2];
    int	    cnt;
    int	    fd;
    int	    loops;
    int	    reads;
    int	    i;


    fd = open (name, 0);

    if (!fd)
	return 0;

    /* Avoid mapped device registers sometimes found in 1st meg by starting 
       read in 2nd meg of memory */

    lseek(fd, (off_t) 1024*1024, SEEK_SET);

    reads = FILE_LIMIT;
    while ((cnt = read (fd, buf, sizeof (buf))) > 0 && --reads > 0) {
	loops = cnt / sizeof (data);
	for (i = 0; i < loops; i+= 2) {
	    sum[0] += buf[i];
	    sum[1] += buf[i+1];
	}
    }

    close(fd);

    /*
     *  if a non-zero value was generated for either sum we'll take
     *  the result, otherwise we'll fail this approach so that
     *  we can try a backup method.
     */
    if ((sum[0]) || (sum[1])) {
      return 1;
    }
    else {
      return 0;
    }
}

static int  cryptoInited = 0;

int 
#ifdef _NO_PROTO
InitCryptoKey()
#else
InitCryptoKey( void )
#endif /* _NO_PROTO */
{
    char    *key_file = "/dev/mem";
    
    if (cryptoInited)
	return;

    /*
     *  If the sumFile fails to produce a result
     *  use the time of day.
     */
    if (!sumFile (key_file, key)) {

#ifdef ITIMER_REAL
      {
	struct timeval  now;
	struct timezone zone;
	gettimeofday (&now, &zone);
	key[0] = now.tv_sec;
	key[1] = now.tv_usec;
      }
#else
      {
	long    time ();
	
	key[0] = time ((long *) 0);
	key[1] = getpid ();
      }
#endif

    }
    cryptoInited = 1;
}
