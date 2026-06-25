#include <stdexcept>

#include "hedgelib/hh_new/hl_hh_mirage.h"

namespace hl::hh_new::mirage
{
namespace sample_chunk
{
    void name::assign(std::string_view str)
    {
        // Copy up to 8 characters into the buffer.
        if (str.size() > data.size())
        {
            throw std::length_error(
                "The length of the given string exceeds the maximum "
                "allowed length for a sample chunk name"
            );
        }
        
        const auto it = std::copy(str.begin(), str.end(), data.begin());

        // Pad the buffer with spaces.
        std::fill(it, data.end(), ' ');
    }
}

rad::string reader::read_string(
    rad::allocator& allocator)
{
    // TODO: What encoding do Mirage files use?? Probably SHIFT-JIS ? Maybe UTF-8?
    return stream_->read_string8(allocator);
}

rad::string reader::read_string_at_offset(
    unsigned long long basePos,
    u64 stringOffset,
    rad::allocator& allocator)
{
    if (!stringOffset)
    {
        return rad::string{allocator};
    }
    else
    {
        const auto curPos = stream_->tell();
        stream_->jump_to(basePos + stringOffset);

        rad::string str = read_string(allocator);

        stream_->jump_to(curPos);
        return str;
    }
}

rad::vector<u32> reader::read_offset_table(
    u32 offsetCount,
    rad::allocator& allocator)
{
    rad::vector<u32> offPositions(rad::no_value_init, allocator, offsetCount);

    for (u32 i = 0; i < offsetCount; ++i)
    {
        offPositions[i] = read_u32();
    }

    return offPositions;
}

static std::array<char, 8> read_sample_chunk_node_name_(
    rad::stream& stream)
{
    std::array<char, 8> name;
    stream.read(name.data(), name.size());
    return name;
}

sample_chunk::node reader::read_sample_chunk_node()
{
    const auto flags = read_u32();
    const auto value = read_u32();

    return sample_chunk::node(
        flags,
        sample_chunk::property(
            read_sample_chunk_node_name_(*stream_),
            value
        )
    );
}

simple_header reader::read_simple_header()
{
    // TODO: Use C++20 delegated initializers.
    return simple_header{
        read_u32(),
        read_u32(),
        read_u32(),
        read_u32(),
        read_u32(),
        read_u32()
    };
}

sample_chunk::header reader::read_sample_chunk_header()
{
    // TODO: Use C++20 delegated initializers.
    return sample_chunk::header{
        read_u32(),
        read_u32(),
        read_u32(),
        read_u32()
    };
}

void writer::write_string(rad::cstring_view utf8Str)
{
    // TODO: What encoding do Mirage files use?? Probably SHIFT-JIS ? Maybe UTF-8?
    stream_->write_string8(utf8Str);
}

void writer::write_offset_table(const rad::vector<u32>& offTable)
{
    for (const auto offPos : offTable)
    {
        write_u32(offPos);
    }
}

void writer::write_sample_chunk_node(const sample_chunk::node& node)
{
    write_u32(node.flags);
    write_u32(node.prop.value);
    stream_->write(node.prop.name.data.data(), node.prop.name.data.size());
}

void writer::write_simple_header(const simple_header& header)
{
    write_u32(header.fileSize);
    write_u32(header.dataVersion);
    write_u32(header.dataSize);
    write_u32(header.dataPos);
    write_u32(header.offTablePos);
    write_u32(header.fileNamePos);
}

void writer::write_sample_chunk_header(const sample_chunk::header& header)
{
    write_u32(header.fileSize);
    write_u32(header.version);
    write_u32(header.offTablePos);
    write_u32(header.offCount);
}

namespace sample_chunk
{
    node node_deserializer::read_node_()
    {
        const auto node = reader_.read_sample_chunk_node();

        if (node.size() < 16)
        {
            throw std::runtime_error("Invalid Mirage sample chunk node size");
        }

        return node;
    }

    bool node_deserializer::try_read_first_child()
    {
        if (curNode_.is_leaf())
        {
            return false;
        }

        const auto nextNodePos = curNodePos_ + 16;
        reader_.stream().jump_to(nextNodePos);

        curNode_ = read_node_();
        curNodePos_ = nextNodePos;

        return true;
    }

