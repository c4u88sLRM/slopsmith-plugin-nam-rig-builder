# Schematics reference — índice

> Traducción de los esquemáticos (PDF/imagen) a fichas `.md` por gear, para
> releer barato en sesiones futuras (sin recargar PDFs ni `.cpp` enteros).
>
> - **Una ficha por gear**: `docs/schematics/<codename>.md` (ver `_TEMPLATE.md`).
> - Esquemáticos fuente: `/Users/nacho/Files/slopsmith/amps/<Real (CODENAME)>/`
>   y `/Users/nacho/Files/slopsmith/pedals/<nombre>.{pdf,png,jpg,gif}`.
> - DSP: `rig_builder/vst/src/{amps,pedals,racks}/<codename>/`.
> - Marca parodia (la cara nunca dice la marca real): Vox→Box, Fender→Bender,
>   Mesa→Silla, Marshall→Marsten, Roland→Ronald, Orange→Citrus, Dr.Z→Mr.Y,
>   Matchless→Unparallel, ENGL→Engel, Polytone→Polystone, Hiwatt→Lovolt,
>   Laney→¿?, Universal Audio→Multiversal, Tech21 SansAmp→SinAmp, Neve→Meve,
>   Budda→Ganddi, Ampeg→Sampleg, Randall/Ampeg VH→Sampleg.

Estado: ✅ ficha hecha · 📄 esquemático disponible (pendiente) · ⚠️ sin esquemático (modelo por datasheet/conocimiento).

## Amps de guitarra

| codename (dir) | parodia (NAME) | RS gear | unidad real | esquemático | ficha |
|---|---|---|---|---|---|
| mark_iii | Silla Boogie Mark III | Amp_CA85 | Mesa/Boogie Mark III | `amps/Mesa Mark III (CA_85)/boogie_mkiii.pdf` | ✅ `mark_iii.md` |
| mark_ii | Silla Boogie Mark II | Amp_CA38 | Mesa/Boogie Mark II | `amps/Mesa Mark II (CA_38)/` | 📄 |
| dual_rect | Silla Boogie Duo Rectifier | Amp_CA100 | Mesa Dual Rectifier | `amps/Dual Rectifier (Cali_100)/` | 📄 |
| en30 | Box DC30 | Amp_EN30 | Vox AC30**C2** (Custom, SS-rect) | `amps/vox ac30 (en30)/Vox_ac30c2.pdf` | ✅ `en30.md` |
| dc30_unparallel | Unparallel DC30 | Amp_BT30 | Matchless DC30 | `amps/Matchless DC30 (BTQ-30)/` | 📄 |
| chieftain_unparallel | Unparallel Chieftain | Amp_BT15 | Matchless Chieftain | `amps/Matchless Chieftan (BTQ-15)/` | 📄 |
| plexi | Marsten Plexi | Amp_MarshallPlexi | Marshall Plexi | `amps/Marshall Plexi/` | 📄 |
| jtm45_marsten | Marsten JTM45 | Amp_MarshallJTM45 | Marshall JTM45 | `amps/Marshall JTM45/` | 📄 |
| jcm800_marsten | Marsten JCM800 | Amp_GB50 (*) | Marshall JCM800 | `amps/Marshall JCM800/` | 📄 |
| dsl100 | Marsten DSL100 | Amp_MarshallDSL100H | Marshall DSL100H | `amps/Marshall DSL100/` | 📄 |
| dsl15_marsten | Marsten DSL15 | Amp_MarshallDSL15H | Marshall DSL15H | `amps/Marshall DSL15/` | 📄 |
| jvm410_marsten | Marsten JVM410 | Amp_MarshallJVM410H | Marshall JVM410H | `amps/Marshall JVM410/` | 📄 |
| bluesbreaker_marsten | Marsten Bluesbreaker | Amp_Marshall1962Bluesbreaker | Marshall 1962 Bluesbreaker | `amps/Marshall Bluesbreaker/` | 📄 |
| vs100_marsten | Marsten VS100 | Amp_HG180 | Marshall VS100RH | `amps/Marhsall VS100RH (HG-180)/` | 📄 |
| dr504_lovolt | Lovolt DR504 | Amp_HG500 | Hiwatt DR504 (Custom 50) | `amps/Hiwatt DR504 (HG500)/DR504_Complete.pdf` | ✅ `dr504_lovolt.md` |
| dr103_lovolt | Lovolt DR103 | Amp_HG100 | Hiwatt DR103 (Custom 100) | `amps/Hiwatt DR103 (HW100B)/` | ✅ (en `dr504_lovolt.md`) |
| engel_fireball | Engel Fireball | Amp_EN50 | ENGL Fireball | `amps/ENGL Fireball (EN-50)/` | 📄 |
| ems_mry | Mr.Y EMS | Amp_GB50 (*) | Dr.Z EMS | `amps/Dr Z. EMS (GB-50)/` | 📄 |
| maz38_mry | Mr.Y Maz38 | Amp_GB38 | Dr.Z Maz 18 / Max 18 | `amps/Dr Z. Max 18 (GB-38)/` | 📄 |
| aor50 | Laney AOR 50 | Amp_GB100 | Laney AOR 50 | `amps/Laney AOR 50 (GB100)/` | 📄 |
| superdrive45 | Ganddi Superdrive 45 | Amp_BT45 | Budda SuperDrive 45 | `amps/Budda SuperDrive 45 (BTQ_45)/` | 📄 |
| tw22 | Bender SuperNova 22 | Amp_TW22 | Fender SuperSonic 22 | `amps/Fender SuperSonic 22 (TW22)/` | 📄 |
| tw26 | Bender Deluxe | Amp_TW26 | Fender Deluxe (5E3) | `amps/Fender Deluxe (TW26)/Fender-57-Deluxe-Schematic.pdf` | ✅ `tw26.md` |
| tw40 | Bender Bassman | Amp_TW40 | Fender Bassman 5F6-A | `amps/Fender Bassman Tweed (TW40)/Fender_bassman_5f6a.pdf` | ✅ `tw40.md` |
| jc120_ronald | Ronald JC-120 | Amp_CS120 | Roland JC-120 | `amps/Roland JC-120 (CS-120)/` | 📄 |
| jc90 | Ronald JC-90 | Amp_CS90 | Roland JC-90 | `amps/Roland JC-90 (CS-90)/` | 📄 |
| polystone_minibrute | Polystone MiniBrute | Amp_CS100 | Polytone Mini Brute | `amps/Polytone Mini Brute (CS-100)/` | 📄 |
| vh140_sampleg | Sampleg VH140C | Amp_AT120 | Ampeg/Randall VH-140C | `amps/Ampeg VH-140C (AT-120)/` | 📄 |
| ad50_citrus | Citrus AD50 | Amp_OrangeAD50 | Orange AD50 | ⚠️ sin carpeta | ⚠️ |
| or100_citrus | Citrus OR100 | Amp_OrangeOR100 | Orange OR100 | `amps/Orange OR100/` | 📄 |
| or50_citrus | Citrus OR50 | Amp_OrangeOR50 | Orange OR50 | `amps/Orange OR50/` | 📄 |
| jimmybean_citrus | Citrus Jimmy Bean | Amp_OrangeJimmyBean | Orange (Jimmy Bean) | ⚠️ sin carpeta | ⚠️ |
| orangerockerverb_rumbleverb | Citrus Rumbleverb 50 | (?) | Orange Rockerverb 50 | ⚠️ sin carpeta | ⚠️ |
| orangetinyterror_bigtremor | Citrus Big Tremor | (?) | Orange Tiny Terror | ⚠️ sin carpeta | ⚠️ |
| epiphonecentury_centura | Epicall Centura | (?) | Epiphone Century | ⚠️ sin carpeta | ⚠️ |
| epiphonezephyr_ruby | Epicall Ruby | (?) | Epiphone Zephyr | ⚠️ sin carpeta | ⚠️ |
| gibsonga8_hipzon | Hipzon GA8 | (?) | Gibson GA-8 | ⚠️ sin carpeta | ⚠️ |
| gibsonga79_hipzon | Hipzon GA79 RVT | (?) | Gibson GA-79 RVT | ⚠️ sin carpeta | ⚠️ |
| gibsonga88_hipzon | Hipzon GA88 | (?) | Gibson GA-88 | ⚠️ sin carpeta | ⚠️ |

