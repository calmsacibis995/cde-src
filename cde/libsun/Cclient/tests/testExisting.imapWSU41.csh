#!/bin/csh -f
# @(#)testExisting.imapWSU41.csh	1.9 97/05/23

set NAME=testExisting.imapWSU41

if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

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

setenv NETPATH /net/${WSU41_HOST}
source setUpTmp

# Cleanup from any previous run.
/bin/rm -f A.${NAME}.out* >& /dev/null

# First time it should create the index file.
echo "${NAME}/#1 (IMAP) Testing loading a folder"
sed "s/localhost/${WSU41_HOST}/g" A.${NAME} > /tmp/imapWSU41.$$

mtest -c ${TEST_DIR}/A.${NAME}
if ( $status != 0 ) then
	echo "${NAME}/#1 FAILURE"
	set testFailed=1
else
	sed "s/${WSU41_HOST}/localhost/g" A.${NAME}.out > A.${NAME}.out.1
	cat A.${NAME}.out.1 | grep -v '^%Parse' > A.${NAME}.out.2
	mv A.${NAME}.out.2 A.${NAME}.out.1
	diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( ${status} != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif
