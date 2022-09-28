/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#pragma ident "@(#)sa.c	1.48 97/06/04 SMI"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <math.h>
#include <fcntl.h>
#include <sys/filio.h>
#include <time.h>
#include <sys/stat.h>
#include <thread.h>
#include <signal.h>
#include <signal.h>
#include <memory.h>
#include <poll.h>
#include <stdarg.h>
#include <values.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/param.h>
#include <signal.h>
#include <nl_types.h>
#include <sys/audioio.h>

#include "a_encode.h"
#include "sa.h"
#include "saP.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Unadvertised Solaris syscall interface. */
extern int __nanosleep(const struct timespec *rqtp,  struct  timespec *rmtp);

#ifdef __cplusplus
}
#endif

/* Mathematically, this #define looks like you should get only
 * value, but in actuality, this #define works on ints, and does
 * the parenthetical expression first.  THIS IS NOT A MISTAKE.
 */
#define MULTIPLE(value, divisor)  (value / divisor) * divisor

#define _SAGlobalLock()		  mutex_lock(&_global_lock)
#define _SAGlobalUnlock()	  mutex_unlock(&_global_lock)

#define _SADeviceLock SADeviceLock
#define _SADeviceUnlock SADeviceUnlock

static mutex_t         _global_lock;
extern nl_catd         msgCatalog; 

/* Set and get audio device data */
static SABoolean _SADeviceAudioSetInfo(int my_fd, audio_info_t *info,
                      SAError *errorReturn);
static SABoolean _SADeviceAudioGetInfo(int my_fd, audio_info_t *info,
                      SAError *errorReturn);

#ifdef DEBUG_LOCKS

static const char              *last_stream_locker_file = 0;
static int			last_stream_locker_line = 0;

#define _SAStreamLock(stream)	__SAStreamLock(stream, __FILE__, __LINE__)
#define _SAStreamUnlock(stream)	__SAStreamUnlock(stream)

static int inited = 0;

static void
usr1handler(int i)
{
	if (last_stream_locker_file)
		fprintf(stderr, "%s:%d\n", last_stream_locker_file,
			last_stream_locker_line);
}

static void
__SAStreamLock(SAStream *stream, const char *file, int line)
{
	if (inited == 0) {
		_SAGlobalLock();
		if (inited == 0) {
			inited = 1;
			sigset(SIGUSR1, usr1handler);
		}
		_SAGlobalUnlock();
	}
	SAStreamLock(stream);
	last_stream_locker_file = file;
	last_stream_locker_line = line;
}

static void
__SAStreamUnlock(SAStream *stream)
{
	SAStreamUnlock(stream);
	last_stream_locker_file = NULL;
	last_stream_locker_line = 0;
}

#else

#define _SAStreamLock		SAStreamLock
#define _SAStreamUnlock		SAStreamUnlock

#endif		/* DEBUG_LOCKS */

unsigned
SAFormatFrameSize(SAFormat * format)
{
	return format->channels * format->sample_size;
}

#if defined(_BIG_ENDIAN)
#define DEVICE_FLAGS	(SAFormatFlag_BigEndian | SAFormatFlag_Signed)
#elif defined(_LITTLE_ENDIAN)
#define DEVICE_FLAGS	(SAFormatFlag_Signed)
#else
#error "Don't know the endianness of this machine"
#endif

/* This is what the device will be set to at a 0db setting */
#define NOMINAL_DEVICE_PLAY_GAIN (78)
#define NOMINAL_DEVICE_RECORD_GAIN (175)

static SAFormat amd_format_table[] = {
	{DEVICE_FLAGS, 8000, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 8000, SAEncoding_ULAW, 1, 1, 1}
};
static int      amd_format_table_count = sizeof(amd_format_table) /
					 sizeof(amd_format_table[0]);

static uint_t amd_record_nominal = NOMINAL_DEVICE_RECORD_GAIN;
static uint_t amd_play_nominal = NOMINAL_DEVICE_PLAY_GAIN;
static uint_t amd_play_min = 0;
static uint_t amd_play_max = 255;
static uint_t amd_record_min = 0;
static uint_t amd_record_max = 255;

static SAFormat dbri_format_table[] = {
	{DEVICE_FLAGS, 8000, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 9600, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 11025, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 16000, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 18900, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 22050, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 32000, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 37800, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 44100, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 48000, SAEncoding_ALAW, 1, 1, 1},
	{DEVICE_FLAGS, 8000, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 9600, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 11025, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 16000, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 18900, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 22050, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 32000, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 37800, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 44100, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 48000, SAEncoding_ULAW, 1, 1, 1},
	{DEVICE_FLAGS, 8000, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 9600, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 11025, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 16000, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 18900, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 22050, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 32000, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 37800, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 44100, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 48000, SAEncoding_LINEAR, 2, 1, 1},
	{DEVICE_FLAGS, 8000, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 9600, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 11025, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 16000, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 18900, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 22050, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 32000, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 37800, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 44100, SAEncoding_LINEAR, 2, 2, 1},
	{DEVICE_FLAGS, 48000, SAEncoding_LINEAR, 2, 2, 1}
};
static int dbri_format_table_count = sizeof(dbri_format_table) / 
				sizeof(dbri_format_table[0]);

static uint_t dbri_play_nominal = NOMINAL_DEVICE_PLAY_GAIN;
static uint_t dbri_record_nominal = NOMINAL_DEVICE_RECORD_GAIN;
static uint_t dbri_play_min = 0;
static uint_t dbri_play_max = 255;
static uint_t dbri_record_min = 0;
static uint_t dbri_record_max = 255;
#define SBDEVICE_FLAGS SAFormatFlag_Signed

static SAFormat sbpro_format_table[] = {
	{SBDEVICE_FLAGS, 0, SAEncoding_ULAW, 1, 1, 1},
	{SBDEVICE_FLAGS, 0, SAEncoding_LINEAR, 1, 1, 1},
	{SBDEVICE_FLAGS, 11025, SAEncoding_LINEAR, 1, 2, 1},
	{SBDEVICE_FLAGS, 22050, SAEncoding_LINEAR, 1, 2, 1},
};
static int sbpro_format_table_count = sizeof(sbpro_format_table) /
                                      sizeof(sbpro_format_table[0]);

static SAFormat sb16_format_table[] = {
	{SBDEVICE_FLAGS, 0, SAEncoding_ULAW, 1, 1, 1},
	{SBDEVICE_FLAGS, 0, SAEncoding_LINEAR, 1, 1, 1},
	{SAFormatFlag_Signed, 0, SAEncoding_LINEAR, 2, 1, 1},
	{SBDEVICE_FLAGS, 0, SAEncoding_LINEAR, 1, 2, 1},
	{SAFormatFlag_Signed, 0, SAEncoding_LINEAR, 2, 2, 1},
};
static int sb16_format_table_count = sizeof(sb16_format_table) /
                                      sizeof(sb16_format_table[0]);

SABoolean 
SANextEvent(int fd, SAEvent * event, SAError * error)
{
	int             result;

#ifdef DEBUG
	int             onread;

	ioctl(fd, FIONREAD, &onread);
	DEBUGMSG1(events, "NextEvent:%d pending\n", onread/sizeof(SAEvent));
#endif

	result = read(fd, event, sizeof(SAEvent));
	if (result != sizeof(SAEvent))
		return(_SASetError(error, SAError_CANNOT_READ, NULL));
	else
		return(SATrue);
}

SABoolean 
SAFlushEvents(int fd, SAError * error)
{
	if (ioctl(fd, I_FLUSH, FLUSHRW) < 0)
		return(_SASetError(error, (SAErrorCode) errno, NULL));

	return(SATrue);
}

static void      *_play_function(void * arg);
static void      *_record_function(void * arg);

static float lower_bound = 0.50;
static float upper_bound = 0.60;

static SADevice *_SADefaultDevice = NULL;

static int read_fd = -1;
static int write_fd = -1;

/*
 * Crank up the audio device pipe
 */
static void
init_sound_notifier()
{
	if (write_fd == -1) {

		_SAGlobalLock();
		if (write_fd == -1) {
			int             p[2];
			pipe(p);
			read_fd = p[0];
			write_fd = p[1];
		}
		_SAGlobalUnlock();
	}
}

/*
 * Read events pertaining to the audio device
 */
int
SAGetPipe()
{
	if (read_fd == -1)
		init_sound_notifier();
	return read_fd;
}

#define stringize(name) #name

/*
 * Audio device event handling
 */
const char*
SAEventToString(SAEventType type)
{
	static const char *strings[] = {
		stringize(SAEvent_Error),
		stringize(SAEvent_Overflow),
		stringize(SAEvent_Underflow),
		stringize(SAEvent_LowWater),
		stringize(SAEvent_HighWater),
		stringize(SAEvent_LevelMeter)
	};

	if (type < 0 || type >= (sizeof(strings) / sizeof(strings[0])))
		return (catgets(msgCatalog, 5, 50, "Unknown event"));

	return strings[type];
}

/*
 * Push stream
 */
static void
sendEvent(SAEvent * event)
{
	_SAGlobalLock();
	write(write_fd, event, sizeof(SAEvent));
	_SAGlobalUnlock();
	DEBUGMSG1(events, "queueing %s\n", SAEventToString(event->type));
}

/*
 * Clean stream : Lock list before cleaning
 */
static void
list_insert(SAStream ** list, SAStream * entry)
{
	entry->_next = *list;
	entry->_prev = NULL;
	*list = entry;
}

/* This assumes that the list is locked on entry */
static void
list_delete(SAStream ** list, SAStream * stream)
{
	if (!list && !stream)
		return;

	/* delete from the list */
	if (stream->_prev) {
		stream->_prev->_next = stream->_next;
	}
	if (stream->_next) {
		stream->_next->_prev = stream->_prev;
	}
	if (*list == stream) {
		*list = stream->_next;
	}
	stream->_prev = stream->_next = NULL;
}

/*
 * Insert record stream and start recording
 * thread - Lock target stream during use.
 */
