#include "hedgelib/io/hl_hh_mirage.h"
#include <cstring>
#include <cassert>

namespace hl
{
namespace hh
{
namespace mirage
{
namespace standard
{
const_off_table_handle raw_header::offsets() const noexcept
{
    return const_off_table_handle(offTable.get() + 1, *offTable);
}

off_table_handle raw_header::offsets() noexcept
{
    return off_table_handle(offTable.get() + 1, *offTable);
}

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

void raw_header::start_write(stream& stream)
{
    // Write placeholder header.
    stream.write_nulls(sizeof(raw_header));
}

void raw_header::finish_write(std::size_t headerPos,
    std::size_t dataPos, u32 version, off_table& offTable,
    stream& stream, const char* fileName)
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

    // Write offset table.
    stream.write_obj(offCount);
    offsets_write(dataPos, offTable, stream);

    // Get end of stream position.
    const std::size_t endPos = stream.tell();

    // Jump to header position.
    stream.jump_to(headerPos);

    // Fill-in header.
    raw_header hhHeader =
    {
        static_cast<u32>(endPos - headerPos),       // fileSize
        version,                                    // version
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

    // Fill-in header.
    stream.write_obj(hhHeader);

    // Jump back to end of stream.
    stream.jump_to(endPos);

    // Write file name if requested.
    if (fileName)
    {
        stream.write_arr(fileNameSize, fileName);
        stream.pad(4);
    }
}
} // standard

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

void raw_node::start_write(const char* name, u32 value, stream& stream)
{
    // Generate placeholder sample chunk node.
    raw_node rawNode;
    rawNode.flags = 0;
    rawNode.value = value;
    make_node_name(name, rawNode.name, false);

    // Endian-swap sample chunk node if necessary.
#ifndef HL_IS_BIG_ENDIAN
    rawNode.endian_swap();
#endif

    // Write sample chunk node.
    stream.write_obj(rawNode);
}

static void in_raw_node_finish_write(std::size_t nodePos,
    u32 nodeSizeAndFlags, stream& stream)
{
    // Endian-swap node size and flags if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hl::endian_swap(nodeSizeAndFlags);
#endif

    // Fill-in node size and flags.
    stream.jump_to(nodePos);
    stream.write_obj(nodeSizeAndFlags);
}

void raw_node::finish_write(std::size_t nodePos,
    std::size_t nodeEndPos, node_flags flags, stream& stream)
{
    // Generate node size and flags.
    const std::size_t endPos = stream.tell();
    const u32 nodeSizeAndFlags = (static_cast<u32>(flags) |
        static_cast<u32>(nodeEndPos - nodePos));

    // Finish writing node and jump back to end position.
    in_raw_node_finish_write(nodePos, nodeSizeAndFlags, stream);
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
    std::size_t basePos, off_table& offTable, stream& stream)
{
    // Pad file for offset table.
    stream.pad(16);

    // Write offset table.
    const std::size_t offTablePos = stream.tell();
    offsets_write(basePos, offTable, stream);

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

void make_node_name(const char* name, char* dst, bool appendNull)
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
    if (appendNull)
    {
        dst[i] = '\0';
    }
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
        standard::fix(rawData);
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

void offsets_write_no_sort(std::size_t basePos,
    const off_table& offTable, stream& stream)
{
    for (std::size_t offPos : offTable)
    {
        // Get relative offset position.
        u32 relOffPos = static_cast<u32>(offPos - basePos);

        // Endian-swap relative offset position if necessary.
#ifndef HL_IS_BIG_ENDIAN
        endian_swap(relOffPos);
#endif

        // Write relative offset position.
        stream.write_obj(relOffPos);
    }
}

void offsets_write(std::size_t basePos,
    off_table& offTable, stream& stream)
{
    // Sort offset table.
    std::sort(offTable.begin(), offTable.end());

    // Write sorted offsets.
    offsets_write_no_sort(basePos, offTable, stream);
}

void writer::start(header_type headerType)
{
    // Clear nodes and offsets.
    m_nodes.clear();
    m_offsets.clear();

    // Store header position and type.
    m_headerPos = m_stream->tell();
    m_headerType = headerType;

    if (headerType == header_type::standard)
    {
        // Store base position.
        m_basePos = (m_headerPos + sizeof(standard::raw_header));

        // Start writing standard header.
        standard::raw_header::start_write(*m_stream);
    }
    else
    {
        // Store base position.
        m_basePos = (m_headerPos + sizeof(sample_chunk::raw_header));

        // Start writing sample chunk header.
        sample_chunk::raw_header::start_write(*m_stream);
    }
}

void writer::start_node(const char* name, u32 value)
{
    using namespace sample_chunk;

    // Update existing nodes if necessary.
    const std::size_t curNodeIndex = m_nodes.size();
    if (m_curUnfinishedNodeIndex != SIZE_MAX)
    {
        // If the current unfinished node has a child, we need to unset its
        // "last child" flag, as it's about to no longer be the last child.
        auto& curUnfinishedNode = m_nodes[m_curUnfinishedNodeIndex];
        if (curUnfinishedNode.lastChildIndex != 0)
        {
            auto& lastChild = m_nodes[curUnfinishedNode.lastChildIndex];
            lastChild.flagsAndSize &= ~static_cast<u32>(node_flags::is_last_child);
        }

        // Otherwise, this node will be the first child of the current unfinished
        // node, so we want to unset the unfinished node's "is leaf" flag.
        else
        {
            curUnfinishedNode.flagsAndSize &= ~static_cast<u32>(node_flags::is_leaf);
        }

        // Update the last current unfinished node's last child index.
        curUnfinishedNode.lastChildIndex = curNodeIndex;
    }

    // Add a new node and update the current unfinished node index.
    m_nodes.emplace_back(m_stream->tell(), m_curUnfinishedNodeIndex);
    m_curUnfinishedNodeIndex = curNodeIndex;

    // Start writing the new node.
    raw_node::start_write(name, value, *m_stream);
}

void writer::finish_node()
{
    using namespace sample_chunk;

    assert(m_curUnfinishedNodeIndex != SIZE_MAX &&
        "You must call writer::finish_node exactly once per writer::start_node call");

    // Mark the current unfinished node as being finished and store its size.
    auto& curNode = m_nodes[m_curUnfinishedNodeIndex];
    curNode.flagsAndSize |= static_cast<u32>(m_stream->tell() - curNode.pos);
    curNode.mark_finished();

    // Update the current unfinished node index.
    m_curUnfinishedNodeIndex = curNode.parentIndex;
}

void writer::start_data(u32 version)
{
    // Store data version.
    m_dataVersion = version;

    if (m_headerType == header_type::standard)
    {
        // Update base position.
        m_basePos = m_stream->tell();
    }

    else if (!m_nodes.empty())
    {
        // Start writing contexts node.
        start_node("Contexts", version);

        // Finish writing contexts node now if necessary.
        if (m_headerType == header_type::sample_chunk_v2)
        {
            finish_node();
        }
    }
}

void writer::fix_offset(std::size_t pos)
{
    m_stream->fix_off32(m_basePos, pos, needs_endian_swap, m_offsets);
}

void writer::finish_data()
{
    // Finish writing contexts node if necessary.
    if (m_headerType == header_type::sample_chunk_v1)
    {
        finish_node();
    }
}

void writer::finish(const char* fileName)
{
    using namespace sample_chunk;

    if (m_headerType == header_type::standard)
    {
        // Finish writing standard header.
        standard::raw_header::finish_write(m_headerPos,
            m_basePos, m_dataVersion, m_offsets,
            *m_stream, fileName);
    }
    else
    {
        // Mark all unfinished sample chunk nodes as being finished.
        while (m_curUnfinishedNodeIndex != SIZE_MAX)
        {
            finish_node();
        }

        // Actually finish writing all sample chunk nodes to the stream.
        const std::size_t endPos = m_stream->tell();
        for (auto& node : m_nodes)
        {
            in_raw_node_finish_write(node.pos, node.flagsAndSize, *m_stream);
        }

        m_stream->jump_to(endPos);

        // Finish writing sample chunk header.
        sample_chunk::raw_header::finish_write(m_headerPos,
            m_basePos, m_offsets, *m_stream);
    }
}

writer::writer(hl::stream& stream) :
    in_writer_base(stream),
    m_headerPos(stream.tell()),
    m_basePos(m_headerPos) {}
} // mirage
} // hh
} // hl
