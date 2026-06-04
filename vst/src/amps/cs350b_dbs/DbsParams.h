#ifndef DBS_PARAMS_H
#define DBS_PARAMS_H

// "Marsten DBS 7400" — Marshall DBS 7400 (Dynamic Bass System) front panel,
// modeled from the 7400 service schematic (FRONTEND + COMPGRAF boards). The
// real panel has Pre-amp Blend, Gain, Bright/Deep, Primary EQ (Bass/Middle/
// Treble), Compression (Threshold/Depth), a graphic EQ and Volume. Rocksmith's
// "CLH-350B" drives Gain, Bass, Treble and the 7 graphic bands; we expose the
// rest (Middle/Bright/Deep/Comp/Volume + Lo input) at faithful defaults.
//   Inputs : Hi / Lo (the Lo jack pads hot basses, like the panel's 2 inputs).
//   Gain   : op-amp preamp gain (clean solid-state — no tube saturation).
//   Bright / Deep : the DBS voicing switches (HF lift / LF lift).
//   Bass / Middle / Treble : the passive Primary EQ.
//   Compression : the built-in compressor amount.
//   7-band graphic EQ : 30/90/275/750/2.2k/6.5k/12k Hz, +/-15 dB (gyrator).
//   Volume : master output into the SS power amp (~400 W, high headroom).
enum DbsParamId {
    kGain = 0, kBass, kMiddle, kTreble, kComp, kVolume,        // knobs
    kEq30, kEq90, kEq275, kEq750, kEq2k2, kEq6k5, kEq12k,      // 7-band graphic
    kBright, kDeep, kLoInput,                                  // switches
    kParamCount
};
static const int kFirstEq = kEq30;     // 7 EQ bands are contiguous from here
static const int kNumEq = 7;
static const float kEqFreqs[kNumEq] = { 30.f, 90.f, 275.f, 750.f, 2200.f, 6500.f, 12000.f };

static const char* const kDbsNames[kParamCount] = {
    "Gain", "Bass", "Middle", "Treble", "Compression", "Volume",
    "30 Hz", "90 Hz", "275 Hz", "750 Hz", "2.2 kHz", "6.5 kHz", "12 kHz",
    "Bright", "Deep", "Lo Input"
};
static const char* const kDbsSymbols[kParamCount] = {
    "gain", "bass", "middle", "treble", "comp", "volume",
    "eq30", "eq90", "eq275", "eq750", "eq2k2", "eq6k5", "eq12k",
    "bright", "deep", "loinput"
};
static const float kDbsMin[kParamCount] = { 0,0,0,0,0,0, 0,0,0,0,0,0,0, 0,0,0 };
static const float kDbsMax[kParamCount] = { 1,1,1,1,1,1, 1,1,1,1,1,1,1, 1,1,1 };
// Tone knobs 0.5 flat; Gain 0.5; Comp 0.35 (the DBS comp sits gently in-circuit);
// Volume 0.7; EQ bands 0.5 (flat); Bright/Deep off; Lo input off.
static const float kDbsDef[kParamCount] = {
    0.50f, 0.50f, 0.50f, 0.50f, 0.35f, 0.70f,
    0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f, 0.50f,
    0.00f, 0.00f, 0.00f
};

#endif // DBS_PARAMS_H