static SAErrorCode
_insert_record_stream(SAStream * stream)
{
	SADevice       *device;

	device = stream->device;

	/* calculate how big of a block to record to meet our timing. */
	if (!device)
		return SAError_NO_DEVICE;

	init_sound_notifier();

	_SADeviceLock(device);
	device->record_format = *SAStreamFormat(stream);
	list_insert(&device->record_streams, stream);

	/* check to see if the record service is running */
	if (device->_thread == 0) {

		int             result;

		device->recordfd = open(device->name, O_RDONLY | O_NONBLOCK);
		if (device->recordfd == -1) {

			/* If we can't record stream, then we need to remove
			 * it, so that we don't tie up the device
			 * unnecessarily.
			 */
			list_delete(&device->record_streams, stream);
			_SADeviceUnlock(device);
			return (SAErrorCode) errno;
		}

		/* configure the audio device for this sound. */
		if (_SADeviceConfigureRecord(device) == -1) {

			/* Clean up and exit as above */
			list_delete(&device->record_streams, stream);
			_SADeviceUnlock(device);
			return SAError_CANNOT_CONFIGURE;
		}

		/* Create record thread. */
		result = thr_create(NULL, 0, _record_function,
				    (void *) device, THR_BOUND | THR_DAEMON,
				    &device->_thread);
		if (result != 0) {

			/* Clean up and exit as above */
			list_delete(&device->record_streams, stream);
			fprintf(stderr, catgets(msgCatalog, 5, 54, "Could not create record stream thread: %s\n"), strerror(errno));
			device->_thread = 0;
			_SADeviceUnlock(device);
			return SAError_CANNOT_RECORD;
		}
	}
	_SADeviceUnlock(device);
	return SASuccess;
}

/*
 * Insert play stream, Create play thread
 * lock target stream during use
 */
static SAErrorCode
_insert_play_stream(SAStream * stream)
{
	SADevice       *device;

	device = stream->device;

	if (!device)
		return SAError_NO_DEVICE;

	/* calculate how big of a block to play to meet our timing. */
	init_sound_notifier();

	_SADeviceLock(device);
	device->play_format = *SAStreamFormat(stream);
	list_insert(&device->play_streams, stream);

	/* check to see if the play service is running */
	if (device->_thread == 0) {

		int             result;

		device->playfd = open(device->name, O_WRONLY | O_NONBLOCK);
		if (device->playfd == -1) {

			/* If we can't play stream, then we need to remove
			 * it, so that we don't tie up the device
			 * unnecessarily. Adding this is part of the fix for
			 * bug 4025886, Sdtaudio frozen if play is pressed
			 * while another process has audio device.
			 */
			list_delete(&device->play_streams, stream);
			_SADeviceUnlock(device);
			return (SAErrorCode) errno;
		}

		device->samples_played = 0;
		device->real_samples_played = 0;
		device->real_timestamp = 0;

		/* configure the audio device for this sound. */
		if (_SADeviceConfigurePlay(device) == -1) {

			/* Part of fix for bug 4025886 */
			list_delete(&device->play_streams, stream);
			_SADeviceUnlock(device);
			return SAError_CANNOT_CONFIGURE;
		}

		/* Create play thread. */
		result = thr_create(NULL, 0, _play_function,
				    (void *) device, THR_BOUND | THR_DAEMON,
				    &device->_thread);
		if (result != 0) {

			fprintf(stderr, catgets(msgCatalog, 5, 55, "Could not create play stream thread: %s\n"), strerror(errno));

			/* Part of fix for bug 4025886 */
			list_delete(&device->play_streams, stream);
			device->_thread = 0;
			_SADeviceUnlock(device);
			return SAError_CANNOT_PLAY;
		}
	}
	_SADeviceUnlock(device);
	return SASuccess;
}

SABoolean
_SADeviceConfigure(SAStream *stream, int state)
{
   SAFormat     myfmt;
   audio_info_t info;
   SADevice     *device;
   SAError      error;

   device = stream->device;
   if (!stream || !device)
        return SAFalse;

   myfmt = *SAStreamFormat(stream);

   _SADeviceUpdateState(stream->device);
   AUDIO_INITINFO(&info);

   if (state == SATrue) {
      info.record.sample_rate = myfmt.sample_rate;
      info.record.channels = myfmt.channels;
      info.record.encoding = myfmt.encoding;
      info.record.precision = 8 * myfmt.sample_size;
   }

   _SADeviceLock(device);
   if (device->recordfd == -1) {
        device->recordfd = open(device->name, O_RDONLY | O_NONBLOCK);
        if (device->recordfd == -1) {

#ifdef DEBUG
        fprintf(stderr, "RETURNING BAD FD\n");
#endif
                _SADeviceUnlock(device);
                return SAFalse;
        }
   }

   if (!_SADeviceAudioSetInfo(device->recordfd, &info, &error)) {

#ifdef DEBUG
        fprintf(stderr, "RETURNING BAD CONFIGURE\n");
        fprintf(stderr, "S.Rate %d : S.Channels %d : S.Precision %d\n",
                myfmt.sample_rate, myfmt.channels, info.record.precision);
#endif

        close(device->recordfd);
        device->recordfd = -1;
        _SADeviceUnlock(device);
        return SAFalse;
   }

   close(device->recordfd);
   device->recordfd = -1;
   _SADeviceUnlock(device);
   return SATrue;
}

/*
 * Set device to handle current record format
 */
int
_SADeviceConfigureRecord(SADevice * device)
{
	SAError	error;

	_SADeviceUpdateState(device);

	if (device->record_format.encoding != 
		device->_audio_info.record.encoding ||
	    device->record_format.sample_rate != 
		device->_audio_info.record.sample_rate ||
	    device->record_format.channels != 
		device->_audio_info.record.channels) {

		audio_info_t    info;
		audio_info_t    cinfo;

		AUDIO_INITINFO(&info);
		info.record.sample_rate = device->record_format.sample_rate;
		info.record.channels = device->record_format.channels;
		info.record.encoding = device->record_format.encoding;
		info.record.precision = 8 * device->record_format.sample_size;

		if (!_SADeviceAudioSetInfo(device->recordfd, &info, &error)) {
			return -1;
        	}

		if (!_SADeviceAudioGetInfo(device->recordfd, &cinfo, &error)) {
			return -1;
		}

		if (info.record.sample_rate != cinfo.record.sample_rate ||
		    info.record.channels != cinfo.record.channels ||
		    info.record.encoding != cinfo.record.encoding ||
		    info.record.precision != cinfo.record.precision) {

			/* Send an error, since we can't use the specified
			 * recording audio device parameters.
			 */
			fprintf(stderr, catgets(msgCatalog, 5, 56, "Could not initialize audio device for recording: %s\n"), strerror(errno));
			return -1;
		}

		device->_audio_info = cinfo;
	}
	return 0;
}

/*
 * Set device to handle current play format
 */
int
_SADeviceConfigurePlay(SADevice * device)
{
	SAError error;

	_SADeviceUpdateState(device);

	if (device->play_format.encoding != 
		device->_audio_info.play.encoding ||
	    device->play_format.sample_rate != 
		device->_audio_info.play.sample_rate ||
	    device->play_format.channels != 
		device->_audio_info.play.channels) {

		audio_info_t    info;
		audio_info_t    cinfo;

		AUDIO_INITINFO(&info);
		info.play.sample_rate = device->play_format.sample_rate;
		info.play.channels = device->play_format.channels;
		info.play.encoding = device->play_format.encoding;
		info.play.precision = 8 * device->play_format.sample_size;

		if (!_SADeviceAudioSetInfo(device->playfd, &info, &error)) {
			return -1;
		}

		if (!_SADeviceAudioGetInfo(device->playfd, &cinfo, &error)) {
			return -1;
		}

		if (info.play.sample_rate != cinfo.play.sample_rate ||
		    info.play.channels != cinfo.play.channels ||
		    info.play.encoding != cinfo.play.encoding ||
		    info.play.precision != cinfo.play.precision) {

			/* Send an error, since we can't use the specified
			 * recording audio device parameters.
			 */
			fprintf(stderr, catgets(msgCatalog, 5, 58, "Could not initialize audio device for playing: %s\n"), strerror(errno));
			return -1;
		}

		device->_audio_info = cinfo;
	}
	return 0;
}

SABoolean
_SADevicePlayChunk(SADevice * device, double time)
{
	SAStream       *stream = NULL;
	SAError         error;
	int             frames = 0;
	int		result = SAFalse;

	_SADeviceLock(device);
	stream = device->play_streams;
	_SADeviceUnlock(device);

	if (stream == NULL) {
		return (result);
	}

	_SAStreamLock(stream);
	_SADeviceLock(device);
	if (device->play_streams != stream) {
		_SADeviceUnlock(device);
		_SAStreamUnlock(stream);
		return (result);
	}

	frames = time * device->play_format.sample_rate;
	stream->meter_gain = 0;
	error.code = SASuccess;
	if (SABufferWriteOut(&stream->buffer, &frames, device->playfd,
			     &stream->meter_gain, &error) == SATrue) {

		result = SATrue;
		stream->device->frames_queued += frames;
		if (stream->buffer.data_length == 0) {

			if ((stream->flags & SAStream_Underflow) == 0) {
				SAEvent         event;

				event.type = SAEvent_Underflow;
				event.stream = stream;
				sendEvent(&event);
				stream->flags |= SAStream_Underflow;
			}
		}

		if (stream->buffer.data_length <= stream->buffer.water_line) {

			/* only notify once if it hasn't been cleared */
			if ((stream->flags & SAStream_LowWater) == 0) {
				SAEvent         event;
				event.type = SAEvent_LowWater;
				event.stream = stream;
				sendEvent(&event);
				stream->flags |= SAStream_LowWater;
			}
		}
	} 
	else
	 	result = SAFalse;
	
	_SADeviceUnlock(device);
	_SAStreamUnlock(stream);
	return result;
}

static int 
timeval_to_ms(struct timeval * tv)
{
	return (tv->tv_sec - 84024000) * 1000 + tv->tv_usec / 1000;
}

