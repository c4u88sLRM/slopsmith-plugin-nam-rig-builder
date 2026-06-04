/*
 * Bender SuperNova22 - Fender Super-Sonic 22 / 6V6-style amp for Rocksmith's Amp_TW22.
 *
 * DPF/VST3 wrapper. All the DSP lives in TW22Core.h (plain C++, offline-testable);
 * see that header for the circuit topology and the schematic reference.
 */
#include "DistrhoPlugin.hpp"
#include "TW22Params.h"
#include "TW22Core.h"

START_NAMESPACE_DISTRHO

class TW22Plugin : public Plugin
{
    tw22::TW22Core left;
    tw22::TW22Core right;
    float params[kParamCount];

    void applyAll()
    {
        left.setVintVol(params[kVintVol]);       right.setVintVol(params[kVintVol]);
        left.setVintTreble(params[kVintTreble]); right.setVintTreble(params[kVintTreble]);
        left.setVintBass(params[kVintBass]);     right.setVintBass(params[kVintBass]);
        left.setNormFat(params[kNormFat]);       right.setNormFat(params[kNormFat]);
        left.setChannel(params[kChannel]);       right.setChannel(params[kChannel]);
        left.setGain1(params[kGain1]);           right.setGain1(params[kGain1]);
        left.setGain2(params[kGain2]);           right.setGain2(params[kGain2]);
        left.setBurnTreble(params[kBurnTreble]); right.setBurnTreble(params[kBurnTreble]);
        left.setBurnBass(params[kBurnBass]);     right.setBurnBass(params[kBurnBass]);
        left.setBurnMid(params[kBurnMid]);       right.setBurnMid(params[kBurnMid]);
        left.setBurnVol(params[kBurnVol]);       right.setBurnVol(params[kBurnVol]);
        left.setReverb(params[kReverb]);         right.setReverb(params[kReverb]);
        left.setPresence(params[kPresence]);     right.setPresence(params[kPresence]);
    }

public:
    TW22Plugin()
        : Plugin(kParamCount, 0, 0)
    {
        for (int i = 0; i < kParamCount; ++i)
            params[i] = kTW22Def[i];
        left.setSampleRate((float)getSampleRate());
        right.setSampleRate((float)getSampleRate());
        applyAll();
    }

protected:
    const char* getLabel() const override { return "Bender SuperNova22"; }
    const char* getDescription() const override { return "Bender SuperNova22 / Super-Sonic 22 style amp"; }
    const char* getMaker() const override { return "RigBuilder"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return d_cconst('T', 'w', '2', '2'); }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        if (index >= (uint32_t)kParamCount)
            return;
        parameter.hints = kParameterIsAutomatable;
        parameter.name = kTW22Names[index];
        parameter.symbol = kTW22Symbols[index];
        parameter.ranges.min = kTW22Min[index];
        parameter.ranges.max = kTW22Max[index];
        parameter.ranges.def = kTW22Def[index];
    }

    float getParameterValue(uint32_t index) const override
    {
        return index < (uint32_t)kParamCount ? params[index] : 0.0f;
    }

    void setParameterValue(uint32_t index, float value) override
    {
        if (index >= (uint32_t)kParamCount)
            return;
        params[index] = tw22::clamp01(value);
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

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TW22Plugin)
};

Plugin* createPlugin()
{
    return new TW22Plugin();
}

END_NAMESPACE_DISTRHO
