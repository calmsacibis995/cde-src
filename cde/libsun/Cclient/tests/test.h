/* @(#)test.h	1.1 97/04/23 */

#ifndef _TEST_H_
#define _TEST_H_
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern	caddr_t		openAndMmap(const char *fn, 
				    int o_mode,
				    struct stat *sbuf);

#endif /* _TEST_H_ */
