# Guía de DSP de los pedales/amps bundled (cómo funcionan y dónde ajustar el sonido)

Este documento explica **cómo están hechos** los plugins VST3 que genera `rig_builder`
(pedales, racks y amps "bundled") y, sobre todo, **dónde tocar para cambiar la
calidad/carácter del sonido**. No son capturas (NAM) ni IRs: son **modelos de
comportamiento** escritos a mano en C++ que imitan el circuito de referencia.

> TL;DR para ajustar tono: el 90% del carácter de cualquier plugin vive en dos
> métodos de su clase `*Core`: **`updateFilters()`** (las ecualizaciones: frecuencia,
> Q y dB de cada `Biquad`) y **`process()`** (el orden de etapas, el `drive`/`bias`
> de la saturación, el `sag`, y la normalización de nivel final). Cambiás esas
> constantes, recompilás, listo.

---

## 1. Dónde vive cada cosa

```
rig_builder/vst/src/
  _shared/            ← toolkit compartido (ver §3)
    automakeup.hpp        RBAutoMakeup  (makeup-gain de los drives)
    reverb_core.hpp       ReverbCore    (motor Freeverb de los reverbs "Studio")
    reverb_plugin.hpp     wrapper DPF común de esos reverbs
    pedalkit.hpp          kit de UI nativa DPF (NO es DSP)
    fonts_data.hpp        fuentes embebidas para la UI nativa
  mark_shared/        ← MarkAmpCore.h  (core compartido de los Mesa Mark III/IV)
  <un dir por plugin>/    p.ej. super_drive/, vintage_distortion/, en30/, tw22/ …
```

Cada directorio de plugin tiene siempre estos archivos:

| Archivo | Qué es | ¿Tocar para tono? |
|---|---|---|
| `XxxParams.h` | enum de parámetros + nombres/símbolos + **min/max/def** | sí (valores por defecto) |
| `XxxPlugin.cpp` | la **DSP** (clase `XxxCore`) + el wrapper DPF | **sí, acá está casi todo** |
| `XxxCore.h` | algunos (amps) separan la DSP a un header testeable | **sí** |
| `DistrhoPluginInfo.h` | nombre/UID/categoría VST3 | no (es metadata/branding) |
| `Makefile` | `NAME =` (= nombre del binario compilado) | no |
| `Xxx_ui.cpp` | UI nativa DPF (no se usa en Slopsmith; usamos el canvas) | no |
| `README.md` | nota corta de referencia | no |

**Dos layouts de DSP** (ambos equivalentes):
- **Inline** (la mayoría de pedales): la clase `XxxCore` está dentro de `XxxPlugin.cpp`.
- **Core header** (amps `en30`, `tw22`, y los Mesa vía `mark_shared`): la DSP está en
  un `.h` aparte (`EN30Core.h`, `TW22Core.h`, `MarkAmpCore.h`) para poder
  **testearla offline** sin depender de DPF. Patrón recomendado para los amps.

---

## 2. Anatomía de un plugin (el patrón que repiten todos)

Tomemos **SuperDrive** (Boss SD-1) como modelo mental — todos siguen esta forma:

```cpp
class SuperDriveCore {
    float sampleRate, gain, tone;          // estado = sample rate + valores de knobs (0..1)
    Biquad inputHp, feedbackVoice, ...;    // los filtros que dan el voicing

    void updateFilters() {                 // (1) se llama al cambiar CUALQUIER knob
        const float g = smoothstep(gain);  //     curva suave del knob
        inputHp.setHighPass(sr, 95 + 115*gain, 0.68);            // freq depende del knob
        feedbackVoice.setPeaking(sr, 720 + 150*gain, 0.78, 2.4 + 3.2*g);
        ...                                //     ← AQUÍ se define el EQ/voicing
    }

    float process(float in) {              // (2) se llama por cada muestra de audio
        float x = inputHp.process(in);     //     cadena de etapas en orden
        x = feedbackVoice.process(x);
        const float drive = 1.15 + 6.0*gain + 18.5*g;   // cuánto empuja al clipper
        float y = sd1FeedbackClip(x*drive, gain);        // ← saturación (el "alma")
        ...
        const float level = 0.76 / (1 + 0.45*gain + ...);// ← normalización de nivel
        return softClip(y*level) * 0.98;   //     límite de seguridad final
    }
};
```

- **Knobs**: cada parámetro de Rocksmith es un `float` 0..1. El mapeo
  knob-de-RS → parámetro-del-plugin está en `data/rs_knob_to_vst_param.json`
  (con `scale`, normalmente 0.01 porque RS guarda 0..100).
