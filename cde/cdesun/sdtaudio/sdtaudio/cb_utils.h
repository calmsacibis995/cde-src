/*
 * (c) Copyright 1996 Sun Microsystems, Inc.
 */


#pragma ident "@(#)cb_utils.h	1.5 97/03/21 SMI"


#ifndef _CB_UTILS_H_
#define _CB_UTILS_H_

#include <Dt/Dnd.h>

#define	VOL_INCREMENT	5
#define VOL_MIN		0
#define VOL_MAX		100

Boolean    SDtAuGetRecordValues(WindowData *);
Boolean	   SDtAudioSetPlayVol(SAStream *, int, SAError *);
Boolean	   SDtAudioGetPlayVol(SAStream *, double *, SAError *);
Boolean	   SDtAudioSetRecVol(SAStream *, int, SAError *);
Boolean	   SDtAudioGetRecVol(SAStream *, double *, SAError *);
Boolean	   SDtAudioSetMuteState(SAStream *, SABoolean, SAError *);
Boolean	   SDtAudioGetMuteState(SAStream *, SABoolean *, SAError *);
void       SDtAuUpdateDevices(WindowData *);
void       SDtAuSaveOptions(WindowData *);
void       SDtAuInitOptions(WindowData *);
void       SDtAuGetOptions(void);
void       SDtAuHideWaveWin(WindowData *);
void       SDtAuShowWaveWin(WindowData *);
void       SDtAuSetMessage(Widget, char *);
void       SDtAuSetSameDimension(Widget *, int, String);
Boolean    SDtAuSetSigpoll(WindowData *);
Boolean    SDtAuSetDeviceState(WindowData *);
void	   SDtAuUpdateView(void);
void	   SDtAuUpdateWaveWin(WindowData *);
void       SDtAuInitDrawables(WindowData *);
void       SDtAuUpdateWaveWin(WindowData *);
void	   SDtAuUpdateTimeLabels(WindowData *);
void       SDtAuUpdateInfoDlog(WindowData *);
void       SDtAuUpdateLevelMeter(WindowData *);
void       SDtAuDndFilename(WindowData *, DtDndTransferCallbackStruct *);
void       SDtAuDndBuffer(WindowData *, DtDndTransferCallbackStruct *);
void       SDtAuWaveLocLine(int, WindowData *);
void       SDtAuWaveLocTime(int, WindowData *);
void	   SDtAuClearSelection(WindowData *);
void	   SDtAuUpdateWaveCursor(WindowData *);
void	   SDtAuDrawWaveCursor(WindowData *, short);
void       SDtAuResetOptions(WindowData *);
void       SDtAuStopAndWait(WindowData *);

#ifdef SELECTION
void       SDtAuChangeSel(int, int, int, WindowData *);
#endif

#endif				/* _CB_UTILS_H_ */
