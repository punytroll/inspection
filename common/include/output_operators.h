#ifndef INSPECTION_COMMON_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_OUTPUT_OPERATORS_H

#include <any>
#include <ostream>

namespace Inspection
{
	class Length;
	
	std::ostream & operator<<(std::ostream & OStream, const std::any & Any);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::Length & Length);
}

#endif
