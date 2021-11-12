#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>

static struct SUBRANGE {
    uint32_t LowCount, HighCount, scale;
} SubRange;

const uint32_t CODE_VALUE_BITS = 32, TOP_VALUE = 1 << 31, BOTTOM_VALUE = TOP_VALUE >> 8;
const int SHIFT_BITS = CODE_VALUE_BITS-9, EXTRA_BITS = (CODE_VALUE_BITS - 2) % 8+1, CTRL_BYTE = 0x79;
static uint32_t low, range, AriVar, StreamStart;
static unsigned char ByteBuffer;

inline void ariInitEncoder(FILE* stream) {
    StreamStart = ftell(stream);
    AriVar = low = 0;
    range = TOP_VALUE;
    ByteBuffer = CTRL_BYTE;
}

inline void ariEncoderNormalize(FILE* stream) {
    while (range <= BOTTOM_VALUE) {
        if (low < (0xFF << SHIFT_BITS)) {
            for (putc(ByteBuffer, stream); AriVar != 0; AriVar--)
                putc(0xFF,stream);
            ByteBuffer = (unsigned char)(low >> SHIFT_BITS);
        } else if ((low & TOP_VALUE) != 0) {
            for (putc(ByteBuffer + 1, stream); AriVar != 0; AriVar--)
                putc(0, stream);
            ByteBuffer = (unsigned char)(low >> SHIFT_BITS);
        } else
            AriVar++;
        range <<= 8;
        low = (low << 8) & (TOP_VALUE-1);
    }
}

inline void ariEncodeSymbol() {
    uint32_t r = range / SubRange.scale;
    uint32_t tmp = r * SubRange.LowCount;
    if (SubRange.HighCount < SubRange.scale)
        range = r * (SubRange.HighCount - SubRange.LowCount);
    else
        range -= tmp;
    low += tmp;
}

inline void ariShiftEncodeSymbol(int SHIFT) {
    uint32_t r = range >> SHIFT;
    uint32_t tmp = r * SubRange.LowCount;
    if (SubRange.HighCount < (1 << SHIFT))
        range = r * (SubRange.HighCount - SubRange.LowCount);
    else
        range -= tmp;
    low += tmp;
}

inline void ariFlushEncoder(FILE* stream) {
    unsigned int ByteCount = ftell(stream) - StreamStart + 5;
    unsigned int tmp = (low >> SHIFT_BITS) +
            ((low & (BOTTOM_VALUE - 1)) >= (ByteCount >> 1));
    for (putc(ByteBuffer + (tmp > 0xFF), stream); AriVar != 0; AriVar--)
        putc(0 - (tmp > 0xFF), stream);
    putc(tmp,stream);
    putc(ByteCount >> 16, stream);
    putc(ByteCount >>  8, stream);
    putc(ByteCount >>  0, stream);
}

inline void ariInitDecoder(FILE* stream) {
    if (getc(stream) != CTRL_BYTE)
        exit(-1);
    ByteBuffer = getc(stream);
    low = ByteBuffer >> (8 - EXTRA_BITS);
    range = 1 << EXTRA_BITS;
}

inline void ariDecoderNormalize(FILE* stream) {
    while (range <= BOTTOM_VALUE) {
        low = (low << 8) | ((ByteBuffer << EXTRA_BITS) & 0xFF);
        low |= (ByteBuffer = getc(stream)) >> (8 - EXTRA_BITS);
        range <<= 8;
    }
}

inline int ariGetCurrentCount() {
    AriVar = range / SubRange.scale;
    uint32_t tmp = low / AriVar;
    return (tmp >= SubRange.scale) ? (SubRange.scale - 1) : tmp;
}

inline int ariGetCurrentShiftCount(int SHIFT) {
    SubRange.scale = 1 << SHIFT;
    AriVar = range >> SHIFT;
    uint32_t tmp = low / AriVar;
    return (tmp >= (1 << SHIFT)) ? ((1 << SHIFT) - 1) : tmp;
}

inline void ariRemoveSubrange() {
    uint32_t tmp = AriVar * SubRange.LowCount;
    low -= tmp;
    if (SubRange.HighCount < SubRange.scale)
        range = AriVar * (SubRange.HighCount - SubRange.LowCount);
    else
        range -= tmp;
}
