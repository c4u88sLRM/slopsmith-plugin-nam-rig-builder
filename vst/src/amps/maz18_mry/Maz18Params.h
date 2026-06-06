#ifndef MAZ18_PARAMS_H
#define MAZ18_PARAMS_H

/*
 * MR. Y MAZ 18 = Dr. Z Maz 18 Jr Reverb — a 2xEL84 (~18W) class-AB head with a
 * GZ34 tube rectifier (=> noticeable SAG / spongy, touch-sensitive feel). Parody
 * brand "Mr. Y" (Dr. Z -> Mr. Y). The face must never read "Dr. Z" or "Maz".
 *
 * Rocksmith gear: Amp_GB38. RS exposes only Gain/Bass/Mid/Treble, so RS Gain ->
 * VOLUME (the preamp gain / the amp's only drive control), Bass/Mid/Treble -> the
 * TMB tone stack. Voiced Vox-meets-Fender: chimey, midrange-forward, breaks up
 * musically and earlier than a big amp because of the small EL84 power section
 * and the spongy GZ34 rectifier sag.
 *
 * Panel (1:1, left->right, per the schematic): VOLUME (1MA), TREBLE, MIDDLE, BASS
 * (TMB stack: Treble 250k/250pF, Middle 10k, Bass ~1M, 56k/100k/47n), CUT (250kA,
 * post-PI treble cut — higher = darker), MASTER (1MA), REVERB (12AT7 driver +
 * spring tank + 12AX7 recovery, 250k level). 2xEL84 ~18W, CinTran OT, GZ34 sag.
 */
enum Maz18ParamId
{
    kVolume = 0,    // VOLUME  — preamp gain (1MA)             [RS Gain]
    kTreble,        // TREBLE  tone stack (250k, 250pF cap)    [RS Treble]
    kMiddle,        // MIDDLE  tone stack (10k)                [RS Mid]
    kBass,          // BASS    tone stack (~1M)                [RS Bass]
    kCut,           // CUT     post-PI treble cut (higher = darker)
    kMaster,        // MASTER  — output master (1MA)
    kReverb,        // REVERB  — spring reverb mix
    kParamCount
};

static const char* const kMaz18Names[kParamCount] = {
    "Volume", "Treble", "Middle", "Bass", "Cut", "Master", "Reverb",
};

static const char* const kMaz18Symbols[kParamCount] = {
    "volume", "treble", "middle", "bass", "cut", "master", "reverb",
};

static const float kMaz18Min[kParamCount] = { 0,0,0,0,0,0,0 };
static const float kMaz18Max[kParamCount] = { 1,1,1,1,1,1,1 };
// Manual-insert defaults: a chimey, midrange-forward Vox-meets-Fender voice,
// moderate Volume just into musical breakup, masters open, reverb off.
static const float kMaz18Def[kParamCount] = {
    0.60f, 0.55f, 0.50f, 0.50f, 0.40f, 0.70f, 0.00f,
};

#endif // MAZ18_PARAMS_H
