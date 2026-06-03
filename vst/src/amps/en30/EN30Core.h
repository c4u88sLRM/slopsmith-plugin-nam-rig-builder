#ifndef EN30_CORE_H
#define EN30_CORE_H

/*
 * EN30Core - BOX DC30 / Vox AC30 Top Boost component model.
 *
 * This is not a full SPICE/MNA netlist solver. It is a white-box audio model
 * where each audible block is driven by the component values in the local AC30
 * schematics: RC coupling caps, grid leaks, cathode bypass caps, 12AX7/ECC83
 * stages, Top Boost passive network, PI cut network, EL84 cathode bias, GZ34
 * supply sag, output transformer and 2x12 speaker filtering.
 *
 * Local references:
 *   amps/vox ac30 (en30)/ac30-60-02-iss5.pdf
 *   amps/vox ac30 (en30)/Vox_ac30cc2_ac30cc2x_2005_sm.pdf
 *   amps/vox ac30 (en30)/Vox_ac30c2.pdf
 *
 * Rocksmith exposes Gain, Bass, Mid, Treble, Pres, Bright. The AC30 has no Mid
 * pot, so Mid controls how much of the Top Boost scoop is filled back in.
 * Pres is inverse Cut: higher Pres means the 4n7/A220K cut network is opened.
 */

#include "EN30Params.h"
#include <cmath>

namespace en30 {

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

static inline float dbToAmp(float db)
{
    return std::pow(10.0f, db / 20.0f);
}

class RcHighPass
{
    float a = 0.0f;
    float x1 = 0.0f;
    float y1 = 0.0f;

public:
    void reset()
    {
        x1 = y1 = 0.0f;
    }

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
        x1 = x;
        y1 = y;
        return y;
    }
};

class RcLowPass
{
    float a = 1.0f;
    float z = 0.0f;

public:
    void reset()
    {
        z = 0.0f;
    }

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

    float process(float x)
    {
        z += a * (x - z);
        return z;
    }
};

class Biquad
{
    float b0 = 1.0f;
    float b1 = 0.0f;
    float b2 = 0.0f;
    float a1 = 0.0f;
    float a2 = 0.0f;
    float z1 = 0.0f;
    float z2 = 0.0f;

    void set(float nb0, float nb1, float nb2, float na0, float na1, float na2)
    {
        if (std::fabs(na0) < 1.0e-12f)
            na0 = 1.0f;
        const float invA0 = 1.0f / na0;
        b0 = nb0 * invA0;
        b1 = nb1 * invA0;
        b2 = nb2 * invA0;
        a1 = na1 * invA0;
        a2 = na2 * invA0;
    }

public:
    void reset()
    {
        z1 = z2 = 0.0f;
    }

    float process(float x)
    {
        const float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void setHighPass(float sr, float hz, float q)
    {
        hz = clampFreq(hz, sr);
        const float w0 = 2.0f * kPi * hz / sr;
        const float c = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * q);
        set((1.0f + c) * 0.5f, -(1.0f + c), (1.0f + c) * 0.5f,
            1.0f + alpha, -2.0f * c, 1.0f - alpha);
    }

    void setLowPass(float sr, float hz, float q)
    {
        hz = clampFreq(hz, sr);
        const float w0 = 2.0f * kPi * hz / sr;
        const float c = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * q);
        set((1.0f - c) * 0.5f, 1.0f - c, (1.0f - c) * 0.5f,
            1.0f + alpha, -2.0f * c, 1.0f - alpha);
    }

    void setPeaking(float sr, float hz, float q, float gainDb)
    {
        hz = clampFreq(hz, sr);
        const float a = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * kPi * hz / sr;
        const float c = std::cos(w0);
        const float alpha = std::sin(w0) / (2.0f * q);
        set(1.0f + alpha * a, -2.0f * c, 1.0f - alpha * a,
            1.0f + alpha / a, -2.0f * c, 1.0f - alpha / a);
    }

    void setHighShelf(float sr, float hz, float slope, float gainDb)
    {
        hz = clampFreq(hz, sr);
        const float a = std::pow(10.0f, gainDb / 40.0f);
        const float w0 = 2.0f * kPi * hz / sr;
        const float c = std::cos(w0);
        const float s = std::sin(w0);
        const float rootA = std::sqrt(a);
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
    float x1 = 0.0f;
    float y1 = 0.0f;

public:
    void reset()
    {
        x1 = y1 = 0.0f;
    }

    float process(float x)
    {
        const float y = x - x1 + 0.995f * y1;
        x1 = x;
        y1 = y;
        return y;
    }
};

