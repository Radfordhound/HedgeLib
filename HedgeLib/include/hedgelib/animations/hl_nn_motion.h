#ifndef HL_NN_MOTION_H_INCLUDED
#define HL_NN_MOTION_H_INCLUDED
#include "hl_animation.h"
#include "../io/hl_nn.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct HlNNMotionBezierHandle
{
    HlVector2 in;
    HlVector2 out;
}
HlNNMotionBezierHandle;

HL_STATIC_ASSERT_SIZE(HlNNMotionBezierHandle, 16);

typedef struct HlNNMotionSISplineHandle
{
    HlVector2 in;
    HlVector2 out;
}
HlNNMotionSISplineHandle;

HL_STATIC_ASSERT_SIZE(HlNNMotionSISplineHandle, 16);

typedef struct HlNNMotionKeyFloat
{
    float frame;
    float value;
}
HlNNMotionKeyFloat,
HlNNMotionKeyDiffuseR,
HlNNMotionKeyDiffuseG,
HlNNMotionKeyDiffuseB,
HlNNMotionKeyOffsetU,
HlNNMotionKeyOffsetV;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyFloat, 8);

typedef struct HlNNMotionKeyBezier
{
    float frame;
    float value;
    HlNNMotionBezierHandle bhandle;
}
HlNNMotionKeyBezier;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyBezier, 24);

typedef struct HlNNMotionKeySISpline
{
    float frame;
    float value;
    HlNNMotionSISplineHandle shandle;
}
HlNNMotionKeySISpline;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeySISpline, 24);

typedef struct HlNNMotionKeyTexCoord
{
    float frame;
    HlNNTexCoord value;
}
HlNNMotionKeyTexCoord,
HlNNMotionKeyOffsetUV;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyTexCoord, 12);

typedef struct HlNNMotionKeyVector
{
    float frame;
    HlVector3 value;
}
HlNNMotionKeyVector;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyVector, 16);

typedef struct HlNNMotionKeyRGB
{
    float frame;
    HlNNRGB value;
}
HlNNMotionKeyRGB;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyRGB, 16);

typedef struct HlNNMotionKeyQuaternion
{
    float frame;
    HlQuaternion value;
}
HlNNMotionKeyQuaternion;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyQuaternion, 20);

typedef struct HlNNMotionKeyInt
{
    float frame;
    HlS32 value;
}
HlNNMotionKeyInt;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyInt, 8);

typedef struct HlNNMotionKeyBezierInt
{
    float frame;
    HlS32 value;
    HlNNMotionBezierHandle bhandle;
}
HlNNMotionKeyBezierInt;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyBezierInt, 24);

typedef struct HlNNMotionKeySISplineInt
{
    float frame;
    HlS32 value;
    HlNNMotionSISplineHandle shandle;
}
HlNNMotionKeySISplineInt;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeySISplineInt, 24);

typedef struct HlNNMotionKeyUInt
{
    float frame;
    HlU32 value;
}
HlNNMotionKeyUInt;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyUInt, 8);

typedef struct HlNNMotionKeyRotateA32
{
    float frame;
    HlNNRotateA32 value;
}
HlNNMotionKeyRotateA32;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyRotateA32, 16);

typedef struct HlNNMotionKeyShort
{
    HlS16 frame;
    HlS16 value;
}
HlNNMotionKeyShort;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyShort, 4);

typedef struct HlNNMotionKeySISplineShort
{
    HlS16 frame;
    HlS16 value;
    HlNNMotionSISplineHandle shandle;
}
HlNNMotionKeySISplineShort;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeySISplineShort, 20);

typedef struct HlNNMotionKeyRotateA16
{
    HlS16 frame;
    HlNNRotateA16 value;
}
HlNNMotionKeyRotateA16;

HL_STATIC_ASSERT_SIZE(HlNNMotionKeyRotateA16, 8);

typedef union HlNNMotionKey
{
    HlNNMotionKeyFloat typeFloat;
    HlNNMotionKeyBezier typeBezier;
    HlNNMotionKeySISpline typeSpline;
    HlNNMotionKeyTexCoord typeTexCoord;
    HlNNMotionKeyVector typeVector;
    HlNNMotionKeyRGB typeRGB;
    HlNNMotionKeyQuaternion typeQuaternion;
    HlNNMotionKeyInt typeInt;
    HlNNMotionKeyBezierInt typeBezierInt;
    HlNNMotionKeySISplineInt typeSplineInt;
    /* TODO: There was another motion key type here that seems to be the same as typeInt. An enum? */
    HlNNMotionKeyUInt typeUInt;
    HlNNMotionKeyRotateA32 typeRotA32;
    HlNNMotionKeyShort typeShort;
    HlNNMotionKeySISplineShort typeSplineShort;
    HlNNMotionKeyRotateA16 typeRotA16;
}
HlNNMotionKey;

