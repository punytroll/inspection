#ifndef INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_INTERNAL_OUTPUT_OPERATORS_H

#include <ostream>

#include "type_definition.h"

namespace Inspection
{
	std::ostream & operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference & DataReference);
	std::ostream & operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataReference::Root & Root);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference::Part & Part);
	std::ostream & operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataReference::Part::Type & Type);
	std::ostream & operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataType & DataType);
	std::ostream & operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::ExpressionType & ExpressionType);
}

#endif
