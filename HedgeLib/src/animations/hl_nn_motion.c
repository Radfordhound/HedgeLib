#include "hedgelib/hl_blob.h"
#include "hedgelib/hl_memory.h"
#include "hedgelib/hl_endian.h"
#include "hedgelib/animations/hl_nn_motion.h"
#include "hedgelib/animations/hl_animation.h"

void hlNNMotionBezierHandleSwap(HlNNMotionBezierHandle* handle)
{
    hlVector2Swap(&handle->in);
    hlVector2Swap(&handle->out);
}

void hlNNMotionSISplineHandleSwap(HlNNMotionSISplineHandle* handle)
{
    hlVector2Swap(&handle->in);
    hlVector2Swap(&handle->out);
}

void hlNNMotionKeyFloatSwap(HlNNMotionKeyFloat* key)
{
    hlSwapFloatP(&key->frame);
    hlSwapFloatP(&key->value);
}

void hlNNMotionKeyBezierSwap(HlNNMotionKeyBezier* key)
{
    hlSwapFloatP(&key->frame);
    hlSwapFloatP(&key->value);
    hlNNMotionBezierHandleSwap(&key->bhandle);
}

void hlNNMotionKeySISplineSwap(HlNNMotionKeySISpline* key)
{
    hlSwapFloatP(&key->frame);
    hlSwapFloatP(&key->value);
    hlNNMotionSISplineHandleSwap(&key->shandle);
}

void hlNNMotionKeyTexCoordSwap(HlNNMotionKeyTexCoord* key)
{
    hlSwapFloatP(&key->frame);
    hlNNTexCoordSwap(&key->value);
}

void hlNNMotionKeyVectorSwap(HlNNMotionKeyVector* key)
{
    hlSwapFloatP(&key->frame);
    hlVector3Swap(&key->value);
}

void hlNNMotionKeyRGBSwap(HlNNMotionKeyRGB* key)
{
    hlSwapFloatP(&key->frame);
    hlNNRGBSwap(&key->value);
}

void hlNNMotionKeyQuaternionSwap(HlNNMotionKeyQuaternion* key)
{
    hlSwapFloatP(&key->frame);
    hlVector4Swap(&key->value);
}

void hlNNMotionKeyIntSwap(HlNNMotionKeyInt* key)
{
    hlSwapFloatP(&key->frame);
    hlSwapS32P(&key->value);
}

void hlNNMotionKeyBezierIntSwap(HlNNMotionKeyBezierInt* key)
{
    /* TODO */
}

void hlNNMotionKeySISplineIntSwap(HlNNMotionKeySISplineInt* key)
{
    /* TODO */
}

void hlNNMotionKeyUIntSwap(HlNNMotionKeyUInt* key)
{
    hlSwapFloatP(&key->frame);
    hlSwapU32P(&key->value);
}

void hlNNMotionKeyRotateA32Swap(HlNNMotionKeyRotateA32* key)
{
    hlSwapFloatP(&key->frame);
    hlNNRotateA32Swap(&key->value);
}

void hlNNMotionKeyShortSwap(HlNNMotionKeyShort* key)
{
    hlSwapS16P(&key->frame);
    hlSwapS16P(&key->value);
}

void hlNNMotionKeySISplineShortSwap(HlNNMotionKeySISplineShort* key)
{
    /* TODO */
}

void hlNNMotionKeyRotateA16Swap(HlNNMotionKeyRotateA16* key)
{
    hlSwapS16P(&key->frame);
    hlNNRotateA16Swap(&key->value);
}

#define hlINNNSubMotionSwapKeysCaseBody(typeSuffix) {\
    HlNNMotionKey##typeSuffix* keys = (HlNNMotionKey##typeSuffix*)hlOff32Get(&subMot->keyframes);\
    size_t i;\
    for (i = 0; i < subMot->keyframeCount; ++i) {\
        hlNNMotionKey##typeSuffix##Swap(&keys[i]);\
    }\
    break;\
}

static void hlINNNSubMotionNodeSwapKeys(HlNNSubMotion* subMot)
{
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_SUBMOTION_NODE_IP_AUTOGEN(nnSubMotFormat, nnSubMotTypeSuffix, hlSubMotTypeSuffix)\
    case nnSubMotFormat: hlINNNSubMotionSwapKeysCaseBody(nnSubMotTypeSuffix)

#include "hl_in_nn_motion_autogen.h"

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    hlINNNSubMotionSwapKeysCaseBody(SISpline)

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    hlINNNSubMotionSwapKeysCaseBody(SISplineInt)
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                if ((subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) ==
                    HL_NN_SMOTTYPE_ANGLE_ANGLE16)
                {
                    hlINNNSubMotionSwapKeysCaseBody(SISplineShort)
                }
                break;
            }
            break;

        /* TODO: Swap HL_NN_SMOTTYPE_QUATERNION */
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_SUBMOTION_NODE_AUTOGEN(nnSubMotFormat, nnSuffix, hlSuffix, hlFormat)\
    case nnSubMotFormat: hlINNNSubMotionSwapKeysCaseBody(nnSuffix)

#include "hl_in_nn_motion_autogen.h"

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    hlINNNSubMotionSwapKeysCaseBody(Float)

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    hlINNNSubMotionSwapKeysCaseBody(Int)
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                if ((subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) ==
                    HL_NN_SMOTTYPE_ANGLE_ANGLE16)
                {
                    hlINNNSubMotionSwapKeysCaseBody(Short)
                }
                break;
            }
            break;

        case HL_NN_SMOTTYPE_ROTATION_XYZ:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    hlINNNSubMotionSwapKeysCaseBody(Vector)

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    hlINNNSubMotionSwapKeysCaseBody(RotateA32)
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                if ((subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) ==
                    HL_NN_SMOTTYPE_ANGLE_ANGLE16)
                {
                    hlINNNSubMotionSwapKeysCaseBody(RotateA16)
                }
                break;
            }
            break;

        /* TODO: Swap HL_NN_SMOTTYPE_QUATERNION */
        }
        break;
    }
}

static void hlINNNSubMotionCameraSwapKeys(HlNNSubMotion* subMot)
{
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            hlINNNSubMotionSwapKeysCaseBody(Bezier)

        /* TODO: Swap HL_NN_SMOTTYPE_ROLL */
        }
        break;

    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            hlINNNSubMotionSwapKeysCaseBody(SISpline)

        /* TODO: Swap HL_NN_SMOTTYPE_ROLL */
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            hlINNNSubMotionSwapKeysCaseBody(Float)

        case HL_NN_SMOTTYPE_TARGET_XYZ:
        case HL_NN_SMOTTYPE_UPTARGET_XYZ:
        case HL_NN_SMOTTYPE_UPVECTOR_XYZ:
            hlINNNSubMotionSwapKeysCaseBody(Vector)

        /* TODO: Swap HL_NN_SMOTTYPE_ROLL */
        }
        break;
    }
}

static void hlINNNSubMotionLightSwapKeys(HlNNSubMotion* subMot)
{
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            hlINNNSubMotionSwapKeysCaseBody(Bezier)

        /* TODO: Swap HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_RANGE */
        }
        break;

    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            hlINNNSubMotionSwapKeysCaseBody(SISpline)

        /* TODO: Swap HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_RANGE */
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            hlINNNSubMotionSwapKeysCaseBody(Float)

        case HL_NN_SMOTTYPE_LIGHT_COLOR_RGB:
            hlINNNSubMotionSwapKeysCaseBody(RGB)

        /* TODO: Swap HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Swap HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Swap HL_NN_SMOTTYPE_OUTER_RANGE */
        }
        break;
    }
}

static void hlINNNSubMotionMorphSwapKeys(HlNNSubMotion* subMot)
{
    if ((subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK) !=
        HL_NN_SMOTTYPE_MORPH_WEIGHT) return;

    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
        case HL_NN_SMOTIPTYPE_BEZIER:
            hlINNNSubMotionSwapKeysCaseBody(Bezier)

        case HL_NN_SMOTIPTYPE_SI_SPLINE:
            hlINNNSubMotionSwapKeysCaseBody(SISpline)

        default:
            hlINNNSubMotionSwapKeysCaseBody(Float)
    }
}

static void hlINNNSubMotionMaterialSwapKeys(HlNNSubMotion* subMot)
{
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            hlINNNSubMotionSwapKeysCaseBody(Bezier)

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            hlINNNSubMotionSwapKeysCaseBody(BezierInt)

        /* TODO: Swap HL_NN_SMOTTYPE_HIDE */
        }
        break;

    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            hlINNNSubMotionSwapKeysCaseBody(SISpline)

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            hlINNNSubMotionSwapKeysCaseBody(SISplineInt)

        /* TODO: Swap HL_NN_SMOTTYPE_HIDE */
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            hlINNNSubMotionSwapKeysCaseBody(Float)

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            hlINNNSubMotionSwapKeysCaseBody(Int)

        case HL_NN_SMOTTYPE_MATCLBK_USER:
            hlINNNSubMotionSwapKeysCaseBody(UInt)

        case HL_NN_SMOTTYPE_OFFSET_UV:
            hlINNNSubMotionSwapKeysCaseBody(TexCoord)

        case HL_NN_SMOTTYPE_DIFFUSE_RGB:
        case HL_NN_SMOTTYPE_SPECULAR_RGB:
        case HL_NN_SMOTTYPE_AMBIENT_RGB:
            hlINNNSubMotionSwapKeysCaseBody(RGB)

        /* TODO: Swap HL_NN_SMOTTYPE_HIDE */
        }
        break;
    }
}

void hlNNSubMotionSwapKeys(HlNNSubMotion* subMot, HlU32 motType)
{
    /* Compute required size to convert this submotion and all of its keys. */
    switch (motType & HL_NN_MOTIONTYPE_CATEGORY_MASK)
    {
/* Auto-generate this switch's body. */
#define HL_IN_NN_MOTION_TYPE_AUTOGEN(nnMotType, nnMotTypeSuffix, hlMotType)\
    case nnMotType: hlINNNSubMotion##nnMotTypeSuffix##SwapKeys(subMot); break;

#include "hl_in_nn_motion_autogen.h"
    }
}

void hlNNSubMotionSwap(HlNNSubMotion* subMot, HlBool swapOffsets)
{
    hlSwapU32P(&subMot->type);
    hlSwapU32P(&subMot->ipType);
    hlSwapS32P(&subMot->id);
    hlSwapFloatP(&subMot->startFrame);
    hlSwapFloatP(&subMot->endFrame);
    hlSwapFloatP(&subMot->startKeyframe);
    hlSwapFloatP(&subMot->endKeyframe);
    hlSwapU32P(&subMot->keyframeCount);
    hlSwapU32P(&subMot->keySize);
    if (swapOffsets) hlSwapU32P(&subMot->keyframes);
}

void hlNNMotionSwap(HlNNMotion* motion, HlBool swapOffsets)
{
    hlSwapU32P(&motion->type);
    hlSwapFloatP(&motion->startFrame);
    hlSwapFloatP(&motion->endFrame);
    hlSwapU32P(&motion->subMotionCount);
    if (swapOffsets) hlSwapU32P(&motion->subMotions);
    hlSwapFloatP(&motion->framerate);
    hlSwapU32P(&motion->reserved0);
    hlSwapU32P(&motion->reserved1);
}

