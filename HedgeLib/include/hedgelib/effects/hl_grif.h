#ifndef HL_GRIF_H_INCLUDED
#define HL_GRIF_H_INCLUDED
#include "../hl_math.h"

namespace hl
{
namespace bina
{
enum class endian_flag : u8;
} // bina

namespace grif
{
namespace v1_0
{
struct animation_param
{
    u32 uk1;
    u32 uk2;
    // TODO: Finish this struct!
};

// TODO: Figure out actual size and finish/uncomment this:
//HL_STATIC_ASSERT_SIZE(animation_param, );

struct texture_param
{
    off32<char> name;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 dataPtr;
    u32 uk1;
    u32 uk2;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(dataPtr);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
    }
};

HL_STATIC_ASSERT_SIZE(texture_param, 16);

struct material_param
{
    off32<char> name;
    texture_param textures[2];
    u32 texCount;
    u32 uk2;
    u32 uk3;
    float uk4;
    u32 uk5;
    u32 uk6;
    off32<char> shaderName;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 vertexShaderPtr;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 pixelShaderPtr;
    u32 uk7;
    u32 uk8;
    u32 uk9;
    u32 uk10;
    u32 isEnableLightField;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    u32 uk16;
    u32 uk17;
    u32 uk18;
    u32 uk19;
    u32 uk20;
    u32 uk21;
    u32 uk22;
    off32<void> uk23;
    u32 uk24;
    u32 uk25;
    u32 uk26;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap<swapOffsets>(textures[0]);
        hl::endian_swap<swapOffsets>(textures[1]);
        hl::endian_swap(texCount);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap<swapOffsets>(shaderName);
        hl::endian_swap(vertexShaderPtr);
        hl::endian_swap(pixelShaderPtr);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(isEnableLightField);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap<swapOffsets>(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);
        hl::endian_swap(uk26);
    }
};

HL_STATIC_ASSERT_SIZE(material_param, 0x9C);

struct particle_param
{
    off32<char> name;
    u32 type;
    float uk1;
    u32 uk2;
    u32 uk3;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    float uk8;
    u32 uk9;
    u32 uk10;
    float uk11;
    float uk12;
    float uk13;
    float uk14;
    float uk15;
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    u32 uk20;
    u32 uk21;
    u32 uk22;
    vec3 uk23;
    u32 uk24;
    vec3 uk25;
    u32 uk26;
    u32 uk27;
    u32 uk28;
    u32 uk29;
    u32 uk30;
    u32 uk31;
    u32 uk32;
    float uk33;
    u32 uk34;
    u32 uk35;
    u32 uk36;
    u32 uk37;
    u32 uk38;
    u32 uk39;
    u32 uk40;
    float uk41;
    u32 uk42;
    u32 uk43;
    u32 uk44;
    u32 uk45;
    u32 uk46;
    u32 uk47;
    u32 uk48;
    u32 uk49;
    u32 uk50;
    vec3 uk51;
    u32 uk52;
    vec3 uk53;
    u32 uk54;
    vec3 uk55;
    u32 uk56;
    u32 uk57;
    float uk58;
    u32 uk59;
    u32 uk60;
    u32 uk61;
    u32 uk62;
    u32 uk63;
    u32 uk64;
    u32 uk65;
    u32 uk66;
    u32 uk67;
    u32 uk68;
    u32 uk69;
    u32 uk70;
    s32 uk71;
    s32 uk72;
    u32 uk73;
    u32 uk74;
    u32 uk75;
    u32 uk76;
    u32 uk77;
    float uk78;
    u32 uk79;
    u32 uk80;
    u32 uk81;
    u32 uk82;
    u32 uk83;
    u32 uk84;
    u32 uk85;
    u32 uk86;
    u32 uk87;
    u32 uk88;
    u32 uk89;
    u32 uk90;
    u32 uk91;
    u32 uk92;
    u32 uk93;
    off32<material_param> material;
    u32 uk94;
    s32 uk95;
    u32 uk96;
    s32 uk97;
    u32 uk98;
    off32<char> meshName;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 meshDataPtr;
    u32 uk99;
    u32 uk100;
    u32 uk101;
    u32 uk102;
    u32 uk103;
    float uk104;
    u32 uk110;
    off32<animation_param> anims[29];
    off32<particle_param> nextParticle;
    u32 reserved2[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(type);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap<swapOffsets>(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap<swapOffsets>(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk33);
        hl::endian_swap(uk34);
        hl::endian_swap(uk35);
        hl::endian_swap(uk36);
        hl::endian_swap(uk37);
        hl::endian_swap(uk38);
        hl::endian_swap(uk39);
        hl::endian_swap(uk40);
        hl::endian_swap(uk41);
        hl::endian_swap(uk42);
        hl::endian_swap(uk43);
        hl::endian_swap(uk44);
        hl::endian_swap(uk45);
        hl::endian_swap(uk46);
        hl::endian_swap(uk47);
        hl::endian_swap(uk48);
        hl::endian_swap(uk49);
        hl::endian_swap(uk50);
        hl::endian_swap<swapOffsets>(uk51);
        hl::endian_swap(uk52);
        hl::endian_swap<swapOffsets>(uk53);
        hl::endian_swap(uk54);
        hl::endian_swap<swapOffsets>(uk55);
        hl::endian_swap(uk56);
        hl::endian_swap(uk57);
        hl::endian_swap(uk58);
        hl::endian_swap(uk59);
        hl::endian_swap(uk60);
        hl::endian_swap(uk61);
        hl::endian_swap(uk62);
        hl::endian_swap(uk63);
        hl::endian_swap(uk64);
        hl::endian_swap(uk65);
        hl::endian_swap(uk66);
        hl::endian_swap(uk67);
        hl::endian_swap(uk68);
        hl::endian_swap(uk69);
        hl::endian_swap(uk70);
        hl::endian_swap(uk71);
        hl::endian_swap(uk72);
        hl::endian_swap(uk73);
        hl::endian_swap(uk74);
        hl::endian_swap(uk75);
        hl::endian_swap(uk76);
        hl::endian_swap(uk77);
        hl::endian_swap(uk78);
        hl::endian_swap(uk79);
        hl::endian_swap(uk80);
        hl::endian_swap(uk81);
        hl::endian_swap(uk82);
        hl::endian_swap(uk83);
        hl::endian_swap(uk84);
        hl::endian_swap(uk85);
        hl::endian_swap(uk86);
        hl::endian_swap(uk87);
        hl::endian_swap(uk88);
        hl::endian_swap(uk89);
        hl::endian_swap(uk90);
        hl::endian_swap(uk91);
        hl::endian_swap(uk92);
        hl::endian_swap(uk93);
        hl::endian_swap<swapOffsets>(material);
        hl::endian_swap(uk94);
        hl::endian_swap(uk95);
        hl::endian_swap(uk96);
        hl::endian_swap(uk97);
        hl::endian_swap(uk98);
        hl::endian_swap<swapOffsets>(meshName);
        hl::endian_swap(meshDataPtr);
        hl::endian_swap(uk99);
        hl::endian_swap(uk100);
        hl::endian_swap(uk101);
        hl::endian_swap(uk102);
        hl::endian_swap(uk103);
        hl::endian_swap(uk104);
        hl::endian_swap(uk110);

        for (unsigned int i = 0; i < 29; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(nextParticle);
    }
};

HL_STATIC_ASSERT_SIZE(particle_param, 0x2D8);

struct emitter_param
{
    off32<char> name;
    u32 uk1;
    float uk2;
    float uk3;
    vec3 uk4;
    u32 uk5;
    u32 uk6;
    u32 uk7;
    u32 uk8;
    u32 uk9;
    u32 uk10;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    u32 uk16;
    u32 uk17;
    float uk18;
    float uk19;
    float uk20;
    u32 uk21;
    u32 uk22;
    float uk23;
    u32 uk24;
    u32 uk25;
    float uk26;
    float uk27;
    u32 uk28;
    u32 uk29;
    vec3 uk30;
    u32 uk31;
    float uk32;
    u32 uk37;
    off32<animation_param> anims[12];
    off32<particle_param> particle;
    off32<emitter_param> nextEmitter;
    u32 reserved[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap<swapOffsets>(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap<swapOffsets>(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk37);

        for (unsigned int i = 0; i < 12; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(particle);
        hl::endian_swap<swapOffsets>(nextEmitter);
    }
};

HL_STATIC_ASSERT_SIZE(emitter_param, 0x150);

struct effect_param
{
    off32<char> name;
    float uk1;
    /** @brief RangeIn? */
    float rangeIn;
    /** @brief RangeOut? */
    float rangeOut;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    vec3 uk8;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    /** @brief Scale XYZ? */
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    float uk20;
    /** @brief Always 1? */
    u32 uk25;
    off32<animation_param> anims[14];
    off32<emitter_param> emitter;
    off32<effect_param> nextEffect;
    u32 reserved2[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(rangeIn);
        hl::endian_swap(rangeOut);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap<swapOffsets>(uk8);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk25);

        for (unsigned int i = 0; i < 14; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(emitter);
        hl::endian_swap<swapOffsets>(nextEffect);
    }
};

HL_STATIC_ASSERT_SIZE(effect_param, 0x118);
} // v1_0

namespace v1_1
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;
using particle_param = v1_0::particle_param;
using emitter_param = v1_0::emitter_param;
using effect_param = v1_0::effect_param;
} // v1_1

namespace v1_2
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;
using particle_param = v1_0::particle_param;
using emitter_param = v1_0::emitter_param;
using effect_param = v1_0::effect_param;
} // v1_2

namespace v1_3
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;
using particle_param = v1_0::particle_param;
using emitter_param = v1_0::emitter_param;
using effect_param = v1_0::effect_param;
} // v1_3

namespace v1_4
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;

struct particle_param
{
    off32<char> name;
    u32 type;
    float uk1;
    u32 uk2;
    u32 uk3;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    float uk8;
    u32 uk9;
    u32 uk10;
    float uk11;
    float uk12;
    float uk13;
    float uk14;
    float uk15;
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    u32 uk20;
    u32 uk21;
    u32 uk22;
    vec3 uk23;
    u32 uk24;
    vec3 uk25;
    u32 uk26;
    u32 uk27;
    u32 uk28;
    u32 uk29;
    u32 uk30;
    u32 uk31;
    u32 uk32;
    float uk33;
    u32 uk34;
    u32 uk35;
    u32 uk36;
    u32 uk37;
    u32 uk38;
    u32 uk39;
    u32 uk40;
    float uk41;
    u32 uk42;
    u32 uk43;
    u32 uk44;
    u32 uk45;
    u32 uk46;
    u32 uk47;
    u32 uk48;
    u32 uk49;
    u32 uk50;
    vec3 uk51;
    u32 uk52;
    vec3 uk53;
    u32 uk54;
    vec3 uk55;
    u32 uk56;
    u32 uk57;
    float uk58;
    u32 uk59;
    u32 uk60;
    u32 uk61;
    u32 uk62;
    u32 uk63;
    u32 uk64;
    u32 uk65;
    u32 uk66;
    u32 uk67;
    u32 uk68;
    u32 uk69;
    u32 uk70;
    s32 uk71;
    s32 uk72;
    u32 uk73;
    u32 uk74;
    u32 uk75;
    u32 uk76;
    u32 uk77;
    float uk78;
    u32 uk79;
    u32 uk80;
    u32 uk81;
    u32 uk82;
    u32 uk83;
    u32 uk84;
    u32 uk85;
    u32 uk86;
    u32 uk87;
    u32 uk88;
    u32 uk89;
    u32 uk90;
    u32 uk91;
    u32 uk92;
    u32 uk93;
    off32<material_param> material;
    u32 uk94;
    s32 uk95;
    u32 uk96;
    s32 uk97;
    u32 uk98;
    off32<char> meshName;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 meshDataPtr;
    u32 uk99;
    u32 uk100;
    u32 uk101;
    u32 uk102;
    u32 uk103;
    float uk104;
    u32 uk110;
    off32<animation_param> anims[30];
    off32<particle_param> nextParticle;
    u32 reserved2[31];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(type);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap<swapOffsets>(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap<swapOffsets>(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk33);
        hl::endian_swap(uk34);
        hl::endian_swap(uk35);
        hl::endian_swap(uk36);
        hl::endian_swap(uk37);
        hl::endian_swap(uk38);
        hl::endian_swap(uk39);
        hl::endian_swap(uk40);
        hl::endian_swap(uk41);
        hl::endian_swap(uk42);
        hl::endian_swap(uk43);
        hl::endian_swap(uk44);
        hl::endian_swap(uk45);
        hl::endian_swap(uk46);
        hl::endian_swap(uk47);
        hl::endian_swap(uk48);
        hl::endian_swap(uk49);
        hl::endian_swap(uk50);
        hl::endian_swap<swapOffsets>(uk51);
        hl::endian_swap(uk52);
        hl::endian_swap<swapOffsets>(uk53);
        hl::endian_swap(uk54);
        hl::endian_swap<swapOffsets>(uk55);
        hl::endian_swap(uk56);
        hl::endian_swap(uk57);
        hl::endian_swap(uk58);
        hl::endian_swap(uk59);
        hl::endian_swap(uk60);
        hl::endian_swap(uk61);
        hl::endian_swap(uk62);
        hl::endian_swap(uk63);
        hl::endian_swap(uk64);
        hl::endian_swap(uk65);
        hl::endian_swap(uk66);
        hl::endian_swap(uk67);
        hl::endian_swap(uk68);
        hl::endian_swap(uk69);
        hl::endian_swap(uk70);
        hl::endian_swap(uk71);
        hl::endian_swap(uk72);
        hl::endian_swap(uk73);
        hl::endian_swap(uk74);
        hl::endian_swap(uk75);
        hl::endian_swap(uk76);
        hl::endian_swap(uk77);
        hl::endian_swap(uk78);
        hl::endian_swap(uk79);
        hl::endian_swap(uk80);
        hl::endian_swap(uk81);
        hl::endian_swap(uk82);
        hl::endian_swap(uk83);
        hl::endian_swap(uk84);
        hl::endian_swap(uk85);
        hl::endian_swap(uk86);
        hl::endian_swap(uk87);
        hl::endian_swap(uk88);
        hl::endian_swap(uk89);
        hl::endian_swap(uk90);
        hl::endian_swap(uk91);
        hl::endian_swap(uk92);
        hl::endian_swap(uk93);
        hl::endian_swap<swapOffsets>(material);
        hl::endian_swap(uk94);
        hl::endian_swap(uk95);
        hl::endian_swap(uk96);
        hl::endian_swap(uk97);
        hl::endian_swap(uk98);
        hl::endian_swap<swapOffsets>(meshName);
        hl::endian_swap(meshDataPtr);
        hl::endian_swap(uk99);
        hl::endian_swap(uk100);
        hl::endian_swap(uk101);
        hl::endian_swap(uk102);
        hl::endian_swap(uk103);
        hl::endian_swap(uk104);
        hl::endian_swap(uk110);

        for (unsigned int i = 0; i < 30; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(nextParticle);
    }
};

HL_STATIC_ASSERT_SIZE(particle_param, 0x2D8);

struct emitter_param
{
    off32<char> name;
    u32 uk1;
    float uk2;
    float uk3;
    vec3 uk4;
    u32 uk5;
    u32 uk6;
    u32 uk7;
    u32 uk8;
    u32 uk9;
    u32 uk10;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    u32 uk16;
    u32 uk17;
    float uk18;
    float uk19;
    float uk20;
    u32 uk21;
    u32 uk22;
    float uk23;
    u32 uk24;
    u32 uk25;
    float uk26;
    float uk27;
    u32 uk28;
    u32 uk29;
    vec3 uk30;
    u32 uk31;
    float uk32;
    u32 uk37;
    off32<animation_param> anims[12];
    off32<particle_param> particle;
    off32<emitter_param> nextEmitter;
    u32 reserved[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap<swapOffsets>(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap<swapOffsets>(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk37);

        for (unsigned int i = 0; i < 12; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(particle);
        hl::endian_swap<swapOffsets>(nextEmitter);
    }
};

HL_STATIC_ASSERT_SIZE(emitter_param, 0x150);

struct effect_param
{
    off32<char> name;
    float uk1;
    /** @brief RangeIn? */
    float rangeIn;
    /** @brief RangeOut? */
    float rangeOut;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    vec3 uk8;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    /** @brief Scale XYZ? */
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    float uk20;
    /** @brief Always 1? */
    u32 uk25;
    off32<animation_param> anims[14];
    off32<emitter_param> emitter;
    off32<effect_param> nextEffect;
    u32 reserved2[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(rangeIn);
        hl::endian_swap(rangeOut);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap<swapOffsets>(uk8);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk25);

        for (unsigned int i = 0; i < 14; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(emitter);
        hl::endian_swap<swapOffsets>(nextEffect);
    }
};

HL_STATIC_ASSERT_SIZE(effect_param, 0x118);
} // v1_4

namespace v1_5
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;

struct particle_param
{
    off32<char> name;
    u32 type;
    float uk1;
    u32 uk2;
    u32 uk3;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    float uk8;
    u32 uk9;
    u32 uk10;
    float uk11;
    float uk12;
    float uk13;
    float uk14;
    float uk15;
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    u32 uk20;
    u32 uk21;
    u32 uk22;
    vec3 uk23;
    u32 uk24;
    vec3 uk25;
    u32 uk26;
    u32 uk27;
    u32 uk28;
    u32 uk29;
    u32 uk30;
    u32 uk31;
    u32 uk32;
    float uk33;
    u32 uk34;
    u32 uk35;
    u32 uk36;
    u32 uk37;
    u32 uk38;
    u32 uk39;
    u32 uk40;
    float uk41;
    u32 uk42;
    u32 uk43;
    u32 uk44;
    u32 uk45;
    u32 uk46;
    u32 uk47;
    u32 uk48;
    u32 uk49;
    u32 uk50;
    vec3 uk51;
    u32 uk52;
    vec3 uk53;
    u32 uk54;
    vec3 uk55;
    u32 uk56;
    u32 uk57;
    float uk58;
    u32 uk59;
    u32 uk60;
    u32 uk61;
    u32 uk62;
    u32 uk63;
    u32 uk64;
    u32 uk65;
    u32 uk66;
    u32 uk67;
    u32 uk68;
    u32 uk69;
    u32 uk70;
    s32 uk71;
    s32 uk72;
    u32 uk73;
    u32 uk74;
    u32 uk75;
    u32 uk76;
    u32 uk77;
    float uk78;
    u32 uk79;
    u32 uk80;
    u32 uk81;
    u32 uk82;
    u32 uk83;
    u32 uk84;
    u32 uk85;
    u32 uk86;
    u32 uk87;
    u32 uk88;
    u32 uk89;
    u32 uk90;
    u32 uk91;
    u32 uk92;
    u32 uk93;
    off32<material_param> material;
    u32 uk94;
    s32 uk95;
    u32 uk96;
    s32 uk97;
    u32 uk98;
    off32<char> meshName;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 meshDataPtr;
    u32 uk99;
    u32 uk100;
    u32 uk101;
    u32 uk102;
    u32 uk103;
    float uk104;
    float uk105;
    u32 uk110;
    off32<animation_param> anims[30];
    off32<particle_param> nextParticle;
    u32 reserved2[30];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(type);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap<swapOffsets>(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap<swapOffsets>(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk33);
        hl::endian_swap(uk34);
        hl::endian_swap(uk35);
        hl::endian_swap(uk36);
        hl::endian_swap(uk37);
        hl::endian_swap(uk38);
        hl::endian_swap(uk39);
        hl::endian_swap(uk40);
        hl::endian_swap(uk41);
        hl::endian_swap(uk42);
        hl::endian_swap(uk43);
        hl::endian_swap(uk44);
        hl::endian_swap(uk45);
        hl::endian_swap(uk46);
        hl::endian_swap(uk47);
        hl::endian_swap(uk48);
        hl::endian_swap(uk49);
        hl::endian_swap(uk50);
        hl::endian_swap<swapOffsets>(uk51);
        hl::endian_swap(uk52);
        hl::endian_swap<swapOffsets>(uk53);
        hl::endian_swap(uk54);
        hl::endian_swap<swapOffsets>(uk55);
        hl::endian_swap(uk56);
        hl::endian_swap(uk57);
        hl::endian_swap(uk58);
        hl::endian_swap(uk59);
        hl::endian_swap(uk60);
        hl::endian_swap(uk61);
        hl::endian_swap(uk62);
        hl::endian_swap(uk63);
        hl::endian_swap(uk64);
        hl::endian_swap(uk65);
        hl::endian_swap(uk66);
        hl::endian_swap(uk67);
        hl::endian_swap(uk68);
        hl::endian_swap(uk69);
        hl::endian_swap(uk70);
        hl::endian_swap(uk71);
        hl::endian_swap(uk72);
        hl::endian_swap(uk73);
        hl::endian_swap(uk74);
        hl::endian_swap(uk75);
        hl::endian_swap(uk76);
        hl::endian_swap(uk77);
        hl::endian_swap(uk78);
        hl::endian_swap(uk79);
        hl::endian_swap(uk80);
        hl::endian_swap(uk81);
        hl::endian_swap(uk82);
        hl::endian_swap(uk83);
        hl::endian_swap(uk84);
        hl::endian_swap(uk85);
        hl::endian_swap(uk86);
        hl::endian_swap(uk87);
        hl::endian_swap(uk88);
        hl::endian_swap(uk89);
        hl::endian_swap(uk90);
        hl::endian_swap(uk91);
        hl::endian_swap(uk92);
        hl::endian_swap(uk93);
        hl::endian_swap<swapOffsets>(material);
        hl::endian_swap(uk94);
        hl::endian_swap(uk95);
        hl::endian_swap(uk96);
        hl::endian_swap(uk97);
        hl::endian_swap(uk98);
        hl::endian_swap<swapOffsets>(meshName);
        hl::endian_swap(meshDataPtr);
        hl::endian_swap(uk99);
        hl::endian_swap(uk100);
        hl::endian_swap(uk101);
        hl::endian_swap(uk102);
        hl::endian_swap(uk103);
        hl::endian_swap(uk104);
        hl::endian_swap(uk105);
        hl::endian_swap(uk110);

        for (unsigned int i = 0; i < 30; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(nextParticle);
    }
};

HL_STATIC_ASSERT_SIZE(particle_param, 0x2D8);

struct emitter_param
{
    off32<char> name;
    u32 uk1;
    float uk2;
    float uk3;
    vec3 uk4;
    u32 uk5;
    u32 uk6;
    u32 uk7;
    u32 uk8;
    u32 uk9;
    u32 uk10;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    u32 uk16;
    u32 uk17;
    float uk18;
    float uk19;
    float uk20;
    u32 uk21;
    u32 uk22;
    float uk23;
    u32 uk24;
    u32 uk25;
    float uk26;
    float uk27;
    u32 uk28;
    u32 uk29;
    vec3 uk30;
    u32 uk31;
    float uk32;
    u32 uk37;
    off32<animation_param> anims[12];
    off32<particle_param> particle;
    off32<emitter_param> nextEmitter;
    u32 reserved[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap<swapOffsets>(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap<swapOffsets>(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk37);

        for (unsigned int i = 0; i < 12; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(particle);
        hl::endian_swap<swapOffsets>(nextEmitter);
    }
};

HL_STATIC_ASSERT_SIZE(emitter_param, 0x150);

struct effect_param
{
    off32<char> name;
    float uk1;
    /** @brief RangeIn? */
    float rangeIn;
    /** @brief RangeOut? */
    float rangeOut;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    vec3 uk8;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    /** @brief Scale XYZ? */
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    float uk20;
    /** @brief Always 1? */
    u32 uk25;
    off32<animation_param> anims[14];
    off32<emitter_param> emitter;
    off32<effect_param> nextEffect;
    u32 reserved2[32];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(rangeIn);
        hl::endian_swap(rangeOut);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap<swapOffsets>(uk8);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk25);

        for (unsigned int i = 0; i < 14; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(emitter);
        hl::endian_swap<swapOffsets>(nextEffect);
    }
};

HL_STATIC_ASSERT_SIZE(effect_param, 0x118);
} // v1_5

namespace v1_6
{
using animation_param = v1_0::animation_param;
using texture_param = v1_0::texture_param;
using material_param = v1_0::material_param;

struct particle_param
{
    off32<char> name;
    u32 type;
    float uk1;
    u32 uk2;
    u32 uk3;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    float uk8;
    u32 uk9;
    u32 uk10;
    float uk11;
    float uk12;
    float uk13;
    float uk14;
    float uk15;
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    u32 uk20;
    u32 uk21;
    u32 uk22;
    vec3 uk23;
    u32 uk24;
    vec3 uk25;
    u32 uk26;
    u32 uk27;
    u32 uk28;
    u32 uk29;
    u32 uk30;
    u32 uk31;
    u32 uk32;
    float uk33;
    u32 uk34;
    u32 uk35;
    u32 uk36;
    u32 uk37;
    u32 uk38;
    u32 uk39;
    u32 uk40;
    float uk41;
    u32 uk42;
    u32 uk43;
    u32 uk44;
    u32 uk45;
    u32 uk46;
    u32 uk47;
    u32 uk48;
    u32 uk49;
    u32 uk50;
    vec3 uk51;
    u32 uk52;
    vec3 uk53;
    u32 uk54;
    vec3 uk55;
    u32 uk56;
    u32 uk57;
    float uk58;
    u32 uk59;
    u32 uk60;
    u32 uk61;
    u32 uk62;
    u32 uk63;
    u32 uk64;
    u32 uk65;
    u32 uk66;
    u32 uk67;
    u32 uk68;
    u32 uk69;
    u32 uk70;
    s32 uk71;
    s32 uk72;
    u32 uk73;
    u32 uk74;
    u32 uk75;
    u32 uk76;
    u32 uk77;
    float uk78;
    u32 uk79;
    u32 uk80;
    u32 uk81;
    u32 uk82;
    u32 uk83;
    u32 uk84;
    u32 uk85;
    u32 uk86;
    u32 uk87;
    u32 uk88;
    u32 uk89;
    u32 uk90;
    u32 uk91;
    u32 uk92;
    u32 uk93;
    off32<material_param> material;
    u32 uk94;
    s32 uk95;
    u32 uk96;
    s32 uk97;
    u32 uk98;
    off32<char> meshName;
    /** @brief Set by the game; always just set to null (0) within the actual files. */
    u32 meshDataPtr;
    u32 uk99;
    u32 uk100;
    u32 uk101;
    u32 uk102;
    u32 uk103;
    float uk104;
    float uk105;
    float uk106;
    float uk107;
    float uk108;
    float uk109;
    u32 uk110;
    off32<animation_param> anims[30];
    off32<particle_param> nextParticle;
    u32 reserved2[26];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(type);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap<swapOffsets>(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap<swapOffsets>(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk33);
        hl::endian_swap(uk34);
        hl::endian_swap(uk35);
        hl::endian_swap(uk36);
        hl::endian_swap(uk37);
        hl::endian_swap(uk38);
        hl::endian_swap(uk39);
        hl::endian_swap(uk40);
        hl::endian_swap(uk41);
        hl::endian_swap(uk42);
        hl::endian_swap(uk43);
        hl::endian_swap(uk44);
        hl::endian_swap(uk45);
        hl::endian_swap(uk46);
        hl::endian_swap(uk47);
        hl::endian_swap(uk48);
        hl::endian_swap(uk49);
        hl::endian_swap(uk50);
        hl::endian_swap<swapOffsets>(uk51);
        hl::endian_swap(uk52);
        hl::endian_swap<swapOffsets>(uk53);
        hl::endian_swap(uk54);
        hl::endian_swap<swapOffsets>(uk55);
        hl::endian_swap(uk56);
        hl::endian_swap(uk57);
        hl::endian_swap(uk58);
        hl::endian_swap(uk59);
        hl::endian_swap(uk60);
        hl::endian_swap(uk61);
        hl::endian_swap(uk62);
        hl::endian_swap(uk63);
        hl::endian_swap(uk64);
        hl::endian_swap(uk65);
        hl::endian_swap(uk66);
        hl::endian_swap(uk67);
        hl::endian_swap(uk68);
        hl::endian_swap(uk69);
        hl::endian_swap(uk70);
        hl::endian_swap(uk71);
        hl::endian_swap(uk72);
        hl::endian_swap(uk73);
        hl::endian_swap(uk74);
        hl::endian_swap(uk75);
        hl::endian_swap(uk76);
        hl::endian_swap(uk77);
        hl::endian_swap(uk78);
        hl::endian_swap(uk79);
        hl::endian_swap(uk80);
        hl::endian_swap(uk81);
        hl::endian_swap(uk82);
        hl::endian_swap(uk83);
        hl::endian_swap(uk84);
        hl::endian_swap(uk85);
        hl::endian_swap(uk86);
        hl::endian_swap(uk87);
        hl::endian_swap(uk88);
        hl::endian_swap(uk89);
        hl::endian_swap(uk90);
        hl::endian_swap(uk91);
        hl::endian_swap(uk92);
        hl::endian_swap(uk93);
        hl::endian_swap<swapOffsets>(material);
        hl::endian_swap(uk94);
        hl::endian_swap(uk95);
        hl::endian_swap(uk96);
        hl::endian_swap(uk97);
        hl::endian_swap(uk98);
        hl::endian_swap<swapOffsets>(meshName);
        hl::endian_swap(meshDataPtr);
        hl::endian_swap(uk99);
        hl::endian_swap(uk100);
        hl::endian_swap(uk101);
        hl::endian_swap(uk102);
        hl::endian_swap(uk103);
        hl::endian_swap(uk104);
        hl::endian_swap(uk105);
        hl::endian_swap(uk106);
        hl::endian_swap(uk107);
        hl::endian_swap(uk108);
        hl::endian_swap(uk109);
        hl::endian_swap(uk110);

        for (unsigned int i = 0; i < 30; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(nextParticle);
    }
};

HL_STATIC_ASSERT_SIZE(particle_param, 0x2D8);

struct emitter_param
{
    off32<char> name;
    u32 uk1;
    float uk2;
    float uk3;
    vec3 uk4;
    u32 uk5;
    u32 uk6;
    u32 uk7;
    u32 uk8;
    u32 uk9;
    u32 uk10;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    u32 uk16;
    u32 uk17;
    float uk18;
    float uk19;
    float uk20;
    u32 uk21;
    u32 uk22;
    float uk23;
    u32 uk24;
    u32 uk25;
    float uk26;
    float uk27;
    u32 uk28;
    u32 uk29;
    vec3 uk30;
    u32 uk31;
    float uk32;
    float uk33;
    float uk34;
    float uk35;
    float uk36;
    u32 uk37;
    off32<animation_param> anims[12];
    off32<particle_param> particle;
    off32<emitter_param> nextEmitter;
    u32 reserved[28];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(uk2);
        hl::endian_swap(uk3);
        hl::endian_swap<swapOffsets>(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap(uk8);
        hl::endian_swap(uk9);
        hl::endian_swap(uk10);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);
        hl::endian_swap(uk26);
        hl::endian_swap(uk27);
        hl::endian_swap(uk28);
        hl::endian_swap(uk29);
        hl::endian_swap<swapOffsets>(uk30);
        hl::endian_swap(uk31);
        hl::endian_swap(uk32);
        hl::endian_swap(uk33);
        hl::endian_swap(uk34);
        hl::endian_swap(uk35);
        hl::endian_swap(uk36);
        hl::endian_swap(uk37);

