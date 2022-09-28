/*
 * Copyright (c) 1996 Sun Microsystems, Inc.
 */

// --> This is the Cclient Utility base class.

#pragma ident "@(#)CclientUtility.cc	1.62 97/06/11 SMI"

// Include Files
#include <Utils/CclientUtility.hh>
#include <PortObjs/DataPort.hh>
#include <SDtMail/SystemUtility.hh>
#include <SDtMail/RepSWMR.hh>
#include <mail.h>
#include <rfc822.h>
#include <fs.h>    // for fs_give and fs_get
#include <nl.h>    // for strcrlfcpy
#include <os_sv5.h>
#include <ctype.h>
#include <sys/time.h>
#include "md5.hh"
#include <VALID.h>	// replacement function for the "VALID" macro

#define CR '\015'
#define LF '\012'

// Richard McAllister complained in bug 4005934 that this 2k was wasted space
// so the code to pick a mime boundary from this list can be compiled out by
// unsetting the following flag

#ifndef MULTIPLE_BOUNDARY_STRINGS
#define MULTIPLE_BOUNDARY_STRINGS
#endif

#ifdef MULTIPLE_BOUNDARY_STRINGS

static const char *boundaryStrings[] = {
  "Ambush_of_Tigers",
  "Army_of_Caterpillars",
  "Array_of_Hedgehogs",
  "Bale_of_Turtles",
  "Band_of_Gorillas",
  "Band_of_Jays",
  "Battery_of_Barracuda",
  "Bed_of_Clams",
  "Bed_of_Mussels",
  "Bed_of_Oysters",
  "Bevy_of_Otters",
  "Blessing_of_Unicorns",
  "Bouguet_of_Pheasants",
  "Brace_of_Ducks",
  "Brace_of_Greyhounds",
  "Brood_of_Chickens",
  "Brood_of_Hens",
  "Business_of_Ferrets",
  "Caravan_of_Camels",
  "Cast_of_Falcons",
  "Cast_of_Hawks",
  "Cete_of_Badgers",
  "Charm_of_Finches",
  "Cloud_of_Gnats",
  "Cloud_of_Grasshoppers",
  "Clutter_of_Cats",
  "Cluster_of_Spiders",
  "Colony_of_Ants",
  "Colony_of_Beavers",
  "Colony_of_Frogs",
  "Colony_of_Gulls",
  "Colony_of_Rats",
  "Company_of_Parrots",
  "Convocation_of_Eagles",
  "Corps_of_Giraffes",
  "Covey_of_Partridges",
  "Covey_of_Quail",
  "Crash_of_Rhinoceros",
  "Cry_of_Hounds",
  "Den_of_Snakes",
  "Descent_of_Woodpeckers",
  "Draught_of_Fish",
  "Drey_of_Squirrels",
  "Drift_of_Hogs",
  "Drove_of_Cattle",
  "Drove_of_Donkeys",
  "Drove_of_Sheep",
  "Dule_of_Doves",
  "Exaltation_of_Larks",
  "Fall_of_Woodcocks",
  "Family_of_Sardines",
  "Farrow_of_Piglets",
  "Fleet_of_Bass",
  "Flight_of_Pigeons",
  "Flight_of_Swallows",
  "Flink_of_Cows",
  "Flock_of_Lice",
  "Flock_of_Ostrich",
  "Flurry_of_Flamingos",
  "Gaggle_of_Geese",
  "Gam_of_Porpoises",
  "Gang_of_Elks",
  "Grist_of_Bees",
  "Herd_of_Bison",
  "Herd_of_Buffalo",
  "Herd_of_Elephants",
  "Herd_of_Hippopotamuses",
  "Herd_of_Walrus",
  "Herd_of_Zebra",
  "Host_of_Sparrows",
  "Hover_of_Trout",
  "Husk_of_Hares",
  "Kindle_of_Kittens",
  "Knot_of_Toads",
  "Leap_of_Leopards",
  "Leash_of_Deer",
  "Litter_of_Pigs",
  "Litter_of_Pups",
  "Mess_of_Officers",
  "Mob_of_Kangaroos",
  "Mumble_of_Moles",
  "Murder_of_Crows",
  "Mustering_of_Storks",
  "Nest_of_Mice",
  "Nest_of_Rabbits",
  "Nest_of_Vipers",
  "Nursery_of_Raccoons",
  "Ostentation_of_Peacocks",
  "Pace_of_Asses",
  "Pack_of_Dogs",
  "Pladge_of_Wasps",
  "Plague_of_Locusts",
  "Pod_of_Dolphins",
  "Pod_of_Seals",
  "Pod_of_Whales",
  "Pop_of_Weasels",
  "Pride_of_Lions",
  "Rout_of_Wolves",
  "Rafter_of_Turkeys",
  "Rake_of_Colts",
  "Rookery_of_Penguins",
  "Scraw_of_Flies",
  "Seige_of_Cranes",
  "Shrewdness_of_Apes",
  "Skulk_of_Foxes",
  "Sleuth_of_Bears",
  "Smuck_of_Jellyfish",
  "Singular_of_Boars",
  "Sounder_of_Swine",
  "Span_of_Mules",
  "Stare_of_Owls",
  "Steam_of_Minnows",
  "String_of_Ponies",
  "Sute_of_Bloodhounds",
  "Swarm_of_Eels",
  "Swarm_of_Insects",
  "Team_of_Horses",
  "Tiding_of_Magpies",
  "Trip_of_Goats",
  "Troop_of_Antelope",
  "Troop_of_Baboons",
  "Troop_of_Monkeys",
  "Troubling_of_Goldfish",
  "Unkindness_of_Ravens",
  "Volery_of_Birds",
  "Watch_of_Nightingales",
  "Wedge_of_Swans",
  "Yoke_of_Oxen",
};

static const int numBoundaryStrings = (sizeof boundaryStrings / sizeof(char*));

#endif


