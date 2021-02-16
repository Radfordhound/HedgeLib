#include "hedgelib/effects/hl_grif.h"
#include "hedgelib/io/hl_bina.h"
#include "hedgelib/hl_endian.h"

void hlGrifTextureParameterV106Swap(HlGrifTextureParameterV106* texture, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&texture->nameOffset);
    }

    hlSwapU32P(&texture->dataPtr);
    hlSwapU32P(&texture->uk1);
    hlSwapU32P(&texture->uk2);
}

void hlGrifMaterialParameterV106Swap(HlGrifMaterialParameterV106* material, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        hlSwapU32P(&material->nameOffset);
        hlSwapU32P(&material->shaderName);
        hlSwapU32P(&material->uk23);
    }

    hlGrifTextureParameterV106Swap(&material->textures[0], swapOffsets);
    hlGrifTextureParameterV106Swap(&material->textures[1], swapOffsets);
    hlSwapU32P(&material->texCount);
    hlSwapU32P(&material->uk2);
    hlSwapU32P(&material->uk3);
    hlSwapFloatP(&material->uk4);
    hlSwapU32P(&material->uk5);
    hlSwapU32P(&material->uk6);
    hlSwapU32P(&material->vertexShaderPtr);
    hlSwapU32P(&material->pixelShaderPtr);
    hlSwapU32P(&material->uk7);
    hlSwapU32P(&material->uk8);
    hlSwapU32P(&material->uk9);
    hlSwapU32P(&material->uk10);
    hlSwapU32P(&material->isEnableLightField);
    hlSwapU32P(&material->uk11);
    hlSwapU32P(&material->uk12);
    hlSwapU32P(&material->uk13);
    hlSwapU32P(&material->uk14);
    hlSwapU32P(&material->uk15);
    hlSwapU32P(&material->uk16);
    hlSwapU32P(&material->uk17);
    hlSwapU32P(&material->uk18);
    hlSwapU32P(&material->uk19);
    hlSwapU32P(&material->uk20);
    hlSwapU32P(&material->uk21);
    hlSwapU32P(&material->uk22);
    hlSwapU32P(&material->uk24);
    hlSwapU32P(&material->uk25);
    hlSwapU32P(&material->uk26);
}

