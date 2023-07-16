#ifndef INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H

#include <ostream>

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

#endif
