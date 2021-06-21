// Define empty macros for any auto-generators that aren't used.
#ifndef HL_IN_PACX_TYPE_AUTOGEN
#define HL_IN_PACX_TYPE_AUTOGEN(type)
#endif

#ifndef HL_IN_PACX_LW_AUTOGEN
#define HL_IN_PACX_LW_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_RIO_AUTOGEN
#define HL_IN_PACX_RIO_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_FORCES_AUTOGEN
#define HL_IN_PACX_FORCES_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_TOKYO1_AUTOGEN
#define HL_IN_PACX_TOKYO1_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_TOKYO2_AUTOGEN
#define HL_IN_PACX_TOKYO2_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_SAKURA_AUTOGEN
#define HL_IN_PACX_SAKURA_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_PPT2_AUTOGEN
#define HL_IN_PACX_PPT2_AUTOGEN(ext, type, kind)
#endif

/* ======================================== PACx Types ======================================== */

// ResRawData is placed at index 0 for fallback purposes.
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
HL_IN_PACX_TYPE_AUTOGEN(ResDivMap)
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
HL_IN_PACX_TYPE_AUTOGEN(ResPacDepend)       // TODO: These are in the Forces exe? Can these be used?
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
HL_IN_PACX_TYPE_AUTOGEN(ResOrcaData)        // TODO: Unused but in Forces .exe?
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
HL_IN_PACX_TYPE_AUTOGEN(ResVibrationHD)     // TODO: Present in Forces .exe but unused? Not used until Sonic Tokyo 2020?

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

// Organized based on frequency information determined via a custom analyzation program.

// NOTE: We place ResPacDepend first so we can easily find/skip it as needed.
HL_IN_PACX_LW_AUTOGEN("pac.d",                  ResPacDepend,                   root,       13, 99)

/* High frequency */
HL_IN_PACX_LW_AUTOGEN("dds",                    ResTexture,                     mixed,      25, 4)
HL_IN_PACX_LW_AUTOGEN("material",               ResMirageMaterial,              split,      8,  1)
HL_IN_PACX_LW_AUTOGEN("model",                  ResModel,                       split,      21, 3)
HL_IN_PACX_LW_AUTOGEN("skl.hkx",                ResSkeleton,                    root,       23, 99)
HL_IN_PACX_LW_AUTOGEN("anm.hkx",                ResAnimSkeleton,                root,       17, 99)
HL_IN_PACX_LW_AUTOGEN("uv-anim",                ResAnimTexSrt,                  mixed,      18, 6)
HL_IN_PACX_LW_AUTOGEN("swif",                   ResSurfRideProject,             root,       24, 99)
HL_IN_PACX_LW_AUTOGEN("effect",                 ResGrifEffect,                  v2_merged,  20, 99)

/* Average frequency */
HL_IN_PACX_LW_AUTOGEN("anm",                    ResCharAnimScript,              root,       1,  99)
HL_IN_PACX_LW_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split,      10, 2)
HL_IN_PACX_LW_AUTOGEN("phy.hkx",                ResHavokMesh,                   root,       4,  99)
HL_IN_PACX_LW_AUTOGEN("mat-anim",               ResAnimMaterial,                mixed,      16, 5)
HL_IN_PACX_LW_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   root,       9,  99)
HL_IN_PACX_LW_AUTOGEN("lua",                    ResLuaData,                     root,       7,  99)
HL_IN_PACX_LW_AUTOGEN("hhd",                    ResCustomData,                  root,       3,  99)
HL_IN_PACX_LW_AUTOGEN("light",                  ResMirageLight,                 root,       5,  99)
HL_IN_PACX_LW_AUTOGEN("path2.bin",              ResSplinePath2,                 v2_merged,  31, 99)
HL_IN_PACX_LW_AUTOGEN("lft",                    ResMirageLightField,            root,       6,  99)

