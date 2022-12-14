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
#pragma ident "@(#)RFCMailValues.C	1.29 02/26/96"
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include <DtMail/IO.hh>
#include "RFCImpl.hh"
#include "RFCMIME.hh"

// String values. These assume an RFC format for now. They will
// apply RFC1522 coding rules to the strings for dealing with
// non-ASCII text in RFC headers.
//

RFCValue::RFCValue(const char * str, int size) : DtMailValue(NULL)
{
    _value = (char *)malloc(size + 1);
    memcpy(_value, str, size);
    _value[size] = 0;

    _decoded = NULL;
}

RFCValue::RFCValue(const char * str, int size, DtMail::Session *s) : DtMailValue(NULL)
{
    _value = (char *)malloc(size + 1);
    memcpy(_value, str, size);
    _value[size] = 0;

    _decoded = NULL;
	_session = s;
}

RFCValue::~RFCValue(void)
{
    if (_decoded) {
	free(_decoded);
    }
}

static const char *
decode1522(const char * enc_start, const char * max_end, char **output, DtMail::Session *s)
{
    // Find the end of the encoded region.
    //
    int qs = 0;
    for (const char * enc_end = enc_start; *enc_end && enc_end < max_end; enc_end++) {
	if (*enc_end == '?') {
	    qs += 1;
	    if (qs > 3 && *(enc_end + 1) == '=') {
		break;
	    }
	}
    }

    // If we have reach the end and have not found a closing ?= then
    // this header is improperly formed - return enc_end as the output
    // has not been touched and will be taken as is.
    //
    if (*enc_end != '?') {
	return(enc_end);
    }

    enc_end += 1;

    // Pull off the char set name.
    //
    for (const char * cs_end = enc_start + 2; *cs_end != '?'; cs_end++) {
	continue;
    }

    char cs_name[64];
    strncpy(cs_name, enc_start + 2, cs_end - enc_start - 2);
    cs_name[cs_end - enc_start - 2] = 0;

    // Set the encoding method and start of buffer.
    //
    char encoding = *(cs_end + 1);
    const char * buf_start = cs_end + 3;

    switch (toupper(encoding)) {
      case 'Q':
      {
	  int len = 0;
	  RFCMIME::readQPrint(*output, len, buf_start, enc_end - buf_start - 1);
	  (*output)[len] = 0;
	  break;
      }
	
      case 'B':
      {
	  int len = 0;
	  RFCMIME::readBase64(*output, len, buf_start, enc_end - buf_start - 1);
	  (*output)[len] = 0;
	  break;
      }
   }

   // Do codeset conversion if charset is present
   char *from_cs = s->csToConvName(cs_name);
   char *to_cs = s->locToConvName();
   if ( from_cs && to_cs ) {
	 if ( strcasecmp(from_cs, to_cs) != 0 ) {
	 unsigned long tmplen = (unsigned long) strlen(*output);
     (void) s->csConvert(&(*output), tmplen, 1, from_cs, to_cs);
	 }
   }

    if ( from_cs )
	free( from_cs );
    if ( to_cs )
	free ( to_cs );

    return(enc_end);
}


RFCValue::operator const char *(void)
{
    if (_decoded) {
	return(_decoded);
    }

    decodeValue();

    return(_decoded);
}

const char *
RFCValue::operator= (const char * str)
{
    if (_decoded) {
	free(_decoded);
	_decoded = NULL;
    }

    if (_value) {
	free(_value);
    }

    _value = strdup(str);

    return(_value);
}

static const char * DaysOfTheWeek[] = {
"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char * MonthsOfTheYear[] = {
"Jan", "Feb", "Mar",
"Apr", "May", "Jun",
"Jul", "Aug", "Sep",
"Oct", "Nov", "Dec"
};

static int
matchDay(const char * start, const char * end)
{
    int len = end - start + 1;

    for (int i = 0; i < 7; i++) {
	if (strncmp(DaysOfTheWeek[i], start, len) == 0) {
	    return(i);
	}
    }

    return(-1);
}

static int
matchMonth(const char * start, const char * end)
{
    int len = end - start + 1;

    for (int i = 0; i < 12; i++) {
	if (strncmp(MonthsOfTheYear[i], start, len) == 0) {
	    return(i);
	}
    }

    return(-1);
}

