#ifndef TW22_CORE_H
#define TW22_CORE_H

/*
 * TW22Core - BENDER SUPERNOVA 22 / Fender Super-Sonic 22 (6V6) component model.
 *
 * White-box audio model (no SPICE), plain C++ so it can be unit-tested offline.
 *
 * Local reference:
 *   amps/Fender SuperSonic 22 (TW22)/Fender-Super-Sonic-22-Schematic.pdf
 *
 * Super-Sonic 22 topology modelled here:
 *   Two footswitchable channels morphed by Rocksmith's single Gain knob:
 *     - VINTAGE (low gain): blackface-style American clean — bright, TIGHT,
 *       mid-scooped, sparkly. Stays clean until pushed.
 *     - BURN (high gain): cascaded 12AX7 gain stages, tighter lows, more mids
 *       and saturation -> hot-rodded modern lead.
 *   12AT7 phase inverter -> 2x 6V6GT push-pull (~22 W) with a SOLID-STATE
 *   rectifier (mild sag) and a global NFB loop -> Celestion V30 12".
 *
 * Voicing target = the REAL Super-Sonic: brighter and tighter than an AC30 (it
 * is an American amp, NOT a fat British one). Rocksmith exposes Gain, Bass, Mid
 * and Treble — and EVERY knob stays audibly active across its FULL sweep (no
 * dead upper half).
 */

#include "TW22Params.h"
#include <cmath>

namespace tw22 {

static constexpr float kPi = 3.14159265359f;

static inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
static inline float clampFreq(float hz, float sr) { const float ny = sr * 0.45f; return std::fmax(10.0f, std::fmin(hz, ny)); }
static inline float smoothstep(float v) { v = clamp01(v); return v * v * (3.0f - 2.0f * v); }
static inline float smoothstepRange(float e0, float e1, float x) { return smoothstep((x - e0) / (e1 - e0)); }
static inline float softClip(float x) { return std::tanh(x); }
static inline float eqDb(float v, float rangeDb) { return (clamp01(v) - 0.5f) * 2.0f * rangeDb; }

// smooth (C-infinity) asymmetric tube curves — NO piecewise zero-crossing kink
static inline float triode12AX7(float x, float bias) {
    const float g = x + bias;
    const float warped = 1.55f * g + 0.34f * g * std::fabs(g);
    const float idle   = 1.55f * bias + 0.34f * bias * std::fabs(bias);
    return std::tanh(warped) - std::tanh(idle);
}
static inline float sixV6Pair(float x, float bias) {
    const float p = std::tanh(1.30f * (x + bias) + 0.05f * x * x);
    const float n = std::tanh(1.30f * (-x + bias) + 0.05f * x * x);
    const float idle = std::tanh(1.30f * bias);
    return 0.5f * ((p - idle) - (n - idle));
}

class RcHighPass {
    float a = 0.0f, x1 = 0.0f, y1 = 0.0f;
public:
    void reset() { x1 = y1 = 0.0f; }
    void setHz(float sr, float hz) { hz = clampFreq(hz, sr); const float tau = 1.0f / (2.0f * kPi * hz), dt = 1.0f / std::fmax(sr, 1000.0f); a = tau / (tau + dt); }
    float process(float x) { const float y = a * (y1 + x - x1); x1 = x; y1 = y; return y; }
};
class RcLowPass {
    float a = 1.0f, z = 0.0f;
public:
    void reset() { z = 0.0f; }
    void setHz(float sr, float hz) { hz = clampFreq(hz, sr); const float tau = 1.0f / (2.0f * kPi * hz), dt = 1.0f / std::fmax(sr, 1000.0f); a = dt / (tau + dt); }
    float process(float x) { z += a * (x - z); return z; }
};

class Biquad {
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f, z1 = 0.0f, z2 = 0.0f;
    void set(float nb0, float nb1, float nb2, float na0, float na1, float na2) {
        if (std::fabs(na0) < 1.0e-12f) na0 = 1.0f; const float k = 1.0f / na0;
        b0 = nb0 * k; b1 = nb1 * k; b2 = nb2 * k; a1 = na1 * k; a2 = na2 * k;
    }
public:
    void reset() { z1 = z2 = 0.0f; }
    float process(float x) { const float y = b0 * x + z1; z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y; return y; }
    void setLowPass(float sr, float hz, float q) { hz = clampFreq(hz, sr); const float w = 2 * kPi * hz / sr, c = std::cos(w), al = std::sin(w) / (2 * q);
        set((1 - c) * .5f, 1 - c, (1 - c) * .5f, 1 + al, -2 * c, 1 - al); }
    void setHighPass(float sr, float hz, float q) { hz = clampFreq(hz, sr); const float w = 2 * kPi * hz / sr, c = std::cos(w), al = std::sin(w) / (2 * q);
        set((1 + c) * .5f, -(1 + c), (1 + c) * .5f, 1 + al, -2 * c, 1 - al); }
    void setPeaking(float sr, float hz, float q, float dB) { hz = clampFreq(hz, sr); const float A = std::pow(10.f, dB / 40), w = 2 * kPi * hz / sr, c = std::cos(w), al = std::sin(w) / (2 * q);
        set(1 + al * A, -2 * c, 1 - al * A, 1 + al / A, -2 * c, 1 - al / A); }
    void setLowShelf(float sr, float hz, float slope, float dB) { hz = clampFreq(hz, sr); const float A = std::pow(10.f, dB / 40), w = 2 * kPi * hz / sr, c = std::cos(w), s = std::sin(w), rA = std::sqrt(A);
        const float al = s * .5f * std::sqrt((A + 1 / A) * (1 / slope - 1) + 2);
        set(A * ((A + 1) - (A - 1) * c + 2 * rA * al), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - 2 * rA * al),
            (A + 1) + (A - 1) * c + 2 * rA * al, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - 2 * rA * al); }
    void setHighShelf(float sr, float hz, float slope, float dB) { hz = clampFreq(hz, sr); const float A = std::pow(10.f, dB / 40), w = 2 * kPi * hz / sr, c = std::cos(w), s = std::sin(w), rA = std::sqrt(A);
        const float al = s * .5f * std::sqrt((A + 1 / A) * (1 / slope - 1) + 2);
        set(A * ((A + 1) + (A - 1) * c + 2 * rA * al), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - 2 * rA * al),
            (A + 1) - (A - 1) * c + 2 * rA * al, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - 2 * rA * al); }
};

