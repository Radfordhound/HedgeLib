#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_file.h"
#include "hedgelib/animations/hl_animation.h"
#include "../depends/cJSON/cJSON.h"

const char* const hlAnimationKeyGroupGetFormatStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
    {
    case HL_ANIM_KEY_FORMAT_FLOAT: return "FLOAT";
    case HL_ANIM_KEY_FORMAT_INDEX: return "INDEX";
    case HL_ANIM_KEY_FORMAT_SHORT: return "SHORT";
    case HL_ANIM_KEY_FORMAT_USHORT: return "USHORT";
    case HL_ANIM_KEY_FORMAT_INT: return "INT";
    case HL_ANIM_KEY_FORMAT_UINT: return "UINT";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetDimensionStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D: return "1D";
    case HL_ANIM_KEY_DIMENSION_2D: return "2D";
    case HL_ANIM_KEY_DIMENSION_3D: return "3D";
    case HL_ANIM_KEY_DIMENSION_4D: return "4D";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetBoneTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_BONE_TYPE_POS_X: return "POS_X";
    case HL_ANIM_KEY_BONE_TYPE_POS_Y: return "POS_Y";
    case HL_ANIM_KEY_BONE_TYPE_POS_Z: return "POS_Z";
    case HL_ANIM_KEY_BONE_TYPE_POS_XYZ: return "POS_XYZ";
    case HL_ANIM_KEY_BONE_TYPE_ROT_X: return "ROT_X";
    case HL_ANIM_KEY_BONE_TYPE_ROT_Y: return "ROT_Y";
    case HL_ANIM_KEY_BONE_TYPE_ROT_Z: return "ROT_Z";
    case HL_ANIM_KEY_BONE_TYPE_ROT_XYZ: return "ROT_XYZ";
    case HL_ANIM_KEY_BONE_TYPE_ROT: return "ROT";
    case HL_ANIM_KEY_BONE_TYPE_SCALE_X: return "SCALE_X";
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Y: return "SCALE_Y";
    case HL_ANIM_KEY_BONE_TYPE_SCALE_Z: return "SCALE_Z";
    case HL_ANIM_KEY_BONE_TYPE_SCALE_XYZ: return "SCALE_XYZ";
    case HL_ANIM_KEY_BONE_TYPE_HIDE: return "HIDE";
    case HL_ANIM_KEY_TYPE_CUSTOM: return "CUSTOM";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetCameraTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_CAM_TYPE_TARGET_X: return "TARGET_X";
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Y: return "TARGET_Y";
    case HL_ANIM_KEY_CAM_TYPE_TARGET_Z: return "TARGET_Z";
    case HL_ANIM_KEY_CAM_TYPE_TARGET_XYZ: return "TARGET_XYZ";
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_X: return "UPTARGET_X";
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Y: return "UPTARGET_Y";
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_Z: return "UPTARGET_Z";
    case HL_ANIM_KEY_CAM_TYPE_UPTARGET_XYZ: return "UPTARGET_XYZ";
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_X: return "UPVECTOR_X";
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Y: return "UPVECTOR_Y";
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_Z: return "UPVECTOR_Z";
    case HL_ANIM_KEY_CAM_TYPE_UPVECTOR_XYZ: return "UPVECTOR_XYZ";
    case HL_ANIM_KEY_CAM_TYPE_FOVY: return "FOVY";
    case HL_ANIM_KEY_CAM_TYPE_ZNEAR: return "ZNEAR";
    case HL_ANIM_KEY_CAM_TYPE_ZFAR: return "ZFAR";
    case HL_ANIM_KEY_CAM_TYPE_ASPECT: return "ASPECT";
    case HL_ANIM_KEY_CAM_TYPE_ROLL: return "ROLL";
    case HL_ANIM_KEY_TYPE_CUSTOM: return "CUSTOM";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetLightTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_R: return "COLOR_R";
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_G: return "COLOR_G";
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_B: return "COLOR_B";
    case HL_ANIM_KEY_LIGHT_TYPE_COLOR_RGB: return "COLOR_RGB";
    case HL_ANIM_KEY_LIGHT_TYPE_ALPHA: return "ALPHA";
    case HL_ANIM_KEY_LIGHT_TYPE_INTENSITY: return "INTENSITY";
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_START: return "FALLOFF_START";
    case HL_ANIM_KEY_LIGHT_TYPE_FALLOFF_END: return "FALLOFF_END";
    case HL_ANIM_KEY_LIGHT_TYPE_INNER_ANGLE: return "INNER_ANGLE";
    case HL_ANIM_KEY_LIGHT_TYPE_OUTER_ANGLE: return "OUTER_ANGLE";
    case HL_ANIM_KEY_LIGHT_TYPE_INNER_RANGE: return "INNER_RANGE";
    case HL_ANIM_KEY_LIGHT_TYPE_OUTER_RANGE: return "OUTER_RANGE";
    case HL_ANIM_KEY_TYPE_CUSTOM: return "CUSTOM";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetMorphTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MORPH_TYPE_WEIGHT: return "WEIGHT";
    case HL_ANIM_KEY_TYPE_CUSTOM: return "CUSTOM";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetMaterialTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_TYPE_MASK)
    {
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_R: return "DIFFUSE_R";
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_G: return "DIFFUSE_G";
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_B: return "DIFFUSE_B";
    case HL_ANIM_KEY_MAT_TYPE_ALPHA: return "ALPHA";
    case HL_ANIM_KEY_MAT_TYPE_DIFFUSE_RGB: return "DIFFUSE_RGB";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_R: return "SPECULAR_R";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_G: return "SPECULAR_G";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_B: return "SPECULAR_B";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_RGB: return "SPECULAR_RGB";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_LEVEL: return "SPECULAR_LEVEL";
    case HL_ANIM_KEY_MAT_TYPE_SPECULAR_GLOSS: return "SPECULAR_GLOSS";
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_R: return "AMBIENT_R";
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_G: return "AMBIENT_G";
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_B: return "AMBIENT_B";
    case HL_ANIM_KEY_MAT_TYPE_AMBIENT_RGB: return "AMBIENT_RGB";
    case HL_ANIM_KEY_MAT_TYPE_TEX_INDEX: return "TEX_INDEX";
    case HL_ANIM_KEY_MAT_TYPE_TEX_BLEND: return "TEX_BLEND";
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_U: return "TEXCOORD_U";
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_V: return "TEXCOORD_V";
    case HL_ANIM_KEY_MAT_TYPE_TEXCOORD_UV: return "TEXCOORD_UV";
    case HL_ANIM_KEY_MAT_TYPE_HIDE: return "HIDE";
    case HL_ANIM_KEY_TYPE_CUSTOM: return "CUSTOM";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetIPTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_IP_TYPE_MASK)
    {
    case HL_ANIM_KEY_IP_TYPE_SPLINE: return "SPLINE";
    case HL_ANIM_KEY_IP_TYPE_LINEAR: return "LINEAR";
    case HL_ANIM_KEY_IP_TYPE_CONSTANT: return "CONSTANT";
    case HL_ANIM_KEY_IP_TYPE_TRIGGER: return "TRIGGER";
    case HL_ANIM_KEY_IP_TYPE_BEZIER: return "BEZIER";
    case HL_ANIM_KEY_IP_TYPE_SI_SPLINE: return "SI_SPLINE";
    default: return NULL;
    }
}

