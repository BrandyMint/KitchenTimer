#include "ios_common.h"

#include <AudioToolbox/AudioToolbox.h>
#include <AVFoundation/AVAudioPlayer.h>
#include <UIKit/UIApplication.h>
#include <Foundation/NSURL.h>

void ios_vibrate ()
{
    AudioServicesPlaySystemSound (kSystemSoundID_Vibrate);
}
void ios_adjust_idle_timeout ()
{
    [UIApplication sharedApplication].idleTimerDisabled = NO;
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

@interface IOSAudioSequencer: NSObject <AVAudioPlayerDelegate>
{
}
- (void) initAudioSequencer:(av_audio_channel_t*) channel sample_count:(int)sample_count sample_source_names:(const char**)sample_source_names;
- (void) startPlay;
- (void) audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag;
@end

struct av_audio_channel_t {
    AVAudioPlayer *player;
    IOSAudioSequencer *sequencer;
    int sequence_started;
};


@implementation IOSAudioSequencer
{
    av_audio_channel_t *audio_channel;
    NSDate *keepalive_start;
    NSMutableArray *sample_sources;
}
- (void) initAudioSequencer:(av_audio_channel_t*) channel sample_count:(int)sample_count sample_source_names:(const char**)sample_source_names
{
    audio_channel = channel;
    keepalive_start = [NSDate date];
    sample_sources = [[NSMutableArray alloc] initWithCapacity:sample_count];
    for (int i = 0; i < sample_count; ++i) {
	NSString *ns_name = [[NSString alloc] initWithUTF8String: sample_source_names[i]];
	NSString *path = [[NSBundle mainBundle] pathForResource: ns_name ofType: nil];
	NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: path];
	[sample_sources addObject:fileURL];
	[ns_name release];
	[fileURL release];
    }
}
- (void) startPlay
{
    [keepalive_start release];
    keepalive_start = [NSDate date];
}
- (void) startSample
{
    if (audio_channel->player)
	[audio_channel->player release];
    audio_channel->player = [[AVAudioPlayer alloc] initWithContentsOfURL: sample_sources[random ()%[sample_sources count]] error:nil];
    audio_channel->player.delegate = self;
    [audio_channel->player setNumberOfLoops: 0];
    [audio_channel->player prepareToPlay];
    [audio_channel->player play];
}
- (void) audioPlayerDidFinishPlaying:(AVAudioPlayer *)player successfully:(BOOL)flag
{
    if (flag && player && audio_channel->sequence_started && ([keepalive_start timeIntervalSinceNow] > -0.5)) {
	[self startSample];
    } else {
	audio_channel->sequence_started = 0;
    }
}
@end

av_audio_channel_t *av_audio_channel_create ()
{
    av_audio_channel_t *s = (av_audio_channel_t*) malloc (sizeof (av_audio_channel_t));
    s->player = NULL;
    s->sequencer = NULL;
    s->sequence_started = 0;
    return s;
}
av_audio_channel_t *av_audio_channel_create_with_sequencer (int sample_count, const char **sample_source_names)
{
    av_audio_channel_t *s = (av_audio_channel_t*) malloc (sizeof (av_audio_channel_t));
    s->player = NULL;
    IOSAudioSequencer *sequencer = [[IOSAudioSequencer alloc] init];
    [sequencer initAudioSequencer:s sample_count:sample_count sample_source_names:sample_source_names];
    s->sequencer = sequencer;
    s->sequence_started = 0;
    return s;
}
void av_audio_channel_destroy (av_audio_channel_t *s)
{
    if (!s) return;
    if (s->sequencer)
	[s->sequencer release];
    if (s->player)
	[s->player release];
    free (s);
}
void av_audio_channel_play (av_audio_channel_t *s, const char *name, int looped)
{
    NSString *ns_name = [[NSString alloc] initWithUTF8String: name];
    NSString *path = [[NSBundle mainBundle] pathForResource: ns_name ofType: nil];
    NSURL *fileURL = [[NSURL alloc] initFileURLWithPath: path];
    if (s->player)
	[s->player release];
    s->player = [[AVAudioPlayer alloc] initWithContentsOfURL: fileURL error:nil];
    [ns_name release];
    [fileURL release];
    if (looped)
	[s->player setNumberOfLoops: -1];
    else
	[s->player setNumberOfLoops: 0];
    [s->player prepareToPlay];
    [s->player play];
}
void av_audio_channel_play_sequence (av_audio_channel_t *s)
{
    IOSAudioSequencer *sequencer = s->sequencer;
    if (sequencer) {
	[sequencer startPlay];
	if (!s->sequence_started) {
	    s->sequence_started = 1;
	    [sequencer startSample];
	}
    }
}
void av_audio_channel_stop (av_audio_channel_t *s)
{
    if (s->player)
	[s->player stop];
}