SABoolean
_SADeviceUpdateState(SADevice * device)
{
	struct timeval  tv;
	int             ms_timestamp;
	SAError		error;

	gettimeofday(&tv, NULL);
	if (!_SADeviceAudioGetInfo(device->ctl_fd, &device->_audio_info, &error)) {
		return SAFalse;
	}
	ms_timestamp = timeval_to_ms(&tv);

	if (device->_audio_info.play.error || device->_audio_info.record.error) {
		audio_info_t    info;

		DEBUGMSG(timing, "hardware underflow\n");
		AUDIO_INITINFO(&info);
		info.play.error = 0;
		info.record.error = 0;

		if (!_SADeviceAudioSetInfo(device->ctl_fd, &info, &error)) {
			return SAFalse;
		}
	}

	if ((device->real_timestamp != 0) &&
		(device->_audio_info.play.samples == device->real_samples_played)) {

		/* The device still hasn't updated so let's calculate a new
		 * value
		 */
		int       difference = ms_timestamp - device->real_timestamp;
		int       samples_played  = difference *
				device->_audio_info.play.sample_rate + 
				device->real_samples_played;

		if (samples_played > device->frames_queued)
			device->samples_played = device->frames_queued;
		else
			device->samples_played = samples_played;

		DEBUGMSG2(timing, "Adjusting samples_played from %d to %d\n",
			  device->real_samples_played,
			  device->samples_played);
	} else {

		device->real_timestamp = ms_timestamp;
		device->real_samples_played = device->samples_played =
			device->_audio_info.play.samples;
		DEBUGMSG1(timing, "Setting samples_played to %d\n",
			  device->real_samples_played);
	}

	return (SATrue);
}

float
_SADeviceCalculateDelay(SADevice * device)
{
	int	samples = 0;
	float	buffered = 0.0;

	_SADeviceLock(device);
	_SADeviceUpdateState(device);
	_SADeviceUnlock(device);

	_SADeviceLock(device);

	samples = (device->frames_queued - device->samples_played);
	buffered = samples / (float) device->play_format.sample_rate;

	_SADeviceUnlock(device);
	return buffered;
}

static void    *
_play_function(void *arg)
{
	struct timespec delay = {0, 0};
	SADevice       *device = (SADevice *) arg;

restart:

	_SADeviceLock(device);
	_SADeviceUpdateState(device);
	_SADeviceUnlock(device);

	while (1) {
		if (delay.tv_sec != 0 || delay.tv_nsec != 0) {
			DEBUGMSG2(timing, 
				  "sleeping for %d sec %d nanoseconds\n",
				  delay.tv_sec, delay.tv_nsec);
			__nanosleep(&delay, NULL);

			DEBUGMSG(timing, "woke up\n");
		}

		while (1) {
			float  buffered = _SADeviceCalculateDelay(device);

			DEBUGMSG3(timing, 
				  "%f seconds buffered %d queued %d processed\n",
				  buffered, device->frames_queued, 
				  device->samples_played);

			if (buffered < lower_bound) {
#ifdef DEBUG
				DEBUGMSG1(timing, "Play Chunk : %d\n",
						upper_bound - buffered);
#endif
				if (_SADevicePlayChunk(device, upper_bound - buffered) == SAFalse)
					goto done;
			} else {

				/* calculate the delay in millisecond */
				buffered = _SADeviceCalculateDelay(device);
				buffered -= lower_bound;
				if (buffered < 0) {
					delay.tv_sec = delay.tv_nsec = 0;

				} else {
					delay.tv_sec = (int) buffered;

					delay.tv_nsec = (buffered - delay.tv_sec) * 1000000000;
				}
				break;
			}
		}
	}

done:
	_SADeviceLock(device);
	if (device->play_streams == NULL) {

		/* there's nothing to play so go ahead and exit. */
		device->_thread = 0;
		device->frames_queued = 0;
		close(device->playfd);
		device->playfd = -1;

		/* Reset these values, since we're done playing! */
		device->samples_played = 0;
		device->real_samples_played = 0;
		device->real_timestamp = 0;
		_SADeviceUnlock(device);

	} else {

		/* something else slipped into the list so we should go play
		 * it.
		 */
		_SADeviceUnlock(device);
		goto restart;
	}

	return 0;
}

static int
record_block(SADevice * device)
{
	SAStream	*stream = NULL;
	SAError		error;
	int		result = SAFalse;

	_SADeviceLock(device);
	stream = device->record_streams;
	_SADeviceUnlock(device);

	if (stream == NULL)
		return (result);

	_SAStreamLock(stream);
	_SADeviceLock(device);
	_SADeviceUpdateState(device);

	stream->meter_gain = 0;
	error.code = SASuccess;
	if (SABufferReadIn(&stream->buffer, 0, device->recordfd,
			   &stream->meter_gain, &error) == SAFalse) {

		/* Prabhat : Should handle a buffer full event here,
		 *           else will risk data corruption
	 	 */
		SAEvent event;

		event.stream = stream;

		if ((error.code == SAError_CANNOT_RESERVE) ||
			(error.code == SAError_BUFFER_FULL)) {

		   if ((stream->buffer.data_length >= stream->buffer.buffer_length)
			&& ((stream->flags & SAStream_Overflow) == 0)) {
			event.type = SAEvent_Overflow;
			sendEvent(&event);
			stream->flags |= SAStream_Overflow;

		   } else if ((stream->flags & SAStream_HighWater) == 0) {
			event.type = SAEvent_HighWater;
			sendEvent(&event);
			stream->flags |= SAStream_HighWater;
		   }

		} else {

			_SADeviceUnlock(device);
			_SAStreamUnlock(stream);
			return (result = SAFalse);
		}

	} else {
	  SAEvent event;

	  event.stream = stream;
	  if (error.code == SAError_CANNOT_RESERVE) {

	     if (stream->buffer.data_length == 
		stream->buffer.buffer_length) {

		if (((stream->flags & SAStream_Overflow) == 0)) {

			event.type = SAEvent_Overflow;
			sendEvent(&event);
			stream->flags |= SAStream_Overflow;
		}

		else if ((stream->flags & SAEvent_HighWater) == 0) {
			event.type = SAEvent_HighWater;
			sendEvent(&event);
			stream->flags |= SAStream_HighWater;
		}
	     }
	  }

	  if (stream->buffer.data_length >= stream->buffer.water_line) {

		/* only notify once if it hasn't been cleared */
		if ((stream->flags & SAStream_HighWater) == 0) {

			event.type = SAEvent_HighWater;
			sendEvent(&event);
			stream->flags |= SAStream_HighWater;
		}
	   }
	}

	_SADeviceUnlock(device);
	_SAStreamUnlock(stream);
	return (result = SATrue);
}

static void *
_record_function(void *arg)
{
	struct pollfd   fds;
	SADevice       *device = (SADevice *) arg;
	int             done;

restart:
	done = SAFalse;
	while (!done) {

		_SADeviceLock(device);
		fds.fd = device->recordfd;
		_SADeviceUnlock(device);
		fds.events = POLLIN;
		fds.revents = NULL;

		if (poll(&fds, 1, 250) < 0) {
		}

		if (record_block(device) == SAFalse)
			done = SATrue;
	}

	_SADeviceLock(device);
	if (device->record_streams == NULL) {

		/* there's nothing to record so go ahead and exit. */
		close(device->recordfd);
		device->recordfd = -1;
		device->_thread = 0;
		_SADeviceUnlock(device);

	} else {

		/* something else slipped into the list so we should go
		 * record it.
		 */
		_SADeviceUnlock(device);
		goto restart;
	}

	return 0;
}

/***********************************************************************
 *
 * Error related functions
 *
 ***********************************************************************/
SABoolean
_SASetError(SAError * error, SAErrorCode code,
	    const char *msg)
{
	error->code = code;
	error->description = SAErrorString(code);
	error->extra_description = msg;
	DEBUGMSG3(errors, "error code %d \"%s\" (%s)\n", error->code, 
		  error->description,
		  error->extra_description ? error->extra_description : "");
	return SAFalse;
}

const char     *
SAErrorString(SAErrorCode code)
{
	char           *error_string;

	if (code == 0)
		return (catgets(msgCatalog, 5, 48, "No error."));

	if (code >= SAError_BASE_ERROR && code <= SAError_LastError) {

		switch (code - SAError_BASE_ERROR) {

		case 1:
			return (catgets(msgCatalog, 5, 1, "Not a sound."));

		case 2:
			return (catgets(msgCatalog, 5, 2, "Bad data format."));

		case 3:
			return (catgets(msgCatalog, 5, 3, 
					"Bad sampling rate."));

		case 4:
			return (catgets(msgCatalog, 5, 4, 
					"Bad channel count."));

		case 5:
			return (catgets(msgCatalog, 5, 5, "Bad size."));

		case 6:
			return (catgets(msgCatalog, 5, 6, "Bad file name."));

		case 7:
			return (catgets(msgCatalog, 5, 7, 
					"Cannot open file."));

		case 8:
			return (catgets(msgCatalog, 5, 8, 
					"Cannot write file."));

		case 9:
			return (catgets(msgCatalog, 5, 9, 
					"Cannot read file."));

		case 10:
			return (catgets(msgCatalog, 5, 10, 
					"Cannot allocate memory."));

		case 11:
			return (catgets(msgCatalog, 5, 11, 
					"Cannot free memory."));

		case 12:
			return (catgets(msgCatalog, 5, 12, 
					"Cannot copy."));

		case 13:
			return (catgets(msgCatalog, 5, 13, 
					"Cannot reserve access."));

		case 14:
			return (catgets(msgCatalog, 5, 14, 
					"Access not reserved."));

		case 15:
			return (catgets(msgCatalog, 5, 15, 
					"Cannot record sound."));

		case 16:
			return (catgets(msgCatalog, 5, 16, 
					"Already recording sound."));

		case 17:
			return (catgets(msgCatalog, 5, 17, 
					"Not recording sound."));

		case 18:
			return (catgets(msgCatalog, 5, 18, 
					"Cannot play sound."));

		case 19:
			return (catgets(msgCatalog, 5, 19, 
					"Already playing sound."));

		case 20:
			return (catgets(msgCatalog, 5, 20, 
					"Not playing sound."));

		case 21:
			return (catgets(msgCatalog, 5, 20, "Not pausing sound."));

		case 22:
			return (catgets(msgCatalog, 5, 22, 
					"Not implemented."));

		case 23:
			return (catgets(msgCatalog, 5, 23, 
					"Cannot find sound."));

		case 24:
			return (catgets(msgCatalog, 5, 24, 
					"Cannot edit sound."));


		case 25:
			return (catgets(msgCatalog, 5, 25, 
					"Bad memory space in dsp load image."));

		case 26:
			return (catgets(msgCatalog, 5, 26, 
					"Mach kernel error."));

		case 27:
			return (catgets(msgCatalog, 5, 27, 
					"Bad configuration."));

		case 28:
			return (catgets(msgCatalog, 5, 28, 
					"Cannot configure."));

		case 29:
			return (catgets(msgCatalog, 5, 29, "Data underrun."));


		case 30:
			return (catgets(msgCatalog, 5, 30, "Aborted."));

		case 31:
			return (catgets(msgCatalog, 5, 31, "Bad tag."));

		case 32:
			return (catgets(msgCatalog, 5, 32, 
					"Cannot access hardware resources."));

		case 33:
			return (catgets(msgCatalog, 5, 33, "Timeout."));

		case 34:
			return (catgets(msgCatalog, 5, 34, 
					"Hardware resources already in use."));

		case 35:
			return (catgets(msgCatalog, 5, 35, 
					"Cannot abort operation."));

		case 36:
			return (catgets(msgCatalog, 5, 36, 
					"Cannot recognize file type from filename."));

		case 37:
			return (catgets(msgCatalog, 5, 37, 
					"Cannot decode audio file header."));

		case 38:
			return (catgets(msgCatalog, 5, 38, 
					"Cannot move file pointer."));

		case 39:
			return (catgets(msgCatalog, 5, 39, 
					"Cannot close file."));

		case 40:
			return (catgets(msgCatalog, 5, 40, 
					"Information string too large."));

		case 41:
			return (catgets(msgCatalog, 5, 41, 
					"Ring buffer is full."));

		case 42:
			return (catgets(msgCatalog, 5, 42, 
					"Ring buffer is empty."));

		case 43:
			return (catgets(msgCatalog, 5, 43, 
					"Can't find audio device."));

		case 44:
			return (catgets(msgCatalog, 5, 44, 
					"Unknown audio device."));

		case 45:
			return (catgets(msgCatalog, 5, 45, 
					"Not a multiple of the frame size."));

		case 46:
			return (catgets(msgCatalog, 5, 56, 
					"Not supported."));

		case 47:
			return (catgets(msgCatalog, 5, 47, 
					"One or more arguments was invalid."));
		}
	}
	error_string = strerror(code);
	if (error_string)
		return error_string;

	return (catgets(msgCatalog, 5, 49, "Unknown error code."));
}