const char* const hlAnimationKeyGroupGetLoopTypeStr(const HlAnimationKeyGroup* keyGroup)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_LOOP_TYPE_MASK)
    {
    case HL_ANIM_KEY_LOOP_TYPE_NOREPEAT: return "NOREPEAT";
    case HL_ANIM_KEY_LOOP_TYPE_CONSTREPEAT: return "CONSTREPEAT";
    case HL_ANIM_KEY_LOOP_TYPE_REPEAT: return "REPEAT";
    case HL_ANIM_KEY_LOOP_TYPE_MIRROR: return "MIRROR";
    case HL_ANIM_KEY_LOOP_TYPE_OFFSET: return "OFFSET";
    default: return NULL;
    }
}

const char* const hlAnimationGetTypeStr(const HlAnimation* anim)
{
    switch (anim->flagsAndType & HL_ANIM_TYPE_MASK)
    {
    case HL_ANIM_TYPE_BONE: return "BONE";
    case HL_ANIM_TYPE_CAMERA: return "CAMERA";
    case HL_ANIM_TYPE_LIGHT: return "LIGHT";
    case HL_ANIM_TYPE_MORPH: return "MORPH";
    case HL_ANIM_TYPE_MATERIAL: return "MATERIAL";
    default: return NULL;
    }
}

