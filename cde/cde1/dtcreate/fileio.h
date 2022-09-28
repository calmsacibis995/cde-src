/* $XConsortium: fileio.h /main/cde1_maint/2 1995/10/02 18:02:09 lehors $ */
/***************************************************************************/
/*                                                                         */
/*  fileio.h                                                               */
/*                                                                         */
/***************************************************************************/

#ifndef _FILEIO_H_INCLUDED
#define _FILEIO_H_INCLUDED

#ifndef STORAGECLASS
#ifdef  NOEXTERN
#define STORAGECLASS
#else
#define STORAGECLASS extern
#endif
#endif

/***************************************************************************/
/*                                                                         */
/*  Prototypes for functions                                               */
/*                                                                         */
/***************************************************************************/
#ifdef _NO_PROTO

ushort  WriteDefinitionFile();
ushort  WriteActionFile();
Boolean check_file_exists();
int     OpenDefinitionFile();
ushort  CopyIconFiles();
char *  CreateIconName();
Boolean ActionHasIcon ();
Boolean FiletypeHasIcon ();

#else /* _NO_PROTO */

ushort  WriteDefinitionFile(char *, ActionData *);
ushort  WriteActionFile(ActionData *);
Boolean check_file_exists(char *);
int     OpenDefinitionFile(char *, ActionData *);
ushort  CopyIconFiles(ActionData *);
char *  CreateIconName(char *, char *, enum icon_size_range, char *, Boolean);
Boolean ActionHasIcon (void);
Boolean FiletypeHasIcon (FiletypeData *);

#endif /* _NO_PROTO */

#endif /* _FILEIO_H_INCLUDED */
