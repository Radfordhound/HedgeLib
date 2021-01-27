/* Define empty macros for any auto-generators that aren't used. */
#ifndef HL_IN_PACX_TYPE_AUTOGEN
#define HL_IN_PACX_TYPE_AUTOGEN(pacxType)
#endif

#ifndef HL_IN_PACX_LW_AUTOGEN
#define HL_IN_PACX_LW_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_RIO_AUTOGEN
#define HL_IN_PACX_RIO_AUTOGEN(ext, pacxType, type, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_WARS_AUTOGEN
#define HL_IN_PACX_WARS_AUTOGEN(ext, pacxType, type)
#endif

/* ======================================== PACx Types ======================================== */

/* ResRawData is placed at index 0 for fallback purposes. */
HL_IN_PACX_TYPE_AUTOGEN(ResRawData)

/* PACxV1+ */
HL_IN_PACX_TYPE_AUTOGEN(ResAffair)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimator)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimCameraContainer)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimLightContainer)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimMaterial)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimMorphTarget)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimSkeleton)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimTexPat)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimTexSrt)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimVis)
HL_IN_PACX_TYPE_AUTOGEN(ResBFTTFData)
HL_IN_PACX_TYPE_AUTOGEN(ResBitmapFont)
HL_IN_PACX_TYPE_AUTOGEN(ResCameraOffset)
HL_IN_PACX_TYPE_AUTOGEN(ResCharAnimScript)
HL_IN_PACX_TYPE_AUTOGEN(ResCustomData)
HL_IN_PACX_TYPE_AUTOGEN(ResCyanEffect)
HL_IN_PACX_TYPE_AUTOGEN(ResFxColFile)
HL_IN_PACX_TYPE_AUTOGEN(ResGismoConfig)
HL_IN_PACX_TYPE_AUTOGEN(ResGrifEffect)
HL_IN_PACX_TYPE_AUTOGEN(ResHavokMesh)
HL_IN_PACX_TYPE_AUTOGEN(ResKerningData)
HL_IN_PACX_TYPE_AUTOGEN(ResLuaData)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageLight)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageLightField)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageMaterial)
HL_IN_PACX_TYPE_AUTOGEN(ResMiragePixelShader)
HL_IN_PACX_TYPE_AUTOGEN(ResMiragePixelShaderCode)
HL_IN_PACX_TYPE_AUTOGEN(ResMiragePixelShaderParameter)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageShaderList)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageTerrainInstanceInfo)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageTerrainModel)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageVertexShader)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageVertexShaderCode)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageVertexShaderParameter)
HL_IN_PACX_TYPE_AUTOGEN(ResModel)
HL_IN_PACX_TYPE_AUTOGEN(ResModelInstanceInfo)
HL_IN_PACX_TYPE_AUTOGEN(ResMusicScore)
HL_IN_PACX_TYPE_AUTOGEN(ResNameExtra)
HL_IN_PACX_TYPE_AUTOGEN(ResOTFData)
HL_IN_PACX_TYPE_AUTOGEN(ResPacDepend)       /* TODO: These are in the Forces exe? Can these be used?? */
HL_IN_PACX_TYPE_AUTOGEN(ResParticleLocation)
HL_IN_PACX_TYPE_AUTOGEN(ResProbe)
HL_IN_PACX_TYPE_AUTOGEN(ResScoreTable)
HL_IN_PACX_TYPE_AUTOGEN(ResShadowModel)
HL_IN_PACX_TYPE_AUTOGEN(ResSkeleton)
HL_IN_PACX_TYPE_AUTOGEN(ResSM4ShaderContainer)
HL_IN_PACX_TYPE_AUTOGEN(ResSplinePath)
HL_IN_PACX_TYPE_AUTOGEN(ResSplinePath2)
HL_IN_PACX_TYPE_AUTOGEN(ResStaffrollData)
HL_IN_PACX_TYPE_AUTOGEN(ResSurfRideProject)
HL_IN_PACX_TYPE_AUTOGEN(ResSvCol)
HL_IN_PACX_TYPE_AUTOGEN(ResTextMSFile)
HL_IN_PACX_TYPE_AUTOGEN(ResTextMSProject)
HL_IN_PACX_TYPE_AUTOGEN(ResTexture)
HL_IN_PACX_TYPE_AUTOGEN(ResTTFData)
HL_IN_PACX_TYPE_AUTOGEN(ResVoxelContainer)
HL_IN_PACX_TYPE_AUTOGEN(ResXTB2Data)

