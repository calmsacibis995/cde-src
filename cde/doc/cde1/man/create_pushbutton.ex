.\" **
.\" **
.\" **  (c) Copyright 1989 by Open Software Foundation, Inc.
.\" **      All Rights Reserved.
.\" **
.\" **  (c) Copyright 1987, 1988, 1989, by Hewlett-Packard Company
.\" **
.\" **  (c) Copyright 1987, 1988 by Digital Equipment Corporation,
.\" **      Maynard, MA.  All Rights Reserved.
.\" **
.\" **
.TH XmCreatePushButton 3X "" "" "" ""
.ds )H Hewlett-Packard Company
.ds ]W Motif Release 1.1: August 1990
.ds )H Hewlett-Packard Company
.ds ]W Motif Release 1.1: August 1990
.SH NAME
\fBXmCreatePushButton \- the PushButton widget creation function.\fP
.iX "XmCreatePushButton"
.iX "creation functions" "XmCreatePushButton"
.sp .5
.SH SYNOPSIS
\fB
\&#include <Xm/PushB.h>
.sp .5
Widget XmCreatePushButton (
.br
.ta 0.75i 1.75i
	Widget	\fIparent\fP,
.br
	String	\fIname\fP,
.br
	ArgList	\fIarglist\fP,
.br
	Cardinal	\fIargcount\fP)
.fi
\fP
.SH DESCRIPTION
\fBXmCreatePushButton\fP creates an instance of a PushButton 
widget and returns the associated widget ID.  
.sp .5
.IP "\fIparent\fP" .75i
Specifies the parent widget ID
.IP "\fIname\fP" .75i
Specifies the name of the created widget
.IP "\fIarglist\fP" 1.00i
Specifies the argument list
.IP "\fIargcount\fP" 1.00i
Specifies the number of attribute/value pairs in the argument list
(\fIarglist\fP)
.sp .5
.PP
For a complete definition of PushButton and its associated resources, see
\fBXmPushButton(3X)\fP.
.sp .5
.SH RETURN VALUE
Returns the PushButton widget ID.
.sp .5
.SH RELATED INFORMATION
\fBXmPushButton(3X)\fP.
.sp .5
