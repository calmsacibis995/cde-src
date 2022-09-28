/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)safiletest.c	1.21 96/08/14 SMI"

/*
 * SAFile (Sun Audio File) API Tests
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/param.h>

#include "sa.h"
#include "safile.h"


void
Test_01()
{
    SAFile   safile;
    SAError  error;
    char     pathname[MAXPATHLEN];

    printf("=====> Testing SAFileOpen <=====\n");
    printf("Please enter a path to an audio file: ");
    scanf("%s", &pathname);

    if (pathname == NULL)
      return;

    if (SAFileOpen(pathname, &safile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_01 with file: %s!\n", pathname);
      printf("=====> Test_01 FAILED <=====\n");    
      return;
    }

    printf("File: %s", pathname);
    printf("Format: ");
    switch (safile.filetype) {
    case SASunType:
      printf("SASunType\n");
      break;
    case SAWavType:
      printf("SAWavType\n");
      break;
    case SAAiffType:
      printf("SAAiffType\n");
      break;
    case SANoType:
      printf("Unrecognized Format\n");
      break;
    }

    printf("=====> Test_01 PASSED <=====\n");

}

void
Test_02()
{
    SAFile   safile;
    SAError  error;
    char     pathname[MAXPATHLEN];
    char     buf[2048];
    int      nread;

    printf("=====> Testing SAFileRead <=====\n");
    printf("Please enter a path to an audio file: ");
    scanf("%s", &pathname);

    if (pathname == NULL)
      return;

    if (SAFileOpen(pathname, &safile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_02 with file: %s!\n", pathname);
      return;
    }

    SAFileRead (&safile, buf, 2048, &nread, &error);

    if (nread <= 0) {
      printf("SAFileRead() failed!\n");
      printf("=====> Test_02 FAILED <=====\n");
    }
    else
      printf("SAFileRead() read %d bytes from %s.\n", nread, pathname);

    printf("=====> Test_02 PASSED <=====\n");    
}

void
Test_03()
{
    SAFile   safile;
    SAError  error;
    char     pathname[MAXPATHLEN];

    printf("=====> Testing SAFileSeek <=====\n");
    printf("Please enter a path to an audio file: ");
    scanf("%s", &pathname);

    if (pathname == NULL)
      return;

    if (SAFileOpen(pathname, &safile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_03 with file: %s!\n", pathname);
      printf("=====> Test_03 FAILED <=====\n");
      return;
    }

    if (SAFileSeek (&safile, 1, &error) == SAFalse) {
      printf("SAFileSeek() failed!\n");
      printf("=====> Test_03 FAILED <=====\n");
    }

    printf("=====> Test_03 PASSED <=====\n");    
}

void
Test_04()
{
    SAFile   safile;
    SAError  error;
    char     pathname[MAXPATHLEN];

    printf("=====> Testing SAFileClose <=====\n");
    printf("Please enter a path to an audio file: ");
    scanf("%s", &pathname);

    if (pathname == NULL)
      return;

    if (SAFileOpen(pathname, &safile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_04 with file: %s!\n", pathname);
      return;
    }

    SAFileClose (&safile, &error);

    printf("=====> Test_04 PASSED <=====\n");    
}

void
Test_05()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Sun 8-bit Ulaw to Sun 8-bit Ulaw <=====\n");
    printf("Please enter a path to an 8-bit Ulaw file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a new 8-bit Ulaw file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_05 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse) {
      printf("SAConvertFormat() failed in Test_05!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_05 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {
	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize,&error);
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_05 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_05 PASSED <=====\n");    
}

void
Test_06()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Sun 16-bit Linear to Sun 16-bit Linear <=====\n");
    printf("Please enter a path to a Sun 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_06 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse) {
      printf("=====> Test_06 FAILED <=====\n");    
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_06 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize,&error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_06 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_06 PASSED <=====\n");    
}

void
Test_07()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Wav 8-bit Linear to Sun 8-bit Ulaw <=====\n");
    printf("Please enter a path to a Wav 8-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a new Sun 8-bit Ulaw file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_07 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse) {
      printf("SAConvertFormat() failed in Test_07!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_07 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize,&error);
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_07 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_07 PASSED <=====\n");    
}

void
Test_08()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Sun 8-bit Ulaw to Wav 8-bit Linear <=====\n");
    printf("Please enter a path to a Sun 8-bit Ulaw file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_08 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAWavType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse) {
      printf("SAConvertFormat() failed in Test_08!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.format.channels = infile.format.channels;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_08 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize,&error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_08 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_08 PASSED <=====\n");    
}

void
Test_09()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*2];

    printf("=====> Testing Sun 16-bit Linear to Wav 8-bit Linear <=====\n");
    printf("Please enter a path to a Sun 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_09 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAWavType;
 
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_09!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_09 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_09 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;	
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_09 PASSED <=====\n");    
}

void
Test_10()
{
    SAFile   infile;
    SAFormat outfmt;
    SAError  error;
    int      outfiletype;
    char     inname[MAXPATHLEN];

    printf("Please enter a path to an audio file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("  1.  Sun (Au)\n ");
    printf("  2.  Wav\n ");
    printf("  3.  Aiff\n ");
    printf("Please enter the number of the format to convert to: ");
    scanf("%d", &outfiletype); 
    if (outfiletype == NULL)
      return;
    outfiletype--;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_10 with file: %s!\n", inname);
      return;
    }

    printf("The file you selected is:\n");
    printf("  Encoding: ");
    if (infile.format.encoding == SAEncoding_LINEAR) {
      printf("Linear\n");
    }
    else if (infile.format.encoding == SAEncoding_ULAW) {
      printf("Ulaw\n");
    }
    else if (infile.format.encoding == SAEncoding_ALAW) {
      printf("Alaw\n");
    }
    else {
      printf("=====> Test_10 FAILED <=====\n");    
      return;
    }
    printf("  Bytes Per Unit: %d\n", infile.format.sample_size);
    /*    printf("  Channels: %d\n\n", infile.format.channels);*/

    if ((SAConvertFormat(infile.format.encoding, 
			 infile.format.sample_size,
			 infile.format.channels,
			 infile.format.samples_per_unit,
			 (SAFileType) outfiletype, 
			 &outfmt, &error)) == SAFalse) {
      printf("SAConvertFormat() failed in Test_10 with file: %s!\n", inname);
      return;
    }

    printf("The file will be converted to:\n");
    printf("  Encoding: ");
    if (outfmt.encoding == SAEncoding_ULAW)
      printf("Ulaw\n");
    else if (outfmt.encoding == SAEncoding_ALAW)
      printf("Alaw\n");
    else if (outfmt.encoding == SAEncoding_LINEAR)
      printf("Linear\n");
    else {
      printf("=====> Test_10 FAILED <=====\n");    
      return;
    }
    printf("  Bytes Per Unit: %d\n", outfmt.sample_size);
    printf("  Signed Data: %d\n", outfmt.flags && SAFormatFlag_Signed);
    printf("  Channels: %d\n", outfmt.channels);

    SAFileClose (&infile, &error);

    printf("=====> Test_10 PASSED <=====\n");    
}

