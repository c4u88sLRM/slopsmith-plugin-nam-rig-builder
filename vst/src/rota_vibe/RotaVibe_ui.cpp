/* RotaVibe rack UI — shared rack_ui template. */
#include "RotaVibeParams.h"
#define RACK_COUNT   kParamCount
#define RACK_TITLE   "ROTA VIBE"
#define RACK_NAMES   kRotaVibeNames
#define RACK_DEFS    kRotaVibeDef
#define RACK_ACR 205
#define RACK_ACG 135
#define RACK_ACB 120
#define RACK_KNOBS { {0.135f,0.50f,0.026f}, {0.262f,0.50f,0.026f}, {0.388f,0.50f,0.026f}, {0.515f,0.50f,0.026f} }
#include "../_shared/rack_ui.hpp"
