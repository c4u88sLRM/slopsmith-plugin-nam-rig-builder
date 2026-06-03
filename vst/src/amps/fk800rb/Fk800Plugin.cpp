/*
 * Freddy Krueger 800BR — Gallien-Krueger 800RB bass-head model.
 *
 * DSP modeled from the GK 800RB service manual (preamp sheet 406-0045 "Bob
 * Gallien 800 RB Preamp", + the Operators Manual block diagram & specs). The
 * 800RB is an all-op-amp (LF353) preamp feeding a bi-amp power section
 * (300W low / 100W high). We recreate each block in order:
 *
 *   1. Input  : 1/4" in with a -10 dB pad (drops preamp gain 10 dB, +headroom).
 *   2. Volume : preamp drive into a gentle op-amp soft-clip — the GK "growl"
 *               that the input/diode stage (D1,D2) produces when pushed.
 *   3. Voicing filters (the 3 square switches):
 *        Lo Cut      — bass roll-off (high-pass) to kill stage rumble.
 *        Mid Contour — notch at ~500 Hz ("mellow round sound").
 *        Hi Boost    — presence/high-shelf boost ("adds edge and definition").
 *   4. 4-band Active EQ, ±15 dB, flat at 0.5:
 *        Bass   low shelf  60 Hz | Lo-Mid peak 250 Hz
 *        Hi-Mid peak     1.0 kHz | Treble high shelf 4 kHz
 *   5. Boost : footswitchable preset gain, up to +15 dB, with a touch more
 *              growl when driven hard.
 *   6. Electronic Crossover + Master volumes: a Linkwitz-ish LP/HP split at the
 *        Crossover frequency (100 Hz .. 1.04 kHz). In Bi-Amp mode the low band
 *        is scaled by the 300W master and the high band by the 100W master and
 *        summed (the real head's two power amps feeding one signal here); in
 *        Full-range mode both masters act on the full-range signal.
 */
#include "DistrhoPlugin.hpp"
#include "Fk800Params.h"
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
    void setLowPass(float fc, float Q, float fs) {
        const float w0 = 6.2831853f * fc / fs, cw = std::cos(w0), sw = std::sin(w0);
        const float alpha = sw / (2.f * Q);
        const float a0 = 1 + alpha;
        b0 = ((1 - cw) * 0.5f) / a0;
        b1 = (1 - cw)          / a0;
        b2 = ((1 - cw) * 0.5f) / a0;
        a1 = (-2 * cw)         / a0;
        a2 = (1 - alpha)       / a0;
    }
    void setHighPass(float fc, float Q, float fs) {
        const float w0 = 6.2831853f * fc / fs, cw = std::cos(w0), sw = std::sin(w0);
        const float alpha = sw / (2.f * Q);
        const float a0 = 1 + alpha;
        b0 = ((1 + cw) * 0.5f) / a0;
        b1 = -(1 + cw)         / a0;
        b2 = ((1 + cw) * 0.5f) / a0;
        a1 = (-2 * cw)         / a0;
        a2 = (1 - alpha)       / a0;
    }
    void setBypass() { b0 = 1; b1 = b2 = a1 = a2 = 0; z1 = z2 = 0; }
};

