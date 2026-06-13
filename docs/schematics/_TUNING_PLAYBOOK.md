# Playbook: afinar amps DSP para que suenen como la referencia

> Cómo pasamos el **BOX DC30** (Vox AC30C2) de "apagado/áspero" a "muy parecido
> al UAD Ruby". Acá está el método + los arreglos concretos para repetirlo en los
> otros ~59 amps. Herramientas reutilizables en `docs/schematics/tools/`.

---

## 0. EL MÉTODO — loop cerrado offline (lo más importante)

Dejar de afinar a oído ciego. **Medir** contra una referencia:

1. Conseguir una **referencia buena** procesando un **DI seco** (un amp-sim que te
   guste — UAD / NAM / Helix — o una grabación de amp real). DI de prueba:
   `/Users/nacho/Files/slopsmith/ui_public_inputs_*.wav`.
2. **La referencia DEBE estar limpia** — sin clipping en su input. Una referencia
   que clipea está artificialmente **brillante y con "más ganancia"** → te hace
   sobre-brillar y sobre-distorsionar. (Nos pasó: el 1er render de Ruby clipeaba a
   −14 dB de input; toda esa comparación quedó mala.)
3. Armar el **harness offline**: `DI.wav → core C++ del amp → WAV → comparar espectro`.
4. Comparar: **espectro por bandas** (normalizado a misma RMS), **centroide**
   (brillo), **crest factor** (distorsión).
5. Editar el DSP → **recompilar el harness** (incluye el `*Core.h` directo) →
   re-medir. Iterar hasta converger. **No tocar el bundle hasta el final.**

```bash
# compilar el harness (incluye <Amp>Core.h directamente; sin DPF)
clang++ -O2 -std=c++17 -I vst/src/amps/<amp> docs/schematics/tools/harness.cpp -o /tmp/harness
# DI.wav -> raw f32
python3 docs/schematics/tools/wav2f32.py "<DI>.wav" /tmp/di.f32
# procesar (args: in out sr inGain input tbvol normvol treble bass cut master bright)
/tmp/harness /tmp/di.f32 /tmp/out.f32 48000 3.2 1.0 0.5 0.5 0.5 0.3
# comparar contra la referencia
python3 docs/schematics/tools/compare.py /tmp/out.f32 "<ref>.wav"
```

---

## 1. LAS MÉTRICAS y qué significan

| Métrica | Qué es | Cómo usarla |
|---|---|---|
| **Centroide espectral** | brillo global (Hz) | iguálalo a la referencia. Es el "norte" del brillo. |
| **Deltas por banda** (ref − nuestro) | la corrección de EQ que falta | "+" = nos falta ahí; "−" = nos sobra |
| **Crest factor** (pico − RMS) | cantidad de **distorsión**, NO nivel | crest alto = limpio. Compara con la ref para igualar distorsión |
| **Diferencia de RMS** | **nivel** (makeup), NO tono ni distorsión | nivela por oído e ignórala |
| **Test de aliasing** (seno→% inarmónico) | si la dist. áspera es aliasing | >5% = necesitas oversampling; casi nunca lo es (nuestro: 1.4%) |

> ⚠️ **"Suena con más ganancia" ≠ más nivel.** Mide el **crest**. El **brillo** se
> percibe como "más ganancia/agresión". Casi siempre el problema real es brillo o
> nivel, no la cantidad de distorsión.

---

## 2. JERARQUÍA DE PALANCAS (de mayor a menor impacto en "sonar bien")

1. **Input drive (pre-gain de entrada).** El engine de Slopsmith **NO** pasa su
   `setGain('input')` a las etapas VST → el VST recibe la guitarra **cruda/bajita**
   → suena limpio aunque subas la ganancia. **Hornea pre-gain** en `run()`:
   `core.process(3.2f * in)` (≈+10 dB para guitarra). Sin esto, nada suena driven.
2. **Voicing del parlante/cab = EL factor de brillo dominante.** Los modelos venían
   **demasiado oscuros** (lowpass ~7 kHz + notch de fizz) por miedo al "fizz" digital.
   Un cab real microfoneado es **brillante** (presencia 2-6 kHz + aire). Esto solo
   movió el centroide del BOX DC30 de **537 → ~1360 Hz**. Es la palanca grande.
3. **Gain staging** (cargas de placa, caps de acople — del esquemático). Afecta
   ganancia/breakup; impacto tonal **menor** que el parlante.

---

## 3. CHECKLIST DE ARREGLOS TRANSFERIBLES

### Brillo / "suena apagado"
- [ ] **Lowpass del parlante muy bajo (~7 kHz)** → ábrelo a **10-15 kHz**. Culpable #1.
- [ ] **Notch de fizz / parlante oscuro** → no sobre-oscurezcas. Agrega **shelf de
      presencia (+6-9 dB @ ~3 kHz)** + **aire (+8-12 dB @ ~5-6 kHz)** (high-shelf).