/* PACxV3+ */
HL_IN_PACX_TYPE_AUTOGEN(ResCodeTable)
HL_IN_PACX_TYPE_AUTOGEN(ResObjectWorld)
HL_IN_PACX_TYPE_AUTOGEN(ResOrcaData)        /* TODO: Unused but in Forces .exe? */
HL_IN_PACX_TYPE_AUTOGEN(ResReflection)
HL_IN_PACX_TYPE_AUTOGEN(ResScalableFontSet)
HL_IN_PACX_TYPE_AUTOGEN(ResScene)
HL_IN_PACX_TYPE_AUTOGEN(ResShaderList)
HL_IN_PACX_TYPE_AUTOGEN(ResSHLightField)
HL_IN_PACX_TYPE_AUTOGEN(ResTerrainGrassInfo)
HL_IN_PACX_TYPE_AUTOGEN(ResText)
HL_IN_PACX_TYPE_AUTOGEN(ResTextMeta)
HL_IN_PACX_TYPE_AUTOGEN(ResTextProject)
HL_IN_PACX_TYPE_AUTOGEN(ResTGAImage)
HL_IN_PACX_TYPE_AUTOGEN(ResVibration)
HL_IN_PACX_TYPE_AUTOGEN(ResVibrationHD)     /* TODO: Present in Forces .exe but unused? Not used until Sonic Tokyo 2020? */

/* PACxV402+ */
HL_IN_PACX_TYPE_AUTOGEN(ResAnim2D)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimatedInstance)
HL_IN_PACX_TYPE_AUTOGEN(ResAnimationPxd)
HL_IN_PACX_TYPE_AUTOGEN(ResBulletMesh)
HL_IN_PACX_TYPE_AUTOGEN(ResDvRawData)
HL_IN_PACX_TYPE_AUTOGEN(ResFlashLight)
HL_IN_PACX_TYPE_AUTOGEN(ResMirageComputeShader)
HL_IN_PACX_TYPE_AUTOGEN(ResMxGraph)
HL_IN_PACX_TYPE_AUTOGEN(ResNavMeshConfig)
HL_IN_PACX_TYPE_AUTOGEN(ResNavMeshTile)
HL_IN_PACX_TYPE_AUTOGEN(ResOpticalKerning)
HL_IN_PACX_TYPE_AUTOGEN(ResScript)
HL_IN_PACX_TYPE_AUTOGEN(ResSkeletonPxd)
HL_IN_PACX_TYPE_AUTOGEN(ResSoftBody)
HL_IN_PACX_TYPE_AUTOGEN(ResVertexAnimationTexture)
HL_IN_PACX_TYPE_AUTOGEN(ResVertexLinkInfo)

/* PACxV403+ */
HL_IN_PACX_TYPE_AUTOGEN(ResPss)

/* ========================================== LW Exts ========================================= */

/* Organized based on frequency information determined via a custom analyzation program. */

/* NOTE: We place ResPacDepend first so we can easily find/skip it as needed. */
HL_IN_PACX_LW_AUTOGEN("pac.d",                  ResPacDepend,                   ROOT,       13, 99)

/* High frequency */
HL_IN_PACX_LW_AUTOGEN("dds",                    ResTexture,                     MIXED,      25, 4)
HL_IN_PACX_LW_AUTOGEN("material",               ResMirageMaterial,              SPLIT,      8,  1)
HL_IN_PACX_LW_AUTOGEN("model",                  ResModel,                       SPLIT,      21, 3)
HL_IN_PACX_LW_AUTOGEN("skl.hkx",                ResSkeleton,                    ROOT,       23, 99)
HL_IN_PACX_LW_AUTOGEN("anm.hkx",                ResAnimSkeleton,                ROOT,       17, 99)
HL_IN_PACX_LW_AUTOGEN("uv-anim",                ResAnimTexSrt,                  MIXED,      18, 6)
HL_IN_PACX_LW_AUTOGEN("swif",                   ResSurfRideProject,             ROOT,       24, 99)
HL_IN_PACX_LW_AUTOGEN("effect",                 ResGrifEffect,                  V2_MERGED,  20, 99)