void hlGrifParticleParameterV106Swap(HlGrifParticleParameterV106* particle, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        unsigned int i;

        hlSwapU32P(&particle->nameOffset);
        hlSwapU32P(&particle->materialOffset);
        hlSwapU32P(&particle->meshNameOffset);

        for (i = 0; i < 30; ++i)
        {
            hlSwapU32P(&particle->animOffsets[i]);
        }

        hlSwapU32P(&particle->nextParticleOffset);
    }

    hlSwapU32P(&particle->type);
    hlSwapFloatP(&particle->uk1);
    hlSwapU32P(&particle->uk2);
    hlSwapU32P(&particle->uk3);
    hlSwapFloatP(&particle->uk4);
    hlSwapFloatP(&particle->uk5);
    hlSwapFloatP(&particle->uk6);
    hlSwapFloatP(&particle->uk7);
    hlSwapFloatP(&particle->uk8);
    hlSwapU32P(&particle->uk9);
    hlSwapU32P(&particle->uk10);
    hlSwapFloatP(&particle->uk11);
    hlSwapFloatP(&particle->uk12);
    hlSwapFloatP(&particle->uk13);
    hlSwapFloatP(&particle->uk14);
    hlSwapFloatP(&particle->uk15);
    hlSwapFloatP(&particle->uk16);
    hlSwapFloatP(&particle->uk17);
    hlSwapFloatP(&particle->uk18);
    hlSwapU32P(&particle->uk19);
    hlSwapU32P(&particle->uk20);
    hlSwapU32P(&particle->uk21);
    hlSwapU32P(&particle->uk22);
    hlVector3Swap(&particle->uk23);
    hlSwapU32P(&particle->uk24);
    hlVector3Swap(&particle->uk25);
    hlSwapU32P(&particle->uk26);
    hlSwapU32P(&particle->uk27);
    hlSwapU32P(&particle->uk28);
    hlSwapU32P(&particle->uk29);
    hlSwapU32P(&particle->uk30);
    hlSwapU32P(&particle->uk31);
    hlSwapU32P(&particle->uk32);
    hlSwapFloatP(&particle->uk33);
    hlSwapU32P(&particle->uk34);
    hlSwapU32P(&particle->uk35);
    hlSwapU32P(&particle->uk36);
    hlSwapU32P(&particle->uk37);
    hlSwapU32P(&particle->uk38);
    hlSwapU32P(&particle->uk39);
    hlSwapU32P(&particle->uk40);
    hlSwapFloatP(&particle->uk41);
    hlSwapU32P(&particle->uk42);
    hlSwapU32P(&particle->uk43);
    hlSwapU32P(&particle->uk44);
    hlSwapU32P(&particle->uk45);
    hlSwapU32P(&particle->uk46);
    hlSwapU32P(&particle->uk47);
    hlSwapU32P(&particle->uk48);
    hlSwapU32P(&particle->uk49);
    hlSwapU32P(&particle->uk50);
    hlVector3Swap(&particle->uk51);
    hlSwapU32P(&particle->uk52);
    hlVector3Swap(&particle->uk53);
    hlSwapU32P(&particle->uk54);
    hlVector3Swap(&particle->uk55);
    hlSwapU32P(&particle->uk56);
    hlSwapU32P(&particle->uk57);
    hlSwapFloatP(&particle->uk58);
    hlSwapU32P(&particle->uk59);
    hlSwapU32P(&particle->uk60);
    hlSwapU32P(&particle->uk61);
    hlSwapU32P(&particle->uk62);
    hlSwapU32P(&particle->uk63);
    hlSwapU32P(&particle->uk64);
    hlSwapU32P(&particle->uk65);
    hlSwapU32P(&particle->uk66);
    hlSwapU32P(&particle->uk67);
    hlSwapU32P(&particle->uk68);
    hlSwapU32P(&particle->uk69);
    hlSwapU32P(&particle->uk70);
    hlSwapS32P(&particle->uk71);
    hlSwapS32P(&particle->uk72);
    hlSwapU32P(&particle->uk73);
    hlSwapU32P(&particle->uk74);
    hlSwapU32P(&particle->uk75);
    hlSwapU32P(&particle->uk76);
    hlSwapU32P(&particle->uk77);
    hlSwapFloatP(&particle->uk78);
    hlSwapU32P(&particle->uk79);
    hlSwapU32P(&particle->uk80);
    hlSwapU32P(&particle->uk81);
    hlSwapU32P(&particle->uk82);
    hlSwapU32P(&particle->uk83);
    hlSwapU32P(&particle->uk84);
    hlSwapU32P(&particle->uk85);
    hlSwapU32P(&particle->uk86);
    hlSwapU32P(&particle->uk87);
    hlSwapU32P(&particle->uk88);
    hlSwapU32P(&particle->uk89);
    hlSwapU32P(&particle->uk90);
    hlSwapU32P(&particle->uk91);
    hlSwapU32P(&particle->uk92);
    hlSwapU32P(&particle->uk93);
    hlSwapU32P(&particle->uk94);
    hlSwapS32P(&particle->uk95);
    hlSwapU32P(&particle->uk96);
    hlSwapS32P(&particle->uk97);
    hlSwapU32P(&particle->uk98);
    hlSwapU32P(&particle->meshDataPtr);
    hlSwapU32P(&particle->uk99);
    hlSwapU32P(&particle->uk100);
    hlSwapU32P(&particle->uk101);
    hlSwapU32P(&particle->uk102);
    hlSwapU32P(&particle->uk103);
    hlSwapFloatP(&particle->uk104);
    hlSwapU32P(&particle->uk105);
    hlSwapFloatP(&particle->uk106);
    hlSwapFloatP(&particle->uk107);
    hlSwapFloatP(&particle->uk108);
    hlSwapFloatP(&particle->uk109);
    hlSwapU32P(&particle->uk110);
    hlSwapU32P(&particle->uk111);
    hlSwapU32P(&particle->uk112);
    hlSwapU32P(&particle->uk113);
    hlSwapU32P(&particle->uk114);
    hlSwapU32P(&particle->uk115);
    hlSwapU32P(&particle->uk116);
    hlSwapU32P(&particle->uk117);
    hlSwapU32P(&particle->uk118);
    hlSwapU32P(&particle->uk119);
    hlSwapU32P(&particle->uk120);
    hlSwapU32P(&particle->uk121);
    hlSwapU32P(&particle->uk122);
    hlSwapU32P(&particle->uk123);
    hlSwapU32P(&particle->uk124);
    hlSwapU32P(&particle->uk125);
    hlSwapU32P(&particle->uk126);
    hlSwapU32P(&particle->uk127);
    hlSwapU32P(&particle->uk128);
    hlSwapU32P(&particle->uk129);
    hlSwapU32P(&particle->uk130);
    hlSwapU32P(&particle->uk131);
    hlSwapU32P(&particle->uk132);
    hlSwapU32P(&particle->uk133);
    hlSwapU32P(&particle->uk134);
    hlSwapU32P(&particle->uk135);
    hlSwapU32P(&particle->uk136);
}

