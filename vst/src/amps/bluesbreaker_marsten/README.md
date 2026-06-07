# MarstenBluesbreaker — "Marsten Bluesbreaker"

White-box model of the **Marshall 1962 Bluesbreaker** combo for Rocksmith's
`Amp_Marshall1962Bluesbreaker`. Parody brand **"Marsten"** (Marshall → Marsten,
same as the Plexi / DSL100 / JCM800). The face must never read "Marshall".

Modelled component-by-component from
`amps/Marshall Bluesbreaker/Marshall_bluesbreaker_reissue_45w_1962.pdf`
(the "1962 Reissue Valve Tremolo Combo" preamp + power + tremolo schematics).

## What it is
The 1962 is a **JTM45 with a power-amp tremolo**. It is a NON-MASTER amp whose
two **Loudness (Volume I/II)** pots ARE the gain. Lineage: Bassman 5F6-A →
JTM45 → 1962, so it uses the Marshall TMB tone stack with the **JTM45 values**
(Treble 220K / Bass 1M / Middle 22K, **56K slope, 220pF treble cap**) into a
warm, sag-y **2× 5881/KT66 (~30W combo)** power amp with a **GZ34** rectifier
(more sag / earlier breakup than the 100W EL34 plexi). PRESENCE (VR6 4k7) taps
the power-amp NFB. The **TREMOLO** (V6 phase-shift LFO + J174 FET) amplitude-
modulates the output: **SPEED** = rate (~2..7.5 Hz), **INTENSITY** = depth
(0 = OFF). The LFO is a deterministic per-sample phase accumulator.

## Panel (1:1, 9 knobs)
`SPEED · INTENSITY · PRESENCE · BASS · MIDDLE · TREBLE · LOUDNESS 1 · LOUDNESS 2
· INPUT`

LOUDNESS 1 = Volume I (bright/lead channel, brighter cap), LOUDNESS 2 = Volume
II (normal channel). INPUT selects the cable/channel: Ch I / Both-jumpered /
Ch II.

## Rocksmith mapping
**RS Gain → LOUDNESS 1** (clean→crunch→roar), Bass/Mid/Treble → tone stack,
Pres → Presence. Tremolo **OFF by default** (Intensity 0); Speed/Intensity stay
editable by hand. `_static` pins Loudness 2 to a musical jumpered blend.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/bb62_build TARGET_DIR=/tmp/bb62_out vst3
```
Copy into `vst/amps/MarstenBluesbreaker.vst3` +
`codesign --force --deep --sign - MarstenBluesbreaker.vst3`.
UNIQUE_ID `Bb62`.
