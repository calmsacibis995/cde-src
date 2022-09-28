/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           copydialog.c
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    This file contains the routines that interact with the
 *                   Andrew File System
 *
 *   FUNCTIONS: aclMaxNameLen
 *		aclsMatch
 *		addNegativeUser
 *		addNormalUser
 *		authToChangeAcl
 *		copyAcl
 *		copyAclEntry
 *		deleteUsers
 *		getFileAcl
 *		lhRights
 *		maxAclEntries
 *		okToAddAclEntries
 *		okToUseNewuser
 *		prAcl
 *		rightsStr
 *		setFileAcl
 *		shRights
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "xacl.h"

/*
 * Defines
 */

#define   FOLLOWSYMLINKS 1        /* last arg in pioctl call */
#define NOFOLLOWSYMLINKS 0        /* last arg in pioctl call */


/*
 * Typedefs
 */

/* return values from okToUseNewuser() */
typedef enum {INVALID, DUPLICATE, VALID, ERROR} UserStatus;

/*
 * global variables
 */

#define EMSGMAX 256                      /* error message buffer */
char g_errorMessage[EMSGMAX];


/*
 * local function declarations
 */

#ifdef _NO_PROTO
static Boolean    okToAddAclEntries();
static UserStatus okToUseNewuser();
static void       copyAclEntry();
static String     rightsStr();
#else
static Boolean    okToAddAclEntries(AclDef *aclp);
static UserStatus okToUseNewuser(const String directory, AclEntry aclEntry[], int nEntries, String NewuserName);
static void       copyAclEntry  (AclEntry * entryOut, AclEntry * entryIn);
static String     rightsStr     (const RightsL rights);
#endif



/*-------------------------------------------------
 *
 *  int maxAclEntries
 *
 *  return the maximum number of entries in an ACL
 *
 *------------------------------------------------*/

int
#ifdef _NO_PROTO
maxAclEntries()
#else
maxAclEntries(void)
#endif

{

   return ACL_MAXENTRIES;

}  /* end maxAclEntries */


/*-----------------------------------------------------
 *
 *  int aclMaxNameLen
 *
 *  return the maximum length for a user or group name
 *
 *----------------------------------------------------*/

int
#ifdef _NO_PROTO
aclMaxNameLen()
#else
aclMaxNameLen(void)
#endif

{

   return PR_MAXNAMELEN;

}  /* end aclMaxNameLen */



/*--------------------------------------------------------------------------------
 *
 *  int getFileAcl
 *
 *  get the ACL from AFS for the input file
 *  the calling program is responsible for allocating the AclDef
 *  return 0 if ok, non-zero for error
 *
 *-------------------------------------------------------------------------------*/

int
#ifdef _NO_PROTO
getFileAcl(directory, aclp)
   const String    directory;
         AclDef  * aclp;
#else
getFileAcl(const String directory, AclDef *aclp)
#endif  /* _NO_PROTO */

{
   char acl[4096];                 /* buffer for acl output */
   char *scanp;                    /* pointer for scanning acl */
   int i, n;                       /* counters for scanning acl */
   int rc;                         /* return code */
   struct ViceIoctl io_record;     /* io record */


   /* get the acl from AFS */
   io_record.in       = NULL;
   io_record.in_size  = 0;
   io_record.out      = acl;
   io_record.out_size = sizeof(acl);

   rc = pioctl(directory, VIOCGETAL, &io_record, NOFOLLOWSYMLINKS);
   if (rc != 0)
      return rc;


   /* get the number of normal and negative acl entries */
   sscanf(acl, "%d %d%n", &aclp->nNorm, &aclp->nNeg, &n);
   scanp = acl + n;

   /* get normal rights */
   for (i = 0; i < aclp->nNorm; i++)
   {
      sscanf(scanp, "%s %d%n", aclp->norm[i].user, &aclp->norm[i].rights, &n);
      scanp += n;
      aclp->norm[i].shRights = shRights(aclp->norm[i].rights);
   }

   /* get negative rights */
   for (i = 0; i < aclp->nNeg; i++)
   {
      sscanf(scanp, "%s %d%n", aclp->neg[i].user, &aclp->neg[i].rights, &n);
      scanp += n;
      aclp->neg[i].shRights = shRights(aclp->neg[i].rights);
   }

   return 0;

}  /* end getFileAcl */