HL_STATIC_ASSERT_SIZE(HlNNMotionKey, 24);

typedef enum HlNNSubMotionType
{
    /* Masks */
    HL_NN_SMOTTYPE_FRAME_MASK = 3,
    HL_NN_SMOTTYPE_ANGLE_MASK = 28,
    HL_NN_SMOTTYPE_TRANSLATION_MASK = 0x700U,
    HL_NN_SMOTTYPE_DIFFUSE_MASK = 0xE00U,
    HL_NN_SMOTTYPE_ROTATION_MASK = 0x7800U,
    HL_NN_SMOTTYPE_SPECULAR_MASK = 0xE000U,
    HL_NN_SMOTTYPE_SCALING_MASK = 0x38000U,
    HL_NN_SMOTTYPE_USER_MASK = 0xC0000U,
    HL_NN_SMOTTYPE_LIGHT_COLOR_MASK = 0xE00000U,
    HL_NN_SMOTTYPE_OFFSET_MASK = 0x1800000U,
    HL_NN_SMOTTYPE_TARGET_MASK = 0x1C0000U,
    HL_NN_SMOTTYPE_AMBIENT_MASK = 0x1C0000U,
    HL_NN_SMOTTYPE_UPTARGET_MASK = 0x1C00000U,
    HL_NN_SMOTTYPE_TEXTURE_MASK = 0x1E00000U,
    HL_NN_SMOTTYPE_UPVECTOR_MASK = 0xE000000U,
    HL_NN_SMOTTYPE_VALUETYPE_MASK = 0xFFFFFF00U,

    /* Frame types */
    HL_NN_SMOTTYPE_FRAME_FLOAT = 1,
    HL_NN_SMOTTYPE_FRAME_SINT16 = 2,

    /* Angle types */
    HL_NN_SMOTTYPE_ANGLE_RADIAN = 4,
    HL_NN_SMOTTYPE_ANGLE_ANGLE32 = 8,
    HL_NN_SMOTTYPE_ANGLE_ANGLE16 = 16,
    
    /* Node types */
    HL_NN_SMOTTYPE_TRANSLATION_X = 0x100U,
    HL_NN_SMOTTYPE_TRANSLATION_Y = 0x200U,
    HL_NN_SMOTTYPE_TRANSLATION_Z = 0x400U,

    HL_NN_SMOTTYPE_TRANSLATION_XYZ = (HL_NN_SMOTTYPE_TRANSLATION_X |
        HL_NN_SMOTTYPE_TRANSLATION_Y | HL_NN_SMOTTYPE_TRANSLATION_Z),

    HL_NN_SMOTTYPE_ROTATION_X = 0x800U,
    HL_NN_SMOTTYPE_ROTATION_Y = 0x1000U,
    HL_NN_SMOTTYPE_ROTATION_Z = 0x2000U,

    HL_NN_SMOTTYPE_ROTATION_XYZ = (HL_NN_SMOTTYPE_ROTATION_X |
        HL_NN_SMOTTYPE_ROTATION_Y | HL_NN_SMOTTYPE_ROTATION_Z),

    HL_NN_SMOTTYPE_QUATERNION = 0x4000U,

    HL_NN_SMOTTYPE_SCALING_X = 0x8000U,
    HL_NN_SMOTTYPE_SCALING_Y = 0x10000U,
    HL_NN_SMOTTYPE_SCALING_Z = 0x20000U,

    HL_NN_SMOTTYPE_SCALING_XYZ = (HL_NN_SMOTTYPE_SCALING_X |
        HL_NN_SMOTTYPE_SCALING_Y | HL_NN_SMOTTYPE_SCALING_Z),

    HL_NN_SMOTTYPE_USER_UINT32 = 0x40000U,
    HL_NN_SMOTTYPE_USER_FLOAT = 0x80000U,

    HL_NN_SMOTTYPE_NODEHIDE = 0x100000U,

    /* Camera types */
    HL_NN_SMOTTYPE_TARGET_X = 0x40000U,
    HL_NN_SMOTTYPE_TARGET_Y = 0x80000U,
    HL_NN_SMOTTYPE_TARGET_Z = 0x100000U,

    HL_NN_SMOTTYPE_TARGET_XYZ = (HL_NN_SMOTTYPE_TARGET_X |
        HL_NN_SMOTTYPE_TARGET_Y | HL_NN_SMOTTYPE_TARGET_Z),

    HL_NN_SMOTTYPE_ROLL = 0x200000U,
    HL_NN_SMOTTYPE_UPTARGET_X = 0x400000U,
    HL_NN_SMOTTYPE_UPTARGET_Y = 0x800000U,
    HL_NN_SMOTTYPE_UPTARGET_Z = 0x1000000U,

    HL_NN_SMOTTYPE_UPTARGET_XYZ = (HL_NN_SMOTTYPE_UPTARGET_X |
        HL_NN_SMOTTYPE_UPTARGET_Y | HL_NN_SMOTTYPE_UPTARGET_Z),

    HL_NN_SMOTTYPE_UPVECTOR_X = 0x2000000U,
    HL_NN_SMOTTYPE_UPVECTOR_Y = 0x4000000U,
    HL_NN_SMOTTYPE_UPVECTOR_Z = 0x8000000U,

    HL_NN_SMOTTYPE_UPVECTOR_XYZ = (HL_NN_SMOTTYPE_UPVECTOR_X |
        HL_NN_SMOTTYPE_UPVECTOR_Y | HL_NN_SMOTTYPE_UPVECTOR_Z),

    HL_NN_SMOTTYPE_FOVY = 0x10000000U,
    HL_NN_SMOTTYPE_ZNEAR = 0x20000000U,
    HL_NN_SMOTTYPE_ZFAR = 0x40000000U,
    HL_NN_SMOTTYPE_ASPECT = 0x80000000U,

    /* Light types */
    HL_NN_SMOTTYPE_LIGHT_COLOR_R = 0x200000U,
    HL_NN_SMOTTYPE_LIGHT_COLOR_G = 0x400000U,
    HL_NN_SMOTTYPE_LIGHT_COLOR_B = 0x800000U,

    HL_NN_SMOTTYPE_LIGHT_COLOR_RGB = (HL_NN_SMOTTYPE_LIGHT_COLOR_R |
        HL_NN_SMOTTYPE_LIGHT_COLOR_G | HL_NN_SMOTTYPE_LIGHT_COLOR_B),

    HL_NN_SMOTTYPE_LIGHT_ALPHA = 0x1000000U,
    HL_NN_SMOTTYPE_LIGHT_INTENSITY = 0x2000000U,

    HL_NN_SMOTTYPE_FALLOFF_START = 0x4000000U,
    HL_NN_SMOTTYPE_FALLOFF_END = 0x8000000U,

    HL_NN_SMOTTYPE_INNER_ANGLE = 0x10000000U,
    HL_NN_SMOTTYPE_OUTER_ANGLE = 0x20000000U,
    HL_NN_SMOTTYPE_INNER_RANGE = 0x40000000U,
    HL_NN_SMOTTYPE_OUTER_RANGE = 0x80000000U,

    /* Morph types */
    HL_NN_SMOTTYPE_MORPH_WEIGHT = 0x1000000U,

    /* Material types */
    HL_NN_SMOTTYPE_HIDE = 0x100U,

    HL_NN_SMOTTYPE_DIFFUSE_R = 0x200U,
    HL_NN_SMOTTYPE_DIFFUSE_G = 0x400U,
    HL_NN_SMOTTYPE_DIFFUSE_B = 0x800U,

    HL_NN_SMOTTYPE_DIFFUSE_RGB = (HL_NN_SMOTTYPE_DIFFUSE_R |
        HL_NN_SMOTTYPE_DIFFUSE_G | HL_NN_SMOTTYPE_DIFFUSE_B),

    HL_NN_SMOTTYPE_ALPHA = 0x1000U,

    HL_NN_SMOTTYPE_SPECULAR_R = 0x2000U,
    HL_NN_SMOTTYPE_SPECULAR_G = 0x4000U,
    HL_NN_SMOTTYPE_SPECULAR_B = 0x8000U,

    HL_NN_SMOTTYPE_SPECULAR_RGB = (HL_NN_SMOTTYPE_SPECULAR_R |
        HL_NN_SMOTTYPE_SPECULAR_G | HL_NN_SMOTTYPE_SPECULAR_B),

    HL_NN_SMOTTYPE_SPECULAR_LEVEL = 0x10000U,
    HL_NN_SMOTTYPE_SPECULAR_GLOSS = 0x20000U,

    HL_NN_SMOTTYPE_AMBIENT_R = 0x40000U,
    HL_NN_SMOTTYPE_AMBIENT_G = 0x80000U,
    HL_NN_SMOTTYPE_AMBIENT_B = 0x100000U,

    HL_NN_SMOTTYPE_AMBIENT_RGB = (HL_NN_SMOTTYPE_AMBIENT_R |
        HL_NN_SMOTTYPE_AMBIENT_G | HL_NN_SMOTTYPE_AMBIENT_B),

    HL_NN_SMOTTYPE_TEXTURE_INDEX = 0x200000U,
    HL_NN_SMOTTYPE_TEXTURE_BLEND = 0x400000U,

    HL_NN_SMOTTYPE_OFFSET_U = 0x800000U,
    HL_NN_SMOTTYPE_OFFSET_V = 0x1000000U,
    HL_NN_SMOTTYPE_OFFSET_UV = (HL_NN_SMOTTYPE_OFFSET_U | HL_NN_SMOTTYPE_OFFSET_V),

    HL_NN_SMOTTYPE_MATCLBK_USER = 0x2000000U
}
HlNNSubMotionType;