// ── Tiny fixed-size Modified Nodal Analysis solver (RT-safe, no heap) ─────────
// Solves a small linear circuit (resistors, capacitor companions, ideal op-amps,
// voltage sources) per sample by Gaussian elimination. Node 0 = ground; nodes
// 1..nN are unknown voltages; nX aux currents follow (one per V-source / op-amp).
// Verified against a textbook RC low-pass (-3dB at 1/(2*pi*R*C)) and a
// non-inverting op-amp (gain = 1 + Rf/Rg).
struct Mna {
    static const int MAXN = 6;
    int sz, nn;
    double A[MAXN*MAXN], b[MAXN], x[MAXN];
    void init(int nN, int nX) { nn = nN; sz = nN + nX;
        for (int i = 0; i < sz*sz; ++i) A[i] = 0.0;
        for (int i = 0; i < sz; ++i) b[i] = 0.0; }
    inline void stampG(int a, int bb, double g) {
        if (a>0)  { A[(a-1)*sz+(a-1)]  += g; if (bb>0) A[(a-1)*sz+(bb-1)] -= g; }
        if (bb>0) { A[(bb-1)*sz+(bb-1)]+= g; if (a>0)  A[(bb-1)*sz+(a-1)] -= g; } }
    inline void R(int a, int bb, double r) { if (r < 1e-9) r = 1e-9; stampG(a, bb, 1.0/r); }
    inline void Isrc(int a, int bb, double I) { if (a>0) b[a-1] -= I; if (bb>0) b[bb-1] += I; }
    inline void Vsrc(int a, double V, int k) { int r = nn+k;
        if (a>0) { A[(a-1)*sz+r] += 1; A[r*sz+(a-1)] += 1; } b[r] = V; }
    inline void OpAmp(int np, int nnode, int no, int k) { int r = nn+k;
        if (no>0)    A[(no-1)*sz+r]    += 1;
        if (np>0)    A[r*sz+(np-1)]    += 1;
        if (nnode>0) A[r*sz+(nnode-1)] -= 1; }
    bool solve() { const int n = sz;
        for (int col = 0; col < n; ++col) {
            int piv = col; double mx = std::fabs(A[col*n+col]);
            for (int r = col+1; r < n; ++r) { double v = std::fabs(A[r*n+col]); if (v > mx) { mx = v; piv = r; } }
            if (mx < 1e-15) return false;
            if (piv != col) { for (int c = 0; c < n; ++c) { double t = A[col*n+c]; A[col*n+c] = A[piv*n+c]; A[piv*n+c] = t; }
                double t = b[col]; b[col] = b[piv]; b[piv] = t; }
            const double d = A[col*n+col];
            for (int r = 0; r < n; ++r) { if (r == col) continue; const double f = A[r*n+col]/d; if (f == 0) continue;
                for (int c = col; c < n; ++c) A[r*n+c] -= f*A[col*n+c]; b[r] -= f*b[col]; } }
        for (int i = 0; i < n; ++i) x[i] = b[i] / A[i*n+i];
        return true; } };

// ── GK 800RB INPUT/PREAMP stage — true component-level (nodal) model ─────────
// Bob Gallien sheet 60045A, U1: non-inverting amp, Rg=R2 4.7K, Rf=R3 1M
// (DC gain 1+R3/R2 ~ 214), C2 100pF across R3 = the warm HF voicing, and the
// op-amp output saturating at the +/-supply rails = the GK growl. Each R, C and
// the op-amp are solved as real circuit elements per sample (no tanh).
struct FkPreamp {
    float fs = 48000.f; double T = 1.0/48000.0;
    double c2v = 0.0, c2i = 0.0;            // C2 trapezoidal companion state
    void setFs(float s) { fs = (s > 0.f) ? s : 48000.f; T = 1.0 / fs; }
    void reset() { c2v = 0.0; c2i = 0.0; }
    inline float process(double vin) {
        const double R2 = 4700.0, R3 = 1.0e6, C2 = 100.0e-12, Vrail = 13.5;
        const double Geq = 2.0*C2/T, Ieq = Geq*c2v + c2i;
        Mna m; m.init(3, 2);                                  // nodes: 1 in, 2 inv, 3 out
        m.Vsrc(1, vin, 0); m.R(2, 0, R2); m.R(3, 2, R3);
        m.stampG(3, 2, Geq); m.Isrc(2, 3, Ieq);              // C2 across R3
        m.OpAmp(1, 2, 3, 1);                                  // + = in, - = inv, out = 3
        double vo = 0.0, vinv = 0.0;
        if (m.solve()) { vo = m.x[2]; vinv = m.x[1]; }
        if (std::fabs(vo) > Vrail) {                          // op-amp saturated -> rail
            Mna m2; m2.init(3, 2);
            m2.Vsrc(1, vin, 0); m2.R(2, 0, R2); m2.R(3, 2, R3);
            m2.stampG(3, 2, Geq); m2.Isrc(2, 3, Ieq);
            m2.Vsrc(3, (vo > 0 ? Vrail : -Vrail), 1);        // output held at the rail
            if (m2.solve()) { vo = m2.x[2]; vinv = m2.x[1]; }
        }
        const double v = vo - vinv;                          // advance C2 state
        const double i = Geq*(v - c2v) - c2i; c2i = i; c2v = v;
        return (float)vo;
    }
};