/* Test for valid header.  Valid formats are:
 * 		From user Wed Dec  2 05:53 1992
 * BSD		From user Wed Dec  2 05:53:22 1992
 * SysV		From user Wed Dec  2 05:53 PST 1992
 * rn		From user Wed Dec  2 05:53:22 PST 1992
 *		From user Wed Dec  2 05:53 -0700 1992
 *		From user Wed Dec  2 05:53:22 -0700 1992
 *		From user Wed Dec  2 05:53 1992 PST
 *		From user Wed Dec  2 05:53:22 1992 PST
 *		From user Wed Dec  2 05:53 1992 -0700
 * SUN-OS	From user Wed Dec  2 05:53:22 1992 -0700
 *
 * Plus all of the above with `` remote from xxx'' after it. Thank you very
 * much, smail and Solaris, for making my life considerably more complicated.
 */

// --> hack alert <--
// We must pass "body" structures down into the bowels of the c-client in order to
// send e-mail; unfortunately, the c-client insists on deallocating all of the 
// pointers associated with those structures before it returns. This implies that
// all such pointers are allocated via malloc() as that's what the c-client expects.
// So, we added the "contents_deallocate" member to the BODY structure, and allow an
// alternative deallocation routine to be placed there which will be called to 
// deallocate that specific part. This allows us to override the deallocator called
// so that we can call the deallocater of our choosing, allowing us to allocate
// such memory with the allocator of our choosing.

// Deallocate an sdmstring object - nothing to do as the string objects are
// deallocated when the submit returns

extern "C" void __cclient_deallocate_sdmstring(void **block);
void __cclient_deallocate_sdmstring(void **block)
{
  assert(block);
  if (*block)
    *block = NULL;
}

// Deallocate an object allocated with SdmMemoryRepository::_ONew
//

extern "C" void __cclient_deallocate_sdmmemoryrepository(void **block);
void __cclient_deallocate_sdmmemoryrepository(void **block)
{
  assert(block);
  if (*block) {
    SdmMemoryRepository::_ODel(*block, Sdm_MM_AnonymousAllocation);
    *block = NULL;
  }
}

const char *
SdmCclientUtility::SkipPastFromLine(const char *line)
{
  char *line_end;
  int time_offset = 0, zone_offset, rfc822_valid = 0;

  if (!line)
    return NULL;

  // Use the c-client's "VALID" function to determine if the
  // given string is in one of the accepted "From " line formats.
  VALID((char *)line, &line_end, &time_offset, &zone_offset, &rfc822_valid);

  // If the string is valid ("time_offset" is non-zero), skip to just past the first
  // newline character.
  if (time_offset) {
    line =  strchr(line, '\n');
    assert(line);
    line += 1;
  }

  return line;
}

/* Copy string with CRLF newlines
 * Accepts: destination string
 *	    pointer to size of destination string buffer
 *	    source string
 *	    length of source string
 * Returns: length of copied string
 */

SdmBoolean SdmCclientUtility::strcrlfcpy 
  (
   char *&r_dst,		// a pointer to the destination buffer is returned here
   unsigned long &r_dstl,	// the size of the destination buffer is returned here
   const char *src,		// the source buffer to copy
   unsigned long srcl,		// the number of bytes in the source buffer
   int ensureNewlineTermination	// 0 to not ensure new line termination (!text), 1 to ensure it (text)
  )
{
  r_dst = NULL;			// No buffer allocated yet
  r_dstl = 0;			// No data placed in the buffer yet

  // Compute the size required to hold the copied and converted buffer

  // Count the number of LF's in the source string(s) and account
  // for any required new line termination that may be missing

  const char *sp;
  const char *esp;
  int csrcl;

  for (csrcl = srcl, sp = src, esp = src+srcl; sp < esp; sp++)
    if (*sp == '\012')       	// Found a new line ?
      csrcl++;			// Yes, bump count by one to account for CR required

  // Handle new line termination required - if the last character in the buffer just
  // scanned is not a new line, then account for adding a return/linefeed to the end

  if (ensureNewlineTermination && csrcl && sp[-1] != '\012')
    csrcl+=2;	// 2 for \r and \n that follows
  else
    ensureNewlineTermination = 0;	// reset flag if required but not needed

  // Now, allocate a buffer large enough to hold the final (converted) data
  char *destbuf = (char *)SdmMemoryRepository::_ONew(csrcl+1, Sdm_MM_AnonymousAllocation);
  if (!destbuf)			// Did we get the allocation?
    return(Sdm_False);		// No: return failure
  char *dp = destbuf;

  // Copy strings, inserting CR's before LF's

  while (srcl--) {
    switch (*src)
      {
      case '\015':			/* unlikely carriage return */
	*dp++ = *src++;		/* copy it and any succeeding linefeed */
	if (srcl && *src == '\012')
	  {
	    *dp++ = *src++;
	    srcl--;
	  }
	break;
      case '\012':			/* line feed? */
	*dp++ ='\015';		/* yes, prepend a CR, drop into default case */
      default:			/* ordinary chararacter */
	*dp++ = *src++;		/* just copy character */
	break;
      }
  }

  // If new line termination still required, account for it here
  if (ensureNewlineTermination)
    {
      *dp++ = '\015';		// add the CRLF sequence to terminate the buffer
      *dp++ = '\012';
    }
  *dp = '\0';			/* tie off destination */

  r_dst = destbuf;		// return address of destination buffer
  r_dstl = dp-destbuf;		// return length of data in destination buffer
  assert(r_dstl <= csrcl);	// assure that we didnt overflow the buffer

  return (Sdm_True);		// return success 
}

