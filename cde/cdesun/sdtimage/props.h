/*
 * @(#)props.h 1.6 96/03/07
 *
 * Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 */

#ifndef PROPS_H
#define PROPS_H

#define DS_FILENAME ".desksetdefaults"

#define VIEW_GRAY_SCALE          "GrayScale"
#define VIEW_COLOR               "Color"

#define COLORS_BW                "BW"             /*  1-bit */
#define COLORS_16                "16"             /*  4-bit */
#define COLORS_256               "256"            /*  8-bit */
#define COLORS_THOUSANDS         "Thousands"      /* 16-bit */
#define COLORS_MILLIONS          "Millions"       /* 24-bit */

#define DISPLAY_PALETTE          "True"
#define UNDISPLAY_PALETTE        "False"

#define USE_DSC                  "True"		  /* Faster */
#define NO_DSC                   "False"	  /* Slower, default */

#define PS_COLOR                 "Color"
#define PS_MONO                  "Monochrome"

#define DEFAULT_VIEW_IN 	 VIEW_COLOR
#define DEFAULT_COLORS           COLORS_256
#define DEFAULT_SHOW_PALETTE	 DISPLAY_PALETTE
#define DEFAULT_DSC              NO_DSC
#define DEFAULT_PS_VIEW          PS_MONO
#define DEFAULT_TT_TIMEOUT       "300"

#define RES_VIEW_IN         0
#define RES_COLORS          1
#define RES_SHOW_PALETTE    2
#define RES_DSC             3
#define RES_PS_VIEW         4
#define RES_TT_TIMEOUT      5

typedef struct {
        char              *name;
        char              *value;
} ResourceInfo;

typedef struct {
        char *        	   view_in;
	char *             color;
	int	           show_palette;
	int                use_dsc;
	char *             ps_view;
	int                tt_timeout;
	int		   props_changed;
} PropInfo;

extern PropInfo   *current_props;

/* Function prototypes */
extern int 		read_props();
extern void             props_size_panel();

#endif /* _PROPS_H */