/*--------------------------------------------------------------------------------
 *
 *  int setFileAcl
 *
 *  set the AFS ACL for the input file
 *  if a file rather than directory is input, the ACL for the directory
 *     containing the file is set
 *  return 0 if ok, non-zero for error
 *
 *-------------------------------------------------------------------------------*/

int 
#ifdef _NO_PROTO
setFileAcl(directory, aclp)
   const String    directory;
   const AclDef  * aclp;
#else
setFileAcl(const String directory, const AclDef *aclp)
#endif  /* _NO_PROTO */

{
   char acl[4096];                 /* buffer for acl output */
   char *scanp;                    /* pointer for scanning acl */
   int i, n;                       /* counters for scanning acl */
   int rc;                         /* return code */
   struct ViceIoctl io_record;     /* io record */


   /* put the number of normal and negative acl entries */
   n = sprintf(acl, "%d\n%d\n", aclp->nNorm, aclp->nNeg);
   scanp = acl + n;

   /* put normal rights */
   for (i = 0; i < aclp->nNorm; i++)
   {
      n = sprintf(scanp, "%s\t%d\n", aclp->norm[i].user, aclp->norm[i].rights);
      scanp += n;
   }

   /* put negative rights */
   for (i = 0; i < aclp->nNeg; i++)
   {
      n = sprintf(scanp, "%s\t%d\n", aclp->neg[i].user, aclp->neg[i].rights);
      scanp += n;
   }

   /* send the acl to AFS */
   io_record.in       = acl;
   io_record.in_size  = strlen(acl) + 1;
   io_record.out      = NULL;
   io_record.out_size = 0;

   rc = pioctl(directory, VIOCSETAL, &io_record, NOFOLLOWSYMLINKS);

   return rc;

}  /* end setFileAcl */


/*-------------------------------------------------------------
 *
 *  authToChangeAcl
 *
 *  This uses an ugly & slow method to determine if you have
 *  privilege to change an acl: first it fetches the acl then
 *  resets it. It might be ugly and slow, but it's safe, doesn't
 *  need to directory query the protection database, and
 *  is unlikely to be tripped up by not coding for some
 *  subtlety in the way AFS treats ACL's.
 *
 *-----------------------------------------------------------*/

Boolean
#ifdef _NO_PROTO
authToChangeAcl(directory)
   const String directory;
#else
authToChangeAcl(const String directory)
#endif


{
   struct ViceIoctl io_record;     /* io record */
   long             rights = 0;
   int              rc;
   char acl[4096];                 /* buffer for acl output */


/* the following only indicates if you have explicit admin privilege
 * it will not tell you if you have implicit admin privilige by
 * owning the directory of by being in system:administrators
 *
 * setAdminister(rights);
 *
 * io_record.in       = (void *) &rights;
 * io_record.in_size  = sizeof(rights);
 * io_record.out      = NULL;
 * io_record.out_size = 0;
 *
 * rc = pioctl(directory, VIOCACCESS, &io_record, NOFOLLOWSYMLINKS);
 *
 * return (rc == 0);
 */

   /* get the acl from AFS */
   io_record.in       = NULL;
   io_record.in_size  = 0;
   io_record.out      = acl;
   io_record.out_size = sizeof(acl);

   /* assume that this runs ok */
   rc = pioctl(directory, VIOCGETAL, &io_record, NOFOLLOWSYMLINKS);

   /* send the acl back to AFS */
   io_record.in       = acl;
   io_record.in_size  = strlen(acl) + 1;
   io_record.out      = NULL;
   io_record.out_size = 0;

   rc = pioctl(directory, VIOCSETAL, &io_record, NOFOLLOWSYMLINKS);

   return (rc == 0);


}  /* end authToAdmin */



/*--------------------------------------------------------------------------------
 *
 *  RightsS shRights
 *
 *  convert from longhand rights to shorthand rights
 *  note that the definintion of shorthand rights is NOT the same as that used
 *     by AFS ... AFS uses read=rl write=rlidwk whereas here:
 *
 *  read=rl
 *  write=idwk
 *  administer=a
 *  advanced=the full rights cannot be described by a cominiation of the other
 *           shorthand rights
 *
 *-------------------------------------------------------------------------------*/

RightsS 
#ifdef _NO_PROTO
shRights(longhand)
   const RightsL longhand; 
#else
shRights(const RightsL longhand)
#endif  /* _NO_PROTO */

