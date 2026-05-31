/* StudioPitch rack UI — shared rack_ui template. */
#include "StudioPitchParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "STUDIO PITCH"
#define RACK_NAMES   kStudioPitchNames
#define RACK_DEFS    kStudioPitchDef
#define RACK_ACR 160
#define RACK_ACG 185
#define RACK_ACB 150
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.262f,0.50f,0.026f}, {0.388f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