void hlGrifEmitterParameterV106Swap(HlGrifEmitterParameterV106* emitter, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        unsigned int i;
        hlSwapU32P(&emitter->nameOffset);

        for (i = 0; i < 12; ++i)
        {
            hlSwapU32P(&emitter->animOffsets[i]);
        }

        hlSwapU32P(&emitter->particleOffset);
        hlSwapU32P(&emitter->nextEmitterOffset);
    }

    hlSwapU32P(&emitter->uk1);
    hlSwapFloatP(&emitter->uk2);
    hlSwapFloatP(&emitter->uk3);
    hlVector3Swap(&emitter->uk4);
    hlSwapU32P(&emitter->uk5);
    hlSwapU32P(&emitter->uk6);
    hlSwapU32P(&emitter->uk7);
    hlSwapU32P(&emitter->uk8);
    hlSwapU32P(&emitter->uk9);
    hlSwapU32P(&emitter->uk10);
    hlSwapU32P(&emitter->uk11);
    hlSwapU32P(&emitter->uk12);
    hlSwapU32P(&emitter->uk13);
    hlSwapU32P(&emitter->uk14);
    hlSwapU32P(&emitter->uk15);
    hlSwapU32P(&emitter->uk16);
    hlSwapU32P(&emitter->uk17);
    hlSwapFloatP(&emitter->uk18);
    hlSwapFloatP(&emitter->uk19);
    hlSwapFloatP(&emitter->uk20);
    hlSwapU32P(&emitter->uk21);
    hlSwapU32P(&emitter->uk22);
    hlSwapFloatP(&emitter->uk23);
    hlSwapU32P(&emitter->uk24);
    hlSwapU32P(&emitter->uk25);
    hlSwapFloatP(&emitter->uk26);
    hlSwapFloatP(&emitter->uk27);
    hlSwapU32P(&emitter->uk28);
    hlSwapU32P(&emitter->uk29);
    hlVector3Swap(&emitter->uk30);
    hlSwapU32P(&emitter->uk31);
    hlSwapU32P(&emitter->uk32);
    hlSwapFloatP(&emitter->uk33);
    hlSwapFloatP(&emitter->uk34);
    hlSwapFloatP(&emitter->uk35);
    hlSwapFloatP(&emitter->uk36);
    hlSwapU32P(&emitter->uk37);
    hlSwapU32P(&emitter->uk46);
    hlSwapU32P(&emitter->uk47);
    hlSwapU32P(&emitter->uk48);
    hlSwapU32P(&emitter->uk49);
    hlSwapU32P(&emitter->uk50);
    hlSwapU32P(&emitter->uk51);
    hlSwapU32P(&emitter->uk52);
    hlSwapU32P(&emitter->uk53);
    hlSwapU32P(&emitter->uk54);
    hlSwapU32P(&emitter->uk55);
    hlSwapU32P(&emitter->uk56);
    hlSwapU32P(&emitter->uk57);
    hlSwapU32P(&emitter->uk58);
    hlSwapU32P(&emitter->uk59);
    hlSwapU32P(&emitter->uk60);
    hlSwapU32P(&emitter->uk61);
    hlSwapU32P(&emitter->uk62);
    hlSwapU32P(&emitter->uk63);
    hlSwapU32P(&emitter->uk64);
    hlSwapU32P(&emitter->uk65);
    hlSwapU32P(&emitter->uk66);
    hlSwapU32P(&emitter->uk67);
    hlSwapU32P(&emitter->uk68);
    hlSwapU32P(&emitter->uk69);
    hlSwapU32P(&emitter->uk70);
    hlSwapU32P(&emitter->uk71);
    hlSwapU32P(&emitter->uk72);
    hlSwapU32P(&emitter->uk73);
}