const char* const hlAnimationGetLoopTypeStr(const HlAnimation* anim)
{
    switch (anim->flagsAndType & HL_ANIM_LOOP_TYPE_MASK)
    {
    case HL_ANIM_LOOP_TYPE_NOREPEAT: return "NOREPEAT";
    case HL_ANIM_LOOP_TYPE_CONSTREPEAT: return "CONSTREPEAT";
    case HL_ANIM_LOOP_TYPE_REPEAT: return "REPEAT";
    case HL_ANIM_LOOP_TYPE_MIRROR: return "MIRROR";
    case HL_ANIM_LOOP_TYPE_OFFSET: return "OFFSET";
    case HL_ANIM_LOOP_TYPE_TRIGGER: return "TRIGGER";
    default: return NULL;
    }
}

#define HL_INJSON_GET_NUMBER(value, type, name)\
    jitem = cJSON_GetObjectItemCaseSensitive(json, name);\
    if (!jitem || !cJSON_IsNumber(jitem)) return NULL;\
    value = (type)jitem->valuedouble

static HlAnimation* hlINAnimationKeyGroupParseJSON(const cJSON* json, HlAnimation* animBuf)
{
    size_t i;
    for (i = 0; i < animBuf->keyGroupCount; ++i)
    {
        const cJSON* jitem;

        /* Parse key group. */
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].flagsAndType,
            HlAnimationKeyGroupFlags, "flagsAndType");

        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].startFrame, float, "startFrame");
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].endFrame, float, "endFrame");
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].startKeyframe, float, "startKeyframe");
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].endKeyframe, float, "endKeyframe");
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].index, size_t, "index");
        HL_INJSON_GET_NUMBER(animBuf->keyGroups[i].subIndex, size_t, "subIndex");

        /* Get keys. */
        jitem = cJSON_GetObjectItemCaseSensitive(json, "keyframes");
        if (!jitem || !cJSON_IsArray(jitem)) return NULL;

        animBuf->keyGroups[i].keyframeCount = (size_t)cJSON_GetArraySize(jitem);
        /* TODO */
    }

    /* TODO */
    return NULL;
}

static HlAnimation* hlINAnimationParseJSON(const cJSON* json)
{
    HlAnimation anim;
    const cJSON* jitem;

    /* Parse animation. */
    HL_INJSON_GET_NUMBER(anim.flagsAndType, HlAnimationFlags, "flagsAndType");
    HL_INJSON_GET_NUMBER(anim.framerate, float, "framerate");
    HL_INJSON_GET_NUMBER(anim.startFrame, float, "startFrame");
    HL_INJSON_GET_NUMBER(anim.endFrame, float, "endFrame");

    /* Get key groups. */
    jitem = cJSON_GetObjectItemCaseSensitive(json, "keyGroups");
    if (!jitem || !cJSON_IsArray(jitem)) return NULL;

    anim.keyGroupCount = (size_t)cJSON_GetArraySize(jitem);

    /* Allocate memory for animation and animation key groups. */
    {
        HlAnimation* retBuf;
        HlAnimation* animBuf = (HlAnimation*)hlAlloc(sizeof(HlAnimation) +
            (sizeof(HlAnimationKeyGroup) * anim.keyGroupCount));

        if (!animBuf) return NULL;

        /* Set key groups pointer. */
        anim.keyGroups = (HlAnimationKeyGroup*)HL_ADD_OFF(animBuf, sizeof(HlAnimation));

        /* Parse JSON key groups. */
        retBuf = hlINAnimationKeyGroupParseJSON(jitem, animBuf);
        if (!retBuf) hlFree(animBuf);
        return retBuf;
    }
}

