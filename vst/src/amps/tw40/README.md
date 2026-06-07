# TW40 — "Bender Bassman"

Bundled RigBuilder amp for Rocksmith's `Amp_TW40`. Models the **Fender Bassman
5F6-A tweed** (1959) — the full front panel, 1:1. Parody brand **"Bender"** (same
as the SuperNova 22 / Deluxe); the in-app face must never read "Fender".

Local reference (modelled component-by-component):

- `amps/Fender Bassman Tweed (TW40)/Fender_bassman_5f6a.pdf`

## Panel (7 controls)

The 5F6-A — the amp that became the Marshall JTM45 — has two jumperable channels
off a 12AY7 input, a real passive FMV tone stack, a 12AX7 driver / long-tail PI,
2× 5881 (~45 W) and a GZ34 rectifier into a 4×10:

- **Input** — clickable cable: Bright / Both (jumpered) / Normal
- **Bright Vol** (1M, with the 100 pF bright cap) + **Normal Vol** (1M)
- **Treble** (250K), **Bass** (1M), **Middle** (25K) — the FMV tone stack
- **Presence** (5K, power-amp NFB)

There is no gain knob — the volumes ARE the gain.

## Rocksmith mapping

`Amp_TW40` exposes Gain/Bass/Mid/Treble/Pres. Since the 5F6-A has no gain knob,
**RS Gain → Bright Volume** (drives the amp into breakup); Treble/Bass/Mid → the
FMV tone stack, Pres → Presence. The input is pinned to **Both** (jumpered — the
signature Bassman tone) with Normal Vol as the blend (`_static` in
`rs_knob_to_vst_param.json`); everything stays editable by hand.
