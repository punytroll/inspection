#ifndef INSPECTION__COMMON__FROM_STRING_H
#define INSPECTION__COMMON__FROM_STRING_H

#include <charconv>
#include <optional>

namespace Inspection
{
    template<typename Type>
    concept Integer = (std::integral<Type> == true) && (std::same_as<Type, bool> == false);
    
    template<typename Type>
    concept Boolean = (std::same_as<Type, bool> == true);
    
    template<Integer IntegerType>
    auto FromString(std::string_view String) -> std::optional<IntegerType>
    {
        auto Result = IntegerType{};
        auto const * Begin = String.data();
        auto const * End = Begin + String.size();
        auto [Pointer, ErrorCode] = std::from_chars(Begin, End, Result, 10);
        
        if((ErrorCode == std::errc{}) && (Pointer == End))
        {
            return Result;
        }
        else
        {
            return std::nullopt;
        }
    }
    
    template<Boolean BooleanType>
    auto FromString(std::string_view String) -> std::optional<BooleanType>
    {
        if(String == "true")
        {
            return true;
        }
        else if(String == "false")
        {
            return false;
        }
        else
        {
            return std::nullopt;
        }
    }
    
    template<std::floating_point FloatingPointType>
    auto FromString(std::string_view String) -> std::optional<FloatingPointType>
    {
        auto Result = FloatingPointType{};
        auto const * Begin = String.data();
        auto const * End = Begin + String.size();
        auto [Pointer, ErrorCode] = std::from_chars(Begin, End, Result, std::chars_format::general);
        
        if((ErrorCode == std::errc{}) && (Pointer == End))
        {
            return Result;
        }
        else
        {
            return std::nullopt;
        }
    }
}

#endif