- **`updateFilters()`** recalcula coeficientes solo cuando cambia un knob (barato).
- **`process()`** corre por muestra y es donde está el orden de la cadena de señal.
- El wrapper DPF (`XxxPlugin`) tiene **L y R** (dos instancias del Core, una por
  canal) y reenvía `setParameterValue` → `core.setXxx()`.

### Valores por defecto y rangos
En `XxxParams.h`:
```cpp
static const float kSuperDriveDef[kParamCount] = { 0.45f, 0.50f };  // Gain, Tone
```
Cambiar `Def` cambia **cómo suena el pedal recién agregado** (antes de tocar knobs).
`Min/Max` casi siempre son `0..1`; no los toques salvo que sepas lo que hacés.

---

## 3. El toolkit DSP compartido (los "ladrillos")

Estos bloques aparecen (a veces copiados) en casi todos los plugins. Entenderlos es
entender el 100% de los modelos.

### 3.1 `Biquad` — todo el EQ
Filtro de 2º orden estándar (RBJ cookbook). Cada plugin instancia varios y los
configura en `updateFilters()`. Métodos:

| Método | Para qué sirve en el tono |
|---|---|
| `setHighPass(sr, hz, q)` | recorta graves / "aprieta" (tightness). Subir `hz` = más apretado/menos panza |
| `setLowPass(sr, hz, q)` | recorta agudos / quita fizz/brillo. Bajar `hz` = más oscuro |
| `setPeaking(sr, hz, q, dB)` | campana: +dB realza, −dB hace un "scoop". `q` alto = más angosto |
| `setLowShelf / setHighShelf(sr, hz, slope, dB)` | sube/baja todo un extremo del espectro |

**Regla de oro para ajustar EQ:** buscá en `updateFilters()` el `Biquad` que toca la
zona que querés mover y cambiá su `hz` (centro), `q` (ancho) o `dB` (cantidad).
Ejemplos típicos de qué controla qué:
- "suena con mucha panza/barro" → subí el `hz` de `inputHp`/`*Tighten`, o bajá el `dB` de un `*Body`/`bassShelf`.
- "le falta presencia/mordida" → subí el `dB` (o `hz`) del `*Bite`/`*Presence`/`speakerBite`.
- "demasiado fizz/agudo áspero" → bajá el `hz` del `speakerLp`/`toneLowPass`, o profundizá el `*FizzNotch`.
- "medios nasales / honky" → bajá el `dB` del `mid*`/`*Hump` a esa frecuencia.

### 3.2 Saturación (el carácter de la distorsión)
Funciones de no-linealidad. La que use cada plugin define si suena a diodo, a válvula,
a op-amp, a fuzz:

| Función | Carácter | Dónde se usa |
|---|---|---|
| `softClip(x)` = `tanh(x)` | clip suave simétrico | redondeo general, límite final |
| `asymTube(x, drive, bias)` | válvula: clip suave **asimétrico** (armónicos pares) | amps, overdrives valvulares |
| `sd1FeedbackClip(x, gain)` | diodo asimétrico SD-1/OD-1 (1 diodo arriba, 2 abajo) | super_drive y familia |
| clip duro / a umbral | RAT, fuzz, metal: clip abrupto = más armónicos impares/áspero | bass_distortion (RAT), fuzz |

Las dos perillas que gobiernan la cantidad y el color del clip:
- **`drive`** (cuánto se empuja la señal al clipper): más `drive` = más distorsión.
  Suele ser algo como `drive = base + a*gain + b*smoothstep(gain)`.
- **`bias`** (asimetría): pequeño offset que añade armónicos **pares** (suena más
  "válvula"/dulce). `bias` 0 = simétrico (más áspero/impares).

### 3.3 `smoothstep(v)` y crossfades de canal
`smoothstep` convierte el knob lineal 0..1 en una curva suave (sin saltos). Los amps
de 2 canales (p.ej. Super-Sonic 22 Vintage↔Burn) usan
`smoothstepRange(a, b, gain)` para **morfear** de un canal a otro entre `gain=a` y
`gain=b`. Mover esos `a`/`b` cambia "a partir de qué punto del Gain entra el canal
saturado".

### 3.4 `sag` — compresión/esponjosidad de la etapa de potencia (amps)
Envolvente lenta que simula la caída de tensión del power amp bajo demanda:
```cpp
sag += (env - sag) * (env>sag ? attack : release);
const float sagDrop = 1.0f / (1.0f + sag * (k0 + k1*gain));   // baja la ganancia al pegar fuerte
```
- Más `k0/k1` o `release` largo = más "esponja"/compresión (válvula con rectificador
  de válvulas). Menos = más firme/inmediato (rectificador de estado sólido).
- Ej.: el BENDER SUPERNOVA 22 usa sag **suave** porque el Super-Sonic rectifica con
  diodos; el BOX DC30 (AC30/EL84) usa algo más de sag.