{
   RightsS shorthand = 0;
   
   if (ifRead   (longhand) && 
       ifLookup (longhand))       shorthand |= SH_READ;

   if (ifInsert (longhand) &&
       ifWrite  (longhand) &&
       ifDelete (longhand) &&
       ifLock   (longhand))       shorthand |= SH_WRITE; 

   if (ifAdminister(longhand))    shorthand |= SH_ADMINISTER;

   if ( ! ifSHRead(shorthand))
      if (ifRead  (longhand) ||
          ifLookup(longhand))     shorthand  = SH_ADVANCED;
           
   if ( ! ifSHWrite(shorthand)) 
      if (ifInsert(longhand) ||
          ifWrite (longhand) ||
          ifDelete(longhand) ||
          ifLock  (longhand))     shorthand  = SH_ADVANCED;
            
   return shorthand;
   
}  /* end shRights */



/*--------------------------------------------------------------------------------
 *
 *  RightsL lhRights
 *
 *  convert from shorthand rights to longhand rights
 *  note that the definintion of shorthand rights is NOT the same as that used
 *     by AFS ... AFS uses read=rl write=rlidwk whereas here:
 *
 *  read=rl
 *  write=idwk
 *  administer=a
 *  advanced=the full rights cannot be described by a cominiation of the other
 *           shorthand rights
 *
 *  If the shorthand rights are "advanced" it is not possible to map to longhand
 *  rights and a zero is returned. This function should never be invoked for this
 *  case.
 *
 *-------------------------------------------------------------------------------*/

RightsL 
#ifdef _NO_PROTO
lhRights(shorthand)
   const RightsS shorthand; 
#else
lhRights(const RightsS shorthand)
#endif  /* _NO_PROTO */

{
   RightsL longhand = 0;
   
   if (ifSHRead(shorthand))       longhand |= (PRSFS_READ   | 
                                               PRSFS_LOOKUP);
   
   if (ifSHWrite(shorthand))      longhand |= (PRSFS_INSERT | 
                                               PRSFS_WRITE  |
                                               PRSFS_DELETE | 
                                               PRSFS_LOCK); 
   
   if (ifSHAdminister(shorthand)) longhand |= PRSFS_ADMINISTER;
         
   return longhand;
   
}  /* end lhRights */


/*-------------
 *
 *  copyAcl
 *
 *-----------*/

void
#ifdef _NO_PROTO
copyAcl(to, from)
   AclDef        * to;
   const AclDef  * from;
#else
copyAcl(AclDef *to, const AclDef *from)
#endif

{

   int i;

   if (to == from)
      return;

   to->nNorm = from->nNorm;
   for (i = 0; i < from->nNorm; i++)
   {
      strncpy(to->norm[i].user, from->norm[i].user, PR_MAXNAMELEN);
      to->norm[i].rights   = from->norm[i].rights;
      to->norm[i].shRights = from->norm[i].shRights;
   }

   to->nNeg = from->nNeg;
   for (i = 0; i < from->nNeg; i++)
   {
      strncpy(to->neg[i].user, from->neg[i].user, PR_MAXNAMELEN);
      to->neg[i].rights   = from->neg[i].rights;
      to->neg[i].shRights = from->neg[i].shRights;
   }

}  /* end copyAcl */


/*---------------------------------------------------------------
 *
 *  addNormalUser
 *
 *  add a user to the normal ACL
 *  see the analagous function addNegativeUser
 *  this does not change the ACL on the file but just updates
 *     data in *aclp
 *  if no more entries will fit in the ACL, *aclp is not changed
 *
 *--------------------------------------------------------------*/

void
#ifdef _NO_PROTO
addNormalUser(directory, aclp, name, lhRights)
   const String    directory;
   AclDef        * aclp;
   const String    name;
   const RightsL   lhRights;
#else
addNormalUser(const String directory, AclDef *aclp, const String name, const RightsL lhRights)
#endif