#define FROMLENGTH 5
SdmBoolean SdmCclientUtility::strcrlfcpy_fstuff(
  char **dst, 
  unsigned long *dstl,
  char *src, 
  unsigned long srcl,
  int ensureNewlineTermination
  )
{
  long i, j, k;
  char *d = src;
  long stuff_counter=0;
				/* flush destination buffer if too small */
  /* count number of LF's in source string(s) */
  for (i = srcl,j = 0, k = srcl; j < srcl; j++) 
    if (*d++ == LF) {
      i+=2;
      k++;
    }

  // Handle new line termination required
  if (ensureNewlineTermination && srcl && src[srcl-1] != '\012')
    i+=2;	// 2 for \r and \n that follows
  else
    ensureNewlineTermination = 0;	// reset flag if required but not needed

  // Allocate memory sufficient to hold the stuffed result
  *dst = (char *)SdmMemoryRepository::_ONew(i+1, Sdm_MM_AnonymousAllocation);
  if (!*dst) {
    *dstl = 0;
    *dst = 0;
    return(Sdm_False);		// return failure
  }
  *dstl = k;
  d = *dst;			/* destination string */

  if (srcl >= FROMLENGTH) 
    if (src[0] == 'F' && src[1] == 'r' && src[2] == 'o' && src[3] == 'm' 
        &&  src[4] == ' ') 
    {
      *d++ = '>';
      *dstl++;
    }
  
  /* copy strings, inserting CR's before LF's */
  while (srcl--) switch (*src) {
  case CR:			/* unlikely carriage return */
    *d++ = *src++;		/* copy it and any succeeding linefeed */
    if (srcl && *src == LF) {
      *d++ = *src++;
      *dstl--;
      srcl--;
      if (srcl >= FROMLENGTH) 
        if (src[0] == 'F' && src[1] == 'r' && src[2] == 'o' && src[3] == 'm' 
            &&  src[4] == ' ') 
        {
          *d++ = '>';
          *dstl++;
        }
    }
    break;
  case LF:			/* line feed? */
    *d++ =CR;		/* yes, prepend a CR, drop into default case */
    *d++ = *src++;		/* just copy character */
    if (srcl >= FROMLENGTH) 
      if (src[0] == 'F' && src[1] == 'r' && src[2] == 'o' && src[3] == 'm' 
        &&  src[4] == ' ') 
      {
        *d++ = '>';
        *dstl++;
      }
    break;
  default:			/* ordinary chararacter */
    *d++ = *src++;		/* just copy character */
    break;
  }

  // If new line termination still required, account for it here
  if (ensureNewlineTermination) {
    *d++ = '\015';		// add the CRLF sequence to terminate the buffer
    *d++ = '\012';
  }

  *d = '\0';			/* tie off destination */
  return(Sdm_True);		// return success
}

static void
_AppendParameter(BODY *body, char *name, char *value)
{
  PARAMETER *parameter, *tmp_parameter;

  parameter = mail_newbody_parameter();
  parameter->attribute = strdup(name);
  parameter->value = strdup(value);
  parameter->next = NULL;

  if(body->parameter == NULL) {
    body->parameter = parameter;
  } else {
    tmp_parameter = body->parameter;
    while(tmp_parameter->next != NULL)
      tmp_parameter = tmp_parameter->next;

    tmp_parameter->next = parameter;
  }
}

#define BINCHKLEN 32768
static SdmBoolean
_IsBinaryData(const SdmString &sdmContents)
{
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change text!
  unsigned char *text = (unsigned char *)(const char *)sdmContents;
  unsigned long len = sdmContents.Length();

  unsigned char c;
  unsigned long nbinary = 0, n8bit = 0, ncntrl = 0, nothers = 0;
  int percent;

  if (!text || !len)
    return Sdm_False;

  if (len > BINCHKLEN) len = BINCHKLEN;

  percent = len/100;
  if (percent == 0)
    percent = 2;

  for (int i = 0; i < len; i++) {
    c = *text++;
    /* Do not count newline, tab, or newpage */
    if (c == '\n' || c == '\t' || c == 014)
      continue;
    else if (c & 0x80) ++n8bit;
    else if (iscntrl(c)) ++ncntrl;
    else if (c == 0x7f) ++nothers;
  }
  /* % calculation */
  int autres = percent/2;
  if (ncntrl > autres)
    return Sdm_True;
  else if (n8bit >= percent)
    return Sdm_True;
  else if (nothers > autres)
    return Sdm_True;
  else 
    return Sdm_False;
}

#ifdef DEAD_WOOD
static SdmBoolean
_NonUsAsciiText(unsigned char *text,  unsigned long len, long *eighthBitSet)
{
  SdmBoolean haveNonAscii = Sdm_False;
  unsigned char c;

  *eighthBitSet = 0;

  if (!text)
    return Sdm_False;

  while (len-- > 0) {
    c = *text++;
    /* do not count newline, tab, or newpage */
    if (c == '\n' || c == '\t' || c == '\014')
      continue;
    if (haveNonAscii) {
      if (c & 0x80) *eighthBitSet += 1;
      continue;
    } else if ((c & 0x80) || iscntrl(c) || (c == 0x7f)) {
      if (c & 0x80) *eighthBitSet += 1;
      haveNonAscii = Sdm_True;
    }
  }
  return haveNonAscii;
}

#endif /* DEAD_WOOD */

static SdmBoolean
_NonUsAsciiText(unsigned char *text,  unsigned long len)
{
  SdmBoolean haveNonAscii = Sdm_False;
  unsigned char c;

  if (!text)
    return Sdm_False;

  while (len-- > 0) {
    c = *text++;
    /* do not count newline, tab, or newpage */
    if (c == '\n' || c == '\t' || c == '\014')
      continue;
    if ((c & 0x80) || iscntrl(c) || (c == 0x7f)) {
      haveNonAscii = Sdm_True;
      break;
    }
  }
  return haveNonAscii;
}

static short
_GetCclientMessageType(SdmMsgStrType type)
{
  switch (type) {
  case Sdm_MSTYPE_text:
    return TYPETEXT;
  case Sdm_MSTYPE_multipart:
    return TYPEMULTIPART;
  case Sdm_MSTYPE_message:
    return TYPEMESSAGE;
  case Sdm_MSTYPE_application:
    return TYPEAPPLICATION;
  case Sdm_MSTYPE_audio:
    return TYPEAUDIO;
  case Sdm_MSTYPE_image:
    return TYPEIMAGE;
  case Sdm_MSTYPE_video:
    return TYPEVIDEO;
  case Sdm_MSTYPE_other:
  default:
    return TYPEOTHER;
  }
}

// This is for "binary" types such as audio and video which should just be
// taken as-is and encoded.
// Return 'Sdm_True' if successful, 'Sdm_False' if not
//
static SdmBoolean
_DoBinary(const SdmString &sdmContents, BODY *corps, SdmBoolean sunAttach)
{
  unsigned long srcl = sdmContents.Length();
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change contents!
  char *contents = (char*)(const char*)sdmContents;

  corps->encoding = ENCBINARY;
  // Place a pointer to the SdmString object containing the contents
  // for this body part into contents.binary and set the
  // deallocation function appropriately
  corps->contents.binary = (void *)contents;
  corps->contents_deallocate = __cclient_deallocate_sdmstring;
  corps->size.bytes = srcl;

  if (!corps->contents.binary)	// Did we get the contents of the body part?
    return (Sdm_False);		// No, return failure

  if (!sunAttach) {
    SdmString md5;
    SdmCclientUtility::GetMD5Checksum(md5, contents, srcl, Sdm_False);
    corps->md5 = strdup((const char*)md5);
  }
  return(Sdm_True);		// return success
}

