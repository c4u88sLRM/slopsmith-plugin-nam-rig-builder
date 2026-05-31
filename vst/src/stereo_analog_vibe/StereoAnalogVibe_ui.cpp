/* StereoAnalogVibe rack UI — shared rack_ui template. */
#include "StereoAnalogVibeParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "STEREO VIBRATO"
#define RACK_NAMES   kStereoAnalogVibeNames
#define RACK_DEFS    kStereoAnalogVibeDef
#define RACK_ACR 140
#define RACK_ACG 135
#define RACK_ACB 195
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.325f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
