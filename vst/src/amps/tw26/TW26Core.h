#ifndef TW26_CORE_H
#define TW26_CORE_H

/*
 * TW26Core - BENDER DELUXE / Fender '57 Deluxe (5E3 tweed) component model.
 *
 * White-box audio model: each audible block is driven by the 5E3 schematic
 * component values rather than a literal SPICE solve.
 *
 * Local reference:
 *   amps/Fender Deluxe (TW26)/Fender-57-Deluxe-Schematic.pdf
 *
 * 5E3 topology modelled here:
 *   Input 68k grid -> V1 12AY7 first stage (low-mu ~44 = warm, early, soft) ->
 *   interactive Volume -> single tweed Tone network (1M pot, C5 .0047uF,
 *   C4 500pF) -> V2-A 12AX7 recovery gain -> V2-B 12AX7 cathodyne (split-load)
 *   phase inverter -> 2x 6V6GT push-pull, CATHODE-biased (R23 250R), NO global
 *   NFB -> 5Y3GT tube rectifier (heavy, blooming sag) -> 1x12 tweed speaker.
 *
 * The 5E3 is mid-forward and woody (NOT scooped like a blackface), compresses
 * and "blooms" early because of the tube rectifier + cathode bias, and breaks
 * up into a loose tweed grind when pushed.
 *
 * Rocksmith exposes Gain, Bass, Mid, Treble, Pres. The real amp has only two
 * Volumes + one Tone, so: Gain drives clean->tweed-breakup; Bass/Mid/Treble are
 * a tweed-voiced 3-band expansion of the single Tone control (kept mid-rich);
 * Pres is a gentle top-end lift (the 5E3 has no presence pot).
 */

#include "TW26Params.h"
#include <cmath>

