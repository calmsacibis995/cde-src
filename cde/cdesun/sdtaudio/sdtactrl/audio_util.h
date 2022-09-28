/*
 * File: audio_util.h
 *
 * (c) Copyright 1996 Sun Microsystems, Inc.
 *
 *
 * 
*/

#pragma ident "@(#)audio_util.h	1.6 96/09/11 SMI"
#ifndef AUDIO_UTIL_H
#define AUDIO_UTIL_H

#include <stdio.h>
#include <sa.h>

void ChangeRecordVolume( int newVol, int low, int hi);
int  GetRecordVolume(int *vol, int low, int hi);
void ChangeRecordBalance( double balance);
int  GetRecordBalance( double *balance);

void ChangePlayVolume( int newVol, int low, int hi);
int  GetPlayVolume( int *vol, int low, int hi);
void ChangePlayBalance( double balance);
int  GetPlayBalance ( double *balance);

void ChangeMuteOutput(SABoolean mute_output);
int GetMuteOutput(SABoolean *set);

void ChangeSpeakerOutputPort(SABoolean set);
void ChangeHeadphoneOutputPort(SABoolean set);
void ChangeLineOutOutputPort(SABoolean set);

#endif
