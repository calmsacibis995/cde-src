#!/bin/csh -f

setenv verifyFailed 0

rsh ${HOST_TO_USE} /bin/grep ${IMAP_ACCOUNT} /etc/passwd > /tmp/x.$$
if ( -z /tmp/x.$$ ) then
       	echo "Need user account 'abcde12345' with password"
	echo " '12345abcde' setup echo on $HOST_TO_USE to run"
	echo " the IMAP tests."
	setenv verifyFailed 1
endif

/bin/rm -f /tmp/x.$$ /tmp/err.$$