/***********************************************************************
 *
 * Stream related functions
 *
 ***********************************************************************/
SAFormat *
SAStreamFormat(SAStream * stream)
{
	return &stream->buffer.format;
}

void
SAStreamLock(SAStream * stream)
{
	int             result;

	result = mutex_lock(&stream->_lock);
	assert(result == 0);
}

void
SAStreamUnlock(SAStream * stream)
{
	int             result;

	result = mutex_unlock(&stream->_lock);
	assert(result == 0);
}

SAStream *
SAStreamOpen(SADevice * device, SAError * error)
{
	SAStream       *stream = (SAStream *) calloc(sizeof(SAStream), 1);

	if (device)
		stream->device = device;
	else {
		stream->device = SADefaultDevice(error);
		if (stream->device == NULL) {
			free(stream);
			return NULL;
		}
	}

	return stream;
}

SABoolean 
SAStreamClose(SAStream * stream, SAError * error)
{
	SAStreamStop(stream, error);

	_SAStreamLock(stream);
	SABufferFree(&stream->buffer);
	_SAStreamUnlock(stream);

	free(stream);
	return SATrue;
}

SABoolean 
SAStreamSetFormat(SAStream * stream, SAFormat * format,
		  SAFormat * alternate, SAError * error)
{
	SAFormat        fmt = *format;

	/* make it easy to tell if we have to do conversion */
	if (alternate)
		alternate->encoding = SAEncoding_NONE;

	if (SADeviceFindFormat(stream->device, &fmt,
			       alternate, error) == SAFalse)
		return _SASetError(error, SAError_NOT_SUPPORTED,
				   catgets(msgCatalog, 5, 60, "hardware does not support or understand specified format"));

	_SAStreamLock(stream);
	SABufferFree(&stream->buffer);
	if (alternate && alternate->encoding != SAEncoding_NONE)
		SABufferInit(&stream->buffer, alternate, upper_bound);
	else
		SABufferInit(&stream->buffer, &fmt, upper_bound);
	_SAStreamUnlock(stream);

	return SATrue;
}

SABoolean 
SAStreamRecord(SAStream * stream, SAError * error)
{
	SAErrorCode     code;

	_SAStreamLock(stream);
	if (stream->flags & SAStream_Playing) {
		_SAStreamUnlock(stream);
		return _SASetError(error, SAError_ALREADY_PLAYING, NULL);
	}

	if (stream->flags & SAStream_Recording) {
		_SAStreamUnlock(stream);
		return SATrue;
	}

	code = _insert_record_stream(stream);
	if (code == SASuccess)
		stream->flags |= SAStream_Recording;

	_SAStreamUnlock(stream);

	if (code == SASuccess)
		return SATrue;
	return _SASetError(error, code, NULL);
}

SABoolean 
SAStreamPlay(SAStream * stream, SAError * error)
{
	SAErrorCode     code;

	_SAStreamLock(stream);
	if (stream->flags & SAStream_Recording) {
		_SAStreamUnlock(stream);
		return _SASetError(error, SAError_ALREADY_RECORDING, NULL);
	}

	if (stream->flags & SAStream_Playing) {
		_SAStreamUnlock(stream);
		return SATrue;
	}

	code = _insert_play_stream(stream);
	if (code == SASuccess)
		stream->flags |= SAStream_Playing;

	_SAStreamUnlock(stream);

	if (code == SASuccess)
		return SATrue;
	return _SASetError(error, code, NULL);
}

SABoolean
SAStreamStop(SAStream * stream, SAError * error)
{
	SABoolean       result = SAFalse;

	_SAStreamLock(stream);
	if (stream->flags & SAStream_Recording) {

		stream->flags &= ~SAStream_Recording;
		_SADeviceLock(stream->device);
		list_delete(&stream->device->record_streams, stream);
		_SADeviceUnlock(stream->device);
		result = SATrue;

	} else if (stream->flags & SAStream_Playing) {

		stream->flags &= ~SAStream_Playing;
		_SADeviceLock(stream->device);
		list_delete(&stream->device->play_streams, stream);
		_SADeviceUnlock(stream->device);
		result = SATrue;

	} else {
		result = SATrue;
	}

	_SAStreamUnlock(stream);
	return result;
}

SABoolean 
SAStreamDrain(SAStream * stream, SAError * error)
{
	SABoolean       done = SAFalse;
	int             count = 0;
	struct pollfd   fds;
	int             underflowed = 0;

	fds.fd = SAGetPipe();
	fds.events = POLLIN;
	fds.revents = NULL;

	/* if we haven't underlowed then wait till we have */
	_SAStreamLock(stream);
	underflowed = (stream->flags & SAStream_Underflow);
	_SAStreamUnlock(stream);

	if (!underflowed)

		while (done == SAFalse) {

			if (poll(&fds, 1, -1) > 0) {

				SAEvent         event;

				if (SANextEvent(fds.fd, &event, error) == SAFalse) {
					return SAFalse;
				}
				count++;

				if (event.type == SAEvent_Underflow &&
				    event.stream == stream)
					done = SATrue;
			}
	}

	_SADeviceLock(stream->device);
	ioctl(stream->device->playfd, AUDIO_DRAIN, NULL);
	_SADeviceUnlock(stream->device);
	return SATrue;
}

SABoolean 
SAStreamRead(SAStream * stream, char *data,
	     int bytes, int *bytes_read, SAError * error)
{
	SABoolean       result = SAFalse;
	int             frames;
	int             frame_size = SAFormatFrameSize(SAStreamFormat(stream));

	_SAStreamLock(stream);
	frames = bytes / frame_size;

	/* Prabhat :: Copy stuff from sdtaudio's stream into data */
	result = SABufferCopyOut(&stream->buffer, &frames, data, NULL, error);

	/* clear the flags since we've written data */
	if ((result == SATrue) && 
	    (stream->flags & (SAStream_Overflow | SAStream_HighWater)))
	   stream->flags &= ~(SAStream_Overflow | SAStream_HighWater);

	*bytes_read = frames * frame_size;
	_SAStreamUnlock(stream);

	return result;
}

SABoolean 
SAStreamWrite(SAStream * stream, char *data,
	      int bytes, int *bytes_written, SAError * error)
{
	int		size = 0, max_size = 0, data_end = 0;
	SABuffer       *buffer = NULL;
	SABoolean	result = SATrue;

	*bytes_written = 0;
	_SAStreamLock(stream);

	/* make sure that the amount is a multiple of the frame_size */
	bytes = MULTIPLE(bytes, SAFormatFrameSize(SAStreamFormat(stream)));
	if (bytes == 0) {
		_SAStreamUnlock(stream);
		return _SASetError(error, SAError_NOT_FRAME, NULL);
	}

	buffer = &stream->buffer;
	data_end = buffer->data_start + buffer->data_length;

	/* the buffer is full */
	if ((buffer->data_length == buffer->buffer_length) ||
		(data_end >= buffer->buffer_length)) {
		_SAStreamUnlock(stream);
		return _SASetError(error, SAError_BUFFER_FULL, NULL);
	}

	size = bytes;
	max_size = buffer->buffer_length - data_end;

	if (size > max_size)
		size = max_size;

	/* add the data onto the end of the current data */
	memcpy(buffer->buffer + data_end, data, size);
	DEBUGMSG4(buffers, "moving %d to %d (%d bytes) into buffer 0x%x (1)\n",
		  data_end, data_end + size, size, buffer);

	/* udpate counts and pointers */
	*bytes_written += size;
	buffer->data_length += size;

	if ((size > 0) && (size < bytes)) {

		result = SATrue;
		(void) _SASetError(error, SAError_BUFFER_FULL, NULL);
	}

	/* clear the flags since we've written data */
	if ((result == SATrue) &&
		(stream->flags & (SAStream_Underflow | SAStream_LowWater)))
	   stream->flags &= ~(SAStream_Underflow | SAStream_LowWater);

	DEBUGMSG2(buffers, "SAStreamWrite : Target : %d Wrote : %d\n",
				bytes, size);

	_SAStreamUnlock(stream);
	return result;
}