### 3.5 `DcBlock`
High-pass de ~5 Hz que quita el offset DC que dejan los clippers asimétricos. No
afecta el tono audible; no tocar.

### 3.6 `RBAutoMakeup` (`_shared/automakeup.hpp`) — nivel de los drives
Iguala el RMS de salida al de entrada (~200 ms) para que **el Gain cambie la
distorsión, no el volumen**. Por eso todos los drives quedan al mismo nivel que el
bypass. Detalles:
- **Stereo-linked**: usá `processStereo(dryL, dryR, wetL, wetR, outL, outR)` con
  **una sola instancia** `makeup` por plugin (no una por canal). Calcula un único
  envelope/gain del promedio de ambos canales y lo aplica idéntico a L y R, así
  `gainL == gainR` por construcción. Dos instancias independientes (el viejo
  `process(dry,wet)` por canal) divergían cuando el host mandaba L≠R (dither, un
  bloque estéreo anterior, DC offset) → la diferencia de gain L/R se oía como un
  **phaser/barrido lento** encima del tono. `process()` (mono) queda solo para
  wahs/octavers donde no importa.
- `snap()` se llama al mover un knob → reajuste rápido (~8 ms) para que no haya un
  "blip" de volumen al girar. Tiene un **refractario de ~250 ms**: un host que
  automatiza un knob a control-rate no puede re-armar la ventana rápida en loop
  (si no, el makeup se vuelve un compresor que bombea).
- El gain aplicado se **glidea** (~30 ms) para que un cambio de régimen
  (clipped→linear al decaer una nota) nunca escalone el nivel.
- Techo de seguridad: `target` máximo 4× (+12 dB) (no amplifica ruido en silencio).
- Si querés que un drive sí **suba volumen** (boost), reducí su rol o sacalo del
  `run()` — pero romperías la coherencia de niveles entre pedales.

### 3.7 `ReverbCore` (`_shared/reverb_core.hpp`) — los reverbs "Studio"
Freeverb: **8 combs** en paralelo → **4 all-pass** en serie, por canal, + una
modulación suave en la cola (Depth). Controles compartidos:

| Knob | Constante | Efecto |
|---|---|---|
| Time | `feedback = 0.70 + time*0.275` | largo de la cola |
| Tone | `damp = (1-tone)*0.45 + dampBias` | oscuro↔brillante |
| Depth | `modDepth` | wobble/chorus de la cola |
| Mix | `mix` | wet/dry |

El **voicing** por rack se setea con `setVoicing(sizeScale, dampBias, apFb)`:
Verb (hall, combs grandes), Chamber (más corto y oscuro), Plate (corto, brillante,
mucha difusión). Para hacer un reverb más grande/largo: subí `sizeScale` o el techo
del `feedback`.

### 3.8 `MarkAmpCore.h` (`mark_shared/`) — Mesa Mark III/IV
Core compartido con bandera `lead`/`crunch`. Modela el famoso **EQ gráfico en "V"**
de los Mesa (`geq750.peak(..., -1.8 - 5.6*lead...)` = scoop de medios en lead). Ojo:
los nombres de carpeta están cruzados (ver el comentario del header):
`Amp_CA85→Mark III (crunch)`, `Amp_CA38→Mark IV (lead)`.

---

## 4. Mapa rápido "quiero cambiar X → tocá Y"

| Quiero… | Dónde | Cómo |
|---|---|---|
| Más/menos distorsión | `process()` | subir/bajar el `drive` (coef. de `gain`) |
| Distorsión más "válvula" / dulce | clipper | usar `asymTube` y/o subir un poco el `bias` |
| Distorsión más áspera/agresiva | clipper | clip más duro / `bias≈0` / menos `softClip` final |
| Más graves / panza | `updateFilters()` | bajar `hz` de `inputHp`/`*Tighten`, subir `dB` de `bassShelf` |
| Más apretado/definido | `updateFilters()` | subir `hz` de los high-pass de entrada |
| Más presencia/mordida | `updateFilters()` | subir `dB`/`hz` del `*Bite`/`*Presence` |
| Menos fizz/agudo áspero | `updateFilters()` | bajar `hz` del low-pass de salida (`speakerLp`/`toneLowPass`) |
| Scoop de medios (metal) | `updateFilters()` | `peak` negativo ~400–750 Hz |
| Más cuerpo de caja (amp) | `updateFilters()` | etapas `speaker*` (HP, body, bite, fizz-notch, LP) |
| Más esponja/compresión (amp) | `process()` | subir `sag` (`k0/k1`, `release`) |
| Más/menos volumen del slot | `process()` | el `level = .../(...)` final (drives usan auto-makeup) |
| Cómo responde el knob | `updateFilters()`/`process()` | la curva `smoothstep`/`smoothstepRange` |
| Sonido por defecto al insertar | `XxxParams.h` | los valores `kXxxDef[]` |