namespace tw26 {

static constexpr float kPi = 3.14159265359f;
static constexpr float kEpsilon = 1.0e-9f;

static inline float clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static inline float clampFreq(float hz, float sr)
{
    const float nyquist = sr * 0.45f;
    return std::fmax(10.0f, std::fmin(hz, nyquist));
}

static inline float smoothstep(float v)
{
    v = clamp01(v);
    return v * v * (3.0f - 2.0f * v);
}

static inline float smoothstepRange(float edge0, float edge1, float x)
{
    return smoothstep((x - edge0) / (edge1 - edge0));
}

static inline float softClip(float x)
{
    return std::tanh(x);
}

// --- smooth (C-infinity) tube transfer curves: asymmetric for even harmonics,
//     NO piecewise zero-crossing kink (that injects buzz even on clean tones).
static inline float triode12AY7(float x, float bias)
{
    // 12AY7: lower mu than a 12AX7 -> warmer, earlier, softer. Used at V1.
    const float g = x + bias;
    const float warped = 1.28f * g + 0.30f * g * std::fabs(g);
    const float idle   = 1.28f * bias + 0.30f * bias * std::fabs(bias);
    return std::tanh(warped) - std::tanh(idle);
}

static inline float triode12AX7(float x, float bias)
{
    const float g = x + bias;
    const float warped = 1.55f * g + 0.34f * g * std::fabs(g);
    const float idle   = 1.55f * bias + 0.34f * bias * std::fabs(bias);
    return std::tanh(warped) - std::tanh(idle);
}

static inline float sixV6Pair(float x, float bias)
{
    // Cathode-biased push-pull pair (smooth, symmetric-ish with a touch of even
    // harmonic from the shared-cathode bias shift).
    const float p = std::tanh(1.28f * (x + bias) + 0.05f * x * x);
    const float n = std::tanh(1.28f * (-x + bias) + 0.05f * x * x);
    const float idle = std::tanh(1.28f * bias);
    return 0.5f * ((p - idle) - (n - idle));
}

class RcHighPass
{
    float a = 0.0f, x1 = 0.0f, y1 = 0.0f;
public:
    void reset() { x1 = y1 = 0.0f; }
    void setRC(float sr, float resistanceOhm, float capacitanceF)
    {
        const float dt = 1.0f / std::fmax(sr, 1000.0f);
        const float tau = std::fmax(resistanceOhm * capacitanceF, kEpsilon);
        a = tau / (tau + dt);
    }
    void setHz(float sr, float hz)
    {
        hz = clampFreq(hz, sr);
        const float tau = 1.0f / (2.0f * kPi * hz);
        const float dt = 1.0f / std::fmax(sr, 1000.0f);
        a = tau / (tau + dt);
    }
    float process(float x)
    {
        const float y = a * (y1 + x - x1);
        x1 = x; y1 = y;
        return y;
    }
};

class RcLowPass
{
    float a = 1.0f, z = 0.0f;
public:
    void reset() { z = 0.0f; }
    void setRC(float sr, float resistanceOhm, float capacitanceF)
    {
        const float dt = 1.0f / std::fmax(sr, 1000.0f);
        const float tau = std::fmax(resistanceOhm * capacitanceF, kEpsilon);
        a = dt / (tau + dt);
    }
    void setHz(float sr, float hz)
    {
        hz = clampFreq(hz, sr);
        const float tau = 1.0f / (2.0f * kPi * hz);
        const float dt = 1.0f / std::fmax(sr, 1000.0f);
        a = dt / (tau + dt);
    }
    float process(float x) { z += a * (x - z); return z; }
};

class Biquad
{
    float b0 = 1.0f, b1 = 0.0f, b2 = 0.0f, a1 = 0.0f, a2 = 0.0f, z1 = 0.0f, z2 = 0.0f;
    void set(float nb0, float nb1, float nb2, float na0, float na1, float na2)
    {
        if (std::fabs(na0) < 1.0e-12f) na0 = 1.0f;
        const float invA0 = 1.0f / na0;
        b0 = nb0 * invA0; b1 = nb1 * invA0; b2 = nb2 * invA0;
        a1 = na1 * invA0; a2 = na2 * invA0;
    }
public:
    void reset() { z1 = z2 = 0.0f; }
    float process(float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void setLowPass(float sr, float hz, float q)
    {
        hz = clampFreq(hz, sr);
        const float w0 = 2.0f * kPi * hz / sr, c = std::cos(w0), alpha = std::sin(w0) / (2.0f * q);
        set((1.0f - c) * 0.5f, 1.0f - c, (1.0f - c) * 0.5f, 1.0f + alpha, -2.0f * c, 1.0f - alpha);
    }
    void setHighPass(float sr, float hz, float q)
    {
        hz = clampFreq(hz, sr);
        const float w0 = 2.0f * kPi * hz / sr, c = std::cos(w0), alpha = std::sin(w0) / (2.0f * q);
        set((1.0f + c) * 0.5f, -(1.0f + c), (1.0f + c) * 0.5f, 1.0f + alpha, -2.0f * c, 1.0f - alpha);
    }
    void setPeaking(float sr, float hz, float q, float gainDb)
    {
        hz = clampFreq(hz, sr);
        const float a = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * kPi * hz / sr, c = std::cos(w0), alpha = std::sin(w0) / (2.0f * q);
        set(1.0f + alpha * a, -2.0f * c, 1.0f - alpha * a, 1.0f + alpha / a, -2.0f * c, 1.0f - alpha / a);
    }
    void setLowShelf(float sr, float hz, float slope, float gainDb)
    {
        hz = clampFreq(hz, sr);
        const float a = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * kPi * hz / sr, c = std::cos(w0), s = std::sin(w0), rootA = std::sqrt(a);
        const float alpha = s * 0.5f * std::sqrt((a + 1.0f / a) * (1.0f / slope - 1.0f) + 2.0f);
        set(a * ((a + 1.0f) - (a - 1.0f) * c + 2.0f * rootA * alpha),
            2.0f * a * ((a - 1.0f) - (a + 1.0f) * c),
            a * ((a + 1.0f) - (a - 1.0f) * c - 2.0f * rootA * alpha),
            (a + 1.0f) + (a - 1.0f) * c + 2.0f * rootA * alpha,
            -2.0f * ((a - 1.0f) + (a + 1.0f) * c),
            (a + 1.0f) + (a - 1.0f) * c - 2.0f * rootA * alpha);
    }
    void setHighShelf(float sr, float hz, float slope, float gainDb)
    {
        hz = clampFreq(hz, sr);
        const float a = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * kPi * hz / sr, c = std::cos(w0), s = std::sin(w0), rootA = std::sqrt(a);
        const float alpha = s * 0.5f * std::sqrt((a + 1.0f / a) * (1.0f / slope - 1.0f) + 2.0f);
        set(a * ((a + 1.0f) + (a - 1.0f) * c + 2.0f * rootA * alpha),
            -2.0f * a * ((a - 1.0f) + (a + 1.0f) * c),
            a * ((a + 1.0f) + (a - 1.0f) * c - 2.0f * rootA * alpha),
            (a + 1.0f) - (a - 1.0f) * c + 2.0f * rootA * alpha,
            2.0f * ((a - 1.0f) - (a + 1.0f) * c),
            (a + 1.0f) - (a - 1.0f) * c - 2.0f * rootA * alpha);
    }
};

class DcBlock
{
    float x1 = 0.0f, y1 = 0.0f;
public:
    void reset() { x1 = y1 = 0.0f; }
    float process(float x) { const float y = x - x1 + 0.995f * y1; x1 = x; y1 = y; return y; }
};

// 5Y3GT tube rectifier + cathode-biased 6V6 supply: deep, slow, blooming sag —
// the heart of the 5E3 "feel". Returns a 0..1 supply scale.
class Y3CathodeBiasedSupply
{
    float sr = 48000.0f, rectSag = 0.0f, cathodeRise = 0.0f;
    float sagAttack = 0.0f, sagRelease = 0.0f, cathodeCoeff = 0.0f;
public:
    void reset() { rectSag = 0.0f; cathodeRise = 0.0f; }
    void setSampleRate(float sampleRate)
    {
        sr = sampleRate > 1000.0f ? sampleRate : 48000.0f;
        // 5Y3 has high internal resistance + small filter caps -> quick dip,
        // slow bloom recovery. Slower/deeper than a solid-state rectifier.
        sagAttack = 1.0f - std::exp(-1.0f / (0.009f * sr));
        sagRelease = 1.0f - std::exp(-1.0f / (0.220f * sr));
        // 6V6 shared cathode R23 250R with 25u -> ~6 ms bias shift.
        const float tau = 250.0f * 25.0e-6f;
        cathodeCoeff = 1.0f - std::exp(-1.0f / (std::fmax(tau, 0.001f) * sr));
    }
    float process(float env, float gain, float hot)
    {
        rectSag += (env - rectSag) * (env > rectSag ? sagAttack : sagRelease);
        cathodeRise += (env - cathodeRise) * cathodeCoeff;
        const float bPlus = 1.0f / (1.0f + rectSag * (0.55f + 1.25f * gain + 0.55f * hot));
        const float cathodeBias = 1.0f / (1.0f + cathodeRise * (0.30f + 0.50f * gain));
        return bPlus * cathodeBias;
    }
};

class TW26Core
{
    float sampleRate = 48000.0f;
    float gain = kTW26Def[kGain];
    float bass = kTW26Def[kBass];
    float mid = kTW26Def[kMid];
    float treble = kTW26Def[kTreble];
    float pres = kTW26Def[kPres];

