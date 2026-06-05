#ifndef RUMBLE_PARAMS_H
#define RUMBLE_PARAMS_H

// "Bender Fumble 800" — Fender Rumble 800 (modern Class-D head) front panel, 1:1:
//   Gain   : preamp input gain.
//   Bright / Contour / Vintage : the 3 voicing buttons (top-end lift / mid
//            scoop "smile" / warm tube-style voicing).
//   Overdrive — Drive : the built-in overdrive amount; Level : its output.
//   4-band EQ : Bass / Low Mid / High Mid / Treble.
//   Master : output level (Class-D power, ~800 W, huge clean headroom).
enum RumbleParamId {
    kGain = 0, kDrive, kLevel, kBass, kLowMid, kHighMid, kTreble, kMaster,  // knobs
    kBright, kContour, kVintage,                                           // buttons
    kParamCount
};

static const char* const kRumbleNames[kParamCount] = {
    "Gain", "Drive", "Level", "Bass", "Low Mid", "High Mid", "Treble", "Master",
    "Bright", "Contour", "Vintage"
};
static const char* const kRumbleSymbols[kParamCount] = {
    "gain", "drive", "level", "bass", "lowmid", "highmid", "treble", "master",
    "bright", "contour", "vintage"
};
static const float kRumbleMin[kParamCount] = { 0,0,0,0,0,0,0,0, 0,0,0 };
static const float kRumbleMax[kParamCount] = { 1,1,1,1,1,1,1,1, 1,1,1 };
// Gain 0.5; Drive 0 (overdrive clean); Level 0.5 (~unity); EQ flat 0.5;
// Master 0.7; voicing buttons off.
static const float kRumbleDef[kParamCount] = {
    0.50f, 0.00f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.70f,
    0.00f, 0.00f, 0.00f
};

#endif // RUMBLE_PARAMS_H
