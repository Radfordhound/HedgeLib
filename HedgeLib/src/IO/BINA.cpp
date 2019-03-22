#include "IO/BINA.h"
#include "IO/File.h"
#include "Offsets.h"
#include <cstdint>
#include <memory>

namespace HedgeLib::IO::BINA
{
    void WriteOffsetsSorted(const File& file, const OffsetTable& offsets) noexcept
    {
        std::uint32_t o, curOffset = 0;
        for (auto& offset : offsets)
        {
            o = ((offset - curOffset) >> 2);
            if (o > 0x3FFF)
            {
                o <<= 24;
                o |= ThirtyBit;
                file.Write(&o, sizeof(std::uint32_t), 1);
            }
            else if (o > 0x3F)
            {
                o <<= 8;
                o |= FourteenBit;
                file.Write(&o, sizeof(std::uint16_t), 1);
            }
            else
            {
                o |= SixBit;
                file.Write(&o, sizeof(std::uint8_t), 1);
            }

            curOffset = offset;
        }

        file.Pad();
    }
}