HlAnimation* hlAnimationReadJSON(const HlBlob* blob)
{
    cJSON* json = cJSON_Parse(blob->data);
    if (!json) return NULL;

    /* TODO */
    return NULL;
}

#define HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type, generator)\
    static HlResult hlINAnimationKey##type##GenerateJSON(\
        const HlAnimationKey##type* HL_RESTRICT keys, size_t keyCount,\
        cJSON* HL_RESTRICT jkeysArr)\
{\
    size_t i;\
    for (i = 0; i < keyCount; ++i)\
    {\
        /* Create JSON object for this key. */\
        cJSON* jkeyObj = cJSON_CreateObject();\
        if (!jkeyObj) return HL_ERROR_UNKNOWN;\
\
        /* Add JSON object to JSON keys array. */\
        if (!cJSON_AddItemToArray(jkeysArr, jkeyObj))\
        {\
            cJSON_Delete(jkeyObj);\
            return HL_ERROR_UNKNOWN;\
        }\
\
        /* From this point on the object is managed by the array; we don't have to delete it. */\
\
        /* Generate JSON for this key. */\
        if (!cJSON_AddNumberToObject(jkeyObj, "frame",\
            (double)keys[i].frame)) return HL_ERROR_UNKNOWN;\
\
        generator;\
    }\
    return HL_RESULT_SUCCESS;\
}

/* TODO: Make this its own function that just adds a Vector2? */
#define HL_INANIM_KEY_GENERATOR_IP_DO_ONE_JSON(value){\
    /* Create JSON object for this key's IP data. */\
    cJSON* jkeyIPVecObj = cJSON_CreateObject();\
    if (!jkeyIPVecObj) return HL_ERROR_UNKNOWN;\
\
    /* Add JSON object to JSON key IP object. */\
    if (!cJSON_AddItemToObject(jkeyIPObj, #value, jkeyIPVecObj))\
    {\
        cJSON_Delete(jkeyIPVecObj);\
        return HL_ERROR_UNKNOWN;\
    }\
\
    /* Add numbers to JSON object. */\
    if (!cJSON_AddNumberToObject(jkeyIPVecObj, "x",\
        (double)keys[i].ip.value.x)) return HL_ERROR_UNKNOWN;\
\
    if (!cJSON_AddNumberToObject(jkeyIPVecObj, "y",\
        (double)keys[i].ip.value.y)) return HL_ERROR_UNKNOWN;\
}

#define HL_INANIM_KEY_GENERATOR_IP_JSON {\
    /* Create JSON object for this key's IP data. */\
    cJSON* jkeyIPObj = cJSON_CreateObject();\
    if (!jkeyIPObj) return HL_ERROR_UNKNOWN;\
\
    /* Add JSON object to JSON key object. */\
    if (!cJSON_AddItemToObject(jkeyObj, "ip", jkeyIPObj))\
    {\
        cJSON_Delete(jkeyIPObj);\
        return HL_ERROR_UNKNOWN;\
    }\
\
    /* Generate IP vectors. */\
    HL_INANIM_KEY_GENERATOR_IP_DO_ONE_JSON(in)\
    HL_INANIM_KEY_GENERATOR_IP_DO_ONE_JSON(out)\
}

