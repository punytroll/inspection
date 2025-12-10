#ifndef INSPECTION_COMMON_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_OUTPUT_OPERATORS_H

#include <format>
#include <ostream>

#include "length.h"

namespace std
{
    class any;
    class exception;
}

namespace Inspection
{
    class DateTime;
    class GUID;
    class Value;
    
    auto operator<<(std::ostream & OStream, std::any const & Any) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::DateTime const & DateTime) -> std::ostream &;
    auto operator<<(std::ostream & OStream, std::exception const & Exception) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::GUID const & GUID) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::Length const & Length) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::Value const & Value) -> std::ostream &;
}


template<class CharType>
struct std::formatter<Inspection::Length, CharType>
{
    constexpr auto parse(std::basic_format_parse_context<CharType> & FormatParseContext)
    {
        return FormatParseContext.begin();
    }

    template<class FormatContextType>
    auto format(Inspection::Length const & Length, FormatContextType & FormatContext) const
    {
        return std::format_to(FormatContext.out(), "{}.{}", Length.GetBytes(), Length.GetBits());
    }
};

#endif
