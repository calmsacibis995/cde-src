/* @(#)testIndexFile.c	1.2 97/05/01 */

#include "../c-client/mail.h"
#include "../c-client/solaris.h"
#include "../c-client/VALID.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "indexVersion.h"
#include "test.h"


int
main(int argc, char **argv)
{
  boolean_t		error = _B_FALSE;
  
  INDEXHDR		indexHeader;
  INDEXENTRY		indexEntry;

  const int		indexHdrSize = sizeof(INDEXHDR);
  const int		indexEntrySize = sizeof(INDEXENTRY);

  int			indexFd;
  int			folderFd;
  int			currentMsgNumber;
  caddr_t		indexMmap;
  caddr_t		folderMmap;

  struct stat		indexStat;
  struct stat		folderStat;

  const char	*	indexSuffix = "_dt_index";
  char			opt;
  char		*	folderFileName = NULL;
  char		*	indexFileName = NULL;
  char		*	fromLine;
  char		*	nl;
  char		*	lastSlash;

  while ((opt = getopt(argc, argv, "f:")) != EOF) {
    switch(opt) {

    case 'f':
      folderFileName = optarg;
      break;

    case '?':
    default:
      error = _B_TRUE;
      break;

    }
  }
  if (error || folderFileName == NULL) {
    printf("%s: FAILED: usage '%s -f <folder-name>\n",
	   argv[0], argv[0]);
    exit(1);
  }

  indexFileName = malloc(strlen(folderFileName) + strlen(indexSuffix) + 2);
  lastSlash = strrchr(folderFileName, '/');

  if (lastSlash != NULL) {
    strncpy(indexFileName, folderFileName, lastSlash - folderFileName + 1);
    strcat(indexFileName, ".");
    strcat(indexFileName, lastSlash+1);
    strcat(indexFileName, indexSuffix);
  } else {
    sprintf(indexFileName, ".%s%s", folderFileName, indexSuffix);
  }

  if ((indexMmap = openAndMmap(indexFileName,
			       O_RDONLY,
			       &indexStat)) == MAP_FAILED) {
    exit(1);
  }
    
  if ((folderMmap = openAndMmap(folderFileName,
				O_RDONLY,
				&indexStat)) == MAP_FAILED) {
    exit(1);
  }

  memcpy(&indexHeader, indexMmap, sizeof(INDEXHDR));

  if (strcmp(INDEXVERSION, indexHeader.index_version) != 0) {
    printf("%s FAILED: indexVersion does not match!\n");
    exit(2);
  }

  for (currentMsgNumber = 1
	 ; currentMsgNumber <= indexHeader.nmsgs
	 ; currentMsgNumber++) {

    memcpy(&indexEntry,
	   indexMmap + indexHdrSize + (indexEntrySize * (currentMsgNumber-1)),
	   sizeof(INDEXENTRY));

    fromLine = &folderMmap[indexEntry.seek_offset];
    nl = strchr(fromLine, '\n');
    if (nl != NULL) {
      *nl = '\0';
    }
    printf("Message %u: %.80s\n",  currentMsgNumber, fromLine);
  }

  return(0);
}
