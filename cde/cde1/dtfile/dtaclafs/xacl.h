/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           xacl.h
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    header file for xacl.c ... AFS ACL functions
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef XACL_H
#define XACL_H
    

/*-----------------
 * Include Section 
 *-----------------*/

#include <sys/types.h>
#include <netinet/in.h>           /* for struct in_addr used in venus.h */
#include <afs/vice.h>
#include <afs/venus.h>
#include <afs/acl.h>              /* for ACL_MAXENTRIES */
#include <afs/ptint.h>            /* for PR_MAXNAMELEN */
#include <afs/prs_fs.h>           /* for rights masks */
#include <Xm/Xm.h>                /* for type Boolean */


/*-----------------
 * Defines Section 
 *-----------------*/

/* masks for shorthand rights */
#define SH_READ       1
#define SH_WRITE      2
#define SH_ADMINISTER 4
#define SH_ADVANCED   8

/* convenience macros to read AFS (longhand) rights */
#define ifRead(lhRights)          ((lhRights) & (PRSFS_READ))
#define ifWrite(lhRights)         ((lhRights) & (PRSFS_WRITE))
#define ifInsert(lhRights)        ((lhRights) & (PRSFS_INSERT))
#define ifLookup(lhRights)        ((lhRights) & (PRSFS_LOOKUP))
#define ifDelete(lhRights)        ((lhRights) & (PRSFS_DELETE))
#define ifLock(lhRights)          ((lhRights) & (PRSFS_LOCK))
#define ifAdminister(lhRights)    ((lhRights) & (PRSFS_ADMINISTER))

/* convenience macros to set AFS (longhand) rights */
#define setRead(lhRights)         ((lhRights) |= PRSFS_READ)
#define setWrite(lhRights)        ((lhRights) |= PRSFS_WRITE)
#define setInsert(lhRights)       ((lhRights) |= PRSFS_INSERT)
#define setLookup(lhRights)       ((lhRights) |= PRSFS_LOOKUP)
#define setDelete(lhRights)       ((lhRights) |= PRSFS_DELETE)
#define setLock(lhRights)         ((lhRights) |= PRSFS_LOCK)
#define setAdminister(lhRights)   ((lhRights) |= PRSFS_ADMINISTER)

/* convenience macros for shorthand rights */
#define ifSHRead(shRights)        ((shRights) & (SH_READ))
#define ifSHWrite(shRights)       ((shRights) & (SH_WRITE))
#define ifSHAdminister(shRights)  ((shRights) & (SH_ADMINISTER))
#define ifSHAdvanced(shRights)    ((shRights) & (SH_ADVANCED))

/* convenience macros to set shorthand rights */
#define setSHRead(shRights)       ((shRights) |= SH_READ)
#define setSHWrite(shRights)      ((shRights) |= SH_WRITE)
#define setSHAdminister(shRights) ((shRights) |= SH_ADMINISTER)
#define setSHAdvanced(shRights)   ((shRights) |= SH_ADVANCED)

/* macro to get message catalog string */
#ifdef MESSAGE_CAT
#ifdef __ultrix
#define _CLIENT_CAT_NAME "dtaclafs.cat"
#else  /* __ultrix */
#define _CLIENT_CAT_NAME "dtaclafs"
#endif /* __ultrix */
#define GETMESSAGE(set, number, string)\
    ((char *) _DtGetMessage(_CLIENT_CAT_NAME, set, number, string))
#else /* MESSAGE_CAT */
#define GETMESSAGE(set, number, string)\
    string
#endif /* MESSAGE_CAT */

 
/*-----------------
 * Typedef Section 
 *-----------------*/

typedef int RightsS;              /* shorthand rights */
typedef int RightsL;              /* longhand rights */

typedef struct {
  char user[PR_MAXNAMELEN];       /* user name */
  RightsL rights;                 /* flags for longhand access rights */
  RightsS shRights;               /* flags for shorthand access rights */
} AclEntry;

typedef struct {
  int nNorm, nNeg;                 /* no of nromal and negative acl entries */
  AclEntry norm[ACL_MAXENTRIES];   /* normal access rights */
  AclEntry neg[ACL_MAXENTRIES];    /* negative access rights */
} AclDef;


/* -----------------------------
 * Function Declaration Section
 *------------------------------*/

#ifdef _NO_PROTO
extern int      getFileAcl();
extern int      setFileAcl();
extern RightsS  shRights();
extern RightsL  lhRights();
extern int      maxAclEntries();
extern int      aclMaxNameLen();
extern void     copyAcl();
extern void     addNormalUser();
extern void     addNegativeUser();
extern void     prAcl();
extern Boolean  authToChangeAcl();
extern Boolean  aclsMatch();
extern void     deleteUsers();
#else
extern int      getFileAcl(const String path, AclDef *aclp);
extern int      setFileAcl(const String path, const AclDef *aclp);
extern RightsS  shRights(const RightsL longhand);
extern RightsL  lhRights(const RightsS shorthand);
extern int      maxAclEntries(void);
extern int      aclMaxNameLen(void);
extern void     copyAcl(AclDef *to, const AclDef *from);
extern void     addNormalUser(AclDef *aclp, const String name, const RightsL lhRights);
extern void     addNegativeUser(AclDef *aclp, const String name, const RightsL lhRights);
extern void     prAcl(const AclDef *aclp);
extern Boolean  authToChangeAcl(const String path);
extern Boolean  aclsMatch(AclDef acl1, AclDef acl2);
extern void     deleteUsers(const Boolean normalACL, 
                            const int posToDel[], 
                            const int numToDel, 
                            AclDef * aclp)
#endif  /* _NO_PROTO */


#endif  /* XACL_H */