static inline float triode12AX7(float x, float bias)
{
    // Smooth approximation of a biased 12AX7 transfer curve: asymmetric,
    // softer near cutoff, steeper when grid drive goes positive.
    const float g = x + bias;
    const float warped = 1.55f * g + 0.34f * g * std::fabs(g);
    const float idle = 1.55f * bias + 0.34f * bias * std::fabs(bias);
    return std::tanh(warped) - std::tanh(idle);
}

static inline float el84PentodePair(float x, float bias)
{
    const float p = std::tanh(1.35f * (x + bias) + 0.08f * x * x);
    const float n = std::tanh(1.35f * (-x + bias) + 0.08f * x * x);
    const float idle = std::tanh(1.35f * bias);
    return 0.5f * ((p - idle) - (n - idle));
}

class CathodeBypass
{
    RcHighPass capPath;
    float unbypassedGain = 0.58f;
    float bypassBoost = 0.42f;
    float compression = 0.18f;
    float env = 0.0f;
    float attack = 0.0f;
    float release = 0.0f;

public:
    void reset()
    {
        capPath.reset();
        env = 0.0f;
    }

    void set(float sr, float rkOhm, float ckF, float baseGain,
             float boostGain, float compressionAmount)
    {
        capPath.setRC(sr, rkOhm, ckF);
        unbypassedGain = baseGain;
        bypassBoost = boostGain;
        compression = compressionAmount;
        attack = 1.0f - std::exp(-1.0f / (0.006f * sr));
        release = 1.0f - std::exp(-1.0f / (0.075f * sr));
    }

    float process(float x)
    {
        const float level = std::fabs(x);
        env += (level - env) * (level > env ? attack : release);
        const float cathodeDegeneration = 1.0f / (1.0f + env * compression);
        return (x * unbypassedGain + capPath.process(x) * bypassBoost) * cathodeDegeneration;
    }
};

class TriodeStage
{
    RcHighPass couplingCap;
    RcLowPass gridStopperPole;
    CathodeBypass cathode;
    float plateGain = 1.0f;
    float baseDrive = 1.0f;
    float bias = -0.08f;
    float outputGain = 1.0f;

public:
    void reset()
    {
        couplingCap.reset();
        gridStopperPole.reset();
        cathode.reset();
    }

    void set(float sr,
             float couplingF, float nextGridLeakOhm,
             float gridStopperOhm, float millerCapF,
             float cathodeOhm, float cathodeCapF,
             float plateOhm, float drive, float biasV,
             float outGain, float unbypassedGain, float bypassBoost)
    {
        couplingCap.setRC(sr, nextGridLeakOhm, couplingF);
        gridStopperPole.setRC(sr, gridStopperOhm, millerCapF);
        cathode.set(sr, cathodeOhm, cathodeCapF, unbypassedGain, bypassBoost, 0.12f);
        plateGain = (100.0f * plateOhm / (plateOhm + 62500.0f)) / 62.0f;
        baseDrive = drive;
        bias = biasV;
        outputGain = outGain;
    }

    float process(float x, float driveKnob, float supplyScale)
    {
        float v = couplingCap.process(x);
        v = gridStopperPole.process(v);
        v = cathode.process(v);
        const float headroom = 0.78f + 0.38f * supplyScale;
        const float drive = baseDrive * (0.70f + 1.15f * driveKnob);
        return triode12AX7(v * drive / headroom, bias) * headroom * plateGain * outputGain;
    }
};

