#ifndef RB_AUTOMAKEUP_HPP
#define RB_AUTOMAKEUP_HPP

#include <cmath>

/*
 * RBAutoMakeup — loudness-matched auto makeup-gain.
 *
 * Tracks the slow RMS of the dry input and of the wet (processed) output and
 * scales the wet so that RMS_out == RMS_in. This decouples a drive pedal's
 * Gain knob from its output level: Gain changes how hard the signal clips, NOT
 * how loud the pedal is, so every distortion / overdrive / fuzz sits at the
 * same level as the bypassed (dry) signal — and therefore at the same level as
 * each other.
 *
 * The time constant is deliberately slow (~250 ms) so it matches overall
 * loudness without behaving like a compressor (a fast follower would squash the
 * pedal's dynamics and kill its character). Near silence the ratio is frozen so
 * the noise floor is never boosted.
 *
 * Usage (per channel), at the plugin's run() level:
 *     outL[i] = makeupL.process(inL[i], core.process(inL[i]));
 */
struct RBAutoMakeup
{
    float coef   = 0.0f;   // one-pole smoothing coefficient
    float inEnv  = 0.0f;   // mean-square of the dry signal
    float outEnv = 0.0f;   // mean-square of the wet signal
    float gain   = 1.0f;   // smoothed makeup gain currently applied

    void setSampleRate(float sr)
    {
        if (sr < 1000.0f)
            sr = 48000.0f;
        coef = std::exp(-1.0f / (0.25f * sr));   // ~250 ms RMS window
        reset();
    }

    void reset()
    {
        inEnv = outEnv = 0.0f;
        gain = 1.0f;
    }

    float process(float dry, float wet)
    {
        inEnv  = coef * inEnv  + (1.0f - coef) * dry * dry;
        outEnv = coef * outEnv + (1.0f - coef) * wet * wet;

        // Only chase a new target when there is real output energy AND real
        // input energy; this freezes the ratio during silence so hiss is not
        // amplified to "signal" level.
        if (outEnv > 1.0e-7f && inEnv > 1.0e-9f)
        {
            float target = std::sqrt(inEnv / outEnv);
            if (target > 8.0f)
                target = 8.0f;                   // safety ceiling
            gain = coef * gain + (1.0f - coef) * target;
        }

        return wet * gain;
    }
};

#endif // RB_AUTOMAKEUP_HPP
