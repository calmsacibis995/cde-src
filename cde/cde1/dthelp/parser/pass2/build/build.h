/* $XConsortium: build.h /main/cde1_maint/1 1995/07/17 22:05:53 drk $ */
/*
              Copyright 1986 Tandem Computers Incorporated.
This product and information is proprietary of Tandem Computers Incorporated.
                   Copyright (c) 1986, 1987, 1988, 1989 Hewlett-Packard Co.
*/

/* Build.h is the primary include file for allocation of global variables
   for program BUILD. */


#if defined(BUILDEF)
#define BUILDEXTERN
#define BUILDINIT(a) = a
#else
#define BUILDEXTERN extern
#define BUILDINIT(a)
#endif

#include <stdio.h>
#include "basic.h"
#include "trie.h"
#include "common.h"

/* Defined types */
typedef struct arc ARC ;
typedef struct state STATE ;
typedef struct tree TREE ;
typedef struct statelist STATELIST ;
typedef struct parameter PARAMETER ;
typedef struct exception EXCEPTION ;
typedef struct ptype PTYPE ;
typedef struct eltstruct ELTSTRUCT ;
typedef struct lhsstruct LHS ;
typedef struct stack STACK ;
typedef struct srefstruct SREFSTRUCT ;
typedef struct srefdata SREFDATA ;
typedef struct mapstruct MAP ;
typedef struct andgroup ANDGROUP ;

/* Each item in a list of states contains value, a pointer to a state;
   next, a pointer to the next item in the list; and level, which
   indicates the level of nested subexpressions within the regular expression
   when the state was added to the FSA. */
BUILDEXTERN struct statelist {
  STATE *value ;
  STATELIST *next ;
  int level ;
  } ;

struct stack {
  STACK *oldtop ;
  STATELIST *starts ;
  STATELIST *finals ;
  STATELIST *allfinal ;
  STATELIST *newfinal ;
  ARC *starta ;
  } ;

BUILDEXTERN STACK bot
#if defined(BUILDEF)
  = {
  NULL, NULL, NULL, NULL, NULL, NULL
  }
#endif
  ;

BUILDEXTERN STACK *top BUILDINIT(&bot) ;

BUILDEXTERN int stacklevels BUILDINIT(0) ;


/* Used to report nondeterminism in and groups */
#define ANDCONFLICT 1
#define DATACONFLICT 2
#define ELTCONFLICT 3

/* Name of an element, and of the first element on the left-hand side of
   the current rule */
#define thisrule lhs->elt->enptr
 
/* Set declared entity type */
#define SETETYPE(p,t) adddefent(p) ; entity->type = t

/* Definitions for entities */

#define EMPTYMAP 1

#define ERREXIT 1

/* Prefix for default short reference map and entities */
#define M_PREFIX "M-"

/* State transition network generated by CONTEXT */
#define sparse
#include "sparse.h"

/* Function prototypes */
#include "proto.h"

/*  Frequently output strings */
#define ndif "}\n#endif\n  ;\n\n" 
#define SUBONE else fputs("[1] ;\n\n", dtd) ;

/* An array used to indicate which characters can appear within
   element names */
#if defined(BUILDEF)
#include "chartype.h"
#else
extern M_CHARTYPE m_ctarray[M_CHARSETLEN] ;
#endif

/* Current state of the finite-state machine which controls BUILD */
BUILDEXTERN int curcon BUILDINIT(RULE) ;
BUILDEXTERN int m_token ;
BUILDEXTERN M_WCHAR scanval ;

/* Used to pass names and literals from the scanner */
BUILDEXTERN M_WCHAR name[M_NAMELEN + 1] ;
BUILDEXTERN M_WCHAR literal[M_LITLEN + 1] ;

/* Scanner buffer for read-ahead */
BUILDEXTERN int toundo BUILDINIT(0) ;
BUILDEXTERN int maxundo BUILDINIT(0) ;
/* Size of scanner buffer for read-ahead */
#define SAVECHAR 50
BUILDEXTERN int savechar[SAVECHAR] ;

/* Whether errors have occurred */
BUILDEXTERN LOGICAL errexit BUILDINIT(FALSE) ;

/* FSA storage */

/* Each state in an FSA is represented by a flag indicating whether it is
   a final state, a pointer to a list of transitions from the state,
   and a flag that indicates whether parsed character data is possible
   from the state */
BUILDEXTERN struct state {
  LOGICAL final ;
  LOGICAL datacontent ;
  LOGICAL frompcdata ;
  ARC *first ;
  int count ;
  STATE *next ;
  } ;
BUILDEXTERN STATE *firststate BUILDINIT(NULL) ;
BUILDEXTERN STATE **nextstate BUILDINIT(&firststate) ;
BUILDEXTERN STATE *startstate ;

