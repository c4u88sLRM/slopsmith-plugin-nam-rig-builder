# Silla Boogie Mark III — Mesa/Boogie Mark III

| | |
|---|---|
| **RS gear** | `Amp_CA85` ("Mesa Boogie Mark III Crunch") |
| **Codename / dir** | `mark_iii` · `vst/src/amps/mark_iii/` |
| **Parodia** | Silla Boogie Mark III (Mesa=mesa→Silla=silla; la cara nunca dice "Mesa"/"Boogie") |
| **Esquemático** | `amps/Mesa Mark III (CA_85)/boogie_mkiii.pdf` — "Tri-Mode Programmable Preamplifier" |
| **DSP** | `MarkIIIPlugin.cpp` + `MarkIIIParams.h` |
| **Binario/bundle** | `vst/amps/MarkIII.vst3` (NAME `MarkIII`, UID `Mk3c`) |

## Topología (del esquemático)
- **Tri-Mode preamp** con 2 voces sobre UN tone stack:
  - **RHYTHM**: Volume → Master (una etapa).
  - **LEAD**: cascada Lead Drive (V3A/V3B) → Lead Master (la "singing Boogie lead").
  - Switch LEAD elige la voz.
- **Tone stack** = Fender-derivado TMB con el **mid pot 10K scooped** (el famoso mid scoop Boogie):
  - R1 250K (treble), R2 1M (bass), R3 10K (middle), R4 100K (slope).
  - C1 250pF, C2 22nF, C3 22nF.
- **GEQ gráfico 5 bandas** (la firma Boogie): 80 / 240 / 750 / 2200 / 6600 Hz, con switch **EQ IN**.
- **Power amp** 6L6/EL34 **Simul-Class ~75W** + presence NFB fijo, con sag.

## Controles (panel real) → RS knobs
Panel 1:1 (izq→der): INPUT, FOOTSWITCH, 7 pots: **VOLUME (pull BRIGHT)**, TREBLE, BASS,
MIDDLE, MASTER, **LEAD DRIVE**, LEAD MASTER, + GEQ 5 bandas (+ switch EQ IN), STANDBY/POWER.

- RS expone solo Gain/Bass/Mid/Treble:
  - **`RS Gain -> Lead Drive`** (param "Lead Drive", scale **0.01**). Canal **fijo en LEAD** vía `_static`.
  - `RS Bass/Mid/Treble -> tone stack` (Middle = el 10K scooped).
  - `_static`: GEQ "V" smile (80/6600 up, 750 down), EQ IN on, Bright off, masters/volume en defaults musicales.
- Params (enum `MarkIIIParamId`): kVolume, kTreble, kBass, kMiddle, kMaster, **kLeadDrive(5)**,
  kLeadMaster, kEq80..kEq6600, **kLead(12)**, kBright, kEqIn.
- Defaults: `{0.50,0.60,0.45,0.40,0.45,0.55,0.50, GEQ.., lead=1.0, bright=0, eqIn=1.0}`.

## Notas del modelo DSP
- **Cascada LEAD** (en `process`): `asymTube(t, 1.40 + 5.4*leadDrive + 2.9*g, ...)` → interHp →
  `asymTube(ld, 1.15 + 6.1*leadDrive + 3.3*pushed, ...)` → cathodeLp → `*(0.45+leadMaster)`.
  - `drv = leadDrive` (canal lead), `g = smoothstep(drv)`, `pushed = smoothstepRange(0.40,0.92,drv)`.
- **Loudness**: `cleanMakeup = 1 + 7*exp(-drv/0.22)` (sube limpios, plano en saturación → no dispara
  volumen a max gain). `level = 0.62*cleanMakeup/((1+0.45*mPush)*toneEnergy)`. Salida ~−8 dBFS.
- Tone stack scooped → atenúa señal pre-cascada (compensado con `*2.0` + stackMakeupLow).

### Tuning aplicado
- **2026-06** cascada lead ~**1.27×** más caliente (era muy limpia a Lead Drive alto).
- **2026-06** `input pre-gain 3.2×` en `inputHp.process(in*3.2f)` — el déficit de entrada VST
  (el `setGain('input')` del engine NO llega a etapas VST) dejaba el lead limpio a RS Gain 90.
  Medido a entrada cruda: g30≈10% · g50≈18% · **g90≈32% THD**; loudness estable −8.3 dBFS.