void hlGrifEffectParameterV106Swap(HlGrifEffectParameterV106* param, HlBool swapOffsets)
{
    if (swapOffsets)
    {
        unsigned int i;
        hlSwapU32P(&param->nameOffset);

        for (i = 0; i < 14; ++i)
        {
            hlSwapU32P(&param->animOffsets[i]);
        }

        hlSwapU32P(&param->emitterOffset);
        hlSwapU32P(&param->nextParamOffset);
    }

    hlSwapFloatP(&param->uk1);
    hlSwapFloatP(&param->rangeIn);
    hlSwapFloatP(&param->rangeOut);
    hlSwapFloatP(&param->uk4);
    hlSwapFloatP(&param->uk5);
    hlSwapFloatP(&param->uk6);
    hlSwapFloatP(&param->uk7);
    hlVector3Swap(&param->uk8);
    hlSwapU32P(&param->uk11);
    hlSwapU32P(&param->uk12);
    hlSwapU32P(&param->uk13);
    hlSwapU32P(&param->uk14);
    hlSwapU32P(&param->uk15);
    hlSwapFloatP(&param->uk16);
    hlSwapFloatP(&param->uk17);
    hlSwapFloatP(&param->uk18);
    hlSwapU32P(&param->uk19);
    hlSwapU32P(&param->uk20);
    hlSwapFloatP(&param->uk21);
    hlSwapFloatP(&param->uk22);
    hlSwapFloatP(&param->uk23);
    hlSwapFloatP(&param->uk24);
    hlSwapU32P(&param->uk25);
    hlSwapU32P(&param->uk40);
    hlSwapU32P(&param->uk41);
    hlSwapU32P(&param->uk42);
    hlSwapU32P(&param->uk43);
    hlSwapU32P(&param->uk44);
    hlSwapU32P(&param->uk45);
    hlSwapU32P(&param->uk46);
    hlSwapU32P(&param->uk47);
    hlSwapU32P(&param->uk48);
    hlSwapU32P(&param->uk49);
    hlSwapU32P(&param->uk50);
    hlSwapU32P(&param->uk51);
    hlSwapU32P(&param->uk52);
    hlSwapU32P(&param->uk53);
    hlSwapU32P(&param->uk54);
    hlSwapU32P(&param->uk55);
    hlSwapU32P(&param->uk56);
    hlSwapU32P(&param->uk57);
    hlSwapU32P(&param->uk58);
    hlSwapU32P(&param->uk59);
    hlSwapU32P(&param->uk60);
    hlSwapU32P(&param->uk61);
    hlSwapU32P(&param->uk62);
    hlSwapU32P(&param->uk63);
    hlSwapU32P(&param->uk64);
    hlSwapU32P(&param->uk65);
    hlSwapU32P(&param->uk66);
    hlSwapU32P(&param->uk67);
}