/* Each arc in an FSA is an element in the arc list emanating from a
   particular state.  Associated fields include label, a pointer to
   the name of the element labelling the arc (garbage if the label is an
   and group); optional, a flag indicating whether or not the label is
   contextually required; minim, a flag indicating whether or not the arc
   corresponds to an element selected by explicit minimization;
   and group, a pointer to the list of FSA's making
   up the andgroup if the arc's label is an andgroup; to, a pointer to the
   state entered by the arc; next, a pointer to the next arc
   in the current arc list; and finally, id, a pointer to the node in the
   tree representation of the model corresponding to the arc (id is used
   in the detection of ambiguity to differentiate between arcs with the same
   label corresponding to one model token and those with the same label
   corresponding to more than one model token).
   */
BUILDEXTERN struct arc {
  ELTSTRUCT *label ;
  LOGICAL optional ;
  LOGICAL minim ;
  ANDGROUP *group ;
  STATE *to ;
  ARC *next ;
  int id ;
  } ;

/* Andgroups are elements in a list of the FSA's corresponding to the
content submodels in an andgroup.  Each list element contains a pointer
to the start state of the FSA and a pointer to the next element in the
list */
BUILDEXTERN struct andgroup{
  STATE *start ;
  ANDGROUP *nextptr ;
  ANDGROUP *next ;
  int count ;
  } ;
BUILDEXTERN ANDGROUP *firstand BUILDINIT(NULL) ;
BUILDEXTERN ANDGROUP **nextand BUILDINIT(&firstand) ;

/* Tree representation of a rule */
/* Each node in the tree representation corresponds to a submodel.
   Associated with the node is the submodel's connector and occurrence
   indicator; terminal, a flag indicating whether the submodel in turn
   has submodels; value, the element of which a terminal submodel consists;
   pointers first and last to the first and last submodel of the node;
   and right pointing to the node's next sibling in the tree. */
struct tree {
  int connector, occurrence ;
  ELTSTRUCT *value ;
  LOGICAL terminal ;
  LOGICAL minim ;
  TREE *first, *right, *parent ;
  int eltid ;
  } ;
BUILDEXTERN TREE *ruletree BUILDINIT(NULL) ;
BUILDEXTERN TREE *curtree ;

/* Various counters */
BUILDEXTERN int eltsinrule BUILDINIT(0) ;
BUILDEXTERN int stateused BUILDINIT(0) ;
BUILDEXTERN int andused BUILDINIT(0) ;

/* Element names on the left-hand side of the current rule */
struct lhsstruct {
  ELTSTRUCT *elt ;
  LHS *next ;
  } ;
BUILDEXTERN LHS *lhs BUILDINIT(NULL) ;
BUILDEXTERN LHS **nextlhs BUILDINIT(&lhs) ;

/* Content type (regular expression, ANY, NONE, CDATA, RCDATA) of the
   current rule. */
BUILDEXTERN int contype ;

/* Indicates whether <PARAM>, <USEMAP>, or <MIN> fields have occurred in
   current rule */
BUILDEXTERN LOGICAL poccur, uoccur, moccur ;

/* Indicates whether an ID parameter for this rule has occurred */
BUILDEXTERN LOGICAL idoccur ;

/* Short reference map for this rule */
BUILDEXTERN int srefp ;
BUILDEXTERN LOGICAL useoradd ;

/* Whether start- and end-tag minimization are permitted for the current
   rule */
BUILDEXTERN LOGICAL smin, emin ;

/* Suffix for generated entity names */
BUILDEXTERN int egensuf ;

/* Storage for exceptions */
BUILDEXTERN int excount BUILDINIT(0) ;
struct exception {
  int element ;
  EXCEPTION *next ;
  EXCEPTION *nextptr ;
  } ;
BUILDEXTERN EXCEPTION *firstex BUILDINIT(NULL) ;
BUILDEXTERN EXCEPTION **nextex BUILDINIT(&firstex) ;
BUILDEXTERN EXCEPTION *exlist ;

/* Total number of parameters for all elements */
BUILDEXTERN int parcount BUILDINIT(0) ;

/* Primary representation of parameters */
struct parameter {
  M_WCHAR *paramname ;
  int type ;
  int kwlist ;
  PTYPE *ptypep ;
  int deftype ;
  int defval ;
  M_WCHAR *defstring ;
  /* Next parameter for this element */
  PARAMETER *next ; 
  /* Link for list of all parameters for all elements */
  PARAMETER *nextptr ; 
  } ;
BUILDEXTERN PARAMETER *newpar BUILDINIT(NULL) ;
BUILDEXTERN PARAMETER *plist BUILDINIT(NULL) ;
BUILDEXTERN PARAMETER *firstpar BUILDINIT(NULL) ;
BUILDEXTERN PARAMETER **nextpar BUILDINIT(&firstpar) ;

/* Storage for parameter names */
BUILDEXTERN int pnamelen BUILDINIT(0) ;

/* Storage for parameter values that are named keywords */
BUILDEXTERN int ptypelen BUILDINIT(0) ;
struct ptype {
  M_WCHAR *keyword ;
  int next ;
  PTYPE *nextptr ;
  } ;
