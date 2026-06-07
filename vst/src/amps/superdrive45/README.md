# Superdrive45 — "Ganddi Superdrive 45"

White-box model of the **Budda Superdrive 45 Series II** for Rocksmith's
`Amp_BT45` (the gear map's generic name is "Vox AC50", but the curated
`gain_variants` are *Budda Super Drive Clean/Crunch/Lead Modern* — it is the
Budda). Parody brand **"Ganddi"** — the oval badge + face must never read
"Budda".

Modelled component-by-component from the local manual + schematic
(`Superdrive45_manual.pdf`, `BuddaSuperdrive80Schematic.jpg`).

## Panel (1:1, left → right)
`ON/OFF · GO/REST · MASTER(pull CHANNEL) · BASS · MID(pull MODERN) · TREBLE ·
DRIVE · RHYTHM(pull BRITE) · INPUT · FOOTSWITCH`

Two channels off one tone stack (3× 12AX7 + 2× KT66 ~45W + 5AR4):
- **RHYTHM** — clean → edge, gain via the RHYTHM knob (+ pull **BRITE** treble boost).
- **HI-GAIN** — the lead "Drive" voice, cascaded 12AX7 gain via **DRIVE** (+ pull
  **MODERN**: scoops mids, lifts bass+treble — hi-gain only).
- **MASTER** pull selects the channel (in = Rhythm / out = Hi-gain).

Shared Bass/Mid/Treble tone stack (Budda values: Treble 500K/220pF, Bass
500K/22nF, Mid 50K/22nF, 56K slope) → long-tail PI → 2× KT66 → OT with a fixed
presence NFB.

## Rocksmith mapping
**RS Gain → DRIVE**; Bass/Mid/Treble → tone stack. `_static` pins **Channel =
Hi-gain** and **Modern = ON** (the gain_variants were captured "Modern");
Master/Rhythm sit at musical defaults. All editable by hand (incl. the three
pull-pots on the face).

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/sd45_build TARGET_DIR=/tmp/sd45_out vst3
```
Copy the binary into `vst/amps/Superdrive45.vst3/Contents/MacOS/Superdrive45`
and `codesign --force --deep --sign - Superdrive45.vst3`.

UNIQUE_ID `Gd45`.