void
Test_11()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*2];

    printf("=====> Testing Sun 8-bit Ulaw to Aiff 8-bit Linear <=====\n");
    printf("Please enter a path to a Sun 8-bit Ulaw file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Aiff 8-bit Linear: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_11 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAAiffType;
 
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_11!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.format.sample_size = infile.format.sample_size;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_11 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_11 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_11 PASSED <=====\n");    
}

void
Test_12()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*2];

    printf("=====> Testing Sun 16-bit Linear to Wav 16-bit Linear <=====\n");
    printf("Please enter a path to a Sun 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_12 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAWavType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_12!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_12 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_12 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_12 PASSED <=====\n");    
}

void
Test_13()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*2];

    printf("=====> Testing Sun 16-bit Linear to Aiff 16-bit Linear <=====\n");
    printf("Please enter a path to a Sun 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Aiff 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_13 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAAiffType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit
,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_13!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_13 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_13 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_13 PASSED <=====\n");    
}

void
Test_14()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*2];

    printf("=====> Testing Sun 16-bit Linear to Sun 16-bit Linear <=====\n");
    printf("Please enter a path to a Wav 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_14 with file: %s!\n", inname);
      return;
    }
 
    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_14!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_14 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_14 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_14 PASSED <=====\n");    
}

void
Test_15()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Wav 16-bit Linear to Aiff 16-bit Linear <=====\n");
    printf("Please enter a path to a Wav 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Aiff 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_15 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAAiffType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_15!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_15 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_15 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_15 PASSED <=====\n");    
}


void
Test_16()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Aiff 16-bit Linear to Wav 16-bit Linear <=====\n");
    printf("Please enter a path to a Aiff 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_16 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAWavType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_16!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_16 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_16 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_16 PASSED <=====\n");    
}

