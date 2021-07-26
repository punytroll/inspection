#include <any>
#include <bitset>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include "colors.h"
#include "date_time.h"
#include "guid.h"
#include "internal_output_operators.h"
#include "length.h"
#include "output_operators.h"
#include "value.h"

std::ostream & _PrintValue(std::ostream & OStream, const Inspection::Value & Value, const std::string & Indentation)
{
	auto HeaderLine = (Value.GetName().empty() == false) || (Value.GetData().has_value() == true) || (Value.GetTags().empty() == false);
	
	if(HeaderLine == true)
	{
		OStream << Inspection::g_White << Indentation;
	}
	if(Value.GetName().empty() == false)
	{
		if(Value.GetName() == "error")
		{
			OStream << Inspection::g_BrightRed;
		}
		else
		{
			OStream << Inspection::g_BrightWhite;
		}
		OStream << Value.GetName();
	}
	if((Value.GetName().empty() == false) && (Value.GetData().has_value() == true))
	{
		OStream << Inspection::g_White << ": ";
	}
	if(Value.GetData().has_value() == true)
	{
		if((Value.GetName().empty() == false) && (Value.GetName() == "error"))
		{
			OStream << Inspection::g_BrightWhite;
		}
		else
		{
			OStream << Inspection::g_BrightCyan;
		}
		Inspection::operator<<(OStream, Value.GetData());
	}
	if(Value.GetTags().empty() == false)
	{
		auto First = true;
		
		OStream << Inspection::g_White <<" {" << Inspection::g_BrightBlack;
		for(auto & Tag : Value.GetTags())
		{
			if(First == false)
			{
				OStream << Inspection::g_White << ", " << Inspection::g_BrightBlack;
			}
			if(Tag->GetName().empty() == false)
			{
				if(Tag->GetName() == "error")
				{
					OStream << Inspection::g_BrightRed;
				}
				else if(Tag->GetData().has_value() == false)
				{
					OStream << Inspection::g_BrightBlack;
				}
				else
				{
					OStream << Inspection::g_Yellow;
				}
				OStream << Tag->GetName();
			}
			if((Tag->GetName().empty() == false) && (Tag->GetData().has_value() == true))
			{
				OStream << Inspection::g_White << '=';
			}
			if(Tag->GetData().has_value() == true)
			{
				if(Tag->GetData().type() == typeid(nullptr))
				{
					OStream << Inspection::g_Green;
				}
				else
				{
					OStream << Inspection::g_BrightBlack;
				}
				Inspection::operator<<(OStream, Tag->GetData());
			}
			if(Tag->GetFields().size() > 0)
			{
				throw std::exception();
			}
			if(Tag->GetTags().size() > 0)
			{
				OStream << Inspection::g_White << " {" << Inspection::g_BrightBlack;
				
				auto FirstSubTag = true;
				
				for(auto & SubTag : Tag->GetTags())
				{
					if(FirstSubTag == false)
					{
						OStream << ", ";
					}
					if(SubTag->GetName().empty() == false)
					{
						if(SubTag->GetName() == "error")
						{
							OStream << Inspection::g_BrightRed;
						}
						OStream << SubTag->GetName();
					}
					OStream << Inspection::g_BrightBlack;
					if((SubTag->GetName().empty() == false) && (SubTag->GetData().has_value() == true))
					{
						OStream << '=';
					}
					if(SubTag->GetData().has_value() == true)
					{
						Inspection::operator<<(OStream, SubTag->GetData());
					}
					if(SubTag->GetFields().size() > 0)
					{
						throw std::exception();
					}
					if(SubTag->GetTags().size() > 0)
					{
						OStream << Inspection::g_White << " {" << Inspection::g_BrightBlack;
						
						auto FirstSubSubTag = true;
						
						for(auto & SubSubTag : SubTag->GetTags())
						{
							if(FirstSubSubTag == false)
							{
								OStream << ", ";
							}
							if(SubSubTag->GetName().empty() == false)
							{
								if(SubSubTag->GetName() == "error")
								{
									OStream << Inspection::g_BrightRed;
								}
								OStream << SubSubTag->GetName();
							}
							OStream << Inspection::g_BrightBlack;
							if((SubSubTag->GetName().empty() == false) && (SubSubTag->GetData().has_value() == true))
							{
								OStream << '=';
							}
							if(SubSubTag->GetData().has_value() == true)
							{
								Inspection::operator<<(OStream, SubSubTag->GetData());
							}
							if(SubSubTag->GetFields().size() > 0)
							{
								throw std::exception();
							}
							if(SubSubTag->GetTags().size() > 0)
							{
								throw std::exception();
							}
							FirstSubSubTag = false;
						}
						OStream << Inspection::g_White << '}' << Inspection::g_BrightBlack;
					}
					FirstSubTag = false;
				}
				OStream << Inspection::g_White << '}' << Inspection::g_BrightBlack;
			}
			First = false;
		}
		OStream << Inspection::g_White << '}';
	}
	
	auto SubIndentation = Indentation;
	
	if(HeaderLine == true)
	{
		OStream << '\n';
		SubIndentation += "    ";
	}
	if(Value.GetFieldCount() > 0)
	{
		for(auto & SubValue : Value.GetFields())
		{
			_PrintValue(OStream, *SubValue, SubIndentation);
		}
	}
	OStream << Inspection::g_Reset;
	
	return OStream;
}