**Nunca quites** el `softClip(y*level) * 0.98` final ni el `DcBlock`: son los que
garantizan que la salida quede acotada (sin clip digital ni DC). Si subís mucho el
`drive`, ese límite final te protege.

---

## 5. Notas por familia (idioma DSP + qué tocar)

### Amps (8)
`Core` propio (o `mark_shared`). Cadena típica: entrada/bright → 1+ etapas
`asymTube` (preamp) → tone stack (`Biquad`) → power amp `asymTube` + `sag` →
modelo de parlante (`speaker*`) → `level`. **Tono**: tone stack y etapas `speaker*`
en `updateFilters()`; **ganancia/saturación y sag** en `process()`. El Gain de RS
suele morfear canal limpio↔saturado (`smoothstepRange`). Ver `en30/EN30Core.h` y
`tw22/TW22Core.h` como referencia comentada.

### Drive / Distortion / Fuzz / Boost
Cadena: high-pass de entrada → voicing pre-clip (`Biquad`) → `drive` → **clipper** →
tone post-clip → `level`, todo con **`RBAutoMakeup`**. **Tono**: el clipper (tipo y
`bias`) define el carácter; la cantidad la da `drive`; el balance lo dan los
`peak/shelf` de pre y post clip. Fuzz = clip más duro y menos head-room.

### Dynamics (comp / limiter / gate)
Detector de envolvente (`attack`/`release`) → cálculo de ganancia (ratio/umbral) →
VCA. **Tono/feel**: `attack`/`release` (rapidez), `ratio`/`threshold` (cuánto
comprime). `studiocomp` (dbx160) usa detector **RMS** + soft-knee "OverEasy".

### EQ (gráficos y paramétricos)
Bancos de `Biquad` (`peak` por banda en gráficos; `peak`+`shelf` con freq/Q
ajustables en paramétricos). **Tono**: son EQ directos — cada banda es un `peak` con
su `hz` fijo y `dB` = knob. `ampeq` (FBM-1) es especial: modela el **tone stack
pasivo Fender real** (interacción Bass/Mid/Treble, scoop real), no bandas
independientes.

### Delay / Echo
Línea de retardo (buffer circular) + realimentación (`feedback`) + filtro en el lazo
(repeticiones que se oscurecen). Variantes: BBD/analógico (oscuro, saturado), tape
(wow & flutter + cross-feed), oil-can, PT2399 digital, RE-201. **Tono**: el filtro
del lazo (qué tan oscuro decae), la saturación por repetición, y el `feedback`.

### Reverb
Dos motores: `ReverbCore` Freeverb (racks Studio Verb/Chamber/Plate, ver §3.7) y
modelos dedicados (RV-2 digital, EMT-140 plate `VOODOO`, spring tanks). **Tono**:
`feedback`/`damp`/`sizeScale` (Freeverb) o las constantes de tanque/difusión.

### Modulation (chorus / flanger / phaser / vibe / trem / rotary / ring)
Un **LFO** modula algo: el delay corto (chorus/flanger), una cadena de all-pass
(phaser/vibe), el volumen (tremolo), o multiplica por portadora (ring mod). **Tono**:
`Rate`/`Depth` (LFO), `Feedback`/`Regen` (resonancia del flanger/phaser),
`Mix`/`Wave`. Los de bajo (`bass_*`) mantienen los graves secos (high-pass en el wet).

### Wah / Filter
Band-pass resonante barrido por: posición de pedal, envolvente de entrada (Sens/
Attack/Release) o LFO (auto). **Tono**: rango de barrido (freq min/max), `Res`
(resonancia/Q), y la fuente de barrido. `bit_cruncher` es lo-fi (reduce bits/SR).

### Pitch / Octave
Octavadores analógicos (comparador + flip-flop = una octava abajo, `bass_sub_octave`/
`octavius`) o pitch shifters digitales (`studio_pitch`, `multi_pitch` por ring-mod).
**Tono**: `Tone` (low-pass del sub) y `Mix` (blend con seco).

### Emulator / Preamp
`acoustic_simulator` (re-ecualiza eléctrica→acústica con body/mids) y `eden_wtdi`
(preamp de bajo: drive + comp óptico + EQ activo 3 bandas + contour). **Tono**: las
bandas de EQ y el contour.

---

## 6. Cómo probar un cambio (offline) y cómo llega a la app

### 6.1 Test offline (rápido, sin recompilar el VST)
Para los Cores en `.h` (amps) podés compilar el DSP solo y verificar estabilidad +
respuesta, sin DPF. Ejemplo (lo usé para validar el TW22):

