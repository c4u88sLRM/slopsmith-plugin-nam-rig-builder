# CitrusAD50 — "Citrus AD50"

White-box model of the **Orange AD50** (Custom Shop) for Rocksmith's
`Amp_OrangeAD50`. Parody brand **"Citrus"** (same lineage as the Citrus OR50 /
OR100 / AD200). The in-app face must never read "Orange".

Orange describes the AD50 as aiming to be "like the old OR120 with more gain than
the AD30" — a SIMPLE British EL34 tube head. It reuses the Citrus OR50 preamp/
power voice (the same modern Orange EL34 lineage) but:
- drops the FMV tone stack + Middle/Depth controls for a **2-band shelving EQ**
  (Bass low-shelf + Treble high-shelf, NO middle),
- adds a **hotter GAIN preamp stage** (one extra hot 12AX7 stage — more preamp
  gain than the OR series),
- makes **PRESENCE** a controllable power-amp NFB high-shelf,
- adds **SUSTAIN** (a footswitchable EQ-bypass gain/sustain boost) and the
  **Class A / AB** power switch (50W Class AB / 30W Class A — Class A breaks up
  earlier and compresses more).

## Panel
`GAIN · BASS · TREBLE · PRESENCE · MASTER · SUSTAIN · CLASS A`

Single-channel British EL34 head: 4× ECC83 gain stages (hotter than the OR) →
2-band shelving EQ → ECC81 PI → 2× EL34 (~50W AB / ~30W A) → output. A thick,
midrange-forward "Orange" voice with more gain/saturation than the OR100/OR50.

## Rocksmith mapping
**RS Gain → GAIN**, Bass/Treble → 2-band EQ, **Presence → Presence**. `_static`
pins Master + Sustain + Class A to musical defaults (Master past noon, Sustain
off, Class AB / 50W). All editable on the face.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/ad50_build TARGET_DIR=/tmp/ad50_out vst3
```
Copy `CitrusAD50.vst3` into `vst/amps/` +
`codesign --force --deep --sign - CitrusAD50.vst3`. UNIQUE_ID `Ad50`.
