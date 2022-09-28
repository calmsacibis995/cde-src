/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */

#ifndef SA_H
#define SA_H

#pragma ident "@(#)sa.h	1.30 97/05/30 SMI"

#include <thread.h>
#include <sys/audioio.h>

/*
 * Sun Audio Types
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    SASuccess = 0,
    SAError_BASE_ERROR = 1000,
    SAError_NOT_SOUND = SAError_BASE_ERROR,
    SAError_BAD_FORMAT,
    SAError_BAD_RATE,
    SAError_BAD_CHANNEL,
    SAError_BAD_SIZE,
    SAError_BAD_FILENAME,
    SAError_CANNOT_OPEN,
    SAError_CANNOT_WRITE,
    SAError_CANNOT_READ,
    SAError_CANNOT_ALLOC,
    SAError_CANNOT_FREE,
    SAError_CANNOT_COPY,
    SAError_CANNOT_RESERVE,
    SAError_NOT_RESERVED,
    SAError_CANNOT_RECORD,
    SAError_ALREADY_RECORDING,
    SAError_NOT_RECORDING,
    SAError_CANNOT_PLAY,
    SAError_ALREADY_PLAYING,
    SAError_NOT_PLAYING,
    SAError_NOT_PAUSED,
    SAError_NOT_IMPLEMENTED,
    SAError_CANNOT_FIND,
    SAError_CANNOT_EDIT,
    SAError_BAD_SPACE,
    SAError_KERNEL,
    SAError_BAD_CONFIGURATION,
    SAError_CANNOT_CONFIGURE,
    SAError_UNDERRUN,
    SAError_ABORTED,
    SAError_BAD_TAG,
    SAError_CANNOT_ACCESS,
    SAError_TIMEOUT,
    SAError_BUSY,
    SAError_CANNOT_ABORT,
    SAError_NO_FILE_TYPE,
    SAError_CANNOT_DECODE_FILE,
    SAError_CANNOT_SEEK,
    SAError_CANNOT_CLOSE,
    SAError_INFO_TOO_BIG,
    SAError_BUFFER_FULL,
    SAError_BUFFER_EMPTY,
    SAError_NO_DEVICE,
    SAError_UNKNOWN_DEVICE,
    SAError_NOT_FRAME,
    SAError_NOT_SUPPORTED,
    SAError_BAD_ARGUMENT,
    SAError_LastError = SAError_NOT_SUPPORTED
} SAErrorCode;

typedef struct _SAError {
	SAErrorCode code;
	const char * description;
	const char * extra_description;
} SAError;

typedef enum {
    SAEncoding_NONE,
    SAEncoding_ULAW,
    SAEncoding_ALAW,
    SAEncoding_LINEAR,
    SAEncoding_DVI,
    SAEncoding_FLOAT,
    SAEncoding_G721,
    SAEncoding_G722,
    SAEncoding_G723,
    SAEncoding_LINEAR8,
    SAEncoding_MaxFormat = SAEncoding_LINEAR8
} SAEncoding;

typedef enum {
    SAFormatFlag_BigEndian = 1 << 0,
    SAFormatFlag_Signed = 1 << 1
} SAFormatFlag;

typedef struct _SAFormat {
    int flags;
    int sample_rate;
    SAEncoding encoding;
    int sample_size;
    int channels;
    int samples_per_unit;
} SAFormat;

typedef int SAFrameTime;
typedef int SABoolean;

#define SAFalse 0
#define SATrue 1

#define AUDIO_UNKNOWN_SIZE	((unsigned)(~0))

typedef struct {
    SAFormat * format;
    int frame_count;
    SAFrameTime frame_time;
    char * data;
} SAData;

typedef enum {
    SADevice_Speaker = 1 << 0,
    SADevice_Headphone = 1 << 1,
    SADevice_LineOut = 1 << 2
} SAOutputFlags;

typedef enum {
    SADevice_Microphone = 1 << 0,
    SADevice_LineIn = 1 << 1,
    SADevice_CD = 1 << 2,
    SADevice_InternalCD = 1 << 3
} SAInputFlags;

struct _SAStream;

typedef struct _SADevice {
    SAFormat play_format;
    SAFormat record_format;
    SAFormat * format_table;
    int format_count;
    int low_rate;
    int high_rate;
    const char * name;
    const char * hardware_name;

    double record_volume;
    uint_t record_nominal;
    uint_t record_min;
    uint_t record_max;
    double record_balance;
    SAInputFlags record_avail_ports;
    SAInputFlags record_ports;

    double play_volume;
    uint_t play_nominal;
    uint_t play_min;
    uint_t play_max;
    double play_balance;
    SAOutputFlags play_avail_ports;
    SAOutputFlags play_ports;
    
    uchar_t output_muted;

    int recordfd;
    int playfd;

    audio_info_t _audio_info;
    int real_timestamp;
    int samples_played;
    int real_samples_played;
    int frames_queued;
    
    struct _SAStream * play_streams;
    struct _SAStream * record_streams;

    thread_t _thread;
    int ctl_fd;
    mutex_t _lock;
} SADevice;

typedef struct {
    char * buffer;
    unsigned buffer_length;
    unsigned data_start;
    unsigned data_length;
    unsigned water_line;
    unsigned frames_transferred;
    SAFormat format;
} SABuffer;

typedef struct _SAStream {
    int flags;

    SADevice * device;
    SABuffer buffer;

    double meter_gain;
    mutex_t _lock;
    struct _SAStream * _next;
    struct _SAStream * _prev;
} SAStream;

typedef enum {
    SAStream_Underflow = 1 << 0,
    SAStream_Overflow = 1 << 1,
    SAStream_LowWater = 1 << 2,
    SAStream_HighWater = 1 << 3,
    SAStream_Playing = 1 << 4,
    SAStream_Recording = 1 << 5
} SAStreamFlags;

typedef enum {
    SAEvent_Error = 0,
    SAEvent_Overflow,
    SAEvent_Underflow,
    SAEvent_LowWater,
    SAEvent_HighWater,
    SAEvent_LevelMeter
} SAEventType;

typedef struct {
    SAEventType type;
    SAStream * stream;
    union {
	SAError error;
    } u;
} SAEvent;

/*
  * various utility functions
  */