{
   int     i;
   String  entriesMsg = GETMESSAGE(1, 24, "The AFS Permissions list cannot have more than %d entries\n");
   String  badUserMsg = GETMESSAGE(1, 25, "\"%s\" is not known to AFS");
   String  dupUserMsg = GETMESSAGE(1, 26, "%s is a duplicate name\nThe permissions for the existing entry will be replaced");
   String  cantCheckMessage = GETMESSAGE(1, 54, "Unable to check the validity of \"%s\".\nEither I can't contact the AFS server or the ACL is full.");

   /* ignore empty names */
   if (strlen(name) > 0)
   {
      /* display an error if there is no more room in ACL */
      if (okToAddAclEntries(aclp))
      {
         /* validate new user or group */
         switch (okToUseNewuser(directory, aclp->norm, aclp->nNorm, name))
         {
            case VALID:
               strncpy(aclp->norm[aclp->nNorm].user, name, PR_MAXNAMELEN);
               aclp->norm[aclp->nNorm].rights   = lhRights;
               aclp->norm[aclp->nNorm].shRights = shRights(lhRights);
               aclp->nNorm++;
               break;
            case INVALID:
               sprintf (g_errorMessage, badUserMsg, name);
               displayError(g_errorMessage);
               break;
            case DUPLICATE:
               for (i = 0; i < aclp->nNorm; i++)
                  if (strcmp(aclp->norm[i].user, name) == 0)
                  {
                     aclp->norm[i].rights = lhRights;
                     aclp->norm[i].shRights = shRights(lhRights);
                     sprintf(g_errorMessage, dupUserMsg, name);
                     displayWarning(g_errorMessage);
                  }
               break;
            case ERROR:
               sprintf (g_errorMessage, cantCheckMessage, name);
               displayError(g_errorMessage);
               break;
         }
      }
      else  /* the acl is full */
      {
         sprintf(g_errorMessage, entriesMsg, ACL_MAXENTRIES);
         displayError(g_errorMessage);
      }
   }

}  /* end addNormalUser */


/*---------------------------------------------------------------
 *
 *  addNegativeUser
 *
 *  add a user to the negative ACL
 *  see the analagous function addNormalUser
 *  this does not change the ACL on the file but just updates
 *     data in *aclp
 *  if no more entries will fit in the ACL, *aclp is not changed
 *
 *--------------------------------------------------------------*/

void
#ifdef _NO_PROTO
addNegativeUser(directory, aclp, name, lhRights)
   const String    directory;
   AclDef        * aclp;
   const String    name;
   const RightsL   lhRights;
#else
addNegativeUser(const String directory, AclDef *aclp, const String name, const RightsL lhRights)
#endif

{
   int     i;
   String  entriesMsg = GETMESSAGE(1, 24, "The Access Control List cannot have more than %d entries\n");
   String  badUserMsg = GETMESSAGE(1, 25, "\"%s\" is not known to AFS");
   String  dupUserMsg = GETMESSAGE(1, 26, "%s is a duplicate name\nThe permissions for the existing entry will be replaced");
   String  cantCheckMessage = GETMESSAGE(1, 54, "Unable to check the validity of \"%s\".\nEither I can't contact the AFS server or the ACL is full.");

   /* ignore empty names */
   if (strlen(name) > 0)
   {
      /* display an error if there is no more room in ACL */
      if (okToAddAclEntries(aclp))
      {
         /* validate new user or group */
         switch (okToUseNewuser(directory, aclp->neg, aclp->nNeg, name))
         {
            case VALID:
               strncpy(aclp->neg[aclp->nNeg].user, name, PR_MAXNAMELEN);
               aclp->neg[aclp->nNeg].rights   = lhRights;
               aclp->neg[aclp->nNeg].shRights = shRights(lhRights);
               aclp->nNeg++;
               break;
            case INVALID:
               sprintf (g_errorMessage, badUserMsg, name);
               displayError(g_errorMessage);
               break;
            case DUPLICATE:
               for (i = 0; i < aclp->nNeg; i++)
                  if (strcmp(aclp->neg[i].user, name) == 0)
                  {
                     aclp->neg[i].rights   = lhRights;
                     aclp->neg[i].shRights = shRights(lhRights);
                     sprintf(g_errorMessage, dupUserMsg, name);
                     displayWarning(g_errorMessage);
                  }
               break;
            case ERROR:
               sprintf (g_errorMessage, cantCheckMessage, name);
               displayError(g_errorMessage);
               break;
         }
      }
      else  /* the acl is full */
      {
         sprintf(g_errorMessage, entriesMsg, ACL_MAXENTRIES);
         displayError(g_errorMessage);
      }
   }

}  /* end addNegativeUser */



/*---------------------------------------------------------------
 *
 *  deleteUsers
 *
 *  delete users from an ACL
 *
 *  posToDel is an array giving the elements to delete
 *     because it was generated from a List widget, the indices
 *     are numbered 1-n rather than the C-array numbering
 *     of 0-(n-1)
 *
 *  The list of items not to be deleted are copied to a temporary
 *     array. When finished, this temp array is copied back
 *     over the original. This is not the most effecient way to
 *     do this, but the arrays are never over 20 elements long
 *     and rarely have more than 5 or so elements.
 *
 *--------------------------------------------------------------*/

