/*
 * OctaveUp - analog octave-up for Rocksmith's Pedal_OctaveUp.
 * The reference layout is Foxrox Octron-style: buffered dry path plus an
 * octave-up path generated from driven rectification. Rocksmith exposes only
 * Tone and Mix, so Tone shapes the rectified octave voice and Mix blends it
 * against the direct guitar.
 */
#include "DistrhoPlugin.hpp"
#include "OctaveUpParams.h"
#include <cmath>

START_NAMESPACE_DISTRHO

namespace {

static inline float clamp01(float v)
{
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

static inline float softClip(float x)
{
    return std::tanh(x);
}

} // namespace

class OctaveUpCore
{
    float sampleRate = 48000.0f;
    float tone = kOctaveUpDef[kTone];
    float mix = kOctaveUpDef[kMix];

    float inHpX1 = 0.0f;
    float inHpY1 = 0.0f;
    float rectHpX1 = 0.0f;
    float rectHpY1 = 0.0f;
    float toneLowY = 0.0f;
    float brightY = 0.0f;
    float dryToneY = 0.0f;

    float inHpA = 0.0f;
    float rectHpA = 0.0f;
    float toneLowA = 0.0f;
    float brightA = 0.0f;
    float dryToneA = 0.0f;

    void updateFilters()
    {
        const float dt = 1.0f / sampleRate;

        const float inHpHz = 45.0f;
        const float inHpRc = 1.0f / (2.0f * 3.14159265359f * inHpHz);
        inHpA = inHpRc / (inHpRc + dt);

        const float rectHpHz = 180.0f + 90.0f * tone;
        const float rectHpRc = 1.0f / (2.0f * 3.14159265359f * rectHpHz);
        rectHpA = rectHpRc / (rectHpRc + dt);

        const float toneLowHz = 1650.0f * std::pow(4.0f, tone);
        toneLowA = 1.0f - std::exp(-2.0f * 3.14159265359f * toneLowHz / sampleRate);

        const float brightHz = 2600.0f + 3600.0f * tone;
        brightA = 1.0f - std::exp(-2.0f * 3.14159265359f * brightHz / sampleRate);

        dryToneA = 1.0f - std::exp(-2.0f * 3.14159265359f * 8500.0f / sampleRate);

    }

    float inputHighPass(float x)
    {
        const float y = inHpA * (inHpY1 + x - inHpX1);
        inHpX1 = x;
        inHpY1 = y;
        return y;
    }

    float rectHighPass(float x)
    {
        const float y = rectHpA * (rectHpY1 + x - rectHpX1);
        rectHpX1 = x;
        rectHpY1 = y;
        return y;
    }

    float lowPass(float x, float& z, float a)
    {
        z += a * (x - z);
        return z;
    }

public:
    void reset()
    {
        inHpX1 = inHpY1 = rectHpX1 = rectHpY1 = 0.0f;
        toneLowY = brightY = dryToneY = 0.0f;
        updateFilters();
    }

    void setSampleRate(float sr)
    {
        sampleRate = sr > 1000.0f ? sr : 48000.0f;
        reset();
    }

    void setTone(float v)
    {
        tone = clamp01(v);
        updateFilters();
    }

    void setMix(float v)
    {
        mix = clamp01(v);
    }

    float process(float in)
    {
        float dry = lowPass(in, dryToneY, dryToneA);
        float x = inputHighPass(in);

        // Octron-like octave-up path: pre-drive into a full-wave rectifier,
        // then block DC. Tone raises both diode bite and top-end.
        const float drive = 1.18f + 1.15f * tone;
        float driven = x * drive;
        driven = 0.94f * driven + 0.06f * softClip(driven * (1.15f + 0.65f * tone));

        float rect = std::fabs(driven);
        rect = rectHighPass(rect);
        float octave = rect;

        const float smooth = lowPass(octave, toneLowY, toneLowA);
        const float brightBase = lowPass(octave, brightY, brightA);
        const float bright = octave - 0.48f * brightBase;
        octave = smooth * (1.00f - 0.34f * tone) + bright * (0.12f + 0.52f * tone);

        // Light diode/output-transistor rounding only. The octave-up pedal
        // should track immediately and stay cleaner than a fuzz.
        octave = 0.96f * octave + 0.04f * softClip(octave * (1.4f + 0.45f * tone));

        const float wet = octave * 1.15f;
        const float dryLevel = 1.0f - 0.78f * mix;
        const float wetLevel = 1.55f * mix;
        return dry * dryLevel + wet * wetLevel;
    }
};

class OctaveUpPlugin : public Plugin
{
    OctaveUpCore left;
    OctaveUpCore right;
    float params[kParamCount];

    void applyAll()
    {
        left.setTone(params[kTone]);
        right.setTone(params[kTone]);
        left.setMix(params[kMix]);
        right.setMix(params[kMix]);
    }

public:
    OctaveUpPlugin()
        : Plugin(kParamCount, 0, 0)
    {
        for (int i = 0; i < kParamCount; ++i)
            params[i] = kOctaveUpDef[i];
        left.setSampleRate((float)getSampleRate());
        right.setSampleRate((float)getSampleRate());
        applyAll();
    }

protected:
    const char* getLabel() const override { return "OctaveUp"; }
    const char* getDescription() const override { return "Analog octave-up pedal"; }
    const char* getMaker() const override { return "RigBuilder"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 1); }
    int64_t getUniqueId() const override { return d_cconst('O', 'c', 'u', 'p'); }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        if (index >= (uint32_t)kParamCount)
            return;
        parameter.hints = kParameterIsAutomatable;
        parameter.name = kOctaveUpNames[index];
        parameter.symbol = kOctaveUpSymbols[index];
        parameter.ranges.min = kOctaveUpMin[index];
        parameter.ranges.max = kOctaveUpMax[index];
        parameter.ranges.def = kOctaveUpDef[index];
    }

    float getParameterValue(uint32_t index) const override
    {
        return index < (uint32_t)kParamCount ? params[index] : 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override
    {
        if (index >= (uint32_t)kParamCount)
            return;
        params[index] = clamp01(value);
        applyAll();
    }

    void sampleRateChanged(double newSampleRate) override
    {
        left.setSampleRate((float)newSampleRate);
        right.setSampleRate((float)newSampleRate);
        applyAll();
    }

    void run(const float** inputs, float** outputs, uint32_t frames) override
    {
        const float* inL = inputs[0];
        const float* inR = inputs[1];
        float* outL = outputs[0];
        float* outR = outputs[1];
        for (uint32_t i = 0; i < frames; ++i)
        {
            outL[i] = left.process(inL[i]);
            outR[i] = right.process(inR[i]);
        }
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OctaveUpPlugin)
};

Plugin* createPlugin()
{
    return new OctaveUpPlugin();
}

END_NAMESPACE_DISTRHO
