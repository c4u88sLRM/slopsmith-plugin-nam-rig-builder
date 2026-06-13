"""Lector WAV (16/24/32-bit int + float) + comparador espectral por bandas.
Sin dependencias más que numpy. Usado por el loop cerrado de afinado de amps
(ver docs/schematics/_TUNING_PLAYBOOK.md)."""
import struct, numpy as np


def read_wav(path):
    d = open(path, 'rb').read()
    assert d[:4] == b'RIFF' and d[8:12] == b'WAVE'
    pos = 12; fmt = None; raw = None
    while pos + 8 <= len(d):
        cid = d[pos:pos+4]; sz = struct.unpack('<I', d[pos+4:pos+8])[0]
        body = d[pos+8:pos+8+sz]
        if cid == b'fmt ':
            fmt = struct.unpack('<HHIIHH', body[:16])
        elif cid == b'data':
            raw = body
        pos += 8 + sz + (sz & 1)
    af, ch, sr, br, ba, bps = fmt
    if af == 3 and bps == 32:
        x = np.frombuffer(raw, dtype='<f4').astype(np.float64)
    elif af == 1 and bps == 16:
        x = np.frombuffer(raw, dtype='<i2').astype(np.float64) / 32768.0
    elif af == 1 and bps == 24:
        a = np.frombuffer(raw, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
        v = a[:, 0] | (a[:, 1] << 8) | (a[:, 2] << 16)
        v = np.where(v >= 2**23, v - 2**24, v); x = v.astype(np.float64) / 2**23
    elif af == 1 and bps == 32:
        x = np.frombuffer(raw, dtype='<i4').astype(np.float64) / 2**31
    else:
        raise ValueError(f"formato no soportado: af{af} bps{bps}")
    if ch > 1:
        x = x.reshape(-1, ch).mean(axis=1)
    return x.astype(np.float64), sr


def spec(x, sr, nfft=16384):
    w = np.hanning(nfft); acc = np.zeros(nfft//2 + 1); n = 0
    for i in range(0, len(x) - nfft, nfft//2):
        acc += np.abs(np.fft.rfft(x[i:i+nfft] * w))**2; n += 1
    return np.fft.rfftfreq(nfft, 1/sr), acc / max(n, 1)


BANDS = [(20, 80, "sub"), (80, 200, "graves"), (200, 500, "low-mid"),
         (500, 1000, "medios"), (1000, 2000, "pres 1-2k"), (2000, 4000, "pres 2-4k"),
         (4000, 6000, "brillo 4-6k"), (6000, 9000, "aire 6-9k"), (9000, 14000, "top 9-14k")]


def crest(x):
    pk = 20*np.log10(np.max(np.abs(x)) + 1e-12)
    rm = 20*np.log10(np.sqrt(np.mean(x**2)) + 1e-12)
    return pk, rm, pk - rm


def compare(ours, ref, sr):
    ro = np.sqrt(np.mean(ours**2)); rr = np.sqrt(np.mean(ref**2))
    f, po = spec(ours/ro, sr); _, pr = spec(ref/rr, sr)
    bd = lambda p, lo, hi: 10*np.log10(np.mean(p[(f >= lo) & (f < hi)]) + 1e-20)
    _, _, cr_o = crest(ours); _, _, cr_r = crest(ref)
    print(f"  nivel: nuestro {20*np.log10(ro):.1f} dBFS  ref {20*np.log10(rr):.1f} dBFS  (dif {20*np.log10(ro/rr):+.1f})")
    print(f"  crest: nuestro {cr_o:.1f} dB  ref {cr_r:.1f} dB   (igual = misma distorsión, NO nivel)")
    print(f"  {'banda':12s}{'ref-nuestro':>13s}")
    tot = 0
    for lo, hi, nm in BANDS:
        dz = bd(pr, lo, hi) - bd(po, lo, hi); tot += abs(dz)
        fl = "  FALTA" if dz > 1.5 else ("  sobra" if dz < -1.5 else "  ok")
        print(f"  {nm:12s}{dz:+12.1f}{fl}")
    cen = lambda p: np.sum(f*p) / np.sum(p)
    print(f"  centroide: nuestro {cen(po):.0f} Hz  ref {cen(pr):.0f} Hz   |error|tot={tot:.1f}")