```bash
cd rig_builder/vst/src/tw22
cat > /tmp/t.cpp <<'EOF'
#include "TW22Core.h"
#include <cstdio>
#include <cmath>
int main(){
  tw22::TW22Core c; c.setSampleRate(48000.f);
  c.setGain(.5f); c.setBass(.5f); c.setMid(.55f); c.setTreble(.65f);
  float peak=0; 
  for(int i=0;i<48000;i++){ float in=0.5f*std::sin(2*M_PI*220.f*i/48000.f);
    float o=c.process(in); if(std::isnan(o)||std::isinf(o)){printf("NaN!\n");return 1;}
    peak=std::fmax(peak,std::fabs(o)); }
  printf("peak=%.3f\n", peak);   // debe quedar <= ~1.0
}
EOF
c++ -std=c++14 -O2 -I. /tmp/t.cpp -o /tmp/t && /tmp/t
```
Chequeá: **sin NaN/Inf**, **peak ≤ ~1.0**, el silencio decae a 0, y que el RMS no se
dispare al subir el Gain (el `level` final lo compensa). Para pedales con la DSP
inline en `*Plugin.cpp`, podés extraer la clase `Core` a un `.h` (como se hizo con
los amps) si querés testearla así.

### 6.2 Cómo llega el cambio a Slopsmith
- **El sonido vive en el binario compilado.** Editar `*Plugin.cpp`/`*Core.h` NO
  cambia nada hasta **recompilar el `.vst3`**. Eso lo hace **CI** (los 3 workflows
  `build-{linux,macos,windows}-vst3.yml`) al mergear a `main`. Localmente el `.vst3`
  bundleado tiene el binario viejo hasta el próximo build.
- **La cara (canvas) sí es en vivo**: `pedal_canvas.js` se sirve sin cache, así que
  cambios visuales se ven al **Cmd+Q + reabrir** Slopsmith (no hay hot-reload; el
  plugin está symlinkeado).
- Tras recompilar/instalar binarios nuevos: **Cmd+Q completo** y reabrir.

### 6.3 Caveats
- No commitees/pushees salvo que se pida.
- Cambios de DB (renames): se prueban en copia primero (ver `routes.py`).
- Mantené `softClip(...)*0.98` y `DcBlock` finales.

---

## 7. Inventario completo (108 plugins)

Formato: **Referencia real** | Nombre bundled (parodia) | RS gear | Knobs RS | dir fuente.
El dir es bajo `rig_builder/vst/src/`. (Los racks/parodias mantienen el binario con
su `NAME` original adentro; el dir fuente es el de la DSP.)

### Amps (8)
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Vox AC30 Top Boost / EL84 | BOX DC30 | Amp_EN30 | Gain/Bass/Mid/Treble/Pres/Bright | `en30` |
| Fender Super-Sonic 22 / 6V6 | BENDER SUPERNOVA 22 | Amp_TW22 | Gain/Bass/Mid/Treble | `tw22` |
| Fender 57 Deluxe / 5E3 tweed | TW26 | Amp_TW26 | Gain/Bass/Mid/Treble/Pres | `tw26` |
| Fender Bassman 5F6-A / 5881 | TW40 | Amp_TW40 | Gain/Bass/Mid/Treble/Pres | `tw40` |
| Marshall DSL100H / JCM2000 | DSL100 | Amp_MarshallDSL100H | Gain/Bass/Mid/Treble/Pres/Res | `dsl100` |
| Mesa Dual Rectifier (Red) | DualRect | Amp_CA100 | Gain/Bass/Mid/Treble/Pres | `dual_rect` |
| Mesa Mark IV (lead) | MarkIV | Amp_CA38 | Gain/Bass/Mid/Treble | `mark_iv` (+`mark_shared`) |
| Mesa Mark III R2 (crunch) | MarkIII | Amp_CA85 | Gain/Bass/Mid/Treble | `mark_iii` (+`mark_shared`) |