#define HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(dimIndex)\
    /* Create JSON object for this key's values. */\
    jkeyValueObj = cJSON_CreateNumber((double)keys[i].value[dimIndex]);\
    if (!jkeyValueObj) return HL_ERROR_UNKNOWN;\
\
    /* Add JSON object to JSON key values array. */\
    if (!cJSON_AddItemToArray(jkeyValueArr, jkeyValueObj))\
    {\
        cJSON_Delete(jkeyValueObj);\
        return HL_ERROR_UNKNOWN;\
    }

#define HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(dimIndex)\
    HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(dimIndex)\
    HL_INANIM_KEY_GENERATOR_IP_JSON

#define HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(code) {\
    cJSON* jkeyValueArr;\
    cJSON* jkeyValueObj;\
\
    /* Create JSON array for this key's value. */\
    jkeyValueArr = cJSON_CreateArray();\
    if (!jkeyValueArr) return HL_ERROR_UNKNOWN;\
\
    /* Add JSON array to JSON key. */\
    if (!cJSON_AddItemToObject(jkeyObj, "value", jkeyValueArr))\
    {\
        cJSON_Delete(jkeyValueArr);\
        return HL_ERROR_UNKNOWN;\
    }\
\
    code;\
}

#define HL_INANIM_KEY_GENERATOR_1D_JSON\
    if (!cJSON_AddNumberToObject(jkeyObj, "value",\
        (double)keys[i].value)) return HL_ERROR_UNKNOWN;

#define HL_INANIM_KEY_GENERATOR_2D_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(1))

#define HL_INANIM_KEY_GENERATOR_3D_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(1)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(2))

#define HL_INANIM_KEY_GENERATOR_4D_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(1)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(2)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM(3))

#define HL_INANIM_KEY_GENERATOR_1D_IP_JSON\
    HL_INANIM_KEY_GENERATOR_1D_JSON\
    HL_INANIM_KEY_GENERATOR_IP_JSON

#define HL_INANIM_KEY_GENERATOR_2D_IP_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(1))

#define HL_INANIM_KEY_GENERATOR_3D_IP_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(1)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(2))

#define HL_INANIM_KEY_GENERATOR_4D_IP_JSON\
    HL_INANIM_KEY_GENERATOR_MULTI_DIM_JSON(\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(0)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(1)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(2)\
        HL_INANIM_KEY_GENERATOR_DO_ONE_DIM_IP(3))

#define HL_INANIM_KEY_GENERATOR_FUNC_JSON(type)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type, HL_INANIM_KEY_GENERATOR_1D_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##2, HL_INANIM_KEY_GENERATOR_2D_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##3, HL_INANIM_KEY_GENERATOR_3D_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##4, HL_INANIM_KEY_GENERATOR_4D_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##IP, HL_INANIM_KEY_GENERATOR_1D_IP_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##IP##2, HL_INANIM_KEY_GENERATOR_2D_IP_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##IP##3, HL_INANIM_KEY_GENERATOR_3D_IP_JSON)\
    HL_INANIM_KEY_GENERATOR_FUNC_JSON_IN(type##IP##4, HL_INANIM_KEY_GENERATOR_4D_IP_JSON)

/* Generate JSON generator functions for each format/dimension combo. */
HL_INANIM_KEY_GENERATOR_FUNC_JSON(Float)
HL_INANIM_KEY_GENERATOR_FUNC_JSON(Index)
HL_INANIM_KEY_GENERATOR_FUNC_JSON(Short)
HL_INANIM_KEY_GENERATOR_FUNC_JSON(UShort)
HL_INANIM_KEY_GENERATOR_FUNC_JSON(Int)
HL_INANIM_KEY_GENERATOR_FUNC_JSON(UInt)

#define HL_INANIM_KEY_GENERATE_JSON(type)\
    hlINAnimationKey##type##GenerateJSON(\
        (const HlAnimationKey##type*)keyGroup->keyframes,\
        keyGroup->keyframeCount, jkeysArr)

static HlResult hlINAnimationKey1DGenerateJSON(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    cJSON * HL_RESTRICT jkeysArr)
{
    if (keyGroup->flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(FloatIP);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(IndexIP);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(ShortIP);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShortIP);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(IntIP);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UIntIP);

        default: return HL_ERROR_UNKNOWN;
        }
    }
    else
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(Float);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(Index);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(Short);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShort);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(Int);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UInt);

        default: return HL_ERROR_UNKNOWN;
        }
    }
}

