#!/bin/csh -f
#
# @(#)setUpTmp.csh	1.5 97/06/06
#

if ( ! ${?NETPATH} ) then
	echo "NETPATH not set, defaulting to ''"
	setenv NETPATH ""
endif

echo "Setting up ${NETPATH}/tmp"

set FOLDER=inboxExisting
set DIR=${NETPATH}/tmp
set INDEX=${DIR}/.${FOLDER}_dt_index
set OLDINDEX="${DIR}/.{FOLDER}_SIMS_index*"

/bin/rm -rf ${NETPATH}/tmp/mailFolders >& /dev/null
/bin/mkdir -p ${NETPATH}/tmp/mailFolders >& /dev/null
/bin/chmod 777 ${NETPATH}/tmp/mailFolders >& /dev/null
/bin/rm -f ${NETPATH}/tmp/inboxExisting >& /dev/null
/bin/rm -f ${NETPATH}/tmp/localMailbox >& /dev/null
/bin/rm -f ${DIR}/${FOLDER}.lock* >& /dev/null
/bin/rm -f ${DIR}/*.lock >& /dev/null
/bin/rm -f ${INDEX} >& /dev/null
/bin/rm -f ${OLDINDEX} >& /dev/null
/bin/rm -f ${NETPATH}/tmp/inbox
/bin/rm -f ${NETPATH}/tmp/foo
/bin/rm -f ${NETPATH}/tmp/doesNotExist
/bin/touch ${NETPATH}/tmp/inbox

/bin/cp MailFolders/Jokes ${NETPATH}/tmp/inboxExisting

/bin/cp MailFolders/nullmail ${NETPATH}/tmp/mailFolders
/bin/cp MailFolders/nullin.mbox ${NETPATH}/tmp/mailFolders/mailboxWithNull
/bin/cp MailFolders/nullHeaderMailbox ${NETPATH}/tmp/mailFolders/nullHeaderMailbox
/bin/cp MailFolders/humor ${NETPATH}/tmp/mailFolders/humor
/bin/cp MailFolders/barbershop.mbox ${NETPATH}/tmp/mailFolders/testMailbox
/bin/cp MailFolders/multimedia-demo.mbox ${NETPATH}/tmp/mailFolders/testMailbox1

ln -s ${NETPATH}/tmp/mailFolders/../mailFolders/humor ${NETPATH}/tmp/mailFolders/humor_link_relative

ln -s ${NETPATH}/tmp/mailFolders/humor ${NETPATH}/tmp/mailFolders/humor_link


/bin/touch ${NETPATH}/tmp/localMailbox

/bin/chmod a+rw ${NETPATH}/tmp/inboxExisting
/bin/chmod a+rw ${NETPATH}/tmp/mailFolders/*

/bin/rm -f /tmp/nfsDirectory
(cd /tmp ; ln -s ${NFS_DIRECTORY} nfsDirectory)
/bin/rm -f /tmp/nfsDirectory/fooFolder1

echo "Setting up ${NETPATH}/tmp - DONE"