/* Low frequency */
HL_IN_PACX_LW_AUTOGEN("shadow-model",           ResShadowModel,                 v2_merged,  28, 99)
HL_IN_PACX_LW_AUTOGEN("gism",                   ResGismoConfig,                 root,       2,  99)
HL_IN_PACX_LW_AUTOGEN("fxcol.bin",              ResFxColFile,                   v2_merged,  15, 99)
HL_IN_PACX_LW_AUTOGEN("xtb2",                   ResXTB2Data,                    v2_merged,  41, 99)
HL_IN_PACX_LW_AUTOGEN("model-instanceinfo",     ResModelInstanceInfo,           v2_merged,  22, 99)
HL_IN_PACX_LW_AUTOGEN("svcol.bin",              ResSvCol,                       v2_merged,  27, 99)
HL_IN_PACX_LW_AUTOGEN("vis-anim",               ResAnimVis,                     split,      19, 7)
HL_IN_PACX_LW_AUTOGEN("nextra.bin",             ResNameExtra,                   v2_merged,  11, 99)
HL_IN_PACX_LW_AUTOGEN("score",                  ResScoreTable,                  root,       40, 99)
HL_IN_PACX_LW_AUTOGEN("voxel.bin",              ResVoxelContainer,              root,       29, 99)
HL_IN_PACX_LW_AUTOGEN("pixelshader",            ResMiragePixelShader,           root,       32, 99)
HL_IN_PACX_LW_AUTOGEN("wpu",                    ResMiragePixelShaderCode,       root,       33, 99)
HL_IN_PACX_LW_AUTOGEN("fpo",                    ResMiragePixelShaderCode,       root,       33, 99)
HL_IN_PACX_LW_AUTOGEN("shader-list",            ResMirageShaderList,            root,       35, 99)
HL_IN_PACX_LW_AUTOGEN("vertexshader",           ResMirageVertexShader,          root,       36, 99)
HL_IN_PACX_LW_AUTOGEN("wvu",                    ResMirageVertexShaderCode,      root,       37, 99)
HL_IN_PACX_LW_AUTOGEN("psparam",                ResMiragePixelShaderParameter,  root,       34, 99)
HL_IN_PACX_LW_AUTOGEN("vpo",                    ResMirageVertexShaderCode,      root,       37, 99)
HL_IN_PACX_LW_AUTOGEN("vsparam",                ResMirageVertexShaderParameter, root,       38, 99)
HL_IN_PACX_LW_AUTOGEN("path.bin",               ResSplinePath,                  root,       30, 99)
HL_IN_PACX_LW_AUTOGEN("gsh",                    ResSM4ShaderContainer,          root,       39, 99)
HL_IN_PACX_LW_AUTOGEN("ttf",                    ResTTFData,                     root,       26, 99)
HL_IN_PACX_LW_AUTOGEN("bfttf",                  ResBFTTFData,                   root,       12, 99)

/* Never used in LW (but present in the .exe) */
// TODO: These are never used in LW but are in the exe. Is this info correct?
HL_IN_PACX_LW_AUTOGEN("pt-anim",                ResAnimTexPat,                  root,       14, 99)
HL_IN_PACX_LW_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split,      14, 99)
// TODO: Does this work? There's also a "ResAnimLightContainer2" apparently.
HL_IN_PACX_LW_AUTOGEN("lit-anim",               ResAnimLightContainer,          split,      14, 99)
HL_IN_PACX_LW_AUTOGEN("morph-anim",             ResAnimMorphTarget,             split,      14, 99)
HL_IN_PACX_LW_AUTOGEN("map.bin",                ResDivMap,                      v2_merged,  14, 99)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_LW_AUTOGEN("",                       ResRawData,                     v2_merged,  14, 99)

/* ========================================= Rio Exts ========================================= */

// Organized based on frequency information determined via a custom analyzation program.

// NOTE: We place ResPacDepend first so we can easily find/skip it as needed.
HL_IN_PACX_RIO_AUTOGEN("pac.d",                 ResPacDepend,                   root,       16, 99)

