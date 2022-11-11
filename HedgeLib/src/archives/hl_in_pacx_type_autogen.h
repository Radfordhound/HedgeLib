// Define empty macros for any auto-generators that aren't used.
#ifndef HL_IN_PACX_TYPE_AUTOGEN
#define HL_IN_PACX_TYPE_AUTOGEN(type)
#endif

#ifndef HL_IN_PACX_SONIC2013_AUTOGEN
#define HL_IN_PACX_SONIC2013_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_UNISON_AUTOGEN
#define HL_IN_PACX_UNISON_AUTOGEN(ext, type, kind, rootSortWeight, splitSortWeight)
#endif

#ifndef HL_IN_PACX_WARS_AUTOGEN
#define HL_IN_PACX_WARS_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_MUSASHI_AUTOGEN
#define HL_IN_PACX_MUSASHI_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_RINGS_AUTOGEN
#define HL_IN_PACX_RINGS_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_ARUKAS_AUTOGEN
#define HL_IN_PACX_ARUKAS_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_TENPEX_AUTOGEN
#define HL_IN_PACX_TENPEX_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_HITE_AUTOGEN
#define HL_IN_PACX_HITE_AUTOGEN(ext, type, kind)
#endif

#ifndef HL_IN_PACX_RANGERS_AUTOGEN
#define HL_IN_PACX_RANGERS_AUTOGEN(ext, type, kind)
#endif

/* ======================================== PACx Types ======================================== */

// ResRawData is placed at index 0 for fallback purposes.
HL_IN_PACX_TYPE_AUTOGEN(ResRawData)

/* PACx V1+ */
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

/* PACx V3+ */
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

/* PACx V402+ */
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

/* PACx V403+ */
HL_IN_PACX_TYPE_AUTOGEN(ResAIStateMachine)
HL_IN_PACX_TYPE_AUTOGEN(ResDensitySetting)
HL_IN_PACX_TYPE_AUTOGEN(ResDensityPointCloud)
HL_IN_PACX_TYPE_AUTOGEN(ResDvScene)
HL_IN_PACX_TYPE_AUTOGEN(ResGismoConfigDesign)
HL_IN_PACX_TYPE_AUTOGEN(ResGismoConfigPlan)
HL_IN_PACX_TYPE_AUTOGEN(ResHeightField)
HL_IN_PACX_TYPE_AUTOGEN(ResLevel)
HL_IN_PACX_TYPE_AUTOGEN(ResOcclusionCapsule)
HL_IN_PACX_TYPE_AUTOGEN(ResPhysicalSkeleton)
HL_IN_PACX_TYPE_AUTOGEN(ResPointcloud)
HL_IN_PACX_TYPE_AUTOGEN(ResPointcloudCollision)
HL_IN_PACX_TYPE_AUTOGEN(ResPointcloudLight)
HL_IN_PACX_TYPE_AUTOGEN(ResPointcloudModel)
HL_IN_PACX_TYPE_AUTOGEN(ResPss)
HL_IN_PACX_TYPE_AUTOGEN(ResSkinnedMeshCollider)
HL_IN_PACX_TYPE_AUTOGEN(ResTerrainMaterial)

/*
   ============================================================================================
   ==== Sonic Lost World ("sonic2013")
   ============================================================================================
*/

// NOTE: We place ResPacDepend first so we can easily find/skip it as needed.
HL_IN_PACX_SONIC2013_AUTOGEN("pac.d",                   ResPacDepend,                   root,       13, 99)