void
Test_17()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Aiff 16-bit Linear to Sun 16-bit Linear <=====\n");
    printf("Please enter a path to a Aiff 16-bit Linear file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 16-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_17 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_17!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_17 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != nbytes_read) {
	  printf("=====> Test_17 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_17 PASSED <=====\n");    
}

void
Test_18()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ];

    printf("=====> Testing Sun Ulaw 2 channels to Sun Ulaw 1 channel <=====\n");
    printf("Please enter a path to a Sun 8-bit Ulaw, 2 channel file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 8-bit Ulaw, 1 channel file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_18 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
			 infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
	) {
	printf("SAConvertFormat() failed in Test_18!\n");
	return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;
 
    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_18 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_18 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_18 PASSED <=====\n");    
}

void
Test_19()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 4-bit G.721 ADPCM to Sun 8-bit Ulaw <=====\n");
    printf("Please enter a path to a Sun 4-bit G.721 ADPCM file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 8-bit Ulaw: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_19 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SASunType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_19!\n");
      return;
    }

    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_19 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_19 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_19 PASSED <=====\n");    
}

void
Test_20()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 4-bit G.721 ADPCM to Wav 8-bit Linear <=====\n");
    printf("Please enter a path to a Sun 4-bit G.721 ADPCM file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_20 with file: %s!\n", inname);
      return;
    }

    outfile.filetype = SAWavType;

    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_20!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = 1;
    outfile.format.sample_size = 1;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_20 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_20 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_20 PASSED <=====\n");    
}

void
Test_21()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 4-bit G.721 ADPCM to Aiff 8-bit Linear <=====\n");
    printf("Please enter a path to a Sun 4-bit G.721 ADPCM file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Aiff 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    outfile.filetype = SAAiffType;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_21 with file: %s!\n", inname);
      return;
    }
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_21!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_21 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_21 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_21 PASSED <=====\n");    
}

void
Test_22()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 8-bit Alaw 2 channels to Sun 8-bit Ulaw 1 channel <=====\n");
    printf("Please enter a path to a Sun 8-bit Alaw 2 channel file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Sun 8-bit Ulaw file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    outfile.filetype = SASunType;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_22 with file: %s!\n", inname);
      return;
    }
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_22!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_22 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_22 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_22 PASSED <=====\n");    
}

void
Test_23()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 8-bit Alaw 2 channels to Wav 8-bit Linear 1 channel <=====\n");
    printf("Please enter a path to a Sun 8-bit Alaw 2 channel file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Wav 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    outfile.filetype = SASunType;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_23 with file: %s!\n", inname);
      return;
    }
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_23!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_23 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_23 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_23 PASSED <=====\n");    
}

void
Test_24()
{
    int      nbytes_read = 1, nbytes_written;
    int      ibufsize, obufsize;
    SAFile   infile, outfile;
    SAError  error;
    char     inname[MAXPATHLEN];
    char     outname[MAXPATHLEN];
    char     ibuf[BUFSIZ], obuf[BUFSIZ*4];

    printf("=====> Testing Sun 8-bit Alaw 2 channels to Aiff 8-bit Linear 1 channel <=====\n");
    printf("Please enter a path to a Sun 8-bit Alaw 2 channel file: ");
    scanf("%s", &inname); 
    if (inname == NULL)
      return;
    printf("Please enter a path to a non-existing Aiff 8-bit Linear file: ");
    scanf("%s", &outname);
    if (outname == NULL)
      return;

    outfile.filetype = SASunType;

    if (SAFileOpen(inname, &infile, &error) == SAFalse) {
      printf("SAFileOpen() failed in Test_24 with file: %s!\n", inname);
      return;
    }
    if ((SAConvertFormat(infile.format.encoding, infile.format.sample_size,
                         infile.format.channels, infile.format.samples_per_unit,
                         outfile.filetype, &outfile.format, &error)) == SAFalse
) {
      printf("SAConvertFormat() failed in Test_24!\n");
      return;
    }
 
    outfile.format.sample_rate = infile.format.sample_rate;
    outfile.format.samples_per_unit = infile.format.samples_per_unit;
    outfile.data_size = infile.data_size;
    outfile.header_size = infile.header_size;
    outfile.header_size = infile.header_size;

    if (SAFileCreate(outname, &outfile, &error) == SAFalse) {
      printf("SAFileCreate() failed in Test_24 with file: %s!\n", outname);
      return;
    }

    ibufsize = sizeof(ibuf);
    while (nbytes_read > 0) {
      SAFileRead(&infile, ibuf, ibufsize, &nbytes_read, &error);
      if (nbytes_read > 0) {

	SAConvertData (&infile.format, &outfile.format, ibuf, nbytes_read, 
		       obuf, &obufsize, &error);
	
	SAFileWrite(&outfile, obuf, obufsize, &nbytes_written, &error);
        if (nbytes_written != obufsize) {
	  printf("=====> Test_24 FAILED <=====\n");    
	  return;
	}
	else if (nbytes_written == 0)
	  break;
      }
    }

    SAFileClose (&infile, &error);
    SAFileClose (&outfile, &error);

    printf("=====> Test_24 PASSED <=====\n");    
}