/*******************************************************
 * Audio(7) specific functionality.
*
*******************************************************/
/* XXX The "right thing" here is to calibrate this
   function per device type. (Oh stop laughing).
   With 0 indicating a reference dialogue level.
   Such as a human voice at about 70 dbA 3 feet
   from the speaker.
   
   An alternative, for the adventurous, is to
   use the well known A weighted scale for sound
   and fully calibrate the individual integer gain
   values to a dbA value. Some reference noise values
   are:
       Threadshold of Hearing      0  dbA
       Concert Hall Empty          30 dbA
       Apartment in City           42 dbA
       Average Office              55 dbA
       Factory Office              65 dbA
       Ordinary Converatio (3ft)   68 dbA
       Riveter (35 ft)             97 dbA
       Hammer blows on
           steel plate (2 ft)     114 dbA
       Threshold of pain          130 dbA

   Some reference Sound Powers at a distance
   of 3 meters:

       Clarinet                    86 dbA
       Piano                       95 dbA
       Bass Drum                  113 dbA


   At least I finally got that off my chest.
          
*/
/* What this actually does is:
   Assume the device integer represents a linear
   amplitude scale and the nominal value is what we'd like 0
   to appear as. Then compute use the following:
   
       Vdb = 20 log (A/Aref)

*/       
static double
_SALinearToDbVolume(uint_t gain, uint_t nominal)
{
	return 20.0 * log10(((double) gain) / ((double) nominal));
}

static double
_SALinearToDbGain(double gain, double nominal)
{
	return 20.0 * log10(gain / nominal);
}

/* And Amplitude = Aref pow(10, Vdb/20) */
static          uint_t
_SADbToLinearVolume(double gain, uint_t nominal)
{
	return (uint_t) nominal *pow(10, gain / 20.);
}

static SABoolean
_SADeviceAudioSetInfo(int my_fd, audio_info_t * info,
		      SAError * errorReturn)
{
	int	ret_ioctl;

	ret_ioctl = ioctl(my_fd, AUDIO_SETINFO, info);
	if ( ret_ioctl < 0) {
		_SASetError(errorReturn, SAError_CANNOT_CONFIGURE, NULL);
        	DEBUGMSG1(device, "IOCTL returned : %s\n", strerror(errno));
		return SAFalse;
	}
	return SATrue;
}

/* Update the hardware with our state */
static SABoolean
_SAUpdateHardware(SADevice * device, SAError * errorReturn)
{
	audio_info_t    info;
	double          balance = 0.0;
	SABoolean	result = SAFalse;

	AUDIO_INITINFO(&info);
	info.play.gain = _SADbToLinearVolume(device->play_volume,
					     device->play_nominal);
	if ((int) info.play.gain < AUDIO_MIN_GAIN)
		info.play.gain = AUDIO_MIN_GAIN;
	else if ((int) info.play.gain > AUDIO_MAX_GAIN)
		info.play.gain = AUDIO_MAX_GAIN;

	/* Put it between 0 and 2.0, then map it to the dev region */
	balance = ((device->play_balance + 1.0) / 2.0) * 64.0;
	info.play.balance = balance;

	info.record.gain = _SADbToLinearVolume(device->record_volume,
					       device->record_nominal);
	if ((int) info.record.gain < AUDIO_MIN_GAIN)
		info.record.gain = AUDIO_MIN_GAIN;
	else if ((int) info.record.gain > AUDIO_MAX_GAIN)
		info.record.gain = AUDIO_MAX_GAIN;

	if (SADeviceRecordBalanceEditable(device, errorReturn)) {
		balance = ((device->record_balance + 1.0) / 2.0) * 64.0;
		info.record.balance = balance;
	}
	info.output_muted = device->output_muted;

	/* Output ports */
	info.play.port = 0;
	if (device->play_ports & SADevice_Speaker)
		info.play.port |= AUDIO_SPEAKER;
	else
		info.play.port &= ~AUDIO_SPEAKER;

	if (device->play_ports & SADevice_Headphone)
		info.play.port |= AUDIO_HEADPHONE;
	else
		info.play.port &= ~AUDIO_HEADPHONE;

	if (device->play_ports & SADevice_LineOut)
		info.play.port |= AUDIO_LINE_OUT;
	else
		info.play.port &= ~AUDIO_LINE_OUT;

	/* Input ports */
	info.record.port = 0;
	if (device->record_ports & SADevice_Microphone)
		info.record.port |= AUDIO_MICROPHONE;
	else
		info.record.port &= ~AUDIO_MICROPHONE;

	if (device->record_ports & SADevice_LineIn)
		info.record.port |= AUDIO_LINE_IN;
	else
		info.record.port &= ~AUDIO_LINE_IN;

	if (device->record_ports & SADevice_CD)
		info.record.port |= AUDIO_CD;
	else
		info.record.port &= ~AUDIO_CD;

	DEBUGMSG2(device, "Setting play volume: %g db (%d to dev)\n",
		  device->play_volume, info.play.gain);
	DEBUGMSG2(device, "Setting play balance: %g (%d to dev)\n",
		  device->play_balance, info.play.balance);
	DEBUGMSG(device, "\n");
	DEBUGMSG2(device, "Setting record volume: %g db (%d to dev)\n",
		  device->record_volume, info.record.gain);
	DEBUGMSG2(device, "Setting record balance: %g (%d to dev)\n",
		  device->record_balance, info.record.balance);

	_SASetError(errorReturn, SASuccess, NULL);
	result = _SADeviceAudioSetInfo(device->ctl_fd, &info, errorReturn);
	return (result);
}

static SABoolean
_SADeviceAudioGetInfo(int my_fd, audio_info_t * info,
		      SAError * errorReturn)
{
	int	ret_ioctl;

	ret_ioctl = ioctl(my_fd, AUDIO_GETINFO, info);
	if ( ret_ioctl < 0) {
		_SASetError(errorReturn, SAError_CANNOT_CONFIGURE, NULL);
		DEBUGMSG1(device, "GETINFO Failed - %s\n", strerror(errno));
		return SAFalse;
	}
	return SATrue;
}

static SABoolean
_SAUpdateDevice(SADevice * device, SAError * errorReturn)
{
	audio_info_t    info;

	if (!_SADeviceAudioGetInfo(device->ctl_fd, &info, errorReturn)) {
		return SAFalse;
	}

	device->record_volume = _SALinearToDbVolume(info.record.gain,
						    device->record_nominal);
	device->record_balance = (((double) info.record.balance) / 32.0) - 1.0;

	device->play_volume = _SALinearToDbVolume(info.play.gain,
						  device->play_nominal);
	device->play_balance = (((double) info.play.balance) / 32.0) - 1.0;

	device->output_muted = info.output_muted;

	device->play_avail_ports = 0;
	if (info.play.avail_ports & AUDIO_SPEAKER)
		device->play_avail_ports |= SADevice_Speaker;
	if (info.play.avail_ports & AUDIO_HEADPHONE)
		device->play_avail_ports |= SADevice_Headphone;
	if (info.play.avail_ports & AUDIO_LINE_OUT)
		device->play_avail_ports |= SADevice_LineOut;

	device->play_ports = 0;
	if (info.play.port & AUDIO_SPEAKER)
		device->play_ports |= SADevice_Speaker;
	if (info.play.port & AUDIO_HEADPHONE)
		device->play_ports |= SADevice_Headphone;
	if (info.play.port & AUDIO_LINE_OUT)
		device->play_ports |= SADevice_LineOut;

	device->record_avail_ports = 0;
	if (info.record.avail_ports & AUDIO_MICROPHONE)
		device->record_avail_ports |= SADevice_Microphone;
	if (info.record.avail_ports & AUDIO_LINE_IN)
		device->record_avail_ports |= SADevice_LineIn;
	if (info.record.avail_ports & AUDIO_CD)
		device->record_avail_ports |= SADevice_CD;

	device->record_ports = 0;
	if (info.record.port & AUDIO_MICROPHONE)
		device->record_ports |= SADevice_Microphone;
	if (info.record.port & AUDIO_LINE_IN)
		device->record_ports |= SADevice_LineIn;
	if (info.record.port & AUDIO_CD)
		device->record_ports |= SADevice_CD;

	return SATrue;
}

static SABoolean
_SASigOn(SADevice * device, SAError * errorReturn)
{
	int	ret_ioctl;

	ret_ioctl = ioctl(device->ctl_fd, I_SETSIG, S_MSG);
	if ( ret_ioctl < 0) {
		_SASetError(errorReturn, SAError_CANNOT_CONFIGURE, NULL);
		return SAFalse;
	}
	return SATrue;
}

/***********************************************************************
 *
 * Device related functions
 *
 ***********************************************************************/
void 
SADeviceLock(SADevice * device)
{
	int             result;

	DEBUGMSG2(locking, "thread %d locking device 0x%x\n", thr_self(), device);
	result = mutex_lock(&device->_lock);
}

void 
SADeviceUnlock(SADevice * device)
{
	int             result;

	DEBUGMSG2(locking, "thread %d unlocking device 0x%x\n", thr_self(), device);
	result = mutex_unlock(&device->_lock);
}

