#!/bin/csh -f
# @(#)testMisc.imapWSU41.csh	1.5 97/05/23

set NAME=testMisc.imapWSU41
setenv IMAP_ACCOUNT abcde12345
 
if ( ${?IMAP_WSU41_HOST} ) then
	setenv HOST_TO_USE ${IMAP_WSU41_HOST}
	setenv WSU41_HOST $IMAP_WSU41_HOST
	source verifyAcct
else
	echo ""
	echo "NOTE:"
	echo "Environmental varable IMAP_WSU41_HOST' not set."
	echo "Must be able to 'rsh' to IMAP_WSU41_HOST."
	echo "And must be able to net mount /net/IMAP_WSU41_HOST/tmp"
	echo ""
	setenv verifyFailed 1
	setenv somethingFailed 1
	 echo "${NAME} FAILED"
	unset testFailed
	exit 1
endif

if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH /net/${WSU41_HOST}
source setUpTmp

# Cleanup from any previous run.
/bin/rm -f A.${NAME}.out* >& /dev/null

# First time it should create the index file.
echo "$NAME Testing various mtest functions (3, 8, C, D, E, H, I, T, U, W, Z)"
sed "s/localhost/${WSU41_HOST}/g" A.${NAME} > A.${NAME}.input
mtest -c ${TEST_DIR}/A.${NAME}.input
if ( $status != 0 ) then
	echo "$NAME/#1 FAILURE"
	set testFailed=1
else
	sed "s/${WSU41_HOST}/localhost/g" A.${NAME}.input.out > A.${NAME}.out.1
	grep -v '^%131 busy' A.${NAME}.out.1 | grep -v '^%Parse time' > A.${NAME}.out.tmp
	mv A.${NAME}.out.tmp A.${NAME}.out.1
	diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( ${status} != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "$NAME/#1 FAILED"
		set testFailed=1
	else
		echo "$NAME/#1 PASSED"
	endif
endif

# Now test it with UID Flags
echo "Testing Miscellaneous Again With UID Flags Set."
source setUpTmp
setenv FETCH_FLAGS FT_UID
sed "s/localhost/${WSU41_HOST}/g" A.${NAME} > A.${NAME}.input
mtest -c ${TEST_DIR}/A.${NAME}.input
if ( $status != 0 ) then
       	echo "$NAME #2 FAILURE"
       	set testFailed=1
else
	sed "s/${WSU41_HOST}/localhost/g" A.${NAME}.input.out > A.${NAME}.out.2
	grep -v '^%131 busy' A.${NAME}.out.2 | grep -v '^%Parse time' >  A.${NAME}.out.tmp 
	mv A.${NAME}.out.tmp A.${NAME}.out.2
	diff  A.${NAME}.out.2 Gold.${NAME}.2
	if ( ${status} != 0 ) then
		echo diff  A.${NAME}.out.2 Gold.${NAME}.2 FAILED.
		echo "$NAME #2 FAILED"
		set testFailed=1
	else
		echo "$NAME #2 PASSED"
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed=1
	echo "$NAME FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif

/bin/rm -f A.${NAME}.input
