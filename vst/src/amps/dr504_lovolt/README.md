# LovoltDR504 — "Lovolt DR504"

White-box model of the **Hiwatt DR504 "Custom Hiwatt 50"** for Rocksmith's
`Amp_HG500` (the gear map's generic name is "Peavey 5150", but the folder +
schematic are the Hiwatt DR504). Parody brand **"Lovolt"** (Hiwatt = high watt
-> Lovolt = low volt; the same brand as the Lovolt 100). The face must never
read "Hiwatt".

Modelled component-by-component from `amps/Hiwatt DR504 (HG500)/DR504_Complete.pdf`.

## Panel (1:1, left → right)
`NORMAL VOL · BRILLIANT VOL · BASS · TREBLE · MIDDLE · PRESENCE · MASTER VOL`
+ 4 inputs (Normal Hi/Lo, Brilliant Hi/Lo, jumperable) + STANDBY/MAINS.

A high-headroom, clean-and-loud EL34 amp (3× ECC83 + ECC81 PI + 2× EL34 ~50W).
Two jumperable channels (NORMAL + BRILLIANT bright cap) → shared tone stack
(Bass 500K, Treble 250K, **Middle 100K** = the strong Hiwatt mids, 56K slope) →
MASTER VOLUME → EL34 power amp; PRESENCE taps the NFB. Breakup comes mostly from
cranking the MASTER (it stays clean far longer than a Plexi).

## Rocksmith mapping
No gain knob, so **RS Gain → BRILLIANT VOL** (the breakup driver); Bass/Mid/Treble
→ tone stack, Pres → Presence. Input pinned to BOTH (jumpered) with Normal Vol +
Master at musical defaults via `_static`.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/dr504_build TARGET_DIR=/tmp/dr504_out vst3
```
Copy into `vst/amps/LovoltDR504.vst3/Contents/MacOS/LovoltDR504` and
`codesign --force --deep --sign - LovoltDR504.vst3`. UNIQUE_ID `Lv50`.
