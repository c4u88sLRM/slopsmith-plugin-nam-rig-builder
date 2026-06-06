# Unparallel Chieftain

A bundled RigBuilder amp VST3 modelling the **Matchless Chieftain (Reverb)**
(Mark Sampson), for Rocksmith's `Amp_BT15`. Parody brand/name — the face never
reads "Matchless" or "Chieftain".

## Voice

A single-channel **clean/crunch boutique head**: 2x EL34 **cathode-biased**
class AB (~40W), moderate sag, **big headroom**. Lots of clean before breakup —
a Fender-meets-Marshall voice that stays cleaner / has more headroom than the
Mark/Boogie heads. Deliberately not over-saturated.

## Signal flow (from the hand-traced 7-page schematic)

- **V1 12AX7** (68k/1M in, 1k5 cathode) -> Marshall-style **TMB tone stack**:
  BASS 1MRA, MID 250kA, TREBLE 1MA with a **LARGE 5100pF (5.1nF)** treble cap
  (warmer, lower-acting treble than a Marshall's 500pF); slope/coupling
  .0022 / 560p / 100k network. **V2 12AX7** -> VOLUME 250kA.
- **Phase inverter** (12AX7, long-tail): MASTER 500kA + **BRILLIANCE** 500kA, a
  presence/high-shelf on the PI via a .0047 cap (higher Brilliance = more top
  sparkle).
- **Spring REVERB** (12AX7 driver + tank + 12AX7 recovery, 100kA level).
- **Power:** 2x EL34, 270ohm/250uF cathode bias, OT WTI9356, GZ34 rect,
  4/8/16 ohm.

## Parameters (7)

| # | Name | Symbol | Default | Notes |
|---|------|--------|---------|-------|
| 0 | Volume     | `volume`     | 0.55 | preamp drive into the EL34s — **RS Gain** |
| 1 | Bass       | `bass`       | 0.50 | TMB tone stack — RS Bass |
| 2 | Middle     | `middle`     | 0.50 | TMB tone stack — RS Mid |
| 3 | Treble     | `treble`     | 0.55 | TMB tone stack (5.1nF, warm) — RS Treble |
| 4 | Brilliance | `brilliance` | 0.40 | presence high-shelf on the PI |
| 5 | Master     | `master`     | 0.70 | power-amp master |
| 6 | Reverb     | `reverb`     | 0.00 | spring reverb mix (off for songs) |

## Rocksmith mapping

RS exposes only Gain/Bass/Mid/Treble. **RS Gain -> Volume** (drives the preamp
into the power stage); Bass/Mid/Treble -> the tone stack. Brilliance / Master /
Reverb are set by hand on the face.

## DSP

Built on the shared RigBuilder amp DSP kit (same `rbAmpLvl` output stage,
helpers, `Biquad`, `DcBlock`, `SpringReverb`, and a 3rd-order bilinear TMB tone
stack with the Chieftain's R/C values). Loudness-calibrated so a Volume sweep
sits around -14 dBFS with the peak collapsing as gain rises.

## Build

```sh
make DPF_PATH=/path/to/dpf vst3
```

Produces `UnparallelChieftain.vst3`, bundled at
`rig_builder/vst/amps/UnparallelChieftain.vst3`.