// Do Message/<subtype> here
static SdmBoolean
_DoMessage(const SdmString &sdmContents, BODY *corps, SdmBoolean sunAttach)
{
  unsigned long srcl = sdmContents.Length();
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change contents!
  char *contents = (char*)(const char*)sdmContents;

  unsigned long destlength = 0;
  char *desttext = 0;
  SdmBoolean ebody = (strcasecmp(corps->subtype, "EXTERNAL-BODY") == 0) ?
    Sdm_True : Sdm_False;

  if (srcl > 0) {
    if (sunAttach && !ebody) {
      // "From stuff" and Terminate lines in <cr><lf> for SMTP.
      // Both strcrlf and strcrlfcpy use the sdm memory repository - set
      // deallocate function appropriately
      (void)SdmCclientUtility::strcrlfcpy_fstuff(&desttext, &destlength, contents, srcl, 0);
      corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
    } 
    else {
      (void)SdmCclientUtility::strcrlfcpy(desttext, destlength, contents, srcl, 0);
      corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
    }
  } 
  else {
    desttext = (char *)malloc(1);
    *desttext = '\0';
  }

  if (!desttext)		// Did we get the contents of the body part?
    return(Sdm_False);		// No, return failure

  corps->encoding = ENC7BIT;
  corps->size.bytes = destlength;
  corps->contents.msg.env = NULL;
  corps->contents.msg.body  = NULL;
  corps->contents.msg.text = (char *)desttext;
  corps->contents.msg.offset = 0;  // Not used.

  // note: don't set md5 checksum in case of message according to rfc 1864.
  
  if (ebody) {
    // We need to append the extended-body parameters
#if 0
    // TODO: Figure out what this means!
    append_externalbody_params(corps, joint);
#endif
    free(corps->description);
    corps->description = strdup("External-Body");
  } else {
    // append the name parameter
#if 0
    _AppendParameter(corps, "name", joint->getLabel());
    // Let us get the description right here. WJY
    free(corps->description);
    corps->description = strdup("mail-message");
#endif
  }
  return(Sdm_True);		// return success
}

// Do APPLICATION/Octet-Stream here
static SdmBoolean
_DoOctetStream(const SdmString &sdmContents,
              BODY *corps, SdmBoolean sunAttach)
{
  unsigned long srcl = sdmContents.Length();
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change contents!
  char *contents = (char*)(const char*)sdmContents;

  void *desttext = NULL;

  corps->type = TYPEAPPLICATION;
  free(corps->subtype);
  corps->subtype = strdup("OCTET-STREAM");
  // Just push out encoding ...
  corps->encoding = ENCBINARY;
  // Place a pointer to the SdmString object containing the contents
  // for this body part into contents.binary and set the
  // deallocation function appropriately
  corps->contents.binary = (void *)contents;
  corps->contents_deallocate = __cclient_deallocate_sdmstring;
  corps->size.bytes = srcl;

  if (!corps->contents.binary)	// Did we get the contents of the body part?
    return (Sdm_False);		// No, return failure

  if (!sunAttach) {
    SdmString md5;
    SdmCclientUtility::GetMD5Checksum(md5, (char*) desttext, srcl, Sdm_False);
    corps->md5 = strdup((const char*)md5);
  }
  return(Sdm_True);		// return success
}

// Do TEXT/PLAIN quoted printable here
static SdmBoolean
_DoQuotedPrintable(const SdmString &sdmContents, BODY *corps,
                    SdmBoolean sunAttach, SdmBoolean eightbit_data)
{
  unsigned long srcl = sdmContents.Length();
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change contents!
  char *contents = (char*)(const char*)sdmContents;

  char *desttext = 0;
  unsigned long destlength = 0;

  if (srcl > 0) {
    if (eightbit_data) {
      if (sunAttach) {
        // Then, we uuencode BUT we cannot change LF to CRLF
        // because the CR will never be stripped off...
        // Ca fait du mal pour cette connerie de V3 Sun Attachments!
        corps->encoding = ENCBINARY;
	// Place a pointer to the SdmString object containing the
	// contents for this body part into desttext and set the
	// deallocation function appropriately
        desttext = contents;
	corps->contents_deallocate = __cclient_deallocate_sdmstring;
        destlength = sdmContents.Length();
      } 
      else {
        // Sending MIME: SEND CRLF for LF
	// Both strcrlf and strcrlfcpy use the sdm memory repository - set deallocate function appropriately
        (void)SdmCclientUtility::strcrlfcpy(desttext, destlength, contents, srcl, 0);
	corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
        // For MIME use quoted printable
        corps->encoding = ENC8BIT;
      }
    } 
    else {
      // No 8bit data.  Just send as text.
      if (sunAttach) {
        // We "From " ">From " because content length is sometimes wrong
	// Both strcrlf and strcrlfcpy use the sdm memory repository -
	// set deallocate function appropriately
        (void) SdmCclientUtility::strcrlfcpy_fstuff(&desttext, &destlength, contents, srcl, 0);
	corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
      } 
      else {
        (void) SdmCclientUtility::strcrlfcpy(desttext, destlength, contents, srcl, 0);
	corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
      }
      corps->encoding = ENC7BIT;
    }
    if (!sunAttach) {
      SdmString md5;
      SdmCclientUtility::GetMD5Checksum(md5, desttext, destlength, Sdm_False);
      corps->md5 = strdup((const char*)md5);
    }
  }
  else {
    desttext = (char *)malloc(1);
    *desttext = 0;
  }

  if (!desttext)		// Did we get the contents of the body part??
    return(Sdm_False);		// No, return failure

  corps->contents.text = (unsigned char *)desttext;
  corps->size.bytes = destlength;
    
  // Make sure we have the right type/subtype and charset
  corps->type = TYPETEXT;
  if (eightbit_data)
    _AppendParameter(corps, "charset", "iso-8859-1");
  else
    _AppendParameter(corps, "charset", "us-ascii");

  return(Sdm_True);		// return success
}