static SABoolean
_SAInitDevices(SAError * error)
{
	SADevice       *device;
	audio_device_t  dev;
	int	        ret_ioctl;

	_SAGlobalLock();
	device = _SADefaultDevice;
	_SAGlobalUnlock();

	if (device != NULL) {
		return SATrue;
	}

	/* Open up the device */
	device = calloc(sizeof(SADevice), 1);
	device->ctl_fd = open("/dev/audioctl", O_RDWR);
	if (device->ctl_fd == -1)
		return _SASetError(error, SAError_CANNOT_OPEN, NULL);

	ret_ioctl = ioctl(device->ctl_fd, AUDIO_GETDEV, &dev);
	if ( ret_ioctl < 0) {
		close(device->ctl_fd);
		device->ctl_fd = -1;
		return _SASetError(error, SAError_CANNOT_OPEN, NULL);
	}

	/* figure out what kind of audio device we have installed */
	_SADeviceLock(device);
	device->name = strdup("/dev/audio");
	device->hardware_name = strdup(dev.name);

	if (strcmp(dev.name, "SUNW,am79c30") == 0) {

		device->format_table = amd_format_table;
		device->format_count = amd_format_table_count;
		device->record_nominal = amd_record_nominal;
		device->record_min = amd_record_min;
		device->record_max = amd_record_max;
		device->play_nominal = amd_play_nominal;
		device->play_min = amd_play_min;
		device->play_max = amd_play_max;

	} else if (strcmp(dev.name, "SUNW,dbri") == 0) {

		device->format_table = dbri_format_table;
		device->format_count = dbri_format_table_count;
		device->record_nominal = dbri_record_nominal;
		device->record_min = dbri_play_min;
		device->record_max = dbri_record_max;
		device->play_nominal = dbri_play_nominal;
		device->play_min = dbri_play_min;
		device->play_max = dbri_play_max;

	} else if (strcmp(dev.name, "SUNW,CS4231") == 0) {

		device->format_table = dbri_format_table;
		device->format_count = dbri_format_table_count;
		device->record_nominal = dbri_record_nominal;
		device->record_min = dbri_play_min;
		device->record_max = dbri_record_max;
		device->play_nominal = dbri_play_nominal;
		device->play_min = dbri_play_min;
		device->play_max = dbri_play_max;
	} else if (strcmp(dev.name, "SUNW,sbpro") == 0) {

		device->format_table = sbpro_format_table;
		device->format_count = sbpro_format_table_count;
		device->record_nominal = dbri_record_nominal;
		device->record_min = dbri_play_min;
		device->record_max = dbri_record_max;
		device->play_nominal = dbri_play_nominal;
		device->play_min = dbri_play_min;
		device->play_max = dbri_play_max;
		device->low_rate = 4000;
		device->high_rate = 44100;
	} else if (strcmp(dev.name, "SUNW,sb16") == 0) {

		device->format_table = sb16_format_table;
		device->format_count = sb16_format_table_count;
		device->record_nominal = dbri_record_nominal;
		device->record_min = dbri_play_min;
		device->record_max = dbri_record_max;
		device->play_nominal = dbri_play_nominal;
		device->play_min = dbri_play_min;
		device->play_max = dbri_play_max;
		device->low_rate = 5000;
		device->high_rate = 44100;
	} else {

		_SADeviceUnlock(device);
		free(device);
		return _SASetError(error, SAError_UNKNOWN_DEVICE, NULL);
	}

	device->playfd = -1;
	device->recordfd = -1;
	_SADeviceUnlock(device);

	_SAGlobalLock();
	_SADefaultDevice = device;
	_SAGlobalUnlock();

	return SATrue;
}

SADevice *
SADefaultDevice(SAError * error)
{
	SADevice       *result;

	if (_SAInitDevices(error) == SAFalse) {
		return NULL;
	}

	_SAGlobalLock();
	result = _SADefaultDevice;
	_SAGlobalUnlock();

	if (result == NULL) {
		_SASetError(error, SAError_NO_DEVICE, NULL);
	}
	return result;
}

SABoolean
SADeviceSigPollUpdateOn(SADevice * device, SAError * errorReturn)
{
	return _SASigOn(device, errorReturn);
}

SABoolean 
SADeviceGetRecordVolume(SADevice * device,
			double *volume,
			SAError * errorReturn)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 61, "Could not retrieve record volume.");
		return SAFalse;
	}
	*volume = device->record_volume;
	return SATrue;
}

SABoolean 
SADeviceSetRecordVolume(SADevice * device,
			double volume,
			SAError * errorReturn)
{
	device->record_volume = volume;
	if (!_SAUpdateHardware(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 62, "Could not set record volume.");
		return SAFalse;
	}
	return SATrue;
}

/* Support for an arbitrary linear gain scale */
SABoolean 
SADeviceSetLinearRecordVolume(SADevice * device,
			      double gain, double low, double hi,
			      SAError * errorReturn)
{
	/* normalize to between 0.0 and 1.0 then scale to the device. */
	double          Ngain = ((gain - low + 1) / (hi - low)) *
				(device->record_max - device->record_min);

	/* We're keeping volume in DB, and the nominal value is specified by
	 * the device.
	 */
	device->record_volume = _SALinearToDbGain(Ngain, 
						  device->record_nominal);
	if (!_SAUpdateHardware(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 62, "Could not set record volume.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceGetLinearRecordVolume(SADevice * device,
			      double *volume, double low, double hi,
			      SAError * errorReturn)
{
	int             devVolume;

	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 61, "Could not retrieve record volume.");
		return SAFalse;
	}

	/* Get the linear volume in device values */
	devVolume = _SADbToLinearVolume(device->record_volume,
					device->record_nominal);

	/* normalizen between 0 and 1 */
	*volume = ((double) (devVolume - device->record_min + 1)) /
		((double) (device->record_max - device->record_min));

	/* Scale to between low and hi */
	*volume = (*volume * (hi - low)) + low;
	return SATrue;
}

SABoolean 
SADeviceRecordBalanceEditable(SADevice * device,
			      SAError * error)
{
	if (strcmp(device->hardware_name, "SUNW,sbpro") == 0)
		return SAFalse;
	else if (strcmp(device->hardware_name, "SUNW,sb16") == 0)
		return SATrue;
	else
		return SATrue;
}

SABoolean 
SADeviceSetRecordBalance(SADevice * device, double balance,
			 SAError * error)
{

	device->record_balance = balance;
	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 63, "Could not set recording balance.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceGetMute(SADevice * device,
		SABoolean * outputMuted,
		SAError * error)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 64, "Could not retrieve mute state of audio device.");
		return SAFalse;
	}
	*outputMuted = device->output_muted;
	return SATrue;
}

SABoolean 
SADeviceSetMuteOn(SADevice * device,
		  SAError * error)
{
	device->output_muted = 1;
	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 65, "Could not set mute state of audio device.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetMuteOff(SADevice * device,
		   SAError * error)
{
	device->output_muted = 0;
	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 65, "Could not set mute state of audio device.");
		return SAFalse;
	}
	return SATrue;
}

/* Determines if Output Ports are exclusive choices only (SATrue)
 * or if only one can be selected at a time.
 */
SABoolean 
SADeviceOutputExclusive(SADevice * device,
			SAError * error)
{
	if (strcmp(device->hardware_name, "SUNW,am79c30") == 0)
		return SATrue;
	else if (strcmp(device->hardware_name, "SUNW,dbri") == 0)
		return SAFalse;
	else if (strcmp(device->hardware_name, "SUNW,CS4231") == 0)
		return SAFalse;
	else
		return SAFalse;
}

SABoolean 
SADeviceGetAvailOutputPorts(SADevice * device,
			    SAOutputFlags * ports,
			    SAError * error)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 66, "Could not retrieve available output devices.");
		return SAFalse;
	}
	*ports = device->play_avail_ports;
	return SATrue;
}

SABoolean 
SADeviceGetOutputPorts(SADevice * device,
		       SAOutputFlags * ports,
		       SAError * error)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 67, "Could not retrieve current output devices.");
		return SAFalse;
	}
	*ports = device->play_ports;
	return SATrue;
}

SABoolean 
SADeviceSetOutputPorts(SADevice * device,
		       SAOutputFlags ports,
		       SAError * error)
{
	device->play_ports = ports;
	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 68, "Could not set output devices.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetSpeakerPort(SADevice * device,
		       SABoolean set,
		       SAError * error)
{
	if (!SADeviceGetOutputPorts(device, &device->play_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 69, "Could not set speaker port.");
		return SAFalse;
	}

	if (set == SATrue) {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = SADevice_Speaker;
		else
			device->play_ports |= SADevice_Speaker;

	} else {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = ~SADevice_Speaker;
		else
			device->play_ports &= ~SADevice_Speaker;
	}

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 69, "Could not set speaker port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetHeadphonePort(SADevice * device,
			 SABoolean set,
			 SAError * error)
{
	if (!SADeviceGetOutputPorts(device, &device->play_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 70, "Could not set headphone port.");
		return SAFalse;
	}

	if (set == SATrue) {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = SADevice_Headphone;
		else
			device->play_ports |= SADevice_Headphone;

	} else {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = ~SADevice_Headphone;
		else
			device->play_ports &= ~SADevice_Headphone;
	}

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 70, "Could not set headphone port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetLineOutPort(SADevice * device,
		       SABoolean set,
		       SAError * error)
{
	if (!SADeviceGetOutputPorts(device, &device->play_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 71, "Could not set line out port.");
		return SAFalse;
	}

	if (set == SATrue) {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = SADevice_LineOut;
		else
			device->play_ports |= SADevice_LineOut;

	} else {

		if (SADeviceOutputExclusive(device, error))
			device->play_ports = ~SADevice_LineOut;
		else
			device->play_ports &= ~SADevice_LineOut;
	}

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 71, "Could not set line out port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceGetAvailInputPorts(SADevice * device,
			   SAInputFlags * ports,
			   SAError * error)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 72, "Could not retrieve available input devices.");
		return SAFalse;
	}
	*ports = device->record_avail_ports;
	return SATrue;
}

SABoolean 
SADeviceGetInputPorts(SADevice * device,
		      SAInputFlags * ports,
		      SAError * error)
{
	/* Smack the old device. */
	if (!_SAUpdateDevice(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 73, "Could not retrieve current input devices.");
		return SAFalse;
	}
	*ports = device->record_ports;
	return SATrue;
}

