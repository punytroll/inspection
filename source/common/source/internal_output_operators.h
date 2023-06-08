#ifndef INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H

#include <ostream>

#include "type_definition.h"
#include "type_definition/data_reference.h"

namespace Inspection
{
    namespace TypeDefinition
    {
        enum class DataType;
    }
    
    auto operator<<(std::ostream & OStream, Inspection::TypeDefinition::DataReference const & DataReference) -> std::ostream &;
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::DataReference::Root const & Root) -> std::ostream &;
    auto operator<<(std::ostream & OStream, Inspection::TypeDefinition::DataReference::Part const & Part) -> std::ostream &;
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::DataReference::Part::Type const & Type) -> std::ostream &;
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::DataType const & DataType) -> std::ostream &;
    auto operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::PartType const & PartType) -> std::ostream &;
    
    auto to_string(enum Inspection::TypeDefinition::DataType const & DataType) -> std::string;
    auto to_string(enum Inspection::TypeDefinition::PartType const & PartType) -> std::string;
    auto to_string(const std::type_info & TypeInformation) -> std::string;
}

#endif
