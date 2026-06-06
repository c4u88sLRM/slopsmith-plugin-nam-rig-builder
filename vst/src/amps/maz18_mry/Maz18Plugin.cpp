/*
 * MR. Y MAZ 18 - Dr. Z Maz 18 Jr Reverb for Rocksmith's Amp_GB38. Parody brand
 * "Mr. Y"; the in-app face must never read "Dr. Z" or "Maz".
 *
 * Hand-traced schematic (Dr. Z Maz 18 Jr Reverb): a 2xEL84 (~18W) class-AB head
 * with a GZ34 tube rectifier (=> noticeable SAG / spongy, touch-sensitive feel).
 * Voiced Vox-meets-Fender: chimey, midrange-forward, breaks up musically.
 *  - Preamp: 12AX7 input (68k/1M, 680ohm cathode), 1n couplings; 2nd 12AX7 (100k
 *    plate, 820ohm+25uF cathode); tone stack: TREBLE 250k (250pF treble cap),
 *    MIDDLE 10k, BASS ~1M, with 56k/100k/47n -> VOLUME (1MA).
 *  - 12AX7 phase inverter; CUT 250kA (post treble cut, higher = darker); MASTER.
 *  - Spring REVERB (12AT7 driver + tank + 12AX7 recovery + 250k Reverb level).
 *  - Power: 2xEL84 (2.7k screens, 360v), CinTran OT; GZ34 rectifier sag (spongy).
 *
 * Rocksmith: RS Gain -> VOLUME (the amp's only preamp/drive control); Bass/Mid/
 * Treble -> the TMB tone stack. Cut/Master/Reverb set on the face by hand.
 */
#include "DistrhoPlugin.hpp"
#include "Maz18Params.h"
#include <cmath>

START_NAMESPACE_DISTRHO

static inline float rbAmpLvl(float x){ const float t=0.90f,c=0.99f,a=(x<0.f?-x:x);
    if(a<=t) return x; return (x<0.f?-1.f:1.f)*(t+(c-t)*std::tanh((a-t)/(c-t))); }

namespace {

static constexpr float kPi = 3.14159265359f;
static inline float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }
static inline float clampFreq(float hz, float sr) { return std::fmax(20.0f, std::fmin(hz, sr * 0.45f)); }
static inline float smoothstep(float v) { v = clamp01(v); return v * v * (3.0f - 2.0f * v); }
static inline float smoothstepRange(float e0, float e1, float x) { return smoothstep((x - e0) / (e1 - e0)); }
static inline float softClip(float x) { return std::tanh(x); }
static inline float asymTube(float x, float drive, float bias)
{
    const float pushed = x * drive + bias;
    const float y = std::tanh(pushed);
    const float correction = std::tanh(bias);
    return (y - correction) / (1.0f - 0.32f * std::fabs(correction));
}
static inline float tonePot(float v) { v = clamp01(v); return v < 0.001f ? 0.001f : (v > 0.999f ? 0.999f : v); }