void hlNNMotionFix(HlNNBinCnkDataHeader* dataHeader)
{
    /* Get platform ID from chunk ID. */
    HlNNPlatform platform = hlNNGetPlatform(dataHeader->id);

    /* Endian swap if necessary for the given platform. */
    if (hlNNPlatformNeedsSwap(platform))
    {
        HlNNMotion* motion;
        HlNNSubMotion* subMots;
        HlU32 i;

        /* Swap NN Motion. */
        motion = (HlNNMotion*)hlOff32Get(&dataHeader->mainDataOffset);
        hlNNMotionSwap(motion, HL_FALSE);

        /* Get NN SubMotions pointer. */
        subMots = (HlNNSubMotion*)hlOff32Get(&motion->subMotions);

        /* Swap NN SubMotions and their keyframes. */
        for (i = 0; i < motion->subMotionCount; ++i)
        {
            /* Swap NN SubMotion. */
            hlNNSubMotionSwap(&subMots[i], 0);

            /* Swap NN keyframes. */
            hlNNSubMotionSwapKeys(&subMots[i], motion->type);
        }
    }
}

static size_t hlINNNSubMotionNodeComputeReqSize(const HlNNSubMotion* subMot)
{
    /* Compute required size to convert this submotion and all of its keys. */
    size_t reqKeySize = 0;
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_SUBMOTION_NODE_IP_AUTOGEN(nnSubMotFormat, nnSubMotTypeSuffix, hlSubMotTypeSuffix)\
    case nnSubMotFormat: reqKeySize = sizeof(HlAnimationKey##hlSubMotTypeSuffix); break;

#include "hl_in_nn_motion_autogen.h"

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    reqKeySize = sizeof(HlAnimationKeyFloatIP);
                    break;

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    reqKeySize = sizeof(HlAnimationKeyIntIP);
                    break;
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                if ((subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) ==
                    HL_NN_SMOTTYPE_ANGLE_ANGLE16)
                {
                    reqKeySize = sizeof(HlAnimationKeyShortIP);
                    break;
                }
                break;
            }
            break;

        case HL_NN_SMOTTYPE_ROTATION_XYZ:
            reqKeySize =
                ((subMot->type & HL_NN_SMOTTYPE_FRAME_MASK) == HL_NN_SMOTTYPE_FRAME_FLOAT &&
                (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) == HL_NN_SMOTTYPE_ANGLE_RADIAN) ?
                sizeof(HlAnimationKeyFloatIP3) : 0;

            break;

        /* TODO: Parse HL_NN_SMOTTYPE_QUATERNION */
        }

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_SUBMOTION_NODE_AUTOGEN(nnSubMotFormat, nnSuffix, hlSuffix, hlFormat)\
    case nnSubMotFormat: reqKeySize = sizeof(HlAnimationKey##hlSuffix); break;

#include "hl_in_nn_motion_autogen.h"

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    reqKeySize = sizeof(HlAnimationKeyFloat);
                    break;

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    reqKeySize = sizeof(HlAnimationKeyInt);
                    break;
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                if ((subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK) ==
                    HL_NN_SMOTTYPE_ANGLE_ANGLE16)
                {
                    reqKeySize = sizeof(HlAnimationKeyShort);
                    break;
                }
                break;
            }
            break;

        case HL_NN_SMOTTYPE_ROTATION_XYZ:
            switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
            {
            case HL_NN_SMOTTYPE_FRAME_FLOAT:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                    reqKeySize = sizeof(HlAnimationKeyFloat3);
                    break;

                case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                    reqKeySize = sizeof(HlAnimationKeyInt3);
                    break;
                }
                break;

            case HL_NN_SMOTTYPE_FRAME_SINT16:
                switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
                {
                case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
                    reqKeySize = sizeof(HlAnimationKeyShort3);
                    break;
                }
                break;
            }
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_QUATERNION */
        }
        break;
    }

    return sizeof(HlAnimationKeyGroup) +        /* Required size for converted submotion. */
        (reqKeySize * subMot->keyframeCount);   /* Required size for all converted keyframes. */
}

static size_t hlINNNSubMotionCameraComputeReqSize(const HlNNSubMotion* subMot)
{
    /* Compute required size to convert this submotion and all of its keys. */
    size_t reqKeySize;
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            reqKeySize = sizeof(HlAnimationKeyFloatIP);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_ROLL */

        default:
            reqKeySize = 0;
            break;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            reqKeySize = sizeof(HlAnimationKeyFloat);
            break;

        case HL_NN_SMOTTYPE_TARGET_XYZ:
        case HL_NN_SMOTTYPE_UPTARGET_XYZ:
        case HL_NN_SMOTTYPE_UPVECTOR_XYZ:
            reqKeySize = sizeof(HlAnimationKeyFloat3);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_ROLL */

        default:
            reqKeySize = 0;
            break;
        }
        break;
    }

    return sizeof(HlAnimationKeyGroup) +        /* Required size for converted submotion. */
        (reqKeySize * subMot->keyframeCount);   /* Required size for all converted keyframes. */
}

static size_t hlINNNSubMotionLightComputeReqSize(const HlNNSubMotion* subMot)
{
    /* Compute required size to convert this submotion and all of its keys. */
    size_t reqKeySize;
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            reqKeySize = sizeof(HlAnimationKeyFloatIP);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_RANGE */

        default:
            reqKeySize = 0;
            break;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            reqKeySize = sizeof(HlAnimationKeyFloat);
            break;

        case HL_NN_SMOTTYPE_LIGHT_COLOR_RGB:
            reqKeySize = sizeof(HlAnimationKeyFloat3);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_RANGE */

        default:
            reqKeySize = 0;
            break;
        }
        break;
    }

    return sizeof(HlAnimationKeyGroup) +        /* Required size for converted submotion. */
        (reqKeySize * subMot->keyframeCount);   /* Required size for all converted keyframes. */
}

static size_t hlINNNSubMotionMorphComputeReqSize(const HlNNSubMotion* subMot)
{
    /* Compute required size to convert this submotion and all of its keys. */
    size_t reqKeySize;

    if ((subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK) == HL_NN_SMOTTYPE_MORPH_WEIGHT)
    {
        switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
        {
        case HL_NN_SMOTIPTYPE_BEZIER:
        case HL_NN_SMOTIPTYPE_SI_SPLINE:
            reqKeySize = sizeof(HlAnimationKeyFloatIP);
            break;

        default:
            reqKeySize = sizeof(HlAnimationKeyFloat);
            break;
        }
    }
    else
    {
        reqKeySize = 0;
    }

    return sizeof(HlAnimationKeyGroup) +        /* Required size for converted submotion. */
        (reqKeySize * subMot->keyframeCount);   /* Required size for all converted keyframes. */
}

static size_t hlINNNSubMotionMaterialComputeReqSize(const HlNNSubMotion* subMot)
{
    /* Compute required size to convert this submotion and all of its keys. */
    size_t reqKeySize;
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            reqKeySize = sizeof(HlAnimationKeyFloatIP);
            break;

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            reqKeySize = sizeof(HlAnimationKeyIndexIP);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_HIDE */

        default:
            reqKeySize = 0;
            break;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            reqKeySize = sizeof(HlAnimationKeyFloat);
            break;

        case HL_NN_SMOTTYPE_MATCLBK_USER:
            reqKeySize = sizeof(HlAnimationKeyUInt);
            break;

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            reqKeySize = sizeof(HlAnimationKeyIndex);
            break;

        case HL_NN_SMOTTYPE_OFFSET_UV:
            reqKeySize = sizeof(HlAnimationKeyFloat2);
            break;

        case HL_NN_SMOTTYPE_DIFFUSE_RGB:
        case HL_NN_SMOTTYPE_SPECULAR_RGB:
        case HL_NN_SMOTTYPE_AMBIENT_RGB:
            reqKeySize = sizeof(HlAnimationKeyFloat3);
            break;

        /* TODO: Parse HL_NN_SMOTTYPE_HIDE */

        default:
            reqKeySize = 0;
            break;
        }
        break;
    }

    return sizeof(HlAnimationKeyGroup) +        /* Required size for converted submotion. */
        (reqKeySize * subMot->keyframeCount);   /* Required size for all converted keyframes. */
}

static size_t hlINNNSubMotionComputeReqSize(const HlNNSubMotion* subMot, HlU32 motType)
{
    /* Compute required size to convert this submotion and all of its keys. */
    switch (motType & HL_NN_MOTIONTYPE_CATEGORY_MASK)
    {
/* Auto-generate this switch's body. */
#define HL_IN_NN_MOTION_TYPE_AUTOGEN(nnMotType, nnMotTypeSuffix, hlMotType)\
    case nnMotType: return hlINNNSubMotion##nnMotTypeSuffix##ComputeReqSize(subMot);

#include "hl_in_nn_motion_autogen.h"

    default: return sizeof(HlAnimationKeyGroup);
    }
}

static HlAnimationFlags hlINNNMotionParseType(const HlNNMotion* motion)
{
    /* Compute type flag. */
    HlAnimationFlags animFlags;
    switch (motion->type & HL_NN_MOTIONTYPE_CATEGORY_MASK)
    {
/* Auto-generate this switch's body. */
#define HL_IN_NN_MOTION_TYPE_AUTOGEN(nnMotType, nnMotTypeSuffix, hlMotType)\
    case nnMotType: animFlags = hlMotType; break;

#include "hl_in_nn_motion_autogen.h"

    default:
        animFlags = (HlAnimationFlags)0;
        break;
    }

    /* Compute loop type. */
    switch (motion->type & HL_NN_MOTIONTYPE_REPEAT_MASK)
    {
    case HL_NN_MOTIONTYPE_TRIGGER:
        animFlags |= HL_ANIM_LOOP_TYPE_TRIGGER;
        break;

    default:
    case HL_NN_MOTIONTYPE_NOREPEAT:
        animFlags |= HL_ANIM_LOOP_TYPE_NOREPEAT;
        break;

    case HL_NN_MOTIONTYPE_CONSTREPEAT:
        animFlags |= HL_ANIM_LOOP_TYPE_CONSTREPEAT;
        break;

    case HL_NN_MOTIONTYPE_REPEAT:
        animFlags |= HL_ANIM_LOOP_TYPE_REPEAT;
        break;

    case HL_NN_MOTIONTYPE_MIRROR:
        animFlags |= HL_ANIM_LOOP_TYPE_MIRROR;
        break;

    case HL_NN_MOTIONTYPE_OFFSET:
        animFlags |= HL_ANIM_LOOP_TYPE_OFFSET;
        break;
    }

    return animFlags;
}

