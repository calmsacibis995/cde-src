#!/bin/ksh

#
# usage
#
usage() {
	echo    "$1: check for media and open media found"
	echo    "Usage: $1 [-z nsecs][-fdD] [media_spec [cmd [arg ...]]]"
	echo    "  media_spec	floppy0, cdrom, *, etc. default: floppy"
	echo    "  cmd arg ...	command to run. default: dtaction Open"
	echo    "  -d		monitor continuously"
	echo    "  -z nsecs	sleep nsecs before doing anything"
	echo    "  -D		pass mount point AND device path to command"
	echo    "  -f		do not format any uninitialized floppies found"
}

#
# cleanup - Call me before exiting
#
trap cleanup EXIT
cleanup() {
	if [[ -n "$eventWatcher" ]]; then kill $eventWatcher; fi
	if [[ -p "$fifo" ]]; then /bin/rm -f "$fifo"; fi
	if [[ -f "$lastCheck" ]]; then /bin/rm -f "$lastCheck"; fi
	if [[ -f "$lockFile" ]]; then /bin/rm -f "$lockFile"; fi
}

#
# OpenMedia mediaSpec [cmd [arg ...]]
#
# Does nothing if /tmp/sdtvolchecking_manually exists.
#
# $lastCheck		if set: only open volumes inserted since
#			mtime of $lastCheck
# $passDevice		see usage
# $formatFloppies	see usage
# $daemonMode		if false, wait for open command
#
OpenMedia() {
	_mediaSpec="$1"
	shift
	if [[ -f /tmp/sdtvolchecking_manually ]]; then
		return;
	fi
	for volumefile in /tmp/.removable/${_mediaSpec}*; do
		if [[ ! -f "$volumefile" ]]; then
			continue;
		fi
		if [[   -n "$lastCheck" \
		     && ! ("$volumefile" -nt "$lastCheck") ]]
		then
			continue;
		fi
		#
		# This would not handle spaces in volume names!
		# read mountPt devicePath < "$volumefile"
		#
		volumeSpec="`head \"$volumefile\"`"
		#
		# volumeSpec format: mountPt /some_volmgt_path
		# Assume the " " in " /" separates the 2 fields in volumeSpec
		#
		# Trim out " /" and everything following it
		mountPt="${volumeSpec% /*}"
		#
		# Trim out " /" and everything before it; prepend trimmed "/"
		devicePath=/"${volumeSpec#* /}"
		uninitialized=
		if [[ "$mountPt" != /* ]]
		then
			uninitialized=1
			formatMode=unlabeled
			mountPt=/floppy/$mountPt
			if [[ "$mountPt" = ?(unlabeled|unformatted) ]]
			then
				formatMode=$mountPt
			fi
		fi
		# Run in the background, so we can process next volume
		( \
			if [[ -n "$formatFloppies" && -n "$uninitialized" ]]; then \
				if sdtformat_floppy -p "$formatMode" \
					-d "$devicePath" \
					-m "$mountPt"; \
				then \
					OpenMedia `basename $volumefile`; \
				fi; \
			else \
				exec ${*:-dtaction Open} "$mountPt" \
					${passDevice:+$devicePath}; \
			fi \
		) &
		if [[ ! -n "$daemonMode" ]]; then
			wait $!
		fi
	done
}

#
# Main
#
cmd=`basename $0`
mediaSpec=floppy
daemonMode=
formatFloppies=1
passDevice=
fifo=
sleepTime=0

#
# Parse command line
#
while getopts h?xdz:Df option
do
	case $option in
		h | \?)		usage $cmd; exit 1;;
		x)		set -x;;
		f)		formatFloppies=;;
		D)		passDevice=1;;
		d)		daemonMode=1;;
		z)		sleepTime=$OPTARG;;
	esac
done

#
# Discard args processed by getopts
#
shift `expr $OPTIND - 1`

if [[ $# -gt 0 ]]; then
	mediaSpec="$1"
	shift
fi

sleep $sleepTime

if [[ ! -n "$daemonMode" ]]; then
	lockFile=/tmp/sdtvolchecking_manually
	/bin/touch $lockFile
	/bin/volcheck
	/bin/rm -f $lockFile
	OpenMedia $mediaSpec ${1+"$@"}
	exit 0
fi

if [[ ! -d /tmp/.removable ]]; then
	if mkdir /tmp/.removable; then :; else exit 2; fi
	if chmod 0777 /tmp/.removable; then :; else exit 2; fi
fi

n=0
while [[ -a /tmp/.removable/notify$n ]]; do
	n=$(($n+1))
	if [[ "$n" -gt 500 ]]; then exit 3; fi
done
if mkfifo /tmp/.removable/notify$n; then :; else exit 3; fi
fifo=/tmp/.removable/notify$n

lastCheck=/tmp/sdtvolcheck$$
/bin/touch 0101000070 $lastCheck

while true; do
	OpenMedia "$mediaSpec" ${1+"$@"}
	/bin/touch $lastCheck
	/bin/cat $fifo >/dev/null &
	eventWatcher=$!
	wait $!
done

exit 0
