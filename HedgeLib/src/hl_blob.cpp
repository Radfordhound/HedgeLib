#include "hedgelib/hl_blob.h"
#include "hedgelib/io/hl_file.h"
#include <cstring>

namespace hl
{
blob& blob::operator=(const blob& other)
{
    if (&other != this)
    {
        u8* newData = new u8[other.m_size];
        std::memcpy(newData, other.m_data, other.m_size);

        delete[] m_data;
        m_data = newData;
        m_size = other.m_size;
    }

    return *this;
}

blob& blob::operator=(blob&& other) noexcept
{
    if (&other != this)
    {
        delete[] m_data;

        m_data = other.m_data;
        other.m_data = nullptr;

        m_size = other.m_size;
        other.m_size = 0;
    }

    return *this;
}

blob::blob(std::size_t size, const void* initialData) :
    m_data(new u8[size]), m_size(size)
{
    // Copy initial data into blob, if any.
    if (initialData)
    {
        std::memcpy(m_data, initialData, size);
    }
}

blob::blob(const nchar* filePath) :
    m_data(file::load(filePath, m_size).release()) {}

blob::blob(const blob& other) :
    m_data(new u8[other.m_size]),
    m_size(other.m_size)
{
    // Copy data from other blob into this blob.
    std::memcpy(m_data, other.m_data, other.m_size);
}

blob::blob(blob&& other) noexcept :
    m_data(other.m_data), m_size(other.m_size)
{
    other.m_data = nullptr;
    other.m_size = 0;
}

blob::~blob()
{
    delete[] m_data;
}
} // hl
