#include <vector>

#include "explode.h"
#include "guid.h"
#include "helper.h"

Inspection::GUID::GUID(void)
{
    Data1 = 0;
    Data2 = 0;
    Data3 = 0;
    Data4[0] = 0;
    Data4[1] = 0;
    Data4[2] = 0;
    Data4[3] = 0;
    Data4[4] = 0;
    Data4[5] = 0;
    Data4[6] = 0;
    Data4[7] = 0;
}

/// parses a string with the following format, containing no whitespaces, case insensitive: "3F2504E0-4F89-41D3-9A0C-0305E82C3301"
Inspection::GUID::GUID(const std::string & RegistryFormat)
{
    GUID();
    
    std::vector< std::string > Fields;
    
    explode(RegistryFormat, '-', std::back_inserter(Fields));
    if(Fields.size() == 5)
    {
        if(Fields[0].length() == 8)
        {
            Data1 = Get_UnsignedInteger_32Bit_FromHexadecimalString(Fields[0]);
            Data2 = Get_UnsignedInteger_16Bit_FromHexadecimalString(Fields[1]);
            Data3 = Get_UnsignedInteger_16Bit_FromHexadecimalString(Fields[2]);
            Data4[0] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[3].substr(0, 2));
            Data4[1] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[3].substr(2, 2));
            Data4[2] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(0, 2));
            Data4[3] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(2, 2));
            Data4[4] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(4, 2));
            Data4[5] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(6, 2));
            Data4[6] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(8, 2));
            Data4[7] = Get_UnsignedInteger_8Bit_FromHexadecimalString(Fields[4].substr(10, 2));
        }
    }
    else
    {
        throw std::invalid_argument("The given GUID string does not have a valid format.");
    }
}

bool Inspection::GUID::operator==(const Inspection::GUID & Other) const
{
    return (Data1 == Other.Data1) && (Data2 == Other.Data2) && (Data3 == Other.Data3) && (Data4[0] == Other.Data4[0]) && (Data4[1] == Other.Data4[1]) && (Data4[2] == Other.Data4[2]) && (Data4[3] == Other.Data4[3]) && (Data4[4] == Other.Data4[4]) && (Data4[5] == Other.Data4[5]) && (Data4[6] == Other.Data4[6]) && (Data4[7] == Other.Data4[7]);
}
