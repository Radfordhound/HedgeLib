#include "hedgelib/io/hl_hh_mirage.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace mirage
{
void raw_header::fix()
{
    // Swap endianness if necessary.
#ifndef HL_IS_BIG_ENDIAN
    endian_swap();
#endif

    // Fix header offsets.
    data.fix(this);
    offTable.fix(this);
    fileName.fix(this);

    // Endian-swap offset count if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hl::endian_swap(*offTable);
#endif

    // Fix file offsets.
    offsets_fix(offsets(), data.get());
}

void raw_header::start_write(u32 version, stream& stream)
{
    // Generate placeholder HH mirage header.
    raw_header rawHeader =
    {
        0U,                                                             // fileSize
        version,                                                        // version
        0U,                                                             // dataSize
        static_cast<u32>(sizeof(raw_header)),                           // data
        0U,                                                             // offTable
        0U                                                              // eof
    };

    // Swap endianness if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawHeader.endian_swap();
#endif

    // Write placeholder header.
    stream.write_obj(rawHeader);
}

void raw_header::finish_write(std::size_t headerPos,
    off_table& offTable, stream& stream, const char* fileName)
{
    // Pad file for offset table.
    stream.pad(4);

    // Get offset table position.
    const std::size_t offTablePos = stream.tell();

    // Get offset count.
    u32 offCount = static_cast<u32>(offTable.size());

    // Endian-swap offset count if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hl::endian_swap(offCount);
#endif

    // Write offset count.
    stream.write_obj(offCount);

    // Write offset table.
    const std::size_t dataPos = (headerPos + sizeof(raw_header));
    offsets_write(dataPos, offTable, stream);

    // Get end of stream position.
    const std::size_t endPos = stream.tell();

    // Jump to header position.
    stream.jump_to(headerPos);

    // Fill-in header.
    raw_header hhHeader =
    {
        static_cast<u32>(endPos - headerPos),       // fileSize
        0U,                                         // version
        static_cast<u32>(offTablePos - dataPos),    // dataSize
        static_cast<u32>(sizeof(raw_header)),       // data
        static_cast<u32>(offTablePos - headerPos),  // offTable
        0U                                          // fileName
    };

    // Set fileName offset and increase fileSize if file name writing was requested.
    std::size_t fileNameSize;
    if (fileName)
    {
        fileNameSize = text::size(fileName);
        hhHeader.fileName = hhHeader.fileSize;
        hhHeader.fileSize += static_cast<u32>(fileNameSize * sizeof(char));
        hhHeader.fileSize = align(hhHeader.fileSize, 4);
    }

    // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hhHeader.endian_swap();
#endif

    // Fill-in header fileSize.
    stream.write_obj(hhHeader.fileSize);

    // Skip over version number.
    stream.jump_ahead(sizeof(hhHeader.version));

    // Fill-in remaining header values.
    stream.write(sizeof(raw_header) - offsetof(
        raw_header, dataSize), &hhHeader.dataSize);

    // Jump back to end of stream.
    stream.jump_to(endPos);

    // Write file name if requested.
    if (fileName)
    {
        stream.write_arr(fileNameSize, fileName);
        stream.pad(4);
    }
}

namespace sample_chunk
{
static const raw_node* in_get_child(const raw_node* curNode, const char* name, bool recursive)
{
    while (curNode)
    {
        // Check the current node's name and return if we've found a match.
        if (std::strncmp(curNode->name, name, 8) == 0)
        {
            return curNode;
        }

        // If recursion is allowed and this node has children, recurse through them.
        if (recursive)
        {
            const raw_node* childMatch = in_get_child(
                curNode->children(), name, recursive);

            if (childMatch) return childMatch;
        }

        // Get the next node.
        curNode = curNode->next();
    }

    return nullptr;
}

const raw_node* raw_node::next_of_name(const char* name) const
{
    // Create "full" 8-character node name (with added spaces if necessary).
    char nameBuf[9];
    make_node_name(name, nameBuf);

    // Search for a node with a matching name and return it if found.
    return in_get_child(next(), nameBuf, false);
}

const raw_node* raw_node::get_child(const char* name, bool recursive) const
{
    // Create "full" 8-character node name (with added spaces if necessary).
    char nameBuf[9];
    make_node_name(name, nameBuf);

    // Search for a node with a matching name and return it if found.
    return in_get_child(children(), nameBuf, recursive);
}

void raw_node::start_write(const char* name, stream& stream, u32 value)
{
    // Create "full" 8-character node name (with added spaces if necessary).
    char nameBuf[9];
    make_node_name(name, nameBuf);

    // Generate placeholder sample chunk node.
    raw_node rawNode;
    rawNode.flags = 0;
    rawNode.value = value;
    std::memcpy(rawNode.name, nameBuf, 8);

    // Endian-swap sample chunk node if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawNode.endian_swap();
#endif

    // Write sample chunk node.
    stream.write_obj(rawNode);
}

void raw_node::finish_write(std::size_t nodePos,
    node_flags flags, stream& stream)
{
    // Generate file size and flags.
    const std::size_t endPos = stream.tell();
    u32 fileSizeAndFlags = (static_cast<u32>(endPos - nodePos) |
        static_cast<u32>(flags));

    // Endian-swap file size and flags if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hl::endian_swap(fileSizeAndFlags);
#endif

    // Fill-in file size and flags.
    stream.jump_to(nodePos);
    stream.write_obj(fileSizeAndFlags);
    stream.jump_to(endPos);
}

const raw_node* raw_header::get_node(const char* name, bool recursive) const
{
    // Treat this sample chunk header as a sample chunk node and get its children.
    return reinterpret_cast<const raw_node*>(this)->get_child(name, recursive);
}

static void in_swap_recursive(raw_node* curNode)
{
    do
    {
        // Swap the current node.
        curNode->endian_swap();

        // If this node has children, swap them recursively.
        raw_node* children = curNode->children();
        if (children)
        {
            in_swap_recursive(children);
        }

        // Get the next node.
        curNode = curNode->next();
    }
    while (curNode);
}

void raw_header::fix()
{
    // Swap endianness if necessary.
#ifndef HL_IS_BIG_ENDIAN
    endian_swap();
#endif

    // Fix header offsets.
    offTable.fix(this);

    // Recursively swap nodes if necessary and fix file offsets.
    raw_node* nodesPtr = nodes();
    if (nodesPtr)
    {
#ifndef HL_IS_BIG_ENDIAN
        in_swap_recursive(nodesPtr);
#endif

        // Fix file offsets.
        offsets_fix(offsets(), nodesPtr);
    }
}

void raw_header::start_write(stream& stream)
{
    // Write placeholder HH sample chunk header.
    stream.write_nulls(sizeof(raw_header));
}

void raw_header::finish_write(std::size_t headerPos,
    std::size_t dataPos, off_table& offTable, stream& stream)
{
    // Pad file for offset table.
    stream.pad(16);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    offsets_write(dataPos, offTable, stream);

    // Get end of stream position.
    const std::size_t endPos = stream.tell();

    // Jump to header position.
    stream.jump_to(headerPos);

    // Fill-in sample chunk header.
    raw_header rawHeader =
    {
        (static_cast<u32>(node_flags::is_root) |                        // fileSize
            static_cast<u32>(endPos - headerPos)),

        raw_header_magic,                                               // magic
        static_cast<u32>(offTablePos - headerPos),                      // offTable
        static_cast<u32>(offTable.size())                               // offCount
    };

    // Endian-swap header if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawHeader.endian_swap();
#endif

    // Write sample chunk header.
    stream.write_obj(rawHeader);

    // Jump back to end of stream.
    stream.jump_to(endPos);
}

void property::set_name(const char* name)
{
    make_node_name(name, m_name);
}

property::property() noexcept
{
    m_name[0] = '\0';
}

property::property(const char* name, u32 value) :
    value(value)
{
    set_name(name);
}

property::property(const raw_node& rawNode) :
    value(rawNode.value)
{
    std::memcpy(m_name, rawNode.name, 8);
    m_name[8] = '\0';
}

void make_node_name(const char* name, char* dst)
{
    std::size_t i;

    // Copy up to 8 characters into name buffer.
    for (i = 0; i < 8 && name[i] != '\0'; ++i)
    {
        dst[i] = name[i];
    }

    // Fill-in any remaining characters with spaces.
    while (i < 8)
    {
        dst[i++] = ' ';
    }

    // Append null terminator.
    dst[i] = '\0';
}
} // sample_chunk

void fix(void* rawData)
{
    /* Sample chunk header */
    if (has_sample_chunk_header_unfixed(rawData))
    {
        sample_chunk::fix(rawData);
    }

    /* Standard header */
    else
    {
        raw_header* hhHeader = static_cast<raw_header*>(rawData);
        hhHeader->fix();
    }
}

void offsets_fix(off_table_handle offsets, void* base)
{
    // Fix all the offsets in the offset table.
    for (auto& relOffPos : offsets)
    {
        // Endian swap offset position if necessary.
#ifndef HL_IS_BIG_ENDIAN
        hl::endian_swap(relOffPos);
#endif

        // Get pointer to current offset.
        off32<void>* curOff = ptradd<off32<void>>(base, relOffPos);

        // Endian swap offset if necessary.
#ifndef HL_IS_BIG_ENDIAN
        hl::endian_swap(*curOff);
#endif

        // Fix offset.
        curOff->fix(base);
    }
}

void offsets_write_no_sort(std::size_t dataPos,
    const off_table& offTable, stream& stream)
{
    for (std::size_t offPos : offTable)
    {
        // Get relative offset position.
        u32 relOffPos = static_cast<u32>(offPos - dataPos);

        // Endian-swap relative offset position if necessary.
#ifndef HL_IS_BIG_ENDIAN
        endian_swap(relOffPos);
#endif

        // Write relative offset position.
        stream.write_obj(relOffPos);
    }
}

void offsets_write(std::size_t dataPos,
    off_table& offTable, stream& stream)
{
    // Sort offset table.
    std::sort(offTable.begin(), offTable.end());

    // Write sorted offsets.
    offsets_write_no_sort(dataPos, offTable, stream);
}
} // mirage
} // hh
} // hl
