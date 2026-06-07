# JTM45 — "Marsten JTM45"

White-box model of the **Marshall JTM45** (~30W, 2x KT66 + GZ34 tube rectifier)
for Rocksmith's `Amp_MarshallJTM45`. Parody brand **"Marsten"** (the same
Marshall parody brand used for the Plexi / DSL100) — the in-app face must never
read "Marshall".

Modelled component-by-component from the local schematic
(`amps/Marshall JTM45/Marshall_jtm45_readable.pdf`, JTM45.DGM issue 7).

## Panel (1:1, left → right)
`PRESENCE · BASS · MIDDLE · TREBLE · LOUDNESS 1 · LOUDNESS 2` + Power/Standby.

The JTM45 is a **non-master-volume** amp — the two Loudness pots **are** the
gain. The HIGH TREBLE ("bright") channel runs a 500pF bright cap across
Loudness 1; the NORMAL channel is darker. Both mix (the jumpered tone).

## Signal path
4 inputs → V1 gain stages (bright + normal) → Loudness mix → V2/V3 recovery +
cathode follower → **Marshall/Bassman tone stack** (Treble 250K / Bass 1M /
Middle 25K, **56K** slope, **270pF** treble cap — the early JTM45 values vs the
Plexi's 33K / 500pF) → long-tail PI → **2× KT66 (~30W) + GZ34 tube rectifier**
with warmer, softer, MUCH more sag, earlier breakup and a darker top than the
100W 4× EL34 Plexi → output transformer. PRESENCE (5K) taps the power-amp NFB.

## Predecessor of the Plexi
The JTM45 is the direct ancestor of the 1959 Super Lead Plexi (and descendant of
the Fender Bassman 5F6-A): **same** jumper-input + dual-Loudness topology and
the Marshall FMV tone stack. This model reuses the Plexi DSP shape and changes
only the power amp (KT66 + GZ34 sag, ~30W) and the tone-stack values.

## Rocksmith mapping
No gain knob, so **RS Gain → Loudness 1** (clean → crunch → roar). Treble/Bass/
Mid → tone stack, Pres → Presence. `kInput` = Bright(0) / Both-jumpered(0.5) /
Normal(1).

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/jt45_build TARGET_DIR=/tmp/jt45_out vst3
```
Copy `MarstenJTM45.vst3` into `vst/amps/` and `codesign --force --deep --sign -`.

UNIQUE_ID `Jt45`.