static HlAnimationKeyGroupFlags hlINNNSubMotionNodeParseTypeFloat(const HlNNSubMotion* subMot)
{
    switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
    {
/* Auto-generate this switch's body. */
#define HL_IN_NN_SUBMOTION_NODE_AUTOGEN(nnSubMotFormat, nnSuffix, hlSuffix, hlFormat)\
    case nnSubMotFormat: return hlFormat;

#include "hl_in_nn_motion_autogen.h"

    case HL_NN_SMOTTYPE_ROTATION_X:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_RADIAN:
            return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_BONE_TYPE_ROT_X);

        case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
            return (HL_ANIM_KEY_FORMAT_INT | HL_ANIM_KEY_BONE_TYPE_ROT_X);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_Y:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_RADIAN:
            return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_BONE_TYPE_ROT_Y);

        case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
            return (HL_ANIM_KEY_FORMAT_INT | HL_ANIM_KEY_BONE_TYPE_ROT_Y);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_Z:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_RADIAN:
            return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_BONE_TYPE_ROT_Z);

        case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
            return (HL_ANIM_KEY_FORMAT_INT | HL_ANIM_KEY_BONE_TYPE_ROT_Z);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_XYZ:
    {
        const HlAnimationKeyGroupFlags flags = (HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_BONE_TYPE_ROT_XYZ);

        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_RADIAN:
            return (HL_ANIM_KEY_FORMAT_FLOAT | flags);

        case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
            return (HL_ANIM_KEY_FORMAT_INT | flags);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    /* TODO: Parse HL_NN_SMOTTYPE_QUATERNION */

    default: return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionNodeParseTypeSInt16(const HlNNSubMotion* subMot)
{
    switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
    {
    case HL_NN_SMOTTYPE_ROTATION_X:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
            return (HL_ANIM_KEY_FORMAT_SHORT | HL_ANIM_KEY_BONE_TYPE_ROT_X);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_Y:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
            return (HL_ANIM_KEY_FORMAT_SHORT | HL_ANIM_KEY_BONE_TYPE_ROT_Y);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_Z:
    {
        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
            return (HL_ANIM_KEY_FORMAT_SHORT | HL_ANIM_KEY_BONE_TYPE_ROT_Z);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    case HL_NN_SMOTTYPE_ROTATION_XYZ:
    {
        const HlAnimationKeyGroupFlags flags = (HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_BONE_TYPE_ROT_XYZ);

        switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
        {
        case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
            return (HL_ANIM_KEY_FORMAT_SHORT | flags);
        }

        return (HlAnimationKeyGroupFlags)0;
    }

    default: return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionCameraParseTypeFloat(const HlNNSubMotion* subMot)
{
    switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
    {
    case HL_NN_SMOTTYPE_TARGET_X:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_TARGET_X);

    case HL_NN_SMOTTYPE_TARGET_Y:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_TARGET_Y);

    case HL_NN_SMOTTYPE_TARGET_Z:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_TARGET_Z);

    case HL_NN_SMOTTYPE_TARGET_XYZ:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ);

    case HL_NN_SMOTTYPE_UPTARGET_X:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPTARGET_X);

    case HL_NN_SMOTTYPE_UPTARGET_Y:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y);

    case HL_NN_SMOTTYPE_UPTARGET_Z:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z);

    case HL_NN_SMOTTYPE_UPTARGET_XYZ:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ);

    case HL_NN_SMOTTYPE_UPVECTOR_X:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X);

    case HL_NN_SMOTTYPE_UPVECTOR_Y:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y);

    case HL_NN_SMOTTYPE_UPVECTOR_Z:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z);

    case HL_NN_SMOTTYPE_UPVECTOR_XYZ:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ);

    case HL_NN_SMOTTYPE_FOVY:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_FOVY);

    case HL_NN_SMOTTYPE_ZNEAR:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_ZNEAR);

    case HL_NN_SMOTTYPE_ZFAR:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_ZFAR);

    case HL_NN_SMOTTYPE_ASPECT:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_CAM_TYPE_ASPECT);

    /* TODO: Parse HL_NN_SMOTTYPE_ROLL */

    default: return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionLightParseTypeFloat(const HlNNSubMotion* subMot)
{
    switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
    {
    case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_COLOR_R);

    case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_COLOR_G);

    case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_COLOR_B);

    case HL_NN_SMOTTYPE_LIGHT_COLOR_RGB:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB);

    case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_ALPHA);

    case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_INTENSITY);

    case HL_NN_SMOTTYPE_FALLOFF_START:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START);

    case HL_NN_SMOTTYPE_FALLOFF_END:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END);

    /* TODO: Parse HL_NN_SMOTTYPE_INNER_ANGLE */
    /* TODO: Parse HL_NN_SMOTTYPE_OUTER_ANGLE */
    /* TODO: Parse HL_NN_SMOTTYPE_INNER_RANGE */
    /* TODO: Parse HL_NN_SMOTTYPE_OUTER_RANGE */

    default: return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionMorphParseTypeFloat(const HlNNSubMotion* subMot)
{
    if ((subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK) ==
        HL_NN_SMOTTYPE_MORPH_WEIGHT)
    {
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MORPH_TYPE_WEIGHT);
    }
    else
    {
        return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionMaterialParseTypeFloat(const HlNNSubMotion* subMot)
{
    switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
    {
    case HL_NN_SMOTTYPE_DIFFUSE_R:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R);

    case HL_NN_SMOTTYPE_DIFFUSE_G:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G);

    case HL_NN_SMOTTYPE_DIFFUSE_B:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B);

    case HL_NN_SMOTTYPE_DIFFUSE_RGB:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB);

    case HL_NN_SMOTTYPE_ALPHA:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_ALPHA);

    case HL_NN_SMOTTYPE_SPECULAR_R:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_SPECULAR_R);

    case HL_NN_SMOTTYPE_SPECULAR_G:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_SPECULAR_G);

    case HL_NN_SMOTTYPE_SPECULAR_B:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_SPECULAR_B);

    case HL_NN_SMOTTYPE_SPECULAR_RGB:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB);

    case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL);

    case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS);

    case HL_NN_SMOTTYPE_AMBIENT_R:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_AMBIENT_R);

    case HL_NN_SMOTTYPE_AMBIENT_G:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_AMBIENT_G);

    case HL_NN_SMOTTYPE_AMBIENT_B:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_AMBIENT_B);

    case HL_NN_SMOTTYPE_AMBIENT_RGB:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_3D |
            HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB);

    case HL_NN_SMOTTYPE_TEXTURE_INDEX:
        return (HL_ANIM_KEY_FORMAT_INDEX | HL_ANIM_KEY_MAT_TYPE_TEX_INDEX);

    case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        /* TODO: Is this format right? */
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_TEX_BLEND);

    case HL_NN_SMOTTYPE_OFFSET_U:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U);

    case HL_NN_SMOTTYPE_OFFSET_V:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V);

    case HL_NN_SMOTTYPE_OFFSET_UV:
        return (HL_ANIM_KEY_FORMAT_FLOAT | HL_ANIM_KEY_DIMENSION_2D |
            HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV);

    case HL_NN_SMOTTYPE_MATCLBK_USER:
        return (HL_ANIM_KEY_FORMAT_UINT | HL_ANIM_KEY_TYPE_CUSTOM);

    /* TODO: Parse HL_NN_SMOTTYPE_HIDE */

    default: return (HlAnimationKeyGroupFlags)0;
    }
}

static HlAnimationKeyGroupFlags hlINNNSubMotionParseType(
    const HlNNSubMotion* subMot, HlU32 motType)
{
    /* Compute format, type, and dimension flags. */
    HlAnimationKeyGroupFlags flags;
    switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
    {
    case HL_NN_SMOTTYPE_FRAME_FLOAT:
        switch (motType)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_MOTION_TYPE_AUTOGEN(nnMotType, nnMotTypeSuffix, hlMotType)\
    case nnMotType: flags = hlINNNSubMotion##nnMotTypeSuffix##ParseTypeFloat(subMot); break;

#include "hl_in_nn_motion_autogen.h"

        default:
            flags = (HlAnimationKeyGroupFlags)0;
            break;
        }
        break;

    case HL_NN_SMOTTYPE_FRAME_SINT16:
        switch (motType)
        {
        case HL_NN_MOTIONTYPE_NODE:
            flags = hlINNNSubMotionNodeParseTypeSInt16(subMot);
            break;

        default:
            flags = (HlAnimationKeyGroupFlags)0;
            break;
        }
        break;

    default:
        flags = (HlAnimationKeyGroupFlags)0;
        break;
    }

    /* Compute interpolation type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_SPLINE:
        flags |= HL_ANIM_KEY_IP_TYPE_SPLINE;
        break;

    case HL_NN_SMOTIPTYPE_LINEAR:
        flags |= HL_ANIM_KEY_IP_TYPE_LINEAR;
        break;

    case HL_NN_SMOTIPTYPE_CONSTANT:
        flags |= HL_ANIM_KEY_IP_TYPE_CONSTANT;
        break;

    case HL_NN_SMOTIPTYPE_BEZIER:
        flags |= HL_ANIM_KEY_IP_TYPE_BEZIER;
        break;

    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        flags |= HL_ANIM_KEY_IP_TYPE_SI_SPLINE;
        break;

    case HL_NN_SMOTIPTYPE_TRIGGER:
        flags |= HL_ANIM_KEY_IP_TYPE_TRIGGER;
        break;

    /* TODO: Parse HL_NN_SMOTIPTYPE_QUAT_LERP */
    /* TODO: Parse HL_NN_SMOTIPTYPE_QUAT_SLERP */
    /* TODO: Parse HL_NN_SMOTIPTYPE_QUAT_SQUAD */
    }

    /* Compute loop type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_REPEAT_MASK)
    {
    default:
    case HL_NN_SMOTIPTYPE_NOREPEAT:
        flags |= HL_ANIM_KEY_LOOP_TYPE_NOREPEAT;
        break;

    case HL_NN_SMOTIPTYPE_CONSTREPEAT:
        flags |= HL_ANIM_KEY_LOOP_TYPE_CONSTREPEAT;
        break;

    case HL_NN_SMOTIPTYPE_REPEAT:
        flags |= HL_ANIM_KEY_LOOP_TYPE_REPEAT;
        break;

    case HL_NN_SMOTIPTYPE_MIRROR:
        flags |= HL_ANIM_KEY_LOOP_TYPE_MIRROR;
        break;

    case HL_NN_SMOTIPTYPE_OFFSET:
        flags |= HL_ANIM_KEY_LOOP_TYPE_OFFSET;
        break;
    }

    return flags;
}

#define hlINNNSubMotionReadKeyframesCaseBody(nntype, hltype, parseLine) {\
    hltype* hlKeys = ((hltype*)hlKeyframes);\
    size_t i;\
    for (i = 0; i < subMot->keyframeCount; ++i) {\
        hlKeys[i].frame = (float)((&keys->nntype)[i].frame);\
        parseLine;\
    }\
    return HL_ADD_OFF(hlKeys, subMot->keyframeCount * sizeof(hltype));\
}

static void* hlINNNSubMotionNodeReadKeyFramesFloat(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TRANSLATION_X:
        case HL_NN_SMOTTYPE_TRANSLATION_Y:
        case HL_NN_SMOTTYPE_TRANSLATION_Z:
        case HL_NN_SMOTTYPE_SCALING_X:
        case HL_NN_SMOTTYPE_SCALING_Y:
        case HL_NN_SMOTTYPE_SCALING_Z:
        case HL_NN_SMOTTYPE_USER_FLOAT:
            hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
                hlKeys[i].value = (&keys->typeBezier)[i].value;
                hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
                hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out);

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
                    hlKeys[i].value = (&keys->typeBezier)[i].value;
                    hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
                    hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out);

            case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                hlINNNSubMotionReadKeyframesCaseBody(typeBezierInt, HlAnimationKeyIntIP,
                    hlKeys[i].value = (int)((&keys->typeBezierInt)[i].value);
                    hlKeys[i].ip.in = (&keys->typeBezierInt)[i].bhandle.in;
                    hlKeys[i].ip.out = (&keys->typeBezierInt)[i].bhandle.out);

            default: return hlKeyframes;
            }

        /* TODO: Parse HL_NN_SMOTTYPE_QUATERNION */
        /* TODO: Parse HL_NN_SMOTTYPE_NODEHIDE */

        default: return hlKeyframes;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TRANSLATION_X:
        case HL_NN_SMOTTYPE_TRANSLATION_Y:
        case HL_NN_SMOTTYPE_TRANSLATION_Z:
        case HL_NN_SMOTTYPE_SCALING_X:
        case HL_NN_SMOTTYPE_SCALING_Y:
        case HL_NN_SMOTTYPE_SCALING_Z:
        case HL_NN_SMOTTYPE_USER_FLOAT:
            hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
                hlKeys[i].value = (&keys->typeFloat)[i].value);

        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
                    hlKeys[i].value = (&keys->typeFloat)[i].value)

            case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                hlINNNSubMotionReadKeyframesCaseBody(typeInt, HlAnimationKeyInt,
                    hlKeys[i].value = (int)((&keys->typeInt)[i].value))

            default: return hlKeyframes;
            }

        case HL_NN_SMOTTYPE_USER_UINT32:
            hlINNNSubMotionReadKeyframesCaseBody(typeUInt, HlAnimationKeyUInt,
                hlKeys[i].value = (unsigned int)((&keys->typeUInt)[i].value));

        case HL_NN_SMOTTYPE_TRANSLATION_XYZ:
        case HL_NN_SMOTTYPE_SCALING_XYZ:
            hlINNNSubMotionReadKeyframesCaseBody(typeVector, HlAnimationKeyFloat3,
                hlKeys[i].value[0] = (&keys->typeVector)[i].value.x;
                hlKeys[i].value[1] = (&keys->typeVector)[i].value.y;
                hlKeys[i].value[2] = (&keys->typeVector)[i].value.z)

        case HL_NN_SMOTTYPE_ROTATION_XYZ:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_RADIAN:
                hlINNNSubMotionReadKeyframesCaseBody(typeVector, HlAnimationKeyFloat3,
                    hlKeys[i].value[0] = (&keys->typeVector)[i].value.x;
                    hlKeys[i].value[1] = (&keys->typeVector)[i].value.y;
                    hlKeys[i].value[2] = (&keys->typeVector)[i].value.z)

            case HL_NN_SMOTTYPE_ANGLE_ANGLE32:
                hlINNNSubMotionReadKeyframesCaseBody(typeRotA32, HlAnimationKeyInt3,
                    hlKeys[i].value[0] = (int)((&keys->typeRotA32)[i].value.x);
                    hlKeys[i].value[1] = (int)((&keys->typeRotA32)[i].value.y);
                    hlKeys[i].value[2] = (int)((&keys->typeRotA32)[i].value.z))

            default: return hlKeyframes;
            }

        /* TODO: Parse HL_NN_SMOTTYPE_QUATERNION */
        /* TODO: Parse HL_NN_SMOTTYPE_NODEHIDE */

        default: return hlKeyframes;
        }
        break;
    }
}