class CathodeFollower
{
    RcLowPass gridStopperPole;
    float gridCurrent = 0.0f;
    float attack = 0.0f;
    float release = 0.0f;

public:
    void reset()
    {
        gridStopperPole.reset();
        gridCurrent = 0.0f;
    }

    void set(float sr)
    {
        // R8 56k and stray/Miller capacitance before the follower.
        gridStopperPole.setRC(sr, 56000.0f, 95.0e-12f);
        attack = 1.0f - std::exp(-1.0f / (0.0025f * sr));
        release = 1.0f - std::exp(-1.0f / (0.050f * sr));
    }

    float process(float x)
    {
        float g = gridStopperPole.process(x);
        const float positiveGrid = std::fmax(0.0f, g - 0.42f);
        gridCurrent += (positiveGrid - gridCurrent) * (positiveGrid > gridCurrent ? attack : release);
        const float loaded = g / (1.0f + 0.55f * gridCurrent);
        return 0.86f * loaded + 0.14f * softClip(loaded * 1.35f);
    }
};

class TopBoostToneStack
{
    RcLowPass c5Bass22n;
    RcHighPass c4Treble22n;
    RcHighPass c6Treble47p;
    Biquad interactionScoop;
    Biquad midFill;
    Biquad postTrebleShelf;
    float bassMix = 1.0f;
    float midMix = 0.55f;
    float trebleMix = 1.0f;
    float airMix = 0.35f;
    float insertionLoss = 0.36f;

public:
    void reset()
    {
        c5Bass22n.reset();
        c4Treble22n.reset();
        c6Treble47p.reset();
        interactionScoop.reset();
        midFill.reset();
        postTrebleShelf.reset();
    }

    void set(float sr, float bass, float mid, float treble, float bright)
    {
        // AC30 Top Boost: C5/C4 22n, C6 47p, R5/R6 220k, Bass/Treble A1M.
        const float pot = 1000000.0f;
        const float slope = 220000.0f;
        // C5 (Bass, 22n) sets a LOW-shelf corner. The cap does NOT see the raw 1M
        // pot — in the cathode-follower-driven network its effective (Thevenin)
        // resistance is far lower, so the audible bass corner lands in the
        // ~120-420 Hz region (it was previously computed from the raw pot value,
        // which pushed the corner to ~6-24 Hz and made the Bass knob nearly inert).
        const float bassFc = 120.0f + 300.0f * bass;          // audible bass corner
        const float rb = 1.0f / (2.0f * kPi * bassFc * 22.0e-9f);
        const float rt = slope + pot * (0.06f + 0.94f * (1.0f - treble));
        const float rAir = slope + pot * (0.03f + 0.97f * (1.0f - treble));

        c5Bass22n.setRC(sr, rb, 22.0e-9f);
        c4Treble22n.setRC(sr, rt, 22.0e-9f);
        c6Treble47p.setRC(sr, rAir, 47.0e-12f);

        const float bothUp = bass * treble;
        const float bothDown = (1.0f - bass) * (1.0f - treble);
        const float crossLoad = bass * (1.0f - treble) + treble * (1.0f - bass);

        interactionScoop.setPeaking(sr, 520.0f + 90.0f * crossLoad, 0.66f,
                                    -11.0f * bothUp + 2.6f * bothDown - 1.2f * crossLoad);
        midFill.setPeaking(sr, 680.0f + 240.0f * mid, 0.68f, -4.6f + 9.2f * mid);
        postTrebleShelf.setHighShelf(sr, 2100.0f + 900.0f * treble, 0.70f,
                                     (treble - 0.5f) * 18.5f + 2.6f * bright);

        bassMix = 0.42f + 2.30f * bass;
        midMix = 0.50f + 0.55f * mid - 0.18f * bothUp;
        trebleMix = 0.18f + 1.40f * treble;
        airMix = (0.08f + 0.85f * treble) * (bright >= 0.5f ? 1.0f : 0.42f);
        insertionLoss = 0.31f + 0.10f * mid - 0.035f * bothUp;
    }

