/* $XConsortium: common.h /main/cde1_maint/1 1995/07/18 01:48:03 drk $ */
#define  NULLCHAR         '\0'
#define  BLANK            ' '

typedef enum {
   DECModeSet=1, DECModeReset=2, DECModeSave=3, DECModeRestore=4
} DECModeType;
