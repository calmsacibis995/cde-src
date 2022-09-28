/**/
#ifndef _VALID_H_
#define _VALID_H_

#include "UnixDefs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void	VALID(char * start,
		      char ** eol,
		      int  * timeOffset,
		      int  * zoneOffset,
		      int  * rfc822Valid);

#ifdef __cplusplus
}
#endif

#endif /*_VALID_H_*/