typedef enum HlNNSubMotionIPType
{
    /* Masks */
    HL_NN_SMOTIPTYPE_IP_MASK = 0xE77U,
    HL_NN_SMOTIPTYPE_REPEAT_MASK = 0x1F0000U,

    /* Repeat types */
    HL_NN_SMOTIPTYPE_NOREPEAT = 0x10000U,
    HL_NN_SMOTIPTYPE_CONSTREPEAT = 0x20000U,
    HL_NN_SMOTIPTYPE_REPEAT = 0x40000U,
    HL_NN_SMOTIPTYPE_MIRROR = 0x80000U,
    HL_NN_SMOTIPTYPE_OFFSET = 0x100000U,

    /* Interpolation types */
    HL_NN_SMOTIPTYPE_SPLINE = 1,
    HL_NN_SMOTIPTYPE_LINEAR = 2,
    HL_NN_SMOTIPTYPE_CONSTANT = 4,
    HL_NN_SMOTIPTYPE_BEZIER = 16,
    HL_NN_SMOTIPTYPE_SI_SPLINE = 32,
    HL_NN_SMOTIPTYPE_TRIGGER = 64,
    HL_NN_SMOTIPTYPE_QUAT_LERP = 512,
    HL_NN_SMOTIPTYPE_QUAT_SLERP = 1024,
    HL_NN_SMOTIPTYPE_QUAT_SQUAD = 2048
}
HlNNSubMotionIPType;

