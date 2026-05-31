/*
 * Shared graphic-EQ UI (DPF NanoVG): a stompbox body (coloured like the real
 * pedal each gear models) with a recessed dark panel holding N vertical faders,
 * one per band, labelled by frequency with the dB value, plus a footswitch.
 * Drag a fader up/down to cut/boost.
 *
 * The per-pedal Bands.h defines kEqBands/kEqFreqs/kEqNames/EQ_PLUGIN_LABEL/EQ_DB
 * and the body accent colour EQ_ACR/EQ_ACG/EQ_ACB (the real pedal's colour).
 */
#include "DistrhoUI.hpp"
#include <cmath>
#include <cstdio>

#ifndef EQ_ACR
#define EQ_ACR 90
#define EQ_ACG 150
#define EQ_ACB 230
#endif

START_NAMESPACE_DISTRHO

class GraphicEqUI : public UI
{
    float fValues[kEqBands];
    int   fDrag;
    bool  fEditing = false;

    float scale()  const { return getWidth() / 460.0f; }
    // recessed slider panel
    float panelX() const { return getWidth()  * 0.07f; }
    float panelW() const { return getWidth()  * 0.86f; }
    float panelY() const { return getHeight() * 0.20f; }
    float panelH() const { return getHeight() * 0.52f; }
    float colW()   const { return panelW() / (float)kEqBands; }
    float colX(int i) const { return panelX() + (i + 0.5f) * colW(); }
    float trackTop()    const { return panelY() + getHeight() * 0.06f; }
    float trackBottom() const { return panelY() + panelH() - getHeight() * 0.07f; }

    float valToY(float v) const { return trackTop() + (1.0f - v) * (trackBottom() - trackTop()); }
    float yToVal(double y) const {
        float v = 1.0f - (float)((y - trackTop()) / (trackBottom() - trackTop()));
        return v < 0.f ? 0.f : v > 1.f ? 1.f : v;
    }
    int colAt(double px, double py) const {
        if (py < panelY() || py > panelY() + panelH()) return -1;
        int i = (int)((px - panelX()) / colW());
        return (i >= 0 && i < kEqBands) ? i : -1;
    }
    // auto text colour for legibility on the body accent
    Color bodyText() const {
        const float lum = 0.299f*EQ_ACR + 0.587f*EQ_ACG + 0.114f*EQ_ACB;
        return lum > 140.f ? Color(28, 28, 32) : Color(238, 240, 246);
    }
public:
    GraphicEqUI() : UI(DISTRHO_UI_DEFAULT_WIDTH, DISTRHO_UI_DEFAULT_HEIGHT), fDrag(-1) {
        loadSharedResources();
        for (int i = 0; i < kEqBands; ++i) fValues[i] = 0.5f;
        setGeometryConstraints(360, 260, true, false);
    }
protected:
    void parameterChanged(uint32_t index, float value) override {
        if (index < (uint32_t)kEqBands) { fValues[index] = value; repaint(); }
    }
    void onNanoDisplay() override {
        const float W = getWidth(), H = getHeight(), f = scale();
        fontFace(NANOVG_DEJAVU_SANS_TTF);
        // backdrop
        beginPath(); rect(0, 0, W, H); fillColor(Color(14, 15, 18)); fill();
        // stompbox body — accent (real pedal colour) → darker bottom
        const float bx = 8*f, by = 8*f, bw = W - 16*f, bh = H - 16*f;
        Paint body = linearGradient(0, by, 0, by + bh,
                                    Color(EQ_ACR, EQ_ACG, EQ_ACB),
                                    Color(EQ_ACR/3 + 14, EQ_ACG/3 + 14, EQ_ACB/3 + 14));
        beginPath(); roundedRect(bx, by, bw, bh, 18*f); fillPaint(body); fill();
        beginPath(); roundedRect(bx, by, bw, bh, 18*f); strokeColor(Color(255,255,255,45)); strokeWidth(2*f); stroke();

        const Color tc = bodyText();
        // title + LED
        textAlign(ALIGN_LEFT | ALIGN_TOP);
        fontSize(20*f); fillColor(tc); text(W*0.07f, H*0.07f, EQ_PLUGIN_LABEL, NULL);
        beginPath(); circle(W*0.90f, H*0.10f, 5*f); fillColor(Color(255, 80, 70)); fill();

        // recessed slider panel
        beginPath(); roundedRect(panelX(), panelY(), panelW(), panelH(), 10*f);
        fillColor(Color(24, 25, 29)); fill();
        beginPath(); roundedRect(panelX(), panelY(), panelW(), panelH(), 10*f);
        strokeColor(Color(0,0,0,120)); strokeWidth(1.5f*f); stroke();

        const float tT = trackTop(), tB = trackBottom(), midY = (tT + tB) * 0.5f;
        for (int i = 0; i < kEqBands; ++i) {
            const float cx = colX(i);
            beginPath(); roundedRect(cx - 2.5f*f, tT, 5*f, tB - tT, 2.5f*f); fillColor(Color(52, 54, 64)); fill();
            beginPath(); rect(cx - 7*f, midY, 14*f, 1.0f*f); fillColor(Color(90, 95, 110)); fill();
            const float hy = valToY(fValues[i]);
            const float y0 = std::fmin(hy, midY), y1 = std::fmax(hy, midY);
            beginPath(); roundedRect(cx - 2.5f*f, y0, 5*f, y1 - y0, 2.5f*f);
            fillColor(Color(EQ_ACR, EQ_ACG, EQ_ACB)); fill();
            beginPath(); roundedRect(cx - 11*f, hy - 5*f, 22*f, 10*f, 3*f); fillColor(Color(225, 228, 236)); fill();
            // dB value (above panel) + freq (below panel) — on body, auto-contrast
            char buf[16]; const float db = (fValues[i] - 0.5f) * (2.0f * EQ_DB);
            std::snprintf(buf, sizeof(buf), "%+.0f", db);
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            fontSize(9.5f*f); fillColor(Color(160, 170, 190)); text(cx, panelY() + 3*f, buf, NULL);
            textAlign(ALIGN_CENTER | ALIGN_TOP);
            fontSize(10*f); fillColor(tc); text(cx, panelY() + panelH() + 5*f, kEqNames[i], NULL);
        }

        // footswitch
        beginPath(); circle(W*0.5f, H*0.90f, 16*f); fillColor(Color(205,208,213)); fill();
        beginPath(); circle(W*0.5f, H*0.90f, 16*f); strokeColor(Color(110,114,120)); strokeWidth(2.5f*f); stroke();
        beginPath(); circle(W*0.5f, H*0.90f, 10*f); fillColor(Color(155,159,166)); fill();
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
