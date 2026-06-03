/*
 * Sampleg SBT-CL — Ampeg SVT-CL all-tube bass head model.
 *
 * The SVT-CL service schematic in hand covers the 6550 power amp + supply;
 * the preamp behaviour here follows the well-documented SVT-CL front end:
 *
 *   1. Input : Normal jack, or the -15 dB padded jack (modeled as a pad switch).
 *   2. Gain  : drives a 12AX7 stage — a gentle ASYMMETRIC soft clip, so pushing
 *              Gain adds the even-harmonic SVT growl/grit (clean at low Gain).
 *   3. Ultra Lo : the SVT's fixed loudness contour — boost deep lows (~50 Hz)
 *              and highs (~8 kHz), scoop the mids (~500 Hz).
 *   4. Ultra Hi : presence/high-shelf boost (~4 kHz) — the SVT clank/bite.
 *   5. Tone stack : Bass low shelf (~70 Hz), Midrange peaking cut/boost at the
 *              Frequency-selected centre (220/450/800/1600/3000 Hz), Treble
 *              high shelf (~5 kHz). All ±15 dB, flat at 0.5.
 *   6. Master : output level.
 */
#include "DistrhoPlugin.hpp"
#include "SvtParams.h"
#include <cmath>

START_NAMESPACE_DISTRHO

// ── RBJ biquad (transposed direct form II) ───────────────────────────────────
class Biquad {
    float b0=1, b1=0, b2=0, a1=0, a2=0;
    float z1=0, z2=0;
public:
    void reset() { z1 = z2 = 0.f; }
    inline float process(float x) {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void setLowShelf(float fc, float dB, float fs) {
        const float A = std::pow(10.f, dB / 40.f);
        const float w0 = 6.2831853f * fc / fs, cw = std::cos(w0), sw = std::sin(w0);
        const float alpha = sw * 0.5f * 1.4142135f;
        const float sA = std::sqrt(A), tsAa = 2.f * sA * alpha;
        const float a0 =       (A + 1) + (A - 1) * cw + tsAa;
        b0 =  A * ((A + 1) - (A - 1) * cw + tsAa) / a0;
        b1 = 2*A * ((A - 1) - (A + 1) * cw)        / a0;
        b2 =  A * ((A + 1) - (A - 1) * cw - tsAa)  / a0;
        a1 = -2 * ((A - 1) + (A + 1) * cw)         / a0;
        a2 =      ((A + 1) + (A - 1) * cw - tsAa)  / a0;
    }
    void setHighShelf(float fc, float dB, float fs) {
        const float A = std::pow(10.f, dB / 40.f);
        const float w0 = 6.2831853f * fc / fs, cw = std::cos(w0), sw = std::sin(w0);
        const float alpha = sw * 0.5f * 1.4142135f;
        const float sA = std::sqrt(A), tsAa = 2.f * sA * alpha;
        const float a0 =       (A + 1) - (A - 1) * cw + tsAa;
        b0 =  A * ((A + 1) + (A - 1) * cw + tsAa) / a0;
        b1 = -2*A * ((A - 1) + (A + 1) * cw)      / a0;
        b2 =  A * ((A + 1) + (A - 1) * cw - tsAa) / a0;
        a1 =  2 * ((A - 1) - (A + 1) * cw)        / a0;
        a2 =      ((A + 1) - (A - 1) * cw - tsAa) / a0;
    }
    void setPeak(float fc, float dB, float Q, float fs) {
        const float A = std::pow(10.f, dB / 40.f);
        const float w0 = 6.2831853f * fc / fs, cw = std::cos(w0), sw = std::sin(w0);
        const float alpha = sw / (2.f * Q);
        const float a0 = 1 + alpha / A;
        b0 = (1 + alpha * A) / a0;
        b1 = (-2 * cw)       / a0;
        b2 = (1 - alpha * A) / a0;
        a1 = (-2 * cw)       / a0;
        a2 = (1 - alpha / A) / a0;
    }
    void setBypass() { b0 = 1; b1 = b2 = a1 = a2 = 0; z1 = z2 = 0; }
};

class SvtChannel {
    float fs = 48000.f;
    Biquad ulLow, ulMid, ulHigh;     // Ultra Lo contour
    Biquad uhShelf;                  // Ultra Hi presence
    Biquad bqBass, bqMid, bqTreble;  // tone stack
    float inGain = 1.f, inComp = 1.f, master = 1.f;

    // gentle asymmetric tube-style soft clip (even + odd harmonics)
    static inline float tube(float x) {
        // bias the curve slightly so positive/negative clip differently
        const float b = 0.18f;
        return (std::tanh(x + b) - std::tanh(b)) * 0.92f;
    }
public:
    void setSampleRate(float s) { fs = (s > 0.f) ? s : 48000.f; }
    void reset() {
        ulLow.reset(); ulMid.reset(); ulHigh.reset(); uhShelf.reset();
        bqBass.reset(); bqMid.reset(); bqTreble.reset();
    }