static void
parseTime(const char * start, const char * end, tm & val)
{
    int size = end - start + 1;

    // Time will be in the form hh:mm:ss where seconds are optional.

    char num_buf[10];
    strncpy(num_buf, start, 2);
    num_buf[2] = 0;

    val.tm_hour = strtol(num_buf, NULL, 10);

    strncpy(num_buf, &start[3], 2);
    num_buf[2] = 0;

    val.tm_min = strtol(num_buf, NULL, 10);

    if (size > 6) {
	strncpy(num_buf, &start[6], 2);
	num_buf[2] = 0;
	val.tm_sec = strtol(num_buf, NULL, 10);
    }
    else {
	val.tm_sec = 0;
    }

    return;
}

static const char * TZNames[] = {
"EST", "CST", "MST", "PST"
};

static const char * TZNamesDST[] = {
"EDT", "CDT", "MDT", "PDT"
};

static time_t
parseTZ(const char * start, const char * end)
{
    int size = end - start + 1;

    // There are at 3 possibilities that we understand. There
    // is the single letter military time zone. In that case
    // Z is 0 UTC. A-M is -1 to -12, skipping J. N-Y is +1 to +12
    // from UTC.
    //
    // Lets start with that one because it is the easiest.

    if (size == 1) {
	int hours_from = 0;
	if (*start >= 'A' && *start <= 'I') {
	    hours_from = *start - 'A' + 1;
	}
	else if (*start >= 'L' && *start <= 'M') {
	    hours_from = *start - 'K' + 10;
	}
	else if (*start >= 'N' && *start <= 'Y') {
	    hours_from = ('N' - *start) - 1;
	}

	return(hours_from * 3600);
    }

    // The next option is one of the ANSI standard time zones. These
    // are three letter abbrievations that tell us where DST in in effect.
    // So, if we have a length of three, lets see if it is in the table.
    if (size == 3) {
	// First normal zones.
	int i;
	for (i = 0; i < 4; i++) {
	    if (strncmp(start, TZNames[i], 3) == 0) {
		return((5 + i) * -3600);
	    }
	}

	// Now DST zones
	for (i = 0; i < 4; i++) {
	    if (strncmp(start, TZNames[i], 3) == 0) {
		return((4 + i) * -3600);
	    }
	}
    }

    // Finally we understand +/- HHMM from UTC.
    if (size == 5) {
	int sign = (*start == '+') ? 1 : -1;

	char num_buf[10];
	strncpy(num_buf, &start[1], 2);
	num_buf[2] = 0;
	int hours = strtol(num_buf, NULL, 10);

	strncpy(num_buf, &start[3], 2);
	num_buf[2] = 0;
	int minutes = strtol(num_buf, NULL, 10);

	return(sign * ((hours * 3600) + (minutes * 60)));
    }

    // We have no idea at this point, and it is very unlikely that the
    // text is meaningful to the reader either. Set the zone to UTC and
    // punt. It is also possible that the text is "UT" or "GMT" in which
    // case offset 0 is the right answer.

    return(0);
}

