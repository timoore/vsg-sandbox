#pragma once

#include "Config.h"
#include <algorithm>

namespace vsgsandbox
{

    constexpr bool isHostBigEndian()
    {
#if defined(VSGSANDBOX_BIGENDIAN)
        return true;
#else
        return false;
#endif
    }

    // Swap bytes in place
    inline void swapBytes( char* in, unsigned int size )
    {
        std::reverse(in, &in[size]);
    }

    // Swap bytes from a buffer and assign to parameter
    template<typename T>
    void swapBytes(T& t, const void* buf)
    {
        const char* bufAsChar = static_cast<const char*>(buf);
        char charBuf[sizeof(T)];
        std::reverse_copy(bufAsChar, bufAsChar + sizeof(T), &charBuf[0]);
        memcpy(&t, &charBuf[0], sizeof(T));
    }
    
}
