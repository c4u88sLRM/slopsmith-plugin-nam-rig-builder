# PolystoneMiniBrute — "Polystone MiniBrute"

White-box model of the **Polytone Mini Brute (CS-100)** for Rocksmith's
`Amp_CS100`. Parody brand **"Polystone"** (Polytone → Polystone). The face must
never read "Polytone".

Modelled component-by-component from
`amps/Polytone Mini Brute (CS-100)/Polytone_mini_brute.pdf` (Polytone Mini
Brutes I–IV / Mega Brute preamp + PA378B power amp) and the panel photos.

## Panel (1:1, per the photo)
`BASS · TREBLE · VOLUME` + a **BRITE** switch + **Hi/Lo** inputs. **NO middle
control.**

A **simple solid-state** jazz combo — the warm clean "jazz box" tone (e.g. Joe
Pass): a 4558 op-amp preamp into a Baxandall-ish BASS/TREBLE tone amp, a VOLUME,
and a BRITE high-shelf boost switch. Solid-state → stays clean (only a gentle
op-amp soft-limit near full VOLUME), dark-ish voicing, a small 1×12 combo cab.
The full schematic's DIST diode-clipper + spring reverb are NOT part of this
clean voice and are omitted; the PA378B power amp is a clean transistor push-pull.

## Rocksmith mapping
**RS Gain → VOLUME** (the only level/drive; mostly clean), **RS Bass → Bass**,
**RS Treble → Treble** (no Mid on this amp).

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/pmb1_build TARGET_DIR=/tmp/pmb1_out vst3
```
Copy into `vst/amps/PolystoneMiniBrute.vst3` +
`codesign --force --deep --sign - PolystoneMiniBrute.vst3`. UNIQUE_ID `Pmb1`.
