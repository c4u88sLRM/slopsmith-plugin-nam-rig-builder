import json,re,subprocess,sys,tempfile,os
from pathlib import Path
import numpy as np
ROOT=Path("/Users/nacho/Files/slopsmith/rig_builder"); AMPS=ROOT/"vst"/"src"/"amps"
DPF_SRC="/tmp/dpf/distrho/src/DistrhoPlugin.cpp"
TMPL=Path("/tmp/centroid_harness.cpp.in").read_text()
def sdk(): return subprocess.check_output(["xcrun","--show-sdk-path"],text=True).strip()
def parse(d):
    cpps=[p for p in d.glob("*.cpp") if "createPlugin" in p.read_text()]
    if not cpps: return None
    src=cpps[0].read_text()
    m=re.search(r"createPlugin\s*\(\s*\)\s*\{\s*return\s+new\s+(\w+)",src) or re.search(r"class\s+(\w+)\s*:\s*public\s+Plugin\b",src)
    if not m: return None
    return {"cpp":cpps[0].name,"cls":m.group(1)}
def centroid(d):
    info=parse(d)
    if not info: return None,"no-parse"
    probe=TMPL.replace("@PLUGIN_CPP@",info["cpp"]).replace("@CLASS@",info["cls"])
    pp=d/"_cent_probe.cpp"; pp.write_text(probe)
    try:
        tf=tempfile.NamedTemporaryFile(suffix="_c",delete=False); tf.close()
        cmd=["/usr/bin/clang++","-isysroot",sdk(),"-std=c++14","-O2","-I.","-I..",
             "-I/tmp/dpf/distrho","-I/tmp/dpf/dgl","_cent_probe.cpp",DPF_SRC,"-o",tf.name]
        r=subprocess.run(cmd,cwd=d,capture_output=True,text=True)
        if r.returncode!=0: return None,"COMPILE FAIL: "+r.stderr[-400:]
        of=tf.name+".f32"
        run=subprocess.run([tf.name,of],capture_output=True,text=True)
        if run.returncode!=0: return None,"RUN FAIL: "+run.stderr[-300:]
        y=np.fromfile(of,"<f4").astype(float); os.unlink(of)
        if len(y)==0 or np.isnan(y).any(): return None,("NaN!" if len(y) else "empty")
        fr=1<<14; cs=[]; sr=48000.0
        for i in range(0,len(y)-fr,fr//2):
            seg=y[i:i+fr]*np.hanning(fr); S=np.abs(np.fft.rfft(seg)); f=np.fft.rfftfreq(fr,1/sr)
            if S.sum()>1e-9: cs.append((f*S).sum()/S.sum())
        rms=np.sqrt(np.mean(y**2))
        return (float(np.mean(cs)) if cs else 0.0, 20*np.log10(rms+1e-12)),"ok"
    finally:
        pp.unlink(missing_ok=True); Path(tf.name).unlink(missing_ok=True)
if __name__=="__main__":
    for a in sys.argv[1:]:
        r,msg=centroid(AMPS/a)
        if r: print(f"{a:28s} centroid={r[0]:6.0f} Hz   rms={r[1]:6.1f} dB")
        else: print(f"{a:28s} {msg}")
