/* $XConsortium: paramu.c /main/cde1_maint/1 1995/07/17 22:56:36 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Paramu.c contains functions for programs ELTDEF and PARSER
   relevant to checking parameter values. */

/* Check to see if string is a valid value for parameter par.  If so
   return a pointer to the keyword if it's a keyword parameter and otherwise
   a pointer to string.  If not valid, return FALSE. */
#if defined(M_PROTO)
const M_WCHAR *m_partype(const int par , const M_WCHAR *string )
#else
M_WCHAR *m_partype(par, string)
  int par ;
  M_WCHAR *string ;
#endif /* M_PROTO */
  {
    LOGICAL first = TRUE ;
    int kw ;
    const M_WCHAR *p = string ;
    int len ;

    if (m_parameter[par - 1].type == M_KEYWORD) {
      for (kw = m_parameter[par - 1].kwlist ; kw ; kw = m_ptype[kw - 1].next)
        if (! m_wcupstrcmp(&m_keyword[m_ptype[kw - 1].keyword], string))
          return(&m_keyword[m_ptype[kw - 1].keyword]) ;
      return(FALSE) ;
      }
    if (m_parameter[par - 1].type == M_CDATT) return(string) ;
    if (! *string) return(FALSE) ;
    if (m_allwhite(string)) return(FALSE) ;
    len = 0 ;
    for ( ; *p ; p++)
      switch (m_cttype(*p)) {
        case M_NONNAME:
          switch (m_parameter[par - 1].type) {
            case M_ID:
            case M_IDRF:
            case M_NAMEPAR:
            case M_NMTOKEN:
            case M_NUMBER:
            case M_NUTOKEN:
            case M_ENTATT:
              return(FALSE) ;
            default:
              len = 0 ;
              if (! m_whitespace(*p)) return(FALSE) ;
            }
          first = TRUE ;
          break ;
        case M_NMSTART:
          len++ ;
          if (len > M_NAMELEN) return(FALSE) ;
          if (m_parameter[par - 1].type == M_NUMBER ||
              m_parameter[par - 1].type == M_NUMS) return(FALSE) ;
          first = FALSE ;
          break ;
        case M_NAMECHAR:
          len++ ;
          if (len > M_NAMELEN) return(FALSE) ;
          switch (m_parameter[par - 1].type) {
            case M_ID:
            case M_IDRF:
            case M_IDRFS:
            case M_NAMEPAR:
            case M_NAMES:
            case M_ENTATT:
              if (first) return(FALSE) ;
              break ;
            case M_NMTOKEN:
            case M_NMSTOKEN:
              break ;
            case M_NUMBER:
            case M_NUMS:
              return(FALSE) ;
            case M_NUTOKEN:
            case M_NUSTOKEN:
              if (first) return(FALSE) ;
              break ;
            default:
              m_error("Undefined parameter type") ;
              m_exit(TRUE) ;
            }
          break ;
        case M_DIGIT:
          len++ ;
          if (len > M_NAMELEN) return(FALSE) ;
          switch (m_parameter[par - 1].type) {
            case M_ID:
            case M_IDRF:
            case M_IDRFS:
            case M_NAMEPAR:
            case M_NAMES:
            case M_ENTATT:
              if (first) return(FALSE) ;
              break ;
            case M_NUMBER:
            case M_NUMS:
              break ;
            case M_NMTOKEN:
            case M_NMSTOKEN:
            case M_NUTOKEN:
            case M_NUSTOKEN:
              first = FALSE ;
              break ;
            default:
              m_error("Undefined parameter type") ;
              m_exit(TRUE) ;
            }
          break ;
        default:
          m_error("Undefined character type") ;
          m_exit(TRUE) ;
        }
    if (m_parameter[par - 1].type == M_ENTATT) {
      if ((M_ENTITY *) m_lookfortrie(string, m_enttrie))
        return(string) ;
      else return(FALSE) ;
      }
    return(string) ;
    }