BUILDEXTERN PTYPE *firstptype BUILDINIT(NULL) ;
BUILDEXTERN PTYPE *thisptype ;
BUILDEXTERN PTYPE **nextptype BUILDINIT(&firstptype) ;
BUILDEXTERN int kwlen BUILDINIT(0) ;

/* Storage for parameter default values */
BUILDEXTERN int deflen BUILDINIT(0) ;

/* Characters needed for element names */
BUILDEXTERN int enamelen BUILDINIT(0) ;

/* Description of an element: */
struct eltstruct {
  /* Pointer to element name */  
  M_WCHAR *enptr ;
  /* Start state of element's content model's FSA */
  STATE *model ;
  /* Type of element's content (Regular expression, ANY, RCDATA, etc) */
  int content ;
  /* Pointer to element's inclusion list */
  int inptr ;
  /* Pointer to element's exclusion list */
  int exptr ;
  /* Pointer to element's parameter list */
  PARAMETER *parptr ;
  /* Index in output of first item in element's parameter list */
  int parindex ;
  /* Number of element's parameters */
  int paramcount ;
  /* Whether start- and end-tag minimization are permitted */
  LOGICAL stmin, etmin ;
  /* Pointer to short reference map, if any */
  int srefptr ;
  /* TRUE for USEMAP, FALSE for ADDMAP */
  LOGICAL useoradd ;

  ELTSTRUCT *next ;
  int eltno ;
  } ;
BUILDEXTERN ELTSTRUCT *firstelt BUILDINIT(NULL) ;
BUILDEXTERN ELTSTRUCT **nextelt BUILDINIT(&firstelt) ;

/* Most number of parameters had by a single element */
BUILDEXTERN int maxpar BUILDINIT(0) ;
/* Number of parameters current element has */
BUILDEXTERN int pcount ;

BUILDEXTERN FILE *dtd ;
BUILDEXTERN FILE *m_errfile ;

/* Line number in input file */
BUILDEXTERN int m_line BUILDINIT(1) ;

/* Options */
/* Scan trace */
BUILDEXTERN LOGICAL scantrace BUILDINIT(FALSE) ;
/* Trace calls to malloc/free */
BUILDEXTERN LOGICAL m_malftrace BUILDINIT(FALSE) ;
/* Test validity of heap in calls to malloc/free */
BUILDEXTERN LOGICAL m_heapchk BUILDINIT(FALSE) ;
/* Flag set if strictly conforming to ISO 8879 */
BUILDEXTERN LOGICAL standard BUILDINIT(FALSE) ;

/* File pointer used by utilities in ..\util\scanutil for input file */
BUILDEXTERN FILE *ifile ;

/* Element names are stored in a trie. */  
BUILDEXTERN M_TRIE eltree ;

/* Count of elements */
BUILDEXTERN int ecount BUILDINIT(0) ;

/* Storage for short reference map names and definitions */
BUILDEXTERN M_TRIE maptree ;
BUILDEXTERN M_TRIE sreftree ;
BUILDEXTERN int mapcnt BUILDINIT(0) ;
BUILDEXTERN int curmap ;
struct srefstruct {
  int srefcnt ;
  SREFDATA *data ;
  SREFSTRUCT *next ;
  } ;
BUILDEXTERN SREFSTRUCT *firstsref BUILDINIT(NULL) ;
BUILDEXTERN SREFSTRUCT **nextsref BUILDINIT(&firstsref) ;
struct srefdata {
  int map ;
  int entidx ;
  SREFDATA *next ;
  } ;
BUILDEXTERN SREFDATA *thissref ;
BUILDEXTERN int sreflen BUILDINIT(0) ;
BUILDEXTERN M_WCHAR mapname[M_NAMELEN + 1] ;
BUILDEXTERN int maxsr ;
BUILDEXTERN int maxseq ;
struct mapstruct {
  int map ;
  M_WCHAR *mapname ;
  LOGICAL defined ;
  MAP *nextptr ;
  } ;
BUILDEXTERN MAP *firstmap BUILDINIT(NULL) ;
BUILDEXTERN MAP **nextmap BUILDINIT(&firstmap) ;


/* Storage for entities */
BUILDEXTERN M_TRIE m_enttrie[1] ;
BUILDEXTERN M_ENTITY *entity ;
BUILDEXTERN M_ENTITY *lastent BUILDINIT(NULL) ;
BUILDEXTERN M_ENTITY *firstent BUILDINIT(NULL) ;
BUILDEXTERN M_WCHAR entcontent[M_LITLEN + 1] ;
BUILDEXTERN int entclen ;
BUILDEXTERN int m_entcnt BUILDINIT(0) ;
BUILDEXTERN FILE *entfile ;

BUILDEXTERN LOGICAL entrule BUILDINIT(FALSE) ;

BUILDEXTERN M_WCHAR genname[M_NAMELEN + sizeof(M_PREFIX)] ;

M_WCHAR *MakeWideCharString(
#if defined(M_PROTO)
  const char *from
#endif
  );
