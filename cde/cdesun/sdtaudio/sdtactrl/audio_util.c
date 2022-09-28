/*
 * file audio_util.c
 *
 * (c) Copyright 1996 Sun Microsystems, Inc.
 *
*/ 

#pragma ident "@(#)audio_util.c	1.10 96/09/12 SMI"

#include "audio_util.h"
#include "audio_control_ui.h"
#include <stdarg.h>


static void _ReportError(char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
}

static void _ReportSAError(SAError e, char *format, ...) {
    char strBuff[512];
    va_list args;
    va_start(args, format);
    
    if (e.extra_description) {
      sprintf(strBuff, "%s (%s) - ", SAErrorString(e.code), e.extra_description);
      strcat(strBuff, format);
    }
    
    vfprintf(stderr, strBuff, args);
    fprintf(stderr, "\n");
}

static void _Log(char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
}



/*
 * Initialize must be called to use any of these
 * routines. It sets this "global" device we'll be using.
*/
static SADevice *theDevice = NULL;
void Initialize() {

    SAError error;

    /* Get the device */
    theDevice = SADefaultDevice(&error);
    if( theDevice == NULL) {
	_ReportSAError(error, "Couldn't initialize the device");
	_ReportError("exiting");
	exit(-1);
    }

    /* Turn on the sigpoll updating */
    if( !SADeviceSigPollUpdateOn(theDevice, &error)) {
	_ReportSAError(error, "Couldn't turn updates on.");
	_ReportError("exiting");
	exit(-1);
    }
}

Boolean OutputExclusive()
{
   SAError  error;

   if (SADeviceOutputExclusive(theDevice, &error) == SATrue)
     return True;
   else
     return False;
}

void ChangeDragRecordVolume(int sliderValue, int lo, int hi) {

    SAError error;

    if( !SADeviceSetLinearRecordVolume(theDevice,
				       (double)sliderValue,
				       (double)lo, (double)hi,
				       &error)) {
        _ReportSAError(error, "Can't change the record volume");
    }
    
}

void ChangeRecordVolume(int sliderValue, int lo, int hi) {

    SAError error;
    SABoolean up;
    int       i, vol;

    if (dtb_audio_control_main_window.last_record_vol < sliderValue)
      up = SATrue;
    else 
      up = SAFalse;
      
    if (up == SATrue) {  /* Increment one at a time */
      for (i = 0; i < 3; i++) {
        if (((sliderValue+i) >= lo) && ((sliderValue+i) <= hi)) {
          if( !SADeviceSetLinearRecordVolume(theDevice,
				     (double)sliderValue + i,
				     (double)lo, (double)hi,
				     &error)) {
	    _ReportSAError(error, "Can't change the record volume");
          }
        }
	/* Check if vol really changed */
	if (GetRecordVolume (&vol, lo, hi)) {
	  if (vol >= (sliderValue + i))   /* value changed! */
	    break;
        }
      }
    }
    else {  /* decrement one at a time */
      for (i = 0; i < 10; i++) {
 	if (((sliderValue-i) >= lo) && ((sliderValue-i) <= hi)) {
          if( !SADeviceSetLinearRecordVolume(theDevice,
				     (double)sliderValue - i,
				     (double)lo, (double)hi,
				     &error)) {
	    _ReportSAError(error, "Can't change the record volume");
	  }
        }
	/* Check if vol really changed */
	if (GetRecordVolume (&vol, lo, hi)) {
	  if (vol <= (sliderValue - i))   /* value changed! */
	    break;
        }
      }
    }

}

/* 0 on error, else nonzero */
int GetRecordVolume( int *vol, int lo, int hi) {
    SAError error;
    double dVol;

    if( SADeviceGetLinearRecordVolume( theDevice, &dVol,
				       (double)lo, (double)hi, &error)) {
	*vol = (int)dVol;
	return 1;
    } else {
	_ReportSAError(error, "Can't get the record volume");
	return 0;
    }
}


void ChangeRecordBalance(double balance) {
    SAError error;

    if( !SADeviceSetRecordBalance(theDevice, balance, &error)) {
	_ReportSAError(error, "Can't change the record balanace");
    }
}


int GetRecordBalance(double *value) {
    SAError error;
    if( !SADeviceGetRecordBalance(theDevice, value, &error)) {
	_ReportSAError(error, "Can't get the record balance");
	return 0;
    }
    
    return 1;
    
}


void ChangeDragPlayVolume(int sliderValue, int lo, int hi) {
    SAError error;

    if( !SADeviceSetLinearPlayVolume(theDevice,
				     (double)sliderValue,
				     (double)lo, (double)hi,
				     &error)) {
	_ReportSAError(error, "Can't change the play volume");
    }
}

void ChangePlayVolume(int sliderValue, int lo, int hi) {

    SAError error;
    SABoolean up;
    int       i, vol;

    if (dtb_audio_control_main_window.last_play_vol < sliderValue)
      up = SATrue;
    else 
      up = SAFalse;
      
    if (up == SATrue) {	/* Increment one at a time */
      for (i = 0; i < 3; i++) {
        if (((sliderValue + i) >= lo) && ((sliderValue + i) <= hi)) {
          if ( !SADeviceSetLinearPlayVolume(theDevice,
				     (double)sliderValue + i,
				     (double)lo, (double)hi,
				     &error)) {
	    _ReportSAError(error, "Can't change the play volume");
	  }
        }
	/* Check if vol really changed */
	if (GetPlayVolume (&vol, lo, hi)) {
	  if (vol >= (sliderValue + i))   /* value changed! */
	    break;
        }
      }
    }
    else {  /* decrement one at a time */
      for (i = 0; i < 3; i++) {
	if (((sliderValue-i) >= lo) && ((sliderValue-i) <= hi)) {
          if( !SADeviceSetLinearPlayVolume(theDevice,
				     (double)sliderValue - i,
				     (double)lo, (double)hi,
				     &error)) {
	    _ReportSAError(error, "Can't change the play volume");
	  }
        }
	/* Check if vol really changed */
	if (GetPlayVolume (&vol, lo, hi)) {
	  if (vol <= (sliderValue - i))   /* value changed! */
	    break;
        }
      }
    }

}

