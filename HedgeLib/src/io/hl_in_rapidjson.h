#include "hedgelib/io/hl_file.h"
#include "hedgelib/hl_memory.h"

#define RAPIDJSON_HAS_STDSTRING 1
// TODO: Support SIMD intrinsics!!!
#include <rapidjson/reader.h>
#include <rapidjson/prettywriter.h>

namespace hl
{
namespace internal
{
// NOTE: The template parameter here is a minor optimization
// to help the compiler avoid virtual function calls.
template<typename StreamType = stream>
class in_rapidjson_input_stream_wrapper
{
    constexpr static std::size_t in_max_stack_buf_size = 10000U;        // 10 KB
    constexpr static std::size_t in_max_heap_buf_size  = 250000000U;    // 250 MB

    std::size_t                                         m_totalByteCount;
    stack_or_heap_memory<char, in_max_stack_buf_size>   m_buffer;
    StreamType*                                         m_stream;
    char*                                               m_curChar;
    char*                                               m_bufEnd;
    std::size_t                                         m_readBytesCount = 0;

    void in_read_chunk()
    {
        // Read one chunk of data into the buffer.
        m_curChar = m_buffer.data();
        const auto bufSize = static_cast<std::size_t>(
            m_bufEnd - m_buffer.data());

        const auto readBytesCount = m_stream->read(
            bufSize, m_buffer.data());

        // Increase total read byte count and set new buffer end.
        m_readBytesCount += readBytesCount;
        m_bufEnd = (m_curChar + readBytesCount);

        // Append null terminator to the end of the buffer
        // if necessary to signify EOF to RapidJSON.
        if (readBytesCount < bufSize)
        {
            *m_bufEnd = '\0';
            ++m_bufEnd;
        }
    }

public:
    using Ch = char;

    inline Ch Peek() const
    {
        return *m_curChar;
    }

    inline const Ch* Peek4() const
    {
        return ((m_curChar + 4) <= m_bufEnd) ?
            m_curChar : nullptr;
    }

    Ch Take()
    {
        const Ch val = *m_curChar;
        if (++m_curChar == m_bufEnd)
        {
            if (m_readBytesCount < m_totalByteCount)
            {
                in_read_chunk();
            }
            else
            {
                // We've reached EOF; push current character pointer back by one.
                --m_curChar;
            }
        }

        return val;
    }

    std::size_t Tell() const noexcept
    {
        return (m_readBytesCount + static_cast<std::size_t>(
            m_curChar - m_buffer.data()));
    }

    inline void Put(Ch)
    {
        throw unsupported_exception();
    }

    inline void Flush()
    {
        throw unsupported_exception();
    }

    inline Ch* PutBegin()
    {
        throw unsupported_exception();
    }

    inline size_t PutEnd(Ch*)
    {
        throw unsupported_exception();
    }

    in_rapidjson_input_stream_wrapper(StreamType& stream) :
        m_totalByteCount(stream.get_size()),
        m_buffer(no_value_init, std::min<>(in_max_heap_buf_size, m_totalByteCount + 1)),
        m_stream(&stream),
        m_curChar(m_buffer.data()),
        m_bufEnd(m_buffer.end())
    {
        in_read_chunk();
    }
};

class in_rapidjson_output_stream_wrapper
{
    constexpr static std::size_t in_max_buf_size = 10000U;  // 10 KB

    stream*                                             m_stream;
    char*                                               m_curChar;
    char                                                m_buffer[in_max_buf_size];

public:
    using Ch = char;

    void Put(Ch c)
    {
        *m_curChar = c;
        if (++m_curChar == std::end(m_buffer))
        {
            Flush();
        }
    }

    void PutN(Ch c, size_t n)
    {
        // NOTE: Based heavily on the FileWriteStream::PutN function from RapidJSON.

        // Write and flush as many times as necessary until we have enough space
        // left in the buffer to contain the rest of the characters we want to put.
        auto freeByteCount = static_cast<std::size_t>(std::end(m_buffer) - m_curChar);
        while (n > freeByteCount)
        {
            std::memset(m_curChar, c, freeByteCount);
            m_curChar += freeByteCount;
            Flush();
            n -= freeByteCount;
            freeByteCount = in_max_buf_size;
        }

        // Write any remaining characters to the buffer.
        if (n > 0)
        {
            std::memset(m_curChar, c, n);
            m_curChar += n;

            if (m_curChar == std::end(m_buffer))
            {
                Flush();
            }
        }
    }

    inline void Flush()
    {
        if (m_curChar == m_buffer) return;
        
        const auto byteCount = static_cast<std::size_t>(m_curChar - m_buffer);
        m_stream->write_all(byteCount, m_buffer);
        m_curChar = m_buffer;
    }