static void* hlINNNSubMotionNodeReadKeyFramesSInt16(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
                hlINNNSubMotionReadKeyframesCaseBody(typeSplineShort, HlAnimationKeyShortIP,
                    hlKeys[i].value = (short)((&keys->typeSplineShort)[i].value);
                    hlKeys[i].ip.in = (&keys->typeSplineShort)[i].shandle.in;
                    hlKeys[i].ip.out = (&keys->typeSplineShort)[i].shandle.out)

            default: return hlKeyframes;
            }
        }

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_ROTATION_X:
        case HL_NN_SMOTTYPE_ROTATION_Y:
        case HL_NN_SMOTTYPE_ROTATION_Z:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
                hlINNNSubMotionReadKeyframesCaseBody(typeShort, HlAnimationKeyShort,
                    hlKeys[i].value = (short)((&keys->typeShort)[i].value))

            default: return hlKeyframes;
            }

        case HL_NN_SMOTTYPE_ROTATION_XYZ:
            switch (subMot->type & HL_NN_SMOTTYPE_ANGLE_MASK)
            {
            case HL_NN_SMOTTYPE_ANGLE_ANGLE16:
                hlINNNSubMotionReadKeyframesCaseBody(typeRotA16, HlAnimationKeyShort3,
                    hlKeys[i].value[0] = (short)((&keys->typeRotA16)[i].value.x);
                    hlKeys[i].value[1] = (short)((&keys->typeRotA16)[i].value.y);
                    hlKeys[i].value[2] = (short)((&keys->typeRotA16)[i].value.z))

            default: return hlKeyframes;
            }
        }
        return hlKeyframes;
    }
}

static void* hlINNNSubMotionCameraReadKeyFramesFloat(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
                hlKeys[i].value = (&keys->typeBezier)[i].value;
                hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
                hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out);

        /* TODO: Parse HL_NN_SMOTTYPE_ROLL */

        default: return hlKeyframes;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_TARGET_X:
        case HL_NN_SMOTTYPE_TARGET_Y:
        case HL_NN_SMOTTYPE_TARGET_Z:
        case HL_NN_SMOTTYPE_UPTARGET_X:
        case HL_NN_SMOTTYPE_UPTARGET_Y:
        case HL_NN_SMOTTYPE_UPTARGET_Z:
        case HL_NN_SMOTTYPE_UPVECTOR_X:
        case HL_NN_SMOTTYPE_UPVECTOR_Y:
        case HL_NN_SMOTTYPE_UPVECTOR_Z:
        case HL_NN_SMOTTYPE_FOVY:
        case HL_NN_SMOTTYPE_ZNEAR:
        case HL_NN_SMOTTYPE_ZFAR:
        case HL_NN_SMOTTYPE_ASPECT:
            hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
                hlKeys[i].value = (&keys->typeFloat)[i].value);

        case HL_NN_SMOTTYPE_TARGET_XYZ:
        case HL_NN_SMOTTYPE_UPTARGET_XYZ:
        case HL_NN_SMOTTYPE_UPVECTOR_XYZ:
            hlINNNSubMotionReadKeyframesCaseBody(typeVector, HlAnimationKeyFloat3,
                hlKeys[i].value[0] = (&keys->typeVector)[i].value.x;
                hlKeys[i].value[1] = (&keys->typeVector)[i].value.y;
                hlKeys[i].value[2] = (&keys->typeVector)[i].value.z)

        /* TODO: Parse HL_NN_SMOTTYPE_ROLL */

        default: return hlKeyframes;
        }
        break;
    }
}

static void* hlINNNSubMotionLightReadKeyFramesFloat(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
                hlKeys[i].value = (&keys->typeBezier)[i].value;
                hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
                hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out);

        /* TODO: Parse HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_RANGE */

        default: return hlKeyframes;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_LIGHT_COLOR_R:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_G:
        case HL_NN_SMOTTYPE_LIGHT_COLOR_B:
        case HL_NN_SMOTTYPE_LIGHT_ALPHA:
        case HL_NN_SMOTTYPE_LIGHT_INTENSITY:
        case HL_NN_SMOTTYPE_FALLOFF_START:
        case HL_NN_SMOTTYPE_FALLOFF_END:
            hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
                hlKeys[i].value = (&keys->typeFloat)[i].value);

        case HL_NN_SMOTTYPE_LIGHT_COLOR_RGB:
            hlINNNSubMotionReadKeyframesCaseBody(typeRGB, HlAnimationKeyFloat3,
                hlKeys[i].value[0] = (&keys->typeRGB)[i].value.r;
                hlKeys[i].value[1] = (&keys->typeRGB)[i].value.g;
                hlKeys[i].value[2] = (&keys->typeRGB)[i].value.b)

        /* TODO: Parse HL_NN_SMOTTYPE_INNER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_ANGLE */
        /* TODO: Parse HL_NN_SMOTTYPE_INNER_RANGE */
        /* TODO: Parse HL_NN_SMOTTYPE_OUTER_RANGE */

        default: return hlKeyframes;
        }
        break;
    }
}

static void* hlINNNSubMotionMorphReadKeyFramesFloat(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Ensure this keyframe is of type HL_NN_SMOTTYPE_MORPH_WEIGHT. */
    if ((subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK) != HL_NN_SMOTTYPE_MORPH_WEIGHT)
        return hlKeyframes;

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
            hlKeys[i].value = (&keys->typeBezier)[i].value;
            hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
            hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out)

    default:
        hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
            hlKeys[i].value = (&keys->typeFloat)[i].value);
    }
}

static void* hlINNNSubMotionMaterialReadKeyFramesFloat(
    const HlNNSubMotion* subMot, void* hlKeyframes)
{
    /* Get NN Motion Keys pointer. */
    const HlNNMotionKey* keys = (const HlNNMotionKey*)hlOff32Get(&subMot->keyframes);

    /* Convert keyframes based on NN SubMotion type. */
    switch (subMot->ipType & HL_NN_SMOTIPTYPE_IP_MASK)
    {
    case HL_NN_SMOTIPTYPE_BEZIER:
    case HL_NN_SMOTIPTYPE_SI_SPLINE:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            hlINNNSubMotionReadKeyframesCaseBody(typeBezier, HlAnimationKeyFloatIP,
                hlKeys[i].value = (&keys->typeBezier)[i].value;
                hlKeys[i].ip.in = (&keys->typeBezier)[i].bhandle.in;
                hlKeys[i].ip.out = (&keys->typeBezier)[i].bhandle.out)

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            hlINNNSubMotionReadKeyframesCaseBody(typeBezierInt, HlAnimationKeyIndexIP,
                hlKeys[i].value = (size_t)((&keys->typeBezierInt)[i].value);
                hlKeys[i].ip.in = (&keys->typeBezierInt)[i].bhandle.in;
                hlKeys[i].ip.out = (&keys->typeBezierInt)[i].bhandle.out)

        default: return hlKeyframes;
        }
        break;

    default:
        switch (subMot->type & HL_NN_SMOTTYPE_VALUETYPE_MASK)
        {
        case HL_NN_SMOTTYPE_DIFFUSE_R:
        case HL_NN_SMOTTYPE_DIFFUSE_G:
        case HL_NN_SMOTTYPE_DIFFUSE_B:
        case HL_NN_SMOTTYPE_ALPHA:
        case HL_NN_SMOTTYPE_SPECULAR_R:
        case HL_NN_SMOTTYPE_SPECULAR_G:
        case HL_NN_SMOTTYPE_SPECULAR_B:
        case HL_NN_SMOTTYPE_SPECULAR_LEVEL:
        case HL_NN_SMOTTYPE_SPECULAR_GLOSS:
        case HL_NN_SMOTTYPE_AMBIENT_R:
        case HL_NN_SMOTTYPE_AMBIENT_G:
        case HL_NN_SMOTTYPE_AMBIENT_B:
        case HL_NN_SMOTTYPE_TEXTURE_BLEND:
        case HL_NN_SMOTTYPE_OFFSET_U:
        case HL_NN_SMOTTYPE_OFFSET_V:
            hlINNNSubMotionReadKeyframesCaseBody(typeFloat, HlAnimationKeyFloat,
                hlKeys[i].value = (&keys->typeFloat)[i].value);

        case HL_NN_SMOTTYPE_MATCLBK_USER:
            hlINNNSubMotionReadKeyframesCaseBody(typeUInt, HlAnimationKeyUInt,
                hlKeys[i].value = (unsigned int)((&keys->typeUInt)[i].value))

        case HL_NN_SMOTTYPE_TEXTURE_INDEX:
            hlINNNSubMotionReadKeyframesCaseBody(typeInt, HlAnimationKeyIndex,
                hlKeys[i].value = (size_t)((&keys->typeInt)[i].value))

        case HL_NN_SMOTTYPE_OFFSET_UV:
            hlINNNSubMotionReadKeyframesCaseBody(typeTexCoord, HlAnimationKeyFloat2,
                hlKeys[i].value[0] = (&keys->typeTexCoord)[i].value.u;
                hlKeys[i].value[1] = (&keys->typeTexCoord)[i].value.v)

        case HL_NN_SMOTTYPE_DIFFUSE_RGB:
        case HL_NN_SMOTTYPE_SPECULAR_RGB:
        case HL_NN_SMOTTYPE_AMBIENT_RGB:
            hlINNNSubMotionReadKeyframesCaseBody(typeRGB, HlAnimationKeyFloat3,
                hlKeys[i].value[0] = (&keys->typeRGB)[i].value.r;
                hlKeys[i].value[1] = (&keys->typeRGB)[i].value.g;
                hlKeys[i].value[2] = (&keys->typeRGB)[i].value.b)

        /* TODO: Parse HL_NN_SMOTTYPE_HIDE */

        default: return hlKeyframes;
        }
        break;
    }
}

