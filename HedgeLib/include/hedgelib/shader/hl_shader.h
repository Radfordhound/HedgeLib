#ifndef HL_SHADER_H_INCLUDED
#define HL_SHADER_H_INCLUDED
#include "../hl_blob.h"
#include <robin_hood.h>
//#include <vector>

namespace hl
{
enum class shader_constant_type
{
    const_bool,
    const_int,
    const_float
};

struct shader_constant_info
{
    /** @brief See hl::shader_constant_type. */
    shader_constant_type type;
    /** @brief The offset into the constant buffer, in bytes, that this constant is located at. */
    unsigned int offset;
    /**
        @brief The size of this constant, in bytes. Can be larger than expected.
        For example: A 4x4 matrix can be represented with a float-typed constant of size 64.
    */
    unsigned int size;

    shader_constant_info(shader_constant_type type,
        unsigned int offset, unsigned int size) noexcept :
        type(type), offset(offset), size(size) {}
};

enum class shader_resource_type
{
    res_texture,
    res_sampler
};

struct shader_resource_info
{
    /** @brief See hl::shader_resource_type. */
    shader_resource_type type;
    /** @brief The index of the register this resource is to be bound to. */
    unsigned int registerIndex;

    shader_resource_info(shader_resource_type type,
        unsigned int registerIndex) noexcept : type(type),
        registerIndex(registerIndex) {}
};

struct shader_constant_buffer_info
{
    /** @brief The index of the register this constant buffer is to be bound to. */
    unsigned int registerIndex;
    /** @brief The size of this constant buffer, in bytes. */
    unsigned int size;
    /** @brief The constant values that are to be placed within the constant buffer. */
    robin_hood::unordered_map<std::string, shader_constant_info> constants;

    shader_constant_buffer_info(unsigned int registerIndex,
        unsigned int size) noexcept : registerIndex(registerIndex),
        size(size) {}
};

struct shader_layout
{
    robin_hood::unordered_map<std::string, shader_resource_info> resources;
    robin_hood::unordered_map<std::string, shader_constant_buffer_info> constantBuffers;
};

struct shader
{
    std::string name;
    blob bytecode;
    nullable_blob inputBytecode;
    shader_layout layout;

    HL_API shader(const char* name, std::size_t bytecodeSize, const void* bytecode,
        std::size_t inputBytecodeSize = 0U, const void* inputBytecode = nullptr);

    HL_API shader(const std::string& name, std::size_t bytecodeSize, const void* bytecode,
        std::size_t inputBytecodeSize = 0U, const void* inputBytecode = nullptr);

    HL_API shader(std::string&& name, std::size_t bytecodeSize, const void* bytecode,
        std::size_t inputBytecodeSize = 0U, const void* inputBytecode = nullptr);

    HL_API shader(const char* name, const nchar* bytecodeFilePath,
        const nchar* inputBytecodeFilePath = nullptr);

    HL_API shader(const std::string& name, const nchar* bytecodeFilePath,
        const nchar* inputBytecodeFilePath = nullptr);

    HL_API shader(std::string&& name, const nchar* bytecodeFilePath,
        const nchar* inputBytecodeFilePath = nullptr);

    HL_API shader(const char* name, const nstring& bytecodeFilePath);
    HL_API shader(const std::string& name, const nstring& bytecodeFilePath);
    HL_API shader(std::string&& name, const nstring& bytecodeFilePath);

    HL_API shader(const char* name, const nstring& bytecodeFilePath,
        const nstring& inputBytecodeFilePath);

    HL_API shader(const std::string& name, const nstring& bytecodeFilePath,
        const nstring& inputBytecodeFilePath);

    HL_API shader(std::string&& name, const nstring& bytecodeFilePath,
        const nstring& inputBytecodeFilePath);
};

//enum class shader_permutation_type
//{
//    pixel,
//    vertex
//};
//
//class shader_list
//{
//    class shader_permutation
//    {
//        shader_permutation_type type;
//        std::size_t all;
//
//    public:
//        inline shader_permutation_type type() const noexcept
//        {
//            return m_type;
//        }
//
//        inline unsigned int index() const noexcept
//        {
//            return m_index;
//        }
//
//        shader_permutation(shader_permutation_type type,
//            unsigned int index) noexcept : m_type(type),
//            m_index(index) {}
//    };
//
//    std::vector<std::string> m_shaderNames;
//    radix_tree<shader_permutation> m_permutations;
//    std::vector<std::size_t> m_shaderNameIndices;
//
//public:
//    HL_API void add_permutation(const char* name, shader_permutation_type type);
//
//    HL_API shader_list(const char* baseShaderName);
//    HL_API shader_list(const std::string& baseShaderName);
//    HL_API shader_list(std::string&& baseShaderName);
//};
} // hl
#endif