/* Average frequency */
HL_IN_PACX_LW_AUTOGEN("anm",                    ResCharAnimScript,              ROOT,       1,  99)
HL_IN_PACX_LW_AUTOGEN("terrain-model",          ResMirageTerrainModel,          SPLIT,      10, 2)
HL_IN_PACX_LW_AUTOGEN("phy.hkx",                ResHavokMesh,                   ROOT,       4,  99)
HL_IN_PACX_LW_AUTOGEN("mat-anim",               ResAnimMaterial,                MIXED,      16, 5)
HL_IN_PACX_LW_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   ROOT,       9,  99)
HL_IN_PACX_LW_AUTOGEN("lua",                    ResLuaData,                     ROOT,       7,  99)
HL_IN_PACX_LW_AUTOGEN("hhd",                    ResCustomData,                  ROOT,       3,  99)
HL_IN_PACX_LW_AUTOGEN("light",                  ResMirageLight,                 ROOT,       5,  99)
HL_IN_PACX_LW_AUTOGEN("path2.bin",              ResSplinePath2,                 V2_MERGED,  31, 99)
HL_IN_PACX_LW_AUTOGEN("lft",                    ResMirageLightField,            ROOT,       6,  99)

/* Low frequency */
HL_IN_PACX_LW_AUTOGEN("shadow-model",           ResShadowModel,                 V2_MERGED,  28, 99)
HL_IN_PACX_LW_AUTOGEN("gism",                   ResGismoConfig,                 ROOT,       2,  99)
HL_IN_PACX_LW_AUTOGEN("fxcol.bin",              ResFxColFile,                   V2_MERGED,  15, 99)
HL_IN_PACX_LW_AUTOGEN("xtb2",                   ResXTB2Data,                    V2_MERGED,  41, 99)
HL_IN_PACX_LW_AUTOGEN("model-instanceinfo",     ResModelInstanceInfo,           V2_MERGED,  22, 99)
HL_IN_PACX_LW_AUTOGEN("svcol.bin",              ResSvCol,                       V2_MERGED,  27, 99)
HL_IN_PACX_LW_AUTOGEN("vis-anim",               ResAnimVis,                     SPLIT,      19, 7)
HL_IN_PACX_LW_AUTOGEN("nextra.bin",             ResNameExtra,                   V2_MERGED,  11, 99)
HL_IN_PACX_LW_AUTOGEN("score",                  ResScoreTable,                  ROOT,       40, 99)
HL_IN_PACX_LW_AUTOGEN("voxel.bin",              ResVoxelContainer,              ROOT,       29, 99)
HL_IN_PACX_LW_AUTOGEN("pixelshader",            ResMiragePixelShader,           ROOT,       32, 99)
HL_IN_PACX_LW_AUTOGEN("wpu",                    ResMiragePixelShaderCode,       ROOT,       33, 99)
HL_IN_PACX_LW_AUTOGEN("fpo",                    ResMiragePixelShaderCode,       ROOT,       33, 99)
HL_IN_PACX_LW_AUTOGEN("shader-list",            ResMirageShaderList,            ROOT,       35, 99)
HL_IN_PACX_LW_AUTOGEN("vertexshader",           ResMirageVertexShader,          ROOT,       36, 99)
HL_IN_PACX_LW_AUTOGEN("wvu",                    ResMirageVertexShaderCode,      ROOT,       37, 99)
HL_IN_PACX_LW_AUTOGEN("psparam",                ResMiragePixelShaderParameter,  ROOT,       34, 99)
HL_IN_PACX_LW_AUTOGEN("vpo",                    ResMirageVertexShaderCode,      ROOT,       37, 99)
HL_IN_PACX_LW_AUTOGEN("vsparam",                ResMirageVertexShaderParameter, ROOT,       38, 99)
HL_IN_PACX_LW_AUTOGEN("path.bin",               ResSplinePath,                  ROOT,       30, 99)
HL_IN_PACX_LW_AUTOGEN("gsh",                    ResSM4ShaderContainer,          ROOT,       39, 99)
HL_IN_PACX_LW_AUTOGEN("ttf",                    ResTTFData,                     ROOT,       26, 99)
HL_IN_PACX_LW_AUTOGEN("bfttf",                  ResBFTTFData,                   ROOT,       12, 99)

