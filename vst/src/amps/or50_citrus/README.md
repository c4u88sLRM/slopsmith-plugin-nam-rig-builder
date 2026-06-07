# CitrusOR50 — "Citrus OR50"

White-box model of the **Orange OR50** (vintage "Graphic" single-channel head)
for Rocksmith's `Amp_OrangeOR50`. Parody brand **"Citrus"** (same as the Citrus
AD200). The face must never read "Orange".

There is no standalone OR50 schematic, so it's **reconstructed** from the closest
documented Orange references:
- `amps/Orange OR100/Orange100.pdf` — the OR100 the 2×EL34 ~50W power amp scales from.
- `amps/Orange OR50/Orange OR15 SCH 115V-230V.pdf` — the modern "OR" preamp + tone
  stack (TREBLE 250K, BASS **A500K**, MIDDLE 25K, treble cap **~1nF** = 470p‖470p).
- `amps/Orange OR50/OR30 preamp.jpg` — confirms the **3-stage 12AX7 cascade** with a
  partial (1µF) cathode bypass → the midrange-forward Orange honk.
- `amps/Orange OR50/Orange Retro 50 Layout.gif` — 3×12AX7 + 2×EL34 + GZ34, ~50W.

## Panel (Orange "pics-only" graphics)
`GAIN (HF Drive) · BASS · MIDDLE (FAC) · TREBLE · DEPTH · VOLUME` + a FULL/HALF
output-power switch.

Single-channel British EL34 head: 3× ECC83 gain stages → tone stack + DEPTH (the
bass-cap voicing) → ECC81 PI → 2× EL34 (~50W, FULL or HALF) → output. A thick,
midrange-forward "Orange" voice (the doom/stoner chunk). VOLUME is the master.

## Rocksmith mapping
**RS Gain → GAIN**, Bass/Mid/Treble → tone stack. `_static` pins Volume + Depth
to musical defaults (FULL power). All editable on the face.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/or50_build TARGET_DIR=/tmp/or50_out vst3
```
Copy into `vst/amps/CitrusOR50.vst3/Contents/MacOS/CitrusOR50` +
`codesign --force --deep --sign - CitrusOR50.vst3`. UNIQUE_ID `Or10`.
