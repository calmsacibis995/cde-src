#!/bin/sh
# This script translates from the common CDE man source format into
# a form suitable for the regular troff -man macros.  See dt2xo
# for the corresponding script to produce X/Open specifications.
# 
# The sed script does the following:
#
#	Deletes all lines between .xos and .xoe and all beginning
#	with ".xo " (X/Open-only material).  Also deletes all company-
#	specific variants of these.
#
#	Adds new macro definitions to the head of the file.  These are:
#
#		.LI  A list item, turned into a .TP for -man.  (This is
#		     used directly by X/Open.)
#
#		.Lc  A list item corresponding to .Cn for a constant.
#
# The perl script includes comments.
#
# Mods:
#	.Ce fixed to return to font 1 and become a .PP (4/5/94 hlj).
#
#	Added .Hd macro for headers (4/6/94 hlj)
#
#	Fixed double translation of \| in SYNOPSIS (4/19/94 hlj)
#
#	Fixed indent problem with multiple embedded .VL lists (4/19/94 hlj)
#
#	Reduced indent of first .VL list (4/20/94 hlj)
#
#	Remove .mc diff-mark commands [X/Open only] (4/20/94 hlj)
#
#	Remove all lines between .iN and .sA [X/Open reviewer note]
#	(5/11/94 hlj)
#
#	Rearranged the initial sed order so that conditional text can
#	be the first line of the man page, if necessary (5/31/94 hlj)
#
#	Added command-line switches to select vendor-only versions
#	(6/8/94 hlj)
#
#	Added .iX macro translation (6/16/94 hlj)
#
#	Changed logic so that it scans for .TS, .LI, and .Lc and
#	adds the appropriate headers only when needed.  (6/30/94 hlj)
#
#	Added -c [act like cat if the magic string isn't there]
#	support. (7/8/94 hlj)
#
#	Fixed .LI-missing-tag bug. (7/12/94 hlj)
#
#	Added support for .tK macro. (7/27/94 hlj)
#
#	Changed output of .Hd macro, based on presence in SEE ALSO
#	or not. (8/22/94 hlj)
#
#	Fixed \- separator in NAME line so that catman still works.
#	(10/31/94 hlj)
#
Sed=/tmp/dt2man-sed.$$
Tmp1=/tmp/dt2man-tmp1.$$
Tmp2=/tmp/dt2man-tmp2.$$
trap "rm -f $Sed $Tmp1 $Tmp2; exit 0" 0 1 2 15
set -- `getopt chisu $@`
for arg in $*
do
	case $arg in
		-c)  Check=yes;  shift;;
		-h)  Vendor=hp;  shift;;
		-i)  Vendor=ibm; shift;;
		-s)  Vendor=sun; shift;;
		-u)  Vendor=usl; shift;;
		--) shift; break;;
	esac
done

# Put all the input into a temp file for scanning
cat "$@" > $Tmp1

# In -c mode, scan to see if this is a converted source file.
# If not, merely cat it and then exit without conversions.
if [ "x$Check" = xyes ]; then
	if grep '^\.\\" CDE Common Source Format, Version 1.0.0' \
		$Tmp1 >/dev/null; then
		:
	else
		cat $Tmp1
		exit 0
	fi
fi

# Build a file of sed commands to process all the combinations
# of vendor-, CDE-, and X/Open-specific code.
(if [ "x$Vendor" != x ]; then
	echo "s/^\\.$Vendor //"
	echo "/^\\.$Vendor/ d"
fi
if [ "x$Vendor" != xhp ]; then
	echo "/^\\.hps/,/^\.hpe/ d"
	echo "/^\\.hp / d"
fi
if [ "x$Vendor" != xibm ]; then
	echo "/^\\.ibms/,/^\.ibme/ d"
	echo "/^\\.ibm / d"
fi
if [ "x$Vendor" != xsun ]; then
	echo "/^\\.suns/,/^\.sune/ d"
	echo "/^\\.sun / d"
fi
if [ "x$Vendor" != xusl ]; then
	echo "/^\\.usls/,/^\.usle/ d"
	echo "/^\\.usl / d"
fi) > $Sed

