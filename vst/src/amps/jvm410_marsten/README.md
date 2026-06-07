# JVM410 — "Marsten JVM410"

Bundled RigBuilder amp for Rocksmith's `Amp_MarshallJVM410H`. Models the
**Marshall JVM410H** — the 4-channel 100 W EL34 head. Parody brand **"Marsten"**
(Marshall → Marsten; the same brand used for the Marsten DSL100 / JCM800 /
Bluesbreaker copies). The in-app face must never read "Marshall".

Local reference (modelled component-by-component):

- `amps/Marshall JVM410/Marshall_jvm410_sch.pdf`
  - SHT1 **PRE AMP** (JVM410-60-02) — input, FX loop, op-amp reverb, NFB
  - SHT2 **POWER AMP / DI OUT** — 4× EL34 (V1–V4), phase inverter, DI
  - **FRONT PANEL 1** (JVM410-61-02) — the four channel strips, each with its own
    TREBLE/BASS/MID pots + VOLUME + GAIN:
    - CLEAN: VR219/217/218 + VR206 (Clean Volume) + shared clean-gain block
    - CRUNCH: VR204/202/203 + VR216 (Crunch Volume) + VR220 (Crunch Gain)
    - OD1: VR201/202/203 + VR205 (OD1 Gain)
    - OD2: VR214/212/213 + VR211 (DD2 Volume) + VR215 (OD2 Gain)
  - **FRONT PANEL 2** — RESONANCE (VR305), PRESENCE (VR326), MASTER 1/2 and the
    per-channel green/orange/red mode relays (LATCH_1/LATCH_2 logic).

## Simplification

The real JVM410H is a 4-channel amp where each channel can be stored with its own
green/orange/red voicing mode at once (footswitchable). **Rocksmith plays one
tone at a time**, so this plugin models the **selected channel + mode** as a
single sound:

- **Channel** (`kChannel`) selects CLEAN (0–0.25) / CRUNCH (0.25–0.5) /
  OD1 (0.5–0.75) / OD2 (0.75–1) via overlapping crossfades — increasing cascade
  gain across the range.
- **Mode** (`kMode`) green (0) / orange (0.5) / red (1) adds preamp gain &
  saturation within the selected channel.

All four channels share one Marshall TMB tone stack (the real strips share the
same R/C topology) reused verbatim from the Marsten JCM800. Power amp: 4× EL34
with sag, Presence (HF NFB) + Resonance (LF NFB), plus the op-amp digital reverb
(off at `Reverb = 0`).

## Panel (11 controls)

`Channel`, `Mode`, `Gain`, `Volume`, `Bass`, `Middle`, `Treble`, `Presence`,
`Resonance`, `Master`, `Reverb`.

## Rocksmith mapping

RS **Gain** → `Gain` (with `Channel` pinned to OD1 ≈ 0.66 and `Mode` orange ≈ 0.5
via the song mapping `_static` entries). RS **Bass/Mid/Treble** → tone stack;
RS **Pres** → `Presence`. The remaining controls sit at musical defaults and stay
editable by hand.

Defaults: Channel 0.66 (OD1), Mode 0.5 (orange), Gain 0.60, Volume 0.50,
Bass/Mid 0.50, Treble 0.60, Presence/Resonance 0.50, Master 0.60, Reverb 0.0.
