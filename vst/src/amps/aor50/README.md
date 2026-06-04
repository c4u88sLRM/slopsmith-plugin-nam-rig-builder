# AOR50 — "Raney AOR50"

Bundled RigBuilder amp for Rocksmith's `Amp_GB100`. Models the **Laney AOR 50
"Pro Tube Lead"** (A50 Series II) — the full two-channel front panel, 1:1.
Parody brand **"Raney"**; the in-app face must never read "Laney".

Local reference (modelled component-by-component):

- `amps/Laney AOR 50 (GB100)/Laney_aor50_series2.pdf`

## Panel (13 controls)

An ECC83 preamp into an EL34 power amp (silicon rectifier), two footswitchable
channels:

- **CHANNEL ONE** — Preamp Volume + Master Volume (Pull-Bright) — British clean/rhythm
- **AOR CHANNEL** — Preamp Volume (Pull-AOR-On) + Master Volume (Pull-Bright) — cascaded lead
- **Channel** select (the "Pull AOR On")
- **Bass** (Pull-Deep), **Middle** (Pull-Boost), **Treble** — shared tone stack
- **Presence** — power-amp NFB

## Rocksmith mapping

`Amp_GB100` exposes Gain/Bass/Mid/Treble/Pres. The single RS **Gain** knob
*drives the channel morph* — low Gain = Channel One (clean), high Gain morphs
into the AOR lead — matching the `gain_variants` clean/crunch/dist split.
Bass/Mid/Treble → tone stack, Pres → Presence. The per-channel preamp/master
volumes and the pull switches sit at musical defaults (`_static` in
`rs_knob_to_vst_param.json`) and stay editable by hand.
