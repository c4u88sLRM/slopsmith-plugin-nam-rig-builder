// Harness offline: DI.f32 -> <Amp>Core -> out.f32, con el MISMO wrapper que el
// plugin (input pre-gain + 0.505 + rbAmpLvl). Compila el Core C++ directo (sin DPF).
//
// EJEMPLO para el EN30 (BOX DC30). Para OTRO amp, adapta 4 cosas:
//   1) #include "<Amp>Core.h"   2) using namespace <ns>;   3) el tipo del Core
//   4) los setters de params (cada amp tiene los suyos) + el input-gain de su run()
//
// Compilar:  clang++ -O2 -std=c++17 -I vst/src/amps/<amp> harness.cpp -o /tmp/harness
// Usar:      /tmp/harness in.f32 out.f32 48000 [inGain input tbvol normvol treble bass cut master bright]
#include "EN30Core.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
using namespace en30;

// copia EXACTA del rbAmpLvl del plugin (techo suave; ver <Amp>Plugin.cpp)
static inline float rbAmpLvl(float x){ const float t=0.90f,c=0.99f,a=(x<0?-x:x);
    if(a<=t) return x; return (x<0?-1.f:1.f)*(t+(c-t)*std::tanh((a-t)/(c-t))); }

int main(int argc, char** argv)
{
    FILE* fi = fopen(argv[1], "rb"); FILE* fo = fopen(argv[2], "wb");
    float sr  = atof(argv[3]);
    float ig  = argc > 4  ? atof(argv[4])  : 3.2f;   // input pre-gain horneado
    float input = argc > 5 ? atof(argv[5]) : 1.0f;
    float tb  = argc > 6  ? atof(argv[6])  : 0.30f;
    float nv  = argc > 7  ? atof(argv[7])  : 0.0f;
    float tr  = argc > 8  ? atof(argv[8])  : 0.20f;
    float bs  = argc > 9  ? atof(argv[9])  : 0.40f;
    float cut = argc > 10 ? atof(argv[10]) : 0.40f;
    float ms  = argc > 11 ? atof(argv[11]) : 0.72f;
    float br  = argc > 12 ? atof(argv[12]) : 0.50f;

    EN30Core c; c.setSampleRate(sr);
    c.setInput(input); c.setTBVol(tb); c.setNormalVol(nv);
    c.setTreble(tr); c.setBass(bs); c.setCut(cut); c.setMaster(ms); c.setBright(br);
    c.setRevLevel(0); c.setDepth(0);

    float x;
    while (fread(&x, 4, 1, fi) == 1) {
        float y = rbAmpLvl(0.505f * c.process(ig * x));   // = el run() del plugin
        fwrite(&y, 4, 1, fo);
    }
    fclose(fi); fclose(fo);
    return 0;
}
