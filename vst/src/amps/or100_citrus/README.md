# CitrusOR100 — "Citrus OR100"

White-box model of the **Orange OR100** (vintage "Graphic" single-channel head)
for Rocksmith's `Amp_OrangeOR100`. Parody brand **"Citrus"** (same as the Citrus
AD200). The face must never read "Orange".

Modelled component-by-component from the **complete** hand-drawn schematic
`amps/Orange OR100/Orange100.pdf` ("Orange Model OR100 serial 94").

## Panel (Orange "pics-only" graphics)
`GAIN (HF Drive) · BASS · MIDDLE (FAC) · TREBLE · DEPTH · VOLUME` + a FULL/HALF
output-power switch.

Single-channel British EL34 head: ECC83 gain stages → tone stack + DEPTH (the
bass-cap voicing) → ECC81 PI → 4× EL34 (~100W, FULL or HALF) → output. A thick,
midrange-forward "Orange" voice (the doom/stoner chunk). VOLUME is the master.

## Rocksmith mapping
**RS Gain → GAIN**, Bass/Mid/Treble → tone stack. `_static` pins Volume + Depth
to musical defaults (FULL power). All editable on the face.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/or100_build TARGET_DIR=/tmp/or100_out vst3
```
Copy into `vst/amps/CitrusOR100.vst3/Contents/MacOS/CitrusOR100` +
`codesign --force --deep --sign - CitrusOR100.vst3`. UNIQUE_ID `Or10`.