# Do the initial preprocessing: conditional text and a couple of
# mods that happen to be more convenient in sed than perl.
sed -f $Sed $Tmp1 | soelim | sed '
/^\.xos/,/^\.xoe/ d
/^\.xo /d
/^\.cde[se]/d
s/^\.cde //
/^\.iN/,/^\.sA/ d
/^\.yS/,/^\.yE/ {
	s/\([^\\]\)|/\1\\|\\(bv\\|/g;
	s/^|/\\(bv/
}
/^\.mc/d
/^\.Bl/d
/SEE ALSO/a\
.na
' > $Tmp2

# Scan the resulting file for special troff needs:  Is tbl required?
# Are either of the two special macro definitions required?
# Then start the main perl script to complete the processing.
(
if grep '^\.TS' $Tmp2 >/dev/null; then
	# if it is using tbl, start with a special comment needed at Sun
	echo "'"\\'"' t
fi
if grep '^\.LI' $Tmp2 >/dev/null; then
	echo .de LI
	echo .\\'"' simulate -mm .LIs by turning them into .TPs
	echo '.TP \\\\n()Jn'
	echo '\\\\$1'
	echo ..
fi
if grep '^\.Lc' $Tmp2 >/dev/null; then
	echo .de Lc
	echo .\\'"' version of .LI that emboldens its argument
	echo '.TP \\\\n()Jn'
	# this kludge is to get around echos that turn \f into linefeed:
	echo @s-1@f3@@\$1@f1@s+1 | tr @ \\\\
	echo ..
fi
cat $Tmp2
) | perl -e '
$\ = "\n";	# set "print" to append newline

sub stripit {
	# Subroutine to strip out some troffisms and create collation
	# entries for .iX processing.
	($string) = @_;
	$stripped = $string;
	# remove point-size changes:
	$stripped =~ s/\\s.1//g;
	$stripped =~ s/\\s0//g;
	# For font changes, strip the \f strings, but append _<font#>
	# to create unique collation strings:
	if ($string =~ /\\f(.)/) {
		$font = $1;
		$stripped =~ s/\\f.//g;
		$stripped = sprintf("%s_%s", $stripped, $font);
	}
	# make collation string all lowercase:
	$stripped =~ tr/[A-Z]/[a-z]/;
	return $stripped;
}

while (<>) {
	chop;
	if (/^\.TH/) {
		# record the volume number used in the .TH
		($cmd, $name, $volume, $rest) = split;
		$volume =~ s/[a-zA-Z]//; # remove any letter suffix
		$vlcount = 0;
		$inSEEALSO = 0;
		print $_;
		# write the .BH "date" command, too:
		s/^\.TH +([^ ]+) +([^ ]+) +"([^"]+)".*/.BH "\3"/;
		print $_;
		next;
	}
	if (/^\.SH.*SEE ALSO/) {
		$inSEEALSO = 1;
		print ".SH \"SEE ALSO\"";
		next;
	}
	if (/^\.yS/) {
		# set up fonts and fill for the Synopsis;
		# note different fill for volume 1 vs others
		print ".ft 3";
		if ($volume == 1) {
			print ".fi";
			print ".na";
		} else {
			print ".nf";
		}
		next;
	}
	# The following 5 ifs are kludges to approximate the -mm
	# variable lists and the X/Open code sample macros.  They
	# rely on the -man macros keeping track of relative indents
	# using .RS/.RE, but a stack of .VL indents is required
	# because the .VLs can be nested.  The .P is translated to
	# .LI \0 to match the -mm macro usage of .P within a list;
	# it does not reset the indentation.
	if (/^\.VL/) {
		$previndent = $indent;
		($cmd, $indent, $rest) = split;
		if ($indent =~ /n$/) {
			chop($indent);
		}
		$indent = ($indent) ? $indent : 8;
		push(@indentarray, $indent);
		$vlcount++;
		print ".PP";
		if ($previndent) {
			printf ".RS %d\n", $previndent + 3;
		} else {
			print ".RS 3";
		}
		printf ".nr )J %d\n", $indent;
		next;
	}
	if (/^\.LE/) {
		print ".PP\n.RE";
		if ($vlcount > 0) {
			pop(@indentarray);
			$indent = pop(@indentarray);
			push(@indentarray, $indent);
			printf ".nr )J %d\n", $indent;
			$vlcount--;
		}
		next;
	}
	if (/^\.P$/) {
		if ($vlcount > 0) {
			print ".LI \\0";
		} else {
			print ".PP";
		}
		next;
	}
	if (/^\.Cs/) {
		if ($indent) {
			printf ".IP \\0 %d\n.sp -1\n", $indent;
		} else {
			print ".PP\n.sp -1";
		}
		if (/I/) {
			printf ".RS %d\n", $indent + 5;
			$CsI = 1;
		}
		print ".ta 4m +4m +4m +4m +4m +4m +4m\n.nf\n.ft 3";
		next;
	}
	if (/^\.Ce/) {
		print ".PP";
		print ".ft 1";
		print ".fi";
		if ($CsI) {
			print ".RE";
		}
		$CsI = 0;
		next;
	}

	if (/^\.iX +"([^"]*)" +"([^"]*)"(.*)/ ||
	    /^\.iX +([^" ]*) +"([^"]*)"(.*)/ ||
	    /^\.iX +"([^"]*)" +([^" ]*) +(.*)/ ||
	    /^\.iX +"*([^" ]*)"*/) {
	    # Process index entries:
	    #	.iX "major entry" "minor entry" [1]
	    #	Quotes can be omitted from either entry if it has no spaces.
	    #	Either entry can have \f fonts and \s point size changes,
	    #	which affect the collation entries in the resulting .IX.
		$major = $1;
		$minor = $2;
		$rest = ($3 =~ /1/) ? " PAGE MAJOR" : "";
		printf(".IX \"%s\" \"%s\" \"%s\" \"%s\"%s\n",
			&stripit($major), &stripit($minor),
			$major, $minor, $rest);
		next;
	}

	# miscellaneous substitutions:
	unless (/NAME/.../SYNOPSIS/) {
		# change all \- minuses to \\(mi unless in NAME line
		s/\\-/\\(mi/g;
	}
	s/Cde/\\s-1CDE\\s+1/g;
	s/^\.P *$/.PP/;
	s/^\.yE/.PP\n.fi/;
	s/^\.HU/.SS/;

	# change .Fn function macros to be .BRs, including the
	# volume number if given, or (\|) if not
	s/^\.Fn +([^ ]+) +([1-9][a-zA-Z])(.*)/.BR $1 ($2)$3/;
	s/^\.Fn +([^ ]+) +([1-9])(.*)/.BR $1 ($2)$3/;
	s/^\.Fn +([^ ]+) +([1-9])/.BR $1 ($2)/;
	s/^\.Fn +([^ ]+) *(.*)/.BR $1 (\\|)$2/;
	s/^\.Fn +([^ ]+) *$/.BR $1 (\\|)/;

	# .Cm command macros are similar to .Fn, but do not use
	# parentheses if there is no volume number
	s/^\.Cm +([^ ]+) +([1-9][a-zA-Z])(.*)/.BR $1 ($2)$3/;
	s/^\.Cm +([^ ]+) +([1-9])(.*)/.BR $1 ($2)$3/;
	s/^\.Cm +([^ ]+) +([1-9])/.BR $1 ($2)/;
	s/^\.Cm +([^ ]+) *(.*)/.BR $1 $2/;
	s/^\.Cm +([^ ]+) *$/.BR $1/;

	# .Hd header macros are similar to .Fn, but add < >
	# and do not use parentheses if there is no volume number
	if ($inSEEALSO && /^\.Hd /) {
		# Within a SEE ALSO section, edit the .Hd lines
		# so that they do not have the / delimiter or the .h.
		# For example, .Hd Dt/Wsm.h --> .Hd DtWsm, which
		# matches the actual manpage name.
		s/\///;
		s/\.h//;
		s/^\.Hd +([^ ]+) +([1-9])(.*)/.BR $1 ($2)$3/;
		s/^\.Hd +([^ ]+) +([1-9])/.BR $1 ($2)/;
		s/^\.Hd +([^ ]+) *(.*)/.BR $1 (5)$2/;
		s/^\.Hd +([^ ]+) *$/.BR $1 (5)/;
	} else {
		s/^\.Hd +([^ ]+) +([1-9][a-zA-Z])(.*)/.BR <$1> $3/;
		s/^\.Hd +([^ ]+) +([1-9])(.*)/.BR <$1> $3/;
		s/^\.Hd +([^ ]+) +([1-9])/.BR <$1>/;
		s/^\.Hd +([^ ]+) *(.*)/.BR <$1> $2/;
		s/^\.Hd +([^ ]+) *$/.BR <$1>/;
	}

	# .tK is used for keystroke names, such as <tab> or <newline>
	# (this algorithm relies on having no embedded spaces unless
	# they are within " ")
	s/^\.tK +"(.+)" +(.*)/<\1>\2/;
	s/^\.tK +"(.+)" *$/<\1>/;
	s/^\.tK +([^ ]+) +(.*)/<\1>\2/;
	s/^\.tK +([^ ]+) *$/<\1>/;

	# .Cn constant macro gets turned into bold, with point
	# reduction if all caps
	s/^\.Cn +([A-Z_][A-Z_0-9]*) +([^ ]+) *$/.BR \\s-1$1\\s+1 $2/;
	s/^\.Cn +([A-Z_][A-Z_0-9]*) *$/.BR \\s-1$1\\s+1/;
	s/^\.Cn +([^ ]+) +([^ ]+) *$/.BR $1 $2/;
	s/^\.Cn/.BR/;

	# directly substitute text for X/Open strings
	s/\\\*\(ZC/X\/Open CAE Specification, \\f3Commands and Utilities\\fP, \\f3Issue 4\\fP/g;
	s/\\\*\(ZK/X\/Open CAE Specification, \\f3System Interfaces and Headers\\fP, \\f3Issue 4\\fP/g;
	s/\\\*\(Zb/X\/Open CAE Specification, \\f3System Interface Definitions\\fP, \\f3Issue 4\\fP/g;
	s/\\\*\(Zu/X\/Open Utility Syntax Guidelines/g;
	s/\\\*\(ZX/X\/Open Common Desktop Environment/g;
	s/\\\*\(Zx/\\s-1XCDE\\s+1/g;
	s/\\\*\(ZD/X\/Open CAE Specification, \\f3\\-1XCDE\\s+1 Definitions and Infrastructure\\fP/;
	s/\\\*\(ZS/X\/Open CAE Specification, \\f3\\-1XCDE\\s+1 Services and Applications\\fP/;

	print $_;
}
'