DtMailValueDate
RFCValue::toDate(void)
{
    DtMailValueDate date;
    const char * pos = _value;
    tm new_time;

    memset(&date, 0, sizeof(date));
    memset(&new_time, 0, sizeof(new_time));

    date.dtm_date = 0;
    date.dtm_tz_offset_secs = 0;

    // Before doing anything, check to see if _value is valid.
    // Some messages have no Date string.  Return date with zeroed fields
    // in those cases.

    if (!_value || (strlen(_value) == 0)) return (date);

    // Find the first non-blank
    for (; *pos && isspace((unsigned char)*pos); pos++) {
	continue;
    }

    // There are usually no more than 6 tokens in an RFC date. We will
    // have a few extras just in case we are given a wierd string.
    const char *token_begin[12];
    const char *token_end[12];
    int	n_tokens = 0;

    // Look for the end of each token. Date tokens are white space
    // separated.
    while (*pos) {
	token_begin[n_tokens] = pos;
	for (; *pos && !isspace((unsigned char)*pos); pos++) {
	    continue;
	}

	if (*pos) {
	    token_end[n_tokens++] = pos - 1;
	}
	else {
	    token_end[n_tokens++] = pos;
	}

	for (; *pos && isspace((unsigned char)*pos); pos++) {
	    continue;
	}
	// This means the message is most likely corrupted so just bail out
	if (n_tokens == 12) 
		break;
    }

    // Some dates will have a comma after the day of the week. We
    // want to remove that. It will always be the first token if
    // we have the day of the week.
    if (*token_end[0] == ',') {
	token_end[0]--;
    }

    if (n_tokens < 2) {
	return(date);
    }

    // There are two possible formats, and many variations, that we
    // will see in an RFC message. They are:
    //
    // Tue Oct 12 10:36:10 1993
    // Tue, 12 Oct 1993 10:35:05 PDT
    //
    // The first is the 821 format put on by sendmail. The second is
    // one of the many variants of the 822 format. The big difference
    // we must detect is "mon dd time year" vs "dd mon year time tz"
    //
    // The first qualifier is usually the day of the week. For our purposes,
    // we will simply throw it away. This information will be recomputed
    // based on the date and time.

    int this_token = 0;

    int day = matchDay(token_begin[this_token], token_end[this_token]);
    if (day >= 0) {
	// Ignore the day.
	this_token += 1;
    }

    // This token should either be a numeric day, or an alpha month.
    // Lets see if it is a month. If so, we know what the rest of
    // the date will look like.

    int month = matchMonth(token_begin[this_token], token_end[this_token]);
    if (month >= 0) {
	new_time.tm_mon = month;

	// Now should be the day of the month.
	char num_buf[20];
	this_token += 1;

	if (this_token == n_tokens) {
	    return(date);
	}

	strncpy(num_buf, token_begin[this_token], 2);
	num_buf[2] = 0;
	new_time.tm_mday = strtol(num_buf, NULL, 10);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}

	parseTime(token_begin[this_token], token_end[this_token], new_time);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}


	// Sometimes the Unix date will include the time zone.
	//
	if (isalpha(*token_begin[this_token])) {
	    this_token += 1;
	    if (this_token == n_tokens) {
		return(date);
	    }
	}

	strncpy(num_buf, token_begin[this_token], 4);
	num_buf[token_end[this_token] - token_begin[this_token]] = 0;
	new_time.tm_year = strtol(num_buf, NULL, 10);
	if (new_time.tm_year > 1900) {
	    new_time.tm_year -= 1900;
	}

	new_time.tm_isdst = -1;
	date.dtm_date = SafeMktime(&new_time);
	date.dtm_tz_offset_secs = timezone;
    }
    else {
	// In this format, we should have a day of the month.
	char num_buf[20];
	strncpy(num_buf, token_begin[this_token], 2);
	num_buf[2] = 0;
	new_time.tm_mday = strtol(num_buf, NULL, 10);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}

	// Now the month name.
	new_time.tm_mon = matchMonth(token_begin[this_token], token_end[this_token]);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}

	// The year, which is either 2 or 4 digits.
	int t_size = token_end[this_token] - token_begin[this_token] + 1;
	strncpy(num_buf, token_begin[this_token], t_size);
	num_buf[t_size] = 0;
	new_time.tm_year = strtol(num_buf, NULL, 10);
	if (new_time.tm_year > 1900) {
	    new_time.tm_year -= 1900;
	}

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}

	// The time, in the specified time zone.
	parseTime(token_begin[this_token], token_end[this_token], new_time);

	this_token += 1;
	if (this_token == n_tokens) {
	    return(date);
	}

	time_t offset = parseTZ(token_begin[this_token], token_end[this_token]);

	time_t orig_zone = timezone;
	timezone = offset;
	new_time.tm_isdst = 1;

	date.dtm_date = SafeMktime(&new_time);
	date.dtm_tz_offset_secs = offset;

	timezone = orig_zone;
    }

    return(date);
}

static char *
findParenComment(const char * value)
{
    int in_quote = 0;
    for (const char * sparen = value; *sparen; sparen++) {
	// We must ignore stuff in quotes.
	//
	if (*sparen == '"') {
	    if (in_quote) {
		in_quote = 0;
	    }
	    else {
		in_quote = 1;
	    }
	    continue;
	}

	if (in_quote) {
	    continue;
	}

	if (*sparen == '(') {
	    break;
	}
    }

    if (*sparen != '(') {
	return(NULL);
    }

    in_quote = 0;
    for (const char * lparen = (sparen + 1); *lparen; lparen++) {
	// We will support nested comments of the form (Joe (Hi) Blow)
	//
	if (*lparen == '(') {
	    in_quote += 1;
	    continue;
	}

	if (*lparen == ')') {
	    in_quote -= 1;
	}

	if (in_quote < 0) {
	    break;
	}
    }

    if (*lparen != ')') {
	return(NULL);
    }

    char * comment = (char *)malloc(lparen - sparen + 1);
    memcpy(comment, (sparen + 1), lparen - sparen - 1);
    comment[lparen - sparen - 1] = 0;

    return(comment);
}