    float process(float x)
    {
        const float low = c5Bass22n.process(x) * bassMix;
        const float high = c4Treble22n.process(x) * trebleMix;
        const float air = c6Treble47p.process(x) * airMix;
        const float mid = x * midMix;
        float y = (low + mid + high + air) * insertionLoss;
        y = interactionScoop.process(y);
        y = midFill.process(y);
        y = postTrebleShelf.process(y);
        return y;
    }
};

class CutNetwork
{
    RcHighPass c1c2Cut4n7;
    float amount = 0.0f;

public:
    void reset()
    {
        c1c2Cut4n7.reset();
    }

    void set(float sr, float pres)
    {
        // VR1 A220k and C1/C2 4n7 cancel highs between PI plates. Wider/deeper
        // so the Cut (Rocksmith Pres) is clearly audible (it was too subtle once
        // the power amp regenerated highs): corner from ~1.5 kHz (full cut) up,
        // with a stronger removal depth.
        const float rCut = 22000.0f + 185000.0f * pres;
        c1c2Cut4n7.setRC(sr, rCut, 4.7e-9f);
        amount = 0.85f * (1.0f - pres);
    }

    float process(float x)
    {
        return x - amount * c1c2Cut4n7.process(x);
    }
};

class Gz34CathodeBiasedSupply
{
    float sr = 48000.0f;
    float rectifierSag = 0.0f;
    float cathodeRise = 0.0f;
    float sagAttack = 0.0f;
    float sagRelease = 0.0f;
    float cathodeCoeff = 0.0f;

public:
    void reset()
    {
        rectifierSag = 0.0f;
        cathodeRise = 0.0f;
    }

    void setSampleRate(float sampleRate)
    {
        sr = sampleRate > 1000.0f ? sampleRate : 48000.0f;
        // GZ34 + 22u/47u filter string: quick dip, slower recovery.
        sagAttack = 1.0f - std::exp(-1.0f / (0.010f * sr));
        sagRelease = 1.0f - std::exp(-1.0f / (0.180f * sr));
        // EL84 shared cathode: R71 50R with C47 220u -> about 11 ms.
        const float tau = 50.0f * 220.0e-6f;
        cathodeCoeff = 1.0f - std::exp(-1.0f / (std::fmax(tau, 0.001f) * sr));
    }

    float process(float env, float gain, float hot)
    {
        rectifierSag += (env - rectifierSag) * (env > rectifierSag ? sagAttack : sagRelease);
        cathodeRise += (env - cathodeRise) * cathodeCoeff;
        // Gentler sag than a full GZ34 dip: the AC30 breathes/compresses but it
        // should not collapse the steady-state level (that read as "muffled/too
        // compressed" when cranked). Keep the dynamic breathing, ease the depth.
        const float bPlus = 1.0f / (1.0f + rectifierSag * (0.30f + 0.52f * gain + 0.26f * hot));
        const float cathodeBias = 1.0f / (1.0f + cathodeRise * (0.16f + 0.28f * gain));
        return bPlus * cathodeBias;
    }
};

class El84PowerAmp
{
    RcHighPass c24c25Coupling100n;
    RcLowPass screenNode;
    Gz34CathodeBiasedSupply supply;

public:
    void reset()
    {
        c24c25Coupling100n.reset();
        screenNode.reset();
        supply.reset();
    }

    void setSampleRate(float sr)
    {
        // C24/C25 100n into roughly 1M PI/power-grid reference.
        c24c25Coupling100n.setRC(sr, 1000000.0f, 100.0e-9f);
        // 100R screen resistors plus supply filtering smooth screen current.
        screenNode.setRC(sr, 100.0f, 47.0e-6f);
        supply.setSampleRate(sr);
    }