/* Never used in LW (but present in the .exe) */
/* TODO: These are never used in LW but are in the exe. Is this info correct?*/
HL_IN_PACX_LW_AUTOGEN("pt-anim",                ResAnimTexPat,                  ROOT,       17, 99)
HL_IN_PACX_LW_AUTOGEN("cam-anim",               ResAnimCameraContainer,         SPLIT,      14, 99)
/* TODO: Does this work? There's also a "ResAnimLightContainer2" apparently. */
HL_IN_PACX_LW_AUTOGEN("lit-anim",               ResAnimLightContainer,          SPLIT,      15, 99)
HL_IN_PACX_LW_AUTOGEN("morph-anim",             ResAnimMorphTarget,             SPLIT,      16, 99)

/* ResRawData placed at the end; used if no other extensions matched. */
HL_IN_PACX_LW_AUTOGEN("",                       ResRawData,                     V2_MERGED,  14, 99)

/* ========================================= Rio Exts ========================================= */

/* Organized based on frequency information determined via a custom analyzation program. */

/* NOTE: We place ResPacDepend first so we can easily find/skip it as needed. */
HL_IN_PACX_RIO_AUTOGEN("pac.d",                 ResPacDepend,                   ROOT,       16, 99)

/* High frequency */
HL_IN_PACX_RIO_AUTOGEN("dds",                   ResTexture,                     SPLIT,      32, 4)
HL_IN_PACX_RIO_AUTOGEN("material",              ResMirageMaterial,              SPLIT,      11, 1)
HL_IN_PACX_RIO_AUTOGEN("model",                 ResModel,                       SPLIT,      28, 3)
HL_IN_PACX_RIO_AUTOGEN("anm.hkx",               ResAnimSkeleton,                ROOT,       23, 99)
HL_IN_PACX_RIO_AUTOGEN("skl.hkx",               ResSkeleton,                    ROOT,       30, 99)
HL_IN_PACX_RIO_AUTOGEN("swif",                  ResSurfRideProject,             ROOT,       31, 99)
HL_IN_PACX_RIO_AUTOGEN("uv-anim",               ResAnimTexSrt,                  SPLIT,      25, 7)
HL_IN_PACX_RIO_AUTOGEN("vis-anim",              ResAnimVis,                     SPLIT,      26, 8)

/* Average frequency */
HL_IN_PACX_RIO_AUTOGEN("pt-anim",               ResAnimTexPat,                  ROOT,       24, 99)
HL_IN_PACX_RIO_AUTOGEN("cam-anim",              ResAnimCameraContainer,         SPLIT,      21, 5)
HL_IN_PACX_RIO_AUTOGEN("asm",                   ResAnimator,                    V2_MERGED,  2,  99)
HL_IN_PACX_RIO_AUTOGEN("phy.hkx",               ResHavokMesh,                   ROOT,       6,  99)
HL_IN_PACX_RIO_AUTOGEN("cemt",                  ResCyanEffect,                  ROOT,       27, 99)
HL_IN_PACX_RIO_AUTOGEN("hhd",                   ResCustomData,                  ROOT,       5,  99)
HL_IN_PACX_RIO_AUTOGEN("light",                 ResMirageLight,                 ROOT,       9,  99)
HL_IN_PACX_RIO_AUTOGEN("lft",                   ResMirageLightField,            ROOT,       10, 99)
HL_IN_PACX_RIO_AUTOGEN("probe",                 ResProbe,                       V2_MERGED,  18, 99)
HL_IN_PACX_RIO_AUTOGEN("terrain-model",         ResMirageTerrainModel,          SPLIT,      13, 2)