### Drive / Distortion / Fuzz / Boost
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Boss SD-1 | SD-1 | Pedal_SuperDrive | Gain/Tone | `super_drive` |
| Boss DS-1 | DS-1 | Pedal_Distortion | Gain/Tone | `standard_distortion` |
| DOD 250 | Vintage Distortion | Pedal_VintageDistortion | Gain/Tone | `vintage_distortion` |
| Boss OS-2 | OS-2 | Pedal_LineDrive | Gain/Tone | `line_drive` |
| Boss HM-2 | HM-2 | Pedal_MetalDistortion | Gain/Tone | `alloy_distortion` |
| Boss MT-2 | MT-2 | Pedal_ShredZone | Gain/Bass/Mid/Treble | `shred_zone` |
| MOSFET custom OD | CDO | Pedal_CustomDrive | Gain/Tone/Voice | `custom_drive` |
| Marshall GV-2 Guv'nor+ | GM-2 | Pedal_MarshallGuvnorPlus | Gain/Bass/Mid/Treble/Deep | `marshall_guvnor_plus` |
| Germanium OD | Germanium Drive | Pedal_GermaniumDrive | Gain/Tone | `germanium_drive` |
| Rangemaster booster | Range Booster | Pedal_RangeBooster | Boost | `range_booster` |
| Octave fuzz | Super-Buzz | Pedal_BuzzOne | Gain/Tone | `super_buzz` |
| Triangle Big Muff-ish | Big Buzz | Pedal_BuzzToo | Gain/Tone | `big_buzz` |
| Chief silicon fuzz | BZ-1 | Pedal_FuzzWasHe | Gain/Tone | `bz_1` |
| Germanium fuzz-tone | Buzz-Tone | Pedal_CaptFuzzle | Gain/Tone | `buzz_tone` |
| Pro Co RAT (bajo) | Mouse | Bass_Pedal_BassDistortion | Gain/Tone/Filter | `bass_distortion` |
| EHX Bass Big Muff | Bass Big Buzz | Bass_Pedal_BassFuzz | Gain/Tone/Filter | `bass_fuzz` |
| Darkglass B3K (bajo) | BLACKBRASS | Bass_Pedal_BassOverdrive | Blend/Gain/Filter/Tone | `bass_overdrive` |

### Dynamics (comp / limiter / gate)
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Dyna Comp | dyna comp | Pedal_Compression | Comp/Attack/Release | `dynamics_compression` |
| dbx 160 (RMS VCA) | HZX | Rack_StudioCompressor | Threshold/Ratio/Attack/Release | `studiocomp` |
| EBS MultiComp (bajo) | Multi Comp | Bass_Pedal_MBComp | Compress/Filter/Rate | `bass_multi_comp` |
| Compresor saturado | Beta Fist | Pedal_Swole | Smash/Rate | `swole` |
| Limiter de pico | LM-2 | Pedal_Limiter | Limit/Rate | `limiter` |
| Noise gate | NF-1 | Pedal_NoiseGate | Thresh/Rate | `noise_gate` |

### EQ
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Gráfico 5 bandas | EQ5 | Pedal_EQ5 | 63/250/750/2200/5700 | `eq5` |
| Gráfico 8 bandas | GE-8 | Pedal_EQ8 | 50/100/200/400/800/1600/3200/6400 | `eq8` |
| Gráfico 8 bandas (bajo) | GEB-8 | Bass_Pedal_BassEQ8 | 30…16000 | `basseq8` |
| Paramétrico 4 bandas (GML) | LNG | Rack_StudioEQ | Bass/LoMid/HiMid/Treble (+freq/Q) | `studioeq` |
| Sweepable 5 bandas (API-550) | G-550 | Rack_StudioGraphicEQ | Bass/LoMid/Mid/HiMid/Treble (+freq) | `studiographiceq` |
| Fender 5F6-A tone stack | FBM-1 | Pedal_AmpEQ | Bass/Mid/Treble/BassFreq/MidShift/TrebleFreq | `ampeq` |

### Delay / Echo
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| MF-104 BBD | FM104 | Pedal_AnalogueDelay | Time/Feedback/Mix | `analog_delay` |
| DM-2 BBD | DM-2 | Pedal_NPNDelay | Time/Feedback/Mix | `npn_delay` |
| DE-7 stereo | No Fi Echo | Pedal_NoFiEcho | Time/Feedback/Mix | `nofi_echo` |
| Binson Echorec (válvula) | Valve Echo | Pedal_ValveEcho | Time/Feedback/Mix | `valve_echo` |
| PT2399 cósmico | Galaxy Echo | Pedal_CosmicEcho | Time/Feedback/Mix | `cosmic_echo` |
| Tel-Ray oil-can | Oil Can Echo | Pedal_OilCanEcho | Time/Feedback/Mix | `oil_can_echo` |
| Modulated delay DLL10 | DL9 | Pedal_ModDelay | Time/Feedback/Mix/Rate/Depth | `mod_delay` |
| Stereo delay | StudioDelay | Rack_StudioDelay | TimeL/TimeR/Feedback/Filter/Mix | `studio_delay` |
| Roland RE-201 Space Echo | TapeEcho | Rack_TapeEcho | Time/Feedback/Filter/Stereo/Mix | `tape_echo` |
| Boss DM-2 (bajo, filter) | DL-3 | Bass_Pedal_BassFilterDelay | Time/Feedback/Mix/Filter | `bass_filter_delay` |
| Tape echo (bajo) | SE-3 | Bass_Pedal_BassFilterEcho | Time/Feedback/Mix/Filter | `bass_filter_echo` |

