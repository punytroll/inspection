#include <any>
#include <bitset>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "date_time.h"
#include "guid.h"
#include "length.h"
#include "output_operators.h"

template < >
inline std::string to_string_cast<Inspection::Length>(const Inspection::Length & Value)
{
	return static_cast<std::ostringstream &>(std::ostringstream{} << Value).str();
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const std::any & Any)
{
	if(Any.has_value() == false)
	{
		// print nothing
		// an empty any indicates a place where, intentionally, there is no data stored in the value hierarchy
	}
	else
	{
		auto Flags = OStream.flags();
		
		if(Any.type() == typeid(char))
		{
			OStream << std::any_cast<char>(Any);
		}
		else if(Any.type() == typeid(std::string))
		{
			OStream << std::any_cast<std::string>(Any);
		}
		else if(Any.type() == typeid(bool))
		{
			OStream << std::boolalpha << std::any_cast<bool>(Any);
		}
		else if(Any.type() == typeid(float))
		{
			OStream << std::any_cast<float>(Any);
		}
		else if(Any.type() == typeid(std::int16_t))
		{
			OStream << std::any_cast<std::int16_t>(Any);
		}
		else if(Any.type() == typeid(std::int32_t))
		{
			OStream << std::any_cast<std::int32_t>(Any);
		}
		else if(Any.type() == typeid(std::int64_t))
		{
			OStream << std::any_cast<std::int64_t>(Any);
		}
		else if(Any.type() == typeid(std::int8_t))
		{
			OStream << static_cast<std::int32_t>(std::any_cast<std::int8_t>(Any));
		}
		else if(Any.type() == typeid(std::uint16_t))
		{
			OStream << std::any_cast<std::uint16_t>(Any);
		}
		else if(Any.type() == typeid(std::uint32_t))
		{
			OStream << std::any_cast< std::uint32_t >(Any);
		}
		else if(Any.type() == typeid(std::uint64_t))
		{
			OStream << std::any_cast<std::uint64_t>(Any);
		}
		else if(Any.type() == typeid(std::uint8_t))
		{
			OStream << static_cast<std::uint32_t>(std::any_cast<std::uint8_t>(Any));
		}
		else if(Any.type() == typeid(std::bitset<4>))
		{
			OStream << std::any_cast<const std::bitset<4> &>(Any);
		}
		else if(Any.type() == typeid(std::bitset<8>))
		{
			OStream << std::any_cast<const std::bitset<8> &>(Any);
		}
		else if(Any.type() == typeid(std::bitset<16>))
		{
			OStream << std::any_cast<const std::bitset<16> &>(Any);
		}
		else if(Any.type() == typeid(std::bitset<32>))
		{
			OStream << std::any_cast<const std::bitset<32> &>(Any);
		}
		else if(Any.type() == typeid(Inspection::GUID))
		{
			auto Value = std::any_cast<const Inspection::GUID &>(Any);
			
			OStream << std::hex << std::setw(8) << std::right << std::setfill('0') << Value.Data1 << '-' << std::setw(4) << std::right << std::setfill('0') << Value.Data2 << '-' << std::setw(4) << std::right << std::setfill('0') << Value.Data3 << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[0]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[1]) << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[2]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[3]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[4]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[5]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[6]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Data4[7]);
		}
		else if(Any.type() == typeid(Inspection::DateTime))
		{
			auto Value = std::any_cast<const Inspection::DateTime &>(Any);
			
			OStream << Value.Year << '/' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(Value.Month) << '/' << std::setw(2) << static_cast<std::uint32_t>(Value.Day) << ' ' << std::setw(2) << static_cast<std::uint32_t>(Value.Hour) << ':' << std::setw(2) << static_cast<std::uint32_t>(Value.Minute) << ':' << std::setw(2) << static_cast<std::uint32_t>(Value.Second);
		}
		else if(Any.type() == typeid(std::vector<std::uint8_t>))
		{
			auto Value = std::any_cast<const std::vector<std::uint8_t> &>(Any);
			auto First = true;
			
			OStream << std::hex << std::setfill('0');
			for(auto Element : Value)
			{
				if(First == false)
				{
					OStream << ' ';
				}
				else
				{
					First = false;
				}
				OStream << std::setw(2) << std::right << static_cast<std::uint32_t>(Element);
			}
			
		}
		else if(Any.type() == typeid(Inspection::Length))
		{
			OStream << to_string_cast(std::any_cast<const Inspection::Length &>(Any));
		}
		else if(Any.type() == typeid(nullptr))
		{
			// print "nothing"
			// an any containing a nullptr indicates that a value is expected but no value could be read or interpreted
			OStream << "nothing";
		}
		else
		{
			OStream << "<unknown type \"" << Any.type().name() << "\">";
		}
		OStream.flags(Flags);
	}
	
	return OStream;
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::Length & Length)
{
	return OStream << Length.GetBytes() << '.' << Length.GetBits();
}