typedef struct HlNNSubMotion
{
    /**
       @brief Various flags representing this submotion's type, as
       well as various other properties; see HlNNSubMotionType.
    */
    HlU32 type;
    /**
       @brief Various flags representing how this submotion
       is interpolated; see HlNNSubMotionIPType.
    */
    HlU32 ipType;
    /**
       @brief The index of the thing (bone, material, etc.) being animated.
       For material submotions, this value is actually comprised of two HlS16s:
       One which represents the material index, and one which represents the
       texture index.

       TODO: Is this information correct?
    */
    HlS32 id;
    /** @brief The frame this submotion starts playing at. */
    float startFrame;
    /** @brief The frame this submotion stops playing at. */
    float endFrame;
    /** @brief The first frame that has a keyframe assigned to it within this submotion. */
    float startKeyframe;
    /** @brief The last frame that has a keyframe assigned to it within this submotion. */
    float endKeyframe;
    /**
       @brief The number of HlNNMotionKeyX (where X is type)
       structs in the array pointed to by subMotions.
    */
    HlU32 keyframeCount;
    /** @brief The size of each individual key within the keyframes array. */
    HlU32 keySize;
    /** @brief Pointer to an array of HlNNMotionKeyX (where X is type) structs. */
    HL_OFF32(HlNNMotionKey) keyframes;
}
HlNNSubMotion;

HL_STATIC_ASSERT_SIZE(HlNNSubMotion, 0x28);