main(int argc, char **argv)
{
   int testNo = 0;
   int done = 0;

   while (!done) {
/*
 * 
 */
    printf("\n----- SAFile Tests -----\n\n");
    printf("0.  Exit.\n");
    printf("1.  SAFileOpen()\n");
    printf("2.  SAFileRead()\n");
    printf("3.  SAFileSeek()\n");
    printf("4.  SAFileClose()\n");
    printf("5.  Convert Sun 8-bit Ulaw to Sun 8-bit Ulaw\n");
    printf("6.  Convert Sun 16-bit Linear to Sun 16-bit Linear\n");
    printf("7.  Convert Wav 8-bit Linear to Sun 8-bit Ulaw\n");
    printf("8.  Convert Sun 8-bit Ulaw to Wav 8-bit Linear\n");
    printf("9.  Convert Sun 16-bit Linear to Wav 8-bit Linear\n ");
    printf("10. SAConvertFormat()\n");
    printf("11. Convert Sun 8-bit Ulaw to Aiff 8-bit Linear\n ");
    printf("12. Convert Sun 16-bit Linear to Wav 16-bit Linear\n ");
    printf("13. Convert Sun 16-bit Linear to Aiff 16-bit Linear\n ");
    printf("14. Convert Wav 16-bit Linear to Sun 16-bit Linear\n ");
    printf("15. Convert Wav 16-bit Linear to Aiff 16-bit Linear\n ");
    printf("16. Convert Aiff 16-bit Linear to Wav 16-bit Linear\n ");
    printf("17. Convert Aiff 16-bit Linear to Sun 16-bit Linear\n ");
    printf("18. Convert Sun Ulaw 2 channels to Sun Ulaw 1 channel\n ");
    printf("19. Convert Sun 4-bit G.721 ADPCM to Sun 8-bit Ulaw\n ");
    printf("20. Convert Sun 4-bit G.721 ADPCM to Wav 8-bit Linear\n");
    printf("21. Convert Sun 4-bit G.721 ADPCM to Aiff 8-bit Linear\n ");
    printf("22. Convert Sun 8-bit Alaw 2 channels to Sun Ulaw 1 channel\n ");
    printf("23. Convert Sun 8-bit Alaw 2 channels to Wav 8-bit Linear 1 channel\n ");
    printf("24. Convert Sun 8-bit Alaw 2 channels to Aiff 8-bit Linear 1 channel\n ");
    printf("\n");
    printf("Please enter test number: ");
    scanf("%d", &testNo);
    printf("\n");

    switch (testNo) {
      case 1:
        Test_01();
	break;
      case 2:
        Test_02();
	break;
      case 3:
        Test_03();
	break;
      case 4:
        Test_04();
	break;
      case 5:
        Test_05();
	break;
      case 6:
        Test_06();
	break;
      case 7:
        Test_07();
	break;
      case 8:
        Test_08();
	break;
      case 9:
        Test_09();
	break;
      case 10:
        Test_10();
	break;
      case 11:
        Test_11();
	break;
      case 12:
        Test_12();
	break;
      case 13:
        Test_13();
	break;
      case 14:
        Test_14();
	break;
      case 15:
        Test_15();
	break;
      case 16:
        Test_16();
	break;
      case 17:
        Test_17();
	break;
      case 18:
        Test_18();
	break;
      case 19:
        Test_19();
	break;
      case 20:
        Test_20();
	break;
      case 21:
        Test_21();
	break;
      case 22:
        Test_22();
	break;
      case 23:
        Test_23();
	break;
      case 24:
        Test_24();
	break;
      default:
        done = 1;
    }

    }
     
}