- [ ] Reduce el "cajón" de low-mid (bump @ ~300-400 Hz) si la referencia tiene menos.

### Distorsión áspera a ganancia alta
- [ ] Descarta aliasing primero (test del seno). Si <5% inarmónico, **NO es eso**.
- [ ] Causa típica: los shelves de brillo (afinados para limpio) **amplifican los
      armónicos de la distorsión** → fizz. **Hazlos dependientes de la ganancia**:
      que presencia/aire **se replieguen** y el LP **baje** a ganancia alta
      (`gain - K*hot`). Imita al parlante real que doma el fizz. El limpio no se toca.

### Volúmenes y controles
- [ ] Los **volúmenes deben llegar a CERO real** (sin piso tipo `0.16 + 1.62*g`).
      Vol 0 = **silencio**. (Verifica: en el harness, vol 0 → RMS ≈ −240 dBFS.)
- [ ] Los **bright caps** deben desaparecer a volumen 0 (gatea por el volumen,
      `clamp01(vol*K) * brightBleed`), si no filtran señal con el volumen cerrado.
- [ ] **Treble/Bass solo deben afectar su canal.** Si modulan filtros **compartidos**
      (post-mezcla) o el loudness, se filtran a otros canales → fíjalos o gatea por canal.

### Canales oscuros (ej. Normal del AC30)
- [ ] Un canal "oscuro/cálido" sin tone stack debe **cortar agudos en el PREAMP**,
      ANTES del parlante brillante compartido (LP bajo, ~450-700 Hz).
- [ ] Debe **clipear ANTES del lowpass oscuro**, no después (si clipea después, la
      distorsión regenera agudos que el filtro ya no toca → fizzy).

### Arquitectura
- [ ] **Amps/pedales: I/O estéreo (2/2) + UN solo `Core` (dual-mono).** El engine de
      Slopsmith **rutea un plugin 1-out a un solo lado → desbalanceado** (suena más
      fuerte en un lado). NO uses 1/1. Pon `NUM_INPUTS/OUTPUTS = 2`, **un solo `Core`**
      (procesa `inputs[0]`) y escribe la MISMA señal a ambos outputs (`outL = outR = y`).
      Así: **balanceado/centrado** Y **CPU de mono** (un core, no dos). El ahorro de CPU
      viene del core único, no del I/O.

---

## 4. CAVEATS DE LA COMPARACIÓN
- Referencia **limpia** (sin clipping de input — bájalo si clipea).
- **Mismo DI, mismos knobs, misma situación de cab** (los dos amp+cab, o los dos
  amp-solo — si no, comparas el parlante, no el amp).
- **Nivela por oído** antes de juzgar tono (nuestro makeup apunta a volumen alto;
  la referencia suele estar bajita).
- La referencia (Ruby) es **otro modelo**, no la verdad absoluta — la igualas
  porque al usuario le gusta, no porque sea "correcta".

---

## 5. RECETA DE BUILD (mecánica)
```bash
# amp (mono): VST3 (Slopsmith) + AU (Logic) en un build
cd vst/src/amps/<amp>
make DPF_PATH=/tmp/dpf BUILD_DIR=/tmp/b TARGET_DIR=/tmp/t vst3 au   # BUILD_DIR y TARGET_DIR ambos
# instalar binario en los bundles (suelen ser DOS: codename + nombre con espacios)
cp /tmp/t/<NAME>.vst3/Contents/MacOS/<NAME> "vst/amps/<NAME>.vst3/Contents/MacOS/<NAME>"
cp /tmp/t/<NAME>.vst3/Contents/MacOS/<NAME> "vst/amps/<Display Name>.vst3/Contents/MacOS/<NAME>"
codesign --force -s - "vst/amps/<NAME>.vst3"          # adhoc, los dos
# AU para Logic (pruebas A/B):
cp -R /tmp/t/<NAME>.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep -s - ~/Library/Audio/Plug-Ins/Components/<NAME>.component
auval -v aufx <SubID> <BrandID>     # debe decir "1 ch" (mono) y "SUCCEEDED"
```
- DPF prep (una vez): `git submodule update --init --recursive` + `libdgl-opengl.a` en `/tmp/dpf/build/`.
- **Logic NO carga VST3, solo AU.** Y un AU **estéreo (2/2) no aparece en pistas mono** → otra razón para mono.
- Tras editar: el usuario debe **Cmd+Q + reabrir Slopsmith** (no hay hot-reload).

---

