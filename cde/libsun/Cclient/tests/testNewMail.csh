#!/bin/csh -f
# @(#)testNewMail.csh	1.6 97/06/06

set NAME=testNewMail
 
if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH ""
source setUpTmp

echo "${NAME}/#1 Testing the checking of new mail from an empty mailbox"

touch /tmp/inbox
mtest -c ${TEST_DIR}/A.${NAME} -e /tmp/inbox
if ( $status != 0) then
	echo "mtest FAILED"
	set testFailed=1
else
	grep -v '^MM_NOTIFY:%Parse time' A.${NAME}.out > A.${NAME}.out.1
	diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( $status != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

echo "${NAME}/#2 Testing the checking of new mail from an existing mailbox"
mtest -c ${TEST_DIR}/A.${NAME} -e /tmp/inbox
if ( $status != 0) then
	echo "mtest FAILED"
	set testFailed 1
else
	grep -v '^MM_NOTIFY:%Parse time' A.${NAME}.out > A.${NAME}.out.2
	diff A.${NAME}.out.2 Gold.${NAME}.2
	if ( $status != 0 ) then
		echo diff A.${NAME}.out.2 Gold.${NAME}.2 FAILED.
		echo "${NAME}/#2 FAILED"
		set testFailed 1
	else
		echo "${NAME}/#2 PASSED"
	endif
endif

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif
