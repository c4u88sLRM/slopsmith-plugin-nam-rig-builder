# MarstenJCM800 — "Marsten JCM800"

White-box model of the **Marshall JCM800 2204** (50W master-volume head) for
Rocksmith's `Amp_MarshallJCM800`. Parody brand **"Marsten"** (same family as the
Marsten Plexi / DSL100). The face must never read "Marshall".

Modelled component-by-component from the official Marshall **2204 STD** schematic
`amps/Marshall JCM800/2204prem.gif` + `2204pwrm.gif` (19-5-88).

## Panel
`PRESENCE · BASS · MIDDLE · TREBLE · MASTER VOL · PREAMP VOL` + High/Low inputs.
Cascaded 12AX7 preamp (Preamp Vol = the drive) → Marshall TMB tone stack (Treble
220k, Bass 1M, Middle 22k, slope 33k, 470pF/22n/22n) → master → 2× EL34 (~50W) +
Presence NFB. (The Mr. Y EMS is this same circuit + a HI/LO switch.)

## Rocksmith mapping
**RS Gain → PREAMP VOL**, Bass/Mid/Treble → tone stack, Pres → Presence. `_static`
pins the master Volume. UNIQUE_ID `Mj80`.
