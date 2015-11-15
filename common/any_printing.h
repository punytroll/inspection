#ifndef COMON_ANY_PRINTING_H
#define COMON_ANY_PRINTING_H

#include <cstdint>
#include <experimental/any>
#include <ostream>
#include <string>

inline std::ostream & operator<<(std::ostream & OStream, const std::experimental::any & Any)
{
	if(Any.type() == typeid(std::string))
	{
		return OStream << std::experimental::any_cast< std::string >(Any);
	}
	else if(Any.type() == typeid(std::uint32_t))
	{
		return OStream << std::experimental::any_cast< std::uint32_t >(Any);
	}
	else
	{
		return OStream << '<' << Any.type().name() << '>';
	}
}

#endif