    RcLowPass inputGrid;        // 68k grid into V1 Miller capacitance
    RcHighPass v1Coupling;      // C2 .1uF into the volume/tone
    RcLowPass v1Miller;
    RcHighPass toneTrebleBleed; // C5 .0047uF treble path of the single Tone pot
    RcHighPass v2Coupling;      // C7 .022uF into V2-A
    Biquad bassShelf;
    Biquad tweedBody;           // fixed woody low-mid hump (tweed is mid-forward)
    Biquad midPeak;             // Rocksmith Mid
    Biquad trebleShelf;         // Rocksmith Treble (tweed Tone control voicing)
    Biquad presenceShelf;       // Rocksmith Pres (the 5E3 has no presence pot)
    RcHighPass piCoupling;      // C9/C10 to the 6V6 grids
    Y3CathodeBiasedSupply supply;
    Biquad transformerLow;      // output transformer low resonance
    Biquad speakerHp;
    Biquad speakerBody;         // 1x12 tweed cone low-mid bump
    Biquad speakerPresence;     // gentle upper-mid (tweed alnico, softer than V30)
    Biquad speakerLp;           // tweed top rolloff (darker than a modern cab)
    DcBlock dcBlock;

    static float eqDb(float v, float rangeDb) { return (clamp01(v) - 0.5f) * 2.0f * rangeDb; }

