# CitrusJimmyBean — "Citrus Jimmy Bean"

White-box model of the **Orange Jimmy Bean JB150** (1975-76) for Rocksmith's
`Amp_OrangeJimmyBean`. Parody brand **"Citrus"** (same as the Citrus AD200 /
OR50 / OR100). The face must never read "Orange".

No schematic exists, so this is a faithful **RECONSTRUCTION** from documented
facts: the JB150 is a **~150W SOLID-STATE** (transistor / op-amp) **TWIN-CHANNEL**
head with a built-in **TREMOLO** and a switchable **SUSTAIN** circuit, in
denim/leather styling. It is mostly a clean/loud solid-state amp; the SUSTAIN
circuit adds gain/dirt; the TREMOLO is an amplitude LFO.

## Panel (8 controls)
`VOLUME · BASS · TREBLE · SUSTAIN · SPEED · DEPTH · CHANNEL · BRIGHT`

SOLID-STATE (NO tube sag / asymTube cascade / power-tube model): an op-amp clean
preamp → Baxandall-ish **BASS/TREBLE** (no MID) → **BRIGHT** high-shelf switch →
**SUSTAIN** (a solid-state compressor + op-amp diode-style soft clip that adds
gain + sustain; 0 = clean, up = controlled fuzzy dirt + swell) → small
solid-state cab → **TREMOLO** (amplitude LFO on the output; SPEED = rate 2..8 Hz
via a per-sample phase accumulator, DEPTH = amount, 0 = OFF). **CHANNEL** 0/1 =
the two channels (Ch2 a touch brighter / more gain).

## Rocksmith mapping
**RS Gain → SUSTAIN** (the dirt/sustain), Bass/Treble → tone (no Mid).

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/jb15_build TARGET_DIR=/tmp/jb15_out vst3
```
Copy `CitrusJimmyBean.vst3` into `vst/amps/` +
`codesign --force --deep --sign - CitrusJimmyBean.vst3`. UNIQUE_ID `Jb15`.
