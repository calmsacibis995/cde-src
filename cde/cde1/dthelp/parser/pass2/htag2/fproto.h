/* $XConsortium: fproto.h /main/cde1_maint/1 1995/07/17 22:09:53 drk $ */
/* Copyright (c) 1988, 1989, 1990 Hewlett-Packard Co. */
/* Function prototypes for HP Tag/TeX translator */


int m_lower(
#if defined(M_PROTO)
  int c
#endif
  ) ;

void options(
#if defined(M_PROTO)
  LOGICAL filelenonly
#endif
  ) ;

void setopt(
#if defined(M_PROTO)
  char *string, LOGICAL filelenonly
#endif
  ) ;

LOGICAL setvalopt(
#if defined(M_PROTO)
  int thisopt, char *string, LOGICAL filelenonly
#endif
  ) ;

char *strstr(
#if defined(M_PROTO)
  const char *s1, const char *s2
#endif
  ) ;

int m_upper(
#if defined(M_PROTO)
  int c
#endif
  ) ;

char *MakeMByteString(
#if defined(M_PROTO)
  const M_WCHAR *from
#endif
  );

M_WCHAR *MakeWideCharString(
#if defined(M_PROTO)
  const char *from
#endif
  );

void PutString(
#if defined(M_PROTO)
  char *string
#endif
  ) ;

void PutWString(
#if defined(M_PROTO)
  M_WCHAR *string
#endif
  ) ;

void PutWChar(
#if defined(M_PROTO)
  M_WCHAR wchar
#endif
  ) ;

void SaveWChar(
#if defined(M_PROTO)
  M_WCHAR wchar
#endif
  ) ;

void OpenTag(
#if defined(M_PROTO)
  LOGICAL hasAttributes,
  LOGICAL newlineOK
#endif
  ) ;

void CloseTag(
#if defined(M_PROTO)
  LOGICAL newlineOK
#endif
  ) ;

void ImpliedlAttribute(
#if defined(M_PROTO)
  char    *name,
  M_WCHAR *value
#endif
  ) ;

void RequiredAttribute(
#if defined(M_PROTO)
  char    *name,
  M_WCHAR *value
#endif
  ) ;

M_WCHAR *CopyWString(
#if defined(M_PROTO)
  M_WCHAR *string
#endif
  ) ;

void MakeNewElement(
#if defined(M_PROTO)
  ElementTypes type
#endif
  ) ;

void DeleteElement(
#if defined(M_PROTO)
  ElementPtr pElement
#endif
  ) ;

void OptimizeAndEmit(
#if defined(M_PROTO)
  ElementPtr pVirpage
#endif
  ) ;

void PushCurrentElement(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

void PopCurrentElement(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

char *mb_realloc(
#if defined(M_PROTO)
    char *ptr, long size
#endif
  );

char *mb_malloc(
#if defined(M_PROTO)
    long size
#endif
  );

void mb_free(
#if defined(M_PROTO)
    char **pptr
#endif
  );

void EmitSDL(
#if defined(M_PROTO)
  ElementPtr pVirpage
#endif
  ) ;

int OpenFile(
#if defined(M_PROTO)
  char *name, int type, int code
#endif
  ) ;

int ReadFile(
#if defined(M_PROTO)
  int fd, char *name, char *buffer, int amount, int code
#endif
  ) ;

int WriteFile(
#if defined(M_PROTO)
  int fd, char *name, char *buffer, int amount, int code
#endif
  ) ;

int FileExists(
#if defined(M_PROTO)
  char *name
#endif
  ) ;

void AssertFileIsReadable(
#if defined(M_PROTO)
  char *name, int code
#endif
  ) ;

int FileSize(
#if defined(M_PROTO)
  char *name, int code
#endif
  ) ;

void CloseFile(
#if defined(M_PROTO)
  int fd, char *name, int code
#endif
  ) ;

FILE *FopenFile(
#if defined(M_PROTO)
  char *name, int type, int code
#endif
  ) ;

int GetALine(
#if defined(M_PROTO)
  FILE *file, char *name, char *line, int max, int code
#endif
  ) ;

int FreadFile(
#if defined(M_PROTO)
  FILE *file, char *name, char *buffer, int amount, int code
#endif
  ) ;

void FwriteFile(
#if defined(M_PROTO)
  FILE *file, char *name, char *buffer, size_t amount, int code
#endif
  ) ;

void FcloseFile(
#if defined(M_PROTO)
  FILE *file, char *name, int code
#endif
  ) ;

int StringToUpper(
#if defined(M_PROTO)
  char *string
#endif
  ) ;

void BuildIndex(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

void OpenDocument(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

void CloseDocument(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

void UpdateTossFileName(
#if defined(M_PROTO)
  M_NOPAR
#endif
  ) ;

void SetLocale(
#if defined(M_PROTO)
  M_WCHAR *pLang, M_WCHAR *pCharset
#endif
  ) ;