static char *
stripAngleAddr(const char * value)
{
    int in_quote = 0;

    for (const char * lt = value; *lt; lt++) {
	if (*lt == '"') {
	    if (in_quote) {
		in_quote = 0;
	    }
	    else {
		in_quote = 1;
	    }
	    continue;
	}

	if (in_quote) {
	    continue;
	}

	if (*lt == '<') {
	    break;
	}
    }

    if (*lt != '<') {
	return(NULL);
    }

    in_quote = 0;

    for (const char * gt = (lt + 1); *gt; gt++) {
	if (*gt == '"') {
	    if (in_quote) {
		in_quote = 0;
	    }
	    else {
		in_quote = 1;
	    }
	    continue;
	}

	if (in_quote) {
	    continue;
	}

	if (*gt == '>') {
	    break;
	}
    }

    if (*gt != '>') {
	return(NULL);
    }

    // Copy everything not in the angle brackets.
    //
    char * name = (char *)malloc(strlen(value) + 1);
    char * out = name;

    for (const char * cp = value; *cp; cp++) {
	if (cp >= lt && cp <= gt) {
	    continue;
	}

	*out++ = *cp;
    }

    *out = 0;

    if (strlen(name) == 0) {
	free(name);
	return(NULL);
    }

    return(name);
}

DtMailAddressSeq *
RFCValue::toAddress(void)
{
    // Count the commas, to figure out how big to make the
    // sequence.
    //
    int commas = 3;
    for (const char * comma = _value; *comma; comma++) {
	if (*comma == ',') {
	    commas += 1;
	}
    }

    DtMailAddressSeq * seq = new DtMailAddressSeq(commas);

    if (!_decoded) {
	decodeValue();
    }

    RFCTransport::arpaPhrase(_decoded, *seq);

    // If we have only one address, then let's try to find a comment
    // so the person can be set. This is trivial to do for one address
    // and can have a win for displaying the headers in the message
    // scrolling list.
    //
    if (seq->length() == 1) {
	// This is less than perfect, but we will look for (Name) and
	// use it first. If we can't find that, then see if we can
	// find something outside <addr>. If not that, then simply
	// give up.
	//
	DtMailValueAddress * addr = (*seq)[0];

	addr->dtm_person = findParenComment(_decoded);

	if (!addr->dtm_person) {
	    addr->dtm_person = stripAngleAddr(_decoded);
	}
    }

    return(seq);
}

const char *
RFCValue::raw(void)
{
    return(_value);
}

void
RFCValue::decodeValue(void)
{
    // Create the output buffer. We will assume that it is
    // the header will only shrink by applying RFC1522.
    //
    int fudgeFactor = 4;
    int outleft = strlen(_value);
    char * output = (char *)malloc(outleft + fudgeFactor);
    char * output_end = output + outleft + fudgeFactor;
    
    *output = 0;
    char * cur_c = output;
    
    char *buf = NULL;
    
    // Scan the value, looking for =? which indicates the start
    // of a encoded string.
    //
    for (const char * in_c = _value; *in_c; in_c++) {
	if (*in_c == '=' && *(in_c + 1) == '?') {
	    // Decode the encoding. Return the last character so the loop
	    // continues to work. Also reset cur_c because the output buffer
	    // has been updated.
	    //
	    // Allocate space for buf to contain rest of output because it is enough
	    // space for the decoded quoted-printable or base64.  If codeset
	    // conversion is done, then csConvert will re-allocate enough space.
	    buf = (char *)malloc(outleft + fudgeFactor);
	    strcpy(buf, in_c);
	    size_t _valueLen = strlen(_value);
	    in_c = decode1522(in_c, _value + _valueLen - 1, &buf, _session);
	    size_t bufLen = strlen(buf);
	    if (bufLen > outleft) {
		output = (char *)realloc((char *)output, _valueLen + bufLen + fudgeFactor);
		output_end = output + _valueLen + bufLen + fudgeFactor;
		outleft += bufLen;
	    } 
	    strncat(output, buf, bufLen);
	    cur_c = output + strlen(output);
	    outleft -= bufLen;
	    free(buf);
	}
	else {
	    // Just copy the byte and reset the null pointer, unless
	    // we are dealing with carriage return.
	    //
	    if (*in_c != '\r') {
		/*
		  if (outleft == 0) {
		  output = (char *)realloc((char *)output, (strlen(output) * 2) + fudgeFactor);
		  outleft = strlen(output);
		  output_end = (outleft * 2) + fudgeFactor;
		  }
		  */
		*cur_c = *in_c;
		cur_c++;
		*cur_c = 0;
		outleft--;
	    }
	}
    }
    
    // Kill any trailing white space.
    //
    *cur_c = 0;
    assert(cur_c >= output);
    assert(cur_c < output_end);
    for (cur_c -= 1; cur_c >= output && isspace((unsigned char)*cur_c); cur_c--) {
	*cur_c = 0;
    }
    
    _decoded = output;
}