    void updateComponentValues()
    {
        const float g = smoothstep(gain);
        const float hot = smoothstepRange(0.50f, 0.98f, gain);

        inputGrid.setRC(sampleRate, 68000.0f, 50.0e-12f);     // ~47 kHz ceiling
        v1Coupling.setRC(sampleRate, 1000000.0f, 0.1e-6f);    // C2 .1uF / 1M -> ~1.6 Hz
        v1Miller.setRC(sampleRate, 100000.0f, 90.0e-12f);     // plate/Miller rolloff

        // Single tweed Tone control (treble bleed). Its audible corner lands in
        // the presence band; the cap sees the network's effective R, not the raw
        // 1M pot (that would push the corner sub-audio and kill the control).
        toneTrebleBleed.setRC(sampleRate, 1.0f / (2.0f * kPi * (2200.0f + 1800.0f * treble) * 0.0047e-6f), 0.0047e-6f);
        v2Coupling.setRC(sampleRate, 470000.0f, 0.022e-6f);   // C7 .022uF -> ~15 Hz

        // --- tweed tone shaping (mid-forward, NEVER scooped) ---
        // more low-end weight so the amp has real body (was too thin -> boxy)
        bassShelf.setLowShelf(sampleRate, 115.0f, 0.72f, eqDb(bass, 9.5f) + 2.4f);
        // woody low-mid hump — eased a touch so it's full, not honky/boxy
        tweedBody.setPeaking(sampleRate, 340.0f, 0.85f, 1.2f + 1.1f * g);
        // Rocksmith Mid sits on the tweed midrange — boosts the bark, only a mild
        // dip at minimum (tweed does not scoop hard like a blackface).
        midPeak.setPeaking(sampleRate, 620.0f + 220.0f * mid, 0.72f, -2.5f + 9.0f * mid);
        trebleShelf.setHighShelf(sampleRate, 2300.0f + 900.0f * treble, 0.70f, eqDb(treble, 10.5f) + 1.0f);
        // Pres: gentle top lift (no real presence pot on a 5E3)
        presenceShelf.setHighShelf(sampleRate, 3000.0f, 0.80f, -1.0f + 5.5f * pres);

        piCoupling.setRC(sampleRate, 220000.0f, 0.1e-6f);     // ~7 Hz into the 6V6 grids
        supply.setSampleRate(sampleRate);

        // --- output transformer + 1x12 tweed speaker (Jensen-style, warm) ---
        transformerLow.setPeaking(sampleRate, 95.0f, 0.72f, 0.6f + 1.8f * bass);   // low thump
        speakerHp.setHighPass(sampleRate, 70.0f, 0.72f);                          // let the lows through
        // body/warmth moved down to ~175 Hz and lifted so the amp feels full
        speakerBody.setPeaking(sampleRate, 175.0f, 0.80f, 2.6f + 1.8f * bass - 0.5f * hot);
        // upper-mid clarity lifted so it isn't "muffled"
        speakerPresence.setPeaking(sampleRate, 2150.0f + 350.0f * treble, 0.72f, 2.4f + 2.4f * treble + 2.4f * pres);
        // tweed is darker than a modern cab, but the top was over-rolled -> open it
        speakerLp.setLowPass(sampleRate, 5500.0f + 2000.0f * pres + 900.0f * treble, 0.66f);
    }

public:
    void reset()
    {
        inputGrid.reset(); v1Coupling.reset(); v1Miller.reset();
        toneTrebleBleed.reset(); v2Coupling.reset();
        bassShelf.reset(); tweedBody.reset(); midPeak.reset(); trebleShelf.reset(); presenceShelf.reset();
        piCoupling.reset(); supply.reset();
        transformerLow.reset(); speakerHp.reset(); speakerBody.reset(); speakerPresence.reset(); speakerLp.reset();
        dcBlock.reset();
        updateComponentValues();
    }

