#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <string.h>
#include <assert.h>
#include "sa.h"
#include "saP.h"
#include "safile.h"


void usage(char * name)
{
    fprintf(stderr, "Usage: %s file\n", name);
    exit(1);
}


main(int argc, char * argv[])
{
    int buffer_size = 1024;
    int record_size;
    char * buffer;
    if (argc == 1)
	usage(argv[0]);

    argv++;
    if (argv[0] && argv[0][0] == '-') {
	buffer_size = atoi(argv[0] + 1);
	if (buffer_size == 0)
	    buffer_size = 1024;
	argv++;
    }
    buffer = (char *)malloc(buffer_size);
    while (argv[0] != NULL) {
	SAFile safile;
	SAError error;
	SAFormat alternate;
	SAStream * stream;
	const char * pathname = argv++[0];
	int total_bytes = 0;

	int buffer_start = 0;
	int bytes_left = 0;
	int bytes_written = 0;
	SABoolean done;
	int frame_size;
	char * outbuf = buffer;
	int outbufsize;
	char leftover_bytes[16];
	int leftover_count = 0;
	SABoolean start_playing = SAFalse;
	SABoolean playing = SAFalse;
	SABoolean still_overflowed = SAFalse;

	printf("Recording: %s\n", pathname);

	safile.filetype = SASunType;
	safile.format.flags = SAFormatFlag_BigEndian;
	safile.format.sample_rate = 44100;
	safile.format.encoding = SAEncoding_LINEAR;
	safile.format.sample_size = 2;
	safile.format.channels = 2;
	safile.format.samples_per_unit = 1;

	if (SAFileCreate(pathname, &safile, &error) == SAFalse) {
	    printf("SAFileOpen() failed with file %s: %s\n", pathname,
		   error.description);
	    continue;
	}

	stream = SAStreamOpen(NULL, &error);
	if (stream == NULL) {
	    fprintf(stderr, "SAStreamOpen: %s\n", error.description);
	    exit(1);
	}
	
	if (SAStreamSetFormat(stream, &safile.format,
			      &alternate, &error) == SAFalse) {
	    fprintf(stderr, "Unable to set format on audio stream: %s\n",
		    error.description);
	    SAFileClose(&safile, &error);
	    SAStreamClose(stream, &error);
	    continue;
	}

	frame_size = SAFormatFrameSize(&safile.format);
	if (alternate.encoding != SAEncoding_NONE) {
	    /* we need to allocate a buffer to do the conversion into */
	    outbuf = (char *) malloc(buffer_size * frame_size /
				     SAFormatFrameSize(&alternate));
	} else
	    outbuf = buffer;

	record_size = 2.0 * frame_size * safile.format.sample_rate;

	done = SAFalse;

	SAStreamRecord(stream, &error);
	while (done == SAFalse) {
	    struct pollfd fds;
	    fds.fd = SAGetPipe();
	    fds.events = POLLIN;
	    fds.revents = NULL;
	    
	    if (still_overflowed == SAFalse)
		while (1)
		    if (poll(&fds, 1, -1) > 0) {
			SAEvent event;
			if (SANextEvent(fds.fd, &event, &error) == SAFalse) {
			    fprintf(stderr, "SAError: %s\n",
				    error.description);
			    continue;
			}
			switch (event.type) {
			case SAEvent_Overflow:
			case SAEvent_HighWater:
			    break;
			default:
			    fprintf(stderr, "Unknown SAEvent: %s\n", SAEventToString(event.type));
			    continue;
			}
			/* fall out and refill the buffer */
			break;
		    }
	    
	    if (bytes_left == 0) {
		int bytes;
	    
		if (leftover_count) {
		    DEBUGMSG(saplay, "restoring leftover bytes\n");
		    memcpy(buffer, leftover_bytes, leftover_count);
		    bytes_left = leftover_count;
		    leftover_count = 0;
		}

		buffer_start = 0;

		DEBUGMSG(saplay, "refilling read buffer\n");
		if (SAStreamRead(stream, buffer + bytes_left, 
				 buffer_size - bytes_left, &bytes, &error) == SAFalse) {
		    fprintf(stderr, "SAFileRead: %s\n", error.description);
		    done = SATrue;
		} else {
		    SAStreamLock(stream);
		    if (stream->buffer.data_length > stream->buffer.water_line)
			still_overflowed = SATrue;
		    else
			still_overflowed = SAFalse;
		    SAStreamUnlock(stream);

		    bytes_left += bytes;
		    if (bytes_left != 0) {
			leftover_count = bytes_left % frame_size;
			if (leftover_count != 0) {
			    /* we got a non multiple of the framesize back  so
			       hang onto them until we can do something with them */
			    memcpy(buffer + bytes_left - leftover_count,
				   leftover_bytes, leftover_count);
			    bytes_left -= leftover_count;
			}
		    }

		}

		if (alternate.encoding != SAEncoding_NONE) {
		    SAConvertData(&safile.format, &alternate,
				  buffer, bytes_left,
				  outbuf, &outbufsize, &error);
		    bytes_left = outbufsize;
		}
	    }
	    
	    DEBUGMSG1(saplay, "writing %d to file\n", bytes_left);
	    if (SAFileWrite(&safile, outbuf + buffer_start,
			    bytes_left, &bytes_written,
			    &error) == SAFalse) {
		if (error.code == SAError_BUFFER_FULL) {
		    start_playing = SATrue;
		    continue;
		} else
		    done = SATrue;
	    }
	    bytes_left -= bytes_written;
	    buffer_start += bytes_written;
	    total_bytes += bytes_written;

	    if (total_bytes > record_size) {
		SAFileClose(&safile, &error);
		SAStreamClose(stream, &error);
		done = SATrue;
		continue;
	    }
	}
    }
}
