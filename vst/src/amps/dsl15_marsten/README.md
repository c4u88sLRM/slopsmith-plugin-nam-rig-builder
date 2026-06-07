# DSL15 — "Marsten DSL15"

Bundled RigBuilder amp for Rocksmith's `Amp_MarshallDSL15H`. Models the
**Marshall DSL15H** — the little-brother 15 W DSL head, two-channel front panel.
Parody brand **"Marsten"** (the same brand used for the GM-2 / UV-1 Marshall-copy
pedals and the larger Marsten DSL100); the in-app face must never read "Marshall".

Local reference (modelled component-by-component):

- `amps/Marshall DSL15/Marshall_DSL15_60_02_v04.pdf` — "15W MAIN BOARD" (iss4):
  4× ECC83 preamp (V1A/B V2A/B V3A/B), Classic/Ultra channel relays (RL1), shared
  Marshall TMB tone stack (VR5 B200K Treble, VR6 B20K Middle, VR7 Bass),
  per-channel Gain/Volume, 2× 6V6 power amp (V5A/V6A) into OTX TXOP-91001 (~15 W),
  DEEP switch (SW1B + R94/C68/C69 low-end resonance), Presence (VR8 C10K NFB),
  Output Power switch (full / ~7.5 W half).

## Panel (11 controls)

- **Channel** — Classic / Ultra select
- **CLASSIC GAIN** — Gain + Volume (clean → crunch)
- **ULTRA GAIN** — Gain + Volume (high-gain OD)
- **EQUALISATION** — Bass, Middle, Treble
- **Deep** — low-frequency power-amp resonance switch (off / on)
- **Tone Shift** — mid-scoop switch (off / on)
- **Presence** — high-frequency power-amp NFB

The DSL15 head has **no Resonance pot, no reverb and a single Master** (vs. the
DSL100): the panel is deliberately simpler.

## Rocksmith mapping

RS **Gain** → **Ultra Gain** with the Channel pinned to **Ultra** (the DSL15 lead
voice). Bass/Mid/Treble → tone stack, Pres → Presence. The per-channel Gain/Vol
and the Deep / Tone Shift switches sit at musical defaults (`_static` in
`rs_knob_to_vst_param.json`) and stay editable by hand.