// Do TEXT/<subtype> here
static SdmBoolean
_DoText(const SdmString &sdmContents, BODY *corps, SdmBoolean sunAttach, 
  SdmBoolean lclAttachment, SdmString data_type_charset,
  SdmString user_charset, SdmBoolean has_attachment)
{
  unsigned long srcl = sdmContents.Length();
  // WARNING - casting away constness because c-client interface
  //           needs a char *.  Do Not change contents!
  char *contents = (char*)(const char*)sdmContents;

  SdmBoolean eightbit;
  unsigned long destlength = 0;
  char *desttext = 0;
  
  // WJY June 27, 1994 Must prevent "from stuffing"
  // Do not know of anything but Text/Plain but ..
  // We do LEAVE the subtype alone! Maybe post a warning

  corps->encoding = ENC7BIT; 
  eightbit = _NonUsAsciiText((unsigned char *)contents, srcl);

  if (!eightbit) {
    _AppendParameter(corps, "charset", "us-ascii");

    // From stuff and Terminate lines in <cr><lf> for SMTP.
    // If we send as MIME, then we dont "From stuff" because
    // Content-Length: is not needed. We have the boundary.
    // Both strcrlf and strcrlfcpy use the sdm memory repository - set deallocate function
    if (sunAttach) {
      SdmCclientUtility::strcrlfcpy_fstuff(&desttext, &destlength, contents, srcl, 1);
      corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
    }
    else {
      SdmCclientUtility::strcrlfcpy(desttext, destlength, contents, srcl, 1);
      corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
    }
    corps->contents.text = (unsigned char *)desttext;
    corps->size.bytes = destlength;
    
    if (!desttext)		// Did we get the contents of the body part??
      return(Sdm_False);	// No, return failure

    if (!sunAttach) {
      SdmString md5;
      SdmCclientUtility::GetMD5Checksum(md5, desttext, destlength, Sdm_False);
      corps->md5 = strdup((const char*) md5);
    }
    
  }
  else {
  
    // I18N
    // content type is supposed to be always TEXT
    // IF NOT, before using this routine, DETERMINE 
    // if the content type is TEXT or not
    // This routine should be used for only TEXT type
    char *body_charset = NULL;
    int body_encoding;
    size_t new_body_length = 0;
    char *new_body = SdmMessageUtility::ConvertBody(new_body_length, contents,
	srcl, &body_charset, &body_encoding, lclAttachment, data_type_charset,
	user_charset, sunAttach, has_attachment);
    /* if user_charset != NULL try default conversion */
    if((new_body == (char *)NULL) && ((const char *)user_charset != (char *)NULL)){
      SdmString	null_str((const char *)NULL);
      new_body = SdmMessageUtility::ConvertBody(new_body_length, contents,
	srcl, &body_charset, &body_encoding, lclAttachment, data_type_charset,
	null_str, sunAttach, has_attachment);
    }
    if(new_body){
      corps->encoding = body_encoding;
      _AppendParameter(corps, "charset", body_charset);
      free (body_charset);

      if (body_encoding != ENCBINARY){
	// From stuff and Terminate lines in <cr><lf> for SMTP.
	// If we send as MIME, then we dont "From stuff" because
	// Content-Length: is not needed. We have the boundary.
	// Both strcrlf and strcrlfcpy use the sdm memory repository - set deallocate function
	if (sunAttach) {
	  SdmCclientUtility::strcrlfcpy_fstuff(&desttext, &destlength, new_body, new_body_length, 1);
	  corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
	}
	else {
	  SdmCclientUtility::strcrlfcpy(desttext, destlength, new_body, new_body_length, 1);
	  corps->contents_deallocate = __cclient_deallocate_sdmmemoryrepository;
	}
	free(new_body);	
      }
      else {
	desttext = new_body;
	destlength = new_body_length;
      }

      if (!desttext)		// Did we get the contents of the body part?
	return(Sdm_False);	// No, return failure

      corps->contents.text = (unsigned char *)desttext;
      corps->size.bytes = destlength;

      if (!sunAttach) {
        SdmString md5;
        SdmCclientUtility::GetMD5Checksum(md5, desttext, destlength,
					  Sdm_False);
        corps->md5 = strdup((const char*) md5);
      }
    }
    else {
      // if binary data is > 1% , then we send it as application/octet
      // stream .. possibly a tar file.   LS June 2 1995
      if (_IsBinaryData(sdmContents)) {
        if (_DoOctetStream(sdmContents, corps, sunAttach) != Sdm_True)
	  return(Sdm_False);		// failed - return failure
        _AppendParameter(corps, "charset", "iso-8859-1");
      }
      else {
        // for all other TEXT/<subtype> parts we qp/uuencode the original
        // source. Sets the charset there appropriately.
        if (_DoQuotedPrintable(sdmContents, corps, sunAttach, eightbit) != Sdm_True)
	  return(Sdm_False);		// failed - return failure
      }
    }

    // I18N - end
  }
  return(Sdm_True);		// return success
}

static SdmErrorCode
_ConstructBodyPart(
  SdmError &err,
  SdmDataPort &dataPort,
  const SdmMessageNumber msgnum,
  const SdmDpMessageComponent &mcmp,
  BODY *body,
  SdmBoolean sunAttach,
  SdmBoolean lclAttachment,	// I18N code for lcl lib
  SdmBoolean has_attachment	// I18N code for lcl lib
)
{
  // the component should not be deleted.
  assert (mcmp.IsComponentDeleted() == Sdm_False);
  
  SdmContentBuffer contentBuffer;
  SdmString sdmContents;
  if (dataPort.GetMessageContents(err, contentBuffer,
                                  DPCTTYPE_processedcomponent, msgnum,
                                  mcmp) != Sdm_EC_Success)
    return (err);

  if (contentBuffer.GetContents(err, sdmContents) != Sdm_EC_Success)
    return (err);

  // I18N - get the data type charset
  SdmString data_type_charset;
  if (contentBuffer.GetDataTypeCharSet(err,data_type_charset) != Sdm_EC_Success)
    return (err);
  
  // I18N - get the user specified charset
  SdmString user_charset;
  if (contentBuffer.GetCharacterSet(err, user_charset) != Sdm_EC_Success)
    return (err);
  
  SdmBoolean success;

  switch (body->type) {
  case TYPETEXT:
    success = _DoText(sdmContents, body, sunAttach, lclAttachment, data_type_charset, user_charset, has_attachment);
    break;
  case TYPEMESSAGE:
    success = _DoMessage(sdmContents, body, sunAttach);
    break;
  default:
    success = _DoBinary(sdmContents, body, sunAttach);
    break;
  }
  
  return (err = (success == Sdm_True ? Sdm_EC_Success : Sdm_EC_NoMemory));
}

