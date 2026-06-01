/*
 * Shared graphic-EQ UI (DPF NanoVG) — styled like a Boss GE-7 / GEB-7 compact,
 * matching the bundled "chief" pedals: coloured body, a black recessed plate at
 * the top holding the vertical faders (one per band), and the big body-colour
 * treadle footswitch carrying the engraved name (with a black step pad). Drag a
 * fader up/down to cut/boost. Copyright-free (no brand/model name).
 *
 * Per-pedal Bands.h defines kEqBands/kEqFreqs/kEqNames/EQ_PLUGIN_LABEL/EQ_DB and
 * the body colour EQ_ACR/EQ_ACG/EQ_ACB. Optionally EQ_NAME1/EQ_NAME2 for a
 * two-word diagonal treadle name (else EQ_PLUGIN_LABEL is centred).
 */
#include "DistrhoUI.hpp"
#include "_shared/fonts_data.hpp"
#include <cmath>
#include <cstdio>

#ifndef EQ_ACR
#define EQ_ACR 190
#define EQ_ACG 192
#define EQ_ACB 188
#endif

START_NAMESPACE_DISTRHO

class GraphicEqUI : public UI
{
    float fValues[kEqBands];
    int   fDrag;
    bool  fEditing = false;
    int   fName = -1, fLbl = -1;

    float scale()  const { return getWidth() / 460.0f; }
    float panelX() const { return getWidth()  * 0.075f; }
    float panelW() const { return getWidth()  * 0.85f; }
    float panelY() const { return getHeight() * 0.10f; }
    float panelH() const { return getHeight() * 0.235f; }
    float colW()   const { return panelW() / (float)kEqBands; }
    float colX(int i) const { return panelX() + (i + 0.5f) * colW(); }
    float trackTop()    const { return panelY() + getHeight() * 0.035f; }
    float trackBottom() const { return panelY() + panelH() - getHeight() * 0.045f; }

    float valToY(float v) const { return trackTop() + (1.0f - v) * (trackBottom() - trackTop()); }
    float yToVal(double y) const {
        float v = 1.0f - (float)((y - trackTop()) / (trackBottom() - trackTop()));
        return v < 0.f ? 0.f : v > 1.f ? 1.f : v;
    }
    int colAt(double px, double py) const {
        if (py < panelY()-6 || py > panelY() + panelH()+6) return -1;
        int i = (int)((px - panelX()) / colW());
        return (i >= 0 && i < kEqBands) ? i : -1;
    }
    Color bodyText() const {
        const float lum = 0.299f*EQ_ACR + 0.587f*EQ_ACG + 0.114f*EQ_ACB;
        return lum > 140.f ? Color(34, 34, 38) : Color(238, 240, 246);
    }
    static int cl(int v){ return v<0?0:(v>255?255:v); }
public:
    GraphicEqUI() : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT), fDrag(-1) {
        loadSharedResources();
        fName = createFontFromMemory("pk_serif",  pk_serif_ttf,  pk_serif_ttf_len,  false); // Crete Round
        fLbl  = createFontFromMemory("pk_barlow", pk_barlow_ttf, pk_barlow_ttf_len, false);
        for (int i = 0; i < kEqBands; ++i) fValues[i] = 0.5f;
        setGeometryConstraints(360, 280, true, false);
    }
private:
    void engrave(float cx, float cy, float size, const char* s) {
        fontFaceId(fName >= 0 ? fName : fLbl); textAlign(ALIGN_CENTER | ALIGN_MIDDLE);
        fontSize(size*scale()); fillColor(Color(16,16,20));
        text(getWidth()*cx, getHeight()*cy, s, NULL);
    }