class DcBlock {
    float x1 = 0.0f, y1 = 0.0f;
public:
    void reset() { x1 = y1 = 0.0f; }
    float process(float x) { const float y = x - x1 + 0.995f * y1; x1 = x; y1 = y; return y; }
};

class TW22Core {
    float sampleRate = 48000.0f;
    float gain = kTW22Def[kGain];
    float bass = kTW22Def[kBass];
    float mid = kTW22Def[kMid];
    float treble = kTW22Def[kTreble];

    RcHighPass inputHp;
    Biquad inputBright;     // Fender bright cap (clean sparkle)
    RcHighPass burnTighten; // tightens the low end into the cascaded Burn stages
    RcLowPass interstageLp;
    Biquad bassShelf;
    Biquad midScoop;        // Fender ~440 Hz scoop, filled by the Mid knob
    Biquad trebleShelf;
    Biquad presence;        // fixed-ish upper presence (bright American voice)
    Biquad speakerHp;
    Biquad speakerBody;     // modest low-mid (tight, NOT fat like an AC30)
    Biquad speakerBite;     // V30 ~2.6 kHz presence
    Biquad speakerFizz;     // V30 ~4.5 kHz dip
    Biquad speakerLp;       // top rolloff — opens with Treble
    DcBlock dcBlock;
    float sag = 0.0f;

    void updateFilters() {
        const float g    = smoothstep(gain);
        const float burn = smoothstepRange(0.38f, 0.95f, gain);   // WIDE morph: action across the whole knob
        const float hot  = smoothstepRange(0.66f, 1.0f, gain);

        inputHp.setHz(sampleRate, 55.0f + 70.0f * g + 60.0f * burn);
        // bright cap: lots of sparkle clean, eases as it drives. Treble adds.
        inputBright.setHighShelf(sampleRate, 2100.0f + 900.0f * treble, 0.70f, (-0.5f + 5.5f * treble) * (1.0f - 0.5f * g));

        burnTighten.setHz(sampleRate, 90.0f + 180.0f * burn + 70.0f * g);
        interstageLp.setHz(sampleRate, 9500.0f - 3200.0f * burn + 1400.0f * treble);

        // --- tone stack: each control spans a WIDE audible range to the max ---
        bassShelf.setLowShelf(sampleRate, 100.0f + 25.0f * bass, 0.74f, eqDb(bass, 11.0f));
        // Fender mid scoop (American voice): deep scoop at min, mild bump at max
        midScoop.setPeaking(sampleRate, 430.0f + 150.0f * mid, 0.64f, -8.0f + (12.5f + 2.5f * burn) * mid);
        // treble: keeps brightening right up to 5 o'clock (wide ±, no early clamp)
        trebleShelf.setHighShelf(sampleRate, 1900.0f + 1300.0f * treble, 0.62f, -7.0f + 17.0f * treble);
        presence.setHighShelf(sampleRate, 3200.0f, 0.80f, -1.5f + 5.0f * treble - 1.5f * burn);

        // --- Celestion V30 12" (bright / tight, modest body) ---
        speakerHp.setHighPass(sampleRate, 84.0f, 0.72f);
        speakerBody.setPeaking(sampleRate, 240.0f, 0.80f, 0.6f + 1.4f * bass - 0.4f * hot);
        speakerBite.setPeaking(sampleRate, 2600.0f + 500.0f * treble, 0.72f, 2.2f + 2.6f * treble + 0.8f * burn);
        speakerFizz.setPeaking(sampleRate, 4500.0f + 400.0f * treble, 0.95f, -3.0f - 2.0f * hot);
        // top rolloff OPENS with Treble across its whole sweep
        speakerLp.setLowPass(sampleRate, 4600.0f + 3200.0f * treble - 700.0f * burn, 0.64f);
    }

public:
    void reset() {
        inputHp.reset(); inputBright.reset(); burnTighten.reset(); interstageLp.reset();
        bassShelf.reset(); midScoop.reset(); trebleShelf.reset(); presence.reset();
        speakerHp.reset(); speakerBody.reset(); speakerBite.reset(); speakerFizz.reset(); speakerLp.reset();
        dcBlock.reset(); sag = 0.0f; updateFilters();
    }
    void setSampleRate(float sr) { sampleRate = sr > 1000.0f ? sr : 48000.0f; reset(); }
    void setGain(float v)   { gain = clamp01(v);   updateFilters(); }
    void setBass(float v)   { bass = clamp01(v);   updateFilters(); }
    void setMid(float v)    { mid = clamp01(v);    updateFilters(); }
    void setTreble(float v) { treble = clamp01(v); updateFilters(); }