static SdmErrorCode
_AttachmentsToParts(
  SdmError &err,
  PART* &r_parts,
  SdmDataPort &dataPort,
  const SdmMessageNumber msgnum,
  SdmDpMessageStructureL &ms_ms,
  SdmBoolean sunAttach,
  char *toplevelBoundary,
  int nestingLevel,
  SdmBoolean *lclAttachment	// I18N code for lcl lib
)
{
  PART *tmppart = NULL, *headpart = NULL, *lastpart;
  BODY *corps;
  const char *s;
  
  err = Sdm_EC_Success;    // initialize error to success.

  for (int i = 0; i < ms_ms.ElementCount(); i++) {
    SdmDpMessageStructure *dpms = ms_ms[i];
    
    // ignore deleted components.
    if (!dpms->ms_component->IsComponentDeleted()) {

      tmppart = mail_newbody_part();
      if (headpart == NULL) 
        headpart = tmppart;
      else
        lastpart->next = tmppart;

      // If the description field of the attachment is not NULL then use
      // it. Otherwise, set the description field to the attachment name.
      corps = &tmppart->body;
      if ((s = (const char*)dpms->ms_description) && *s)
        corps->description = strdup(s);
      else if ((s = (const char*)dpms->ms_attachment_name) && *s)
        corps->description = strdup(s);

      // I18N convert and encode string
      SdmMessageUtility::EncodeHeader(&(corps->description), "Content-Description: ", *lclAttachment, sunAttach);
      // I18N end

      // Append the name parameter
      // WARNING - casting away constness because c-client interface
      //           needs a char *.  Do Not change s!
      if ((s = (char*)(const char*)dpms->ms_attachment_name) && *s) {
	char *tmp_str = (char *)malloc(strlen(s) + 1);
	strcpy(tmp_str, s);
	// I18N convert and encode string
	SdmMessageUtility::EncodeHeader(&tmp_str, "Content-Type: ", *lclAttachment, sunAttach);
	_AppendParameter(corps, "name", tmp_str);
	free(tmp_str); // IF _AppendParameter() copy the content of tmp_str
      }

      corps->type = _GetCclientMessageType(dpms->ms_type);

      if (s = (const char*) dpms->ms_subtype)
        corps->subtype = strdup(s);

      if (corps->type == TYPEMULTIPART) {
        int i;
        SdmBoolean sunAttach = Sdm_False;
        PART *parts = NULL;

        if (_AttachmentsToParts(err, parts, dataPort, msgnum, *dpms->ms_ms,
                                 sunAttach, toplevelBoundary,
                                nestingLevel + 1, lclAttachment) != Sdm_EC_Success)
        {
          break;  // break out of loop because of error
        }

        corps->contents.part = parts;

        char nestedBoundary[SMALLBUFLEN];
        sprintf(nestedBoundary, "%s_%.3d", toplevelBoundary, nestingLevel);
        _AppendParameter(corps, "BOUNDARY", nestedBoundary);
      }
      else if (_ConstructBodyPart(err, dataPort, msgnum, *dpms->ms_component,
	corps, sunAttach, *lclAttachment, Sdm_True))
      {
        break;  // break out of loop because of error.
      }

      // Finally, apply any given x-unix-mode setting as a parameter
      if (dpms->ms_unix_mode) {
        char modeString[5];

        sprintf(modeString, "%.4o", dpms->ms_unix_mode & 07777);
	_AppendParameter(corps, "x-unix-mode", modeString);
      }

      *lclAttachment = Sdm_True; // I18N code
      lastpart = tmppart;
      lastpart->next = NULL;
    }
  }

  if (err == Sdm_EC_Success) {
    r_parts = headpart;
  } else if (headpart) {
    // error occurred.  clean up parts list.
    mail_free_body_part(&headpart);
  }
  
  return (err);
}

// This function creates the envelope for the given message.  The caller
// must provide a pointer to a valid ENVELOPE structure by calling
// mail_new_envelope.  It is the caller's responsibility to free the
// envelope by calling mail_free_envelope.

