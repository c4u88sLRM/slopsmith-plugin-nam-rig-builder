# MarkII — "Silla Boogie Mark II"

White-box model of the **Mesa/Boogie Mark IIB** for Rocksmith's `Amp_CA38`.
Parody brand **"Silla"** (Mesa = mesa/table → Silla = chair). The face must
never read "Mesa" or "Boogie".

> The RS gear-map name for `Amp_CA38` said "Mark IV", but the `CA_38` folder,
> the local schematic (`boogie_mkii.pdf`) and the photos are the **Mark IIB** —
> so this replaces the earlier Mark IV build.

Modelled component-by-component from `amps/Mesa Mark II (CA_38)/boogie_mkii.pdf`
(MESA/Boogie Mark II Rev.B) + the panel photos `1.jpg`/`2.jpg`.

## Panel (1:1, per the photos — no graphic EQ)
`VOLUME 1 (pull BRIGHT) · TREBLE (pull SHIFT) · BASS · MIDDLE · MASTER 1 (pull
GAIN BOOST) · LEAD DRIVE (pull LEAD) · LEAD MASTER (pull BRIGHT) · REVERB` +
100/60 RMS · STANDBY · POWER.

Two voices off ONE scooped Fender-derived tone stack placed before the gain
(the Mark signature): RHYTHM (Volume 1 → Master 1) and LEAD (Lead Drive cascade
→ Lead Master), picked by the LEAD relay. 4× 6L6GC (~100W / 60W half) + fixed
presence NFB + spring reverb.

## Rocksmith mapping
**RS Gain → LEAD DRIVE**, Bass/Mid/Treble → tone stack. `_static` pins the LEAD
channel, masters/volume at musical defaults, reverb + pulls off, FULL power.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/mk2_build TARGET_DIR=/tmp/mk2_out vst3
```
Copy into `vst/amps/MarkII.vst3` + `codesign --force --deep --sign - MarkII.vst3`.
UNIQUE_ID `Mk2c`.
