/* StudioWahFilter rack UI — shared rack_ui template. */
#include "StudioWahFilterParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "STUDIO WAH FILTER"
#define RACK_NAMES   kStudioWahFilterNames
#define RACK_DEFS    kStudioWahFilterDef
#define RACK_ACR 130
#define RACK_ACG 180
#define RACK_ACB 155
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.230f,0.50f,0.026f}, {0.325f,0.50f,0.026f}, {0.420f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
