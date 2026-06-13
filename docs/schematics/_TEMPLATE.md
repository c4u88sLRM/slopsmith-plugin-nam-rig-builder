# <Parodia> — <Unidad real>

| | |
|---|---|
| **RS gear** | `Amp_XXX` / `Pedal_XXX` |
| **Codename / dir** | `xxx` · `vst/src/{amps,pedals,racks}/xxx/` |
| **Parodia** | Nombre de cara (marca real prohibida en la cara) |
| **Esquemático** | `amps/<Real (CODENAME)>/archivo.pdf` (o `pedals/x.pdf`) |
| **DSP** | `vst/src/.../XxxPlugin.cpp` (+ `XxxParams.h`, `XxxCore.h`) |
| **Binario/bundle** | `vst/{amps,pedals}/<Bundle>.vst3` (NAME interno) |

## Topología (del esquemático)
- Entrada / acoplamiento:
- Etapas de preamp (válvulas/op-amps + valores R/C clave):
- Tone stack (tipo + R/C):
- Lazos / feedback / presence:
- Power amp (válvulas, clase, ~W, bias, sag):
- Reverb / tremolo / extras:

## Controles (panel real) → RS knobs
- Lista de pots/switches reales y a qué param/RS knob mapean.
- `RS Gain -> <param>` (scale, curva, pins `_static`).

## Notas del modelo DSP
- Estructura (funciones clave: asymTube / ToneStack / Biquad / etc.).
- Loudness: cleanMakeup / level / `rbAmpLvl`.
- Ajustes/tuning aplicados (fecha + motivo). Ej: input pre-gain 3.2× (déficit de entrada VST), boosts de cascada, etc.
- Problemas conocidos / cosas a revisar.