static void* hlINNNSubMotionReadKeyFrames(
    const HlNNSubMotion* subMot, HlU32 motType, void* hlKeyframes)
{
    switch (subMot->type & HL_NN_SMOTTYPE_FRAME_MASK)
    {
    case HL_NN_SMOTTYPE_FRAME_FLOAT:
        switch (motType)
        {
/* Auto-generate this switch's body. */
#define HL_IN_NN_MOTION_TYPE_AUTOGEN(nnMotType, nnMotTypeSuffix, hlMotType)\
    case nnMotType: return hlINNNSubMotion##nnMotTypeSuffix##ReadKeyFramesFloat(\
                        subMot, hlKeyframes);

#include "hl_in_nn_motion_autogen.h"
        }
        break;

    case HL_NN_SMOTTYPE_FRAME_SINT16:
        switch (motType)
        {
        case HL_NN_MOTIONTYPE_NODE:
            return hlINNNSubMotionNodeReadKeyFramesSInt16(subMot, hlKeyframes);
        }
        break;
    }

    return hlKeyframes;
}

HlResult hlNNMotionParse(const HlNNMotion* HL_RESTRICT motion,
    HlAnimation** HL_RESTRICT hlAnim)
{
    HlAnimation* hlAnimBuf;
    const HlNNSubMotion* subMots;
    size_t totalReqSize, i;

    /* Get NN SubMotions pointer. */
    subMots = (const HlNNSubMotion*)hlOff32Get(&motion->subMotions);

    /* Compute total required size to convert this NN Motion. */
    totalReqSize = sizeof(HlAnimation);
    for (i = 0; i < motion->subMotionCount; ++i)
    {
        /* Compute required size to convert this NN SubMotion and all of its keys. */
        totalReqSize += hlINNNSubMotionComputeReqSize(&subMots[i], motion->type);
    }

    /* Allocate a buffer of the required size. */
    hlAnimBuf = (HlAnimation*)hlAlloc(totalReqSize);
    if (!hlAnimBuf) return HL_ERROR_OUT_OF_MEMORY;

    /* Convert NN Motion to HlAnimation. */
    {
        void* hlAnimKeys;
        HlU32 motType;
        
        /* Get motion type. */
        motType = (motion->type & HL_NN_MOTIONTYPE_CATEGORY_MASK);

        /* Setup HlAnimation. */
        hlAnimBuf->flagsAndType = hlINNNMotionParseType(motion);
        hlAnimBuf->framerate = motion->framerate;
        hlAnimBuf->startFrame = motion->startFrame;
        hlAnimBuf->endFrame = motion->endFrame;
        hlAnimBuf->keyGroupCount = (size_t)motion->subMotionCount;
        hlAnimBuf->keyGroups = (HlAnimationKeyGroup*)
            HL_ADD_OFF(hlAnimBuf, sizeof(*hlAnimBuf));

        /* Get animation keys pointer. */
        hlAnimKeys = HL_ADD_OFF(hlAnimBuf->keyGroups,
            sizeof(HlAnimationKeyGroup) * hlAnimBuf->keyGroupCount);

        /* Convert NN SubMotions to HlAnimationKeyGroups. */
        for (i = 0; i < motion->subMotionCount; ++i)
        {
            HlAnimationKeyGroup* keyGroup = &hlAnimBuf->keyGroups[i];

            keyGroup->flagsAndType = hlINNNSubMotionParseType(&subMots[i], motType);
            keyGroup->startFrame = subMots[i].startFrame;
            keyGroup->endFrame = subMots[i].endFrame;
            keyGroup->startKeyframe = subMots[i].startKeyframe;
            keyGroup->endKeyframe = subMots[i].endKeyframe;

            if (motType == HL_NN_MOTIONTYPE_MATERIAL)
            {
                keyGroup->index = (size_t)(subMots[i].id & 0xFFFFU);
                keyGroup->subIndex = ((size_t)(subMots[i].id >> 16) & 0xFFFFU);
            }
            else
            {
                keyGroup->index = (size_t)subMots[i].id;
                keyGroup->subIndex = 0;
            }
            
            keyGroup->keyframeCount = (size_t)subMots[i].keyframeCount;
            keyGroup->keyframes = hlAnimKeys;

            /* Convert NN keyframes to HedgeLib animation keys and increase hlAnimKeys pointer. */
            hlAnimKeys = hlINNNSubMotionReadKeyFrames(&subMots[i], motType, hlAnimKeys);

            /*
               If the hlAnimKeys pointer wasn't increased, it means the current
               NN SubMotion's type is currently unsupported, and no animation keys
               were generated; set the keyframe pointer, keyframeCount, and type to 0.
            */
            if (hlAnimKeys == keyGroup->keyframes)
            {
                keyGroup->flagsAndType = (HlAnimationKeyGroupFlags)0;
                keyGroup->keyframeCount = 0;
                keyGroup->keyframes = NULL;
            }
        }
    }

    /* Set pointer and return success. */
    *hlAnim = hlAnimBuf;
    return HL_RESULT_SUCCESS;
}

HlResult hlNNMotionRead(HlBlob* HL_RESTRICT blob,
    HlAnimation** HL_RESTRICT hlAnim)
{
    /* Fix NN general data. */
    HlNNBinCnkDataHeader* dataHeader;
    hlNNFix(blob);

    /* Fix NN motion data. */
    dataHeader = hlNNGetDataHeader(blob);
    hlNNMotionFix(dataHeader);

    /* Parse NN motion data into HlAnim and return result. */
    return hlNNMotionParse((const HlNNMotion*)hlOff32Get(
        &dataHeader->mainDataOffset), hlAnim);
}

#define hlINNNSubMotionWriteKeyframesLoop(parseLine, swapFunc)\
    for (i = 0; i < keyGroup->keyframeCount; ++i) {\
        /* Setup NN keyframe. */\
        key.frame = keyframes[i].frame;\
        parseLine;\
\
        /* Endian swap if necessary for the given platform. */\
        if (hlNNPlatformNeedsSwap(platform))\
        {\
            swapFunc(&key);\
        }\
\
        /* Write NN keyframe. */\
        result = hlFileWrite(file, sizeof(key), &key, 0);\
        if (HL_FAILED(result)) return result;\
    }

#define hlINNNSubMotionWriteKeyframesCase(format, type, swapFunc, parseLine)\
    case format: {\
        /* Get keyframes pointer. */\
        const type* keyframes = (const type*)keyGroup->keyframes;\
\
        /* Write keyframes. */\
        size_t i;\
        hlINNNSubMotionWriteKeyframesLoop(parseLine, swapFunc)\
\
        /* Return success. */\
        return HL_RESULT_SUCCESS;\
    }