        for (unsigned int i = 0; i < 12; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(particle);
        hl::endian_swap<swapOffsets>(nextEmitter);
    }
};

HL_STATIC_ASSERT_SIZE(emitter_param, 0x150);

struct effect_param
{
    off32<char> name;
    float uk1;
    /** @brief RangeIn? */
    float rangeIn;
    /** @brief RangeOut? */
    float rangeOut;
    float uk4;
    float uk5;
    float uk6;
    float uk7;
    vec3 uk8;
    u32 uk11;
    u32 uk12;
    u32 uk13;
    u32 uk14;
    u32 uk15;
    /** @brief Scale XYZ? */
    float uk16;
    float uk17;
    float uk18;
    u32 uk19;
    float uk20;
    float uk21; // 0x54
    float uk22; // 0x58
    float uk23; // 0x5C
    float uk24; // 0x60
    /** @brief Always 1? */
    u32 uk25; // 0x64
    off32<animation_param> anims[14];
    off32<emitter_param> emitter;
    off32<effect_param> nextEffect;
    u32 reserved2[28];

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap<swapOffsets>(name);
        hl::endian_swap(uk1);
        hl::endian_swap(rangeIn);
        hl::endian_swap(rangeOut);
        hl::endian_swap(uk4);
        hl::endian_swap(uk5);
        hl::endian_swap(uk6);
        hl::endian_swap(uk7);
        hl::endian_swap<swapOffsets>(uk8);
        hl::endian_swap(uk11);
        hl::endian_swap(uk12);
        hl::endian_swap(uk13);
        hl::endian_swap(uk14);
        hl::endian_swap(uk15);
        hl::endian_swap(uk16);
        hl::endian_swap(uk17);
        hl::endian_swap(uk18);
        hl::endian_swap(uk19);
        hl::endian_swap(uk20);
        hl::endian_swap(uk21);
        hl::endian_swap(uk22);
        hl::endian_swap(uk23);
        hl::endian_swap(uk24);
        hl::endian_swap(uk25);