SABoolean 
SADeviceSetMicrophonePort(SADevice * device,
			  SABoolean set,
			  SAError * error)
{
	if (!SADeviceGetInputPorts(device, &device->record_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 74, "Could not set microphone port.");
		return SAFalse;
	}

	if (set == SATrue)
		device->record_ports = SADevice_Microphone;
	else
		device->record_ports &= ~SADevice_Microphone;

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 74, "Could not set microphone port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetLineInPort(SADevice * device,
		      SABoolean set,
		      SAError * error)
{
	if (!SADeviceGetInputPorts(device, &device->record_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 75, "Could not set line in port.");
		return SAFalse;
	}
	if (set == SATrue)
		device->record_ports = SADevice_LineIn;
	else
		device->record_ports &= ~SADevice_LineIn;

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 75, "Could not set line in port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetCDPort(SADevice * device,
		  SABoolean set,
		  SAError * error)
{
	if (!SADeviceGetInputPorts(device, &device->record_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 76, "Could not set CD port.");
		return SAFalse;
	}
	if (set == SATrue)
		device->record_ports = SADevice_CD;
	else
		device->record_ports &= ~SADevice_CD;

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 76, "Could not set CD port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceSetInternalCDPort(SADevice * device,
			  SABoolean set,
			  SAError * error)
{
	if (!SADeviceGetInputPorts(device, &device->record_ports, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 76, "Could not set CD port.");
		return SAFalse;
	}

	if (set == SATrue)
		device->record_ports = SADevice_InternalCD;
	else
		device->record_ports &= ~SADevice_InternalCD;

	if (!_SAUpdateHardware(device, error)) {
		error->extra_description = 
			catgets(msgCatalog, 5, 76, "Could not set CD port.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceGetRecordBalance(SADevice * device, double *bal,
			 SAError * errorReturn)
{
	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 77, "Could not retrieve recording balance.");
		return SAFalse;
	}

	*bal = device->record_balance;
	return SATrue;
}

SABoolean 
SADeviceGetPlayVolume(SADevice * device,
		      double *volume,
		      SAError * errorReturn)
{
	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 78, "Could not retrieve playback volume.");
		return SAFalse;
	}
	*volume = device->play_volume;
	return SATrue;
}

SABoolean 
SADeviceGetLinearPlayVolume(SADevice * device,
			    double *volume, double low, double hi,
			    SAError * errorReturn)
{
	int             devVolume;

	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 78, "Could not retrieve playback volume.");
		return SAFalse;
	}

	/* Get the linear volume in device values */
	devVolume = _SADbToLinearVolume(device->play_volume,
					device->play_nominal);

	/* normalize between 0 and 1 */
	*volume = ((double) (devVolume - device->play_min + 1)) /
		  ((double) (device->play_max - device->play_min));

	/* Scale to between low and hi */
	*volume = (*volume * (hi - low)) + low;

	return SATrue;
}

SABoolean 
SADeviceSetPlayVolume(SADevice * device,
		      double volume,
		      SAError * errorReturn)
{
	device->play_volume = volume;
	if (!_SAUpdateHardware(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 79, "Could not set playback volume.");
		return SAFalse;
	}
	return SATrue;
}

/* Support for an arbitrary linear gain scale */
SABoolean 
SADeviceSetLinearPlayVolume(SADevice * device,
			    double gain, double low, double hi,
			    SAError * errorReturn)
{
	/* normalize to between 0.0 and 1.0 then scale to the device. */
	double          Ngain = ((gain - low + 1) / (hi - low)) *
				(device->play_max - device->play_min);

	/* We're keeping volume in DB, and the nominal value is specified by
	 * the device.
	 */
	device->play_volume = _SALinearToDbGain(Ngain, device->play_nominal);
	if (!_SAUpdateHardware(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 79, "Could not set playback volume.");
		return SAFalse;
	}

	/*
	 * DEBUGMSG3(device, "Set linear play gain: %g (%gN) (%g db)", gain,
	 * Ngain, device->play_volume);
	 */
	return SATrue;
}

SABoolean 
SADeviceGetPlayBalance(SADevice * device, double *bal,
		       SAError * errorReturn)
{
	if (!_SAUpdateDevice(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 80, "Could not retrieve playback balance.");
		return SAFalse;
	}
	*bal = device->play_balance;
	return SATrue;
}

SABoolean 
SADeviceSetPlayBalance(SADevice * device, double balance,
		       SAError * errorReturn)
{

	device->play_balance = balance;
	if (!_SAUpdateHardware(device, errorReturn)) {
		errorReturn->extra_description = 
			catgets(msgCatalog, 5, 81, "Could not set playback balance.");
		return SAFalse;
	}
	return SATrue;
}

SABoolean 
SADeviceFindFormat(SADevice * device,
		   SAFormat * format, SAFormat * alternate,
		   SAError * error)
{
	SABoolean       result = SAFalse;
	int             index = 0;
	int             format_count = 0;
	SAFormat       *table;
	double          sample_rate_error = MAXDOUBLE;
	int             closest = -1;
	const double    max_rate_error = 0.08;	/* this is the acceptable %
						 * error */
	_SADeviceLock(device);
	table = device->format_table;
	format_count = device->format_count;

	index = 0;
	while (result == SAFalse && index < format_count) {

		if (table[index].channels == format->channels &&
		    format->flags == table[index].flags &&
		    format->encoding == table[index].encoding &&
		    format->sample_size == table[index].sample_size) {

			if (table[index].sample_rate == 0 &&
			    format->sample_rate <= device->high_rate &&
			    format->sample_rate >= device->low_rate)
				result = SATrue;
			else if (format->sample_rate == 
					table[index].sample_rate)
				result = SATrue;
			else {
				/* we matched everything except the sample
				 * rate, so let's see if it's at least close
				 * to what we want
				 */
				double   error = format->sample_rate /
					(double) table[index].sample_rate;

				if (error < 1)
					error = 1 - error;
				else
					error = error - 1;

				if (error < sample_rate_error) {
					sample_rate_error = error;
					closest = index;
				}
			}
		}
		index++;
	}

	if (result == SAFalse) {
		if ((closest != -1) && (sample_rate_error <= max_rate_error)) {
			result = SATrue;
			*format = table[closest];
		} else
			_SASetError(error, SAError_NOT_SUPPORTED, NULL);
	}
	_SADeviceUnlock(device);

	if (result == NULL && alternate) {
		SAFormat   dev_format = *format;

#if defined(_BIG_ENDIAN)
		dev_format.flags = SAFormatFlag_BigEndian;
#elif defined(_LITTLE_ENDIAN)
		dev_format.flags = 0;
#else
#error "Don't know the endianness of this machine"
#endif

		dev_format.flags |= SAFormatFlag_Signed;

		/* they are willing to allow conversions, so let's see what
		 * we can do
		 */

		/* if 8-bit linear, try 8-bit ulaw, 1 channel */
		if (format->encoding == SAEncoding_LINEAR &&
		    format->sample_size == 1) {

			dev_format.encoding = SAEncoding_ULAW;
			dev_format.channels = 1;
			result = SADeviceFindFormat(device, &dev_format,
						    NULL, error);
		}

		/* if 8-bit ulaw, alaw with 2 channels, try 2 channels */
		else if (format->encoding == SAEncoding_ULAW &&
			 format->sample_size == 1 && format->channels == 2) {

			dev_format.channels = 1;
			result = SADeviceFindFormat(device, &dev_format,
						    NULL, error);
		}

		/* if 4-bit G.721 ADPCM, try 8-bit ulaw */
		else if (format->encoding == SAEncoding_G721 &&
		format->sample_size == 1 && format->samples_per_unit == 2) {

			dev_format.encoding = SAEncoding_ULAW;
			dev_format.channels = 1;
			dev_format.sample_size = 1;
			result = SADeviceFindFormat(device, &dev_format, 
						    NULL, error);
		}

		/* if 8-bit alaw with 2 channels, try 2 channels */
		else if (format->encoding == SAEncoding_ALAW &&
			 format->sample_size == 1 && format->channels == 2) {

			dev_format.channels = 1;
			result = SADeviceFindFormat(device, &dev_format, 
						    NULL, error);
		} else
			result = SADeviceFindFormat(device, &dev_format, 
						    NULL, error);

		if (result == SATrue) {
			*alternate = dev_format;
		}
	}
	return result;
}

/***********************************************************************
 *
 * Utiltity functions
 *
 ***********************************************************************/
const double	LoSigInstantRange	= .008;
const double	HiSigInstantRange	= .48;
const int       DCfreq                  = 500;
const double	PeakSig		        = .9803765;

SABoolean
SAGainCalculate(SAFormat * format, char *buffer, int size, double *gain,
		SAError * error)
{
	double          sum = 0;
	int             i;
	int             frames = 0;
	double          dcweight;
	double          timeconstant;
	SABoolean       last_peak = SAFalse;
	double          DCaverage = 0;	/* XXX this should probably have a
					 * longer lifetime */
	double          val;
	int             clipcnt = 0;

	*gain = 0;

	/* Time constant corresponds to the number of samples for 500Hz */
	timeconstant = 1. / (format->sample_rate / (double) DCfreq);
	dcweight = 1. - timeconstant;

	frames = size / format->sample_size;

	switch (format->encoding) {

	case SAEncoding_ULAW:
		if (format->sample_size != 1)
			return _SASetError(error, SAError_BAD_FORMAT, NULL);
		break;

	case SAEncoding_LINEAR:
		if ((format->sample_size != 1) &&
		    (format->sample_size != 2) &&
		    (format->sample_size != 4))
			return _SASetError(error, SAError_BAD_FORMAT, NULL);
		break;

	default:
		return _SASetError(error, SAError_BAD_FORMAT, NULL);
	}

	for (i = 0; i < frames; i++) {

		switch (format->encoding) {

		case SAEncoding_ULAW:
			val = audio_u2l(buffer[i]) / (double) MAXINT;
			break;

		case SAEncoding_LINEAR:
			if (format->sample_size == 2) {

				short   *ptr = (short *) buffer;
				val = (ptr[i] << 16) / (double) MAXINT;;

			} else if (format->sample_size == 4) {

				int     *ptr = (int *) buffer;
				val = ptr[i] / (double) MAXINT;

			} else if (format->sample_size == 1) {

				signed char  *ptr = (signed char *) buffer;
				val = (ptr[i] << 24) / (double) MAXINT;

			} else {
				assert(0);
			}
			break;

		default:
			assert(0);
		}

		/* Two max values in a row constitutes clipping */
		if ((val >= PeakSig) || (val <= -PeakSig)) {

			if (last_peak) {
				clipcnt++;
			} else {
				last_peak = SATrue;
			}
		} else {
			last_peak = SAFalse;
		}

		/* Add in this value to weighted average */
		DCaverage = (DCaverage * dcweight) + (val * timeconstant);
		val -= DCaverage;
		if (val > 1.)
			val = 1.;
		else if (val < -1.)
			val = -1.;
		sum += fabs(val);
	}

	sum /= frames;
	val = log10(1. + (9. * sum));

	val -= LoSigInstantRange;
	if (val > HiSigInstantRange) {
		val = 1.;
	} else if (val < 0.) {
		val = 0.;
	} else {
		val /= HiSigInstantRange;
	}
	*gain = val;
	return SATrue;
}