    bool node_deserializer::try_read_next_sibling()
    {
        if (curNode_.is_last_child())
        {
            return false;
        }
        
        const auto nextNodePos = curNodePos_ + curNode_.size();
        reader_.stream().jump_to(nextNodePos);

        curNode_ = read_node_();
        curNodePos_ = nextNodePos;

        return true;
    }

    node_deserializer node_deserializer::get_child_deserializer()
    {
        auto childDr = *this;
        if (!childDr.try_read_first_child())
        {
            throw std::runtime_error(
                "Tried to deserialize children of leaf "
                "Mirage sample chunk node"
            );
        }
        
        return childDr;
    }

    node_deserializer::node_deserializer(rad::stream& stream)
        : reader_(stream)
        , curNodePos_(stream.tell())
        , curNode_(read_node_())
    {
    }
}

struct contexts_node_info_
{
    unsigned long long  dataPos = 0;
    u32                 dataVersion = 0;
    unsigned char       revision = 0;
};

static contexts_node_info_ read_contexts_sample_chunk_node_(
    reader& reader)
{
    contexts_node_info_ contextsNodeInfo;
    sample_chunk::node_deserializer scNodeDr(reader.stream());

    if (scNodeDr.try_read_first_child())
    {
        do
        {
            const auto& scNode = scNodeDr.current_node();

            if (scNode.prop.name == "Contexts")
            {
                if (!scNode.is_leaf())
                {
                    // TODO: Log warning or maybe error?
                }

                if (!scNode.is_last_child())
                {
                    // TODO: Log warning??
                }

                // TODO: Use C++20 delegated initializers.
                return contexts_node_info_{
                    reader.stream().tell(),
                    scNode.prop.value,
                    (scNode.size() == 16)
                };
            }
        }
        while (scNodeDr.try_read_next_sibling());
    }

    throw std::runtime_error(
        "No valid \"Contexts\" Mirage sample chunk node was found"
    );
}

off_type deserializer::determine_offset_type_(
    off_read_mode offsetReadMode,
    u32 offCount,
    u32 baseOff)
{
    if (offCount)
    {
        const auto firstOffPos = reader_.read_u32();
        reader_.stream().jump_to(headerPos_ + baseOff + firstOffPos);

        const auto firstOffTarget = reader_.read_u32();
        if (firstOffTarget == 0)
        {
            return off_type::u64;
        }
    }
    
    return off_type::u32;
}

deserializer::deserialize_info_ deserializer::read_deserialize_info_(
    off_read_mode offsetReadMode,
    rad::allocator& allocator)
{
    const auto fileSize = reader_.read_u32();

    if (fileSize & sample_chunk::marker)
    {
        // Read sample chunk header.
        // TODO: Use C++20 delegated initializers.
        const sample_chunk::header header = {
            fileSize & ~sample_chunk::marker,
            reader_.read_u32(),
            reader_.read_u32(),
            reader_.read_u32()
        };

        // Read sample chunk nodes and get info from the Contexts node.
        const auto contextsNodeInfo = read_contexts_sample_chunk_node_(reader_);

        // Determine offset type.
        const auto offType = (
            (offsetReadMode == off_read_mode::u32) ? off_type::u32 :
            (offsetReadMode == off_read_mode::u64) ? off_type::u64 :
            (reader_.stream().jump_to(headerPos_ + header.offTablePos),
            determine_offset_type_(offsetReadMode, header.offCount, 16))
        );

        // Jump to data position.
        reader_.stream().jump_to(contextsNodeInfo.dataPos);

        // TODO: Use C++20 delegated initializers.
        return deserialize_info_{
            16,
            header.offTablePos,
            header.offCount,
            // TODO: Use C++20 delegated initializers.
            mirage::file_info{
                ((contextsNodeInfo.revision == 1)
                    ? container_type::sample_chunk_r1
                    : container_type::sample_chunk_r0
                ),
                offType,
                contextsNodeInfo.dataVersion,
                rad::string{allocator}
            }
        };
    }
    else
    {
        // Read simple header.
        // TODO: Use C++20 delegated initializers.
        const simple_header header = {
            fileSize,
            reader_.read_u32(),
            reader_.read_u32(),
            reader_.read_u32(),
            reader_.read_u32(),
            reader_.read_u32()
        };

        const auto dataPos = headerPos_ + header.dataPos;

        // Read offset count.
        reader_.stream().jump_to(headerPos_ + header.offTablePos);
        const auto offCount = reader_.read_u32();

        // Determine offset type.
        const auto offType = (
            (offsetReadMode == off_read_mode::u32) ? off_type::u32 :
            (offsetReadMode == off_read_mode::u64) ? off_type::u64 :
            determine_offset_type_(offsetReadMode, offCount, header.dataPos)
        );

        // Jump to data position.
        reader_.stream().jump_to(dataPos);

        // TODO: Use C++20 delegated initializers.
        return deserialize_info_{
            header.dataPos,
            header.offTablePos + 4,
            offCount,
            // TODO: Use C++20 delegated initializers.
            mirage::file_info{
                container_type::simple,
                offType,
                header.dataVersion,
                reader_.read_string_at_offset(
                    headerPos_,
                    header.fileNamePos,
                    allocator
                )
            }
        };
    }
}

rad::vector<u32> deserializer::read_offset_table(
    rad::allocator& allocator)
{
    if (!info_.offCount)
    {
        return rad::vector<u32>{allocator};
    }
    else
    {
        const auto curPos = reader_.stream().tell();
        reader_.stream().jump_to(headerPos_ + info_.offTableOff);

        auto offsetPositions = reader_.read_offset_table(
            info_.offCount,
            allocator
        );

        reader_.stream().jump_to(curPos);

        return offsetPositions;
    }
}

u64 deserializer::read_offset()
{
    // TODO: Remove aligns from this function!!!
    if (info_.fileInfo.offsetType == off_type::u64)
    {
        align(8);
        return reader_.read_u64();
    }
    else
    {
        align(4);
        return reader_.read_u32();
    }
}

deserializer::deserializer(
    rad::stream& stream,
    off_read_mode offsetReadMode,
    rad::allocator& allocator)
    : reader_(stream)
    , headerPos_(stream.tell())
    , sampleChunkNodes_(allocator)
    , info_(read_deserialize_info_(offsetReadMode, allocator))
    , basePos_(headerPos_ + info_.baseOff)
{
}

static std::size_t get_header_size_(container_type containerType) noexcept
{
    return (containerType == container_type::simple) ? 0x18 : 0x10;
}

void serializer::start(
    mirage::file_info fileInfo,
    unsigned char flags)
{
    // TODO: Validate sequence is SEQ_NONE_

    offTable_.clear();
    nodeInfo_.clear();

    fileInfo_ = std::move(fileInfo);
    headerPos_ = writer_.stream().tell();

    const auto headerSize = get_header_size_(fileInfo_.containerType);

    dataEndPos_ = basePos_ = headerPos_ + headerSize;
    curNodeIndex_ = UINT32_MAX;
    offSize_ = (fileInfo_.offsetType == off_type::u64) ? 8 : 4;
    flags_ = flags;

    writer_.stream().write_nulls(headerSize);

    lastSeqStep_ = SEQ_HEADER_;
}

void serializer::start_sample_chunk_node(sample_chunk::property prop)
{
    // TODO: Validate container type is sample_chunk

    // TODO: Validate sequence is SEQ_HEADER_

    constexpr u32 maxNodeCount = sample_chunk::NODE_MASK_FLAGS / 16;

    const auto newNodePos = writer_.stream().tell();
    const auto newNodeIndex = nodeInfo_.size();

    if (newNodeIndex >= maxNodeCount)
    {
        throw std::runtime_error(
            "Mirage sample chunk node count exceeds maximum possible node count"
        );
    }

    writer_.write_sample_chunk_node(sample_chunk::node{ 0, prop });

    nodeInfo_.emplace_back(newNodePos, curNodeIndex_);

    // Update existing nodes.
    if (curNodeIndex_ != UINT32_MAX)
    {
        // If a child of the current node already exists, unset its
        // "last child" flag, because we are now the new last child.
        auto& curNodeInfo = nodeInfo_[curNodeIndex_];
        if (curNodeInfo.lastChildIndex)
        {
            auto& lastChildInfo = nodeInfo_[curNodeInfo.lastChildIndex];
            lastChildInfo.flags &= ~sample_chunk::NODE_FLAGS_IS_LAST_CHILD;
        }

        // Otherwise, we are the first child of the current node, so
        // unset the current node's "is leaf" flag.
        else
        {
            curNodeInfo.flags &= ~sample_chunk::NODE_FLAGS_IS_LEAF;
        }

        // Update the current node's last child index.
        curNodeInfo.lastChildIndex = static_cast<u32>(newNodeIndex);
    }

    // Update the current node index.
    curNodeIndex_ = static_cast<u32>(newNodeIndex);
}

void serializer::finish_sample_chunk_node()
{
    // TODO: Validate container type is sample_chunk

    // TODO: Validate sequence is SEQ_HEADER_

    assert(curNodeIndex_ != UINT32_MAX &&
        // HACK: Marker indicates the node is not finished
        (nodeInfo_[curNodeIndex_].flags & sample_chunk::marker) != 0 &&
        "finish_sample_chunk_node() MUST be called exactly once per "
        "preceding call to start_sample_chunk_node()"
    );

    // Compute and validate node size.
    auto& curNodeInfo = nodeInfo_[curNodeIndex_];
    const auto nodeEndPos = writer_.stream().tell();

    assert(nodeEndPos >= (curNodeInfo.nodePos + 16) &&
        "finish_sample_chunk_node() MUST be called with the "
        "stream position set to a value >= the node's position + 16"
    );

    const auto nodeSize = nodeEndPos - curNodeInfo.nodePos;

    if (nodeSize > sample_chunk::NODE_MASK_SIZE)
    {
        throw std::runtime_error(
            "Mirage sample chunk node size exceeds maximum possible range"
        );
    }

    // Set node size and unset marker.
    curNodeInfo.flags = (static_cast<u32>(nodeSize) |
        (curNodeInfo.flags & sample_chunk::NODE_MASK_FLAGS)
    );

    // Update the current node index.
    curNodeIndex_ = curNodeInfo.parentIndex;
}

void serializer::write_sample_chunk_leaf(sample_chunk::property prop)
{
    start_sample_chunk_node(prop);
    finish_sample_chunk_node();
}

void serializer::start_data_section()
{
    // TODO: Validate sequence is SEQ_HEADER_

    const auto dataPos = writer_.stream().tell();

    assert(dataPos >= basePos_ &&
        "start_data_section() MUST be called with the "
        "stream position set to a value >= the end of the header"
    );

    if (fileInfo_.containerType == container_type::simple)
    {
        if ((dataPos - basePos_) > UINT32_MAX)
        {
            throw std::runtime_error(
                "Mirage data section offset exceeds u32 range"
            );
        }

        basePos_ = dataPos;
    }
    else
    {
        // Validate node depth.
        assert(curNodeIndex_ != UINT32_MAX &&
            nodeInfo_[curNodeIndex_].parentIndex == UINT32_MAX &&
            "For sample chunk containers, start_data_section() MUST be called "
            "at node depth level 1"
        );

        // Start writing Contexts node.
        start_sample_chunk_node({ "Contexts", fileInfo_.dataVersion });

        // Finish writing now if necessary.
        if (fileInfo_.containerType == container_type::sample_chunk_r1)
        {
            finish_sample_chunk_node();
        }
    }

    lastSeqStep_ = SEQ_DATA_;
}

u32 serializer::tell_local() const
{
    // TODO: Validate sequence is SEQ_DATA_

    const auto pos = writer_.stream().tell();

    assert(pos >= basePos_ &&
        "Attempted to get local stream position from "
        "outside of the local data range"
    );

    return static_cast<u32>(pos - basePos_);
}

u32 serializer::start_offset()
{
    const auto localOffPos = tell_local();
    writer_.stream().write_nulls(offSize_);
    return localOffPos;
}

void serializer::finish_offset(u32 localOffPos, u64 offTarget)
{
    // TODO: Validate sequence is SEQ_DATA_

    // Resolve offset.
    const auto curPos = writer_.stream().tell();
    writer_.stream().jump_to(basePos_ + localOffPos);

    writer_.write_offset(offTarget, fileInfo_.offsetType);
    writer_.stream().jump_to(curPos);

    // Add offset position to offset table.
    offTable_.push_back(localOffPos);
}

u32 serializer::start_offsets(u32 offCount)
{
    // TODO: Validate sequence is SEQ_DATA_

    const auto localOffPos = tell_local();

    writer_.stream().write_nulls(
        static_cast<std::size_t>(offSize_) * offCount
    );

    return localOffPos;
}

void serializer::finish_data_section()
{
    // TODO: Validate sequence is SEQ_DATA_

    const auto dataEndPos = writer_.stream().tell();

    assert(dataEndPos >= basePos_ &&
        "finish_data_section() MUST be called with the "
        "stream position set to a value >= the start of the data"
    );
    
    if ((dataEndPos - basePos_) > ~sample_chunk::marker)
    {
        throw std::runtime_error(
            "Mirage data section size exceeds maximum possible range"
        );
    }

    if (fileInfo_.containerType == container_type::sample_chunk_r0)
    {
        finish_sample_chunk_node();
    }

    dataEndPos_ = dataEndPos;
    lastSeqStep_ = SEQ_DATA_END_;
}

void serializer::finish()
{
    // TODO: Validate sequence is not SEQ_NONE_

    if (lastSeqStep_ == SEQ_DATA_)
    {
        finish_data_section();
    }

    assert(writer_.stream().tell() >= dataEndPos_ &&
        "finish() MUST be called with the stream position "
        "set to a value >= the end of the data"
    );

    assert(curNodeIndex_ == UINT32_MAX &&
        "finish() must NOT be called until all "
        "sample chunk nodes have been finished"
    );
    
    if (offTable_.size() > UINT32_MAX)
    {
        throw std::runtime_error("Mirage offset count exceeds u32 range");
    }

    if (!(flags_ & SERIALIZER_FLAG_NO_SORT_OFF_TABLE))
    {
        std::sort(offTable_.begin(), offTable_.end());
    }

    if (fileInfo_.containerType == container_type::simple)
    {
        // Write offset table.
        pad(4); // TODO: We probably need to write this before the dataEndPos is set for an accurate data size!
        const auto offTablePos = writer_.stream().tell();

        writer_.write_u32(static_cast<u32>(offTable_.size()));
        writer_.write_offset_table(offTable_);

        // Write file name.
        u32 fileNameOff = 0;

        if (!fileInfo_.fileName.empty())
        {
            fileNameOff = (writer_.stream().tell() - headerPos_);
            writer_.write_string(fileInfo_.fileName);
        }

        pad(offSize_);

        // Resolve header.
        const auto fileEndPos = writer_.stream().tell();

        if ((fileEndPos - headerPos_) > ~sample_chunk::marker)
        {
            throw std::runtime_error(
                "Mirage file size exceeds maximum possible range"
            );
        }

        writer_.stream().jump_to(headerPos_);

        // TODO: Use C++20 delegated initializers.
        writer_.write_simple_header(simple_header{
            static_cast<u32>(fileEndPos - headerPos_),
            fileInfo_.dataVersion,
            static_cast<u32>(dataEndPos_ - basePos_),
            static_cast<u32>(basePos_ - headerPos_),
            static_cast<u32>(offTablePos - headerPos_),
            fileNameOff
        });

        writer_.stream().jump_to(fileEndPos);
    }
    else
    {
        // Finish all unfinished sample chunk nodes.
        while (curNodeIndex_ != UINT32_MAX)
        {
            finish_sample_chunk_node();
        }

        // Write offset table.
        pad(16);
        const auto offTablePos = writer_.stream().tell();

        writer_.write_offset_table(offTable_);

        // Resolve header.
        const auto fileEndPos = writer_.stream().tell();

        if ((fileEndPos - headerPos_) > ~sample_chunk::marker)
        {
            throw std::runtime_error(
                "Mirage file size exceeds maximum possible range"
            );
        }

        writer_.stream().jump_to(headerPos_);

        // TODO: Use C++20 delegated initializers.
        writer_.write_sample_chunk_header(sample_chunk::header{
            sample_chunk::marker | static_cast<u32>(fileEndPos - headerPos_),
            sample_chunk::v1,
            static_cast<u32>(offTablePos - headerPos_),
            static_cast<u32>(offTable_.size())
        });

        // Resolve sample chunk nodes.
        for (const auto& node : nodeInfo_)
        {
            // HACK: Marker indicates the node is not finished
            assert((node.flags & sample_chunk::marker) == 0 &&
                "Unfinished Mirage sample chunk node upon resolve; "
                "this should never happen!"
            );

            writer_.stream().jump_to(node.nodePos);
            writer_.write_u32(node.flags);
        }

        writer_.stream().jump_to(fileEndPos);
    }

    lastSeqStep_ = SEQ_NONE_;
}

serializer::serializer(
    rad::stream& stream,
    rad::allocator& allocator)
    : writer_(stream)
    , offTable_(allocator)
    , nodeInfo_(allocator)
{
}
}