SABoolean SANextEvent(int fd, SAEvent * event, SAError * errorReturn);
SABoolean SAFlushEvents(int fd, SAError * errorReturn);
const char * SAEventToString(SAEventType type);
int SAGetPipe();
const char * SAErrorString(SAErrorCode code);
SABoolean SAGainCalculate(SAFormat * format, char * buffer, int size,
			  double * gain, SAError * errorReturn);

unsigned SAFormatFrameSize(SAFormat * format);



/*
  *  SADevice functions
  */

SADevice * SADefaultDevice(SAError * errorReturn);
void SADeviceLock(SADevice * device);
void SADeviceUnlock(SADevice * device);
SABoolean SADeviceFindFormat(SADevice * device, SAFormat * format,
			     SAFormat * alternate, SAError * errorReturn);



SABoolean SADeviceGetPlayVolume(SADevice * device,
				double * volume,
				SAError * errorReturn);

SABoolean SADeviceSetPlayVolume(SADevice * device,
				double volume,
				SAError * errorReturn);


SABoolean SADeviceGetLinearPlayVolume(SADevice * device,
				    double *volume, double low, double hi,
				    SAError * errorReturn);

SABoolean SADeviceSetLinearPlayVolume(SADevice *device,
				    double volume, double low, double hi,
				    SAError * errorReturn);


SABoolean SADeviceGetPlayBalance( SADevice * device,
				  double * balance,
				  SAError * error);

SABoolean SADeviceSetPlayBalance( SADevice * device,
				  double balance,
				  SAError * error);

SABoolean SADeviceGetRecordVolume(SADevice * device,
				  double * volume,
				  SAError * errorReturn);

SABoolean SADeviceSetRecordVolume(SADevice *device,
				  double volume,
				  SAError * errorReturn);


SABoolean SADeviceGetLinearRecordVolume(SADevice * device,
					double *volume, double low, double hi,
					SAError * errorReturn);

SABoolean SADeviceSetLinearRecordVolume(SADevice *device,
				      double volume, double low, double hi,
				      SAError *errorReturn);

SABoolean SADeviceRecordBalanceEditable (SADevice *device,
				         SAError *error);

