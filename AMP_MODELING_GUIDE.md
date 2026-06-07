# Guía para crear amps VST (modelo componente-a-componente + volumen estandarizado)

Cómo construir un amp bundled nuevo (`rig_builder/vst/src/<amp>/`) como **modelo
white-box** a partir de su esquemático, y cómo dejar **todos los amps al mismo
nivel** para que al cambiar entre ellos no salte el volumen.

Amps de referencia ya hechos con este método (mirar como ejemplo):
`en30/EN30Core.h` (Vox AC30 → **Box DC30**), `tw22/TW22Core.h` (Fender
Super-Sonic 22 → **Bender SuperNova 22**), `tw26/TW26Core.h` (Fender '57 Deluxe
5E3 → **Bender Deluxe**).

> Complemento de `PEDAL_DSP_GUIDE.md` (referencia de TODO lo bundled). Éste es el
> playbook específico para amps.

---

## 0. Filosofía: white-box, no SPICE

No resolvemos el netlist. Modelo de **comportamiento guiado por componentes**:
cada bloque audible se modela con un filtro/curva cuyos parámetros salen de los
**valores reales** del esquemático (caps en F, resistencias en Ω, tipo de
válvula, rectificador). Suena fiel, corre en tiempo real, y se entiende leyendo
el esquemático.

Cada amp = **un `XxxCore.h`** (C++ plano, sin DPF → testeable offline) +
**`XxxPlugin.cpp`** (wrapper DPF delgado, copiar de un amp existente).

---

## 1. Vocabulario: componente → bloque DSP

| En el esquemático | Bloque DSP | Notas |
|---|---|---|
| Cap de acople entre etapas | `RcHighPass` | corta sub-graves (1M/0.1µF ≈ 1.6 Hz) |
| Grid-stopper + Miller / cap de placa | `RcLowPass` | techo de agudos ultrasónico |
| Triodo (12AX7/12AY7/12AT7) | `triodeXX(x,bias)` **suave** (§3) | μ alto = más gain; bias = asimetría |
| Tone stack (Bass/Mid/Treble) | varios `Biquad` (shelf/peak) | corner AUDIBLE (§4) |
| Cut / presence | `RcHighPass` restado, o high-shelf | `y = x - amount*HP(x)` |
| Par de potencia (EL84/6V6/6L6) | `powerPair(x,bias)` push-pull (§3) | tanh ± con bias |
| Rectificador (GZ34/5Y3/diodos) | envolvente de **sag** (§5) | válvula = sag fuerte; SS = suave |
| Transformer + parlante | `Biquad` HP + body + presence + LP | define "caja" y roll-off |
| NFB | restar fracción filtrada de la salida | muchos Fender/Vox NO tienen |

---

## 2. Anatomía de un `XxxCore.h`

```cpp
namespace xxx {
  // helpers: clamp01, clampFreq, smoothstep, smoothstepRange, softClip, eqDb
  // bloques: RcHighPass, RcLowPass, Biquad, DcBlock  (copiar de EN30Core.h)
  // curvas de válvula suaves (§3) + supply/sag (§5)
  class XxxCore {
    float sampleRate, gain, bass, mid, treble /*, pres*/;   // estado (knobs 0..1)
    void updateFilters() {            // se llama al cambiar un knob
      const float g    = smoothstep(gain);
      const float burn = smoothstepRange(LO, HI, gain);     // morph clean->dirty
      const float hot  = smoothstepRange(0.6f, 1.0f, gain);
      /* setear coeficientes desde valores de componentes */
    }
  public:
    void setSampleRate(float sr){ sampleRate = sr>1000?sr:48000; reset(); }
    void setGain(float v){ gain=clamp01(v); updateFilters(); }   // idem cada knob
    float process(float in){
      /* in -> preamp tubes -> tone stack -> power+sag -> dcBlock -> speaker */
      return softClip(y * makeup / toneEnergy) * 0.98f;   // §6 + límite final
    }
  };
}
```
El wrapper `XxxPlugin.cpp` es idéntico salvo nombres/UID/setters. `XxxParams.h`
= enum de knobs RS + nombres/símbolos + `kXxxDef[]` (defaults).

---

## 3. REGLA #1 — curvas de válvula SUAVES (C∞)

El error más caro: una curva partida en dos ramas (positiva/negativa) tiene
**discontinuidad de derivada en el cruce por cero** → mete distorsión áspera
**hasta en señal limpia** (medimos 38% THD en un "limpio"). Una válvula real es
suave. Usar siempre `tanh(warp) - tanh(idle)`:

