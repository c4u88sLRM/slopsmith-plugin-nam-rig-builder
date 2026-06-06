# Mr. Y MAZ 18 (MrYMaz18.vst3)

A bundled RigBuilder amp VST3 modelling the **Dr. Z Maz 18 Jr Reverb**
(parody brand "Mr. Y" — the face must never read "Dr. Z" or "Maz").
Rocksmith gear: **Amp_GB38**.

## Voice

A 2xEL84 (~18W) class-AB head with a **GZ34 tube rectifier** for a noticeable,
spongy **sag** and a touch-sensitive feel. Voiced Vox-meets-Fender: chimey,
midrange-forward, and it breaks up musically and earlier than a big amp because
of the small EL84 power section.

## Signal path (hand-traced schematic)

- 12AX7 input stage (68k/1M, 680R cathode), 1n couplings -> 2nd 12AX7
  (100k plate, 820R + 25uF cathode).
- **TMB tone stack** (3rd-order bilinear `MarkToneStack`): TREBLE 250k with a
  250pF treble cap, MIDDLE 10k, BASS 1M, slope R 100k, 47n coupling/bass caps.
- VOLUME (1MA) preamp gain -> 12AX7 phase inverter.
- **CUT** (250kA): post-PI treble cut to ground (higher = darker).
- MASTER (1MA) output level.
- Spring **REVERB** (12AT7 driver + tank + 12AX7 recovery, 250k level), off
  when Reverb = 0.
- Power: 2xEL84 (2.7k screens, 360v), CinTran OT, GZ34 rectifier sag (modelled
  a touch spongier than usual).

## Parameters (7, in order)

`Volume`, `Treble`, `Middle`, `Bass`, `Cut`, `Master`, `Reverb`
(symbols: `volume`, `treble`, `middle`, `bass`, `cut`, `master`, `reverb`).

Rocksmith maps **Gain -> Volume**; Bass/Mid/Treble -> the tone stack.

## Build

```
make DPF_PATH=/path/to/dpf vst3
```

Output normalized to ~-14 dBFS via the folded `cleanMakeup` / `level` stage in
`Maz18Plugin.cpp` (see the loudness harness); the peak collapses as Volume rises.
