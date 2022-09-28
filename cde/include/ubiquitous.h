/*
 * This header file is included by ALL the source files used to build
 * any of the supported libraries.  It is provided so that things like
 * apollo/shlib.h which need to be included in the source for all shared
 * library object modules may be added/deleted in a single location.
 */

#ifdef __apollo
#  ifdef DOMAIN_ALLOW_MALLOC_OVERRIDE
#  include <apollo/shlib.h>
#  endif
#endif