### Reverb
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Boss RV-2 digital | RV-2 | Pedal_DigitalVerb | Time/Mix/Depth/Tone | `digital_verb` |
| EMT 140 plate | VOODOO | Pedal_PlateVerb | Time/Depth/Mix/Voice | `plate_verb` |
| Holy Grail spring | Holy Spring | Pedal_SpringReverb | Time/Mix/Depth | `spring_reverb` |
| Tube spring tank | Real Spring | Pedal_TubeSpring | Mix/Depth | `tube_spring` |
| Freeverb hall | StudioVerb | Rack_StudioVerb | Time/Tone/Depth/Mix | `studio_verb` (`reverb_core`) |
| Freeverb chamber | StudioChamber | Rack_StudioChamber | Time/Tone/Depth/Mix | `studio_chamber` (`reverb_core`) |
| Freeverb plate | StudioPlate | Rack_StudioPlate | Time/Tone/Depth/Mix | `studio_plate` (`reverb_core`) |

### Modulation (chorus / flanger / phaser / vibe / tremolo / rotary / ring / thickener)
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Boss CE-2 chorus | CH-2 | Pedal_Chorus | Rate/Depth/Mix | `chorus` |
| Stereo BBD chorus | Deja Chorus | Pedal_Chorus20 | Rate/Depth/Mix | `chorus_20` |
| Digital chorus CH-5 | CH-5 | Pedal_DigitalChorus | Rate/Depth/LoFilter/HiFilter/Mix | `digital_chorus` |
| Analog stereo chorus | 134 Stereo Chorus | Pedal_VintageChorus | Rate/Depth/Mix | `analog_chorus` |
| Boss RCE-10 ensemble | StudioChorus | Rack_StudioChorus | Rate/Depth/Mix/LoFilter/HiFilter/Stereo/Delay | `studio_chorus` |
| Clone Theory | Attack of the Clones | Pedal_SendInTheClones | Clones/Depth/Mix | `send_in_the_clones` |
| Boss CEB-3 (bajo) | CB-3 | Bass_Pedal_BassChorus | Rate/Depth/LoFilter/Mix | `bass_chorus` |
| Boss BF-2 flanger | FL-2 | Pedal_ClassicFlanger | Rate/Depth/Mix | `classic_flanger` |
| MXR Flanger (SAD1024) | N117R Flanger | Pedal_80sFlanger | Rate/Depth/Mix | `eighties_flanger` |
| MF-108M Cluster Flux | FM107 | Pedal_ModernFlanger | Rate/Depth/Regen/Mix | `modern_flanger` |
| Deluxe Electric Mistress | Deluxe Servant | Pedal_VintageFlanger | Rate/Depth/Mix | `vintage_flanger` |
| Stereo flanger | StudioFlanger | Rack_StudioFlanger | Rate/Depth/Regen/Tone/Mix | `studio_flanger` |
| Boss BF-2/BF-3 (bajo) | FL-3 | Bass_Pedal_BassFlanger | Rate/Depth/Filter/Mix | `bass_flanger` |
| MXR Phase 90 | phase 90 | Pedal_Phaser | Rate | `phaser_363` |
| AP-7 8-stage phaser | Rocket Phase | Pedal_PlanePhase | Rate/Depth/Mix | `plane_phase` |
| ET-25B phaser | PH-1 | Pedal_ShaverPhaser | Rate/Depth | `shaver_phaser` |
| Stereo phaser | StereoPhaser | Rack_StereoPhaser | Rate/Depth/Mix | `stereo_phaser` |
| Analog phaser (bajo) | phase 99 | Bass_Pedal_BassPhase | Rate/Depth/Mix/Filter | `bass_phase` |
| Boss VB-2 vibrato | VB-2 | Pedal_MultiVibe | Speed/Mix/Waveform | `multi_vibe` |
| Uni-Vibe (amp) | Multi-Vibe | Pedal_AmpVibe | Speed/Mix | `amp_vibe` |
| Uni-Vibe/Shin-ei | UniMod | Pedal_OmniMod | Rate/Depth/Mix | `omni_mod` |
| Envelope vibe | Oceanduct | Pedal_AutoVibe | Sens/Attack/Release/Mix | `auto_vibe` |
| Univibe vibrato (rack) | StereoAnalogVibe | Rack_StereoAnalogVibe | Speed/Waveform/Mix | `stereo_analog_vibe` |
| Marshall SV-1 Supervibe | UV-1 | Pedal_MarshallSupervibe | Rate/Depth/Mix/Wave | `marshall_supervibe` |
| Boss TR-2 tremolo | TR-2 | Pedal_MultiTrem | Speed/Mix/Waveform | `multi_trem` |
| Colorsound tremolo | Tremolo | Pedal_Tremolo | Speed/Mix | `tremolo` |
| Demeter Tremulator | Mega-Trem | Pedal_AmpTrem | Speed/Depth | `amp_trem` |
| Envelope tremolo | Dyna-Trem | Pedal_TremOle | Sens/Attack/Release/Mix | `trem_ole` |
| Tube tremolo (rack) | StereoTubeTrem | Rack_StereoTubeTrem | Speed/Mix/Waveform | `stereo_tube_trem` |
| Leslie/RT-20 rotary | RT-2 | Pedal_BakedRotatoe | Rate/Depth/Mix/Balance | `baked_rotatoe` |
| Rotary (rack) | RotaVibe | Rack_RotaVibe | Rate/Depth/Mix/Balance | `rota_vibe` |
| Maestro RM-1A ring mod | RingMod | Pedal_RingMod | Depth/Waveform/Sensitivity/Attack | `ring_mod` |
| Thickener/widener | MIME | Pedal_Enbiggenator | Rate/Depth/Mix | `enbiggenator` |
| Thickener (bajo) | ENBIGGEN | Bass_Pedal_BassEnbig | Rate/Depth/Mix/Filter | `bass_enbig` |