void hlGrifEffectSwap(HlGrifEffect* effect, HlBool swapOffsets)
{
    hlSwapU32P(&effect->signature);
    hlSwapU32P(&effect->version);
    hlSwapU32P(&effect->flags);
    if (swapOffsets) hlSwapU32P(&effect->paramsOffset);
}

static void hlINGrifParticleParameterV106SwapRecursive(HlGrifParticleParameterV106* particle)
{
    /* Swap particle. */
    hlGrifParticleParameterV106Swap(particle, HL_FALSE);

    /* Swap animations. */
    /* TODO */

    /* Swap material. */
    {
        HlGrifMaterialParameterV106* material = (HlGrifMaterialParameterV106*)
            hlOff32Get(&particle->materialOffset);

        hlGrifMaterialParameterV106Swap(material, HL_FALSE);
    }

    /* Swap next particle (if any). */
    {
        HlGrifParticleParameterV106* nextParticle = (HlGrifParticleParameterV106*)
            hlOff32Get(&particle->nextParticleOffset);

        if (nextParticle)
        {
            hlINGrifParticleParameterV106SwapRecursive(nextParticle);
        }
    }
}

static void hlINGrifEmitterParameterV106SwapRecursive(HlGrifEmitterParameterV106* emitter)
{
    /* Swap emitter. */
    hlGrifEmitterParameterV106Swap(emitter, HL_FALSE);

    /* Swap animations. */
    /* TODO */

    /* Swap particle recursively. */
    {
        HlGrifParticleParameterV106* particle = (HlGrifParticleParameterV106*)
            hlOff32Get(&emitter->particleOffset);

        hlINGrifParticleParameterV106SwapRecursive(particle);
    }

    /* Swap next emitter (if any). */
    {
        HlGrifEmitterParameterV106* nextEmitter = (HlGrifEmitterParameterV106*)
            hlOff32Get(&emitter->nextEmitterOffset);

        if (nextEmitter)
        {
            hlINGrifEmitterParameterV106SwapRecursive(nextEmitter);
        }
    }
}

static void hlINGrifEffectParameterV106SwapRecursive(HlGrifEffectParameterV106* param)
{
    /* Swap effect. */
    hlGrifEffectParameterV106Swap(param, HL_FALSE);

    /* Swap animations. */
    /* TODO */

    /* Swap emitter recursively. */
    {
        HlGrifEmitterParameterV106* emitter = (HlGrifEmitterParameterV106*)
            hlOff32Get(&param->emitterOffset);

        hlINGrifEmitterParameterV106SwapRecursive(emitter);
    }

    /* Swap next parameter (if any). */
    {
        HlGrifEffectParameterV106* nextParam = (HlGrifEffectParameterV106*)
            hlOff32Get(&param->nextParamOffset);

        if (nextParam)
        {
            hlINGrifEffectParameterV106SwapRecursive(nextParam);
        }
    }
}

HlResult hlGrifEffectFix(HlGrifEffect* effect, HlBINAEndianFlag endianFlag)
{
    /* Swap GRIF effect data if necessary. */
    if (hlBINANeedsSwap(endianFlag))
    {
        void* params;

        /* Swap header. */
        hlGrifEffectSwap(effect, HL_FALSE);

        /* Get params pointer. */
        params = hlOff32Get(&effect->paramsOffset);

        /* Recursively fix parameters. */
        if (((effect->version & 0xFF000000) >> 24) == 1)
        {
            switch ((effect->version & 0x00FF0000) >> 16)
            {
            /* TODO: Support minor version 1. */
            /* TODO: Support minor version 2. */
            /* TODO: Support minor version 3. */
            /* TODO: Support minor version 4. */
            /* TODO: Support minor version 5. */

            case 6:
                hlINGrifEffectParameterV106SwapRecursive(params);
                return HL_RESULT_SUCCESS;
            }
        }

        return HL_ERROR_UNSUPPORTED;
    }

    return HL_RESULT_SUCCESS;
}