SdmErrorCode SdmCclientUtility::ConstructMessageEnvelope(
  SdmError& err,
  ENVELOPE *env,
  SdmDataPort &dataPort,
  const SdmMessageNumber msgnum,
  SdmBoolean sunAttach
  )
{
  char tmp[SMALLBUFLEN];

  // Fetch the headers required to construct the envelope
  SdmIntStrL hdrs;
  if (dataPort.GetMessageHeaders(err, hdrs,
                                 Sdm_MHA_P_MessageFrom | Sdm_MHA_P_To |
                                 Sdm_MHA_P_Cc | Sdm_MHA_P_Bcc |
                                 Sdm_MHA_P_Subject, msgnum) != Sdm_EC_Success)
    return (err);

  int len;
  if ((len = hdrs.ElementCount()) == 0)
    return (err = Sdm_EC_Fail);

  char *from = NULL, *to = NULL, *cc = NULL, *bcc = NULL, *subject = NULL;

  for (int i = 0; i < len; i++) {
    switch (hdrs[i].GetNumber()) {
    case Sdm_MHA_P_MessageFrom:
      from = strdup ((const char*)(hdrs[i].GetString()));
      // I18N
      SdmMessageUtility::EncodeHeader(&from, "From: ", Sdm_False, sunAttach);
      break;
      
    // need to check for the various types of headers for "to"
    case Sdm_MHA_P_To:
    case Sdm_MHA_To:
    case Sdm_MHA_Apparently_To:
    case Sdm_MHA_Resent_To:
      to = strdup ((const char*)(hdrs[i].GetString()));
      // I18N
      SdmMessageUtility::EncodeHeader(&to, "To: ", Sdm_False, sunAttach);
      break;
    case Sdm_MHA_P_Cc:
      cc = strdup ((char*)(const char *)(hdrs[i].GetString()));
      // I18N
      SdmMessageUtility::EncodeHeader(&cc, "cc: ", Sdm_False, sunAttach);
      break;
    case Sdm_MHA_P_Bcc:
      bcc = strdup ((char*)(const char*)(hdrs[i].GetString()));
      // I18N
      SdmMessageUtility::EncodeHeader(&bcc, "bcc: ", Sdm_False, sunAttach);
      break;
    case Sdm_MHA_P_Subject:
      subject = strdup ((char*)(const char*)(hdrs[i].GetString()));
      // I18N
      SdmMessageUtility::EncodeHeader(&subject, "Subject: ", Sdm_False, sunAttach);
      break;
    }
  }

  struct passwd pw;
  SdmSystemUtility::GetPasswordEntry(pw);
  char *personalName = SdmSystemUtility::GetQuotedPersonalName();

  SdmString sendingUserName;	// This is the user name that this mail appears to come from
  SdmMessageUtility::GetMailSendingUserName(sendingUserName);

  SdmString sendingHostName;	// This is the host name that this mail appears to come from
  SdmMessageUtility::GetMailSendingHostName(sendingHostName);

  if (!sendingUserName.Length() || !sendingHostName.Length()) {
    if (from) free (from);
    if (to) free (to);
    if (cc) free (cc);
    if (bcc) free (bcc);
    if (subject) free (subject);
    return (err = Sdm_EC_Fail);
  }

  if (!from) {
    from = (char *)malloc(SMALLBUFLEN);
    sprintf(from, "%s <%s@%s>", 
	    personalName ? personalName : "",
            (const char *) sendingUserName, 
	    (const char *) sendingHostName);
    SdmMessageUtility::EncodeHeader(&from, "From: ", Sdm_False, sunAttach);
  }
  rfc822_parse_adrlist(&env->from, from, (char *)((const char *)sendingHostName));
  SdmBoolean senderFound = Sdm_False;

  if (to && *to) {
    rfc822_parse_adrlist(&env->to, to, (char *)((const char *)sendingHostName));
    senderFound = Sdm_True;
  }

  if (cc && *cc) {
    rfc822_parse_adrlist(&env->cc, cc, (char *)((const char *)sendingHostName));
    senderFound = Sdm_True;
  }

  if (bcc && *bcc) {
    rfc822_parse_adrlist(&env->bcc, bcc, (char *)((const char *)sendingHostName));
    senderFound = Sdm_True;
  }

  // return error if no sender specified.
  if (!senderFound) {
    if (from) free (from);
    if (to) free (to);
    if (cc) free (cc);
    if (bcc) free (bcc);
    if (subject) free (subject);
    return (err = Sdm_EC_Fail);
  }

  rfc822_parse_adrlist(&env->return_path, 
		       (char *)((const char *)sendingUserName), 
		       (char *)((const char *)sendingHostName) );

  if (subject)
    env->subject = strdup(subject);

  //
  // if reply-to header defined for this message, use it for
  // the reply_to field.  otherwise, use the "from" address.
  //
  SdmIntStrL replyToHdr;
  if (dataPort.GetMessageHeaders(err, replyToHdr, Sdm_MHA_Reply_To, msgnum) != Sdm_EC_Success) {
    if (err == Sdm_EC_RequestedDataNotFound) {
      env->reply_to = rfc822_cpy_adr(env->from);
      err.Reset();  
    } else {
      if (from) free (from);
      if (to) free (to);
      if (cc) free (cc);
      if (bcc) free (bcc);
      if (subject) free (subject);
      return (err);
    }
  } else {    // we found the reply-to header in this message.
    assert (replyToHdr.ElementCount());
    // if the value for the reply-to header is an empty string, use the
    // value of the from header for the reply-to header.
    if (replyToHdr[0].GetString().Length() == 0) {
      env->reply_to = rfc822_cpy_adr(env->from);
    } else {
      char *replyTo = strdup ((const char*)(replyToHdr[0].GetString()));
      SdmMessageUtility::EncodeHeader(&replyTo, "Reply-To: ", Sdm_False, sunAttach);
      rfc822_parse_adrlist(&env->reply_to, replyTo,  (char *)((const char *)sendingHostName));
      free (replyTo);
    }
  }

  // Set the In-Reply-To header to any existing Message-Id header in the
  // original message.
  SdmStrStrL hdr;
  SdmString messageId("Message-Id");
  err = dataPort.GetMessageHeader(err, hdr, messageId, msgnum);

  if (err != Sdm_EC_Success && err != Sdm_EC_RequestedDataNotFound) {
    if (from) free (from);
    if (to) free (to);
    if (cc) free (cc);
    if (bcc) free (bcc);
    if (subject) free (subject);
    return (err);
  }

  if (err == Sdm_EC_Success) {
    assert (hdr.ElementCount());
    env->in_reply_to = strdup((const char*)hdr[0].GetSecondString());
  }
  else  // err must be Sdm_EC_RequestedDataNotFound, reset it.
    err = Sdm_EC_Success;

  rfc822_date(tmp);
  env->date = strdup(tmp);

  env->message_id = NULL;	// leave this to the smtp agent

  if (from) free (from);
  if (to) free (to);
  if (cc) free (cc);
  if (bcc) free (bcc);
  if (subject) free (subject);
  return (err = Sdm_EC_Success);
}