class Gk800Channel {
    float fs = 48000.f;
    Biquad loCut;                          // voicing: bass roll-off
    Biquad contour;                        // voicing: ~500 Hz notch
    Biquad hiBoost;                        // voicing: presence
    Biquad bqBass, bqLoMid, bqHiMid, bqTreble;   // 4-band active EQ
    // 2nd-order Butterworth (cascaded biquads, Q=0.707 then ~0.541) for a
    // 4th-order-ish crossover split that recombines cleanly.
    Biquad xLow1, xLow2, xHigh1, xHigh2;

    FkPreamp pre;                          // nodal input/preamp stage (the growl)
    float preDrive = 0.2f, preMakeup = 0.014f;
    float boostGain = 1.f;
    bool  boostOn = false;
    float g100 = 1.f, g300 = 1.f;          // master gains
    bool  biamp = false;

    static inline float softclip(float x) { return std::tanh(x); }
public:
    void setSampleRate(float s) { fs = (s > 0.f) ? s : 48000.f; pre.setFs(s); }
    void reset() {
        pre.reset();
        loCut.reset(); contour.reset(); hiBoost.reset();
        bqBass.reset(); bqLoMid.reset(); bqHiMid.reset(); bqTreble.reset();
        xLow1.reset(); xLow2.reset(); xHigh1.reset(); xHigh2.reset();
    }

    void setParams(float volume, float treble, float hiMid, float loMid, float bass,
                   float boostLevel, float xover, float master100, float master300,
                   bool pad, bool loCutOn, bool contourOn, bool hiBoostOn,
                   bool boostOnP, bool biampP) {
        // ── input / preamp: Volume drives the nodal preamp stage (clean at low
        //    Volume, op-amp rail-clipping growl when pushed). -10 dB pad cuts the
        //    drive into it. preMakeup normalises the ~13.5 V rail back to ~unity.
        const float padScale = pad ? 0.316f : 1.0f;
        preDrive  = padScale * (0.04f + volume * 0.24f);   // signal level (V) into U1
        preMakeup = 3.0f / 214.0f;                          // ~unity small-signal gain

        // ── voicing filters ──
        if (loCutOn)  loCut.setHighPass(110.f, 0.707f, fs);          else loCut.setBypass();
        if (contourOn) contour.setPeak(500.f, -11.f, 1.1f, fs);      else contour.setBypass();
        if (hiBoostOn) hiBoost.setHighShelf(2200.f, 6.5f, fs);       else hiBoost.setBypass();

        // ── 4-band active EQ, ±15 dB (0.5 = flat). Frequencies/Q derived from
        //    the preamp R/C (Bob Gallien sheet 60045A), which confirm the manual:
        //      Bass   : R30 12K + C16 .22uF  -> 1/(2pi*R*C) = 60.3 Hz (low shelf)
        //      Lo-Mid : C13/C14 .022uF net   -> ~250 Hz peak
        //      Hi-Mid : C11/C12 .0047uF (same topology) -> 250*(.022/.0047) ~ 1.17 kHz
        //      Treble : high shelf, design 4 kHz
        //    GK's mid bands are broad/gentle, so Q ~ 0.7 (not a narrow notch).
        bqBass.setLowShelf(60.f,      (bass   - 0.5f) * 30.f, fs);
        bqLoMid.setPeak(250.f,        (loMid  - 0.5f) * 30.f, 0.7f, fs);
        bqHiMid.setPeak(1150.f,       (hiMid  - 0.5f) * 30.f, 0.7f, fs);
        bqTreble.setHighShelf(4000.f, (treble - 0.5f) * 30.f, fs);

        // ── boost: preset, footswitchable, up to +15 dB ──
        boostOn   = boostOnP;
        boostGain = std::pow(10.f, (boostLevel * 15.f) / 20.f);   // 1 .. ~5.6

        // ── crossover split + masters ──
        biamp = biampP;
        const float fc = 100.f + 940.f * xover;        // 100 Hz .. 1040 Hz
        xLow1.setLowPass(fc, 0.707f, fs);  xLow2.setLowPass(fc, 0.707f, fs);
        xHigh1.setHighPass(fc, 0.707f, fs); xHigh2.setHighPass(fc, 0.707f, fs);
        g300 = master300 / 0.7f;   // low / 300W amp  (unity ~ 0.7)
        g100 = master100 / 0.7f;   // high / 100W amp
    }

