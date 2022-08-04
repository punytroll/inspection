#ifndef INSPECTION_COMMON_DATE_TIME_H
#define INSPECTION_COMMON_DATE_TIME_H

namespace Inspection
{
    class DateTime
    {
    public:
        std::uint32_t Year;
        std::uint8_t Month;
        std::uint8_t Day;
        std::uint8_t Hour;
        std::uint8_t Minute;
        std::uint8_t Second;
        std::uint32_t Microsecond;
    };
}

#endif