template < >
std::string to_string_cast<Inspection::Length>(const Inspection::Length & Value)
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
			OStream << std::any_cast<const Inspection::GUID &>(Any);
		}
		else if(Any.type() == typeid(Inspection::DateTime))
		{
			OStream << std::any_cast<const Inspection::DateTime &>(Any);
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
			OStream << std::any_cast<const Inspection::Length &>(Any);
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

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::DateTime & DateTime)
{
	return OStream << DateTime.Year << '/' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(DateTime.Month) << '/' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Day) << ' ' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Hour) << ':' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Minute) << ':' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Second);
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::GUID & GUID)
{
	return OStream << std::hex << std::setw(8) << std::right << std::setfill('0') << GUID.Data1 << '-' << std::setw(4) << std::right << std::setfill('0') << GUID.Data2 << '-' << std::setw(4) << std::right << std::setfill('0') << GUID.Data3 << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[0]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[1]) << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[2]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[3]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[4]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[5]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[6]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[7]);
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::Length & Length)
{
	return OStream << Length.GetBytes() << '.' << Length.GetBits();
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::Value & Value)
{
	return _PrintValue(OStream, Value, "");
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference & DataReference)
{
	OStream << "DataReference[" << DataReference.GetRoot() << ", {";
	
	auto First = true;
	
	for(auto & Part : DataReference.Parts)
	{
		if(First == false)
		{
			OStream << ", ";
		}
		else
		{
			First = false;
		}
		OStream << Part;
	}
	
	return OStream << "}]";
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataReference::Root & Root)
{
	switch(Root)
	{
	case Inspection::TypeDefinition::DataReference::Root::Type:
		{
			return OStream << "Type";
		}
	case Inspection::TypeDefinition::DataReference::Root::Current:
		{
			return OStream << "Current";
		}
	default:
		{
			assert(false);
		}
	}
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference::Part & Part)
{
	return OStream << "Part[" << Part.GetType() << ", \"" << Part.DetailName << "\"]";
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataReference::Part::Type & Type)
{
	switch(Type)
	{
	case Inspection::TypeDefinition::DataReference::Part::Type::Field:
		{
			return OStream << "Field";
		}
	case Inspection::TypeDefinition::DataReference::Part::Type::Tag:
		{
			return OStream << "Tag";
		}
	default:
		{
			assert(false);
		}
	}
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::DataType & DataType)
{
	switch(DataType)
	{
	case Inspection::TypeDefinition::DataType::Unknown:
		{
			return OStream << "<unknown>";
		}
	case Inspection::TypeDefinition::DataType::Boolean:
		{
			return OStream << "boolean";
		}
	case Inspection::TypeDefinition::DataType::GUID:
		{
			return OStream << "guid";
		}
	case Inspection::TypeDefinition::DataType::Length:
		{
			return OStream << "length";
		}
	case Inspection::TypeDefinition::DataType::Nothing:
		{
			return OStream << "nothing";
		}
	case Inspection::TypeDefinition::DataType::Parameters:
		{
			return OStream << "parameters";
		}
	case Inspection::TypeDefinition::DataType::SinglePrecisionReal:
		{
			return OStream << "single-precision-real";
		}
	case Inspection::TypeDefinition::DataType::String:
		{
			return OStream << "string";
		}
	case Inspection::TypeDefinition::DataType::Type:
		{
			return OStream << "type";
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger8Bit:
		{
			return OStream << "usigned-integer-8bit";
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
		{
			return OStream << "usigned-integer-16bit";
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
		{
			return OStream << "usigned-integer-32bit";
		}
	case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
		{
			return OStream << "usigned-integer-64bit";
		}
	}
	assert(false);
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const enum Inspection::TypeDefinition::ExpressionType & ExpressionType)
{
	switch(ExpressionType)
	{
	case Inspection::TypeDefinition::ExpressionType::Add:
		{
			return OStream << "Add";
		}
	case Inspection::TypeDefinition::ExpressionType::Cast:
		{
			return OStream << "Cast";
		}
	case Inspection::TypeDefinition::ExpressionType::DataReference:
		{
			return OStream << "DataReference";
		}
	case Inspection::TypeDefinition::ExpressionType::Divide:
		{
			return OStream << "Divide";
		}
	case Inspection::TypeDefinition::ExpressionType::Equals:
		{
			return OStream << "Equals";
		}
	case Inspection::TypeDefinition::ExpressionType::FieldReference:
		{
			return OStream << "FieldReference";
		}
	case Inspection::TypeDefinition::ExpressionType::Length:
		{
			return OStream << "Length";
		}
	case Inspection::TypeDefinition::ExpressionType::LengthReference:
		{
			return OStream << "LengthReference";
		}
	case Inspection::TypeDefinition::ExpressionType::ParameterReference:
		{
			return OStream << "ParameterReference";
		}
	case Inspection::TypeDefinition::ExpressionType::Parameters:
		{
			return OStream << "Parameters";
		}
	case Inspection::TypeDefinition::ExpressionType::Subtract:
		{
			return OStream << "Subtract";
		}
	case Inspection::TypeDefinition::ExpressionType::TypeReference:
		{
			return OStream << "TypeReference";
		}
	case Inspection::TypeDefinition::ExpressionType::Value:
		{
			return OStream << "Value";
		}
	}
	assert(false);
}
