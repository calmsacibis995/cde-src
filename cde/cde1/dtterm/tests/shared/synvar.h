/* $XConsortium: synvar.h /main/cde1_maint/1 1995/07/18 01:49:34 drk $ */

/******************************************************************
 *
 * (c) Copyright Hewlett-Packard Company, 1993.
 *
 ******************************************************************/

#include   <stdio.h>
#include   <X11/Xlib.h>
#include   <synlib/synlib.h>

typedef struct {
    Display       *display;
    int            waitTime;
    SynFocus      *focusMap;
} TestData, *TestDataPtr;


#define WIN_WAIT_TIME       120
#define MULTI_CLICK_DELAY   5

#define IMAGE_DIR           "../image/"
#define EXPECTED_SUFFIX     ".exp"
#define ACTUAL_SUFFIX       ".act"

#define NEED_LEN            256

#define IMAGE_FILE_LEN      100  /* should be strlen(IMAGE_DIR) + 20 (say) */


#define TERM_EMU            "dtterm"
