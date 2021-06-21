#include "hedgelib/shader/hl_shader.h"
#include <utility>

namespace hl
{
shader::shader(const char* name, std::size_t bytecodeSize, const void* bytecode,
    std::size_t inputBytecodeSize, const void* inputBytecode) : name(name),
    bytecode(bytecodeSize, bytecode), inputBytecode(inputBytecodeSize, inputBytecode) {}

shader::shader(const std::string& name, std::size_t bytecodeSize, const void* bytecode,
    std::size_t inputBytecodeSize, const void* inputBytecode) : name(name),
    bytecode(bytecodeSize, bytecode), inputBytecode(inputBytecodeSize, inputBytecode) {}

shader::shader(std::string&& name, std::size_t bytecodeSize, const void* bytecode,
    std::size_t inputBytecodeSize, const void* inputBytecode) :
    name(std::move(name)), bytecode(bytecodeSize, bytecode),
    inputBytecode(inputBytecodeSize, inputBytecode) {}

shader::shader(const char* name, const nchar* bytecodeFilePath,
    const nchar* inputBytecodeFilePath) : name(name),
    bytecode(bytecodeFilePath), inputBytecode(inputBytecodeFilePath) {}

shader::shader(const std::string& name, const nchar* bytecodeFilePath,
    const nchar* inputBytecodeFilePath) : name(name),
    bytecode(bytecodeFilePath), inputBytecode(inputBytecodeFilePath) {}

shader::shader(std::string&& name, const nchar* bytecodeFilePath,
    const nchar* inputBytecodeFilePath) : name(std::move(name)),
    bytecode(bytecodeFilePath), inputBytecode(inputBytecodeFilePath) {}

shader::shader(const char* name, const nstring& bytecodeFilePath) :
    shader(name, bytecodeFilePath.c_str()) {}

shader::shader(const std::string& name, const nstring& bytecodeFilePath) :
    shader(name, bytecodeFilePath.c_str()) {}

shader::shader(std::string&& name, const nstring& bytecodeFilePath) :
    shader(std::move(name), bytecodeFilePath.c_str()) {}

shader::shader(const char* name, const nstring& bytecodeFilePath,
    const nstring& inputBytecodeFilePath) : shader(name,
    bytecodeFilePath.c_str(), inputBytecodeFilePath.c_str()) {}

shader::shader(const std::string& name, const nstring& bytecodeFilePath,
    const nstring& inputBytecodeFilePath) : shader(name,
    bytecodeFilePath.c_str(), inputBytecodeFilePath.c_str()) {}

shader::shader(std::string&& name, const nstring& bytecodeFilePath,
    const nstring& inputBytecodeFilePath) : shader(std::move(name),
    bytecodeFilePath.c_str(), inputBytecodeFilePath.c_str()) {}
} // hl
