#ifndef INSPECTION_COMMON_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_OUTPUT_OPERATORS_H

#include <any>
#include <ostream>

namespace std
{
    class exception;
}

namespace Inspection
{
    class DateTime;
    class GUID;
    class Length;
    class Value;
    
    auto operator<<(std::ostream & OStream, std::any const & Any) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::DateTime const & DateTime) -> std::ostream &;
    auto operator<<(std::ostream & OStream, std::exception const & Exception) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::GUID const & GUID) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::Length const & Length) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::Value const & Value) -> std::ostream &;
}

#endif
