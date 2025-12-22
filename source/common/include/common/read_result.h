#ifndef INSPECTION_COMMON_READ_RESULT_H
#define INSPECTION_COMMON_READ_RESULT_H

#include <cstdint>

#include <common/length.h>

namespace Inspection
{
    struct ReadResult
    {
        std::uint8_t Data;
        Inspection::Length InputLength;
        Inspection::Length OutputLength;
        Inspection::Length RequestedLength;
        bool Success;
    };
}

#endif
