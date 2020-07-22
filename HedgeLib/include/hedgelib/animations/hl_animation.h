#ifndef HL_ANIMATION_H_INCLUDED
#define HL_ANIMATION_H_INCLUDED
#include "../hl_math.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlFile HlFile;
typedef struct HlBlob HlBlob;

typedef struct HlAnimationIPData
{
    HlVector2 in;
    HlVector2 out;
}
HlAnimationIPData;

/* Defines an HlAnimationKeyX struct where X is a type of data that can be stored in a keyframe. */
#define HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix, type, typeSuffix, extra)\
    typedef struct HlAnimationKey##nameSuffix {\
        float frame;\
        type value typeSuffix;\
        extra\
    } HlAnimationKey##nameSuffix

/* Defines 8 HlAnimationKeyX structs, one for each dimension (1D, 2D, 3D, and 4D) and IP type. */
#define HL_IN_DEFINE_ANIM_KEY_TYPE(nameSuffix, type)\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix, type, HL_EMPTY, HL_EMPTY);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##2, type, [2], HL_EMPTY);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##3, type, [3], HL_EMPTY);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##4, type, [4], HL_EMPTY);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##IP, type, HL_EMPTY, HlAnimationIPData ip;);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##IP##2, type, [2], HlAnimationIPData ip;);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##IP##3, type, [3], HlAnimationIPData ip;);\
    HL_IN_DEFINE_ANIM_KEY_TYPE_DIM(nameSuffix##IP##4, type, [4], HlAnimationIPData ip;)

/* Define HlAnimationKeys for each type. */
HL_IN_DEFINE_ANIM_KEY_TYPE(Float, float);
HL_IN_DEFINE_ANIM_KEY_TYPE(Short, short);
HL_IN_DEFINE_ANIM_KEY_TYPE(UShort, unsigned short);
HL_IN_DEFINE_ANIM_KEY_TYPE(Int, int);
HL_IN_DEFINE_ANIM_KEY_TYPE(UInt, unsigned int);
HL_IN_DEFINE_ANIM_KEY_TYPE(Index, size_t);

typedef enum HlAnimationKeyGroupFlags
{
    /* Masks */
    HL_ANIM_KEY_FORMAT_MASK = 0xFU,
    HL_ANIM_KEY_DIMENSION_MASK = 0x30U,
    HL_ANIM_KEY_TYPE_MASK = 0x3FFFFC0U,
    HL_ANIM_KEY_IP_TYPE_MASK = 0x1C000000U,
    HL_ANIM_KEY_LOOP_TYPE_MASK = 0xE0000000U,

    /* Flags */
    HL_ANIM_KEY_HAS_IP_DATA = 0x10000000U,

    /* Formats */
    HL_ANIM_KEY_FORMAT_IS_UNSIGNED = 1,
    HL_ANIM_KEY_FORMAT_FLOAT = 0,
    HL_ANIM_KEY_FORMAT_INDEX = HL_ANIM_KEY_FORMAT_IS_UNSIGNED,
    HL_ANIM_KEY_FORMAT_SHORT = 2,
    HL_ANIM_KEY_FORMAT_USHORT = (HL_ANIM_KEY_FORMAT_SHORT | HL_ANIM_KEY_FORMAT_IS_UNSIGNED),
    HL_ANIM_KEY_FORMAT_INT = 4,
    HL_ANIM_KEY_FORMAT_UINT = (HL_ANIM_KEY_FORMAT_INT | HL_ANIM_KEY_FORMAT_IS_UNSIGNED),

    /* Dimensions */
    HL_ANIM_KEY_DIMENSION_1D = 0,
    HL_ANIM_KEY_DIMENSION_2D = 0x10U,
    HL_ANIM_KEY_DIMENSION_3D = 0x20U,
    HL_ANIM_KEY_DIMENSION_4D = 0x30U,

    /* General types */
    HL_ANIM_KEY_TYPE_CUSTOM = 0,

    /* Bone types */
    HL_ANIM_KEY_BONE_TYPE_POS_X = 0x40U,
    HL_ANIM_KEY_BONE_TYPE_POS_Y = 0x80U,
    HL_ANIM_KEY_BONE_TYPE_POS_Z = 0x100U,

    HL_ANIM_KEY_BONE_TYPE_POS_XYZ = (HL_ANIM_KEY_BONE_TYPE_POS_X |
        HL_ANIM_KEY_BONE_TYPE_POS_Y | HL_ANIM_KEY_BONE_TYPE_POS_Z),

    HL_ANIM_KEY_BONE_TYPE_ROT_X = 0x200U,
    HL_ANIM_KEY_BONE_TYPE_ROT_Y = 0x400U,
    HL_ANIM_KEY_BONE_TYPE_ROT_Z = 0x800U,

    HL_ANIM_KEY_BONE_TYPE_ROT_XYZ = (HL_ANIM_KEY_BONE_TYPE_ROT_X |
        HL_ANIM_KEY_BONE_TYPE_ROT_Y | HL_ANIM_KEY_BONE_TYPE_ROT_Z),

    HL_ANIM_KEY_BONE_TYPE_ROT = 0x1000U,

    HL_ANIM_KEY_BONE_TYPE_SCALE_X = 0x2000U,
    HL_ANIM_KEY_BONE_TYPE_SCALE_Y = 0x4000U,
    HL_ANIM_KEY_BONE_TYPE_SCALE_Z = 0x8000U,

    HL_ANIM_KEY_BONE_TYPE_SCALE_XYZ = (HL_ANIM_KEY_BONE_TYPE_SCALE_X |
        HL_ANIM_KEY_BONE_TYPE_SCALE_Y | HL_ANIM_KEY_BONE_TYPE_SCALE_Z),

    HL_ANIM_KEY_BONE_TYPE_HIDE = 0x10000U,

    /* Camera types */
    HL_ANIM_KEY_CAM_TYPE_TARGET_X = 0x40U,
    HL_ANIM_KEY_CAM_TYPE_TARGET_Y = 0x80U,
    HL_ANIM_KEY_CAM_TYPE_TARGET_Z = 0x100U,

    HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ = (HL_ANIM_KEY_CAM_TYPE_TARGET_X |
        HL_ANIM_KEY_CAM_TYPE_TARGET_Y | HL_ANIM_KEY_CAM_TYPE_TARGET_Z),

    HL_ANIM_KEY_CAM_TYPE_UPTARGET_X = 0x200U,
    HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y = 0x400U,
    HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z = 0x800U,

    HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ = (HL_ANIM_KEY_CAM_TYPE_UPTARGET_X |
        HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y | HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z),

    HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X = 0x1000U,
    HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y = 0x2000U,
    HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z = 0x4000U,

    HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ = (HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X |
        HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y | HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z),

    HL_ANIM_KEY_CAM_TYPE_FOVY = 0x8000U,
    HL_ANIM_KEY_CAM_TYPE_ZNEAR = 0x10000U,
    HL_ANIM_KEY_CAM_TYPE_ZFAR = 0x20000U,
    HL_ANIM_KEY_CAM_TYPE_ASPECT = 0x40000U,
    HL_ANIM_KEY_CAM_TYPE_ROLL = 0x80000U,

    /* Light types */
    HL_ANIM_KEY_LIGHT_TYPE_COLOR_R = 0x40U,
    HL_ANIM_KEY_LIGHT_TYPE_COLOR_G = 0x80U,
    HL_ANIM_KEY_LIGHT_TYPE_COLOR_B = 0x100U,

    HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB = (HL_ANIM_KEY_LIGHT_TYPE_COLOR_R |
        HL_ANIM_KEY_LIGHT_TYPE_COLOR_G | HL_ANIM_KEY_LIGHT_TYPE_COLOR_B),

    HL_ANIM_KEY_LIGHT_TYPE_ALPHA = 0x200U,
    HL_ANIM_KEY_LIGHT_TYPE_INTENSITY = 0x400U,
    HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START = 0x800U,
    HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END = 0x1000U,
    HL_ANIM_KEY_LIGHT_TYPE_INNER_ANGLE = 0x2000U,
    HL_ANIM_KEY_LIGHT_TYPE_OUTER_ANGLE = 0x4000U,
    HL_ANIM_KEY_LIGHT_TYPE_INNER_RANGE = 0x8000U,
    HL_ANIM_KEY_LIGHT_TYPE_OUTER_RANGE = 0x10000U,

    /* Morph types */
    HL_ANIM_KEY_MORPH_TYPE_WEIGHT = 0x40U,

    /* Material types */
    HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R = 0x40U,
    HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G = 0x80U,
    HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B = 0x100U,
    HL_ANIM_KEY_MAT_TYPE_ALPHA = 0x200U,

    HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB = (HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R |
        HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G | HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B),

    HL_ANIM_KEY_MAT_TYPE_SPECULAR_R = 0x400U,
    HL_ANIM_KEY_MAT_TYPE_SPECULAR_G = 0x800U,
    HL_ANIM_KEY_MAT_TYPE_SPECULAR_B = 0x1000U,

    HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB = (HL_ANIM_KEY_MAT_TYPE_SPECULAR_R |
        HL_ANIM_KEY_MAT_TYPE_SPECULAR_G | HL_ANIM_KEY_MAT_TYPE_SPECULAR_B),

    HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL = 0x2000U,
    HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS = 0x4000U,

    HL_ANIM_KEY_MAT_TYPE_AMBIENT_R = 0x8000U,
    HL_ANIM_KEY_MAT_TYPE_AMBIENT_G = 0x10000U,
    HL_ANIM_KEY_MAT_TYPE_AMBIENT_B = 0x20000U,

    HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB = (HL_ANIM_KEY_MAT_TYPE_AMBIENT_R |
        HL_ANIM_KEY_MAT_TYPE_AMBIENT_G | HL_ANIM_KEY_MAT_TYPE_AMBIENT_B),

    HL_ANIM_KEY_MAT_TYPE_TEX_INDEX = 0x40000U,
    HL_ANIM_KEY_MAT_TYPE_TEX_BLEND = 0x80000U,

    HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U = 0x100000U,
    HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V = 0x200000U,
    HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV = (HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U |
        HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V), 
    
    HL_ANIM_KEY_MAT_TYPE_HIDE = 0x400000U,

    /* Interpolation types */
    HL_ANIM_KEY_IP_TYPE_SPLINE = 0,
    HL_ANIM_KEY_IP_TYPE_LINEAR = 0x4000000U,
    HL_ANIM_KEY_IP_TYPE_CONSTANT = 0x8000000U,
    HL_ANIM_KEY_IP_TYPE_TRIGGER = 0xC000000U,
    HL_ANIM_KEY_IP_TYPE_BEZIER = HL_ANIM_KEY_HAS_IP_DATA,
    HL_ANIM_KEY_IP_TYPE_SI_SPLINE = (0x4000000U | HL_ANIM_KEY_HAS_IP_DATA),
    
    /* Loop types */
    HL_ANIM_KEY_LOOP_TYPE_NOREPEAT = 0,
    HL_ANIM_KEY_LOOP_TYPE_CONSTREPEAT = 0x20000000U,
    HL_ANIM_KEY_LOOP_TYPE_REPEAT = 0x40000000U,
    HL_ANIM_KEY_LOOP_TYPE_MIRROR = 0x60000000U,
    HL_ANIM_KEY_LOOP_TYPE_OFFSET = 0x80000000U
}
HlAnimationKeyGroupFlags;

typedef struct HlAnimationKeyGroup
{
    HlAnimationKeyGroupFlags flagsAndType;
    float startFrame;
    float endFrame;
    float startKeyframe;
    float endKeyframe;
    /**
       @brief The index of the thing (texture/material/bone/etc.)
       being animated by this key group.
    */
    size_t index;
    /**
       @brief Optional subindex of the thing (texture/material/bone/etc.)
       being animated by this key group. For example, index might
       represent the index of a material, while subIndex represents a
       texture entry or parameter within that material, which is what
       this animation actually applies to.
    */
    size_t subIndex;
    size_t keyframeCount;
    void* keyframes;
}
HlAnimationKeyGroup;

typedef enum HlAnimationFlags
{
    /* Masks */
    HL_ANIM_TYPE_MASK = 0x1FFFFFFFU,
    HL_ANIM_LOOP_TYPE_MASK = 0xE0000000U,

    /* Types */
    HL_ANIM_TYPE_BONE = 1,
    HL_ANIM_TYPE_CAMERA = 2,
    HL_ANIM_TYPE_LIGHT = 3,
    HL_ANIM_TYPE_MORPH = 4,
    HL_ANIM_TYPE_MATERIAL = 5,

    /* Loop types */
    HL_ANIM_LOOP_TYPE_NOREPEAT = 0,
    HL_ANIM_LOOP_TYPE_CONSTREPEAT = 0x20000000U,
    HL_ANIM_LOOP_TYPE_REPEAT = 0x40000000U,
    HL_ANIM_LOOP_TYPE_MIRROR = 0x60000000U,
    HL_ANIM_LOOP_TYPE_OFFSET = 0x80000000U,
    HL_ANIM_LOOP_TYPE_TRIGGER = 0xA0000000U
}
HlAnimationFlags;

typedef struct HlAnimation
{
    /**
       @brief Various flags representing this animation's type, as
       well as various other properties; see HlAnimationFlags.
    */
    HlAnimationFlags flagsAndType;
    /** @brief How many frames of the animation are shown each second. */
    float framerate;
    /** @brief The frame this animation starts playing at. */
    float startFrame;
    /** @brief The frame this animation stops playing at. */
    float endFrame;
    /** @brief The number of HlAnimationKeyGroups in the keyGroups array. */
    size_t keyGroupCount;
    /** @brief Pointer to an array of HlAnimationKeyGroups. */
    HlAnimationKeyGroup* keyGroups;
}
HlAnimation;

HL_API const char* const hlAnimationKeyGroupGetFormatStr(const HlAnimationKeyGroup* keyGroup);
HL_API const char* const hlAnimationKeyGroupGetDimensionStr(const HlAnimationKeyGroup* keyGroup);
HL_API const char* const hlAnimationKeyGroupGetBoneTypeStr(const HlAnimationKeyGroup* keyGroup);
HL_API const char* const hlAnimationKeyGroupGetMaterialTypeStr(const HlAnimationKeyGroup* keyGroup);
HL_API const char* const hlAnimationKeyGroupGetIPTypeStr(const HlAnimationKeyGroup* keyGroup);
HL_API const char* const hlAnimationKeyGroupGetLoopTypeStr(const HlAnimationKeyGroup* keyGroup);

HL_API const char* const hlAnimationGetTypeStr(const HlAnimation* anim);
HL_API const char* const hlAnimationGetLoopTypeStr(const HlAnimation* anim);

HL_API HlAnimation* hlAnimationReadJSON(const HlBlob* blob);

HL_API HlResult hlAnimationWriteJSON(const HlAnimation* HL_RESTRICT anim,
    HlFile* HL_RESTRICT file);

HL_API HlAnimation* hlAnimationImportJSON(const HlNChar* filePath);

HL_API HlResult hlAnimationExportJSON(const HlAnimation* HL_RESTRICT anim,
    const HlNChar* HL_RESTRICT filePath);

#ifdef __cplusplus
}
#endif
#endif