    float process(float x, float gain, float hot, float bass, float treble)
    {
        float grid = c24c25Coupling100n.process(x);
        const float env = screenNode.process(std::fabs(grid));
        const float supplyScale = supply.process(env, gain, hot);
        const float drive = (1.02f + 1.45f * gain + 1.25f * hot) * supplyScale;
        float y = el84PentodePair(grid * drive, 0.03f + 0.012f * (treble - bass));
        // No global NFB in the AC30; preserve the raw EL84 edge but avoid
        // digital runaway when a booster hits the input. Lighter secondary clip
        // so the cranked tone stays open instead of squashing.
        y = 0.94f * y + 0.06f * softClip(y * (1.4f + 0.5f * gain));
        return y * supplyScale;
    }
};

class EN30Core
{
    float sampleRate = 48000.0f;
    float gain = kEN30Def[kGain];
    float bass = kEN30Def[kBass];
    float mid = kEN30Def[kMid];
    float treble = kEN30Def[kTreble];
    float pres = kEN30Def[kPres];
    float bright = kEN30Def[kBright];

    RcLowPass inputGridStopper;
    RcHighPass v1Bright120p;
    RcHighPass v1Bright470p;
    TriodeStage v8aFirstTriode;
    CathodeFollower v8bFollower;
    TopBoostToneStack topBoost;
    TriodeStage v7bRecovery;
    CutNetwork cut;
    El84PowerAmp powerAmp;

    Biquad transformerLow;
    Biquad speakerHp;
    Biquad speakerBody;
    Biquad speakerChime;
    Biquad speakerFizzNotch;
    Biquad speakerLp;
    DcBlock dcBlock;

    void updateComponentValues()
    {
        const float g = smoothstep(gain);
        const float hot = smoothstepRange(0.52f, 0.96f, gain);
        const float b = bright >= 0.5f ? 1.0f : 0.0f;

        // Input jack/grid: 68k stopper into 1M grid leak, with the first-stage
        // Miller capacitance setting the ultrasonic rolloff.
        inputGridStopper.setRC(sampleRate, 68000.0f, 55.0e-12f);

        // Brilliant volume treble bypass caps: C8 120pF and C9 470pF.
        v1Bright120p.setRC(sampleRate, 470000.0f, 120.0e-12f);
        v1Bright470p.setRC(sampleRate, 470000.0f, 470.0e-12f);

        // V8a ECC83: 100k plate, R9 1k5 with C7 22u bypass. The first coupling
        // cap is effectively direct from input for audio, so use a sub-audio HP.
        v8aFirstTriode.set(sampleRate,
                           1.0e-6f, 1000000.0f,
                           68000.0f, 55.0e-12f,
                           1500.0f, 22.0e-6f,
                           100000.0f, 1.15f, -0.075f,
                           1.18f, 0.58f, 0.54f);

        v8bFollower.set(sampleRate);
        topBoost.set(sampleRate, bass, mid, treble, b);

        // V7b recovery after the lossy Top Boost network. C36/C37 bypass parts
        // in the local schematic keep this stage full-range and chimey.
        v7bRecovery.set(sampleRate,
                        22.0e-9f, 470000.0f,
                        56000.0f, 80.0e-12f,
                        1500.0f, 10.0e-6f,
                        100000.0f, 1.28f, -0.085f,
                        1.62f, 0.62f, 0.46f);

        cut.set(sampleRate, pres);
        powerAmp.setSampleRate(sampleRate);

        transformerLow.setPeaking(sampleRate, 95.0f, 0.72f, -1.5f + 1.8f * bass);
        speakerHp.setHighPass(sampleRate, 72.0f, 0.72f);
        speakerBody.setPeaking(sampleRate, 335.0f, 0.80f, 1.2f + 1.5f * bass - 0.5f * hot);
        // Chime/presence band sits post-power-amp, so let Pres and Bright move it
        // here too — that way they stay audible even when the EL84s regenerate
        // highs (the pre-PI Cut alone was getting masked by the distortion).
        speakerChime.setPeaking(sampleRate, 2250.0f + 440.0f * treble, 0.74f,
                                2.0f + 2.3f * treble + 1.7f * b + 2.4f * pres);
        // Shallower notch that no longer deepens hard with Gain (it was killing
        // the highs exactly when the amp was driven = "distorted but muffled").
        speakerFizzNotch.setPeaking(sampleRate, 4700.0f + 380.0f * pres, 0.9f,
                                    -2.3f - 0.7f * g);
        // Open the top so the AC30 chime is there and Treble/Pres/Bright land
        // BELOW the rolloff (they were acting above it -> "they do nothing").
        speakerLp.setLowPass(sampleRate, 6600.0f + 3400.0f * pres + 1100.0f * b, 0.62f);
    }

public:
    void reset()
    {
        inputGridStopper.reset();
        v1Bright120p.reset();
        v1Bright470p.reset();
        v8aFirstTriode.reset();
        v8bFollower.reset();
        topBoost.reset();
        v7bRecovery.reset();
        cut.reset();
        powerAmp.reset();
        transformerLow.reset();
        speakerHp.reset();
        speakerBody.reset();
        speakerChime.reset();
        speakerFizzNotch.reset();
        speakerLp.reset();
        dcBlock.reset();
        updateComponentValues();
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr > 1000.0f ? sr : 48000.0f;
        reset();
    }

