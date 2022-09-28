/* $Header: version.c,v 1.2 91/12/10 15:28:26 xbuild Exp $
 *
 * $Log:	version.c,v $
 * Revision 1.2  91/12/10  15:28:26  15:28:26  xbuild (See Marc Ayotte)
 * "Dec 10 1991 mit patch  craig & marca "
 * 
 * Revision 2.0  86/09/17  15:40:11  lwall
 * Baseline for netwide release.
 * 
 */

#include "EXTERN.h"
#include "common.h"
#include "util.h"
#include "INTERN.h"
#include "patchlevel.h"
#include "version.h"

/* Print out the version number and die. */

void
version()
{
    extern char rcsid[];

#ifdef lint
    rcsid[0] = rcsid[0];
#else
    fatal3("%s\nPatch level: %d\n", rcsid, PATCHLEVEL);
#endif
}