// Verify that the given message structure appears to contain "sane"
// contents.
static SdmErrorCode
_VerifyMessageStructure(SdmDpMessageStructureL &structure)
{
  int len = structure.ElementCount();
  SdmBoolean componentFound = Sdm_False;
  
  if (len == 0)
    return (Sdm_EC_Fail);

  for (int i = 0; i < len; i++) {
    assert (structure[i]->ms_component != NULL);

    // ignore deleted components
    if (!structure[i]->ms_component->IsComponentDeleted()) {
    
      // If message is multipart, then it must have nested parts; if it has
      // nested parts, then it must be multipart.
      if ((structure[i]->ms_type == Sdm_MSTYPE_multipart) ^
          (structure[i]->ms_ms != NULL))
        return (Sdm_EC_Fail);

      if (structure[i]->ms_ms) {
        if (_VerifyMessageStructure(*structure[i]->ms_ms) != Sdm_EC_Success)
          return (Sdm_EC_Fail);
      }

      componentFound = Sdm_True;
    }
  }

  // there should be at least one component defined.
  if (!componentFound) {
    return (Sdm_EC_Fail);
  }

  return (Sdm_EC_Success);
}

// This function creates the body for the given message.  The caller must
// provide a pointer to a valid BODY structure by calling mail_new_body.
// It is the caller's responsibility to free the body by calling
// mail_free_body.

SdmErrorCode SdmCclientUtility::ConstructMessageBody(
  SdmError& err,
  BODY *body,
  SdmDataPort &dataPort,
  const SdmMessageNumber msgnum,
  SdmBoolean sunAttach
  )
{
  SdmDpMessageStructureL structure;
  SdmContentBuffer content_buffer;
  
  // we need to set the delete flag for the structure list
  // otherwise the message structures in the list don't get deleted.
  structure.SetPointerDeleteFlag();

  if (dataPort.GetMessageStructure(err, structure, msgnum) != Sdm_EC_Success)
    return (err);

  // loop through the components.  find the first non-deleted body part
  // and determine the total number of body parts.  return error if more
  // than one top-level body part is found.
  int start = -1;    
  int totalComponents = 0;
  for (int i=0; i< structure.ElementCount(); i++) {
    if (!structure[i]->ms_component->IsComponentDeleted()) {
      if (start < 0) {
        start = i;  // save the first non-deleted component.
      }
      
      totalComponents++;
      // MIME only allows one toplevel body part
      if (totalComponents > 1)
        return (err = Sdm_EC_Fail);
    }
  }
  
  // return error if no top-level body part is found.
  if (start < 0) {
    return (err = Sdm_EC_Fail);
  }

  // check the data in the structure list.
  if (_VerifyMessageStructure(structure) != Sdm_EC_Success)
    return (err = Sdm_EC_Fail);

  body->type = _GetCclientMessageType(structure[start]->ms_type);
  const char *s = (const char*) structure[start]->ms_subtype;
  body->subtype = (s && *s) ? strdup(s) : NULL;

  if (structure[start]->ms_ms) {
    assert(body->type == TYPEMULTIPART);

    if (body->subtype == NULL)
      body->subtype = strdup("MIXED");

#ifdef  MULTIPLE_BOUNDARY_STRINGS
    // Construct the toplevel boundary
    char boundary[SMALLBUFLEN];
    int r = SdmSystemUtility::Random();
    sprintf(boundary, "%s_%.3d",
            boundaryStrings[r % numBoundaryStrings], r % 1000);
#else
    // Construct the toplevel boundary
    char boundary[SMALLBUFLEN];
    sprintf(boundary, "libSDtMail_rfc1521_boundary_B%x_O%x-R%x_I%x-N%x_G%x", SdmSystemUtility::Random(), SdmSystemUtility::Random(), SdmSystemUtility::Random(),
	    SdmSystemUtility::Random(), SdmSystemUtility::Random(), SdmSystemUtility::Random());
    assert(strlen(boundary)<SMALLBUFLEN);
#endif

    PART *parts = NULL;
    SdmBoolean	lclattachment = Sdm_False;	// I18N attachment flag
    if (_AttachmentsToParts(err, parts, dataPort, msgnum, *structure[start]->ms_ms,
                            sunAttach, boundary, 1, &lclattachment) != Sdm_EC_Success)
      return (err);

    body->contents.part = parts;

    strcat(boundary, "_000");
    _AppendParameter(body, "BOUNDARY", boundary);
  }
  else {
    if (_ConstructBodyPart(err, dataPort, msgnum, *structure[start]->ms_component,
          body, sunAttach, Sdm_False, Sdm_False) != Sdm_EC_Success)
        return err;
  }
  
  return (err = Sdm_EC_Success);
}

void 
SdmCclientUtility::GetMD5Checksum(
  SdmString &r_digest, const char* buffer, 
  const unsigned long length, SdmBoolean doCanonicalFormText)
{
  unsigned char digest[16];
  memset(digest, 0, sizeof(digest));

  MD5_CTX context;
  MD5Init(&context);
  
  // Handle case when buffer contains text that's not in canonical form.  
  // We need to compute the md5 signature based on a message that has
  // the CRLF line terminator. Most of our buffers don't so we will need
  // to scan the body and do some magic. The approach will be to sum
  // one line at a time. If the buffer doesn't have CRLF we will do that
  // independently.
  //
  if (doCanonicalFormText) {
    unsigned char * local_crlf = (unsigned char *)"\r\n";
    const char *last = buffer;
    for (const char * cur = buffer; cur < (buffer + length); cur++) {
      if (*cur == '\n') {
        if (cur == buffer || *(cur - 1) == '\r') {
          MD5Update(&context, (unsigned char *)last, cur - last + 1);
        }
        else {
          MD5Update(&context, (unsigned char *)last, cur - last);
          MD5Update(&context, local_crlf, 2);
        }
        last = cur + 1;
      }
    }
 
    if (buffer[length - 1] != '\n') {
      // Need to sum the trailing fraction with a CRLF.
      MD5Update(&context, (unsigned char *)last, cur - last);
      MD5Update(&context, local_crlf, 2);
    }
        
  } else {
    MD5Update(&context, (unsigned char *)buffer, length);
  }
  
  MD5Final(digest, &context);
  
  unsigned long len;
  unsigned char* base64_digest = rfc822_binary(digest, 16, &len);

  // remove CRLF from the digest that got added by the rfc822_binary function.
  if (len >= 2 && base64_digest[len-1] == LF && base64_digest[len-2] == CR) {
    base64_digest[len-2] = '\0';
    len -= 2;
  }
  SdmString result((char*)base64_digest, len);
  free (base64_digest);    // need to free memory of string returned from rfc822_binary.
  
  r_digest = result;
}
