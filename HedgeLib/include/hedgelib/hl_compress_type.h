#ifndef HL_COMPRESS_TYPE_H_INCLUDED
#define HL_COMPRESS_TYPE_H_INCLUDED

typedef enum HlCompressType
{
    /** @brief No compression. */
    HL_COMPRESS_TYPE_NONE = 0,
    /** @brief CAB-Compressed. Generally used on Windows. */
    HL_COMPRESS_TYPE_CAB,
    /** @brief X-Compressed. Generally used on Xbox 360. */
    HL_COMPRESS_TYPE_XCOMPRESS,
    /** @brief SEGS-Compressed. Generally used on PlayStation 3. */
    HL_COMPRESS_TYPE_SEGS
}
HlCompressType;

#endif