## 6. CASO DE REFERENCIA: BOX DC30 (qué movió la aguja, en orden)
1. Input pre-gain 3.2× (de "siempre limpio" a driven).
2. Parlante: LP 7k→18k, +presencia/aire shelves, −low-mid → centroide 537→1360 (≈Ruby).
3. Repliegue de brillo a ganancia alta (`-K*hot`) → quitó el fizz áspero del crank.
4. Volúmenes a cero real + bright-cap gateado → vol 0 silencia.
5. Treble/Bass solo Top Boost (no leak al Normal).
6. Normal oscurecido (LP 5.2k→450, clip-antes-del-LP) → cálido como el real.
7. I/O estéreo (2/2) + UN solo core dual-mono → balanceado en Slopsmith + CPU de mono.
   (Probamos 1/1 mono: el engine lo ruteaba a un solo lado → desbalanceado. Revertido.)

> Gain staging del esquemático (placa 220K, acople C9 470pF) ayudó al feel/breakup
> pero fue **secundario** vs el parlante. Lección: **el cab manda el brillo.**

---

## 7. BARRIDO DE FLOTA: brillar TODOS los amps (2026-06-13)

El "suena apagado" era **sistémico**: TODOS los amps de guitarra tenían el lowpass
del parlante en ~5–9 kHz (+ algunos un "fizz notch" que cortaba el top). Se aplicó
el mismo arreglo del BOX/Deluxe/TW40 a los ~36 restantes.

### Probe de brillo SIN extraer headers (clave para amps inline)
La mayoría de los cores son **inline en su `*Plugin.cpp`** (no headers), así que el
harness de loop-cerrado normal no aplica. Solución: reusar el mecanismo de compilación
de `tools/measure_amp_loudness.py` (sustituye `@PLUGIN_CPP@`/`@CLASS@` y compila el
plugin con DPF). Ver `tools/centroid_harness.cpp.in` + `tools/measure_centroid.py`:
alimenta **ruido blanco** a params default y reporta **centroide + RMS + NaN** de cualquier
amp inline. Uso: `python3 docs/schematics/tools/measure_centroid.py <amp_dir>`.
(Requiere DPF; el driver apunta a `/tmp/dpf` — ajustar si se reubica.)

### Receta de brillo (edits SOLO en `updateFilters()`, sin nuevos miembros Biquad)
Sea `<G>` la variable proxy de drive de ese amp (`pushed`/`drive`/`hot`/`ultraA`/`leadA`…):
1. **`speakerLp`**: base 5–9k → **14500** (cabs cerrados 4x12 Marshall/Mesa) / **16000**
   (combos abiertos, tweed, AC30) / **12500** (vintage lo-fi Gibson/Epiphone) / **13500**
   (Roland SS limpio). Mantener `+...*treble/+...*pres`; retreat `- 3500.0f * <G>`.
2. **`phaseLowPass`** (si existe y base<9k): → 10500, retreat `- 2000.0f * <G>` (2º cuello oculto).
3. **`speakerFizzNotch`** (si es un `setPeaking` con dB NEGATIVO = corte): convertir EN SITIO a
   `setHighShelf(sr, 4700, 0.70, 9.5 + 2.0*treble + 2.0*pres - 4.5*<G>)` (aire que se repliega
   en crank = de-fizz; se mantiene el nombre del miembro).
4. Si NO hay fizzNotch pero hay `presenceShelf`/high-shelf de top: subir su gain base ~+2.5 dB.
5. **`speakerBite`** (pico de medios-altos ~2.7k): +1 dB base, `- 0.5f * <G>`.
- **NO tocar**: gain staging, asymTube/clip, tone stack, params, I/O, pre-gain 3.2×, makeup/level.
- Vintage (Gibson/Epi) tienen tone bipolar (`(knob-0.5)*N`) — NO subir su base (es el tone stack).
- Amps cuyo top lo capa un LP **upstream off-limits** (clip de canal, p.ej. vh140 `bClipLp`) suben
  menos — correcto, no tocar la ruta de clip/drive.

### ⚠️ Loudness: re-medir SIEMPRE después de brillar
Abrir el LP **sube el RMS ~5 dB** (ej. plexi −14→−9). El modelo de loudness del hermano
(`data/amp_loudness_model.json`, normaliza cada amp a **−14 LUFS** en la capa rig-builder)
queda **obsoleto** para los amps tocados → **re-correr** `python3 tools/measure_amp_loudness.py
<amps...>` (merge; preserva los no tocados). Esto resuelve "mismo volumen" objetivamente.
- Gotcha shell: el tool NO hace word-split en **zsh** con `$VAR` suelto → usar `bash` + array
  `"${A[@]}"`, o pasar dirs explícitos. (Mismo gotcha en cualquier loop sobre listas.)
- El tool espera `vst/src/DPF` → `ln -s /tmp/dpf vst/src/DPF` (o donde esté DPF).
