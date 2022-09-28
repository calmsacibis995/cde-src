/*
 * File:	hashtable.C $Revision: 1.2 $
 *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company
 * (c) Copyright 1993, 1994 International Business Machines Corp.
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.
 * (c) Copyright 1993, 1994 Novell, Inc.
 */
//
// The default `bump' function for <hashtable.h>.
//
// The `prime' array is a sequence of primes such that prime[0] = 11 and
// for all 0 < i < N, prime[i] is the smallest prime number greater
// than 2 * prime[i - 1].  Requires an N > 1.
//
// The new hashsize is the smallest prime[i] which is greater than twice
// the current hashsize.  If there is no such prime[i], the new hashsize
// is prime[N - 1].

const N = 27;

static unsigned prime[N] =
{
	11,
	23,
	47,
	97,
	197,
	397,
	797,
	1597,
	3203,
	6421,
	12853,
	25717,
	51437,
	102877,
	205759,
	411527,
	823117,
	1646237,
	3292489,
	6584983,
	13169977,
	26339969,
	52679969,
	105359939,
	210719881,
	421439783,
	842879579
};

unsigned __hashtable_bump(unsigned hs)
{
	if (hs < prime[N - 2])
		for (int i = 0; i < N; i++)
			if (prime[i] > 2 * hs)
				return prime[i];

	return prime[N - 1];
}