void
#ifdef _NO_PROTO
deleteUsers(normalAcl, posToDel, numToDel, aclp)
   const Boolean   normalAcl;
   const int       posToDel[];
   const int       numToDel;
   AclDef        * aclp;
#else
deleteUsers(const Boolean normalACL, const int posToDel[], const int numToDel, AclDef * aclp)
#endif

{
   int        numUsers;
   AclEntry   *userList, myList[ACL_MAXENTRIES];
   int        i, j, k;

   if (numToDel == 0) return;

   if (normalAcl)
   {
      numUsers = aclp->nNorm;
      userList = aclp->norm;
   }
   else
   {
      numUsers = aclp->nNeg;
      userList = aclp->neg;
   }

   /* i points at the original list of users */
   /* j points at the list of positions which are to be deleted */
   /* k points at the new list of users */
   for (i = 0, j = 0, k = 0; i < numUsers; i++)
   {    
      if ((i != (posToDel[j]-1)) || (j >= numToDel) )
      {
         copyAclEntry(&myList[k], &userList[i]);
         k++;
      }
      else 
         if (i == (posToDel[j]-1))
            j++;
   }

   numUsers = numUsers - numToDel;
   if (normalAcl)
      aclp->nNorm = numUsers;
   else
      aclp->nNeg  = numUsers;

   for (i = 0; i < numUsers; i++)
      copyAclEntry(&userList[i], &myList[i]);

}  /* end deleteUser */

/*------------------------------------------------------------------
 *
 *  aclsMatch
 *
 *  compare two acl's, return TRUE if they are the same
 *  assume that users/groups in the two are in the same order
 *
 *-----------------------------------------------------------------*/

Boolean
#ifdef _NO_PROTO
aclsMatch(acl1, acl2)
   AclDef  acl1, acl2;
#else
aclsMatch(AclDef acl1, AclDef acl2)
#endif

{
   int i;

   if ( (acl1.nNorm != acl2.nNorm) || (acl1.nNeg != acl2.nNeg))
      return FALSE;

   for (i = 0; i < acl1.nNorm; i++)
      if ( (strcmp(acl1.norm[i].user,acl2.norm[i].user) != 0) ||
           (acl1.norm[i].rights != acl2.norm[i].rights) )
         return FALSE;

   for (i = 0; i < acl1.nNeg; i++)
      if ( (strcmp(acl1.neg[i].user,acl2.neg[i].user) != 0) ||
           (acl1.neg[i].rights != acl2.neg[i].rights) )
         return FALSE;

   return TRUE;

}  /* end aclsMatch */



/*------------------------------------------------------------------
 *
 *  okToAddAclEntries
 *
 *  display an error dialog and return false if the ACL is full
 *
 *----------------------------------------------------------------*/

static Boolean
#ifdef _NO_PROTO
okToAddAclEntries(aclp)
   AclDef *aclp;
#else
okToAddAclEntries(AclDef *aclp)
#endif

{

   return ( (aclp->nNorm + aclp->nNeg) < ACL_MAXENTRIES );

}  /* end okToAddAclEntries */


/*------------------------------------------------------------------
 *
 *  copyAclEntry
 *
 *----------------------------------------------------------------*/

static void
#ifdef _NO_PROTO
copyAclEntry(entryOut, entryIn)
   AclEntry * entryOut, * entryIn;
#else
copyAclEntry(AclEntry * entryOut, AclEntry * entryIn)
#endif

{

   strncpy(entryOut->user, entryIn->user, PR_MAXNAMELEN);
   entryOut->rights   = entryIn->rights;
   entryOut->shRights = entryIn->shRights;


}  /* end copyAclEntry */


/*------------------------------------------------------------------
 *
 *  okToUseNewuser
 *
 *  is the new user valid?
 *  possibilities are: user not valid
 *                     user valid and already in ACL
 *                     user valid
 *                     unable to check user's validity
 *
 *----------------------------------------------------------------*/

static UserStatus
#ifdef _NO_PROTO
okToUseNewuser(directory, aclEntry, nEntries, NewuserName)
   const String  directory;
   AclEntry      aclEntry[];
   int           nEntries;
   String        NewuserName;
#else
okToUseNewuser(const String directory, AclEntry aclEntry[], int nEntries, String NewuserName)
#endif

