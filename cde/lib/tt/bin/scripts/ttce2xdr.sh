#! /bin/sh
# %%  (c) Copyright 1993, 1994 Hewlett-Packard Company			
# %%  (c) Copyright 1993, 1994 International Business Machines Corp.	
# %%  (c) Copyright 1993, 1994 Sun Microsystems, Inc.			
# %%  (c) Copyright 1993, 1994 Novell, Inc. 				
# %%  $Revision: $ 			 				
# @(#)ttce2xdr.sh	1.12 @(#)
# ttce2xdr - Convert ToolTalk Classing Engine tables to XDR types database
#
# Copyright (c) 1992 by Sun Microsystems, Inc.
#

#
# usage
#
usage() {
	echo "$1: convert ToolTalk CE tables to XDR types database"
	echo "Usage: $1 [-xn][-d user|system]"
	echo "       $1 [-xn]-d network [<OPENWINHOME1> [<OPENWINHOME2>]]"
	echo "       $1 [-h]"
	echo "       $1 [-v]"
	echo "  -d <db>	database to read (default: all) and "
	echo "    		write (default: user)"
	echo "  -n		echo commands instead of executing them"
	echo "  -h		help"
	echo "  -v		version"
	echo "  -x		debug mode (pass 'set -x' to /bin/sh)"
	echo "  <OPENWINHOME1>	OWv3 installation to read from"
	echo "  <OPENWINHOME2>	OWv3 installation to write to"
}

#
# version
#
version() {
	echo ttce2xdr version: "TT_VERSION_STRING"
}

#
# cleanup - Call me before exiting
#
cleanup() {
	rm -f /tmp/ttce2xdr.types.$$ /tmp/ttce2xdr.types.$$.deps /tmp/ttce2xdr.ceascii.$$
}

#
# nullit - create null ~/.tt/xdr.types so auto convert won't be run
# next time
#
nullit() {
	test -d $HOME/.tt || mkdir $HOME/.tt
	touch $HOME/.tt/types.xdr
}

#
# Main
#
trap cleanup 0 1 2 3 15

cmd=`basename $0`

dryrun=
database=
ow1=
ow2=

#
# Parse command line
#
args=$*
while getopts hxnvrfd: option
do
	case $option in
		h | \?)		usage $cmd; exit 1;;
		x)		set -x;;
		v)		version $cmd; exit 1;;
		n)		dryrun=1;;
		d)		database=$OPTARG;;
	esac
done

#
# Discard args processed by getopts
#
shift `expr $OPTIND - 1`