/***********************************************************************
 *
 * SABuffer
 *
 ***********************************************************************/
void
SABufferInit(SABuffer * buffer, SAFormat * fmt, double time)
{
	int             buffer_size = time * fmt->sample_rate;

	buffer_size *= SAFormatFrameSize(fmt);

	memset(buffer, 0, sizeof(SABuffer));
	buffer->format = *fmt;
	buffer->buffer_length = buffer_size * 2;
	buffer->buffer = (char *) malloc(buffer->buffer_length);
	buffer->water_line = buffer_size / 2;
}

void
SABufferFree(SABuffer * buffer)
{
	if (buffer->buffer) {
		free(buffer->buffer);
		memset(buffer, 0, sizeof(SABuffer));
	}
}

/* Function     : SABufferWriteOut()
 * Objective    : Write data fron sdtaudio's buffer into audio device
 * Comments     : Leave memory handling to list_delete or SAStreamClose
 */
SABoolean
SABufferWriteOut(SABuffer * buffer, int *frames, int fd,
		 double *gain, SAError * error)
{
	int	frame_size = SAFormatFrameSize(&buffer->format);
	int	max_bytes = 0;

	if (frames == NULL)
		max_bytes = 0;
	else {
		max_bytes = *frames * frame_size;

		/* we may not write anything so make sure we don't return
		 * saying that we have
		 */
		*frames = 0;
	}

	if (buffer->data_length == 0)
	   return SAFalse;

	if (buffer->data_length > 0) {
		int	to_write = buffer->buffer_length - 
				   buffer->data_start;
		int	written = 0;

		if (to_write > buffer->data_length)
			to_write = buffer->data_length;

		if ((max_bytes > 0) && (to_write > max_bytes))
			to_write = max_bytes;
		else
			to_write = MULTIPLE(to_write, frame_size);

		/* write the data. */
		written = write(fd, buffer->buffer + buffer->data_start, 
				to_write);
		if (written < 0) {
			DEBUGMSG1(errors, "SABufferWriteOut Error :%d\n", errno);
			return _SASetError(error, (SAErrorCode) errno, NULL);
		}
		to_write = written;

		buffer->frames_transferred += to_write / frame_size;
		if (frames)
			*frames = to_write / frame_size;

		if (gain)
			SAGainCalculate(&buffer->format,
					buffer->buffer + buffer->data_start, 
					to_write, gain, error);

		DEBUGMSG3(buffers, "moving %d bytes (%d frames) to fd %d\n",
			  to_write, to_write / frame_size, fd);

		buffer->data_start += to_write;
		buffer->data_length -= to_write;

		/* udpate the start pointer */
		if ((buffer->data_start >= buffer->buffer_length) ||
			(buffer->data_length == 0))
			buffer->data_start = 0;
	}
	return SATrue;
}

/* Function     : SABufferCopyOut()
 * Objective    : Copy data from sdtaudio's buffer to a temporary buffer
 * Comments     : Caller should handle memory allocated to 'data'
 */
SABoolean
SABufferCopyOut(SABuffer * buffer, int *frames, char *data,
		double *gain, SAError * error)
{
	int	frame_size = SAFormatFrameSize(&buffer->format);
	int	max_bytes = 0;

	if (frames == NULL) {
		max_bytes = 0;
		return _SASetError(error, SAError_BAD_ARGUMENT, NULL);
	}

	if (buffer->data_length == 0)
           return _SASetError(error, SAError_BUFFER_EMPTY, NULL);

	/* we may not write anything so make sure we don't
	 * return saying that we have
	 */
	max_bytes = *frames * frame_size;
	*frames = 0;

	if (buffer->data_length > 0) {
		int	to_write = buffer->buffer_length - 
					buffer->data_start;

		if (to_write > buffer->data_length)
			to_write = buffer->data_length;

		if ((max_bytes > 0) && (to_write > max_bytes))
			to_write = max_bytes;
		else
			to_write = MULTIPLE(to_write, frame_size);

		/* copy the data. */
		memcpy(data, buffer->buffer + buffer->data_start, to_write);

		/* update the count of how much was written */
		*frames = to_write / frame_size;

		if (gain)
			SAGainCalculate(&buffer->format,
					buffer->buffer + buffer->data_start, 
					to_write, gain, error);

		DEBUGMSG3(buffers, 
			  "moving %d bytes (%d frames) from buffer 0x%x\n",
			  to_write, to_write / frame_size, buffer);

		buffer->data_start += to_write;
		buffer->data_length -= to_write;

		/* Prabhat ::	Udpate the start pointer  so that there
				are less buffer overflows */
		if ((buffer->data_start >= buffer->buffer_length) ||
			(buffer->data_length == 0))
		   buffer->data_start = 0;
	}
	return SATrue;
}

/* Function     : SABufferReadIn()
 * Objective    : Read data from record fd into sdtaudio's stream buffer
 * Comments     : Leave memory handling to list_delete or SAStreamClose
 */
SABoolean
SABufferReadIn(SABuffer * buffer, int *frames, int fd,
	       double *gain, SAError * error)
{
	int		frame_size = SAFormatFrameSize(&buffer->format);
	int		max_bytes = 0;

	/* Prabhat :: As our buffer is not implemented for a circular
         *            operation, calling program should send an over-
         *            flow immediately on reception of this error.
         */
        if (buffer->data_length == buffer->buffer_length) {
            return(_SASetError(error, SAError_BUFFER_FULL, NULL));
        }
        else
          if ((buffer->data_start + buffer->data_length) >=
                buffer->buffer_length) {
                return (_SASetError(error, SAError_CANNOT_RESERVE, NULL));
        }

#ifdef DEBUG
        DEBUGMSG3(buffers, "Buffer %d | Start - %d | Length - %d\n",
         buffer->buffer_length, buffer->data_start, buffer->data_length);
#endif

	if (buffer->data_length < buffer->buffer_length) {
		int	nread = 0;
		int	data_end = buffer->data_start + 
				   buffer->data_length;
		int	to_read = 0;

		max_bytes = buffer->buffer_length - buffer->data_length;

		if (buffer->data_start != 0)
			max_bytes -= buffer->data_start;

		max_bytes = MULTIPLE(max_bytes, frame_size);

		if (data_end >= buffer->buffer_length) {
			data_end -= buffer->buffer_length;
			to_read = buffer->data_start - data_end;
		} else
			to_read = buffer->buffer_length - data_end;

		if (max_bytes > 0 && to_read > max_bytes)
			to_read = max_bytes;
		else
			to_read = MULTIPLE(to_read, frame_size);

		/* read the data. */
		nread = read(fd, buffer->buffer + data_end, to_read);
		if (nread < 0) {
			DEBUGMSG1(buffers, "Stream Read Error - %d\n", errno);
			return _SASetError(error, (SAErrorCode) errno, NULL);
		}
		to_read = nread;

		buffer->frames_transferred += (to_read / frame_size);
		if (frames)
			*frames = to_read / frame_size;

		if (gain)
			SAGainCalculate(&buffer->format,
					buffer->buffer + buffer->data_start, 
					to_read, gain, error);

		DEBUGMSG4(buffers,
			  "moving %d to %d from fd %d to buffer 0x%x \n",
			  data_end, data_end + to_read, fd, buffer);

		buffer->data_length += to_read;
	}
	return SATrue;
}

/***********************************************************************
 *
 * Debug logging support
 *
 ***********************************************************************/
static FILE    *logfile;
_SADebugStruct  _SADebugFlags;

static void 
initLogging()
{
	const char     *file = getenv("SALOGFILE");
	const char     *categ = getenv("SALOG");

	memset(&_SADebugFlags, 0, sizeof(_SADebugFlags));

	if (categ) {

		char           *lasts;
		char           *c = strdup(categ);
		char           *token;

		categ = c;
		while (token = strtok_r(c, ": ,:", &lasts)) {

			if (strcasecmp(token, "all") == 0)
				memset(&_SADebugFlags, 0xff, 
				       sizeof(_SADebugFlags));
			else if (strcasecmp(token, "locking") == 0)
				_SADebugFlags.locking = 1;
			else if (strcasecmp(token, "saplay") == 0)
				_SADebugFlags.saplay = 1;
			else if (strcasecmp(token, "errors") == 0)
				_SADebugFlags.errors = 1;
			else if (strcasecmp(token, "timing") == 0)
				_SADebugFlags.timing = 1;
			else if (strcasecmp(token, "buffers") == 0)
				_SADebugFlags.buffers = 1;
			else if (strcasecmp(token, "device") == 0)
				_SADebugFlags.device = 1;
			else if (strcasecmp(token, "events") == 0)
				_SADebugFlags.events = 1;
			else if (strcasecmp(token, "help") == 0) {
				fprintf(stderr, "Categories are:\n");
				fprintf(stderr, "events\ntiming\nbuffers\nlocking\nall\nerrors\n\n");
				fprintf(stderr,
					"Pass a list of space or comma separated categories to enable logging.\nThe names are case-insensitive.\n");
			} else
				fprintf(stderr, "Unknown log category \"%s\"\n",
					token);
			c = NULL;
		}
		free((void *) categ);
	}
	if (file) {
		logfile = fopen(file, "w");
	}
	if (logfile == NULL)
		logfile = stderr;
}

#pragma init(initLogging)

void 
_SALog(const char *type, const char *format,...)
{
	va_list         args;
	char            buffer[480];
	char            asc[27];

	va_start(args, format);

#ifdef USE_HRTIME
	{
		static int      first = 1;
		static hrtime_t last;
		hrtime_t        time = gethrtime();

		if (first) {
			last = time;
			first = 0;
		}
		sprintf(asc, "%9lld ", time - last);
		last = time;
	}
#else
	{
		struct timeval  tp;
		(void) gettimeofday(&tp, NULL);
		strcpy(asc, asctime(localtime(&tp.tv_sec)));
		asc[19] = 0;
	}
#endif

	sprintf(buffer, "%s [%s] ", asc, type);
	strcat(buffer, format);
	vfprintf(logfile, buffer, args);
}
