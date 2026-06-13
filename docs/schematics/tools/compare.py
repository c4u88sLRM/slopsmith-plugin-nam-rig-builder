#!/usr/bin/env python3
"""Compara la salida del harness vs una referencia. Loudness-normaliza y reporta
bandas + centroide + crest. Uso: python3 compare.py out.f32 ref.wav
(out.f32 = raw float32 del harness; ref.wav = el render de la referencia)."""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wavtools as W, numpy as np

out = np.fromfile(sys.argv[1], '<f4').astype(float)
ref, sr = W.read_wav(sys.argv[2])
n = min(len(out), len(ref))
if n == 0:
    print("ERROR: archivo vacío"); sys.exit(1)
W.compare(out[:n], ref[:n], sr)
