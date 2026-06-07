# Plexi — "Marsten Plexi"

White-box model of the **Marshall 1959 Super Lead 100W (Plexi / JMP)** for
Rocksmith's `Amp_MarshallPlexi`. Parody brand **"Marsten"** (the same Marshall
parody brand used for the DSL100 amp and the GM-2 / UV-1 pedals) — the in-app
face must never read "Marshall".

Modelled component-by-component from the local schematic
(`amps/Marshall Plexi/1959-01-60-02.pdf`, 1959SLP).

## Panel (1:1, left → right)
`PRESENCE · BASS · MIDDLE · TREBLE · LOUDNESS I · LOUDNESS II` + Power/Standby.

The 1959 is a **non-master-volume** amp — the two Loudness pots **are** the
gain. The HIGH TREBLE ("bright") channel runs a 5000pF bright cap across
Loudness I; the NORMAL channel is darker. Both mix (the jumpered plexi voice).

## Signal path
4 inputs → V1 gain stages (bright + normal) → Loudness mix → V2 recovery +
cathode follower → **Marshall tone stack** (Treble 250K / Bass 1M / Middle 25K,
**33K** slope, **500pF** treble cap — the Marshall values vs the Bassman's 56K /
250pF) → long-tail PI → **4× EL34 (~100W)** with earlier, more compressed,
midrange-grind breakup than the Bassman's 6L6 → output transformer. PRESENCE
taps the power-amp NFB.

## Rocksmith mapping
No gain knob, so **RS Gain → Loudness I** (clean → crunch → roar, matching the
`gain_variants` G3/G5/G10 split). Treble/Bass/Mid → tone stack, Pres → Presence.
Loudness II sits at a musical jumpered blend via `_static` and stays editable.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/plexi_build TARGET_DIR=/tmp/plexi_out vst3
```
Copy the binary into `vst/amps/Plexi.vst3/Contents/MacOS/Plexi` and
`codesign --force --deep --sign - Plexi.vst3`.

UNIQUE_ID `Pl59`.
