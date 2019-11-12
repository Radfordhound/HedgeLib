#pragma once
#include <cstdint>

namespace hl
{
    struct PACxV2DataNode;

    const std::uint8_t* INDPACxGetOffsetTableV2(
        const PACxV2DataNode& dataNode, std::uint32_t& offTableSize);
}