class Biquad
{
    float b0=1.0f,b1=0.0f,b2=0.0f,a1=0.0f,a2=0.0f,z1=0.0f,z2=0.0f;
    void set(float nb0,float nb1,float nb2,float na0,float na1,float na2)
    { if(std::fabs(na0)<1.0e-12f) na0=1.0f; const float i=1.0f/na0;
      b0=nb0*i; b1=nb1*i; b2=nb2*i; a1=na1*i; a2=na2*i; }
public:
    void reset(){ z1=z2=0.0f; }
    float process(float x){ const float y=b0*x+z1; z1=b1*x-a1*y+z2; z2=b2*x-a2*y; return y; }
    void setHighPass(float sr,float hz,float q){ hz=clampFreq(hz,sr); const float w=2.0f*kPi*hz/sr,c=std::cos(w),al=std::sin(w)/(2.0f*q);
        set((1.0f+c)*0.5f,-(1.0f+c),(1.0f+c)*0.5f,1.0f+al,-2.0f*c,1.0f-al); }
    void setLowPass(float sr,float hz,float q){ hz=clampFreq(hz,sr); const float w=2.0f*kPi*hz/sr,c=std::cos(w),al=std::sin(w)/(2.0f*q);
        set((1.0f-c)*0.5f,1.0f-c,(1.0f-c)*0.5f,1.0f+al,-2.0f*c,1.0f-al); }
    void setPeaking(float sr,float hz,float q,float dB){ hz=clampFreq(hz,sr); const float a=std::pow(10.0f,dB/40.0f),w=2.0f*kPi*hz/sr,c=std::cos(w),al=std::sin(w)/(2.0f*q);
        set(1.0f+al*a,-2.0f*c,1.0f-al*a,1.0f+al/a,-2.0f*c,1.0f-al/a); }
    void setHighShelf(float sr,float hz,float sl,float dB){ hz=clampFreq(hz,sr); const float a=std::pow(10.0f,dB/40.0f),w=2.0f*kPi*hz/sr,c=std::cos(w),sn=std::sin(w),ra=std::sqrt(a),al=sn*0.5f*std::sqrt((a+1.0f/a)*(1.0f/sl-1.0f)+2.0f);
        set(a*((a+1.0f)+(a-1.0f)*c+2.0f*ra*al),-2.0f*a*((a-1.0f)+(a+1.0f)*c),a*((a+1.0f)+(a-1.0f)*c-2.0f*ra*al),
            (a+1.0f)-(a-1.0f)*c+2.0f*ra*al,2.0f*((a-1.0f)-(a+1.0f)*c),(a+1.0f)-(a-1.0f)*c-2.0f*ra*al); }
    void setLowShelf(float sr,float hz,float sl,float dB){ hz=clampFreq(hz,sr); const float a=std::pow(10.0f,dB/40.0f),w=2.0f*kPi*hz/sr,c=std::cos(w),sn=std::sin(w),ra=std::sqrt(a),al=sn*0.5f*std::sqrt((a+1.0f/a)*(1.0f/sl-1.0f)+2.0f);
        set(a*((a+1.0f)-(a-1.0f)*c+2.0f*ra*al),2.0f*a*((a-1.0f)-(a+1.0f)*c),a*((a+1.0f)-(a-1.0f)*c-2.0f*ra*al),
            (a+1.0f)+(a-1.0f)*c+2.0f*ra*al,-2.0f*((a-1.0f)+(a+1.0f)*c),(a+1.0f)+(a-1.0f)*c-2.0f*ra*al); }
};

