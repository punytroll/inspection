#ifndef COMON_ANY_PRINTING_H
#define COMON_ANY_PRINTING_H

#include <bitset>
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
	else if(Any.type() == typeid(std::uint16_t))
	{
		return OStream << std::experimental::any_cast< std::uint16_t >(Any);
	}
	else if(Any.type() == typeid(std::uint32_t))
	{
		return OStream << std::experimental::any_cast< std::uint32_t >(Any);
	}
	else if(Any.type() == typeid(std::uint64_t))
	{
		return OStream << std::experimental::any_cast< std::uint64_t >(Any);
	}
	else if(Any.type() == typeid(std::uint8_t))
	{
		return OStream << static_cast< std::uint32_t >(std::experimental::any_cast< std::uint8_t >(Any));
	}
	else if(Any.type() == typeid(std::bitset<32>))
	{
		return OStream << std::experimental::any_cast< std::bitset<32> >(Any);
	}
	else if(Any.type() == typeid(std::bitset<8>))
	{
		return OStream << std::experimental::any_cast< std::bitset<8> >(Any);
	}
	else
	{
		return OStream << "<Unknown any type: " << Any.type().name() << '>';
	}
}

#endif
