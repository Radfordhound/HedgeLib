#ifndef HL_GRIF_H_INCLUDED
#define HL_GRIF_H_INCLUDED
#include "../hl_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum HlBINAEndianFlag HlBINAEndianFlag;

typedef struct HlGrifAnimationParameterV106
{
    HlU32 uk1;
    HlU32 uk2;
    /* TODO: Finish this struct. */
}
HlGrifAnimationParameterV106;

/* TODO: Figure out actual size and finish/uncomment this: */
/*HL_STATIC_ASSERT_SIZE(HlGrifAnimationParameterV106, );*/

typedef struct HlGrifTextureParameterV106
{
    HL_OFF32_STR nameOffset;
    /** @brief Set by the game; always just set to NULL (0) within the actual files. */
    HlU32 dataPtr;
    HlU32 uk1;
    HlU32 uk2;
}
HlGrifTextureParameterV106;

HL_STATIC_ASSERT_SIZE(HlGrifTextureParameterV106, 16);

typedef struct HlGrifMaterialParameterV106
{
    HL_OFF32_STR nameOffset;
    HlGrifTextureParameterV106 textures[2];
    HlU32 texCount;
    HlU32 uk2;
    HlU32 uk3;
    float uk4;
    HlU32 uk5;
    HlU32 uk6;
    HL_OFF32_STR shaderName;
    /** @brief Set by the game; always just set to NULL (0) within the actual files. */
    HlU32 vertexShaderPtr;
    /** @brief Set by the game; always just set to NULL (0) within the actual files. */
    HlU32 pixelShaderPtr;
    HlU32 uk7;
    HlU32 uk8;
    HlU32 uk9;
    HlU32 uk10;
    HlU32 isEnableLightField;
    HlU32 uk11;
    HlU32 uk12;
    HlU32 uk13;
    HlU32 uk14;
    HlU32 uk15;
    HlU32 uk16;
    HlU32 uk17;
    HlU32 uk18;
    HlU32 uk19;
    HlU32 uk20;
    HlU32 uk21;
    HlU32 uk22;
    HL_OFF32(void) uk23;
    HlU32 uk24;
    HlU32 uk25;
    HlU32 uk26;
}
HlGrifMaterialParameterV106;

HL_STATIC_ASSERT_SIZE(HlGrifMaterialParameterV106, 0x9C);