/* High frequency */
HL_IN_PACX_RIO_AUTOGEN("dds",                   ResTexture,                     split,      32, 4)
HL_IN_PACX_RIO_AUTOGEN("material",              ResMirageMaterial,              split,      11, 1)
HL_IN_PACX_RIO_AUTOGEN("model",                 ResModel,                       split,      28, 3)
HL_IN_PACX_RIO_AUTOGEN("anm.hkx",               ResAnimSkeleton,                root,       23, 99)
HL_IN_PACX_RIO_AUTOGEN("skl.hkx",               ResSkeleton,                    root,       30, 99)
HL_IN_PACX_RIO_AUTOGEN("swif",                  ResSurfRideProject,             root,       31, 99)
HL_IN_PACX_RIO_AUTOGEN("uv-anim",               ResAnimTexSrt,                  split,      25, 7)
HL_IN_PACX_RIO_AUTOGEN("vis-anim",              ResAnimVis,                     split,      26, 8)

/* Average frequency */
HL_IN_PACX_RIO_AUTOGEN("pt-anim",               ResAnimTexPat,                  root,       24, 99)
HL_IN_PACX_RIO_AUTOGEN("cam-anim",              ResAnimCameraContainer,         split,      21, 5)
HL_IN_PACX_RIO_AUTOGEN("asm",                   ResAnimator,                    v2_merged,  2,  99)
HL_IN_PACX_RIO_AUTOGEN("phy.hkx",               ResHavokMesh,                   root,       6,  99)
HL_IN_PACX_RIO_AUTOGEN("cemt",                  ResCyanEffect,                  root,       27, 99)
HL_IN_PACX_RIO_AUTOGEN("hhd",                   ResCustomData,                  root,       5,  99)
HL_IN_PACX_RIO_AUTOGEN("light",                 ResMirageLight,                 root,       9,  99)
HL_IN_PACX_RIO_AUTOGEN("lft",                   ResMirageLightField,            root,       10, 99)
HL_IN_PACX_RIO_AUTOGEN("probe",                 ResProbe,                       v2_merged,  18, 99)
HL_IN_PACX_RIO_AUTOGEN("terrain-model",         ResMirageTerrainModel,          split,      13, 2)

/* Low frequency */
HL_IN_PACX_RIO_AUTOGEN("path2.bin",             ResSplinePath2,                 v2_merged,  34, 99)
HL_IN_PACX_RIO_AUTOGEN("affair",                ResAffair,                      root,       1,  99)
HL_IN_PACX_RIO_AUTOGEN("mat-anim",              ResAnimMaterial,                split,      22, 6)
HL_IN_PACX_RIO_AUTOGEN("msbt",                  ResTextMSFile,                  root,       35, 99)
HL_IN_PACX_RIO_AUTOGEN("msbp",                  ResTextMSProject,               root,       36, 99)
HL_IN_PACX_RIO_AUTOGEN("effdb",                 ResParticleLocation,            v2_merged,  17, 99)
HL_IN_PACX_RIO_AUTOGEN("music-score",           ResMusicScore,                  v2_merged,  15, 99)
HL_IN_PACX_RIO_AUTOGEN("model-instanceinfo",    ResModelInstanceInfo,           v2_merged,  29, 99)
HL_IN_PACX_RIO_AUTOGEN("bfnt.bin",              ResBitmapFont,                  v2_merged,  3,  99)
HL_IN_PACX_RIO_AUTOGEN("pso",                   ResMiragePixelShader,           mixed,      12, 10)
HL_IN_PACX_RIO_AUTOGEN("vso",                   ResMirageVertexShader,          mixed,      14, 9)
HL_IN_PACX_RIO_AUTOGEN("tga",                   ResTGAImage,                    root,       37, 99)
HL_IN_PACX_RIO_AUTOGEN("cam-offset",            ResCameraOffset,                v2_merged,  4,  99)
HL_IN_PACX_RIO_AUTOGEN("otf",                   ResOTFData,                     root,       8,  99)
HL_IN_PACX_RIO_AUTOGEN("kern.bin",              ResKerningData,                 v2_merged,  7,  99)
HL_IN_PACX_RIO_AUTOGEN("stf.bin",               ResStaffrollData,               v2_merged,  20, 99)
HL_IN_PACX_RIO_AUTOGEN("ttf",                   ResTTFData,                     root,       33, 99)

