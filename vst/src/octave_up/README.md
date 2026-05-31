# OctaveUp - analog octave-up pedal (bundled VST3)

Small two-knob VST for Rocksmith's `Pedal_OctaveUp`.

It keeps the Rocksmith controls:

- `Tone`: octave-up brightness and diode bite.
- `Mix`: dry/octave blend.

The local `pedals/octavup.png` reference is based on a Foxrox Octron-style
analog octave circuit. This is not a full circuit clone; the DSP keeps the
audible cues needed in a Rocksmith rig slot: a buffered dry path, a driven
full-wave rectifier for the octave-up voice, DC blocking after rectification,
and tone shaping so low settings are smoother while high settings get brighter
and more ringy.

## Build (macOS arm64)

```sh
make DPF_PATH=/private/tmp/DPF \
  DPF_BUILD_DIR=/private/tmp/dpf-build/OctaveUp \
  DPF_TARGET_DIR=/private/tmp/dpf-bin \
  PKG_CONFIG=false vst3
codesign --force --sign - /private/tmp/dpf-bin/OctaveUp.vst3
```

Copy `OctaveUp.vst3` to `rig_builder/vst/`.