protected:
    void parameterChanged(uint32_t index, float value) override {
        if (index < (uint32_t)kEqBands) { fValues[index] = value; repaint(); }
    }
    void onNanoDisplay() override {
        const float W = getWidth(), H = getHeight(), f = scale(), m = 7*f;
        const int R = EQ_ACR, G = EQ_ACG, B = EQ_ACB;
        beginPath(); rect(0, 0, W, H); fillColor(Color(11, 11, 13)); fill();
        Paint body = linearGradient(0, m, 0, H-m, Color(cl(R+16),cl(G+16),cl(B+16)), Color(cl(R-14),cl(G-14),cl(B-14)));
        beginPath(); roundedRect(m, m, W-2*m, H-2*m, 12*f); fillPaint(body); fill();
        beginPath(); roundedRect(m, m, W-2*m, H-2*m, 12*f); strokeColor(Color(0,0,0,110)); strokeWidth(2*f); stroke();

        const Color tc = bodyText();
        // black recessed plate for the faders (the top section)
        beginPath(); roundedRect(panelX()-8*f, panelY()-8*f, panelW()+16*f, panelH()+22*f, 7*f);
        fillColor(Color(20,20,22)); fill();
        // LED top-centre
        beginPath(); circle(W*0.5f, H*0.066f, 4.5f*f); fillColor(Color(224,70,58)); fill();

        const float tT = trackTop(), tB = trackBottom(), midY = (tT + tB) * 0.5f;
        for (int i = 0; i < kEqBands; ++i) {
            const float cx = colX(i);
            beginPath(); roundedRect(cx - 2.2f*f, tT, 4.4f*f, tB - tT, 2.2f*f); fillColor(Color(48, 50, 58)); fill();
            beginPath(); rect(cx - 5*f, midY, 10*f, 1.0f*f); fillColor(Color(84, 88, 102)); fill();
            const float hy = valToY(fValues[i]);
            const float y0 = std::fmin(hy, midY), y1 = std::fmax(hy, midY);
            beginPath(); roundedRect(cx - 2.2f*f, y0, 4.4f*f, y1 - y0, 2.2f*f); fillColor(Color(cl(R-30),cl(G-30),cl(B-30))); fill();
            Paint capp = linearGradient(0, hy-4.5f*f, 0, hy+4.5f*f, Color(232,234,238), Color(150,153,160));
            beginPath(); roundedRect(cx - 9*f, hy - 4.5f*f, 18*f, 9*f, 2*f); fillPaint(capp); fill();
            beginPath(); rect(cx - 9*f, hy-0.5f*f, 18*f, 1.0f*f); fillColor(Color(60,62,68)); fill();
            // freq label (on the body, just under the black plate)
            fontFaceId(fLbl); textAlign(ALIGN_CENTER | ALIGN_TOP);
            fontSize(8*f); fillColor(tc); text(cx, panelY() + panelH() + 5*f, kEqNames[i], NULL);
        }

        // treadle (body colour, full width) + black step pad
        const float tx = m+4*f, tw = W-2*m-8*f, tyTop = H*0.44f, tBot = H - m - 6*f;
        Paint tre = linearGradient(0, tyTop, 0, tBot, Color(cl(R-2),cl(G-2),cl(B-2)), Color(cl(R-16),cl(G-16),cl(B-16)));
        beginPath(); roundedRect(tx, tyTop, tw, tBot - tyTop, 12*f); fillPaint(tre); fill();
        beginPath(); roundedRect(tx, tyTop, tw, 10*f, 12*f); fillColor(Color(255,255,255,20)); fill();
        beginPath(); roundedRect(tx, tyTop, tw, tBot - tyTop, 12*f); strokeColor(Color(0,0,0,120)); strokeWidth(1.6f*f); stroke();
        beginPath(); roundedRect(tx+12*f, H*0.83f, tw-24*f, tBot-9*f-H*0.83f, 9*f); fillColor(Color(20,20,22)); fill();
        // engraved name on the treadle
#if defined(EQ_NAME1) && defined(EQ_NAME2)
        engrave(0.31f, 0.555f, 40, EQ_NAME1);
        engrave(0.64f, 0.665f, 40, EQ_NAME2);
#else
        engrave(0.5f, 0.61f, 40, EQ_PLUGIN_LABEL);
#endif
    }
    bool onMouse(const MouseEvent& ev) override {
        if (ev.button != 1) return false;
        if (ev.press) {
            const int i = colAt(ev.pos.getX(), ev.pos.getY());
            if (i >= 0) { fDrag = i; setFromY(i, ev.pos.getY()); return true; }
        } else if (fDrag >= 0) { editParameter(fDrag, false); fEditing = false; fDrag = -1; return true; }
        return false;
    }
    bool onMotion(const MotionEvent& ev) override {
        if (fDrag >= 0) { setFromY(fDrag, ev.pos.getY()); return true; }
        return false;
    }
private:
    void setFromY(int i, double y) {
        if (!fEditing) { editParameter(i, true); fEditing = true; }
        const float v = yToVal(y);
        fValues[i] = v; setParameterValue(i, v); repaint();
    }
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphicEqUI)
};

UI* createUI() { return new GraphicEqUI(); }

END_NAMESPACE_DISTRHO
