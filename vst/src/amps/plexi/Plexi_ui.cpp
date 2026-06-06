/* Plexi fallback DPF UI — shared pedal_ui template. The real in-app face is
 * drawn by pedal_canvas.js (P.plexi, "Marsten" brand, gold-panel Plexi head).
 * Knob count + labels come from the plugin params (6: the full 1959 panel). */
#include "PlexiParams.h"
#define PEDAL_TITLE  "MARSTEN PLEXI"
#define PEDAL_NAMES  kPlexiNames
#define PEDAL_DEFS   kPlexiDef
#define PEDAL_ACR 198
#define PEDAL_ACG 162
#define PEDAL_ACB 78
#define PEDAL_ARCR 26
#define PEDAL_ARCG 24
#define PEDAL_ARCB 22
#define PEDAL_W 560
#define PEDAL_H 300
// 7 controls (Presence, Bass, Middle, Treble, Loudness I, Loudness II, Input)
#define PEDAL_KNOBS { \
  {0.10f,0.30f,0.052f}, {0.24f,0.30f,0.052f}, {0.38f,0.30f,0.052f}, \
  {0.52f,0.30f,0.052f}, {0.66f,0.30f,0.052f}, {0.80f,0.30f,0.052f}, \
  {0.93f,0.30f,0.052f} }
#include "../../_shared/pedal_ui.hpp"