{
   int       i;
   RightsL   read   = 0;
   RightsS   shRead = 0;
   AclDef    originalAcl;
   AclDef    trialAcl;

   /* look for duplicate entries */
   for (i = 0; i < nEntries; i++)
      if (strcmp(aclEntry[i].user,NewuserName) == 0)
         return DUPLICATE;

   /* is user (or group) valid? */
   /*    do this by trial & error; get the current ACL, add the new user, */
   /*    and try to set the updated ACL */

   /* if the current ACL can't be obtained, or if the ACL is full, then   */
   /*    we can't check the validity of the new user */
   if (getFileAcl(directory, &originalAcl) != 0) 
      return ERROR;
   if ((originalAcl.nNorm + originalAcl.nNeg) >= maxAclEntries())
      return ERROR;

   copyAcl(&trialAcl, &originalAcl);
   strncpy(trialAcl.norm[trialAcl.nNorm].user,NewuserName,PR_MAXNAMELEN);
   read   = setRead(read);
   shRead = setSHRead(shRead);
   trialAcl.norm[trialAcl.nNorm].rights   = read;
   trialAcl.norm[trialAcl.nNorm].shRights = shRead;
   trialAcl.nNorm++;
   if (setFileAcl(directory, &trialAcl) == 0)
      setFileAcl(directory, &originalAcl);
   else
      return INVALID;

   /* no duplicates and name is in pdb */
   return VALID;


}  /* end okToUseNewuser */


/*--------------------------------------------------------------------------------
 *
 *  prAcl
 *
 *  print ACL to stdout ... primarily for debugging
 *
 *-------------------------------------------------------------------------------*/

void
#ifdef _NO_PROTO
prAcl(aclp)
   const AclDef  * aclp;
#else
prAcl(const AclDef *aclp)
#endif  /* _NO_PROTO */

{
   int  i;
   char rightsS[4];

   printf ("%d positive ACL entries:\n", aclp->nNorm);
   for (i = 0; i < aclp->nNorm; i++)
   {
     rightsS[0] = '\0';
     if (ifSHRead(aclp->norm[i].shRights))       strcat(rightsS,"R");
     if (ifSHWrite(aclp->norm[i].shRights))      strcat(rightsS,"W");
     if (ifSHAdminister(aclp->norm[i].shRights)) strcat(rightsS,"A");
     if (ifSHAdvanced(aclp->norm[i].shRights))   strcat(rightsS,"V");
     printf("   \"%s\"  0x%x  %s  %s  sh2lh: 0x%x\n",
              aclp->norm[i].user,
              aclp->norm[i].rights,
              rightsStr(aclp->norm[i].rights),
              rightsS,
              lhRights(aclp->norm[i].shRights));
   }

   printf ("%d negative ACL entries:\n", aclp->nNeg);
   for (i = 0;  i < aclp->nNeg; i++)
   {
     rightsS[0] = '\0';
     if (ifSHRead(aclp->neg[i].shRights))       strcat(rightsS,"R");
     if (ifSHWrite(aclp->neg[i].shRights))      strcat(rightsS,"W");
     if (ifSHAdminister(aclp->neg[i].shRights)) strcat(rightsS,"A");
     if (ifSHAdvanced(aclp->neg[i].shRights))   strcat(rightsS,"V");

     printf("   \"%s\"   0x%x  %s  %s  sh2lh: 0x%x\n",
              aclp->neg[i].user,
              aclp->neg[i].rights,
              rightsStr(aclp->neg[i].rights),
              rightsS,
              lhRights(aclp->neg[i].shRights));
   }

}  /* end prAcl */


/*-------------------------------------------------------------------------------
 *
 * rightsStr
 *
 * return a string of the form "rlidwka" given the numeric value of the acl rights
 * the string is stored in static storage so will not survive multiple calls
 *
 *--------------------------------------------------------------------------------*/

static String
rightsStr (const RightsL rights)

{
   static char s[8];

   s[0] = '\0';
   if (ifRead(rights))       strcat(s,"r");
   if (ifLookup(rights))     strcat(s,"l");
   if (ifInsert(rights))     strcat(s,"i");
   if (ifWrite(rights))      strcat(s,"w");
   if (ifDelete(rights))     strcat(s,"d");
   if (ifLock(rights))       strcat(s,"k");
   if (ifAdminister(rights)) strcat(s,"a");

   return &s;

}  /* end rightsStr */

