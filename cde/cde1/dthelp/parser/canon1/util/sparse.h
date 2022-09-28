/* $XConsortium: sparse.h /main/cde1_maint/1 1995/07/17 21:30:54 drk $ */
/* Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co. */

/* Sparse.h contains definitions which allow selection of the sparse
   matrix output option of CONTEXT */

#if defined(sparse)
#define m_newcon(i, j) m_sprscon(i, j)

int m_sprscon(
#if defined(M_PROTO)
  int i, int j
#endif
  ) ;

#else
#define m_newcon(i, j) m_nextcon[i][j]
#endif