    void setGain(float v)   { gain = clamp01(v);   updateComponentValues(); }
    void setBass(float v)   { bass = clamp01(v);   updateComponentValues(); }
    void setMid(float v)    { mid = clamp01(v);    updateComponentValues(); }
    void setTreble(float v) { treble = clamp01(v); updateComponentValues(); }
    void setPres(float v)   { pres = clamp01(v);   updateComponentValues(); }
    void setBright(float v) { bright = clamp01(v); updateComponentValues(); }

    float process(float in)
    {
        const float g = smoothstep(gain);
        const float hot = smoothstepRange(0.52f, 0.96f, gain);
        const float b = bright >= 0.5f ? 1.0f : 0.0f;

        float x = inputGridStopper.process(in);

        // V8a first gain stage before the volume control.
        float v1 = v8aFirstTriode.process(x, 0.55f + 0.45f * hot, 1.0f);

        // Brilliant channel volume and bright caps. The 120p/470p caps bypass
        // the volume attenuation, so they matter most at lower Gain settings.
        const float volume = 0.16f + 1.62f * g + 0.88f * hot;
        const float brightBleed = b * (1.0f - 0.42f * g) *
            (0.11f * v1Bright120p.process(v1) + 0.15f * v1Bright470p.process(v1));
        float y = v1 * volume + brightBleed;

        // Cathode follower drives the low-impedance Top Boost network.
        y = v8bFollower.process(y);
        y = topBoost.process(y);

        // Recovery triode brings back the tone-stack insertion loss.
        y = v7bRecovery.process(y, 0.75f + 0.65f * g + 0.45f * hot, 1.0f);

        // Long-tail PI/cut and EL84 cathode-biased output stage.
        y = cut.process(y);
        y = powerAmp.process(y, gain, hot, bass, treble);
        y = dcBlock.process(y);

        // Output transformer and 2x12 Celestion-style cabinet.
        y = transformerLow.process(y);
        y = speakerHp.process(y);
        y = speakerBody.process(y);
        y = speakerChime.process(y);
        y = speakerFizzNotch.process(y);
        y = speakerLp.process(y);

        const float toneEnergy = 1.0f
            + 0.013f * std::fabs((bass - 0.5f) * 20.0f)
            + 0.012f * std::fabs((mid - 0.5f) * 22.0f)
            + 0.013f * std::fabs((treble - 0.5f) * 22.0f);
        // Output makeup (NOT a circuit element). The EL84 saturation + GZ34 sag
        // deliberately lower the *dynamic* level as Gain rises — that compression
        // is part of the class-A AC30 feel and is kept. But the steady-state
        // loudness must stay roughly constant across the Gain sweep so high-gain
        // presets don't drop ~10 dB in the mix. This makeup rises with Gain to
        // counter the saturation/sag RMS loss while preserving the sag dynamics.
        const float makeup = 1.0f + 0.74f * g + 0.42f * hot;
        const float level = (0.82f * makeup) / toneEnergy;
        return softClip(y * level) * 0.98f;
    }
};

} // namespace en30

#endif // EN30_CORE_H
