#include <any>
#include <bitset>
#include <iomanip>
#include <ostream>
#include <string>
#include <vector>

#include <string_cast/string_cast.h>

#include "assertion.h"
#include "colors.h"
#include "date_time.h"
#include "guid.h"
#include "internal_output_operators.h"
#include "length.h"
#include "output_operators.h"
#include "type_definition/data_type.h"
#include "value.h"

using namespace std::string_literals;

static auto m_PrintTags(std::ostream & OStream, std::list<std::unique_ptr<Inspection::Value>> const & Tags) -> void
{
    auto First = true;
    
    OStream << Inspection::g_White <<" {" << Inspection::g_BrightBlack;
    for(auto const & Tag : Tags)
    {
        if(First == false)
        {
            OStream << Inspection::g_White << ", " << Inspection::g_BrightBlack;
        }
        else
        {
            First = false;
        }
        if(Tag->GetName().empty() == false)
        {
            if(Tag->GetName() == "error")
            {
                OStream << Inspection::g_BrightRed;
            }
            OStream << Tag->GetName();
            if(Tag->GetName() == "error")
            {
                OStream << Inspection::g_BrightBlack;
            }
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
                OStream << Inspection::g_Yellow;
            }
            Inspection::operator<<(OStream, Tag->GetData());
        }
        if(Tag->GetFields().size() > 0)
        {
            throw std::exception();
        }
        if(Tag->GetTags().size() > 0)
        {
            m_PrintTags(OStream, Tag->GetTags());
        }
    }
    OStream << Inspection::g_White << '}';
}

static auto m_PrintValue(std::ostream & OStream, Inspection::Value const & Value, std::string const & Indentation) -> std::ostream &
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
        m_PrintTags(OStream, Value.GetTags());
    }
    
    auto SubIndentation = Indentation;
    
    if(HeaderLine == true)
    {
        OStream << '\n';
        SubIndentation += "    ";
    }
    if(Value.GetFieldCount() > 0)
    {
        for(auto const & SubValue : Value.GetFields())
        {
            m_PrintValue(OStream, *SubValue, SubIndentation);
        }
    }
    OStream << Inspection::g_Reset;
    
    return OStream;
}

template < >
std::string to_string_cast<Inspection::Length>(const Inspection::Length & Value)
{
    auto OStream = std::ostringstream{};
    
    OStream << Value;
    
    return OStream.str();
}

auto Inspection::operator<<(std::ostream & OStream, std::any const & Any) -> std::ostream &
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
            OStream << std::any_cast<std::string const &>(Any);
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
            OStream << std::any_cast<std::bitset<4> const &>(Any);
        }
        else if(Any.type() == typeid(std::bitset<8>))
        {
            OStream << std::any_cast<std::bitset<8> const &>(Any);
        }
        else if(Any.type() == typeid(std::bitset<16>))
        {
            OStream << std::any_cast<std::bitset<16> const &>(Any);
        }
        else if(Any.type() == typeid(std::bitset<32>))
        {
            OStream << std::any_cast<std::bitset<32> const &>(Any);
        }
        else if(Any.type() == typeid(Inspection::GUID))
        {
            OStream << std::any_cast<Inspection::GUID const &>(Any);
        }
        else if(Any.type() == typeid(Inspection::DateTime))
        {
            OStream << std::any_cast<Inspection::DateTime const &>(Any);
        }
        else if(Any.type() == typeid(std::vector<std::uint8_t>))
        {
            auto Value = std::any_cast<std::vector<std::uint8_t> const &>(Any);
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
            OStream << std::any_cast<Inspection::Length const &>(Any);
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

auto Inspection::operator<<(std::ostream & OStream, Inspection::DateTime const & DateTime) -> std::ostream &
{
    return OStream << DateTime.Year << '/' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(DateTime.Month) << '/' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Day) << ' ' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Hour) << ':' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Minute) << ':' << std::setw(2) << static_cast<std::uint32_t>(DateTime.Second);
}

static auto m_PrintException(std::ostream & OStream, std::exception const & Exception) -> void
{
    OStream << Inspection::g_Yellow << Exception.what() << Inspection::g_Reset << '\n';
    try
    {
        std::rethrow_if_nested(Exception);
    }
    catch(std::exception const & NestedException)
    {
        OStream << Inspection::g_Red << "Nested exception" << Inspection::g_Reset << ":" << '\n';
        m_PrintException(OStream, NestedException);
    }
}

auto Inspection::operator<<(std::ostream & OStream, std::exception const & Exception) -> std::ostream &
{
    OStream << Inspection::g_Red << "Caught an exception while processing" << Inspection::g_Reset << ":" << '\n';
    m_PrintException(OStream, Exception);
    
    return OStream;
}