// Dr.Z Maz 18 TMB tone stack (3rd-order bilinear): TREBLE 250k (250pF treble cap),
// MIDDLE 10k, BASS 1M, slope R 100k, coupling/bass C 47nF. Vox-meets-Fender
// midrange-forward voicing; placed before the volume / output drive.
class MarkToneStack
{
    float b0=1,b1=0,b2=0,b3=0,a1=0,a2=0,a3=0,x1=0,x2=0,x3=0,y1=0,y2=0,y3=0,sampleRate=48000.0f;
public:
    void reset(){ x1=x2=x3=y1=y2=y3=0.0f; }
    void setSampleRate(float sr){ sampleRate=sr>1000.0f?sr:48000.0f; }
    void update(float treble,float mid,float bass)
    {
        const float t=tonePot(treble),m=tonePot(mid),l=tonePot(bass);
        const float R1=250.0e3f, R2=1.0e6f, R3=10.0e3f, R4=100.0e3f;
        const float C1=250.0e-12f, C2=47.0e-9f, C3=47.0e-9f;
        const float ab0=0.0f;
        const float ab1=t*C1*R1 + m*C3*R3 + l*(C1*R2+C2*R2) + (C1*R3+C2*R3);
        const float ab2=t*(C1*C2*R1*R4+C1*C3*R1*R4) - m*m*(C1*C3*R3*R3+C2*C3*R3*R3)
                      + m*(C1*C3*R1*R3+C1*C3*R3*R3+C2*C3*R3*R3) + l*(C1*C2*R1*R2+C1*C2*R2*R4+C1*C3*R2*R4)
                      + l*m*(C1*C3*R2*R3+C2*C3*R2*R3) + (C1*C2*R1*R3+C1*C2*R3*R4+C1*C3*R3*R4);
        const float ab3=l*m*(C1*C2*C3*R1*R2*R3+C1*C2*C3*R2*R3*R4) - m*m*(C1*C2*C3*R1*R3*R3+C1*C2*C3*R3*R3*R4)
                      + m*(C1*C2*C3*R1*R3*R3+C1*C2*C3*R3*R3*R4) + t*C1*C2*C3*R1*R3*R4 - t*m*C1*C2*C3*R1*R3*R4
                      + t*l*C1*C2*C3*R1*R2*R4;
        const float aa0=1.0f;
        const float aa1=(C1*R1+C1*R3+C2*R3+C2*R4+C3*R4) + m*C3*R3 + l*(C1*R2+C2*R2);
        const float aa2=m*(C1*C3*R1*R3-C2*C3*R3*R4+C1*C3*R3*R3+C2*C3*R3*R3) - m*m*(C1*C3*R3*R3+C2*C3*R3*R3)
                      + l*m*(C1*C3*R2*R3+C2*C3*R2*R3) + l*(C1*C2*R2*R4+C1*C2*R1*R2+C1*C3*R2*R4+C2*C3*R2*R4)
                      + (C1*C2*R1*R4+C1*C3*R1*R4+C1*C2*R3*R4+C1*C2*R1*R3+C1*C3*R3*R4+C2*C3*R3*R4);
        const float aa3=l*m*(C1*C2*C3*R1*R2*R3+C1*C2*C3*R2*R3*R4) - m*m*(C1*C2*C3*R1*R3*R3+C1*C2*C3*R3*R3*R4)
                      + m*(C1*C2*C3*R3*R3*R4+C1*C2*C3*R1*R3*R3-C1*C2*C3*R1*R3*R4) + l*(C1*C2*C3*R1*R2*R4) + C1*C2*C3*R1*R3*R4;
        const float c=2.0f*sampleRate, c2=c*c, c3=c2*c;
        const float nb0=-ab0-ab1*c-ab2*c2-ab3*c3, nb1=-3.0f*ab0-ab1*c+ab2*c2+3.0f*ab3*c3,
                    nb2=-3.0f*ab0+ab1*c+ab2*c2-3.0f*ab3*c3, nb3=-ab0+ab1*c-ab2*c2+ab3*c3;
        const float na0=-aa0-aa1*c-aa2*c2-aa3*c3, na1=-3.0f*aa0-aa1*c+aa2*c2+3.0f*aa3*c3,
                    na2=-3.0f*aa0+aa1*c+aa2*c2-3.0f*aa3*c3, na3=-aa0+aa1*c-aa2*c2+aa3*c3;
        if(std::fabs(na0)<1.0e-30f){ b0=1.0f; b1=b2=b3=a1=a2=a3=0.0f; return; }
        const float i=1.0f/na0; b0=nb0*i; b1=nb1*i; b2=nb2*i; b3=nb3*i; a1=na1*i; a2=na2*i; a3=na3*i;
    }
    float process(float x){ const float y=b0*x+b1*x1+b2*x2+b3*x3-a1*y1-a2*y2-a3*y3;
        x3=x2; x2=x1; x1=x; y3=y2; y2=y1; y1=y; return y; }
};

class DcBlock
{
    float x1=0.0f,y1=0.0f;
public:
    void reset(){ x1=y1=0.0f; }
    float process(float x){ const float y=x-x1+0.995f*y1; x1=x; y1=y; return y; }
};