        for (unsigned int i = 0; i < 14; ++i)
        {
            hl::endian_swap<swapOffsets>(anims[i]);
        }

        hl::endian_swap<swapOffsets>(emitter);
        hl::endian_swap<swapOffsets>(nextEffect);
    }
};

HL_STATIC_ASSERT_SIZE(effect_param, 0x118);
} // v1_6

struct effect
{
    u32 signature;
    u32 version;
    u32 flags;
    off32<void> effects;

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        hl::endian_swap(signature);
        hl::endian_swap(version);
        hl::endian_swap(flags);
        hl::endian_swap<swapOffsets>(effects);
    }

    inline u8 major_version() const noexcept
    {
        return static_cast<u8>((version & 0xFF000000U) >> 24);
    }

    inline u8 minor_version() const noexcept
    {
        return static_cast<u8>((version & 0x00FF0000U) >> 16);
    }

    inline const v1_0::effect_param* effects_v1_0() const noexcept
    {
        return static_cast<const v1_0::effect_param*>(effects.get());
    }

    inline v1_0::effect_param* effects_v1_0() noexcept
    {
        return static_cast<v1_0::effect_param*>(effects.get());
    }

    inline const v1_1::effect_param* effects_v1_1() const noexcept
    {
        return static_cast<const v1_1::effect_param*>(effects.get());
    }