typedef enum HlNNMotionType
{
    /* Masks */
    HL_NN_MOTIONTYPE_CATEGORY_MASK = 31,
    HL_NN_MOTIONTYPE_REPEAT_MASK = 0x1F0040U,

    /* Flags */
    HL_NN_MOTIONTYPE_VERSION2 = 0x10000000U,

    /* Repeat types */
    HL_NN_MOTIONTYPE_TRIGGER = 64,
    HL_NN_MOTIONTYPE_NOREPEAT = 0x10000U,
    HL_NN_MOTIONTYPE_CONSTREPEAT = 0x20000U,
    HL_NN_MOTIONTYPE_REPEAT = 0x40000U,
    HL_NN_MOTIONTYPE_MIRROR = 0x80000U,
    HL_NN_MOTIONTYPE_OFFSET = 0x100000U,

    /* Motion types */
    HL_NN_MOTIONTYPE_NODE = 1,
    HL_NN_MOTIONTYPE_CAMERA = 2,
    HL_NN_MOTIONTYPE_LIGHT = 4,
    HL_NN_MOTIONTYPE_MORPH = 8,
    HL_NN_MOTIONTYPE_MATERIAL = 16
}
HlNNMotionType;

typedef struct HlNNMotion /* NNS_MOTION */
{
    /**
       @brief Various flags representing this motion's type, as
       well as various other properties; see HlNNMotionType.
    */
    HlU32 type;
    /** @brief The frame this motion starts playing at. */
    float startFrame;
    /** @brief The frame this motion stops playing at. */
    float endFrame;
    /** @brief The number of HlNNSubMotion structs in the array pointed to by subMotions. */
    HlU32 subMotionCount;
    /** @brief Pointer to an array of HlNNSubMotion structs. */
    HL_OFF32(HlNNSubMotion) subMotions;
    /** @brief How many frames of the motion are shown each second. */
    float framerate;
    /** @brief Reserved value that was never used; always 0. */
    HlU32 reserved0;
    /** @brief Reserved value that was never used; always 0. */
    HlU32 reserved1;
}
HlNNMotion;

HL_STATIC_ASSERT_SIZE(HlNNMotion, 0x20);

HL_API void hlNNMotionBezierHandleSwap(HlNNMotionBezierHandle* handle);
HL_API void hlNNMotionSISplineHandleSwap(HlNNMotionSISplineHandle* handle);

HL_API void hlNNMotionKeyFloatSwap(HlNNMotionKeyFloat* key);
HL_API void hlNNMotionKeyBezierSwap(HlNNMotionKeyBezier* key);
HL_API void hlNNMotionKeySISplineSwap(HlNNMotionKeySISpline* key);
HL_API void hlNNMotionKeyTexCoordSwap(HlNNMotionKeyTexCoord* key);
HL_API void hlNNMotionKeyVectorSwap(HlNNMotionKeyVector* key);
HL_API void hlNNMotionKeyRGBSwap(HlNNMotionKeyRGB* key);
HL_API void hlNNMotionKeyQuaternionSwap(HlNNMotionKeyQuaternion* key);
HL_API void hlNNMotionKeyIntSwap(HlNNMotionKeyInt* key);
HL_API void hlNNMotionKeyBezierIntSwap(HlNNMotionKeyBezierInt* key);
HL_API void hlNNMotionKeySISplineIntSwap(HlNNMotionKeySISplineInt* key);
HL_API void hlNNMotionKeyUIntSwap(HlNNMotionKeyUInt* key);
HL_API void hlNNMotionKeyRotateA32Swap(HlNNMotionKeyRotateA32* key);
HL_API void hlNNMotionKeyShortSwap(HlNNMotionKeyShort* key);
HL_API void hlNNMotionKeySISplineShortSwap(HlNNMotionKeySISplineShort* key);
HL_API void hlNNMotionKeyRotateA16Swap(HlNNMotionKeyRotateA16* key);

HL_API void hlNNSubMotionSwapKeys(HlNNSubMotion* subMot, HlU32 motType);
HL_API void hlNNSubMotionSwap(HlNNSubMotion* subMot, HlBool swapOffsets);
HL_API void hlNNMotionSwap(HlNNMotion* motion, HlBool swapOffsets);
HL_API void hlNNMotionFix(HlNNBinCnkDataHeader* dataHeader);

HL_API HlResult hlNNMotionParse(const HlNNMotion* HL_RESTRICT motion,
    HlAnimation* HL_RESTRICT * HL_RESTRICT hlAnim);

HL_API HlResult hlNNMotionRead(HlBlob* HL_RESTRICT blob,
    HlAnimation* HL_RESTRICT * HL_RESTRICT hlAnim);

HL_API HlResult hlNNMotionWrite(const HlAnimation* HL_RESTRICT anim, size_t dataPos,
    HlNNPlatform platform, HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file);

#ifdef __cplusplus
}
#endif
#endif
