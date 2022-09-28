#!/bin/ksh

PATH=/usr/bin:/usr/ucb:${PATH}
COMMAND=`basename $0`

function UsageExit
{
    print -u2 "Usage: $COMMAND [-b browser] [-o browser_opts] url-string"
    exit 1
}

function ErrorExit
{
    typeset status=$1 ; shift
    print -u2 "$COMMAND:" $@
    exit $status
}

function FindBrowser
{
    typeset altbrowser="$1"
    typeset bList="${2:-netscape sdthotjava hotjava}"
    typeset browser bpath

    if [[ ! -z "$altbrowser" ]] then
	bpath=$(whence $altbrowser) || {
	    ErrorExit 2 "web browser command \`$altbrowser' not found in PATH."
	}
	browser=$altbrowser
    else
	for browser in $bList
	do
	    if bpath=$(whence $browser) ; then
		break
	    fi
	done
	if [[ -z "$bpath" ]] then
	    ErrorExit 2 "no web browser found in PATH; searched for: $bList"
	fi
    fi

    if [[ $browser = netscape ]] then
	browser=NetscapeRemote
    fi

    BROWSER=$browser
}

function NetscapeRemote
{
    typeset cmd="netscape $1"
    typeset url="$2"
    typeset remote="$cmd -noraise -remote"
    typeset err="not running on display"

    if [[ -z "$url" ]] then
	remurl="new()"
    else
	remurl="OpenURL(${url},new-window)"
    fi
    if $remote "$remurl" 2>&1 | grep "$err" > /dev/null ; then
	$cmd "$url" &
    fi
}

# main
{
    typeset BROWSER BROWSER_OPTS URL
    typeset uFlag=0

    while getopts ub:o: flag
    do
	case $flag in
	  u) uFlag=1;;
	  b) BROWSER="$OPTARG";;
	  o) BROWSER_OPTS="$OPTARG";;
	  *) UsageExit;;
	esac
    done
    shift $OPTIND-1

    if (( $# > 1 )) then
	UsageExit
    fi

    FindBrowser "$BROWSER"

    URL="$1"

    $BROWSER "$BROWSER_OPTS" "$URL" &

    sleep 60

    exit 0
}
