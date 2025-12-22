#ifndef INSPECTION__COMMON__GETTERS__HELPERS_H
#define INSPECTION__COMMON__GETTERS__HELPERS_H

#include <string>

namespace Inspection
{
    class Length;
    class ReadResult;
    class Value;
    
    auto AppendLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName = "length") -> Inspection::Value *;
    auto AppendReadErrorTag(Inspection::Value * Value, Inspection::ReadResult const & ReadResult) -> Inspection::Value *;
}

#endif