```cpp
static inline float triode12AX7(float x, float bias) {        // μ alto
    const float g = x + bias;
    const float warped = 1.55f*g + 0.34f*g*std::fabs(g);      // asimetría suave
    const float idle   = 1.55f*bias + 0.34f*bias*std::fabs(bias);
    return std::tanh(warped) - std::tanh(idle);               // -idle quita el DC
}
// 12AY7 ~k=1.28 (más warm), 12AT7 ~k=0.98; sixV6Pair = 0.5*(tanh(+)-tanh(-))
```
- `k` ≈ μ del tubo. bias chico (±0.03..0.09) = armónicos pares ("válvula").
- **NUNCA** `return shifted>=0 ? tanh(a*g) : tanh(b*g)` (eso es el kink).
- Verificar con el test de **THD en limpio** (§7): a gain bajo debe dar < ~3-5%.

---

## 4. REGLA #2 — tone stack con corners AUDIBLES

Trampa clásica: calcular el corner RC con el valor **crudo del pot** (1M) da un
corner sub-audible (~6–24 Hz) → el control "no hace nada". El cap ve la
**resistencia efectiva (Thévenin)** del nodo (mucho menor). Calcular por la
**frecuencia objetivo**:
```cpp
// MAL: c5.setRC(sr, 1e6f*(1-bass), 22e-9f);   // corner ~7 Hz, banda inerte
const float bassFc = 120.0f + 300.0f * bass;   // corner audible ~120–420 Hz
c5.setRC(sr, 1.0f/(2*kPi*bassFc*22e-9f), 22e-9f);  // mantené el cap real (22n)
```
**Voicing por familia:** Blackface/Super-Sonic = scoop de medios ~450 Hz,
brillante/tight. Tweed (5E3) = **mid-forward**, NADA scooped, woody. Vox Top
Boost = Bass/Treble interactúan.

**REGLA #2b — rango usable en TODA la perilla.** Cada control debe cambiar de
mínimo a máximo (de 7pm a 5pm), no clavarse a mitad de camino. Verificar con el
sweep (§7): medir la banda relevante a 0.5 / 0.58 / 0.75 / 1.0 — debe seguir
moviéndose hasta 1.0. Si se satura: ensanchar el rango de dB del shelf, abrir el
LP del parlante con el knob, y repartir el morph de Gain a lo ancho del recorrido.

---

## 5. REGLA #3 — el sag es DINÁMICA, no nivel

```cpp
sag += (env - sag) * (env>sag ? attack : release);
const float supplyScale = 1.0f / (1.0f + sag * (k0 + k1*gain + k2*hot));
// usar supplyScale para escalar drive y salida del power amp
```
- Válvula (GZ34/5Y3): sag **profundo y lento** (release ~0.18–0.22 s; firma del 5E3).
- Estado sólido (diodos): sag **suave** (release ~0.09 s).
- **No matar el sag para arreglar nivel.** El nivel se compensa aparte (§6).

---

## 6. Estandarización de volumen — todos al mismo nivel

Que al cambiar de amp (o subir el Gain) el volumen percibido no salte. Se hace
con un **makeup de salida** (NO es un componente — va antes del `softClip` final).

### 6.1 Forma del makeup según la trayectoria de nivel del amp
- **Sag fuerte (cae al subir gain)** → makeup SUBE: `1.5f + 0.4f*g + 0.7f*hot` (Box DC30, Deluxe).
- **Breakup que se hace más fuerte** → makeup BAJA: `0.6f + 2.0f*exp(-4.5f*gain)` (estilo SuperNova).
- **Salto-S por morph de canal (clean→burn)** → un `exp` no alcanza; usar un
  **sigmoide** centrado en el punto del morph:
  `makeup = 0.50f + 1.6f / (1.0f + std::exp(8.0f*(gain - 0.42f)))`.

### 6.2 REGLA #4 — medir con MULTITONO, no con un seno
Un seno de 220 Hz **engaña** (mucha energía en una freq pero suena más bajo en
banda ancha). Medir RMS con un **multitono** (110 Hz–1.8 kHz). Pasó literal: el
SuperNova medía "más fuerte" con seno y se oía **más bajo** → el multitono lo
confirmó (~3–4 dB abajo).

### 6.3 "Más bajo" suele ser falta de CUERPO
Si el RMS calza pero se siente chico/bajo → comparar el **espectro bin-por-bin**
vs la referencia. El SuperNova estaba ~3 dB hundido en **196–294 Hz** (zona de
cuerpo) → se sentía delgado. Fix: campana de cuerpo en ~235 Hz. (Pero ojo: no
engordarlo más allá de lo que el amp real es — un Fender es más tight que un AC30.)

### 6.4 Referencia y proceso
Target ~**0.40 RMS** (multitono), nivel del **Box DC30** (referencia). Meta: los
promedios de todos los amps dentro de **~1 dB**.
1. Medir todos los amps con el mismo multitono a varios Gains (script §7.2).
2. Ver el que se desvía; `nuevo_makeup ≈ makeup × (target / RMS_medido)` por punto.
3. Ajustar las constantes del makeup. Re-medir. Repetir hasta ~1 dB.
4. Confirmar `peak ≤ ~1.0` (el `softClip*0.98` lo garantiza).

---