SABoolean SADeviceGetRecordBalance( SADevice * device,
				    double *balance,
				    SAError * errorReturn);

SABoolean SADeviceSetRecordBalance( SADevice * device,
				    double balance,
				    SAError * errorReturn);

SABoolean SADeviceGetMute( SADevice *device,
		           SABoolean *outputMuted,
			   SAError *errorReturn);

SABoolean SADeviceSetMuteOn( SADevice *device,
			     SAError *errorReturn);

SABoolean SADeviceSetMuteOff( SADevice *device,
			      SAError *errorReturn);

SABoolean SADeviceOutputExclusive (SADevice *device,
				   SAError  *errorReturn);

SABoolean SADeviceGetAvailOutputPorts ( SADevice *device,
		                        SAOutputFlags *ports,
			                SAError *errorReturn);

SABoolean SADeviceGetOutputPorts ( SADevice *device,
		                   SAOutputFlags *flags,
			           SAError *errorReturn);

SABoolean SADeviceSetOutputPorts( SADevice *device,
		                  SAOutputFlags flags,
			          SAError *errorReturn);

SABoolean SADeviceSetSpeakerPort( SADevice *device,
		                  SABoolean set,
			          SAError *errorReturn);

SABoolean SADeviceSetHeadphonePort( SADevice *device,
		                    SABoolean set,
			            SAError *errorReturn);

SABoolean SADeviceSetLineOutPort( SADevice *device,
		                  SABoolean set,
			          SAError *errorReturn);

SABoolean SADeviceGetAvailInputPorts ( SADevice *device,
		                       SAInputFlags *ports,
			               SAError *errorReturn);

SABoolean SADeviceGetInputPorts ( SADevice *device,
		                  SAInputFlags *flags,
			          SAError *errorReturn);

SABoolean SADeviceSetMicrophonePort( SADevice *device,
		                     SABoolean set,
			             SAError *errorReturn);

SABoolean SADeviceSetLineInPort( SADevice *device,
		                 SABoolean set,
			         SAError *errorReturn);

SABoolean SADeviceSetCDPort( SADevice *device,
		             SABoolean set,
			     SAError *errorReturn);

SABoolean SADeviceSetInternalCDPort( SADevice *device,
		                     SABoolean set,
			             SAError *errorReturn);

SABoolean SADeviceSigPollUpdateOn(SADevice * device, SAError *errorReturn);

SABoolean SADeviceSigPollUpdateOn(SADevice * device, SAError *errorReturn);


SAStream * SAStreamOpen(SADevice * device, SAError * errorReturn);
SAFormat * SAStreamFormat(SAStream * stream);
SABoolean SAStreamClose(SAStream * stream, SAError * errorReturn);
SABoolean SAStreamSetFormat(SAStream * stream, SAFormat * format,
			    SAFormat * alternate, SAError * errorReturn);

SABoolean SAStreamPlay(SAStream * stream, SAError * errorReturn);
SABoolean SAStreamRecord(SAStream * stream, SAError * errorReturn);
SABoolean SAStreamStop(SAStream * stream, SAError * errorReturn);

SABoolean SAStreamDrain(SAStream * stream, SAError * errorReturn);

SABoolean SAStreamRead(SAStream * stream, char * data,
		       int bytes, int * bytes_written, SAError * errorReturn);
SABoolean SAStreamWrite(SAStream * stream, char * data,
			int bytes, int * bytes_written, SAError * errorReturn);

void SAStreamLock(SAStream * stream);
void SAStreamUnlock(SAStream * stream);


/*
  *  SABuffer functions
  */

void SABufferInit(SABuffer * buffer, SAFormat * fmt, double time);
void SABufferFree(SABuffer * buffer);
SABoolean SABufferWriteOut(SABuffer * buffer, int * frames, int fd,
			   double * gain, SAError * error);
SABoolean SABufferReadIn(SABuffer * buffer, int * frames, int fd,
			 double * gain, SAError * error);

SABoolean SABufferCopyOut(SABuffer * buffer, int * frames, char * data,
			  double * gain, SAError * error);
SABoolean SABufferCopyIn(SABuffer * buffer, int * frames, char * data,
			 double * gain, SAError * error);

#ifdef __cplusplus
}
#endif

#endif /* SA_H */