    void setParams(float gain, float bass, float midrange, float freq,
                   float treble, float masterP,
                   bool pad, bool ultraLo, bool ultraHi) {
        // ── input / preamp drive (the SVT growl). -15 dB pad cuts ~5.6x ──
        const float padScale = pad ? 0.178f : 1.0f;
        inGain = (0.5f + gain * 3.5f) * padScale;     // ~0.5 .. 4.0
        inComp = 1.0f / (0.55f + 0.5f * inGain);      // keep level sane vs drive

        // ── Ultra Lo: boost deep lows + highs, scoop mids (loudness contour) ──
        if (ultraLo) {
            ulLow.setLowShelf(50.f, 6.0f, fs);
            ulMid.setPeak(500.f, -9.0f, 0.8f, fs);
            ulHigh.setHighShelf(8000.f, 4.0f, fs);
        } else { ulLow.setBypass(); ulMid.setBypass(); ulHigh.setBypass(); }

        // ── Ultra Hi: presence/high-shelf bite ──
        if (ultraHi) uhShelf.setHighShelf(4000.f, 6.0f, fs); else uhShelf.setBypass();

        // ── tone stack (±15 dB, 0.5 = flat) ──
        bqBass.setLowShelf(70.f, (bass - 0.5f) * 30.f, fs);
        // Frequency selector -> nearest of the 5 SVT midrange centres.
        int sel = (int)(freq * 5.0f); if (sel > 4) sel = 4; if (sel < 0) sel = 0;
        bqMid.setPeak(kSvtMidFreqs[sel], (midrange - 0.5f) * 30.f, 0.8f, fs);
        bqTreble.setHighShelf(5000.f, (treble - 0.5f) * 30.f, fs);

        master = masterP / 0.7f;     // unity ~ 0.7
    }

    inline float process(float x) {
        float s = tube(inGain * x) * inComp;   // 1-2. drive + growl
        s = ulLow.process(s); s = ulMid.process(s); s = ulHigh.process(s);  // 3
        s = uhShelf.process(s);                                             // 4
        s = bqBass.process(s); s = bqMid.process(s); s = bqTreble.process(s); // 5
        return s * master;                                                  // 6
    }
};

class SvtPlugin : public Plugin {
    SvtChannel L, R;
    float fParams[kParamCount];
    void recalc() {
        const bool pad = fParams[kPad] > 0.5f, ul = fParams[kUltraLo] > 0.5f, uh = fParams[kUltraHi] > 0.5f;
        L.setParams(fParams[kGain], fParams[kBass], fParams[kMidrange], fParams[kFreq],
                    fParams[kTreble], fParams[kMaster], pad, ul, uh);
        R.setParams(fParams[kGain], fParams[kBass], fParams[kMidrange], fParams[kFreq],
                    fParams[kTreble], fParams[kMaster], pad, ul, uh);
    }
public:
    SvtPlugin() : Plugin(kParamCount, 0, 0) {
        for (int i = 0; i < kParamCount; ++i) fParams[i] = kSvtDef[i];
        const float sr = (float)getSampleRate();
        L.setSampleRate(sr); R.setSampleRate(sr); L.reset(); R.reset(); recalc();
    }
protected:
    const char* getLabel()       const override { return "SamplegSBTCL"; }
    const char* getDescription() const override { return "Ampeg SVT-CL all-tube bass head model"; }
    const char* getMaker()       const override { return "RigBuilder"; }
    const char* getLicense()     const override { return "ISC"; }
    uint32_t    getVersion()     const override { return d_version(1, 0, 0); }
    int64_t     getUniqueId()    const override { return d_cconst('R', 'B', 'S', 'v'); }

    void initParameter(uint32_t i, Parameter& p) override {
        if (i >= (uint32_t)kParamCount) return;
        p.hints = kParameterIsAutomatable;
        if (i >= (uint32_t)kPad) p.hints |= kParameterIsBoolean;
        p.name = kSvtNames[i]; p.symbol = kSvtSymbols[i];
        p.ranges.min = kSvtMin[i]; p.ranges.max = kSvtMax[i]; p.ranges.def = kSvtDef[i];
    }
    float getParameterValue(uint32_t i) const override { return (i < (uint32_t)kParamCount) ? fParams[i] : 0.f; }
    void  setParameterValue(uint32_t i, float v) override { if (i < (uint32_t)kParamCount) { fParams[i] = v; recalc(); } }
    void  sampleRateChanged(double r) override { L.setSampleRate((float)r); R.setSampleRate((float)r); L.reset(); R.reset(); recalc(); }

    void run(const float** in, float** out, uint32_t frames) override {
        const float* iL = in[0]; const float* iR = in[1];
        float* oL = out[0]; float* oR = out[1];
        for (uint32_t i = 0; i < frames; ++i) { oL[i] = L.process(iL[i]); oR[i] = R.process(iR[i]); }
    }
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SvtPlugin)
};

Plugin* createPlugin() { return new SvtPlugin(); }

END_NAMESPACE_DISTRHO
