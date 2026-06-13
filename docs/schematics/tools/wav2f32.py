#!/usr/bin/env python3
"""DI.wav -> raw float32 mono (para el harness). Uso: python3 wav2f32.py in.wav out.f32"""
import sys, os
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import wavtools, numpy as np

x, sr = wavtools.read_wav(sys.argv[1])
x.astype('<f4').tofile(sys.argv[2])
print(f"{len(x)} samples  {sr} Hz  {len(x)/sr:.1f}s  peak {float(np.max(np.abs(x))):.3f}")