    float process(float in) {
        const float g    = smoothstep(gain);
        const float burn = smoothstepRange(0.38f, 0.95f, gain);
        const float hot  = smoothstepRange(0.66f, 1.0f, gain);

        float x = inputHp.process(in);
        x = inputBright.process(x);
        x = softClip(x * (1.04f + 0.10f * burn)) * (0.95f - 0.05f * burn);

        // --- VINTAGE path: one 12AX7 stage, stays clean until pushed ---
        const float vintageDrive = 0.90f + 1.7f * gain + 0.8f * g;
        float vintage = triode12AX7(x * vintageDrive, 0.012f + 0.018f * gain);
        const float cleanBlend = 0.62f * (1.0f - g);
        vintage = vintage * (1.0f - cleanBlend) + x * cleanBlend;

        // --- BURN path: cascaded gain stages; keeps adding grit to the max ---
        float burnPath = burnTighten.process(x);
        const float gain1 = 1.7f + 5.5f * gain + 6.5f * g;       // rises across the whole sweep
        burnPath = triode12AX7(burnPath * gain1, 0.026f + 0.030f * gain);
        burnPath = interstageLp.process(burnPath);
        const float gain2 = 1.4f + 3.0f * gain + 4.5f * hot;     // top of the knob keeps biting
        burnPath = triode12AX7(burnPath * gain2, -0.018f - 0.026f * gain);

        float y = vintage * (1.0f - burn) + burnPath * burn;

        // --- tone stack ---
        y = bassShelf.process(y);
        y = midScoop.process(y);
        y = trebleShelf.process(y);
        y = presence.process(y);

        // --- 6V6 push-pull + mild solid-state-rectifier sag ---
        const float env = std::fabs(y);
        const float atk = 1.0f - std::exp(-1.0f / (0.0040f * sampleRate));
        const float rel = 1.0f - std::exp(-1.0f / (0.090f * sampleRate));
        sag += (env - sag) * (env > sag ? atk : rel);
        const float sagDrop = 1.0f / (1.0f + sag * (0.28f + 0.55f * gain));
        const float powerDrive = (1.0f + 1.3f * gain + 1.7f * hot) * sagDrop;
        y = sixV6Pair(y * powerDrive, 0.03f + 0.012f * (treble - bass));
        y = 0.90f * y + 0.10f * softClip(y * (1.5f + 1.0f * gain));

        y = dcBlock.process(y);

        // --- Celestion V30 ---
        y = speakerHp.process(y);
        y = speakerBody.process(y);
        y = speakerBite.process(y);
        y = speakerFizz.process(y);
        y = speakerLp.process(y);

        // --- output makeup: flat loudness at the BOX DC30 reference (~0.40) ---
        const float toneEnergy = 1.0f
            + 0.013f * std::fabs((bass - 0.5f) * 20.0f)
            + 0.012f * std::fabs((mid - 0.5f) * 22.0f)
            + 0.014f * std::fabs((treble - 0.5f) * 22.0f);
        const float makeup = 0.50f + 1.6f / (1.0f + std::exp(8.0f * (gain - 0.42f)));
        const float level = makeup / toneEnergy;
        return softClip(y * level) * 0.98f;
    }
};

} // namespace tw22

#endif // TW22_CORE_H