## 7. Tests offline (antes de compilar el VST)

### 7.1 Por amp: estabilidad + THD-limpio + sweep de controles + loudness
```bash
cd rig_builder/vst/src/<amp>
c++ -std=c++14 -O2 -I. /tmp/t.cpp -o /tmp/t && /tmp/t
```
Chequear: sin NaN/Inf en 44.1/48/96k; **peak ≤ ~1.0**; silencio decae a 0;
**THD limpio bajo**; cada control mueve su banda **hasta 1.0** (sweep
0.5/0.58/0.75/1.0); loudness ~plano. (Harness completo: ver lo usado para
EN30/TW22/TW26 en el historial.)

### 7.2 Comparación de loudness ENTRE amps (multitono, macros)
Los `Params.h` definen `kGain` global → NO incluir dos a la vez en un TU;
compilar un `.cpp` parametrizado una vez por amp:
```bash
c++ -O2 -Ivst/src/en30 -DHDR='"EN30Core.h"' -DNS=en30 -DCLS=EN30Core -DNAME='"BOX"'    cmp.cpp -o /tmp/a && /tmp/a
c++ -O2 -Ivst/src/tw22 -DHDR='"TW22Core.h"' -DNS=tw22 -DCLS=TW22Core -DNAME='"SNOVA"'  cmp.cpp -o /tmp/b && /tmp/b
c++ -O2 -Ivst/src/tw26 -DHDR='"TW26Core.h"' -DNS=tw26 -DCLS=TW26Core -DNAME='"DELUXE"' cmp.cpp -o /tmp/c && /tmp/c
```
`cmp.cpp` arranca con `#include HDR` y usa `NS::CLS`; mide RMS de un multitono a
g=.2/.4/.6/.8. Los RMS deben quedar dentro de ~1 dB entre amps.

---

## 8. Build local + instalar + firmar (probar YA)

```bash
cd rig_builder/vst/src/<amp>
rm -rf /tmp/b /tmp/o && mkdir -p /tmp/b /tmp/o
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/b TARGET_DIR=/tmp/o   # overrides: el DPF default es /build /bin (read-only)
cp /tmp/o/<NAME>.vst3/Contents/MacOS/<NAME> "rig_builder/vst/<BUNDLE>.vst3/Contents/MacOS/<NAME>"
codesign --force --deep --sign - "rig_builder/vst/<BUNDLE>.vst3"   # ad-hoc; en Apple Silicon si no, no carga
codesign --verify "rig_builder/vst/<BUNDLE>.vst3"
```
Después: **Cmd+Q completo a Slopsmith y reabrir** (no hay hot-reload del binario;
la cara canvas sí es en vivo). El binario local es solo para probar; el build
oficial (CI) lo reproduce desde la fuente con el `alias_of()` de los workflows.

---

## 9. Rename parodia + cara (cerrar el amp)

- **Rename 3 niveles + categorías** (igual que EN30→Box DC30, TW22→Bender
  SuperNova 22, TW26→Bender Deluxe): `DistrhoPluginInfo.h` (`DISTRHO_PLUGIN_NAME`),
  bundle dir, `vst_display_names.json` (casing lindo, ej. "Bender Deluxe"),
  `rs_gear_to_vst.json` (bundled path), `rs_knob_to_vst_param.json` (stem),
  `pedal_type_tags.json`, `routes.py` (`_RENAMED_VST_BUNDLES` + migración DB
  idempotente), 3 workflows (`alias_of()`). El binario adentro mantiene el NAME
  del Makefile; UID/URI estables. Re-firmar el bundle tras renombrar.
- **Cara canvas** en `pedal_canvas.js`: `P.<stem>` (stem = filename del bundle en
  minúsculas sin caracteres no-alfanuméricos). Solo los knobs de RS. Reusar
  estilos de knob (`'vox'` chicken-head, `'cream'` Fender marfil, etc.).

---

## 10. Checklist por amp

- [ ] Leer el esquemático: válvulas, tone stack, PI, par de potencia, rectificador, NFB?
- [ ] `XxxCore.h`: corners audibles (§4), válvulas suaves (§3), sag según rectificador (§5).
- [ ] `XxxPlugin.cpp` + `XxxParams.h` (copiar de un amp existente, ajustar nombres/UID/knobs).
- [ ] Test offline: estable, peak ≤ 1.0, THD limpio bajo, **cada control activo hasta 1.0**, loudness plano (§7.1).
- [ ] Loudness estandarizada: comparar multitono vs Box, ajustar makeup a ~1 dB (§6, §7.2).
- [ ] ¿Se siente con cuerpo? Comparar espectro bin-por-bin (§6.3).
- [ ] Build local + instalar + firmar; Cmd+Q + reabrir; aprobar de oído.
- [ ] Rename parodia 3 niveles + categorías + cara canvas (§9).
- [ ] (Cuando se pida) commit/push — el CI reproduce los binarios desde la fuente.