static HlResult hlINNNSubMotionWriteKeyframesFloat(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyFloat key;
    HlResult result;

    if ((keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK) != HL_ANIM_KEY_DIMENSION_1D)
        return HL_ERROR_UNSUPPORTED;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
    {
    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
        HlAnimationKeyFloat, hlNNMotionKeyFloatSwap, 
        key.value = keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
        HlAnimationKeyShort, hlNNMotionKeyFloatSwap, 
        key.value = ((float)keyframes[i].value / SHRT_MAX))

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
        HlAnimationKeyUShort, hlNNMotionKeyFloatSwap, 
        key.value = ((float)keyframes[i].value / USHRT_MAX))

    /* TODO: Should we convert ints/uints too? */

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesInt(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyInt key;
    HlResult result;

    if ((keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK) != HL_ANIM_KEY_DIMENSION_1D)
        return HL_ERROR_UNSUPPORTED;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
    {
    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_INDEX,
        HlAnimationKeyIndex, hlNNMotionKeyIntSwap, 
        key.value = (HlS32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
        HlAnimationKeyShort, hlNNMotionKeyIntSwap, 
        key.value = (HlS32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
        HlAnimationKeyUShort, hlNNMotionKeyIntSwap, 
        key.value = (HlS32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_INT,
        HlAnimationKeyInt, hlNNMotionKeyIntSwap,
        key.value = (HlS32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_UINT,
        HlAnimationKeyUInt, hlNNMotionKeyIntSwap,
        key.value = (HlS32)keyframes[i].value)

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesUInt(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyUInt key;
    HlResult result;

    if ((keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK) != HL_ANIM_KEY_DIMENSION_1D)
        return HL_ERROR_UNSUPPORTED;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
    {
    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_INDEX,
        HlAnimationKeyIndex, hlNNMotionKeyUIntSwap,
        key.value = (HlU32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
        HlAnimationKeyShort, hlNNMotionKeyUIntSwap,
        key.value = (HlU32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
        HlAnimationKeyUShort, hlNNMotionKeyUIntSwap,
        key.value = (HlU32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_INT,
        HlAnimationKeyInt, hlNNMotionKeyUIntSwap,
        key.value = (HlU32)keyframes[i].value)

    hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_UINT,
        HlAnimationKeyUInt, hlNNMotionKeyUIntSwap,
        key.value = (HlU32)keyframes[i].value)

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesUV(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyTexCoord key;
    HlResult result;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort, hlNNMotionKeyTexCoordSwap, 
            key.value.u = ((float)keyframes[i].value / SHRT_MAX);
            key.value.v = key.value.u)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort, hlNNMotionKeyTexCoordSwap, 
            key.value.u = ((float)keyframes[i].value / USHRT_MAX);
            key.value.v = key.value.u)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat, hlNNMotionKeyTexCoordSwap, 
            key.value.u = keyframes[i].value;
            key.value.v = key.value.u)

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    case HL_ANIM_KEY_DIMENSION_2D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort2, hlNNMotionKeyTexCoordSwap, 
            key.value.u = ((float)keyframes[i].value[0] / SHRT_MAX);
            key.value.v = ((float)keyframes[i].value[1] / SHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort2, hlNNMotionKeyTexCoordSwap, 
            key.value.u = ((float)keyframes[i].value[0] / USHRT_MAX);
            key.value.v = ((float)keyframes[i].value[1] / USHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat2, hlNNMotionKeyTexCoordSwap,
            key.value.u = keyframes[i].value[0];
            key.value.v = keyframes[i].value[1])

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesRGB(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyRGB key;
    HlResult result;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort, hlNNMotionKeyRGBSwap, 
            key.value.r = ((float)keyframes[i].value / SHRT_MAX);
            key.value.g = key.value.r; key.value.b = key.value.r)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort, hlNNMotionKeyRGBSwap, 
            key.value.r = ((float)keyframes[i].value / USHRT_MAX);
            key.value.g = key.value.r; key.value.b = key.value.r)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat, hlNNMotionKeyRGBSwap, 
            key.value.r = keyframes[i].value;
            key.value.g = key.value.r; key.value.b = key.value.r)

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    case HL_ANIM_KEY_DIMENSION_3D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort3, hlNNMotionKeyRGBSwap, 
            key.value.r = ((float)keyframes[i].value[0] / SHRT_MAX);
            key.value.g = ((float)keyframes[i].value[1] / SHRT_MAX);
            key.value.b = ((float)keyframes[i].value[2] / SHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort3, hlNNMotionKeyRGBSwap, 
            key.value.r = ((float)keyframes[i].value[0] / USHRT_MAX);
            key.value.g = ((float)keyframes[i].value[1] / USHRT_MAX);
            key.value.b = ((float)keyframes[i].value[2] / USHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat3, hlNNMotionKeyRGBSwap,
            key.value.r = keyframes[i].value[0];
            key.value.g = keyframes[i].value[1];
            key.value.b = keyframes[i].value[2])

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesVector(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyVector key;
    HlResult result;

    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort, hlNNMotionKeyVectorSwap,
            key.value.x = ((float)keyframes[i].value / SHRT_MAX);
            key.value.y = key.value.x; key.value.z = key.value.x)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort, hlNNMotionKeyVectorSwap,
            key.value.x = ((float)keyframes[i].value / USHRT_MAX);
            key.value.y = key.value.x; key.value.z = key.value.x)

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat, hlNNMotionKeyVectorSwap,
            key.value.x = keyframes[i].value;
            key.value.y = key.value.x; key.value.z = key.value.x)

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    case HL_ANIM_KEY_DIMENSION_3D:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_SHORT,
            HlAnimationKeyShort3, hlNNMotionKeyVectorSwap,
            key.value.x = ((float)keyframes[i].value[0] / SHRT_MAX);
            key.value.y = ((float)keyframes[i].value[1] / SHRT_MAX);
            key.value.z = ((float)keyframes[i].value[2] / SHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_USHORT,
            HlAnimationKeyUShort3, hlNNMotionKeyVectorSwap,
            key.value.x = ((float)keyframes[i].value[0] / USHRT_MAX);
            key.value.y = ((float)keyframes[i].value[1] / USHRT_MAX);
            key.value.z = ((float)keyframes[i].value[2] / USHRT_MAX))

        hlINNNSubMotionWriteKeyframesCase(HL_ANIM_KEY_FORMAT_FLOAT,
            HlAnimationKeyFloat3, hlNNMotionKeyVectorSwap,
            key.value.x = keyframes[i].value[0];
            key.value.y = keyframes[i].value[1];
            key.value.z = keyframes[i].value[2])

        /* TODO: Should we also support ints/uints? */

        default: return HL_ERROR_UNSUPPORTED;
        }
        break;

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesRotA16(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyRotateA16 key;
    HlResult result;

    /* Ensure the given animation key group's format is short. */
    if ((keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK) != HL_ANIM_KEY_FORMAT_SHORT)
        return HL_ERROR_UNSUPPORTED;

    /* Write A16 short keyframes. */
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
    {
        /* Get keyframes pointer. */
        const HlAnimationKeyShort* keyframes =
            (const HlAnimationKeyShort*)keyGroup->keyframes;
        
        /* Write keyframes. */
        size_t i;
        for (i = 0; i < keyGroup->keyframeCount; ++i)
        {
            /* Setup NN keyframe. */
            key.frame = (HlS16)keyframes[i].frame;
            key.value.x = (HlS16)keyframes[i].value;
            key.value.y = key.value.x;
            key.value.z = key.value.x;

            /* Endian swap if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlNNMotionKeyRotateA16Swap(&key);
            }
            
            /* Write NN keyframe. */
            result = hlFileWrite(file, sizeof(key), &key, 0);
            if (HL_FAILED(result)) return result;
        }
        
        /* Return success. */
        return HL_RESULT_SUCCESS;
    }

    case HL_ANIM_KEY_DIMENSION_3D:
    {
        /* Get keyframes pointer. */
        const HlAnimationKeyShort3* keyframes =
            (const HlAnimationKeyShort3*)keyGroup->keyframes;

        /* Write keyframes. */
        size_t i;
        for (i = 0; i < keyGroup->keyframeCount; ++i)
        {
            /* Setup NN keyframe. */
            key.frame = (HlS16)keyframes[i].frame;
            key.value.x = (HlS16)keyframes[i].value[0];
            key.value.y = (HlS16)keyframes[i].value[1];
            key.value.z = (HlS16)keyframes[i].value[2];

            /* Endian swap if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlNNMotionKeyRotateA16Swap(&key);
            }

            /* Write NN keyframe. */
            result = hlFileWrite(file, sizeof(key), &key, 0);
            if (HL_FAILED(result)) return result;
        }

        /* Return success. */
        return HL_RESULT_SUCCESS;
    }

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframesRotA32(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    HlNNMotionKeyRotateA32 key;
    HlResult result;

    /* Ensure the given animation key group's format is int. */
    if ((keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK) != HL_ANIM_KEY_FORMAT_INT)
        return HL_ERROR_UNSUPPORTED;

    /* Write A16 short keyframes. */
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
    {
        /* Get keyframes pointer. */
        const HlAnimationKeyInt* keyframes =
            (const HlAnimationKeyInt*)keyGroup->keyframes;
        
        /* Write keyframes. */
        size_t i;
        for (i = 0; i < keyGroup->keyframeCount; ++i)
        {
            /* Setup NN keyframe. */
            key.frame = keyframes[i].frame;
            key.value.x = (HlS32)keyframes[i].value;
            key.value.y = key.value.x;
            key.value.z = key.value.x;

            /* Endian swap if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlNNMotionKeyRotateA32Swap(&key);
            }
            
            /* Write NN keyframe. */
            result = hlFileWrite(file, sizeof(key), &key, 0);
            if (HL_FAILED(result)) return result;
        }
        
        /* Return success. */
        return HL_RESULT_SUCCESS;
    }

    case HL_ANIM_KEY_DIMENSION_3D:
    {
        /* Get keyframes pointer. */
        const HlAnimationKeyInt3* keyframes =
            (const HlAnimationKeyInt3*)keyGroup->keyframes;

        /* Write keyframes. */
        size_t i;
        for (i = 0; i < keyGroup->keyframeCount; ++i)
        {
            /* Setup NN keyframe. */
            key.frame = keyframes[i].frame;
            key.value.x = (HlS32)keyframes[i].value[0];
            key.value.y = (HlS32)keyframes[i].value[1];
            key.value.z = (HlS32)keyframes[i].value[2];

            /* Endian swap if necessary for the given platform. */
            if (hlNNPlatformNeedsSwap(platform))
            {
                hlNNMotionKeyRotateA32Swap(&key);
            }

            /* Write NN keyframe. */
            result = hlFileWrite(file, sizeof(key), &key, 0);
            if (HL_FAILED(result)) return result;
        }

        /* Return success. */
        return HL_RESULT_SUCCESS;
    }

    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionBoneWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_BONE_TYPE_POS_X:
    case HL_ANIM_KEY_BONE_TYPE_POS_Y:
    case HL_ANIM_KEY_BONE_TYPE_POS_Z:
    case HL_ANIM_KEY_BONE_TYPE_ROT_X:
    case HL_ANIM_KEY_BONE_TYPE_ROT_Y:
    case HL_ANIM_KEY_BONE_TYPE_ROT_Z:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_X:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Y:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Z:
        return hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    case HL_ANIM_KEY_MAT_TYPE_TEX_INDEX:
        return hlINNNSubMotionWriteKeyframesInt(keyGroup, platform, file);

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return ((keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK) == HL_ANIM_KEY_FORMAT_UINT) ?
            hlINNNSubMotionWriteKeyframesUInt(keyGroup, platform, file) :
            hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV:
        return hlINNNSubMotionWriteKeyframesUV(keyGroup, platform, file);

    case HL_ANIM_KEY_BONE_TYPE_POS_XYZ:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_XYZ:
        return hlINNNSubMotionWriteKeyframesVector(keyGroup, platform, file);

    case HL_ANIM_KEY_BONE_TYPE_ROT_XYZ:
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return hlINNNSubMotionWriteKeyframesVector(keyGroup, platform, file);

        case HL_ANIM_KEY_FORMAT_SHORT:
            return hlINNNSubMotionWriteKeyframesRotA16(keyGroup, platform, file);

        case HL_ANIM_KEY_FORMAT_INT:
            return hlINNNSubMotionWriteKeyframesRotA32(keyGroup, platform, file);
        }

        /* Just so compilers don't complain; we should never even reach this. */
        return HL_ERROR_UNKNOWN;

    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_ROT */
    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_HIDE */
    /* TODO: Write Bezier, Spline, etc. stuff. */

    /* NN doesn't have a corresponding type for this; return failure. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionCameraWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_CAM_TYPE_TARGET_X:
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Y:
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Z:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_X:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z:
    case HL_ANIM_KEY_CAM_TYPE_FOVY:
    case HL_ANIM_KEY_CAM_TYPE_ZNEAR:
    case HL_ANIM_KEY_CAM_TYPE_ZFAR:
    case HL_ANIM_KEY_CAM_TYPE_ASPECT:
        return hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    case HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ:
        return hlINNNSubMotionWriteKeyframesVector(keyGroup, platform, file);

    /* TODO: Write HL_ANIM_KEY_CAM_TYPE_ROLL. */

    /* NN doesn't have a corresponding type for this; return failure. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionLightWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_R:
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_G:
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_B:
    case HL_ANIM_KEY_LIGHT_TYPE_ALPHA:
    case HL_ANIM_KEY_LIGHT_TYPE_INTENSITY:
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START:
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END:
        return hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB:
        return hlINNNSubMotionWriteKeyframesRGB(keyGroup, platform, file);

    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_ANGLE. */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_ANGLE. */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_RANGE. */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_RANGE. */

    /* NN doesn't have a corresponding type for this; return failure. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionMorphWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MORPH_TYPE_WEIGHT:
        return hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    /* NN doesn't have a corresponding type for this; return failure. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionMaterialWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlNNPlatform platform, HlFile* HL_RESTRICT file)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R:
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G:
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B:
    case HL_ANIM_KEY_MAT_TYPE_ALPHA:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_R:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_G:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_B:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_R:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_G:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_B:
    case HL_ANIM_KEY_MAT_TYPE_TEX_BLEND:
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U:
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V:
        return hlINNNSubMotionWriteKeyframesFloat(keyGroup, platform, file);

    case HL_ANIM_KEY_MAT_TYPE_TEX_INDEX:
        return hlINNNSubMotionWriteKeyframesInt(keyGroup, platform, file);

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return hlINNNSubMotionWriteKeyframesUInt(keyGroup, platform, file);

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV:
        return hlINNNSubMotionWriteKeyframesUV(keyGroup, platform, file);

    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB:
        return hlINNNSubMotionWriteKeyframesRGB(keyGroup, platform, file);

    /* TODO: Write HL_ANIM_KEY_MAT_TYPE_HIDE. */

    /* NN doesn't have a corresponding type for this; return failure. */
    default: return HL_ERROR_UNSUPPORTED;
    }
}

static HlResult hlINNNSubMotionWriteKeyframes(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    HlAnimationFlags animFlags, HlNNPlatform platform,
    HlFile* HL_RESTRICT file)
{
    switch (animFlags & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE:
        return hlINNNSubMotionBoneWriteKeyframes(keyGroup, platform, file);

    case HL_ANIM_TYPE_CAMERA:
        return hlINNNSubMotionCameraWriteKeyframes(keyGroup, platform, file);

    case HL_ANIM_TYPE_LIGHT:
        return hlINNNSubMotionLightWriteKeyframes(keyGroup, platform, file);

    case HL_ANIM_TYPE_MORPH:
        return hlINNNSubMotionMorphWriteKeyframes(keyGroup, platform, file);

    case HL_ANIM_TYPE_MATERIAL:
        return hlINNNSubMotionMaterialWriteKeyframes(keyGroup, platform, file);

    default: return HL_ERROR_UNKNOWN;
    }
}

static HlU32 hlINNNSubMotionBoneGetType(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_BONE_TYPE_POS_X:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TRANSLATION_X);

    case HL_ANIM_KEY_BONE_TYPE_POS_Y:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TRANSLATION_Y);

    case HL_ANIM_KEY_BONE_TYPE_POS_Z:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TRANSLATION_Z);

    case HL_ANIM_KEY_BONE_TYPE_POS_XYZ:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TRANSLATION_XYZ);

    case HL_ANIM_KEY_BONE_TYPE_ROT_X:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_RADIAN |
                HL_NN_SMOTTYPE_ROTATION_X);

        case HL_ANIM_KEY_FORMAT_SHORT:
            return (HL_NN_SMOTTYPE_FRAME_SINT16 | HL_NN_SMOTTYPE_ANGLE_ANGLE16 |
                HL_NN_SMOTTYPE_ROTATION_X);

        case HL_ANIM_KEY_FORMAT_INT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_ANGLE32 |
                HL_NN_SMOTTYPE_ROTATION_X);
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_BONE_TYPE_ROT_Y:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_RADIAN |
                HL_NN_SMOTTYPE_ROTATION_Y);

        case HL_ANIM_KEY_FORMAT_SHORT:
            return (HL_NN_SMOTTYPE_FRAME_SINT16 | HL_NN_SMOTTYPE_ANGLE_ANGLE16 |
                HL_NN_SMOTTYPE_ROTATION_Y);

        case HL_ANIM_KEY_FORMAT_INT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_ANGLE32 |
                HL_NN_SMOTTYPE_ROTATION_Y);
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_BONE_TYPE_ROT_Z:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_RADIAN |
                HL_NN_SMOTTYPE_ROTATION_Z);

        case HL_ANIM_KEY_FORMAT_SHORT:
            return (HL_NN_SMOTTYPE_FRAME_SINT16 | HL_NN_SMOTTYPE_ANGLE_ANGLE16 |
                HL_NN_SMOTTYPE_ROTATION_Z);

        case HL_ANIM_KEY_FORMAT_INT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_ANGLE32 |
                HL_NN_SMOTTYPE_ROTATION_Z);
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_BONE_TYPE_ROT_XYZ:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_RADIAN |
                HL_NN_SMOTTYPE_ROTATION_XYZ);

        case HL_ANIM_KEY_FORMAT_SHORT:
            return (HL_NN_SMOTTYPE_FRAME_SINT16 | HL_NN_SMOTTYPE_ANGLE_ANGLE16 |
                HL_NN_SMOTTYPE_ROTATION_XYZ);

        case HL_ANIM_KEY_FORMAT_INT:
            return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ANGLE_ANGLE32 |
                HL_NN_SMOTTYPE_ROTATION_XYZ);
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_BONE_TYPE_SCALE_X:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SCALING_X);

    case HL_ANIM_KEY_BONE_TYPE_SCALE_Y:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SCALING_Y);

    case HL_ANIM_KEY_BONE_TYPE_SCALE_Z:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SCALING_Z);

    case HL_ANIM_KEY_BONE_TYPE_SCALE_XYZ:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SCALING_XYZ);

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT |
            (((flags & HL_ANIM_KEY_FORMAT_MASK) == HL_ANIM_KEY_FORMAT_UINT) ?
                HL_NN_SMOTTYPE_USER_UINT32 : HL_NN_SMOTTYPE_USER_FLOAT));

    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_ROT */
    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_HIDE */
    /* TODO: Write Bezier, Spline, etc. stuff. */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionCameraGetType(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_CAM_TYPE_TARGET_X:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TARGET_X);

    case HL_ANIM_KEY_CAM_TYPE_TARGET_Y:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TARGET_Y);

    case HL_ANIM_KEY_CAM_TYPE_TARGET_Z:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TARGET_Z);

    case HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TARGET_XYZ);

    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_X:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPTARGET_X);

    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPTARGET_Y);

    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPTARGET_Z);

    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPTARGET_XYZ);

    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPVECTOR_X);

    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPVECTOR_Y);

    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPVECTOR_Z);

    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_UPVECTOR_XYZ);

    case HL_ANIM_KEY_CAM_TYPE_FOVY:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_FOVY);

    case HL_ANIM_KEY_CAM_TYPE_ZNEAR:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ZNEAR);

    case HL_ANIM_KEY_CAM_TYPE_ZFAR:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ZFAR);

    case HL_ANIM_KEY_CAM_TYPE_ASPECT:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ASPECT);

    /* TODO: Write HL_ANIM_KEY_CAM_TYPE_ROLL */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionLightGetType(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_R:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_COLOR_R);

    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_G:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_COLOR_G);

    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_B:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_COLOR_B);

    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_COLOR_RGB);

    case HL_ANIM_KEY_LIGHT_TYPE_ALPHA:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_ALPHA);

    case HL_ANIM_KEY_LIGHT_TYPE_INTENSITY:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_LIGHT_INTENSITY);

    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_FALLOFF_START);

    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_FALLOFF_END);

    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_ANGLE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_ANGLE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_RANGE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_RANGE */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionMorphGetType(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MORPH_TYPE_WEIGHT:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_MORPH_WEIGHT);

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionMaterialGetType(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_DIFFUSE_R);

    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_DIFFUSE_G);

    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_DIFFUSE_B);

    case HL_ANIM_KEY_MAT_TYPE_ALPHA:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_ALPHA);

    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_DIFFUSE_RGB);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_R:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_R);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_G:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_G);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_B:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_B);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_RGB);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_LEVEL);

    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_SPECULAR_GLOSS);

    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_R:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_AMBIENT_R);

    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_G:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_AMBIENT_G);

    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_B:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_AMBIENT_B);

    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_AMBIENT_RGB);

    case HL_ANIM_KEY_MAT_TYPE_TEX_INDEX:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TEXTURE_INDEX);

    case HL_ANIM_KEY_MAT_TYPE_TEX_BLEND:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_TEXTURE_BLEND);

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_OFFSET_U);

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_OFFSET_V);

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_OFFSET_UV);

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return (HL_NN_SMOTTYPE_FRAME_FLOAT | HL_NN_SMOTTYPE_MATCLBK_USER);

    /* TODO: Write HL_ANIM_KEY_MAT_TYPE_HIDE */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionGetType(HlAnimationKeyGroupFlags flags,
    HlAnimationFlags animFlags)
{
    switch (animFlags & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE:
        return hlINNNSubMotionBoneGetType(flags);

    case HL_ANIM_TYPE_CAMERA:
        return hlINNNSubMotionCameraGetType(flags);

    case HL_ANIM_TYPE_LIGHT:
        return hlINNNSubMotionLightGetType(flags);

    case HL_ANIM_TYPE_MORPH:
        return hlINNNSubMotionMorphGetType(flags);

    case HL_ANIM_TYPE_MATERIAL:
        return hlINNNSubMotionMaterialGetType(flags);

    default: return 0;
    }
}