#
# Process mandatory args
#
if [ $# -gt 0 ]; then
	ow1=$1
	shift
	if [ $# -gt 0 ]; then
		ow2=$1
		shift
	fi
fi

#
# Check for extraneous args
#
if [ $# -gt 0 ]; then
	usage $cmd
	exit 1
fi

if [ "`set | grep TTHOME`" ]; then
	echo "\$TTHOME is set. $cmd does not support \$TTHOME."
	echo "Unset it and rerun $cmd, or run tt_type_comp(1) by hand."
	exit 1
fi

if [ $database ]; then
	case $database in
		user | system | network)
			;;
		*)	echo "$cmd: bogus database: $database";
			usage $cmd; exit 1;;
	esac
	dbopt="-d $database"
else
	database="user"
fi

# if we are doing user (perhaps automatic) conversion. make
# sure there is some conversion to do, otherwise we can bail out
# early.

if [ "$database" = "user" ]; then
	if [ ! -f $HOME/.cetables/cetables ]; then
#		user doesn't even have a cetables, nothing to do
		nullit
		exit 0
	fi

	ce_db_build user -to_ascii /tmp/ttce2xdr.ceascii.$$

	if egrep -s "NS_NAME=SUN_TOOLTALK_TYPES" /tmp/ttce2xdr.ceascii.$$ ; then
		:
	else
#		user has cetables, but no ToolTalk types
		nullit
		exit 0
	fi

#	Now, we know the user has a cetables with ToolTalk types.  Now
#	check for a possible malformed cetables with no ToolTalk name
#	space definition.

	if egrep -s '^NS_ATTR=$' /tmp/ttce2xdr.ceascii.$$ ; then

# 		The cetables exists, and has ToolTalk types, but 
#		since the basic definitions in the network CE database
#		may be gone, we need to add them back in.  Eventually
#		in a future release we should remove all ToolTalk info
#		from user's cetables.

		ed /tmp/ttce2xdr.ceascii.$$  >/dev/null <<- 'EOF'
/^NS_ATTR=$/d
i
NS_ATTR=((NS_MANAGER,string,<$CEPATH/tns_mgr.so>)
)
.
/^NS_ENTRIES=/a
(TYPE_NAME,string,<SUN_TOOLTALK::attrs>)
(TYPE_NAME,string,<attr>)
(TT_OP,string,<attr>)
(TT_ARG,string,<attr>)
(TT_SCOPE,string,<attr>)
(TT_SCOPE,string,<attr>)
(TT_DISPOSITION,string,<attr>)
(TT_MSET_DISPOSITION,string,<attr>)
(TT_MSET_OPNUM,string,<attr>)
(TT_OPNUM,string,<attr>)
(TT_MSET_HANDLER_PTYPE,string,<attr>)
(TT_HANDLER_PTYPE,string,<attr>)
(TT_MSET_OTYPE,string,<attr>)
(TT_OTYPE,string,<attr>)
(TT_PARENT,string,<attr>)
(TT_OUT,string,<attr>)
(TT_IN,string,<attr>)
(TT_INOUT,string,<attr>)
(TT_SESSION,string,<attr>)
(TT_FILE,string,<attr>)
(TT_BOTH,string,<attr>)
(TT_FILE_IN_SESSION,string,<attr>)
(TT_DISCARD,string,<attr>)
(TT_QUEUE,string,<attr>)
(TT_START,string,<attr>)
(TT_CATEGORY,string,<attr>)
(TT_OBSERVE,string,<attr>)
(TT_HANDLE,string,<attr>)
(TT_CLASS,string,<attr>)
(TT_REQUEST,string,<attr>)
(TT_NOTICE,string,<attr>)
(SUN_TOOLTALK_TYPE,string,<attr>)
(SUN_TOOLTALK_PTYPE,string,<attr>)
(SUN_TOOLTALK_OTYPE,string,<attr>)
(SUN_TOOLTALK_SIGNATURE,string,<attr>)
.
w
q
EOF
		ce_db_build user -from_ascii /tmp/ttce2xdr.ceascii.$$
	fi
fi


if [ $dryrun ]; then
	if [ $ow1 ]; then
		echo "OPENWINHOME=$ow1; export OPENWINHOME"
	fi
	echo "tt_type_comp -Eps $dbopt > /tmp/ttce2xdr.types.$$"
	if [ $ow2 ]; then
		echo "OPENWINHOME=$ow2; export OPENWINHOME"
	fi
	echo "tt_type_comp -Ms $dbopt /tmp/ttce2xdr.types.$$"
else
	if [ $ow1 ]; then
		OPENWINHOME=$ow1
		export OPENWINHOME
	fi
	rm -f /tmp/ttce2xdr.types.$$
	if tt_type_comp -Eps $dbopt > /tmp/ttce2xdr.types.$$; then :; else
#		Conversion failed for some reason. If we're working on a
#		user database, we're probably doing an automatic conversion.
#		and there's no point in continuing to retry this every time.
#		Just touch the .tt/types.xdr file so auto conversion won't
#		keep getting run.
	
		case $database in
			user)
				nullit
				exit 0
				;;
	
			*)
				echo $cmd: could not read ToolTalk types from Classing Engine database.
				echo $cmd: there may not be any ToolTalk types in this database.
				exit 1
				;;
		esac
	fi
	if [ $ow2 ]; then
		OPENWINHOME=$ow2
		export OPENWINHOME
	fi
	if tt_type_comp -Ms $dbopt /tmp/ttce2xdr.types.$$; then :; else
		echo $cmd: could not compile types into XDR database.
		exit 1
	fi
fi
