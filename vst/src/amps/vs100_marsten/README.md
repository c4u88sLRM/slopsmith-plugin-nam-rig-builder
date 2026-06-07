# MarstenVS100 — "Marsten VS100"

White-box model of the **Marshall Valvestate VS100RH** (hybrid head) for
Rocksmith's `Amp_HG180`. Parody brand **"Marsten"** (same as the DSL100 / Plexi).
The face must never read "Marshall".

Modelled component-by-component from `amps/Marhsall VS100RH (HG-180)/v100-6x-02.pdf`.

## Panel
CLEAN (Volume, Bass, Middle, Treble) · OD1 (Gain, Volume) · OD2 (Gain, Contour,
Volume, Bass, Middle, Treble) · FX Mix · Clean Reverb · OD Reverb + Power/Standby.

Hybrid: solid-state op-amp preamp + a 12AX7 "Valvestate" warmth stage into a
solid-state power amp. CLEAN / OD1 (crunch) / OD2 (the Valvestate diode-clip lead)
modes. RS Gain -> OD2 Gain; Bass/Mid/Treble -> OD2 EQ (Channel pinned OD2,
reverb/FX off via _static). UNIQUE_ID `Vs10`.