/* Never used in Rio (but present in the .rpx) */
// TODO: These are never used in Rio but are in the rpx. Is this info correct?
// TODO: Does this work? There's also a "ResAnimLightContainer2" apparently.
HL_IN_PACX_RIO_AUTOGEN("lit-anim",              ResAnimLightContainer,          split,      19, 99)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_RIO_AUTOGEN("",                      ResRawData,                     v2_merged,  19, 99)

/* ======================================== Forces Exts ======================================= */

// Organized based on frequency information determined via a custom analyzation program.

/* High frequency */
HL_IN_PACX_FORCES_AUTOGEN("dds",                    ResTexture,                     split)
HL_IN_PACX_FORCES_AUTOGEN("model",                  ResModel,                       split)
HL_IN_PACX_FORCES_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split)
HL_IN_PACX_FORCES_AUTOGEN("material",               ResMirageMaterial,              split)
HL_IN_PACX_FORCES_AUTOGEN("swif",                   ResSurfRideProject,             root)
HL_IN_PACX_FORCES_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_FORCES_AUTOGEN("uv-anim",                ResAnimTexSrt,                  split)
HL_IN_PACX_FORCES_AUTOGEN("cemt",                   ResCyanEffect,                  root)
HL_IN_PACX_FORCES_AUTOGEN("rfl",                    ResReflection,                  root)
HL_IN_PACX_FORCES_AUTOGEN("skl.hkx",                ResSkeleton,                    root)
HL_IN_PACX_FORCES_AUTOGEN("anm.hkx",                ResAnimSkeleton,                root)
HL_IN_PACX_FORCES_AUTOGEN("mat-anim",               ResAnimMaterial,                split)
HL_IN_PACX_FORCES_AUTOGEN("codetbl",                ResCodeTable,                   root)
HL_IN_PACX_FORCES_AUTOGEN("cnvrs-text",             ResText,                        root)
HL_IN_PACX_FORCES_AUTOGEN("light",                  ResMirageLight,                 root)
HL_IN_PACX_FORCES_AUTOGEN("asm",                    ResAnimator,                    v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("model-instanceinfo",     ResModelInstanceInfo,           v2_merged)

/* Average frequency */
HL_IN_PACX_FORCES_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split)
HL_IN_PACX_FORCES_AUTOGEN("gedit",                  ResObjectWorld,                 root)
HL_IN_PACX_FORCES_AUTOGEN("phy.hkx",                ResHavokMesh,                   root)
HL_IN_PACX_FORCES_AUTOGEN("vis-anim",               ResAnimVis,                     split)
HL_IN_PACX_FORCES_AUTOGEN("grass.bin",              ResTerrainGrassInfo,            root)
HL_IN_PACX_FORCES_AUTOGEN("scene",                  ResScene,                       root)
HL_IN_PACX_FORCES_AUTOGEN("effdb",                  ResParticleLocation,            v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("shlf",                   ResSHLightField,                root)
HL_IN_PACX_FORCES_AUTOGEN("gism",                   ResGismoConfig,                 root)
HL_IN_PACX_FORCES_AUTOGEN("probe",                  ResProbe,                       v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("svcol.bin",              ResSvCol,                       v2_merged)

/* Low frequency */
HL_IN_PACX_FORCES_AUTOGEN("fxcol.bin",              ResFxColFile,                   v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("path",                   ResSplinePath,                  v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("pt-anim",                ResAnimTexPat,                  split)
HL_IN_PACX_FORCES_AUTOGEN("lit-anim",               ResAnimLightContainer,          split)
HL_IN_PACX_FORCES_AUTOGEN("cnvrs-proj",             ResTextProject,                 root)
HL_IN_PACX_FORCES_AUTOGEN("cnvrs-meta",             ResTextMeta,                    root)
HL_IN_PACX_FORCES_AUTOGEN("scfnt",                  ResScalableFontSet,             root)
HL_IN_PACX_FORCES_AUTOGEN("pso",                    ResMiragePixelShader,           split)
HL_IN_PACX_FORCES_AUTOGEN("vso",                    ResMirageVertexShader,          split)
HL_IN_PACX_FORCES_AUTOGEN("shader-list",            ResShaderList,                  root)
HL_IN_PACX_FORCES_AUTOGEN("vib",                    ResVibration,                   root)
HL_IN_PACX_FORCES_AUTOGEN("bfnt",                   ResBitmapFont,                  v2_merged)

/* Never used in Forces (but present in the .exe) */
// TODO: These are never used in Forces but are in the exe. Is this info correct?
HL_IN_PACX_FORCES_AUTOGEN("hhd",                    ResCustomData,                  root)
HL_IN_PACX_FORCES_AUTOGEN("lua",                    ResLuaData,                     root)
HL_IN_PACX_FORCES_AUTOGEN("lft",                    ResMirageLightField,            root)
HL_IN_PACX_FORCES_AUTOGEN("xtb2",                   ResXTB2Data,                    v2_merged)
HL_IN_PACX_FORCES_AUTOGEN("nextra.bin",             ResNameExtra,                   v2_merged)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_FORCES_AUTOGEN("",                       ResRawData,                     v2_merged)

/* =================================== Tokyo 2020 Wars Exts =================================== */

// Organized based on frequency information determined via a custom analyzation program.

/* High frequency */
HL_IN_PACX_TOKYO1_AUTOGEN("dds",                    ResTexture,                     split)
HL_IN_PACX_TOKYO1_AUTOGEN("material",               ResMirageMaterial,              split)
HL_IN_PACX_TOKYO1_AUTOGEN("model",                  ResModel,                       mixed)
HL_IN_PACX_TOKYO1_AUTOGEN("skl.pxd",                ResSkeletonPxd,                 root)
HL_IN_PACX_TOKYO1_AUTOGEN("vli",                    ResVertexLinkInfo,              root)
HL_IN_PACX_TOKYO1_AUTOGEN("rfl",                    ResReflection,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("swif",                   ResSurfRideProject,             root)
HL_IN_PACX_TOKYO1_AUTOGEN("cnvrs-text",             ResText,                        root)

/* Average frequency */
HL_IN_PACX_TOKYO1_AUTOGEN("uv-anim",                ResAnimTexSrt,                  split)
HL_IN_PACX_TOKYO1_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split)
HL_IN_PACX_TOKYO1_AUTOGEN("anm.pxd",                ResAnimationPxd,                root)
HL_IN_PACX_TOKYO1_AUTOGEN("lua",                    ResLuaData,                     root)
HL_IN_PACX_TOKYO1_AUTOGEN("asm",                    ResAnimator,                    root)
HL_IN_PACX_TOKYO1_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split)
HL_IN_PACX_TOKYO1_AUTOGEN("light",                  ResMirageLight,                 root)
HL_IN_PACX_TOKYO1_AUTOGEN("lf",                     ResSHLightField,                root)
HL_IN_PACX_TOKYO1_AUTOGEN("cnvrs-meta",             ResTextMeta,                    root)
HL_IN_PACX_TOKYO1_AUTOGEN("cnvrs-proj",             ResTextProject,                 root)
HL_IN_PACX_TOKYO1_AUTOGEN("cemt",                   ResCyanEffect,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("mat-anim",               ResAnimMaterial,                split)

/* Low frequency */
HL_IN_PACX_TOKYO1_AUTOGEN("scfnt",                  ResScalableFontSet,             root)
HL_IN_PACX_TOKYO1_AUTOGEN("pso",                    ResMiragePixelShader,           split)
HL_IN_PACX_TOKYO1_AUTOGEN("scene",                  ResScene,                       root)
HL_IN_PACX_TOKYO1_AUTOGEN("path.bin",               ResSplinePath,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("probe",                  ResProbe,                       root)
HL_IN_PACX_TOKYO1_AUTOGEN("aid",                    ResAnimatedInstance,            root)
HL_IN_PACX_TOKYO1_AUTOGEN("gedit",                  ResObjectWorld,                 root)
HL_IN_PACX_TOKYO1_AUTOGEN("btmesh",                 ResBulletMesh,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("affair",                 ResAffair,                      root)
HL_IN_PACX_TOKYO1_AUTOGEN("vso",                    ResMirageVertexShader,          split)
HL_IN_PACX_TOKYO1_AUTOGEN("vat",                    ResVertexAnimationTexture,      root)
HL_IN_PACX_TOKYO1_AUTOGEN("pt-anim",                ResAnimTexPat,                  split)
HL_IN_PACX_TOKYO1_AUTOGEN("cso",                    ResMirageComputeShader,         split)
HL_IN_PACX_TOKYO1_AUTOGEN("shader-list",            ResShaderList,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("anm.hkx",                ResAnimSkeleton,                root)
HL_IN_PACX_TOKYO1_AUTOGEN("bfnt",                   ResBitmapFont,                  root)
HL_IN_PACX_TOKYO1_AUTOGEN("effdb",                  ResParticleLocation,            root)
HL_IN_PACX_TOKYO1_AUTOGEN("vib",                    ResVibration,                   root)
HL_IN_PACX_TOKYO1_AUTOGEN("skl.hkx",                ResSkeleton,                    root)

/* ResRawData placed at the end; used if no other extensions matched. */
HL_IN_PACX_TOKYO1_AUTOGEN("",                       ResRawData,                     v2_merged)

/* ================================= M&S Tokyo 2020 Wars Exts ================================= */

// Organized based on frequency information determined via a custom analyzation program.

/* High frequency */
HL_IN_PACX_TOKYO2_AUTOGEN("dds",                    ResTexture,                     split)
HL_IN_PACX_TOKYO2_AUTOGEN("material",               ResMirageMaterial,              split)
HL_IN_PACX_TOKYO2_AUTOGEN("model",                  ResModel,                       mixed)
HL_IN_PACX_TOKYO2_AUTOGEN("swif",                   ResSurfRideProject,             root)
HL_IN_PACX_TOKYO2_AUTOGEN("anm.pxd",                ResAnimationPxd,                root)
HL_IN_PACX_TOKYO2_AUTOGEN("asm",                    ResAnimator,                    v2_merged)
HL_IN_PACX_TOKYO2_AUTOGEN("effdb",                  ResParticleLocation,            root)
HL_IN_PACX_TOKYO2_AUTOGEN("uv-anim",                ResAnimTexSrt,                  split)
HL_IN_PACX_TOKYO2_AUTOGEN("vis-anim",               ResAnimVis,                     split)
HL_IN_PACX_TOKYO2_AUTOGEN("skl.pxd",                ResSkeletonPxd,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("lua",                    ResLuaData,                     root)
HL_IN_PACX_TOKYO2_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split)
HL_IN_PACX_TOKYO2_AUTOGEN("cnvrs-text",             ResText,                        root)
HL_IN_PACX_TOKYO2_AUTOGEN("rfl",                    ResReflection,                  root)
HL_IN_PACX_TOKYO2_AUTOGEN("gedit",                  ResObjectWorld,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split)

/* Average frequency */
HL_IN_PACX_TOKYO2_AUTOGEN("aid",                    ResAnimatedInstance,            root)
HL_IN_PACX_TOKYO2_AUTOGEN("pt-anim",                ResAnimTexPat,                  split)
HL_IN_PACX_TOKYO2_AUTOGEN("light",                  ResMirageLight,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("cemt",                   ResCyanEffect,                  root)
HL_IN_PACX_TOKYO2_AUTOGEN("lf",                     ResSHLightField,                root)
HL_IN_PACX_TOKYO2_AUTOGEN("probe",                  ResProbe,                       v2_merged)
HL_IN_PACX_TOKYO2_AUTOGEN("btmesh",                 ResBulletMesh,                  root)
HL_IN_PACX_TOKYO2_AUTOGEN("anim2d",                 ResAnim2D,                      root)
HL_IN_PACX_TOKYO2_AUTOGEN("fpd",                    ResFlashLight,                  root)
HL_IN_PACX_TOKYO2_AUTOGEN("mat-anim",               ResAnimMaterial,                split)
HL_IN_PACX_TOKYO2_AUTOGEN("path.bin",               ResSplinePath,                  v2_merged)
HL_IN_PACX_TOKYO2_AUTOGEN("scfnt",                  ResScalableFontSet,             root)

/* Low frequency */
HL_IN_PACX_TOKYO2_AUTOGEN("cnvrs-meta",             ResTextMeta,                    root)
HL_IN_PACX_TOKYO2_AUTOGEN("cnvrs-proj",             ResTextProject,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("pso",                    ResMiragePixelShader,           split)
HL_IN_PACX_TOKYO2_AUTOGEN("vso",                    ResMirageVertexShader,          split)
HL_IN_PACX_TOKYO2_AUTOGEN("okern",                  ResOpticalKerning,              root)
HL_IN_PACX_TOKYO2_AUTOGEN("btsb",                   ResSoftBody,                    root)
HL_IN_PACX_TOKYO2_AUTOGEN("scene",                  ResScene,                       root)
HL_IN_PACX_TOKYO2_AUTOGEN("svcol.bin",              ResSvCol,                       v2_merged)
HL_IN_PACX_TOKYO2_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_TOKYO2_AUTOGEN("cso",                    ResMirageComputeShader,         split)
HL_IN_PACX_TOKYO2_AUTOGEN("nmt",                    ResNavMeshTile,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("shader-list",            ResShaderList,                  root)
HL_IN_PACX_TOKYO2_AUTOGEN("vib",                    ResVibration,                   root)
HL_IN_PACX_TOKYO2_AUTOGEN("bnvib",                  ResVibrationHD,                 root)
HL_IN_PACX_TOKYO2_AUTOGEN("bfnt",                   ResBitmapFont,                  v2_merged)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_TOKYO2_AUTOGEN("",                       ResRawData,                     v2_merged)

/* ===================================== Sakura Wars Exts ===================================== */

// Organized based on frequency information determined via a custom analyzation program.

/* High frequency */
HL_IN_PACX_SAKURA_AUTOGEN("rfl",                    ResReflection,                  root)
HL_IN_PACX_SAKURA_AUTOGEN("dds",                    ResTexture,                     split)
HL_IN_PACX_SAKURA_AUTOGEN("material",               ResMirageMaterial,              split)
HL_IN_PACX_SAKURA_AUTOGEN("model",                  ResModel,                       split)
HL_IN_PACX_SAKURA_AUTOGEN("anm.pxd",                ResAnimationPxd,                root)
HL_IN_PACX_SAKURA_AUTOGEN("asm",                    ResAnimator,                    v2_merged)
HL_IN_PACX_SAKURA_AUTOGEN("dv.bin",                 ResDvRawData,                   root)
HL_IN_PACX_SAKURA_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split)
HL_IN_PACX_SAKURA_AUTOGEN("skl.pxd",                ResSkeletonPxd,                 root)
HL_IN_PACX_SAKURA_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split)
HL_IN_PACX_SAKURA_AUTOGEN("swif",                   ResSurfRideProject,             root)
HL_IN_PACX_SAKURA_AUTOGEN("mat-anim",               ResAnimMaterial,                split)
HL_IN_PACX_SAKURA_AUTOGEN("light",                  ResMirageLight,                 root)

/* Average frequency */
HL_IN_PACX_SAKURA_AUTOGEN("uv-anim",                ResAnimTexSrt,                  split)
HL_IN_PACX_SAKURA_AUTOGEN("btmesh",                 ResBulletMesh,                  root)
HL_IN_PACX_SAKURA_AUTOGEN("lf",                     ResSHLightField,                root)
HL_IN_PACX_SAKURA_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_SAKURA_AUTOGEN("gedit",                  ResObjectWorld,                 root)
HL_IN_PACX_SAKURA_AUTOGEN("probe",                  ResProbe,                       v2_merged)
HL_IN_PACX_SAKURA_AUTOGEN("msb",                    ResScript,                      root)
HL_IN_PACX_SAKURA_AUTOGEN("cemt",                   ResCyanEffect,                  root)
HL_IN_PACX_SAKURA_AUTOGEN("cnvrs-text",             ResText,                        root)
HL_IN_PACX_SAKURA_AUTOGEN("lit-anim",               ResAnimLightContainer,          split)
HL_IN_PACX_SAKURA_AUTOGEN("lua",                    ResLuaData,                     root)
HL_IN_PACX_SAKURA_AUTOGEN("effdb",                  ResParticleLocation,            root)
HL_IN_PACX_SAKURA_AUTOGEN("model-instanceinfo",     ResModelInstanceInfo,           root)

/* Low frequency */
HL_IN_PACX_SAKURA_AUTOGEN("fxcol.bin",              ResFxColFile,                   root)
HL_IN_PACX_SAKURA_AUTOGEN("nmt",                    ResNavMeshTile,                 root)
HL_IN_PACX_SAKURA_AUTOGEN("vis-anim",               ResAnimVis,                     split)
HL_IN_PACX_SAKURA_AUTOGEN("mx_path",                ResMxGraph,                     root)
HL_IN_PACX_SAKURA_AUTOGEN("svcol.bin",              ResSvCol,                       v2_merged)
HL_IN_PACX_SAKURA_AUTOGEN("pso",                    ResMiragePixelShader,           split)
HL_IN_PACX_SAKURA_AUTOGEN("nmc",                    ResNavMeshConfig,               root)
HL_IN_PACX_SAKURA_AUTOGEN("scfnt",                  ResScalableFontSet,             root)
HL_IN_PACX_SAKURA_AUTOGEN("pt-anim",                ResAnimTexPat,                  split)
HL_IN_PACX_SAKURA_AUTOGEN("bfnt",                   ResBitmapFont,                  v2_merged)
HL_IN_PACX_SAKURA_AUTOGEN("cso",                    ResMirageComputeShader,         split)
HL_IN_PACX_SAKURA_AUTOGEN("vso",                    ResMirageVertexShader,          split)
HL_IN_PACX_SAKURA_AUTOGEN("okern",                  ResOpticalKerning,              root)
HL_IN_PACX_SAKURA_AUTOGEN("png",                    ResRawData,                     root)
HL_IN_PACX_SAKURA_AUTOGEN("vib",                    ResVibration,                   root)
HL_IN_PACX_SAKURA_AUTOGEN("cnvrs-proj",             ResTextProject,                 root)
HL_IN_PACX_SAKURA_AUTOGEN("cnvrs-meta",             ResTextMeta,                    root)
HL_IN_PACX_SAKURA_AUTOGEN("shader-list",            ResShaderList,                  root)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_SAKURA_AUTOGEN("",                       ResRawData,                     v2_merged)

/* ========================================= PPT2 Exts ======================================== */

// Organized based on frequency information determined via a custom analyzation program.

/* High frequency */
HL_IN_PACX_PPT2_AUTOGEN("dds",                      ResTexture,                     split)
HL_IN_PACX_PPT2_AUTOGEN("swif",                     ResSurfRideProject,             root)

/* Average frequency */
HL_IN_PACX_PPT2_AUTOGEN("scfnt",                    ResScalableFontSet,             root)
HL_IN_PACX_PPT2_AUTOGEN("pss",                      ResPss,                         root)
HL_IN_PACX_PPT2_AUTOGEN("rfl",                      ResReflection,                  root)
HL_IN_PACX_PPT2_AUTOGEN("cnvrs-proj",               ResTextProject,                 root)
HL_IN_PACX_PPT2_AUTOGEN("cnvrs-text",               ResText,                        root)

/* Low frequency */
HL_IN_PACX_PPT2_AUTOGEN("cnvrs-meta",               ResTextMeta,                    root)
HL_IN_PACX_PPT2_AUTOGEN("bfnt",                     ResBitmapFont,                  v2_merged)
HL_IN_PACX_PPT2_AUTOGEN("bnvib",                    ResVibrationHD,                 root)
HL_IN_PACX_PPT2_AUTOGEN("vib",                      ResVibration,                   root)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_PPT2_AUTOGEN("",                         ResRawData,                     v2_merged)

// Undefine all auto-generator macros.
#undef HL_IN_PACX_TYPE_AUTOGEN
#undef HL_IN_PACX_LW_AUTOGEN
#undef HL_IN_PACX_RIO_AUTOGEN
#undef HL_IN_PACX_FORCES_AUTOGEN
#undef HL_IN_PACX_TOKYO1_AUTOGEN
#undef HL_IN_PACX_TOKYO2_AUTOGEN
#undef HL_IN_PACX_SAKURA_AUTOGEN
#undef HL_IN_PACX_PPT2_AUTOGEN