// Compact Schroeder spring reverb (2 combs + 2 allpass). The Maz 18 reverb is
// off for songs (RS pins it to 0); this gives the REVERB knob something musical
// when dialled by hand (12AT7 driver + tank + 12AX7 recovery).
class SpringReverb
{
    static const int N1=1481, N2=1709, A1=229, A2=97;
    float c1[N1], c2[N2], ap1[A1], ap2[A2];
    int i1=0,i2=0,j1=0,j2=0; float lp1=0.0f,lp2=0.0f;
public:
    void reset(){ for(int i=0;i<N1;++i)c1[i]=0.f; for(int i=0;i<N2;++i)c2[i]=0.f;
        for(int i=0;i<A1;++i)ap1[i]=0.f; for(int i=0;i<A2;++i)ap2[i]=0.f;
        i1=i2=j1=j2=0; lp1=lp2=0.f; }
    float process(float x){
        float y1=c1[i1]; lp1 += 0.42f*(y1-lp1); c1[i1]= x + 0.80f*lp1; if(++i1>=N1)i1=0;
        float y2=c2[i2]; lp2 += 0.42f*(y2-lp2); c2[i2]= x + 0.76f*lp2; if(++i2>=N2)i2=0;
        float y=(y1+y2)*0.5f;
        float t1=ap1[j1]; float o1=-0.6f*y+t1; ap1[j1]= y+0.6f*o1; if(++j1>=A1)j1=0; y=o1;
        float t2=ap2[j2]; float o2=-0.6f*y+t2; ap2[j2]= y+0.6f*o2; if(++j2>=A2)j2=0; y=o2;
        return y;
    }
};

} // namespace

class Maz18Core
{
    float sampleRate = 48000.0f;
    float volume = kMaz18Def[kVolume];
    float treble = kMaz18Def[kTreble];
    float mid    = kMaz18Def[kMiddle];
    float bass   = kMaz18Def[kBass];
    float cut    = kMaz18Def[kCut];
    float master = kMaz18Def[kMaster];
    float reverb = kMaz18Def[kReverb];

    // derived
    float drv = 0.6f, outM = 0.7f;

    Biquad inputHp, pickupLoad, brightCap;
    MarkToneStack toneStack;
    Biquad stackMakeupLow, interHp, cathodeLp;
    Biquad phaseLp, cutShelf, presenceShelf;
    Biquad speakerHp, speakerThump, speakerLowMid, speakerBite, speakerFizz, speakerLp;
    DcBlock dcBlock;
    SpringReverb spring;
    float sag = 0.0f;

    void updateFilters()
    {
        drv  = clamp01(volume);
        outM = clamp01(master);
        const float g = smoothstep(drv);
        const float pushed = smoothstepRange(0.40f, 0.92f, drv);

        inputHp.setHighPass(sampleRate, 40.0f + 34.0f * g, 0.70f);
        pickupLoad.setLowPass(sampleRate, 13200.0f - 1200.0f * pushed + 800.0f * treble, 0.64f);
        // 12AX7 input: a small bright cap across the input network (chime).
        brightCap.setHighShelf(sampleRate, 2000.0f, 0.72f, 2.0f - 1.2f * g);

        toneStack.update(treble, mid, bass);
        // Tone-stack makeup low end (the TMB stack is bass-shy; restore body).
        stackMakeupLow.setLowShelf(sampleRate, 130.0f + 30.0f * bass, 0.72f,
            ((clamp01(bass) - 0.5f) * 7.0f) - 0.8f * pushed);
        interHp.setHighPass(sampleRate, 120.0f + 110.0f * pushed, 0.70f);
        cathodeLp.setLowPass(sampleRate, 9500.0f + 1400.0f * treble - 1200.0f * pushed, 0.64f);

        phaseLp.setLowPass(sampleRate, 7400.0f + 1500.0f * treble - 800.0f * pushed, 0.64f);
        // CUT 250kA: a post-PI treble bleed to ground — HIGHER = DARKER.
        cutShelf.setHighShelf(sampleRate, 3000.0f, 0.70f, -1.0f - 9.0f * smoothstep(cut));
        // EL84 chime / presence: a touch brighter than a 6L6 amp.
        presenceShelf.setHighShelf(sampleRate, 2900.0f, 0.78f, 2.6f + 1.0f * treble);

        // Vox-meets-Fender 1x12 (EL84) voicing: midrange-forward, chimey top.
        speakerHp.setHighPass(sampleRate, 84.0f, 0.72f);
        speakerThump.setPeaking(sampleRate, 120.0f, 0.84f, 0.6f + 2.0f * bass);
        speakerLowMid.setPeaking(sampleRate, 480.0f + 120.0f * mid, 0.74f, 1.0f + 2.2f * mid - 0.5f * pushed);
        speakerBite.setPeaking(sampleRate, 2800.0f + 500.0f * treble, 0.74f, 1.6f + 2.0f * treble);
        speakerFizz.setPeaking(sampleRate, 5600.0f, 0.96f, -2.6f - 2.2f * pushed);
        speakerLp.setLowPass(sampleRate, 6200.0f + 1900.0f * treble - 800.0f * pushed, 0.66f);
    }

public:
    void reset()
    {
        inputHp.reset(); pickupLoad.reset(); brightCap.reset();
        toneStack.reset(); stackMakeupLow.reset(); interHp.reset(); cathodeLp.reset();
        phaseLp.reset(); cutShelf.reset(); presenceShelf.reset();
        speakerHp.reset(); speakerThump.reset(); speakerLowMid.reset(); speakerBite.reset(); speakerFizz.reset(); speakerLp.reset();
        dcBlock.reset(); spring.reset(); sag = 0.0f;
        updateFilters();
    }

