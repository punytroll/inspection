#ifndef COMON_ANY_PRINTING_H
#define COMON_ANY_PRINTING_H

#include <bitset>
#include <cstdint>
#include <experimental/any>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "guid.h"

inline std::ostream & operator<<(std::ostream & OStream, const std::experimental::any & Any)
{
	if(Any.type() == typeid(std::string))
	{
		return OStream << std::experimental::any_cast< std::string >(Any);
	}
	else if(Any.type() == typeid(bool))
	{
		std::stringstream StringStream;
		
		StringStream << std::boolalpha << std::experimental::any_cast< bool >(Any);
		
		return OStream << StringStream.str();
	}
	else if(Any.type() == typeid(std::int16_t))
	{
		return OStream << std::experimental::any_cast< std::int16_t >(Any);
	}
	else if(Any.type() == typeid(std::int32_t))
	{
		return OStream << std::experimental::any_cast< std::int32_t >(Any);
	}
	else if(Any.type() == typeid(std::int64_t))
	{
		return OStream << std::experimental::any_cast< std::int64_t >(Any);
	}
	else if(Any.type() == typeid(std::int8_t))
	{
		return OStream << static_cast< std::int32_t >(std::experimental::any_cast< std::int8_t >(Any));
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
	else if(Any.type() == typeid(std::bitset<16>))
	{
		return OStream << std::experimental::any_cast< std::bitset<16> >(Any);
	}
	else if(Any.type() == typeid(std::bitset<32>))
	{
		return OStream << std::experimental::any_cast< std::bitset<32> >(Any);
	}
	else if(Any.type() == typeid(std::bitset<8>))
	{
		return OStream << std::experimental::any_cast< std::bitset<8> >(Any);
	}
	else if(Any.type() == typeid(Inspection::GUID))
	{
		auto Value{std::experimental::any_cast< Inspection::GUID >(Any)};
		std::stringstream StringStream;
		
		StringStream << std::hex << std::setw(8) << std::right << std::setfill('0') << Value.Data1 << '-' << std::setw(4) << std::right << std::setfill('0') << Value.Data2 << '-' << std::setw(4) << std::right << std::setfill('0') << Value.Data3 << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[0]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[1]) << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[2]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[3]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[4]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[5]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[6]) << std::setw(2) << std::right << std::setfill('0') << static_cast< std::uint32_t >(Value.Data4[7]);
		
		return OStream << StringStream.str();
	}
	else if(Any.type() == typeid(std::vector< uint8_t >))
	{
		auto Value{std::experimental::any_cast< std::vector< uint8_t > >(Any)};
		auto Index{0ul};
		std::stringstream StringStream;
		
		StringStream << std::hex << std::setfill('0');
		while(Index < Value.size())
		{
			if(Index > 0)
			{
				StringStream << ' ';
			}
			StringStream << std::setw(2) << std::right << static_cast< std::uint32_t >(Value[Index]);
			Index += 1;
		}
		
		return OStream << StringStream.str();
	}
	else
	{
		return OStream << "<Unknown any type: " << Any.type().name() << '>';
	}
}

#endif
