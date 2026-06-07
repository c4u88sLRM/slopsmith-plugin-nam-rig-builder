# UnparallelDC30 — "Unparallel DC30"

White-box model of the **Matchless DC30** (hand-wired AC30-class boutique combo)
for Rocksmith's `Amp_BT30`. Parody brand: the face must never read "Matchless".

Hand-traced from the 4-page DC30 schematic. TWO independent channels into a
shared **4×EL84 CLASS-A** power amp (~30W) with **NO global negative feedback**
→ very chimey, jangly, blooms & compresses at high volume.

- **CHANNEL 1 "Brilliant"** — two 12AX7 stages (input 68k/1M; stage1 220k plate,
  25µF cathode bypass; 560pF+180pF coupling → 500kA Volume; stage2 100k plate,
  1k5 + 25µF) → a **VOX TOP-BOOST** tone stack with TREBLE (220k, 56pF treble
  cap) and BASS (1M, .022) **only** — no mid. Bright, glassy.
- **CHANNEL 2 "EF86"** — one EF86 pentode (higher gain, fatter/darker; 330k+2M2
  plate, 2k2 + 25µF) → a 6-position TONE rotary (caps 360p/56p/.0012/.0022/.0047/
  .01 with 1M5) modelled as a **continuous Tone** sweeping dark(fat)→bright →
  180pF → 1MA Volume. Thick midrange, more gain.
- **Shared** — CUT (250kA, post/PI treble cut → higher = darker), MASTER (1MA).

## Panel
`Ch1 Volume · Bass · Treble · Ch2 Volume · Tone · Cut · Master · Channel`
(Channel: 0 = Ch1 Brilliant, 1 = Ch2 EF86). The DSP channel-select morphs
between the Ch1 top-boost voice and the Ch2 EF86 voice.

## Rocksmith mapping
**RS Gain → Ch1 Volume** (drives the EL84 breakup), Channel pinned to Ch1
Brilliant via `_static`; Bass/Treble → the Ch1 top-boost stack. All editable on
the face.

## Build
```
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/udc30_build TARGET_DIR=/tmp/udc30_out vst3
```
Copy `UnparallelDC30.vst3` into `vst/amps/` then
`codesign --force --deep --sign - UnparallelDC30.vst3`. UNIQUE_ID `Ud30`.
