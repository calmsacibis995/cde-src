/*
 *
 * FILE: file.h
 *
 * Temporary 'C' wrappers for the C++ functions in XAudio...
 *
 */

#ifndef AUDIOFILE_H
#define AUDIOFILE_H


/*
 * my XAudio helper functions
 */
void xa_play_audio(WindowData *wd);
void xa_record_audio(WindowData *wd);
void xa_stop_audio(WindowData *wd);

#endif /* AUDIOFILE_H */