static HlU32 hlINNNSubMotionGetIPType(HlAnimationKeyGroupFlags flags)
{
    /* Compute interpolation type. */
    HlU32 iptype;
    switch (flags & HL_ANIM_KEY_IP_TYPE_MASK)
    {
    case HL_ANIM_KEY_IP_TYPE_LINEAR:
        iptype = HL_NN_SMOTIPTYPE_LINEAR;
        break;

    case HL_ANIM_KEY_IP_TYPE_CONSTANT:
        iptype = HL_NN_SMOTIPTYPE_CONSTANT;
        break;

    case HL_ANIM_KEY_IP_TYPE_BEZIER:
        iptype = HL_NN_SMOTIPTYPE_BEZIER;
        break;

    case HL_ANIM_KEY_IP_TYPE_SI_SPLINE:
        iptype = HL_NN_SMOTIPTYPE_SI_SPLINE;
        break;

    case HL_ANIM_KEY_IP_TYPE_TRIGGER:
        iptype = HL_NN_SMOTIPTYPE_TRIGGER;
        break;

    default:
        iptype = HL_NN_SMOTIPTYPE_SPLINE;
        break;
    }

    /* Compute loop type. */
    switch (flags & HL_ANIM_KEY_LOOP_TYPE_MASK)
    {
    default:
    case HL_ANIM_KEY_LOOP_TYPE_NOREPEAT:
        iptype |= HL_NN_SMOTIPTYPE_NOREPEAT;
        break;

    case HL_ANIM_KEY_LOOP_TYPE_CONSTREPEAT:
        iptype |= HL_NN_SMOTIPTYPE_CONSTREPEAT;
        break;

    case HL_ANIM_KEY_LOOP_TYPE_REPEAT:
        iptype |= HL_NN_SMOTIPTYPE_REPEAT;
        break;

    case HL_ANIM_KEY_LOOP_TYPE_MIRROR:
        iptype |= HL_NN_SMOTIPTYPE_MIRROR;
        break;

    case HL_ANIM_KEY_LOOP_TYPE_OFFSET:
        iptype |= HL_NN_SMOTIPTYPE_OFFSET;
        break;
    }

    return iptype;
}

