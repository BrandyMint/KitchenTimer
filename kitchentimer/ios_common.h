// -*- mode: c++ -*-

#ifndef IOS_COMMON_H
#define IOS_COMMON_H

extern void ios_vibrate ();
extern void ios_adjust_idle_timeout ();

struct av_audio_channel_t;
typedef struct av_audio_channel_t av_audio_channel_t;

extern av_audio_channel_t *av_audio_channel_create ();
extern av_audio_channel_t *av_audio_channel_create_with_sequencer (int, const char**);
extern void av_audio_channel_destroy (av_audio_channel_t*);
extern void av_audio_channel_play (av_audio_channel_t*, const char*, int);
extern void av_audio_channel_play_sequence (av_audio_channel_t*);
extern void av_audio_channel_stop (av_audio_channel_t*);

#endif
