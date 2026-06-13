# Germanium Drive — Hudson Electronics Broadcast (Aion Skywave)

| | |
|---|---|
| **RS gear** | `Pedal_GermaniumDrive` ("Germanium Drive", "classic smooth overdrive") |
| **Codename / dir** | `germanium_drive` · `vst/src/pedals/germanium_drive/` |
| **Parodia** | Germanium Drive (sin marca real en la cara) |
| **Esquemático** | `pedals/germanium drive.pdf` — Aion **SKYWAVE** = clon del **Hudson Electronics Broadcast** (germanium fuzz/overdrive, basado en consolas broadcast 60s). 11 págs. |
| **DSP** | `GermaniumDrivePlugin.cpp` (+ `GermaniumDriveParams.h`) |
| **Binario/bundle** | `vst/pedals/Germanium Drive.vst3` y `GermaniumDrive.vst3` (NAME `GermaniumDrive`, UID `Gdrv`) |

## Topología (del esquemático, pág. 7)
Boost/overdrive **híbrido silicio+germanio, class-A, con salida por transformador** que satura en
drives altos. NO es un fuzz áspero — es boost→OD ligero **cálido**.

- **Entrada**: IN → RPD 1M (pulldown) → **C2 1µF** (acople) → base Q1. (C1 10µF opcional en paralelo;
  C3 330pF base→GND, rolloff RF/treble).
- **Q1 = 2N5088** (silicio NPN), 1ª etapa class-A common-emitter:
  - R1 220K (bias desde VC), R3 5K6 (colector), emisor R2 5K6.
  - **TRIM 1K** interno = gain del silicio (set-to-taste), + R4 68R, **GAIN SW** (3 ratios: low/med/high) con R5 470R.
- **GAIN 250kA** + C7 1n + R8 5K6 = **realimentación de Q2 (germanio) de vuelta al emisor de Q1**.
  → "Gain sets the amount of feedback from the germanium stage fed into the silicon stage" (= sube drive).
- **Q2 = 2N404A** (germanio PNP), 2ª etapa de ganancia:
  - R6 5K6 (carga), R7 4K7 (a VA), C8 330n.
- **LO CUT 10kA** + **C9 330µF** = corte de graves variable (mushy↔tight).
- **Salida**: C10 100µF → R9 15K → **XFM = Triad TY-141P (transformador 10K:10K)** que **satura en niveles altos** (armónicos pares) → R110 33K → **LEVEL 100kA** → OUT.
- Alimentación: tripler de voltaje (9/18/24V seleccionable), no relevante para el modelo de tono.

## Controles (panel real) → RS knobs
Real: **Gain**, **Low Cut**, **Level**, + toggle **Gain Mode** (3 ratios) + slide **Voltage** (9/18/24V).
RS expone solo **Gain** y **Tone**:
- `RS Gain -> drive de realimentación` (silicio→germanio→transformador).
- `RS Tone -> brillo post` (el pedal real no tiene control de agudos; es interpretación musical).
- Low Cut / Level / Gain Mode / Voltage **fijos** internamente.

## Notas del modelo DSP (reescrito 2026-06)
Cadena fiel al circuito, **2 saturaciones reales** (germanio + transformador), suaves:
1. **HP** de entrada (acople C2 + low cut fijo ~85 Hz).
2. **Q1 silicio**: `tanh(x * (1.7+7.8*g)*0.45) * 1.55` — casi limpio (es preamp; el GAIN = realimentación sube drive).
3. **Q2 germanio**: `tanh(q1*(1+1.7*g) + 0.17) - tanh(0.17)` — **soft sat asimétrica** (bias 0.17 → armónicos pares, el calor del Broadcast).
4. **Transformador TY-141P**: `tanh(q2*(0.85+0.5*g))` — redondeo even-harmonic que entra con el nivel.
5. **Tone**: tilt de brillo (blend dark/bright).
6. **Makeup ESTÁTICO** (solo función de Gain): `0.13 + 1.10*exp(-g/0.22)`.

### Problema que arreglamos (importante)
- La versión vieja usaba un **AGC (RMS auto-makeup, `RBAutoMakeup`)** que sobre el clipper
  **hacía swell de volumen por nota** → sonaba como "reverb raro / ataque extraño". **Eliminado**.
- También tenía **sobre-cascada** (3 tanh con drive hasta 44×) → áspero/fuzzy. Simplificado.
- Resultado: nivel +0.3..+2.8 dB en todo el barrido de Gain (consistente), THD suave 2.4%→29%.