typedef struct HlGrifParticleParameterV106
{
    HL_OFF32_STR nameOffset;
    HlU32 type;
    float uk1;
    HlU32 uk2;
    HlU32 uk3;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    float uk8;
    HlU32 uk9;
    HlU32 uk10;
    float uk11;
    float uk12;
    float uk13;
    float uk14;
    float uk15;
    float uk16;
    float uk17;
    float uk18;
    HlU32 uk19;
    HlU32 uk20;
    HlU32 uk21;
    HlU32 uk22;
    HlVector3 uk23;
    HlU32 uk24;
    HlVector3 uk25;
    HlU32 uk26;
    HlU32 uk27;
    HlU32 uk28;
    HlU32 uk29;
    HlU32 uk30;
    HlU32 uk31;
    HlU32 uk32;
    float uk33;
    HlU32 uk34;
    HlU32 uk35;
    HlU32 uk36;
    HlU32 uk37;
    HlU32 uk38;
    HlU32 uk39;
    HlU32 uk40;
    float uk41;
    HlU32 uk42;
    HlU32 uk43;
    HlU32 uk44;
    HlU32 uk45;
    HlU32 uk46;
    HlU32 uk47;
    HlU32 uk48;
    HlU32 uk49;
    HlU32 uk50;
    HlVector3 uk51;
    HlU32 uk52;
    HlVector3 uk53;
    HlU32 uk54;
    HlVector3 uk55;
    HlU32 uk56;
    HlU32 uk57;
    float uk58;
    HlU32 uk59;
    HlU32 uk60;
    HlU32 uk61;
    HlU32 uk62;
    HlU32 uk63;
    HlU32 uk64;
    HlU32 uk65;
    HlU32 uk66;
    HlU32 uk67;
    HlU32 uk68;
    HlU32 uk69;
    HlU32 uk70;
    HlS32 uk71;
    HlS32 uk72;
    HlU32 uk73;
    HlU32 uk74;
    HlU32 uk75;
    HlU32 uk76;
    HlU32 uk77;
    float uk78;
    HlU32 uk79;
    HlU32 uk80;
    HlU32 uk81;
    HlU32 uk82;
    HlU32 uk83;
    HlU32 uk84;
    HlU32 uk85;
    HlU32 uk86;
    HlU32 uk87;
    HlU32 uk88;
    HlU32 uk89;
    HlU32 uk90;
    HlU32 uk91;
    HlU32 uk92;
    HlU32 uk93;
    HL_OFF32(HlGrifMaterialParameterV106) materialOffset;
    HlU32 uk94;
    HlS32 uk95;
    HlU32 uk96;
    HlS32 uk97;
    HlU32 uk98;
    HL_OFF32_STR meshNameOffset;
    /** @brief Set by the game; always just set to NULL (0) within the actual files. */
    HlU32 meshDataPtr;
    HlU32 uk99;
    HlU32 uk100;
    HlU32 uk101;
    HlU32 uk102;
    HlU32 uk103;
    float uk104;
    HlU32 uk105;
    float uk106;
    float uk107;
    float uk108;
    float uk109;
    HlU32 uk110;
    HL_OFF32(HlGrifAnimationParameterV106) animOffsets[30];
    HL_OFF32(struct HlGrifParticleParameterV106) nextParticleOffset;
    HlU32 uk111;
    HlU32 uk112;
    HlU32 uk113;
    HlU32 uk114;
    HlU32 uk115;
    HlU32 uk116;
    HlU32 uk117;
    HlU32 uk118;
    HlU32 uk119;
    HlU32 uk120;
    HlU32 uk121;
    HlU32 uk122;
    HlU32 uk123;
    HlU32 uk124;
    HlU32 uk125;
    HlU32 uk126;
    HlU32 uk127;
    HlU32 uk128;
    HlU32 uk129;
    HlU32 uk130;
    HlU32 uk131;
    HlU32 uk132;
    HlU32 uk133;
    HlU32 uk134;
    HlU32 uk135;
    HlU32 uk136;
}
HlGrifParticleParameterV106;

HL_STATIC_ASSERT_SIZE(HlGrifParticleParameterV106, 0x2D8);

typedef struct HlGrifEmitterParameterV106
{
    HL_OFF32_STR nameOffset;
    HlU32 uk1;
    float uk2;
    float uk3;
    HlVector3 uk4;
    HlU32 uk5;
    HlU32 uk6;
    HlU32 uk7;
    HlU32 uk8;
    HlU32 uk9;
    HlU32 uk10;
    HlU32 uk11;
    HlU32 uk12;
    HlU32 uk13;
    HlU32 uk14;
    HlU32 uk15;
    HlU32 uk16;
    HlU32 uk17;
    float uk18;
    float uk19;
    float uk20;
    HlU32 uk21;
    HlU32 uk22;
    float uk23;
    HlU32 uk24;
    HlU32 uk25;
    float uk26;
    float uk27;
    HlU32 uk28;
    HlU32 uk29;
    HlVector3 uk30;
    HlU32 uk31;
    HlU32 uk32;
    float uk33;
    float uk34;
    float uk35;
    float uk36;
    /** @brief Always 1? */
    HlU32 uk37;
    HL_OFF32(HlGrifAnimationParameterV106) animOffsets[12];
    HL_OFF32(HlGrifParticleParameterV106) particleOffset;
    HL_OFF32(struct HlGrifEmitterParameterV106) nextEmitterOffset;
    HlU32 uk46;
    HlU32 uk47;
    HlU32 uk48;
    HlU32 uk49;
    HlU32 uk50;
    HlU32 uk51;
    HlU32 uk52;
    HlU32 uk53;
    HlU32 uk54;
    HlU32 uk55;
    HlU32 uk56;
    HlU32 uk57;
    HlU32 uk58;
    HlU32 uk59;
    HlU32 uk60;
    HlU32 uk61;
    HlU32 uk62;
    HlU32 uk63;
    HlU32 uk64;
    HlU32 uk65;
    HlU32 uk66;
    HlU32 uk67;
    HlU32 uk68;
    HlU32 uk69;
    HlU32 uk70;
    HlU32 uk71;
    HlU32 uk72;
    HlU32 uk73;
}
HlGrifEmitterParameterV106;