    inline v1_1::effect_param* effects_v1_1() noexcept
    {
        return static_cast<v1_1::effect_param*>(effects.get());
    }

    inline const v1_2::effect_param* effects_v1_2() const noexcept
    {
        return static_cast<const v1_2::effect_param*>(effects.get());
    }

    inline v1_2::effect_param* effects_v1_2() noexcept
    {
        return static_cast<v1_2::effect_param*>(effects.get());
    }

    inline const v1_3::effect_param* effects_v1_3() const noexcept
    {
        return static_cast<const v1_3::effect_param*>(effects.get());
    }

    inline v1_3::effect_param* effects_v1_3() noexcept
    {
        return static_cast<v1_3::effect_param*>(effects.get());
    }

    inline const v1_4::effect_param* effects_v1_4() const noexcept
    {
        return static_cast<const v1_4::effect_param*>(effects.get());
    }

    inline v1_4::effect_param* effects_v1_4() noexcept
    {
        return static_cast<v1_4::effect_param*>(effects.get());
    }

    inline const v1_5::effect_param* effects_v1_5() const noexcept
    {
        return static_cast<const v1_5::effect_param*>(effects.get());
    }

    inline v1_5::effect_param* effects_v1_5() noexcept
    {
        return static_cast<v1_5::effect_param*>(effects.get());
    }

    inline const v1_6::effect_param* effects_v1_6() const noexcept
    {
        return static_cast<const v1_6::effect_param*>(effects.get());
    }

    inline v1_6::effect_param* effects_v1_6() noexcept
    {
        return static_cast<v1_6::effect_param*>(effects.get());
    }

    HL_API void fix(bina::endian_flag endianFlag);
};

HL_STATIC_ASSERT_SIZE(effect, 16);
} // grif
} // hl
#endif
