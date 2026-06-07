# SamplegVH140C — "Sampleg VH-140C"

White-box model of the **Ampeg VH-140C** (solid-state 2×70W stereo guitar head)
for Rocksmith's `Amp_AT120` (the gear map's generic name is "Marshall JCM 800",
but the folder + schematic are the Ampeg VH-140C). Parody brand **"Sampleg"**
(same as the Sampleg SBT-CL / V-4B). The face must never read "Ampeg".

Modelled component-by-component from `amps/Ampeg VH-140C (AT-120)/Ampeg_VH-140C.pdf`.

## Panel (1:1)
Chorus (Rate, Depth B, Depth A) · Reverb (Reverb B, Reverb A) · **Channel B**
(Level, High, Mid, Low, Gain) · **Channel A** (Level, High, Ultra Mid, Low, Gain)
· Power · Input Low/High · Effects Loop.

Solid-state (TL074/JRC4558 op-amps + 1N914 diode clipping, TDA power) — NO tubes.
- **Channel A** (clean): op-amp gain → light limit → active Low/Ultra-Mid/High EQ.
- **Channel B** (lead): big op-amp gain → tightened → hard 1N914 diode clip → active
  Low/Mid/High EQ. The famous tight, aggressive VH-140C metal/djent distortion.
- Per-channel spring reverb + BBD stereo chorus (opens L/R). High headroom, no sag.

## Rocksmith mapping
**RS Gain → CHANNEL B Gain**, Bass/Mid/Treble → Channel B Low/Mid/High. `_static`
pins the LEAD channel (B), reverb + chorus OFF, Channel A at a clean default.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/vh140_build TARGET_DIR=/tmp/vh140_out vst3
```
Copy into `vst/amps/SamplegVH140C.vst3/Contents/MacOS/SamplegVH140C` +
`codesign --force --deep --sign - SamplegVH140C.vst3`. UNIQUE_ID `Vh14`.
