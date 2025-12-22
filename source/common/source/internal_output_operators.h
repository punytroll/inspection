#ifndef INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H

#include <ostream>
#include <vector>

namespace Inspection
{
    namespace TypeDefinition
    {
        enum class DataType;
        enum class PartType;
    }
    
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::DataType const & DataType) -> std::ostream &;
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::PartType const & PartType) -> std::ostream &;
    
    auto to_string(enum Inspection::TypeDefinition::DataType const & DataType) -> std::string;
    auto to_string(enum Inspection::TypeDefinition::PartType const & PartType) -> std::string;
    auto to_string(const std::type_info & TypeInformation) -> std::string;
}

template<class CharType>
struct std::formatter<std::vector<std::uint8_t>, CharType>
{
    constexpr auto parse(std::basic_format_parse_context<CharType> & FormatParseContext)
    {
        return FormatParseContext.begin();
    }

    template<class FormatContextType>
    auto format(std::vector<std::uint8_t> const & UInt8Buffer, FormatContextType & FormatContext) const
    {
        auto Output = FormatContext.out();

        for(auto Index = 0UZ; Index < UInt8Buffer.size(); ++Index)
        {
            if(Index > 0)
            {
                *Output++ = ' ';
            }
            Output = std::format_to(Output, "{:02x}", static_cast<std::uint32_t>(UInt8Buffer[Index]));
        }

        return Output;
    }
};

#endif
