# EngelFireball — "Engel Fireball"

Bundled RigBuilder amp for Rocksmith's `Amp_EN50`. Models the **ENGL Fireball
100 (EN-50)** — a 2-channel high-gain modern metal head (~100 W). Parody brand
**"Engel"** (ENGL → Engel, the German word the company name puns on); the in-app
face must never read "ENGL".

Local reference (modelled block-by-block):

- `amps/ENGL Fireball (EN-50)/engl-fireball-amplifier-schematic_new.pdf`
  ("ENGL Gerätebau GmbH 625")
- `amps/ENGL Fireball (EN-50)/1.jpg`, `2.jpg` — the front panel, for the canvas

## Panel (12 controls)

- **Clean Gain** — clean (low-gain, headroomy) channel input gain
- **Lead Gain** — the Ultra high-gain cascade drive (tight, aggressive,
  scooped-capable, more gain than a JCM800)
- **Bass / Middle / Treble** — shared Marshall-ish passive TMB tone stack
- **Lead Volume** — lead channel volume
- **Master** — global output master
- **Presence** — power-amp high-frequency NFB
- **Channel** — Clean (0) / Lead (1)
- **Bright** — treble-boost voicing switch
- **Bottom** — low-end-boost voicing switch
- **Mid Boost** — mid-push voicing switch (on when ≥ 0.5)

## Rocksmith mapping

RS **Gain** → **Lead Gain** (Channel pinned to Lead via `_static`), Bass/Mid/
Treble → tone stack, Pres → Presence. Clean Gain, Lead Volume, Master and the
voicing switches sit at musical defaults and stay editable by hand.
