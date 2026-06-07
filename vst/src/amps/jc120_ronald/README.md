# RonaldJC120 — "Ronald JC-120"

Bundled RigBuilder amp for Rocksmith's `Amp_CS120`. Models the **Roland JC-120
"Jazz Chorus"** — the Effect channel front panel, 1:1. Parody brand **"Ronald"**
(Roland → Ronald); the in-app face must never read "Roland".

Local reference (modelled from the service manual):

- `amps/Roland JC-120 (CS-120)/Roland JC-120, JC-160 (79-5) Jazz Chorus Service Manual.pdf`

## Panel (9 controls)

A **solid-state** 120W amp (NJM4558 / M5218-class op-amps + 2SD736A transistor
power amp — no tubes), 2× 30cm (12") speakers: a clean, high-headroom preamp
with a diode-clipping distortion, a passive tone stack (+ a fixed BRIGHT lift),
a spring reverb (Z-3F unit), and the signature analogue BBD (MN3007) **stereo
chorus**.

- **Volume**
- **Treble**, **Middle**, **Bass** (the EQUALIZER section)
- **Distortion** (diode-clip drive — clean at 0)
- **Reverb** (spring)
- **Speed**, **Depth** + **Chorus** 3-way (Off / Chorus / Vibrato)

Tone control range (per the spec, Volume at center): Treble 17 dB @10 kHz,
Middle 13 dB @350 Hz, Bass 14 dB @50 Hz, Bright 5 dB @10 kHz.

The chorus opens the stereo image (dry on one side, the pitch-modulated wet on
the other — the famous wide Jazz Chorus shimmer).

## Rocksmith mapping

`Amp_CS120` exposes Gain/Bass/Mid/Treble. **Gain → Distortion** (clean at 0 →
the gritty solid-state drive); Bass/Mid/Treble → tone stack. Reverb + Chorus sit
OFF for songs (Rocksmith adds those via its own pedals/racks) and stay editable
by hand (`_static` in `rs_knob_to_vst_param.json`).

## Build

```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/jc12_build TARGET_DIR=/tmp/jc12_out vst3
```

Copy into `vst/amps/RonaldJC120.vst3` + `codesign --force --deep --sign -
RonaldJC120.vst3`. UNIQUE_ID `Jc12`.
