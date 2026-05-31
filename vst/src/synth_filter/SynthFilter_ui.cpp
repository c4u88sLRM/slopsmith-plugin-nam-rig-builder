/* SynthFilter rack UI — shared rack_ui template. */
#include "SynthFilterParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "SYNTH FILTER"
#define RACK_NAMES   kSynthFilterNames
#define RACK_DEFS    kSynthFilterDef
#define RACK_ACR 150
#define RACK_ACG 185
#define RACK_ACB 130
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.230f,0.50f,0.026f}, {0.325f,0.50f,0.026f}, {0.420f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
