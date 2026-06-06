# MrYEMS — "Mr. Y EMS"

White-box model of the **Dr. Z EMS** (a Marshall JCM800/JTM50-style master-volume
head) for Rocksmith's `Amp_GB50`. Parody brand **"Mr. Y"** (Dr. Z → Mr. Y; same
family as the Mr. Y MAZ 18). The face must never read "Dr. Z".

Reference: `amps/Dr Z. EMS (GB-50)/schematic.pdf` (the EMS is a JCM800 2204 circuit
+ a HI/LO gain switch) + `front.jpeg`.

## Panel (per the photo)
`PRESENCE · BASS · MIDDLE · TREBLE · VOLUME (master) · GAIN (preamp)` + a HI/LO
gain switch + Power/Standby. Big gold "Z"→"Y" on the grille + "Legacy" badge.

Cascaded 12AX7 preamp (GAIN = the JCM800 drive) → Marshall TMB tone stack → master
volume → 2× EL34 (~50W) with a presence NFB. HI = full JCM800 gain; LO drops to
JTM50 levels (input divider + lifted 2nd-stage cathode bypass).

## Rocksmith mapping
**RS Gain → GAIN**, Bass/Mid/Treble → tone stack, Pres → Presence. `_static` pins
the master Volume + HI gain mode. UNIQUE_ID `Yems`.
