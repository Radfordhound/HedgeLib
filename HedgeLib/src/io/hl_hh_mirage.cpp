#include "hedgelib/io/hl_hh_mirage.h"
#include <cstring>

namespace hl
{
namespace hh
{
namespace mirage
{
namespace standard
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

void raw_node::start_write(const char* name, stream& stream, u32 value)
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
    u32 fileSizeAndFlags, stream& stream)
{
    // Endian-swap file size and flags if necessary.
#ifndef HL_IS_BIG_ENDIAN
    hl::endian_swap(fileSizeAndFlags);
#endif

    // Fill-in file size and flags.
    stream.jump_to(nodePos);
    stream.write_obj(fileSizeAndFlags);
}

void raw_node::finish_write(std::size_t nodePos,
    std::size_t nodeEndPos, node_flags flags, stream& stream)
{
    // Generate file size and flags.
    const std::size_t endPos = stream.tell();
    u32 fileSizeAndFlags = (static_cast<u32>(flags) |
        static_cast<u32>(nodeEndPos - nodePos));

    // Finish writing node and jump back to end position.
    in_raw_node_finish_write(nodePos, fileSizeAndFlags, stream);
    stream.jump_to(endPos);
}

void raw_node::finish_write(std::size_t nodePos,
    node_flags flags, stream& stream)
{
    // Generate file size and flags.
    const std::size_t endPos = stream.tell();
    u32 fileSizeAndFlags = (static_cast<u32>(flags) |
        static_cast<u32>(endPos - nodePos));

    // Finish writing node and jump back to end position.
    in_raw_node_finish_write(nodePos, fileSizeAndFlags, stream);
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

node_writer::node_writer(writer& writer) :
    m_writer(&writer),
    m_pos(writer.m_stream->tell()) {}

void node_writer::in_real_finish_write()
{
    // Call finish_write if we haven't yet.
    if (!is_finished())
    {
        finish_write();
    }

    // Generate file size and flags.
    const std::size_t endPos = m_writer->m_stream->tell();
    u32 fileSizeAndFlags = (m_flags & static_cast<u32>(~node_flags::is_root));

    // Finish writing node and jump back to end position.
    in_raw_node_finish_write(m_pos, fileSizeAndFlags, *m_writer->m_stream);
    m_writer->m_stream->jump_to(endPos);
}

void node_writer::start_write(const char* name, u32 value)
{
    raw_node::start_write(name, *m_writer->m_stream, value);
}

node_writer* node_writer::add_child(const char* name, u32 value)
{
    // Unset this node's leaf flag, as this node is no longer a leaf.
    m_flags &= static_cast<u32>(~node_flags::is_leaf);

    // Add a new child node to the writer's global node list.
    const std::size_t newLastChildIndex = m_writer->m_nodes.size();
    std::unique_ptr<sample_chunk::node_writer> newLastChildTmpPtr(
        new sample_chunk::node_writer(*m_writer));

    m_writer->m_nodes.emplace_back(std::move(newLastChildTmpPtr));

    // Start writing the new child node.
    node_writer& newLastChild = *m_writer->m_nodes[newLastChildIndex];
    newLastChild.start_write(name, value);

    // If this node already had a child, unset that child's last child flag, as
    // that node is no longer the last child of this node.
    if (m_lastChildIndex != 0)
    {
        m_writer->m_nodes[m_lastChildIndex]->m_flags &=
            static_cast<u32>(~node_flags::is_last_child);
    }

    // Update this node's last child index, and return a reference to the last child.
    m_lastChildIndex = newLastChildIndex;
    return &newLastChild;
}

void node_writer::finish_write()
{
    // HACK: Use root flag as a marker that means we finished writing this node.
    m_flags |= (static_cast<u32>(node_flags::is_root) |
        static_cast<u32>(m_writer->stream().tell() - m_pos));
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
void off_handle::fix()
{
    m_writer->stream().fix_off32(m_writer->data_pos(),
        m_offPos, needs_endian_swap, m_writer->off_table());
}

sample_chunk::node_writer* writer::start_write(header_type headerType,
    const char* nodeName, u32 nodeValue)
{
    // Store header type and position.
    m_headerType = headerType;
    m_headerPos = m_stream->tell();

    if (headerType == header_type::standard)
    {
        // Store data position.
        m_dataPos = (m_headerPos + sizeof(standard::raw_header));

        // Start writing standard header.
        standard::raw_header::start_write(*m_stream);
    }
    else
    {
        // Store data position.
        m_dataPos = (m_headerPos + sizeof(sample_chunk::raw_header));

        // Start writing sample chunk header.
        sample_chunk::raw_header::start_write(*m_stream);

        // Start writing initial node if requested.
        if (nodeName)
        {
            // Create sample chunk node writer.
            std::unique_ptr<sample_chunk::node_writer> nodeTmpPtr(
                new sample_chunk::node_writer(*this));

            m_nodes.emplace_back(std::move(nodeTmpPtr));

            // Start writing initial sample chunk node.
            sample_chunk::node_writer& node = *m_nodes.back();
            node.start_write(nodeName, nodeValue);
            return &node;
        }
    }

    return nullptr;
}

void writer::start_write_data(u32 version)
{
    // Store data version.
    m_dataVersion = version;

    if (m_headerType == header_type::standard)
    {
        // Update data position.
        m_dataPos = m_stream->tell();
    }

    else if (!m_nodes.empty())
    {
        // Start writing contexts node.
        auto& node = *m_nodes.front();
        auto contextsNode = node.add_child("Contexts", version);

        // Finish writing contexts node now if necessary.
        if (m_headerType == header_type::sample_chunk_v2)
        {
            contextsNode->finish_write();
        }
    }
}

off_handle writer::add_offset(std::size_t relOffPos)
{
    // Get absolute offset position.
    const std::size_t offPos = (m_stream->tell() + relOffPos);

    // Construct a handle for this offset and return it.
    return off_handle(*this, offPos);
}

void writer::finish_write(const char* fileName)
{
    if (m_headerType == header_type::standard)
    {
        // Finish writing standard header.
        standard::raw_header::finish_write(m_headerPos,
            m_dataPos, m_dataVersion, m_offTable,
            *m_stream, fileName);
    }
    else
    {
        // Finish writing all non-initial sample chunk nodes.
        for (std::size_t i = 1; i < m_nodes.size(); ++i)
        {
            m_nodes[i]->in_real_finish_write();
        }

        // Pad to 16.
        m_stream->pad(16); // TODO: Do we do this with sample chunk v2??

        // Finish writing initial sample chunk node.
        if (!m_nodes.empty())
        {
            m_nodes.front()->in_real_finish_write();
        }

        // Finish writing sample chunk header.
        sample_chunk::raw_header::finish_write(m_headerPos,
            m_dataPos, m_offTable, *m_stream);
    }
}
} // mirage
} // hh
} // hl