HL_STATIC_ASSERT_SIZE(HlGrifEmitterParameterV106, 0x150);

typedef struct HlGrifEffectParameterV106
{
    HL_OFF32_STR nameOffset;
    float uk1;
    /** @brief RangeIn? */
    float rangeIn;
    /** @brief RangeOut? */
    float rangeOut;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    HlVector3 uk8;
    HlU32 uk11;
    HlU32 uk12;
    HlU32 uk13;
    HlU32 uk14;
    HlU32 uk15;
    /** @brief Scale XYZ? */
    float uk16;
    float uk17;
    float uk18;
    HlU32 uk19;
    HlU32 uk20;
    float uk21;
    float uk22;
    float uk23;
    float uk24;
    /** @brief Always 1? */
    HlU32 uk25;
    HL_OFF32(HlGrifAnimationParameterV106) animOffsets[14];
    HL_OFF32(HlGrifEmitterParameterV106) emitterOffset;
    HL_OFF32(struct HlGrifEffectParameterV106) nextParamOffset;
    HlU32 uk40;
    HlU32 uk41;
    HlU32 uk42;
    HlU32 uk43;
    HlU32 uk44;
    HlU32 uk45;
    HlU32 uk46;
    HlU32 uk47;
    HlU32 uk48;
    HlU32 uk49;
    HlU32 uk50;
    HlU32 uk51;
    HlU32 uk52;
    HlU32 uk53;
    HlU32 uk54;
    HlU32 uk55;
    HlU32 uk56;
    HlU32 uk57;
    HlU32 uk58;
    HlU32 uk59;
    HlU32 uk60;
    HlU32 uk61;
    HlU32 uk62;
    HlU32 uk63;
    HlU32 uk64;
    HlU32 uk65;
    HlU32 uk66;
    HlU32 uk67;
}
HlGrifEffectParameterV106;

HL_STATIC_ASSERT_SIZE(HlGrifEffectParameterV106, 0x118);

typedef struct HlGrifEffect
{
    HlU32 signature;
    HlU32 version;
    HlU32 flags;
    HL_OFF32(void) paramsOffset;
}
HlGrifEffect;

HL_STATIC_ASSERT_SIZE(HlGrifEffect, 16);

HL_API void hlGrifTextureParameterV106Swap(HlGrifTextureParameterV106* texture,
    HlBool swapOffsets);

HL_API void hlGrifMaterialParameterV106Swap(HlGrifMaterialParameterV106* material,
    HlBool swapOffsets);

HL_API void hlGrifParticleParameterV106Swap(HlGrifParticleParameterV106* particle,
    HlBool swapOffsets);

HL_API void hlGrifEmitterParameterV106Swap(HlGrifEmitterParameterV106* emitter,
    HlBool swapOffsets);

HL_API void hlGrifEffectParameterV106Swap(HlGrifEffectParameterV106* param,
    HlBool swapOffsets);

HL_API void hlGrifEffectSwap(HlGrifEffect* effect, HlBool swapOffsets);
HL_API HlResult hlGrifEffectFix(HlGrifEffect* effect, HlBINAEndianFlag endianFlag);

#ifdef __cplusplus
}
#endif
#endif
