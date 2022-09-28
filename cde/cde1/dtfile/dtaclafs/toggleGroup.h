/************************************<+>*************************************
 ****************************************************************************
 *
 *   FILE:           toggleGroup.h
 *
 *   COMPONENT_NAME: dtaclafs - File awareness dialog for 'afs'
 *
 *   DESCRIPTION:    header file for toggleGroup.c ... functions to maintain
 *                   a collection of widgets representing AFS access rights.
 *
 *   (c) Copyright 1993, 1994, 1995 Hewlett-Packard Company
 *   (c) Copyright 1993, 1994, 1995 International Business Machines Corp.
 *   (c) Copyright 1993, 1994, 1995 Sun Microsystems, Inc.
 *   (c) Copyright 1993, 1994, 1995 Novell, Inc.
 *
 ****************************************************************************
 ************************************<+>*************************************/

#ifndef TOGGLEGROUP_H
#define TOGGLEGROUP_H
 

/*-----------------
 * Include Section
 *-----------------*/

#include "xacl.h"


/*-----------------
 * Defines Section
 *-----------------*/

#define NADVRIGHTS 7           /* number of AFS rights, advanced mode */
#define NBASRIGHTS 3           /* number of AFS rights, basic mode    */

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

typedef enum {BASIC, ADVANCED} DialogMode;

typedef struct {
   DialogMode  mode;                 /* are these basic or advanced mode toggles? */
   Widget      manager;              /* a manager widget which holds ...  */
   Widget      rights[NADVRIGHTS];   /* ... a set of toggle gadgets ... */
   Widget      advLabel;             /* ... and a label for basic mode for adv rights */
} ToggleGroup;


/* -----------------------------
 * Function Declaration Section
 *------------------------------*/

#ifdef _NO_PROTO
extern ToggleGroup * TGNew();
extern Widget        TGmanager();
extern void          TGsetToggles();
extern RightsL       TGgetRights();
extern void          TGdesensitize();
#else
extern ToggleGroup * TGNew(Widget parent, Boolean advanced);
extern Widget        TGmanager(ToggleGroup * TG);
extern void          TGsetToggles(ToggleGroup * advTG, RightsL advRights);
extern RightsL       TGgetRights(ToggleGroup *TG);
extern void          TGdesensitize(ToggleGroup * TG);
#endif

#endif /* TOGGLEGROUP_H */