    inline float process(float x) {
        // 1-2. INPUT/PREAMP — solved as the real op-amp circuit (nodal), so the
        //      growl is the actual U1 output clipping at the supply rails.
        float s = pre.process((double)(preDrive * x)) * preMakeup;

        // 3. voicing filters
        s = loCut.process(s);
        s = contour.process(s);
        s = hiBoost.process(s);

        // 4. active EQ
        s = bqBass.process(s);
        s = bqLoMid.process(s);
        s = bqHiMid.process(s);
        s = bqTreble.process(s);

        // 5. boost stage (extra growl only when pushed hard)
        if (boostOn) {
            s *= boostGain;
            if (boostGain > 2.0f) s = softclip(s) ;  // gentle clip at high boost
        }

        // 6. crossover + masters
        const float low  = xLow2.process(xLow1.process(s));
        const float high = xHigh2.process(xHigh1.process(s));
        if (biamp)
            return low * g300 + high * g100;          // bi-amp: split & re-sum
        return s * (0.5f * g300 + 0.5f * g100);       // full range: combined master
    }
};

class Fk800Plugin : public Plugin {
    Gk800Channel L, R;
    float fParams[kParamCount];
    void recalc() {
        const bool pad = fParams[kPad] > 0.5f, lc = fParams[kLoCut] > 0.5f;
        const bool ct  = fParams[kContour] > 0.5f, hb = fParams[kHiBoost] > 0.5f;
        const bool bo  = fParams[kBoostOn] > 0.5f, ba = fParams[kBiamp] > 0.5f;
        L.setParams(fParams[kVolume], fParams[kTreble], fParams[kHiMid], fParams[kLoMid], fParams[kBass],
                    fParams[kBoostLevel], fParams[kXover], fParams[kMaster100], fParams[kMaster300],
                    pad, lc, ct, hb, bo, ba);
        R.setParams(fParams[kVolume], fParams[kTreble], fParams[kHiMid], fParams[kLoMid], fParams[kBass],
                    fParams[kBoostLevel], fParams[kXover], fParams[kMaster100], fParams[kMaster300],
                    pad, lc, ct, hb, bo, ba);
    }
public:
    Fk800Plugin() : Plugin(kParamCount, 0, 0) {
        for (int i = 0; i < kParamCount; ++i) fParams[i] = kFk800Def[i];
        const float sr = (float)getSampleRate();
        L.setSampleRate(sr); R.setSampleRate(sr); L.reset(); R.reset(); recalc();
    }
protected:
    const char* getLabel()       const override { return "FreddyKrueger800BR"; }
    const char* getDescription() const override { return "Gallien-Krueger 800RB bass head model"; }
    const char* getMaker()       const override { return "RigBuilder"; }
    const char* getLicense()     const override { return "ISC"; }
    uint32_t    getVersion()     const override { return d_version(1, 0, 0); }
    int64_t     getUniqueId()    const override { return d_cconst('R', 'B', 'F', 'k'); }

    void initParameter(uint32_t i, Parameter& p) override {
        if (i >= (uint32_t)kParamCount) return;
        p.hints = kParameterIsAutomatable;
        if (i >= (uint32_t)kPad) p.hints |= kParameterIsBoolean;
        p.name = kFk800Names[i]; p.symbol = kFk800Symbols[i];
        p.ranges.min = kFk800Min[i]; p.ranges.max = kFk800Max[i]; p.ranges.def = kFk800Def[i];
    }
    float getParameterValue(uint32_t i) const override { return (i < (uint32_t)kParamCount) ? fParams[i] : 0.f; }
    void  setParameterValue(uint32_t i, float v) override { if (i < (uint32_t)kParamCount) { fParams[i] = v; recalc(); } }
    void  sampleRateChanged(double r) override { L.setSampleRate((float)r); R.setSampleRate((float)r); L.reset(); R.reset(); recalc(); }

    void run(const float** in, float** out, uint32_t frames) override {
        const float* iL = in[0]; const float* iR = in[1];
        float* oL = out[0]; float* oR = out[1];
        for (uint32_t i = 0; i < frames; ++i) { oL[i] = L.process(iL[i]); oR[i] = R.process(iR[i]); }
    }
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Fk800Plugin)
};

Plugin* createPlugin() { return new Fk800Plugin(); }

END_NAMESPACE_DISTRHO