/* Low frequency */
HL_IN_PACX_RIO_AUTOGEN("path2.bin",             ResSplinePath2,                 V2_MERGED,  34, 99)
HL_IN_PACX_RIO_AUTOGEN("affair",                ResAffair,                      ROOT,       1,  99)
HL_IN_PACX_RIO_AUTOGEN("mat-anim",              ResAnimMaterial,                SPLIT,      22, 6)
HL_IN_PACX_RIO_AUTOGEN("msbt",                  ResTextMSFile,                  ROOT,       35, 99)
HL_IN_PACX_RIO_AUTOGEN("msbp",                  ResTextMSProject,               ROOT,       36, 99)
HL_IN_PACX_RIO_AUTOGEN("effdb",                 ResParticleLocation,            V2_MERGED,  17, 99)
HL_IN_PACX_RIO_AUTOGEN("music-score",           ResMusicScore,                  V2_MERGED,  15, 99)
HL_IN_PACX_RIO_AUTOGEN("model-instanceinfo",    ResModelInstanceInfo,           V2_MERGED,  29, 99)
HL_IN_PACX_RIO_AUTOGEN("bfnt.bin",              ResBitmapFont,                  V2_MERGED,  3,  99)
HL_IN_PACX_RIO_AUTOGEN("pso",                   ResMiragePixelShader,           MIXED,      12, 10)
HL_IN_PACX_RIO_AUTOGEN("vso",                   ResMirageVertexShader,          MIXED,      14, 9)
HL_IN_PACX_RIO_AUTOGEN("tga",                   ResTGAImage,                    ROOT,       37, 99)
HL_IN_PACX_RIO_AUTOGEN("cam-offset",            ResCameraOffset,                V2_MERGED,  4,  99)
HL_IN_PACX_RIO_AUTOGEN("otf",                   ResOTFData,                     ROOT,       8,  99)
HL_IN_PACX_RIO_AUTOGEN("kern.bin",              ResKerningData,                 V2_MERGED,  7,  99)
HL_IN_PACX_RIO_AUTOGEN("stf.bin",               ResStaffrollData,               V2_MERGED,  20, 99)
HL_IN_PACX_RIO_AUTOGEN("ttf",                   ResTTFData,                     ROOT,       33, 99)

/* Never used in Rio (but present in the .rpx) */
/* TODO: These are never used in Rio but are in the rpx. Is this info correct?*/
/* TODO: Does this work? There's also a "ResAnimLightContainer2" apparently. */
HL_IN_PACX_RIO_AUTOGEN("lit-anim",              ResAnimLightContainer,          SPLIT,      38, 11)

/* ResRawData placed at the end; used if no other extensions matched. */
HL_IN_PACX_RIO_AUTOGEN("",                      ResRawData,                     V2_MERGED,  19, 99)

/* ======================================== Forces Exts ======================================= */

/* Organized based on frequency information determined via a custom analyzation program. */

/* High frequency */
HL_IN_PACX_WARS_AUTOGEN("dds",                  ResTexture,                     SPLIT)
HL_IN_PACX_WARS_AUTOGEN("model",                ResModel,                       SPLIT)
HL_IN_PACX_WARS_AUTOGEN("terrain-model",        ResMirageTerrainModel,          SPLIT)
HL_IN_PACX_WARS_AUTOGEN("material",             ResMirageMaterial,              SPLIT)
HL_IN_PACX_WARS_AUTOGEN("swif",                 ResSurfRideProject,             ROOT)
HL_IN_PACX_WARS_AUTOGEN("terrain-instanceinfo", ResMirageTerrainInstanceInfo,   ROOT)
HL_IN_PACX_WARS_AUTOGEN("uv-anim",              ResAnimTexSrt,                  SPLIT)
HL_IN_PACX_WARS_AUTOGEN("cemt",                 ResCyanEffect,                  ROOT)
HL_IN_PACX_WARS_AUTOGEN("rfl",                  ResReflection,                  ROOT)
HL_IN_PACX_WARS_AUTOGEN("skl.hkx",              ResSkeleton,                    ROOT)
HL_IN_PACX_WARS_AUTOGEN("anm.hkx",              ResAnimSkeleton,                ROOT)
HL_IN_PACX_WARS_AUTOGEN("mat-anim",             ResAnimMaterial,                SPLIT)
HL_IN_PACX_WARS_AUTOGEN("codetbl",              ResCodeTable,                   ROOT)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-text",           ResText,                        ROOT)
HL_IN_PACX_WARS_AUTOGEN("light",                ResMirageLight,                 ROOT)
HL_IN_PACX_WARS_AUTOGEN("asm",                  ResAnimator,                    V2_MERGED)
HL_IN_PACX_WARS_AUTOGEN("model-instanceinfo",   ResModelInstanceInfo,           V2_MERGED)