    inline Ch* PutBegin()
    {
        throw unsupported_exception();
    }

    inline size_t PutEnd(Ch*)
    {
        throw unsupported_exception();
    }

    in_rapidjson_output_stream_wrapper(stream& stream) :
        m_stream(&stream),
        m_curChar(m_buffer) {}
};

/**
 * @brief Convenience wrapper around RapidJSON PrettyWriter.
 * 
 * Uses HedgeLib output stream wrapper as default stream type and
 * replaces all functions that can actually fail and return false
 * on failure with wrapper functions that throw exceptions or
 * handle the error internally in a different way (such as writing
 * 0.0 instead of failing with NaN or Infinity double values) instead
 * so you don't have to check the return values.
 * 
 * NOTE: There are several RapidJSON PrettyWriter functions that theoretically
 * can return false on failure, but whose implementations NEVER actually
 * return false. As such, these functions currently are not wrapped, but
 * you still don't actually have to check their return values.
 */
class in_rapidjson_writer : public rapidjson::PrettyWriter<in_rapidjson_output_stream_wrapper>
{
public:
    inline void Double(double d)
    {
        PrettyWriter::Double((rapidjson::internal::Double(d).IsNanOrInf()) ?
            0.0 : d);
    }

    inline void RawNumber(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        if (!PrettyWriter::RawNumber(str, length, copy))
        {
            throw std::runtime_error("Failed to serialize JSON raw number");
        }
    }

    inline void String(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        if (!PrettyWriter::String(str, length, copy))
        {
            throw std::runtime_error("Failed to serialize JSON string");
        }
    }

    inline void String(const std::basic_string<Ch>& str)
    {
        if (!PrettyWriter::String(str))
        {
            throw std::runtime_error("Failed to serialize JSON string");
        }
    }

    inline void String(const Ch* const& str)
    {
        if (!PrettyWriter::String(str))
        {
            throw std::runtime_error("Failed to serialize JSON string");
        }
    }

    inline void Key(const Ch* str, rapidjson::SizeType length, bool copy = false)
    {
        if (!PrettyWriter::Key(str, length, copy))
        {
            throw std::runtime_error("Failed to serialize JSON key");
        }
    }

    inline void Key(const std::basic_string<Ch>& str)
    {
        if (!PrettyWriter::Key(str))
        {
            throw std::runtime_error("Failed to serialize JSON key");
        }
    }

    inline void Key(const Ch* const& str)
    {
        if (!PrettyWriter::Key(str))
        {
            throw std::runtime_error("Failed to serialize JSON key");
        }
    }

    inline void RawValue(const Ch* json, size_t length, rapidjson::Type type)
    {
        if (!PrettyWriter::RawValue(json, length, type))
        {
            throw std::runtime_error("Failed to serialize JSON raw value");
        }
    }

    in_rapidjson_writer(in_rapidjson_output_stream_wrapper& stream) :
        PrettyWriter(stream)
    {
        indentCharCount_ = 2;
    }
};

template<typename HandlerType>
void in_parse_json(HandlerType& handler, const void* rawData, std::size_t rawDataSize)
{
    rapidjson::Reader reader;
    rapidjson::MemoryStream jsonStream(
        static_cast<const char*>(rawData),
        rawDataSize);

    if (!reader.Parse(jsonStream, handler))
    {
        // TODO: Throw error with message!
        throw std::runtime_error("Failed to parse JSON");
    }
}

template<typename HandlerType>
void in_read_json(HandlerType& handler, stream& stream)
{
    rapidjson::Reader reader;
    in_rapidjson_input_stream_wrapper jsonStream(stream);

    if (!reader.Parse(jsonStream, handler))
    {
        // TODO: Throw error with message!
        throw std::runtime_error("Failed to parse JSON");
    }
}

template<typename HandlerType>
void in_load_json(HandlerType& handler, const nchar* filePath)
{
    rapidjson::Reader reader;
    file_stream stream(filePath, file::mode::read);
    in_rapidjson_input_stream_wrapper jsonStream(stream);

    if (!reader.Parse(jsonStream, handler))
    {
        // TODO: Throw error with message!
        throw std::runtime_error("Failed to parse JSON");
    }
}

template<typename HandlerType>
inline void in_load_json(HandlerType& handler, const nstring& filePath)
{
    in_load_json(handler, filePath.c_str());
}
} // internal
} // hl

namespace rapidjson
{
template<>
inline void PutN(
    hl::internal::in_rapidjson_output_stream_wrapper& stream,
    char c, size_t n)
{
    stream.PutN(c, n);
}
} // rapidjson