auto Inspection::operator<<(std::ostream & OStream, Inspection::GUID const & GUID) -> std::ostream &
{
    return OStream << std::hex << std::setw(8) << std::right << std::setfill('0') << GUID.Data1 << '-' << std::setw(4) << std::right << std::setfill('0') << GUID.Data2 << '-' << std::setw(4) << std::right << std::setfill('0') << GUID.Data3 << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[0]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[1]) << '-' << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[2]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[3]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[4]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[5]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[6]) << std::setw(2) << std::right << std::setfill('0') << static_cast<std::uint32_t>(GUID.Data4[7]);
}

auto Inspection::operator<<(std::ostream & OStream, Inspection::Length const & Length) -> std::ostream &
{
    return OStream << Length.GetBytes() << '.' << Length.GetBits();
}

auto Inspection::operator<<(std::ostream & OStream, Inspection::Value const & Value) -> std::ostream &
{
    return m_PrintValue(OStream, Value, "");
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference & DataReference)
{
    OStream << "DataReference[" << DataReference.GetRoot() << ", {";
    
    auto First = true;
    
    for(auto & Part : DataReference.GetParts())
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
    }
    IMPOSSIBLE_CODE_REACHED("switch handling of Inspection::TypeDefinition::DataReference::Root incomplete");
}

std::ostream & Inspection::operator<<(std::ostream & OStream, const Inspection::TypeDefinition::DataReference::Part & Part)
{
    return OStream << "Part[" << Part.GetType() << ", \"" << Part.GetName() << "\"]";
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
    }
    IMPOSSIBLE_CODE_REACHED("switch handling of Inspection::TypeDefinition::DataReference::Part::Type incomplete");
}

auto Inspection::operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::DataType const & DataType) -> std::ostream &
{
    switch(DataType)
    {
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
            return OStream << "unsigned-integer-8bit";
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger16Bit:
        {
            return OStream << "unsigned-integer-16bit";
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger32Bit:
        {
            return OStream << "unsigned-integer-32bit";
        }
    case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
        {
            return OStream << "unsigned-integer-64bit";
        }
    }
    IMPOSSIBLE_CODE_REACHED("switch handling of Inspection::TypeDefinition::DataType incomplete");
}

auto Inspection::operator<<(std::ostream & OStream, enum Inspection::TypeDefinition::PartType const & PartType) -> std::ostream &
{
    switch(PartType)
    {
    case Inspection::TypeDefinition::PartType::Alternative:
        {
            return OStream << "Alternative";
        }
    case Inspection::TypeDefinition::PartType::Array:
        {
            return OStream << "Array";
        }
    case Inspection::TypeDefinition::PartType::Field:
        {
            return OStream << "Field";
        }
    case Inspection::TypeDefinition::PartType::Forward:
        {
            return OStream << "Forward";
        }
    case Inspection::TypeDefinition::PartType::Select:
        {
            return OStream << "Select";
        }
    case Inspection::TypeDefinition::PartType::Sequence:
        {
            return OStream << "Sequence";
        }
    case Inspection::TypeDefinition::PartType::Type:
        {
            return OStream << "Type";
        }
    }
    IMPOSSIBLE_CODE_REACHED("switch handling of Inspection::TypeDefinition::PartType incomplete");
}

auto Inspection::to_string(enum Inspection::TypeDefinition::DataType const & DataType) -> std::string
{
    auto Stream = std::ostringstream{};
    
    Stream << DataType;
    
    return Stream.str();
}

auto Inspection::to_string(enum Inspection::TypeDefinition::PartType const & PartType) -> std::string
{
    auto Stream = std::ostringstream{};
    
    Stream << PartType;
    
    return Stream.str();
}

auto Inspection::to_string(std::type_info const & TypeInformation) -> std::string
{
    if(TypeInformation == typeid(std::bitset<16>))
    {
        return "std::bitset<16>";
    }
    else if(TypeInformation == typeid(std::bitset<32>))
    {
        return "std::bitset<32>";
    }
    else if(TypeInformation == typeid(bool))
    {
        return "boolean";
    }
    else if(TypeInformation == typeid(nullptr))
    {
        return "nothing";
    }
    else if(TypeInformation == typeid(std::int8_t))
    {
        return "signed integer 8bit";
    }
    else if(TypeInformation == typeid(std::int16_t))
    {
        return "signed integer 16bit";
    }
    else if(TypeInformation == typeid(std::int32_t))
    {
        return "signed integer 32bit";
    }
    else if(TypeInformation == typeid(std::int64_t))
    {
        return "signed integer 64bit";
    }
    else if(TypeInformation == typeid(float))
    {
        return "single precision real";
    }
    else if(TypeInformation == typeid(std::string))
    {
        return "string";
    }
    else if(TypeInformation == typeid(std::uint8_t))
    {
        return "unsigned integer 8bit";
    }
    else if(TypeInformation == typeid(std::uint16_t))
    {
        return "unsigned integer 16bit";
    }
    else if(TypeInformation == typeid(std::uint32_t))
    {
        return "unsigned integer 32bit";
    }
    else if(TypeInformation == typeid(std::uint64_t))
    {
        return "unsigned integer 64bit";
    }
    else
    {
        UNEXPECTED_CASE("TypeInformation == "s + TypeInformation.name());
    }
}