/* Average frequency */
HL_IN_PACX_WARS_AUTOGEN("cam-anim",             ResAnimCameraContainer,         SPLIT)
HL_IN_PACX_WARS_AUTOGEN("gedit",                ResObjectWorld,                 ROOT)
HL_IN_PACX_WARS_AUTOGEN("phy.hkx",              ResHavokMesh,                   ROOT)
HL_IN_PACX_WARS_AUTOGEN("vis-anim",             ResAnimVis,                     SPLIT)
HL_IN_PACX_WARS_AUTOGEN("grass.bin",            ResTerrainGrassInfo,            ROOT)
HL_IN_PACX_WARS_AUTOGEN("scene",                ResScene,                       ROOT)
HL_IN_PACX_WARS_AUTOGEN("effdb",                ResParticleLocation,            V2_MERGED)
HL_IN_PACX_WARS_AUTOGEN("shlf",                 ResSHLightField,                ROOT)
HL_IN_PACX_WARS_AUTOGEN("gism",                 ResGismoConfig,                 ROOT)
HL_IN_PACX_WARS_AUTOGEN("probe",                ResProbe,                       V2_MERGED)
HL_IN_PACX_WARS_AUTOGEN("svcol.bin",            ResSvCol,                       V2_MERGED)

/* Low frequency */
HL_IN_PACX_WARS_AUTOGEN("fxcol.bin",            ResFxColFile,                   V2_MERGED)
HL_IN_PACX_WARS_AUTOGEN("path",                 ResSplinePath,                  ROOT)
HL_IN_PACX_WARS_AUTOGEN("pt-anim",              ResAnimTexPat,                  SPLIT)
HL_IN_PACX_WARS_AUTOGEN("lit-anim",             ResAnimLightContainer,          SPLIT)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-proj",           ResTextProject,                 ROOT)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-meta",           ResTextMeta,                    ROOT)
HL_IN_PACX_WARS_AUTOGEN("scfnt",                ResScalableFontSet,             ROOT)
HL_IN_PACX_WARS_AUTOGEN("pso",                  ResMiragePixelShader,           SPLIT)
HL_IN_PACX_WARS_AUTOGEN("vso",                  ResMirageVertexShader,          SPLIT)
HL_IN_PACX_WARS_AUTOGEN("shader-list",          ResShaderList,                  ROOT)
HL_IN_PACX_WARS_AUTOGEN("vib",                  ResVibration,                   ROOT)
HL_IN_PACX_WARS_AUTOGEN("bfnt",                 ResBitmapFont,                  V2_MERGED)

/* Never used in Forces (but present in the .exe) */
/* TODO: These are never used in Forces but are in the exe. Is this info correct?*/
HL_IN_PACX_WARS_AUTOGEN("hhd",                  ResCustomData,                  ROOT)
HL_IN_PACX_WARS_AUTOGEN("lua",                  ResLuaData,                     ROOT)
HL_IN_PACX_WARS_AUTOGEN("lft",                  ResMirageLightField,            ROOT)
HL_IN_PACX_WARS_AUTOGEN("xtb2",                 ResXTB2Data,                    V2_MERGED)
HL_IN_PACX_WARS_AUTOGEN("nextra.bin",           ResNameExtra,                   V2_MERGED)

/* ResRawData placed at the end; used if no other extensions matched. */
HL_IN_PACX_WARS_AUTOGEN("",                     ResRawData,                     V2_MERGED)

/* Undefine all auto-generator macros. */
#undef HL_IN_PACX_TYPE_AUTOGEN
#undef HL_IN_PACX_LW_AUTOGEN
#undef HL_IN_PACX_RIO_AUTOGEN
#undef HL_IN_PACX_WARS_AUTOGEN