static HlResult hlINAnimationKey2DGenerateJSON(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    cJSON* HL_RESTRICT jkeysArr)
{
    if (keyGroup->flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(FloatIP2);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(IndexIP2);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(ShortIP2);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShortIP2);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(IntIP2);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UIntIP2);

        default: return HL_ERROR_UNKNOWN;
        }
    }
    else
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(Float2);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(Index2);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(Short2);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShort2);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(Int2);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UInt2);

        default: return HL_ERROR_UNKNOWN;
        }
    }
}

static HlResult hlINAnimationKey3DGenerateJSON(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    cJSON* HL_RESTRICT jkeysArr)
{
    if (keyGroup->flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(FloatIP3);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(IndexIP3);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(ShortIP3);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShortIP3);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(IntIP3);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UIntIP3);

        default: return HL_ERROR_UNKNOWN;
        }
    }
    else
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(Float3);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(Index3);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(Short3);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShort3);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(Int3);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UInt3);

        default: return HL_ERROR_UNKNOWN;
        }
    }
}

static HlResult hlINAnimationKey4DGenerateJSON(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    cJSON* HL_RESTRICT jkeysArr)
{
    if (keyGroup->flagsAndType & HL_ANIM_KEY_HAS_IP_DATA)
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(FloatIP4);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(IndexIP4);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(ShortIP4);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShortIP4);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(IntIP4);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UIntIP4);

        default: return HL_ERROR_UNKNOWN;
        }
    }
    else
    {
        switch (keyGroup->flagsAndType & HL_ANIM_KEY_FORMAT_MASK)
        {
        case HL_ANIM_KEY_FORMAT_FLOAT:
            return HL_INANIM_KEY_GENERATE_JSON(Float4);
        case HL_ANIM_KEY_FORMAT_INDEX:
            return HL_INANIM_KEY_GENERATE_JSON(Index4);
        case HL_ANIM_KEY_FORMAT_SHORT:
            return HL_INANIM_KEY_GENERATE_JSON(Short4);
        case HL_ANIM_KEY_FORMAT_USHORT:
            return HL_INANIM_KEY_GENERATE_JSON(UShort4);
        case HL_ANIM_KEY_FORMAT_INT:
            return HL_INANIM_KEY_GENERATE_JSON(Int4);
        case HL_ANIM_KEY_FORMAT_UINT:
            return HL_INANIM_KEY_GENERATE_JSON(UInt4);

        default: return HL_ERROR_UNKNOWN;
        }
    }
}

static HlResult hlINAnimationKeyGenerateJSON(
    const HlAnimationKeyGroup* HL_RESTRICT keyGroup,
    cJSON* HL_RESTRICT jkeysArr)
{
    switch (keyGroup->flagsAndType & HL_ANIM_KEY_DIMENSION_MASK)
    {
    case HL_ANIM_KEY_DIMENSION_1D:
        return hlINAnimationKey1DGenerateJSON(keyGroup, jkeysArr);

    case HL_ANIM_KEY_DIMENSION_2D:
        return hlINAnimationKey2DGenerateJSON(keyGroup, jkeysArr);

    case HL_ANIM_KEY_DIMENSION_3D:
        return hlINAnimationKey3DGenerateJSON(keyGroup, jkeysArr);

    case HL_ANIM_KEY_DIMENSION_4D:
        return hlINAnimationKey4DGenerateJSON(keyGroup, jkeysArr);

    default: return HL_ERROR_UNKNOWN;
    }
}