### Wah / Filter
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Mu-Tron III | BU-TRON III | Pedal_AutoFilter | FilterType/Res/Sens/Attack/Release | `auto_filter` |
| Dunlop Cry Baby GCB-95 | Cry Man | Pedal_USWah | Auto/Pedal/Sens/Speed | `us_wah` |
| Vox V847 / Clyde McCoy | BOX B847 | Pedal_UKWah | Auto/Pedal/Sens/Speed | `uk_wah` |
| Morley Bad Horsie | Jockey Bad | Pedal_ModernWah | Auto/Pedal/Sens/Speed | `modern_wah` |
| Moog MF-105 MuRF | FM105 | Pedal_BobFilter | Sens/Attack/Release/Mix/Filter | `bob_filter` |
| Lofinator lo-fi filter | Lo Fi Filter | Pedal_LoFiFilter | FilterType/Mix | `lofi_filter` |
| Envelope filterbank | SynthFilter | Rack_SynthFilter | Sens/Attack/Release/FilterType/Mix | `synth_filter` |
| Auto wah filter (rack) | StudioWahFilter | Rack_StudioWahFilter | Sens/Attack/Release/Pedal/Auto | `studio_wah_filter` |
| Bit crusher ADC0804 | BitCruncher | Pedal_BitCruncher | Attack/FilterType/Mix/Release/Sens | `bit_cruncher` |
| Cry Baby Bass Wah 105Q | Bass Wah | Bass_Pedal_BassWah | Pedal/Sens | `bass_wah` |
| Q-Tron (bajo) | Qtrix | Bass_Pedal_BassAutoFilter | FilterType/Res/Sens/Attack/Release/Speed | motor envelope-filter compartido |

### Pitch / Octave
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Boss OC-2 octave | OC-5 | Pedal_Octavius | Tone/Mix | `octavius` |
| Octave-up limpio | OCTUP | Pedal_OctaveUp | Tone/Mix | `octave_up` |
| MF-102 ring pitch | Multi Pitch | Pedal_MultiPitch | Pitch1/Tone/Mix | `multi_pitch` |
| Pitch shifter (rack) | StudioPitch | Rack_StudioPitch | Pitch/Tone/Mix/Pan | `studio_pitch` |
| Boss OC-2 sub (bajo) | SO-2 | Bass_Pedal_BassSubOctave | Mix | `bass_sub_octave` |
| Guitarra→bajo octaver | Bass Emulator | Pedal_BassEmulator | Body/Tone | `bass_emulator` |

### Emulator / Preamp
| Referencia | Bundled | RS gear | Knobs | dir |
|---|---|---|---|---|
| Acoustic simulator | Acoustic Emulator | Pedal_AcousticEmulator | Tone/MidShift/Body/Mid | `acoustic_simulator` |
| Eden WTDI preamp (bajo) | WT-DX | Bass_Pedal_EdenWTDI | Bass/Mid/Treble/Gain/Enhance/Compression/LoShift/MidShift | `eden_wtdi` |

---

## 8. Resumen para iterar tono

1. Encontrá el dir del plugin en el inventario (§7).
2. Abrí `*Plugin.cpp` (o `*Core.h`) → mirá `updateFilters()` (EQ/voicing) y `process()` (drive/sag/nivel).
3. Cambiá constantes (§4 = mapa "quiero X → tocá Y").
4. Si es un Core en `.h`, testealo offline (§6.1).
5. El cambio de **sonido** se oye recién al **recompilar el VST por CI**; la **cara** se ve al reabrir Slopsmith.
