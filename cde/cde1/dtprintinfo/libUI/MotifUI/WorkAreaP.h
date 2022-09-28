/* $XConsortium: WorkAreaP.h /main/cde1_maint/2 1995/10/23 10:24:29 gtsang $ */
/*                                                                      *
 * (c) Copyright 1993, 1994 Hewlett-Packard Company                     *
 * (c) Copyright 1993, 1994 International Business Machines Corp.       *
 * (c) Copyright 1993, 1994 Sun Microsystems, Inc.                      *
 * (c) Copyright 1993, 1994 Novell, Inc.                                *
 */

#ifndef WORKAREAP_H
#define WORKAREAP_H

#include <Xm/ManagerP.h>

#include "WorkArea.h"

#ifdef __cplusplus
extern "C" {
#endif
 
typedef struct _WorkAreaClassPart {
    int         ignore;
} WorkAreaClassPart;

typedef struct _WorkAreaClassRec {
    CoreClassPart       core_class;
    CompositeClassPart  composite_class;
    ConstraintClassPart constraint_class;
    XmManagerClassPart  manager_class;
    WorkAreaClassPart     workArea_class;
} WorkAreaClassRec;

externalref WorkAreaClassRec workAreaClassRec;

typedef struct _WorkAreaPart {
    /* public resources */
    Dimension      horizontal_spacing;
    Dimension      vertical_spacing;
    Dimension      line_thickness;
    Dimension      line_offset;
    Dimension      node_line_length;
    unsigned char  alignment;
    unsigned char  vertical_alignment;
    unsigned char  packing;
    Boolean	   auto_resize_width;
    Boolean	   is_list;

    /* private resources */
    Dimension      old_width;
    Dimension      old_height;
    Boolean	   delay_layout;	/* Set with GuiWorkAreaDoLayout function */
    GC             gc;
    Widget         workArea_root;
} WorkAreaPart;

typedef struct _WorkAreaRec {
    CorePart        core;
    CompositePart   composite;
    ConstraintPart  constraint;
    XmManagerPart   manager;
    WorkAreaPart        workArea;
}  WorkAreaRec;

typedef struct _WorkAreaConstraintsPart {
    /* public resources */
    Widget        super_node;
    Boolean	  is_workArea;
    unsigned char orientation;
    unsigned char attachment;
    Boolean	  is_opened;
    Boolean       is_transient;
    WidgetList    sub_nodes;	/* Can only be queried.  Can be set indirectly
				   with the GuiWorkAreaReorderChildren function */
    int           n_sub_nodes;
    int           n_columns;	/* Used when orientation is horizontal. Zero
				   means there are infinite columns */
    Position      x, y; 	/* Can be set if is_transient == TRUE */
} WorkAreaConstraintsPart;

typedef struct _WorkAreaConstraintsRec {
    WorkAreaConstraintsPart workArea;
} WorkAreaConstraintsRec, *WorkAreaConstraints;

#define WORKAREA_CONSTRAINT(w) ((WorkAreaConstraints)((w)->core.constraints))

#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* WORKAREAP_H */