/* 0 on error, else nonzero */
int GetPlayVolume(int *vol, int lo, int hi) {

    SAError error;
    double dVol;

    /* Get the value */
    if( SADeviceGetLinearPlayVolume(theDevice, &dVol, lo, hi, &error)) {
	*vol = (int)dVol;
	return 1;
    } else {
	_ReportSAError(error, "Can't get the play volume");
	return 0;
    }
}

void ChangeHeadphoneOutputPort(SABoolean set) {
    SAError error;

    if( !SADeviceSetHeadphonePort(theDevice, set, &error)) {
	if (!SADeviceOutputExclusive(theDevice, &error))
	  _ReportSAError(error, "Can't change the headphone port");
    }
}

void ChangeLineOutOutputPort(SABoolean set) {
    SAError error;

    if( !SADeviceSetLineOutPort(theDevice, set, &error)) {
	if (!SADeviceOutputExclusive(theDevice, &error))
	  _ReportSAError(error, "Can't change the line out port");
    }
}

void ChangeMicrophoneInputPort(SABoolean set) 
{
    SAError error;

    if( !SADeviceSetMicrophonePort(theDevice, set, &error)) {
	_ReportSAError(error, "Can't change the mic port");
    }
}

void ChangeLineInInputPort(SABoolean set) 
{
    SAError error;

    if( !SADeviceSetLineInPort(theDevice, set, &error)) {
	_ReportSAError(error, "Can't change the line in port");
    }
}

void ChangeCDInputPort(SABoolean set) 
{
    SAError error;

    if( !SADeviceSetCDPort(theDevice, set, &error)) {
	_ReportSAError(error, "Can't change the CD input port");
    }
}

void ChangeInternalCDInputPort(SABoolean set) 
{
    SAError error;

    if( !SADeviceSetCDPort(theDevice, set, &error)) {
      _ReportSAError(error, "Can't change the Internal CD input port");
    }
}

void ChangeSpeakerOutputPort(SABoolean set) 
{
    SAError error;

    if( !SADeviceSetSpeakerPort(theDevice, set, &error)) {
	if (!SADeviceOutputExclusive(theDevice, &error))
	  _ReportSAError(error, "Can't change the speaker port");
    }
}

void ChangeMuteOutput(SABoolean mute_output) {
    SAError error;

    if (mute_output == SATrue) {
      if( !SADeviceSetMuteOn(theDevice, &error)) {
	_ReportSAError(error, "Can't change the mute output");
      }
    }
    else {
      if( !SADeviceSetMuteOff(theDevice, &error)) {
	_ReportSAError(error, "Can't change the mute output");
      }
    }
}

/* 0 on error, else nonzero */
int GetMuteOutput(SABoolean *set) {

    SAError error;

    /* Get the value */
    if( SADeviceGetMute(theDevice, set, &error)) {
        return 1;
    } else {
        _ReportSAError(error, "Can't get the mute output");
        return 0;
    }
}

/* 0 on error, else nonzero */
int GetAvailOutputPorts(SAOutputFlags *ports) {

    SAError error;

    /* Get the value */
    if( SADeviceGetAvailOutputPorts(theDevice, ports, &error)) {
        return 1;
    } else {
        _ReportSAError(error, "Can't get the available output ports.");
        return 0;
    }
}

/* 0 on error, else nonzero */
int GetAvailInputPorts(SAInputFlags *ports) {

    SAError error;

    /* Get the value */
    if( SADeviceGetAvailInputPorts(theDevice, ports, &error)) {
        return 1;
    } else {
        _ReportSAError(error, "Can't get the available input ports.");
        return 0;
    }
}

/* 0 on error, else nonzero */
int GetOutputPorts(SAOutputFlags *outputs) {

    SAError error;

    /* Get the value */
    if( SADeviceGetOutputPorts(theDevice, outputs, &error)) {
        return 1;
    } else {
        _ReportSAError(error, "Can't get the output ports.");
        return 0;
    }
}

/* 0 on error, else nonzero */
int GetInputPorts(SAInputFlags *inputs) {

    SAError error;

    /* Get the value */
    if( SADeviceGetInputPorts(theDevice, inputs, &error)) {
        return 1;
    } else {
        _ReportSAError(error, "Can't get the input ports.");
        return 0;
    }
}

void ChangePlayBalance(double balance) {
    SAError error;

    if( !SADeviceSetPlayBalance(theDevice, balance, &error)) {
	_ReportSAError(error, "Can't change the play balanace");
    }
}

int GetPlayBalance(double *value) {
    
    SAError error;
    if( !SADeviceGetPlayBalance(theDevice, value, &error)) {
	_ReportSAError(error, "Can't get the play balance");
	return 0;
    }

    return 1;
    
}

