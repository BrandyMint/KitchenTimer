#include "ios_vibro.h"

#include <AudioToolbox/AudioToolbox.h>

void ios_vibro ()
{
    AudioServicesPlaySystemSound (kSystemSoundID_Vibrate);
}
