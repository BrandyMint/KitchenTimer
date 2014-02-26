#include "ios_common.h"

#include <AudioToolbox/AudioToolbox.h>
#include <UIKit/UIApplication.h>

void ios_vibrate ()
{
    AudioServicesPlaySystemSound (kSystemSoundID_Vibrate);
}
void ios_adjust_idle_timeout ()
{
    [UIApplication sharedApplication].idleTimerDisabled = NO;
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}
