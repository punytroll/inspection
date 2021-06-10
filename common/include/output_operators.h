#ifndef INSPECTION_COMMON_OUTPUT_OPERATORS_H
#define INSPECTION_COMMON_OUTPUT_OPERATORS_H

#include <any>
#include <ostream>

namespace Inspection
{
	class DateTime;
	class GUID;
	class Length;
	class Value;
	
	std::ostream & operator<<(std::ostream & OStream, const std::any & Any);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::DateTime & DateTime);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::GUID & GUID);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::Length & Length);
	std::ostream & operator<<(std::ostream & OStream, const Inspection::Value & Value);
}

#endif