/* High frequency */
HL_IN_PACX_SONIC2013_AUTOGEN("dds",                     ResTexture,                     mixed,      25, 4)
HL_IN_PACX_SONIC2013_AUTOGEN("material",                ResMirageMaterial,              split,      8,  1)
HL_IN_PACX_SONIC2013_AUTOGEN("model",                   ResModel,                       split,      21, 3)
HL_IN_PACX_SONIC2013_AUTOGEN("skl.hkx",                 ResSkeleton,                    root,       23, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("anm.hkx",                 ResAnimSkeleton,                root,       17, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("uv-anim",                 ResAnimTexSrt,                  mixed,      18, 6)
HL_IN_PACX_SONIC2013_AUTOGEN("swif",                    ResSurfRideProject,             root,       24, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("effect",                  ResGrifEffect,                  v2_merged,  20, 99)

/* Medium frequency */
HL_IN_PACX_SONIC2013_AUTOGEN("anm",                     ResCharAnimScript,              root,       1,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("terrain-model",           ResMirageTerrainModel,          split,      10, 2)
HL_IN_PACX_SONIC2013_AUTOGEN("phy.hkx",                 ResHavokMesh,                   root,       4,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("mat-anim",                ResAnimMaterial,                mixed,      16, 5)
HL_IN_PACX_SONIC2013_AUTOGEN("terrain-instanceinfo",    ResMirageTerrainInstanceInfo,   root,       9,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("lua",                     ResLuaData,                     root,       7,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("hhd",                     ResCustomData,                  root,       3,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("light",                   ResMirageLight,                 root,       5,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("path2.bin",               ResSplinePath2,                 v2_merged,  31, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("lft",                     ResMirageLightField,            root,       6,  99)

/* Low frequency */
HL_IN_PACX_SONIC2013_AUTOGEN("shadow-model",            ResShadowModel,                 v2_merged,  28, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("gism",                    ResGismoConfig,                 root,       2,  99)
HL_IN_PACX_SONIC2013_AUTOGEN("fxcol.bin",               ResFxColFile,                   v2_merged,  15, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("xtb2",                    ResXTB2Data,                    v2_merged,  41, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("model-instanceinfo",      ResModelInstanceInfo,           v2_merged,  22, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("svcol.bin",               ResSvCol,                       v2_merged,  27, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("vis-anim",                ResAnimVis,                     split,      19, 7)
HL_IN_PACX_SONIC2013_AUTOGEN("nextra.bin",              ResNameExtra,                   v2_merged,  11, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("score",                   ResScoreTable,                  root,       40, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("voxel.bin",               ResVoxelContainer,              root,       29, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("pixelshader",             ResMiragePixelShader,           root,       32, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("wpu",                     ResMiragePixelShaderCode,       root,       33, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("fpo",                     ResMiragePixelShaderCode,       root,       33, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("shader-list",             ResMirageShaderList,            root,       35, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("vertexshader",            ResMirageVertexShader,          root,       36, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("wvu",                     ResMirageVertexShaderCode,      root,       37, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("psparam",                 ResMiragePixelShaderParameter,  root,       34, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("vpo",                     ResMirageVertexShaderCode,      root,       37, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("vsparam",                 ResMirageVertexShaderParameter, root,       38, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("path.bin",                ResSplinePath,                  root,       30, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("gsh",                     ResSM4ShaderContainer,          root,       39, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("ttf",                     ResTTFData,                     root,       26, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("bfttf",                   ResBFTTFData,                   root,       12, 99)

/* Never used in LW (but present in the .exe) */
// TODO: These are never used in LW but are in the exe. Is this info correct?
HL_IN_PACX_SONIC2013_AUTOGEN("pt-anim",                 ResAnimTexPat,                  root,       14, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("cam-anim",                ResAnimCameraContainer,         split,      14, 99)
// TODO: Does this work? There's also a "ResAnimLightContainer2" apparently.
HL_IN_PACX_SONIC2013_AUTOGEN("lit-anim",                ResAnimLightContainer,          split,      14, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("morph-anim",              ResAnimMorphTarget,             split,      14, 99)
HL_IN_PACX_SONIC2013_AUTOGEN("map.bin",                 ResDivMap,                      v2_merged,  14, 99)

// ResRawData placed at the end; used as a fallback if no other extensions matched.
HL_IN_PACX_SONIC2013_AUTOGEN("",                        ResRawData,                     v2_merged,  14, 99)

/*
   ============================================================================================
   ==== Mario & Sonic at the Rio 2016 Olympic Games ("unison")
   ============================================================================================
*/

// NOTE: We place ResPacDepend first so we can easily find/skip it as needed.
HL_IN_PACX_UNISON_AUTOGEN("pac.d",              ResPacDepend,                   root,       16, 99)

/* High frequency */
HL_IN_PACX_UNISON_AUTOGEN("dds",                ResTexture,                     split,      32, 4)
HL_IN_PACX_UNISON_AUTOGEN("material",           ResMirageMaterial,              split,      11, 1)
HL_IN_PACX_UNISON_AUTOGEN("model",              ResModel,                       split,      28, 3)
HL_IN_PACX_UNISON_AUTOGEN("anm.hkx",            ResAnimSkeleton,                root,       23, 99)
HL_IN_PACX_UNISON_AUTOGEN("skl.hkx",            ResSkeleton,                    root,       30, 99)
HL_IN_PACX_UNISON_AUTOGEN("swif",               ResSurfRideProject,             root,       31, 99)
HL_IN_PACX_UNISON_AUTOGEN("uv-anim",            ResAnimTexSrt,                  split,      25, 7)
HL_IN_PACX_UNISON_AUTOGEN("vis-anim",           ResAnimVis,                     split,      26, 8)

/* Medium frequency */
HL_IN_PACX_UNISON_AUTOGEN("pt-anim",            ResAnimTexPat,                  root,       24, 99)
HL_IN_PACX_UNISON_AUTOGEN("cam-anim",           ResAnimCameraContainer,         split,      21, 5)
HL_IN_PACX_UNISON_AUTOGEN("asm",                ResAnimator,                    v2_merged,  2,  99)
HL_IN_PACX_UNISON_AUTOGEN("phy.hkx",            ResHavokMesh,                   root,       6,  99)
HL_IN_PACX_UNISON_AUTOGEN("cemt",               ResCyanEffect,                  root,       27, 99)
HL_IN_PACX_UNISON_AUTOGEN("hhd",                ResCustomData,                  root,       5,  99)
HL_IN_PACX_UNISON_AUTOGEN("light",              ResMirageLight,                 root,       9,  99)
HL_IN_PACX_UNISON_AUTOGEN("lft",                ResMirageLightField,            root,       10, 99)
HL_IN_PACX_UNISON_AUTOGEN("probe",              ResProbe,                       v2_merged,  18, 99)
HL_IN_PACX_UNISON_AUTOGEN("terrain-model",      ResMirageTerrainModel,          split,      13, 2)

/* Low frequency */
HL_IN_PACX_UNISON_AUTOGEN("path2.bin",          ResSplinePath2,                 v2_merged,  34, 99)
HL_IN_PACX_UNISON_AUTOGEN("affair",             ResAffair,                      root,       1,  99)
HL_IN_PACX_UNISON_AUTOGEN("mat-anim",           ResAnimMaterial,                split,      22, 6)
HL_IN_PACX_UNISON_AUTOGEN("msbt",               ResTextMSFile,                  root,       35, 99)
HL_IN_PACX_UNISON_AUTOGEN("msbp",               ResTextMSProject,               root,       36, 99)
HL_IN_PACX_UNISON_AUTOGEN("effdb",              ResParticleLocation,            v2_merged,  17, 99)
HL_IN_PACX_UNISON_AUTOGEN("music-score",        ResMusicScore,                  v2_merged,  15, 99)
HL_IN_PACX_UNISON_AUTOGEN("model-instanceinfo", ResModelInstanceInfo,           v2_merged,  29, 99)
HL_IN_PACX_UNISON_AUTOGEN("bfnt.bin",           ResBitmapFont,                  v2_merged,  3,  99)
HL_IN_PACX_UNISON_AUTOGEN("pso",                ResMiragePixelShader,           mixed,      12, 10)
HL_IN_PACX_UNISON_AUTOGEN("vso",                ResMirageVertexShader,          mixed,      14, 9)
HL_IN_PACX_UNISON_AUTOGEN("tga",                ResTGAImage,                    root,       37, 99)
HL_IN_PACX_UNISON_AUTOGEN("cam-offset",         ResCameraOffset,                v2_merged,  4,  99)
HL_IN_PACX_UNISON_AUTOGEN("otf",                ResOTFData,                     root,       8,  99)
HL_IN_PACX_UNISON_AUTOGEN("kern.bin",           ResKerningData,                 v2_merged,  7,  99)
HL_IN_PACX_UNISON_AUTOGEN("stf.bin",            ResStaffrollData,               v2_merged,  20, 99)
HL_IN_PACX_UNISON_AUTOGEN("ttf",                ResTTFData,                     root,       33, 99)

/* Never used in Rio (but present in the .rpx) */
// TODO: These are never used in Rio but are in the rpx. Is this info correct?
// TODO: Does this work? There's also a "ResAnimLightContainer2" apparently.
HL_IN_PACX_UNISON_AUTOGEN("lit-anim",              ResAnimLightContainer,          split,      19, 99)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_UNISON_AUTOGEN("",                      ResRawData,                     v2_merged,  19, 99)

/*
   ============================================================================================
   ==== Sonic Forces ("wars")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_WARS_AUTOGEN("dds",                  ResTexture,                     split)
HL_IN_PACX_WARS_AUTOGEN("model",                ResModel,                       split)
HL_IN_PACX_WARS_AUTOGEN("terrain-model",        ResMirageTerrainModel,          split)
HL_IN_PACX_WARS_AUTOGEN("material",             ResMirageMaterial,              split)
HL_IN_PACX_WARS_AUTOGEN("swif",                 ResSurfRideProject,             root)
HL_IN_PACX_WARS_AUTOGEN("terrain-instanceinfo", ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_WARS_AUTOGEN("uv-anim",              ResAnimTexSrt,                  split)
HL_IN_PACX_WARS_AUTOGEN("cemt",                 ResCyanEffect,                  root)
HL_IN_PACX_WARS_AUTOGEN("rfl",                  ResReflection,                  root)
HL_IN_PACX_WARS_AUTOGEN("skl.hkx",              ResSkeleton,                    root)
HL_IN_PACX_WARS_AUTOGEN("anm.hkx",              ResAnimSkeleton,                root)
HL_IN_PACX_WARS_AUTOGEN("mat-anim",             ResAnimMaterial,                split)
HL_IN_PACX_WARS_AUTOGEN("codetbl",              ResCodeTable,                   root)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-text",           ResText,                        root)
HL_IN_PACX_WARS_AUTOGEN("light",                ResMirageLight,                 root)
HL_IN_PACX_WARS_AUTOGEN("asm",                  ResAnimator,                    v2_merged)
HL_IN_PACX_WARS_AUTOGEN("model-instanceinfo",   ResModelInstanceInfo,           v2_merged)

/* Medium frequency */
HL_IN_PACX_WARS_AUTOGEN("cam-anim",             ResAnimCameraContainer,         split)
HL_IN_PACX_WARS_AUTOGEN("gedit",                ResObjectWorld,                 root)
HL_IN_PACX_WARS_AUTOGEN("phy.hkx",              ResHavokMesh,                   root)
HL_IN_PACX_WARS_AUTOGEN("vis-anim",             ResAnimVis,                     split)
HL_IN_PACX_WARS_AUTOGEN("grass.bin",            ResTerrainGrassInfo,            root)
HL_IN_PACX_WARS_AUTOGEN("scene",                ResScene,                       root)
HL_IN_PACX_WARS_AUTOGEN("effdb",                ResParticleLocation,            v2_merged)
HL_IN_PACX_WARS_AUTOGEN("shlf",                 ResSHLightField,                root)
HL_IN_PACX_WARS_AUTOGEN("gism",                 ResGismoConfig,                 root)
HL_IN_PACX_WARS_AUTOGEN("probe",                ResProbe,                       v2_merged)
HL_IN_PACX_WARS_AUTOGEN("svcol.bin",            ResSvCol,                       v2_merged)

/* Low frequency */
HL_IN_PACX_WARS_AUTOGEN("fxcol.bin",            ResFxColFile,                   v2_merged)
HL_IN_PACX_WARS_AUTOGEN("path",                 ResSplinePath,                  v2_merged)
HL_IN_PACX_WARS_AUTOGEN("pt-anim",              ResAnimTexPat,                  split)
HL_IN_PACX_WARS_AUTOGEN("lit-anim",             ResAnimLightContainer,          split)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-proj",           ResTextProject,                 root)
HL_IN_PACX_WARS_AUTOGEN("cnvrs-meta",           ResTextMeta,                    root)
HL_IN_PACX_WARS_AUTOGEN("scfnt",                ResScalableFontSet,             root)
HL_IN_PACX_WARS_AUTOGEN("pso",                  ResMiragePixelShader,           split)
HL_IN_PACX_WARS_AUTOGEN("vso",                  ResMirageVertexShader,          split)
HL_IN_PACX_WARS_AUTOGEN("shader-list",          ResShaderList,                  root)
HL_IN_PACX_WARS_AUTOGEN("vib",                  ResVibration,                   root)
HL_IN_PACX_WARS_AUTOGEN("bfnt",                 ResBitmapFont,                  v2_merged)

/* Never used in Forces (but present in the .exe) */
// TODO: These are never used in Forces but are in the exe. Is this info correct?
HL_IN_PACX_WARS_AUTOGEN("hhd",                  ResCustomData,                  root)
HL_IN_PACX_WARS_AUTOGEN("lua",                  ResLuaData,                     root)
HL_IN_PACX_WARS_AUTOGEN("lft",                  ResMirageLightField,            root)
HL_IN_PACX_WARS_AUTOGEN("xtb2",                 ResXTB2Data,                    v2_merged)
HL_IN_PACX_WARS_AUTOGEN("nextra.bin",           ResNameExtra,                   v2_merged)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_WARS_AUTOGEN("",                     ResRawData,                     v2_merged)

/*
   ============================================================================================
   ==== Olympic Games Tokyo 2020 ("musashi")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_MUSASHI_AUTOGEN("dds",           ResTexture,                     split)
HL_IN_PACX_MUSASHI_AUTOGEN("material",      ResMirageMaterial,              split)
HL_IN_PACX_MUSASHI_AUTOGEN("model",         ResModel,                       mixed)
HL_IN_PACX_MUSASHI_AUTOGEN("skl.pxd",       ResSkeletonPxd,                 root)
HL_IN_PACX_MUSASHI_AUTOGEN("vli",           ResVertexLinkInfo,              root)
HL_IN_PACX_MUSASHI_AUTOGEN("rfl",           ResReflection,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("swif",          ResSurfRideProject,             root)
HL_IN_PACX_MUSASHI_AUTOGEN("cnvrs-text",    ResText,                        root)

/* Medium frequency */
HL_IN_PACX_MUSASHI_AUTOGEN("uv-anim",       ResAnimTexSrt,                  split)
HL_IN_PACX_MUSASHI_AUTOGEN("cam-anim",      ResAnimCameraContainer,         split)
HL_IN_PACX_MUSASHI_AUTOGEN("anm.pxd",       ResAnimationPxd,                root)
HL_IN_PACX_MUSASHI_AUTOGEN("lua",           ResLuaData,                     root)
HL_IN_PACX_MUSASHI_AUTOGEN("asm",           ResAnimator,                    root)
HL_IN_PACX_MUSASHI_AUTOGEN("terrain-model", ResMirageTerrainModel,          split)
HL_IN_PACX_MUSASHI_AUTOGEN("light",         ResMirageLight,                 root)
HL_IN_PACX_MUSASHI_AUTOGEN("lf",            ResSHLightField,                root)
HL_IN_PACX_MUSASHI_AUTOGEN("cnvrs-meta",    ResTextMeta,                    root)
HL_IN_PACX_MUSASHI_AUTOGEN("cnvrs-proj",    ResTextProject,                 root)
HL_IN_PACX_MUSASHI_AUTOGEN("cemt",          ResCyanEffect,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("mat-anim",      ResAnimMaterial,                split)

/* Low frequency */
HL_IN_PACX_MUSASHI_AUTOGEN("scfnt",         ResScalableFontSet,             root)
HL_IN_PACX_MUSASHI_AUTOGEN("pso",           ResMiragePixelShader,           split)
HL_IN_PACX_MUSASHI_AUTOGEN("scene",         ResScene,                       root)
HL_IN_PACX_MUSASHI_AUTOGEN("path.bin",      ResSplinePath,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("probe",         ResProbe,                       root)
HL_IN_PACX_MUSASHI_AUTOGEN("aid",           ResAnimatedInstance,            root)
HL_IN_PACX_MUSASHI_AUTOGEN("gedit",         ResObjectWorld,                 root)
HL_IN_PACX_MUSASHI_AUTOGEN("btmesh",        ResBulletMesh,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("affair",        ResAffair,                      root)
HL_IN_PACX_MUSASHI_AUTOGEN("vso",           ResMirageVertexShader,          split)
HL_IN_PACX_MUSASHI_AUTOGEN("vat",           ResVertexAnimationTexture,      root)
HL_IN_PACX_MUSASHI_AUTOGEN("pt-anim",       ResAnimTexPat,                  split)
HL_IN_PACX_MUSASHI_AUTOGEN("cso",           ResMirageComputeShader,         split)
HL_IN_PACX_MUSASHI_AUTOGEN("shader-list",   ResShaderList,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("anm.hkx",       ResAnimSkeleton,                root)
HL_IN_PACX_MUSASHI_AUTOGEN("bfnt",          ResBitmapFont,                  root)
HL_IN_PACX_MUSASHI_AUTOGEN("effdb",         ResParticleLocation,            root)
HL_IN_PACX_MUSASHI_AUTOGEN("vib",           ResVibration,                   root)
HL_IN_PACX_MUSASHI_AUTOGEN("skl.hkx",       ResSkeleton,                    root)

/* ResRawData placed at the end; used if no other extensions matched. */
HL_IN_PACX_MUSASHI_AUTOGEN("",              ResRawData,                     v2_merged)

/*
   ============================================================================================
   ==== Mario & Sonic at the Tokyo 2020 Olympic Games ("rings")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_RINGS_AUTOGEN("dds",                     ResTexture,                     split)
HL_IN_PACX_RINGS_AUTOGEN("material",                ResMirageMaterial,              split)
HL_IN_PACX_RINGS_AUTOGEN("model",                   ResModel,                       mixed)
HL_IN_PACX_RINGS_AUTOGEN("swif",                    ResSurfRideProject,             root)
HL_IN_PACX_RINGS_AUTOGEN("anm.pxd",                 ResAnimationPxd,                root)
HL_IN_PACX_RINGS_AUTOGEN("asm",                     ResAnimator,                    v2_merged)
HL_IN_PACX_RINGS_AUTOGEN("effdb",                   ResParticleLocation,            root)
HL_IN_PACX_RINGS_AUTOGEN("uv-anim",                 ResAnimTexSrt,                  split)
HL_IN_PACX_RINGS_AUTOGEN("vis-anim",                ResAnimVis,                     split)
HL_IN_PACX_RINGS_AUTOGEN("skl.pxd",                 ResSkeletonPxd,                 root)
HL_IN_PACX_RINGS_AUTOGEN("lua",                     ResLuaData,                     root)
HL_IN_PACX_RINGS_AUTOGEN("terrain-model",           ResMirageTerrainModel,          split)
HL_IN_PACX_RINGS_AUTOGEN("cnvrs-text",              ResText,                        root)
HL_IN_PACX_RINGS_AUTOGEN("rfl",                     ResReflection,                  root)
HL_IN_PACX_RINGS_AUTOGEN("gedit",                   ResObjectWorld,                 root)
HL_IN_PACX_RINGS_AUTOGEN("cam-anim",                ResAnimCameraContainer,         split)

/* Medium frequency */
HL_IN_PACX_RINGS_AUTOGEN("aid",                     ResAnimatedInstance,            root)
HL_IN_PACX_RINGS_AUTOGEN("pt-anim",                 ResAnimTexPat,                  split)
HL_IN_PACX_RINGS_AUTOGEN("light",                   ResMirageLight,                 root)
HL_IN_PACX_RINGS_AUTOGEN("cemt",                    ResCyanEffect,                  root)
HL_IN_PACX_RINGS_AUTOGEN("lf",                      ResSHLightField,                root)
HL_IN_PACX_RINGS_AUTOGEN("probe",                   ResProbe,                       v2_merged)
HL_IN_PACX_RINGS_AUTOGEN("btmesh",                  ResBulletMesh,                  root)
HL_IN_PACX_RINGS_AUTOGEN("anim2d",                  ResAnim2D,                      root)
HL_IN_PACX_RINGS_AUTOGEN("fpd",                     ResFlashLight,                  root)
HL_IN_PACX_RINGS_AUTOGEN("mat-anim",                ResAnimMaterial,                split)
HL_IN_PACX_RINGS_AUTOGEN("path.bin",                ResSplinePath,                  v2_merged)
HL_IN_PACX_RINGS_AUTOGEN("scfnt",                   ResScalableFontSet,             root)

/* Low frequency */
HL_IN_PACX_RINGS_AUTOGEN("cnvrs-meta",              ResTextMeta,                    root)
HL_IN_PACX_RINGS_AUTOGEN("cnvrs-proj",              ResTextProject,                 root)
HL_IN_PACX_RINGS_AUTOGEN("pso",                     ResMiragePixelShader,           split)
HL_IN_PACX_RINGS_AUTOGEN("vso",                     ResMirageVertexShader,          split)
HL_IN_PACX_RINGS_AUTOGEN("okern",                   ResOpticalKerning,              root)
HL_IN_PACX_RINGS_AUTOGEN("btsb",                    ResSoftBody,                    root)
HL_IN_PACX_RINGS_AUTOGEN("scene",                   ResScene,                       root)
HL_IN_PACX_RINGS_AUTOGEN("svcol.bin",               ResSvCol,                       v2_merged)
HL_IN_PACX_RINGS_AUTOGEN("terrain-instanceinfo",    ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_RINGS_AUTOGEN("cso",                     ResMirageComputeShader,         split)
HL_IN_PACX_RINGS_AUTOGEN("nmt",                     ResNavMeshTile,                 root)
HL_IN_PACX_RINGS_AUTOGEN("shader-list",             ResShaderList,                  root)
HL_IN_PACX_RINGS_AUTOGEN("vib",                     ResVibration,                   root)
HL_IN_PACX_RINGS_AUTOGEN("bnvib",                   ResVibrationHD,                 root)
HL_IN_PACX_RINGS_AUTOGEN("bfnt",                    ResBitmapFont,                  v2_merged)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_RINGS_AUTOGEN("",                        ResRawData,                     v2_merged)

/*
   ============================================================================================
   ==== Sakura Wars ("arukas")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_ARUKAS_AUTOGEN("rfl",                    ResReflection,                  root)
HL_IN_PACX_ARUKAS_AUTOGEN("dds",                    ResTexture,                     split)
HL_IN_PACX_ARUKAS_AUTOGEN("material",               ResMirageMaterial,              split)
HL_IN_PACX_ARUKAS_AUTOGEN("model",                  ResModel,                       split)
HL_IN_PACX_ARUKAS_AUTOGEN("anm.pxd",                ResAnimationPxd,                root)
HL_IN_PACX_ARUKAS_AUTOGEN("asm",                    ResAnimator,                    v2_merged)
HL_IN_PACX_ARUKAS_AUTOGEN("dv.bin",                 ResDvRawData,                   root)
HL_IN_PACX_ARUKAS_AUTOGEN("cam-anim",               ResAnimCameraContainer,         split)
HL_IN_PACX_ARUKAS_AUTOGEN("skl.pxd",                ResSkeletonPxd,                 root)
HL_IN_PACX_ARUKAS_AUTOGEN("terrain-model",          ResMirageTerrainModel,          split)
HL_IN_PACX_ARUKAS_AUTOGEN("swif",                   ResSurfRideProject,             root)
HL_IN_PACX_ARUKAS_AUTOGEN("mat-anim",               ResAnimMaterial,                split)
HL_IN_PACX_ARUKAS_AUTOGEN("light",                  ResMirageLight,                 root)

/* Medium frequency */
HL_IN_PACX_ARUKAS_AUTOGEN("uv-anim",                ResAnimTexSrt,                  split)
HL_IN_PACX_ARUKAS_AUTOGEN("btmesh",                 ResBulletMesh,                  root)
HL_IN_PACX_ARUKAS_AUTOGEN("lf",                     ResSHLightField,                root)
HL_IN_PACX_ARUKAS_AUTOGEN("terrain-instanceinfo",   ResMirageTerrainInstanceInfo,   root)
HL_IN_PACX_ARUKAS_AUTOGEN("gedit",                  ResObjectWorld,                 root)
HL_IN_PACX_ARUKAS_AUTOGEN("probe",                  ResProbe,                       v2_merged)
HL_IN_PACX_ARUKAS_AUTOGEN("msb",                    ResScript,                      root)
HL_IN_PACX_ARUKAS_AUTOGEN("cemt",                   ResCyanEffect,                  root)
HL_IN_PACX_ARUKAS_AUTOGEN("cnvrs-text",             ResText,                        root)
HL_IN_PACX_ARUKAS_AUTOGEN("lit-anim",               ResAnimLightContainer,          split)
HL_IN_PACX_ARUKAS_AUTOGEN("lua",                    ResLuaData,                     root)
HL_IN_PACX_ARUKAS_AUTOGEN("effdb",                  ResParticleLocation,            root)
HL_IN_PACX_ARUKAS_AUTOGEN("model-instanceinfo",     ResModelInstanceInfo,           root)

/* Low frequency */
HL_IN_PACX_ARUKAS_AUTOGEN("fxcol.bin",              ResFxColFile,                   root)
HL_IN_PACX_ARUKAS_AUTOGEN("nmt",                    ResNavMeshTile,                 root)
HL_IN_PACX_ARUKAS_AUTOGEN("vis-anim",               ResAnimVis,                     split)
HL_IN_PACX_ARUKAS_AUTOGEN("mx_path",                ResMxGraph,                     root)
HL_IN_PACX_ARUKAS_AUTOGEN("svcol.bin",              ResSvCol,                       v2_merged)
HL_IN_PACX_ARUKAS_AUTOGEN("pso",                    ResMiragePixelShader,           split)
HL_IN_PACX_ARUKAS_AUTOGEN("nmc",                    ResNavMeshConfig,               root)
HL_IN_PACX_ARUKAS_AUTOGEN("scfnt",                  ResScalableFontSet,             root)
HL_IN_PACX_ARUKAS_AUTOGEN("pt-anim",                ResAnimTexPat,                  split)
HL_IN_PACX_ARUKAS_AUTOGEN("bfnt",                   ResBitmapFont,                  v2_merged)
HL_IN_PACX_ARUKAS_AUTOGEN("cso",                    ResMirageComputeShader,         split)
HL_IN_PACX_ARUKAS_AUTOGEN("vso",                    ResMirageVertexShader,          split)
HL_IN_PACX_ARUKAS_AUTOGEN("okern",                  ResOpticalKerning,              root)
HL_IN_PACX_ARUKAS_AUTOGEN("png",                    ResRawData,                     root)
HL_IN_PACX_ARUKAS_AUTOGEN("vib",                    ResVibration,                   root)
HL_IN_PACX_ARUKAS_AUTOGEN("cnvrs-proj",             ResTextProject,                 root)
HL_IN_PACX_ARUKAS_AUTOGEN("cnvrs-meta",             ResTextMeta,                    root)
HL_IN_PACX_ARUKAS_AUTOGEN("shader-list",            ResShaderList,                  root)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_ARUKAS_AUTOGEN("",                       ResRawData,                     v2_merged)

/*
   ============================================================================================
   ==== Puyo Puyo Tetris 2 ("tenpex")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_TENPEX_AUTOGEN("dds",                ResTexture,                     split)
HL_IN_PACX_TENPEX_AUTOGEN("swif",               ResSurfRideProject,             root)

/* Medium frequency */
HL_IN_PACX_TENPEX_AUTOGEN("pss",                ResPss,                         root)
HL_IN_PACX_TENPEX_AUTOGEN("scfnt",              ResScalableFontSet,             root)
HL_IN_PACX_TENPEX_AUTOGEN("cnvrs-text",         ResText,                        root)
HL_IN_PACX_TENPEX_AUTOGEN("cnvrs-proj",         ResTextProject,                 root)
HL_IN_PACX_TENPEX_AUTOGEN("rfl",                ResReflection,                  root)

/* Low frequency */
HL_IN_PACX_TENPEX_AUTOGEN("cnvrs-meta",         ResTextMeta,                    root)
HL_IN_PACX_TENPEX_AUTOGEN("bfnt",               ResBitmapFont,                  v2_merged)
HL_IN_PACX_TENPEX_AUTOGEN("bnvib",              ResVibrationHD,                 root)
HL_IN_PACX_TENPEX_AUTOGEN("vib",                ResVibration,                   root)

// ResRawData placed at the end; used if no other extensions matched.
HL_IN_PACX_TENPEX_AUTOGEN("",                   ResRawData,                     v2_merged)

/*
   ============================================================================================
   ==== Sonic Origins ("hite")
   ============================================================================================
*/

/* High frequency */
HL_IN_PACX_HITE_AUTOGEN("level",            ResLevel,                   root)
HL_IN_PACX_HITE_AUTOGEN("cnvrs-text",       ResText,                    root)
HL_IN_PACX_HITE_AUTOGEN("dds",              ResTexture,                 split)

/* Medium frequency */
HL_IN_PACX_HITE_AUTOGEN("swif",             ResSurfRideProject,         root)
HL_IN_PACX_HITE_AUTOGEN("cnvrs-proj",       ResTextProject,             root)
HL_IN_PACX_HITE_AUTOGEN("material",         ResMirageMaterial,          split)
HL_IN_PACX_HITE_AUTOGEN("rfl",              ResReflection,              root)

/* Low frequency */
HL_IN_PACX_HITE_AUTOGEN("bfnt",             ResBitmapFont,              root)
HL_IN_PACX_HITE_AUTOGEN("gedit",            ResObjectWorld,             root)
HL_IN_PACX_HITE_AUTOGEN("cso",              ResMirageComputeShader,     split)
HL_IN_PACX_HITE_AUTOGEN("pso",              ResMiragePixelShader,       split)
HL_IN_PACX_HITE_AUTOGEN("vso",              ResMirageVertexShader,      split)
HL_IN_PACX_HITE_AUTOGEN("shader-list",      ResShaderList,              root)
HL_IN_PACX_HITE_AUTOGEN("anm.pxd",          ResAnimationPxd,            root)
HL_IN_PACX_HITE_AUTOGEN("cam-anim",         ResAnimCameraContainer,     split)
HL_IN_PACX_HITE_AUTOGEN("uv-anim",          ResAnimTexSrt,              split)
HL_IN_PACX_HITE_AUTOGEN("vis-anim",         ResAnimVis,                 split)
HL_IN_PACX_HITE_AUTOGEN("cemt",             ResCyanEffect,              root)
HL_IN_PACX_HITE_AUTOGEN("model",            ResModel,                   split)
HL_IN_PACX_HITE_AUTOGEN("skl.pxd",          ResSkeletonPxd,             root)
HL_IN_PACX_HITE_AUTOGEN("scfnt",            ResScalableFontSet,         root)
HL_IN_PACX_HITE_AUTOGEN("mat-anim",         ResAnimMaterial,            split)
HL_IN_PACX_HITE_AUTOGEN("light",            ResMirageLight,             root)
HL_IN_PACX_HITE_AUTOGEN("terrain-model",    ResMirageTerrainModel,      split)
HL_IN_PACX_HITE_AUTOGEN("probe",            ResProbe,                   root)
HL_IN_PACX_HITE_AUTOGEN("lf",               ResSHLightField,            root)
HL_IN_PACX_HITE_AUTOGEN("path",             ResSplinePath,              root)
HL_IN_PACX_HITE_AUTOGEN("cnvrs-meta",       ResTextMeta,                root)

/*
   ============================================================================================
   ==== Sonic Frontiers ("rangers")
   ============================================================================================
*/

HL_IN_PACX_RANGERS_AUTOGEN("level",             ResLevel,                   root)
HL_IN_PACX_RANGERS_AUTOGEN("anm.pxd",           ResAnimationPxd,            root)
HL_IN_PACX_RANGERS_AUTOGEN("skl.pxd",           ResSkeletonPxd,             root)
HL_IN_PACX_RANGERS_AUTOGEN("dds",               ResTexture,                 split)
HL_IN_PACX_RANGERS_AUTOGEN("asm",               ResAnimator,                root)
HL_IN_PACX_RANGERS_AUTOGEN("mat-anim",          ResAnimMaterial,            split)
HL_IN_PACX_RANGERS_AUTOGEN("dvscene",           ResDvScene,                 root)
HL_IN_PACX_RANGERS_AUTOGEN("uv-anim",           ResAnimTexSrt,              split)
HL_IN_PACX_RANGERS_AUTOGEN("cemt",              ResCyanEffect,              root)
HL_IN_PACX_RANGERS_AUTOGEN("material",          ResMirageMaterial,          split)
HL_IN_PACX_RANGERS_AUTOGEN("model",             ResModel,                   split)
HL_IN_PACX_RANGERS_AUTOGEN("cam-anim",          ResAnimCameraContainer,     split)
HL_IN_PACX_RANGERS_AUTOGEN("vis-anim",          ResAnimVis,                 split)
HL_IN_PACX_RANGERS_AUTOGEN("rfl",               ResReflection,              root)
HL_IN_PACX_RANGERS_AUTOGEN("cnvrs-text",        ResText,                    root)
HL_IN_PACX_RANGERS_AUTOGEN("btmesh",            ResBulletMesh,              root)
HL_IN_PACX_RANGERS_AUTOGEN("effdb",             ResParticleLocation,        root)
HL_IN_PACX_RANGERS_AUTOGEN("gedit",             ResObjectWorld,             root)
HL_IN_PACX_RANGERS_AUTOGEN("pccol",             ResPointcloudCollision,     root)
HL_IN_PACX_RANGERS_AUTOGEN("path",              ResSplinePath,              root)
HL_IN_PACX_RANGERS_AUTOGEN("lf",                ResSHLightField,            root)
HL_IN_PACX_RANGERS_AUTOGEN("probe",             ResProbe,                   root)
HL_IN_PACX_RANGERS_AUTOGEN("occ",               ResOcclusionCapsule,        root)
HL_IN_PACX_RANGERS_AUTOGEN("swif",              ResSurfRideProject,         root)
HL_IN_PACX_RANGERS_AUTOGEN("densitysetting",    ResDensitySetting,          root)
HL_IN_PACX_RANGERS_AUTOGEN("densitypointcloud", ResDensityPointCloud,       root)
HL_IN_PACX_RANGERS_AUTOGEN("lua",               ResLuaData,                 root)
HL_IN_PACX_RANGERS_AUTOGEN("btsmc",             ResSkinnedMeshCollider,     root)
HL_IN_PACX_RANGERS_AUTOGEN("terrain-model",     ResMirageTerrainModel,      split)
HL_IN_PACX_RANGERS_AUTOGEN("gismop",            ResGismoConfigPlan,         root)
HL_IN_PACX_RANGERS_AUTOGEN("fxcol",             ResFxColFile,               root)
HL_IN_PACX_RANGERS_AUTOGEN("gismod",            ResGismoConfigDesign,       root)
HL_IN_PACX_RANGERS_AUTOGEN("nmt",               ResNavMeshTile,             root)
HL_IN_PACX_RANGERS_AUTOGEN("pcmodel",           ResPointcloudModel,         root)
HL_IN_PACX_RANGERS_AUTOGEN("nmc",               ResNavMeshConfig,           root)
HL_IN_PACX_RANGERS_AUTOGEN("vat",               ResVertexAnimationTexture,  root)
HL_IN_PACX_RANGERS_AUTOGEN("heightfield",       ResHeightField,             root)
HL_IN_PACX_RANGERS_AUTOGEN("light",             ResMirageLight,             root)
HL_IN_PACX_RANGERS_AUTOGEN("pba",               ResPhysicalSkeleton,        root)
HL_IN_PACX_RANGERS_AUTOGEN("pcrt",              ResPointcloudLight,         root)
HL_IN_PACX_RANGERS_AUTOGEN("aism",              ResAIStateMachine,          root)
HL_IN_PACX_RANGERS_AUTOGEN("cnvrs-proj",        ResTextProject,             root)
HL_IN_PACX_RANGERS_AUTOGEN("terrain-material",  ResTerrainMaterial,         root)
HL_IN_PACX_RANGERS_AUTOGEN("pt-anim",           ResAnimTexPat,              split)
HL_IN_PACX_RANGERS_AUTOGEN("okern",             ResOpticalKerning,          root)
HL_IN_PACX_RANGERS_AUTOGEN("scfnt",             ResScalableFontSet,         root)
HL_IN_PACX_RANGERS_AUTOGEN("cso",               ResMirageComputeShader,     split)
HL_IN_PACX_RANGERS_AUTOGEN("pso",               ResMiragePixelShader,       split)
HL_IN_PACX_RANGERS_AUTOGEN("vib",               ResVibration,               root)
HL_IN_PACX_RANGERS_AUTOGEN("vso",               ResMirageVertexShader,      split)
HL_IN_PACX_RANGERS_AUTOGEN("pointcloud",        ResPointcloud,              root)
HL_IN_PACX_RANGERS_AUTOGEN("shader-list",       ResShaderList,              root)
HL_IN_PACX_RANGERS_AUTOGEN("cnvrs-meta",        ResTextMeta,                root)

// Undefine all auto-generator macros.
#undef HL_IN_PACX_TYPE_AUTOGEN
#undef HL_IN_PACX_SONIC2013_AUTOGEN
#undef HL_IN_PACX_UNISON_AUTOGEN
#undef HL_IN_PACX_WARS_AUTOGEN
#undef HL_IN_PACX_MUSASHI_AUTOGEN
#undef HL_IN_PACX_RINGS_AUTOGEN
#undef HL_IN_PACX_ARUKAS_AUTOGEN
#undef HL_IN_PACX_TENPEX_AUTOGEN
#undef HL_IN_PACX_HITE_AUTOGEN
#undef HL_IN_PACX_RANGERS_AUTOGEN