(*) `Amp_GB50` aparece compartido en varias extracciones; verificar el gear real por canción.

## DI / preamps (transparentes — NO llevan input pre-gain)

| codename | parodia | RS gear | real | esquemático | ficha |
|---|---|---|---|---|---|
| ua610_multiversal | Multiversal 610 | Amp_TubePre / DI_Amp_TubePre | UA 610-A | `amps/UA 610 Preamp (TubePre)/` | 📄 |
| sansamp_sinamp | SinAmp Bass Driver | Amp_BassDriver / DI_Amp_BassDriver | Tech21 SansAmp BDDI | `amps/TECH21 Sansamp Bass Driver (BassDriver)/` | 📄 |
| neve1073_meve | Meve 1073 | Amp_MixerPre / DI_Amp_MixerPre | Neve 1073 | `amps/Neve 1073 Preamp (MixerPre)/` | 📄 |

## Amps de bajo (excluidos del input pre-gain 3.2×)

bt600b_tmax, cs240b_tracer, cs300b_rumble, cs350b_dbs, cs75b_v4b, edene300_wt300,
edenwt550_wt550, edenwt800_wt880, ht100b_lovolt, ht300b_redhead, ht400b_silla,
lt25b_dustup, lt85b_electric, fk800rb, sharke_hb3500, sharke_hb5000,
orangead200b_citrus, sampleg_sbtcl (Ampeg SVT). — fichas 📄 pendientes.

## Pedales

| codename | parodia / bundle | RS gear | real | esquemático | ficha |
|---|---|---|---|---|---|
| germanium_drive | Germanium Drive | Pedal_GermaniumDrive | Hudson Broadcast (Aion Skywave) | `pedals/germanium drive.pdf` | ✅ `germanium_drive.md` |
| custom_drive | OCD (CDO/OC-... wait CDO) | Pedal_CustomDrive | Fulltone OCD-style (op-amp+MOSFET) | `pedals/custom drive.png` | 📄 |
| octavius | OC-5 / Octavius | Pedal_Octavius | Boss OC-2 (octava abajo) | `pedals/octavius.pdf` | 📄 |
| bass_emulator | Bass Emulator | Pedal_BassEmulator | Guitar→Bass (pitch −12 st) | sin esquemático | 📄 |
| super_drive | Super Drive | Pedal_SuperDrive | Boss SD-1 | `pedals/super drive.pdf` | 📄 |
| line_drive | Line Drive | Pedal_LineDrive | Boss OS-2 | `pedals/line drive.png` | 📄 |
| ... | (resto de ~40 pedales) | | | `pedals/*` | 📄 |
