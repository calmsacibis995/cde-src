#!/bin/csh -f
# @(#)testSmtp.csh	1.3 97/06/06


set NAME=testSmtp
 
if ( ! $?MTEST_VARS_SET_OK) then
	source testVariables
endif

setenv NETPATH ""
source setUpTmp

echo "${NAME}/#1 Testing SMTP"

setenv SEND_HOST `/bin/uname -n`
setenv SENDER `/bin/who am i|/bin/awk '{print $1}'`

# Firt remove users INBOX and IGNORE the errors.
mtest -c ${TEST_DIR}/A.${NAME}.2 >& /dev/null

mtest -c ${TEST_DIR}/A.${NAME}.1
if ( $status != 0) then
	echo "$NAME FAILED"
	set testFailed=1
else
	sed "s/${SEND_HOST}/localhost/g" A.${NAME}.1.out > A.${NAME}.tmp
	sed "s/${SENDER}/testuser/g" A.${NAME}.tmp > A.${NAME}.out
	/bin/grep -v '^%131 busy' A.${NAME}.out | /bin/grep -v '^%Parse time' | /bin/grep -v 'Sendmail' | /bin/grep -v 'Hello' | /bin/grep -v 'Message accepted for delivery' > A.${NAME}.out.1
	/bin/diff A.${NAME}.out.1 Gold.${NAME}.1
	if ( $status != 0 ) then
		echo diff A.${NAME}.out.1 Gold.${NAME}.1 FAILED.
		echo "${NAME}/#1 FAILED"
		set testFailed=1
	else
		echo "${NAME}/#1 PASSED"
	endif
endif

# now cleanup
mtest -c ${TEST_DIR}/A.${NAME}.2 >& /dev/null

if ( $?testFailed ) then
	setenv somethingFailed 1
	echo "${NAME} FAILED"
	unset testFailed
else
	echo "${NAME} PASSED"
endif
