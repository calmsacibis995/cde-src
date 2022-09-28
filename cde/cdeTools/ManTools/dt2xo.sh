#!/bin/sh
# This script translates from the common CDE man source format into
# a form suitable for the X/Open specification macros.  See dt2man
# for the corresponding script to produce troff -man man pages.
# 
# The sed scripts do the following:
#
#	Deletes all lines between .cdes and .cdee and all beginning
#	with ".cde " (CDE-only material).  Removes ".xo " prefixes.
#	Also deletes all company-specific variants of these.
#
#	Fixes up fonts within the Synopsis.
#
#	Deletes .BH and adds ".na" for the SEE ALSO.
#
# The perl script includes comments.
#
# Mods:
#	Corrected .mS format for headers. (4/5/94 hlj)
#
#	Made .mE and CHANGE HISTORY unnecessary.  Added support
#	for .Hd header macros. (4/6/94 hlj)
#
#	Omit .mE and CHANGE HISTORY if no .TH in file. (4/14/94 hlj)
#
#	Fixed double translation of \| in SYNOPSIS (4/19/94 hlj)
#
#	Added Murkin-->Bwitish spelling translation (7/22/94 hlj)
#
soelim "$@" | sed '
1i\
.tr *\\(**
/^\.cdes/,/^\.cdee/ d
/^\.suns/,/^\.sune/ d
/^\.ibms/,/^\.ibme/ d
/^\.hps/,/^\.hpe/ d
/^\.usls/,/^\.usle/ d
/^\.cde /d
/^\.sun /d
/^\.ibm /d
/^\.hp /d
/^\.usl /d
/^\.xo[se]/d
s/^\.xo //
/^\.yS/,/^\.yE/ {
	s/\[/\\f7[\\fP/g;
	s/\]/\\f7]\\fP/g;
	s/^|/\\(bv/;
	s/\([^\\]\)|/\1\\|\\(bv\\|/g;
	s/\\f2/\\f6/g;
}
/^\.BH/d
/SEE ALSO/a\
.na
' | perl -e '
$\ = "\n";	# set "print" to append newline
$mE = 0;
$manpage = 0;
$bwits = 1;
$endstring=".mH \"CHANGE HISTORY\"\nFirst released in Issue 1.\n.mE";
while (<>) {
	chop;
	if (/^\.TH/) {
		if ($. > 3 && $mE == 0) {
			# output end-of-manpage macro for previous page
			print $endstring;
		}
		# turn .TH into .mS and record the volume number
		($cmd, $name, $volume, $rest) = split;
		$volume =~ s/[a-zA-Z]//; # remove any letter suffix
		if ($volume == 3) {
			printf ".mS %s (\\|)\n", $name;
		} elsif ($volume == 5) {
			printf ".mS <%s> h\n", $name;
		} else {
			printf ".mS %s\n", $name;
		}
		$manpage = 1;	# flag this as a man-page file
		$mE = 0;	# reset have-.mE flag
		next;
	}
	if (/^\.yS/) {
		# set up fonts and fill for the Synopsis;
		# note different fill for volume 1 vs others
		print ".yS";
		if ($volume == 1) {
			print ".fi";
			print ".na";
		}
		$bwits = 0;
		next;
	}
	if (/^\.mE/) {
		# keep track of .mE macros already in the file to avoid
		# duplicating them
		$mE = 1;
		print $_;
		next;
	}
	if (/^\.VL/) {
		# change .VL to a 2-arg version, which indents the
		# entire list
		($cmd, $indent, $rest) = split;
		if ($indent =~ /n$/) {
			chop($indent);
		}
		printf ".VL %d 5\n", $indent + 5;
		next;
	}

	if (/^\.SH.*NAME/.../^\.SH.*SYN/) {
		# In the NAME line, change \- to \(em
		s/\\-/\\(em/;
	}
	s/\\-/\\(mi/g;	# change all other \- minuses to \\(mi

	if (/^\.Cs/.../^\.Ce/) {
		# In code samples, use CourierOblique instead of Italics
		s/\\f2/\\f6/g;
		$bwits = 0;
	}

	# miscellaneous substitutions:
	s/^\.SH/.mH/;
	s/^\.LP/.P/;
	s/^\.PP/.P/;
	s/^\.Lc/.LI/;
	s/\ba\sCde/an \\\*(Zx/g;
	s/\bA\sCde/An \\\*(Zx/g;
	s/Cde/\\\*(Zx/g;	# use X/Open string for CDE name

	$bwits = 1 if (/^(\.yE|\.Ce)/); # resume bwitishizing

	if ($bwits && /^([^.]|\.[BHILR])/ && !/\\f/ && !/@/ && !/_/) {
		# Perform Murkin to Bwitish spelling translations.
		# Do not translate in synopses or code samples.
		# Do not translate in "dot" lines, except .H,
		# .B, .BR, .I, .IR, .LI, etc., but also not if
		# there is a \\f font change or an @ or _ in the line.
		s/([^Ss])ize([ds]*)\b/\1ise\2/g; # organized, initializes,
					# serialize (but not resize)
		s/([^Ss])izing\b/\1ising/g; # standardizing
		s/ization/isation/g;	# organization
		s/avior/aviour/g;	# behavior
		s/avor/avour/g;		# favor
		s/olor/olour/g;		# color
		s/onor/onour/g;		# honor
		s/ghbor/ghbour/g;	# neighbor
		s/icens/icenc/g;	# license, licensing
		s/atalog\b/atalogue/g;	# catalog
		s/dgment/dgement/g;	# acknowledgment
	}

	# fix up .Fn, .Cm, and .Hd macros to remove the volume
	# number used in -man:
	s/^\.(Fn|Cm|Hd) +([^ ]+) +[1-9][a-zA-Z](.*)/.$1 $2 $3/;
	s/^\.(Fn|Cm|Hd) +([^ ]+) +\d(.*)/.$1 $2 $3/;
	s/^\.(Fn|Cm|Hd) +([^ ]+) +\d *$/.$1 $2/;
	s/^\.(Fn|Cm|Hd) +([^ ]+) *(.*)/.$1 $2 $3/;

	# .Cn constants are merely Roman in X/Open:
	s/^\.Cn +([^ ]+) +([^ ]+) *$/$1$2/;
	s/^\.Cn +([^ ]+) *$/$1/;

	print $_;
}
if ($manpage && !$mE) {
	# output end-of-manpage macro at end of file if one already
	# not already there
	print $endstring;
}
'
