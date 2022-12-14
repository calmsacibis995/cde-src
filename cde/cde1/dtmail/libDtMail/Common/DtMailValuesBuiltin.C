/*
 *+SNOTICE
 *
 *
 *	$Revision: 1.1 $
 *
 *	RESTRICTED CONFIDENTIAL INFORMATION:
 *	
 *	The information in this document is subject to special
 *	restrictions in a confidential disclosure agreement bertween
 *	HP, IBM, Sun, USL, SCO and Univel.  Do not distribute this
 *	document outside HP, IBM, Sun, USL, SCO, or Univel wihtout
 *	Sun's specific written approval.  This documment and all copies
 *	and derivative works thereof must be returned or destroyed at
 *	Sun's request.
 *
 *	Copyright 1993 Sun Microsystems, Inc.  All rights reserved.
 *
 *+ENOTICE
 */

#ifndef I_HAVE_NO_IDENT
#pragma ident "@(#)DtMailValuesBuiltin.C	1.21 02/22/95"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <DtMail/DtMailValues.hh>
#include <DtMail/IO.hh>
#include <DtMail/DtMailError.hh>

DtMailValueSeq::DtMailValueSeq(int size) : DtVirtArray<DtMailValue *>(size)
{
}

DtMailValueSeq::~DtMailValueSeq(void)
{
    for (int ent = 0; ent < this->length(); ent++) {
	delete (*this)[ent];
    }
}

void
DtMailValueSeq::clear(void)
{
    while(this->length()) {
	delete (*this)[0];
	this->remove(0);
    }
}

DtMailValue::DtMailValue(const char * str)
{
    _value = NULL;

    if (str) {
	_value = strdup(str);
    }
}

DtMailValue::~DtMailValue(void)
{
    if (_value) {
	free(_value);
    }
}

DtMailValue::operator const char *(void)
{
    return(_value);
}

const char *
DtMailValue::operator= (const char * str)
{
    if (_value) {
	free(_value);
    }

    _value = strdup(str);

    return(_value);
}

DtMailValueDate
DtMailValue::toDate(void)
{
    DtMailValueDate	date;
    memset(&date, 0, sizeof(DtMailValueDate));

    return(date);
}

#ifdef DEAD_WOOD
void
DtMailValue::fromDate(const DtMailValueDate & date)
{
    if (_value) {
	free(_value);
    }

    _value = (char *)malloc(64);

    tm result;

    SafeLocaltime(&date.dtm_date, result);

    /* NL_COMMENT
     * The %C is the time and date format, please refer to strftime man page for 
     * explanation of each format.
     */
    SafeStrftime(_value, 64, catgets(DtMailEnv::_errorCatalog, 2, 1, "%C"), &result);
}
#endif /* DEAD_WOOD */

DtMailAddressSeq *
DtMailValue::toAddress(void)
{
    return(NULL);
}

#ifdef DEAD_WOOD
void
DtMailValue::fromAddress(const DtMailAddressSeq &)
{
}
#endif /* DEAD_WOOD */

const char *
DtMailValue::raw(void)
{
    return(_value);
}

DtMailValueAddress::DtMailValueAddress(void)
{
    dtm_address = NULL;
    dtm_person = NULL;
    dtm_namespace = NULL;
}

DtMailValueAddress::DtMailValueAddress(const DtMailValueAddress & other)
{
    dtm_address = NULL;
    dtm_person = NULL;
    dtm_namespace = NULL;

    if (other.dtm_address) {
	dtm_address = strdup(other.dtm_address);
    }

    if (other.dtm_person) {
	dtm_person = strdup(other.dtm_person);
    }
    
    if (other.dtm_namespace) {
	dtm_namespace = strdup(other.dtm_namespace);
    }
}

DtMailValueAddress::DtMailValueAddress(const char * address,
				       const char * person,
				       const char * namespace)
{
    dtm_address = NULL;
    dtm_person = NULL;
    dtm_namespace = NULL;

    if (address) {
	dtm_address = strdup(address);
    }
    
    if (person) {
	dtm_person = strdup(person);
    }
    
    if (namespace) {
	dtm_namespace = strdup(namespace);
    }
}

DtMailValueAddress::DtMailValueAddress(const char * address, const int addr_len,
				       const char * person, const int per_len,
				       const char * namespace)
{
    dtm_address = NULL;
    dtm_person = NULL;
    dtm_namespace = NULL;

    if (address) {
	dtm_address = (char *)malloc(addr_len + 1);
	memcpy(dtm_address, address, addr_len);
	dtm_address[addr_len] = 0;
    }
    
    if (person) {
	dtm_person = (char *)malloc(per_len + 1);
	memcpy(dtm_person, person, per_len);
	dtm_person[per_len] = 0;
    }
    
    if (namespace) {
	dtm_namespace = strdup(namespace);
    }
}

DtMailValueAddress::~DtMailValueAddress(void)
{
    if (dtm_address) {
	free(dtm_address);
    }
    
    if (dtm_person) {
	free(dtm_person);
    }
    
    if (dtm_namespace) {
	free(dtm_namespace);
    }
}

DtMailAddressSeq::DtMailAddressSeq(int size) : DtVirtArray<DtMailValueAddress *>(size)
{
}

DtMailAddressSeq::~DtMailAddressSeq(void)
{
    while(this->length()) {
	delete (*this)[0];
	this->remove(0);
    }
}
