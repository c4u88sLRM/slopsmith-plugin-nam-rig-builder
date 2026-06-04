# DSL100 — "Regis DSL100"

Bundled RigBuilder amp for Rocksmith's `Amp_MarshallDSL100H`. Models the
**Marshall JCM2000 DSL100(H)** — the full two-channel front panel, 1:1.
Parody brand **"Regis"** (the same brand used for the GM-2 / UV-1 Marshall-copy
pedals); the in-app face must never read "Marshall".

Local references (modelled component-by-component):

- `amps/Marshall DSL100/JCM2-60-02 (2003) iss9.pdf` — power amp + PSU (4× EL34)
- `amps/Marshall DSL100/JCM2-61-00 (2001) iss5.pdf` — control board (tone stack,
  relay channel switching, op-amp reverb, Presence/Resonance NFB)
- `amps/Marshall DSL100/JCM2-62/63/64` — preamp / channel boards
- `amps/Marshall DSL100/DSL50-100 manual (2004).pdf`

## Panel (19 controls)

- **CLASSIC GAIN** — Clean / Crunch mode + Gain + Volume
- **ULTRA GAIN** — OD1 / OD2 mode + Gain + Volume
- **Channel** select (Classic / Ultra)
- **EQUALISATION** — Bass, Middle, Treble + Tone Shift (mid-scoop switch)
- **Resonance** (low-frequency power-amp NFB) + **Presence** (high-frequency NFB)
- **Reverb** — per-channel level (Classic / Ultra)
- **Master 1 / Master 2** + Master Select
- **Output** — Low (50 W) / High (100 W)

## Rocksmith mapping

The single RS **Gain** knob *drives the channel morph* — low Gain = Classic
clean, mid = Classic crunch, high = Ultra OD — matching the `gain_variants`
split (clean 0–35 / crunch 35–70 / ultra 70–100). Bass/Mid/Treble → tone stack,
Pres → Presence, Res → Resonance. Per-channel Gain/Vol, modes and the masters
sit at musical defaults (`_static` in `rs_knob_to_vst_param.json`) and stay
editable by hand.
