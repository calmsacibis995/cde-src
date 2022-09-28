/* @(#)openAndMmap.c	1.1 97/04/23 */

#include "test.h"

/* 
 * Open the named file in the mode provided and
 * return the mmap() pointer to the data.
 *
 * sbuf has the results of a stat().
 */
caddr_t
openAndMmap(const char *fn, int openMode, struct stat *sbuf)
{
  int		fd = open(fn, openMode);
  int		prot = PROT_READ | PROT_WRITE;
  int		flags = MAP_PRIVATE;

  caddr_t	results = MAP_FAILED;

  if (fd < 0) {
    fprintf(stderr, "Can not open '%s' - ", fn);
    perror("open()");
  } else {

    if (fstat(fd, sbuf) < 0) {
      fprintf(stderr, "Can not stat file '%s' - ", fn);
      perror("stat()");
    } else {

      results = mmap(NULL, sbuf->st_size, prot, flags, fd, (off_t)0);
      
      if (results == MAP_FAILED) {
	fprintf(stderr, "Can not mmap file '%s' - ", fn);
	perror("mmap()");

      }
    }
  }
  if (fd > -1) {
    close(fd);
  }

  return(results);
}