    void setSampleRate(float sr) { sampleRate = sr > 1000.0f ? sr : 48000.0f; reset(); }

    void setGain(float v)   { gain = clamp01(v);   updateComponentValues(); }
    void setBass(float v)   { bass = clamp01(v);   updateComponentValues(); }
    void setMid(float v)    { mid = clamp01(v);    updateComponentValues(); }
    void setTreble(float v) { treble = clamp01(v); updateComponentValues(); }
    void setPres(float v)   { pres = clamp01(v);   updateComponentValues(); }

    float process(float in)
    {
        const float g = smoothstep(gain);
        const float hot = smoothstepRange(0.50f, 0.98f, gain);

        float x = inputGrid.process(in);

        // V1 12AY7 first stage — warm, low-mu, soft early compression.
        float y = v1Coupling.process(x);
        const float v1Drive = 0.80f + 1.55f * gain + 0.55f * g;
        y = triode12AY7(y * v1Drive, -0.020f - 0.020f * gain);
        y = v1Miller.process(y);

        // interactive Volume + single tweed Tone control (treble bleed blended in)
        const float volume = 0.30f + 1.35f * g + 0.55f * hot;
        y = (y + toneTrebleBleed.process(y) * (0.18f + 0.42f * treble)) * volume;

        // --- tweed tone stack (mid-forward) ---
        y = bassShelf.process(y);
        y = tweedBody.process(y);
        y = midPeak.process(y);
        y = trebleShelf.process(y);

        // V2-A 12AX7 recovery/gain stage
        y = v2Coupling.process(y);
        const float v2Drive = 0.85f + 1.65f * gain + 1.55f * hot;
        y = triode12AX7(y * v2Drive, -0.012f - 0.018f * gain);

        // --- V2-B cathodyne PI + 6V6 push-pull, cathode-biased, no NFB,
        //     5Y3 blooming sag ---
        y = piCoupling.process(y);
        const float env = std::fabs(y);
        const float supplyScale = supply.process(env, gain, hot);
        const float powerDrive = (1.02f + 1.40f * gain + 2.10f * hot) * supplyScale;
        y = sixV6Pair(y * powerDrive, 0.03f + 0.012f * (treble - bass));
        y = (0.92f * y + 0.08f * softClip(y * (1.5f + 0.9f * gain))) * supplyScale;

        y = dcBlock.process(y);

        // --- output transformer + 1x12 tweed speaker ---
        y = transformerLow.process(y);
        y = speakerHp.process(y);
        y = speakerBody.process(y);
        y = speakerPresence.process(y);
        y = speakerLp.process(y);

        // output makeup: hold perceived level ~constant across the Gain sweep so
        // it sits at the BOX DC30 level and presets don't jump (the tweed adds
        // level as it breaks up; this gently trims it back). Tone-energy term
        // keeps big EQ moves from changing loudness.
        const float toneEnergy = 1.0f
            + 0.013f * std::fabs((bass - 0.5f) * 20.0f)
            + 0.013f * std::fabs((mid - 0.5f) * 22.0f)
            + 0.013f * std::fabs((treble - 0.5f) * 20.0f);
        // The heavy 5Y3/cathode-bias sag pulls the steady level DOWN as Gain
        // rises (like the AC30), so the makeup RISES with Gain to hold the
        // perceived loudness ~constant at the BOX DC30 level while keeping the
        // sag bloom/compression dynamics intact.
        const float makeup = 1.50f + 0.40f * g + 0.70f * hot;
        const float level = makeup / toneEnergy;
        return softClip(y * level) * 0.98f;
    }
};

} // namespace tw26

#endif // TW26_CORE_H