static HlResult hlINAnimationKeyGroupGenerateJSON(
    const HlAnimation* HL_RESTRICT anim, cJSON* HL_RESTRICT jkeyGroupsArr)
{
    /* Generate JSON for key groups. */
    size_t i;
    for (i = 0; i < anim->keyGroupCount; ++i)
    {
        /* Create JSON object for this key group. */
        cJSON* jkeyGroupObj = cJSON_CreateObject();
        if (!jkeyGroupObj) return HL_ERROR_UNKNOWN;

        /* Add JSON object to JSON key groups array. */
        if (!cJSON_AddItemToArray(jkeyGroupsArr, jkeyGroupObj))
        {
            cJSON_Delete(jkeyGroupObj);
            return HL_ERROR_UNKNOWN;
        }
        
        /* From this point on the object is managed by the array; we don't have to delete it. */

        /* Generate JSON for this key group. */
        {
            const char* str;

            /* Write format. */
            str = hlAnimationKeyGroupGetFormatStr(&anim->keyGroups[i]);
            if (!str || !cJSON_AddStringToObject(jkeyGroupObj, "format", str))
                return HL_ERROR_UNKNOWN;

            /* Write dimension. */
            str = hlAnimationKeyGroupGetDimensionStr(&anim->keyGroups[i]);
            if (!str || !cJSON_AddStringToObject(jkeyGroupObj, "dimension", str))
                return HL_ERROR_UNKNOWN;

            /* Write type. */
            switch (anim->flagsAndType & HL_ANIM_TYPE_MASK)
            {
            case HL_ANIM_TYPE_BONE:
                str = hlAnimationKeyGroupGetBoneTypeStr(&anim->keyGroups[i]);
                break;

            case HL_ANIM_TYPE_CAMERA:
                str = hlAnimationKeyGroupGetCameraTypeStr(&anim->keyGroups[i]);
                break;

            case HL_ANIM_TYPE_LIGHT:
                str = hlAnimationKeyGroupGetLightTypeStr(&anim->keyGroups[i]);
                break;

            case HL_ANIM_TYPE_MORPH:
                str = hlAnimationKeyGroupGetMorphTypeStr(&anim->keyGroups[i]);
                break;

            case HL_ANIM_TYPE_MATERIAL:
                str = hlAnimationKeyGroupGetMaterialTypeStr(&anim->keyGroups[i]);
                break;

            default: return HL_ERROR_UNKNOWN;
            }

            if (!str || !cJSON_AddStringToObject(jkeyGroupObj, "type", str))
                return HL_ERROR_UNKNOWN;

            /* Write IP type. */
            str = hlAnimationKeyGroupGetIPTypeStr(&anim->keyGroups[i]);
            if (!str || !cJSON_AddStringToObject(jkeyGroupObj, "ipType", str))
                return HL_ERROR_UNKNOWN;

            /* Write loop type. */
            str = hlAnimationKeyGroupGetLoopTypeStr(&anim->keyGroups[i]);
            if (!str || !cJSON_AddStringToObject(jkeyGroupObj, "loopType", str))
                return HL_ERROR_UNKNOWN;
        }

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "startFrame",
            (double)anim->keyGroups[i].startFrame)) return HL_ERROR_UNKNOWN;

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "endFrame",
            (double)anim->keyGroups[i].endFrame)) return HL_ERROR_UNKNOWN;

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "startKeyframe",
            (double)anim->keyGroups[i].startKeyframe)) return HL_ERROR_UNKNOWN;

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "endKeyframe",
            (double)anim->keyGroups[i].endKeyframe)) return HL_ERROR_UNKNOWN;

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "index",
            (double)anim->keyGroups[i].index)) return HL_ERROR_UNKNOWN;

        if (!cJSON_AddNumberToObject(jkeyGroupObj, "subIndex",
            (double)anim->keyGroups[i].subIndex)) return HL_ERROR_UNKNOWN;
        
        {
            cJSON* jkeysArr;
            HlResult result;

            /* Create JSON array for keys. */
            jkeysArr = cJSON_CreateArray();
            if (!jkeysArr) return HL_ERROR_UNKNOWN;

            /* Add JSON array to JSON key group object. */
            if (!cJSON_AddItemToObject(jkeyGroupObj, "keyframes", jkeysArr))
            {
                cJSON_Delete(jkeysArr);
                return HL_ERROR_UNKNOWN;
            }

            /* Generate JSON for keys. */
            result = hlINAnimationKeyGenerateJSON(&anim->keyGroups[i], jkeysArr);
            if (HL_FAILED(result)) return result;
        }
    }

    return HL_RESULT_SUCCESS;
}