    void setSampleRate(float sr) { sampleRate = sr > 1000.0f ? sr : 48000.0f; toneStack.setSampleRate(sampleRate); reset(); }

    void setParam(int idx, float v)
    {
        v = clamp01(v);
        switch (idx)
        {
            case kVolume: volume = v; break;
            case kTreble: treble = v; break;
            case kMiddle: mid = v; break;
            case kBass:   bass = v; break;
            case kCut:    cut = v; break;
            case kMaster: master = v; break;
            case kReverb: reverb = v; break;
            default: break;
        }
        updateFilters();
    }

    void initDefaults() { for (int i = 0; i < kParamCount; ++i) setParam(i, kMaz18Def[i]); }

    float process(float in)
    {
        const float g = smoothstep(drv);
        const float pushed = smoothstepRange(0.40f, 0.92f, drv);
        const float mPush = smoothstep(outM);

        float x = inputHp.process(in);
        x = pickupLoad.process(x);
        x = brightCap.process(x);
        // 12AX7 input gain stage (mild) -> the TMB tone stack (pre-drive EQ).
        x = asymTube(x, 1.05f + 0.5f * g, 0.008f);
        float t = toneStack.process(x) * 2.0f;
        t = stackMakeupLow.process(t);

        // 2nd 12AX7 + VOLUME: the amp's preamp gain (chimey, breaks up musically).
        float y = asymTube(t * (0.7f + 2.6f * volume), 1.15f + 2.6f * volume + 1.6f * g, 0.012f + 0.012f * volume);
        y = interHp.process(y);
        // a 2nd mild stage to fatten as Volume climbs (touch-sensitive feel)
        y = asymTube(y, 0.95f + 1.6f * volume + 1.2f * pushed, -0.008f);
        y = cathodeLp.process(y);

        // spring reverb (parallel send), off when REVERB = 0
        if (reverb > 0.001f) { const float wet = spring.process(y); y += (0.9f * reverb) * wet; }

        y = phaseLp.process(y);

        // 2xEL84 ~18W class-AB + GZ34 rectifier sag. The Maz 18 is a SMALL amp:
        // it compresses earlier, and the GZ34 sag is a touch spongier than usual
        // (slower release, deeper drop) -> the spongy, touch-sensitive feel.
        const float env = std::fabs(y);
        const float attack = 1.0f - std::exp(-1.0f / (0.0072f * sampleRate));   // a bit slower attack
        const float release = 1.0f - std::exp(-1.0f / (0.190f * sampleRate));   // spongier (longer) release
        sag += (env - sag) * (env > sag ? attack : release);
        const float sagDrop = 1.0f / (1.0f + sag * (0.44f + 0.78f * mPush + 0.58f * pushed));  // deeper sag
        const float powerDrive = (1.05f + 1.75f * mPush + 1.45f * pushed) * sagDrop;           // EL84 breaks up early
        y = asymTube(y, powerDrive, 0.006f + 0.012f * (treble - bass));
        y = 0.84f * y + 0.16f * softClip(y * (1.6f + 1.2f * pushed));   // a touch more 2nd-stage compression
        y *= 0.96f - 0.10f * sag;                                       // spongier level droop

        y = cutShelf.process(y);
        y = presenceShelf.process(y);
        y = dcBlock.process(y);

        y = speakerHp.process(y);
        y = speakerThump.process(y);
        y = speakerLowMid.process(y);
        y = speakerBite.process(y);
        y = speakerFizz.process(y);
        y = speakerLp.process(y);

        // Loudness normalization: VOLUME adds clipping (not level), so cleanMakeup
        // carries the drive compensation and the base is flat across VOLUME; the
        // master keeps a mild swing (~-14 dBFS).
        const float toneEnergy = 1.0f
            + 0.011f * std::fabs((bass - 0.5f) * 15.0f)
            + 0.012f * std::fabs((mid - 0.5f) * 17.0f)
            + 0.012f * std::fabs((treble - 0.5f) * 17.0f);
        const float cleanMakeup = 1.0f + 8.0f * std::exp(-drv / 0.28f);
        const float level = 0.46f * cleanMakeup / ((1.0f + 0.42f * mPush) * toneEnergy);
        return softClip(y * level) * 0.97f;
    }
};