static HlU32 hlINNNSubMotionBoneGetKeySize(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_BONE_TYPE_POS_X:
    case HL_ANIM_KEY_BONE_TYPE_POS_Y:
    case HL_ANIM_KEY_BONE_TYPE_POS_Z:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_X:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Y:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Z:
        return (sizeof(float) + sizeof(float));

    case HL_ANIM_KEY_BONE_TYPE_ROT_X:
    case HL_ANIM_KEY_BONE_TYPE_ROT_Y:
    case HL_ANIM_KEY_BONE_TYPE_ROT_Z:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_SHORT:
            return sizeof(HlNNMotionKeySISplineShort);

        case HL_ANIM_KEY_FORMAT_INT:
            return sizeof(HlNNMotionKeySISplineInt);

        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return sizeof(HlNNMotionKeySISpline);
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_BONE_TYPE_POS_XYZ:
    case HL_ANIM_KEY_BONE_TYPE_SCALE_XYZ:
        return (sizeof(float) + (sizeof(float) * 3));

    case HL_ANIM_KEY_BONE_TYPE_ROT_XYZ:
        switch (flags & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_SHORT:
            return (sizeof(HlS16) * 4);

        case HL_ANIM_KEY_FORMAT_INT:
            return (sizeof(float) + (sizeof(HlS32) * 3));

        default:
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return (sizeof(float) + (sizeof(float) * 3));
        }
        return 0; /* Just so the compiler doesn't complain; we should never reach this. */

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return (sizeof(float) + (((flags & HL_ANIM_KEY_FORMAT_MASK) ==
            HL_ANIM_KEY_FORMAT_UINT) ? sizeof(HlU32) : sizeof(float)));

    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_ROT */
    /* TODO: Write HL_ANIM_KEY_BONE_TYPE_HIDE */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionCameraGetKeySize(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_CAM_TYPE_TARGET_X:
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Y:
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Z:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_X:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z:
    case HL_ANIM_KEY_CAM_TYPE_FOVY:
    case HL_ANIM_KEY_CAM_TYPE_ZNEAR:
    case HL_ANIM_KEY_CAM_TYPE_ZFAR:
    case HL_ANIM_KEY_CAM_TYPE_ASPECT:
        return (sizeof(float) + sizeof(float));

    case HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ:
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ:
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ:
        return (sizeof(float) + (sizeof(float) * 3));

    /* TODO: Write HL_ANIM_KEY_CAM_TYPE_ROLL */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionLightGetKeySize(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_R:
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_G:
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_B:
    case HL_ANIM_KEY_LIGHT_TYPE_ALPHA:
    case HL_ANIM_KEY_LIGHT_TYPE_INTENSITY:
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START:
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END:
        return (sizeof(float) + sizeof(float));

    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB:
        return (sizeof(float) + (sizeof(float) * 3));

    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_ANGLE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_ANGLE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_INNER_RANGE */
    /* TODO: Write HL_ANIM_KEY_LIGHT_TYPE_OUTER_RANGE */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionMorphGetKeySize(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MORPH_TYPE_WEIGHT:
        return (sizeof(float) + sizeof(float));

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionMaterialGetKeySize(HlAnimationKeyGroupFlags flags)
{
    switch (flags & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R:
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G:
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B:
    case HL_ANIM_KEY_MAT_TYPE_ALPHA:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_R:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_G:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_B:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_R:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_G:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_B:
    case HL_ANIM_KEY_MAT_TYPE_TEX_BLEND:
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U:
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V:
        return (sizeof(float) + sizeof(float));

    case HL_ANIM_KEY_MAT_TYPE_TEX_INDEX:
        return (sizeof(float) + sizeof(HlS32));

    case HL_ANIM_KEY_TYPE_CUSTOM:
        return (sizeof(float) + sizeof(HlU32));

    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV:
        return (sizeof(float) + (sizeof(float) * 2));

    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB:
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB:
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB:
        return (sizeof(float) + (sizeof(float) * 3));

    /* TODO: Write HL_ANIM_KEY_MAT_TYPE_HIDE */

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }
}

static HlU32 hlINNNSubMotionGetKeySize(HlAnimationKeyGroupFlags flags,
    HlAnimationFlags animFlags)
{
    switch (animFlags & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE:
        return hlINNNSubMotionBoneGetKeySize(flags);

    case HL_ANIM_TYPE_CAMERA:
        return hlINNNSubMotionCameraGetKeySize(flags);

    case HL_ANIM_TYPE_LIGHT:
        return hlINNNSubMotionLightGetKeySize(flags);

    case HL_ANIM_TYPE_MORPH:
        return hlINNNSubMotionMorphGetKeySize(flags);

    case HL_ANIM_TYPE_MATERIAL:
        return hlINNNSubMotionMaterialGetKeySize(flags);

    default: return 0;
    }
}

static HlU32 hlINNNMotionGetType(HlAnimationFlags flags)
{
    /* Compute type. */
    HlU32 type;
    switch (flags & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE:
        type = HL_NN_MOTIONTYPE_NODE;
        break;

    case HL_ANIM_TYPE_CAMERA:
        type = HL_NN_MOTIONTYPE_CAMERA;
        break;

    case HL_ANIM_TYPE_LIGHT:
        type = HL_NN_MOTIONTYPE_LIGHT;
        break;

    case HL_ANIM_TYPE_MORPH:
        type = HL_NN_MOTIONTYPE_MORPH;
        break;

    case HL_ANIM_TYPE_MATERIAL:
        type = HL_NN_MOTIONTYPE_MATERIAL;
        break;

    /* NN doesn't have a corresponding type for this; return 0. */
    default: return 0;
    }

    /* Compute loop type. */
    switch (flags & HL_ANIM_LOOP_TYPE_MASK)
    {
    case HL_ANIM_LOOP_TYPE_CONSTREPEAT:
        type |= HL_NN_MOTIONTYPE_CONSTREPEAT;
        break;

    case HL_ANIM_LOOP_TYPE_REPEAT:
        type |= HL_NN_MOTIONTYPE_REPEAT;
        break;

    case HL_ANIM_LOOP_TYPE_MIRROR:
        type |= HL_NN_MOTIONTYPE_MIRROR;
        break;

    case HL_ANIM_LOOP_TYPE_OFFSET:
        type |= HL_NN_MOTIONTYPE_OFFSET;
        break;

    case HL_ANIM_LOOP_TYPE_TRIGGER:
        type |= HL_NN_MOTIONTYPE_TRIGGER;
        break;

    default:
        type |= HL_NN_MOTIONTYPE_NOREPEAT;
        break;
    }

    /* Add version 2 flag. */
    /* TODO: Allow user to specify whether or not they want this flag. */
    type |= HL_NN_MOTIONTYPE_VERSION2;

    return type;
}

static HlNNCnkID hlINNNMotionGetChunkID(const HlAnimation* anim)
{
    switch (anim->flagsAndType & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE: return HL_NN_ID_MOTION;
    case HL_ANIM_TYPE_LIGHT: return HL_NN_ID_MOTION_LIGHT;
    case HL_ANIM_TYPE_CAMERA: return HL_NN_ID_MOTION_CAMERA;
    case HL_ANIM_TYPE_MORPH: return HL_NN_ID_MOTION_MORPH;
    case HL_ANIM_TYPE_MATERIAL: return HL_NN_ID_MOTION_MATERIAL;
    default: return 0;
    }
}

HlResult hlNNMotionWrite(const HlAnimation* HL_RESTRICT anim, size_t dataPos,
    HlNNPlatform platform, HlOffTable* HL_RESTRICT offTable, HlFile* HL_RESTRICT file)
{
    /* Create NN header. */
    HlNNBinCnkDataHeader header =
    {
        hlINNNMotionGetChunkID(anim)
    };

    size_t cnkPos, keyframeOff, motPos, i;
    HlResult result;

    /* Ensure chunk ID is not 0. */
    if (!header.id) return HL_ERROR_UNKNOWN;

    /* Make chunk ID platform-specific. */
    header.id = hlNNPlatformMakeCnkID(header.id, platform);

    /* Get chunk position. */
    cnkPos = hlFileTell(file);

    /* Write motion chunk header. */
    result = hlFileWrite(file, sizeof(header), &header, 0);
    if (HL_FAILED(result)) return result;

    /* Write NN SubMotion Keyframes. */
    for (i = 0; i < anim->keyGroupCount; ++i)
    {
        /* Convert keyframes based on key group format. */
        if (anim->keyGroups[i].flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
        {
            /* TODO */
            /*result = hlINNNSubMotionWriteKeyframesIP(&anim->keyGroups[i],
                anim->flagsAndType, platform, file);*/
            return HL_ERROR_UNKNOWN;
        }
        else
        {
            result = hlINNNSubMotionWriteKeyframes(&anim->keyGroups[i],
                anim->flagsAndType, platform, file);
        }
        
        /* Return if an error was encountered. */
        if (HL_FAILED(result)) return result;
    }

    /* Write NN SubMotions. */
    keyframeOff = (cnkPos + sizeof(header));
    for (i = 0; i < anim->keyGroupCount; ++i)
    {
        /* Generate NN SubMotion. */
        HlNNSubMotion subMot;
        subMot.type = hlINNNSubMotionGetType(
            anim->keyGroups[i].flagsAndType, anim->flagsAndType);

        subMot.ipType = hlINNNSubMotionGetIPType(anim->keyGroups[i].flagsAndType);
        subMot.id = (HlS32)anim->keyGroups[i].index;

        if (anim->keyGroups[i].subIndex)
        {
            subMot.id = ((subMot.id & 0xFFFFU) |
                ((HlS32)((HlS16)anim->keyGroups[i].subIndex) << 16));
        }

        subMot.startFrame = anim->keyGroups[i].startFrame;
        subMot.endFrame = anim->keyGroups[i].endFrame;
        subMot.startKeyframe = anim->keyGroups[i].startKeyframe;
        subMot.endKeyframe = anim->keyGroups[i].endKeyframe;
        subMot.keyframeCount = (HlU32)anim->keyGroups[i].keyframeCount;

        /* Compute keySize. */
        subMot.keySize = hlINNNSubMotionGetKeySize(
            anim->keyGroups[i].flagsAndType, anim->flagsAndType);

        if (anim->keyGroups[i].flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
        {
            subMot.keySize += (sizeof(HlVector2) * 2);
        }

        /* Set NN SubMotion keyframes offset. */
        subMot.keyframes = (HlU32)(keyframeOff - dataPos);

        /* Add offset to offset table. */
        result = HL_LIST_PUSH(*offTable, hlFileTell(file) + 36);
        if (HL_FAILED(result)) return result;

        /* Increase keyframe offset. */
        keyframeOff += ((size_t)subMot.keySize * (size_t)subMot.keyframeCount);

        /* Endian swap if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlNNSubMotionSwap(&subMot, 1);
        }

        /* Write NN SubMotion. */
        result = hlFileWrite(file, sizeof(subMot), &subMot, 0);
        if (HL_FAILED(result)) return result;
    }

    /* Write NN Motion. */
    {
        /* Generate NN motion. */
        HlNNMotion motion =
        {
            hlINNNMotionGetType(anim->flagsAndType),    /* type */
            anim->startFrame,                           /* startFrame */
            anim->endFrame,                             /* endFrame */
            (HlU32)anim->keyGroupCount,                 /* subMotionCount */
            (HlU32)(keyframeOff - dataPos),             /* subMotions */
            anim->framerate                             /* framerate */
        };

        /* Get NN Motion position. */
        motPos = hlFileTell(file);

        /* Add offset to offset table. */
        result = HL_LIST_PUSH(*offTable, motPos + 16);
        if (HL_FAILED(result)) return result;

        /* Endian swap if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlNNMotionSwap(&motion, 1);
        }

        /* Write NN Motion. */
        result = hlFileWrite(file, sizeof(motion), &motion, 0);
        if (HL_FAILED(result)) return result;
    }

    /* Fix padding. */
    result = hlFilePad(file, hlNNPlatformGetPadSize(platform));
    if (HL_FAILED(result)) return result;

    /* Fill-in motion chunk header. */
    {
        /* Get end of file. */
        size_t eof = hlFileTell(file);

        /* Jump to motion chunk. */
        result = hlFileJumpTo(file, cnkPos + 4);
        if (HL_FAILED(result)) return result;

        /* Generate motion chunk header values. */
        header.nextIDOffset   = (HlU32)(eof - (cnkPos + 8));
        header.mainDataOffset = (HlU32)(motPos - dataPos);

        /* Endian swap if necessary for the given platform. */
        if (hlNNPlatformNeedsSwap(platform))
        {
            hlNNDataHeaderSwap(&header);
        }

        /* Fill-in motion chunk header. */
        result = hlFileWrite(file, 8, &header.nextIDOffset, 0);
        if (HL_FAILED(result)) return result;

        /* Jump to end of file. */
        result = hlFileJumpTo(file, eof);
    }

    return result;
}
