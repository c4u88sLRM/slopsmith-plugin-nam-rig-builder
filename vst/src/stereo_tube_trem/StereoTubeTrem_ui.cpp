/* StereoTubeTrem rack UI — shared rack_ui template. */
#include "StereoTubeTremParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "STEREO TUBE TREM"
#define RACK_NAMES   kStereoTubeTremNames
#define RACK_DEFS    kStereoTubeTremDef
#define RACK_ACR 150
#define RACK_ACG 180
#define RACK_ACB 160
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.325f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