class Maz18Plugin : public Plugin
{
    Maz18Core left;
    Maz18Core right;
    float params[kParamCount];

    void applyAll() { for (int i = 0; i < kParamCount; ++i) { left.setParam(i, params[i]); right.setParam(i, params[i]); } }

public:
    Maz18Plugin() : Plugin(kParamCount, 0, 0)
    {
        for (int i = 0; i < kParamCount; ++i) params[i] = kMaz18Def[i];
        left.setSampleRate((float)getSampleRate());
        right.setSampleRate((float)getSampleRate());
        applyAll();
    }

protected:
    const char* getLabel() const override { return "MrYMaz18"; }
    const char* getDescription() const override { return "Dr. Z Maz 18 Jr Reverb style 2xEL84 amp"; }
    const char* getMaker() const override { return "RigBuilder"; }
    const char* getLicense() const override { return "ISC"; }
    uint32_t getVersion() const override { return d_version(1, 0, 0); }
    int64_t getUniqueId() const override { return d_cconst('Y', 'm', '1', '8'); }

    void initParameter(uint32_t index, Parameter& parameter) override
    {
        if (index >= (uint32_t)kParamCount) return;
        parameter.hints = kParameterIsAutomatable;
        parameter.name = kMaz18Names[index];
        parameter.symbol = kMaz18Symbols[index];
        parameter.ranges.min = kMaz18Min[index];
        parameter.ranges.max = kMaz18Max[index];
        parameter.ranges.def = kMaz18Def[index];
    }

    float getParameterValue(uint32_t index) const override { return index < (uint32_t)kParamCount ? params[index] : 0.0f; }

    void setParameterValue(uint32_t index, float value) override
    {
        if (index >= (uint32_t)kParamCount) return;
        params[index] = clamp01(value);
        left.setParam((int)index, params[index]);
        right.setParam((int)index, params[index]);
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
            outL[i] = rbAmpLvl(0.560f * left.process(inL[i]));
            outR[i] = rbAmpLvl(0.560f * right.process(inR[i]));
        }
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Maz18Plugin)
};

Plugin* createPlugin() { return new Maz18Plugin(); }

END_NAMESPACE_DISTRHO