static HlResult hlINAnimationGenerateJSON(const HlAnimation* HL_RESTRICT anim,
    cJSON* HL_RESTRICT jrootObj)
{
    const char* str;

    /* Generate JSON for animation. */
    str = hlAnimationGetTypeStr(anim);
    if (!str || !cJSON_AddStringToObject(jrootObj, "type", str)) return HL_ERROR_UNKNOWN;

    str = hlAnimationGetLoopTypeStr(anim);
    if (!str || !cJSON_AddStringToObject(jrootObj, "loopType", str)) return HL_ERROR_UNKNOWN;

    if (!cJSON_AddNumberToObject(jrootObj, "framerate",
        (double)anim->framerate)) return HL_ERROR_UNKNOWN;

    if (!cJSON_AddNumberToObject(jrootObj, "startFrame",
        (double)anim->startFrame)) return HL_ERROR_UNKNOWN;

    if (!cJSON_AddNumberToObject(jrootObj, "endFrame",
        (double)anim->endFrame)) return HL_ERROR_UNKNOWN;

    /* Generate JSON for key groups. */
    {
        cJSON* jkeyGroupsArr = cJSON_AddArrayToObject(jrootObj, "keyGroups");
        if (!jkeyGroupsArr) return HL_ERROR_UNKNOWN;
        return hlINAnimationKeyGroupGenerateJSON(anim, jkeyGroupsArr);
    }
}

HlResult hlAnimationWriteJSON(const HlAnimation* HL_RESTRICT anim,
    HlStream* HL_RESTRICT stream)
{
    cJSON* jrootObj;
    HlResult result;

    /* Create root object.*/
    jrootObj = cJSON_CreateObject();
    if (!jrootObj) return HL_ERROR_UNKNOWN;

    /* Generate JSON. */
    result = hlINAnimationGenerateJSON(anim, jrootObj);
    if (HL_FAILED(result)) goto end;

    /* Write JSON to stream. */
    {
        char* jsonText;
        size_t size;

        /* Get the JSON text we just generated. */
        jsonText = cJSON_Print(jrootObj, &size);
        if (!jsonText || !size)
        {
            result = HL_ERROR_UNKNOWN;
            goto end;
        }

        /* Write JSON to stream. */
        result = hlStreamWrite(stream, (size - 1) * sizeof(char), jsonText, NULL);
        hlFree(jsonText);
    }

end:
    cJSON_Delete(jrootObj);
    return result;
}

HlAnimation* hlAnimationImportJSON(const HlNChar* filePath)
{
    /* TODO */
    return 0;
}

HlResult hlAnimationExportJSON(const HlAnimation* HL_RESTRICT anim,
    const HlNChar* HL_RESTRICT filePath)
{
    HlFileStream* file;
    HlResult result;

    /* Open file. */
    result = hlFileStreamOpen(filePath, HL_FILE_MODE_WRITE, &file);
    if (HL_FAILED(result)) return result;

    /* Write JSON to file, close it, and return. */
    result = hlAnimationWriteJSON(anim, file);
    if (HL_FAILED(result))
    {
        hlFileStreamClose(file);
        return result;
    }

    return hlFileStreamClose(file);
}
