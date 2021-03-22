#ifndef INSPECTION_COMMON_GUID_H
#define INSPECTION_COMMON_GUID_H

#include <cstdint>
#include <string>

namespace Inspection
{
	class GUID
	{
	public:
		GUID(void);
		/// parses a string with the following format, containing no whitespaces, case insensitive: "3F2504E0-4F89-41D3-9A0C-0305E82C3301"
		GUID(const std::string & RegistryFormat);
		bool operator==(const GUID & Other) const;
		std::uint32_t Data1;
		std::uint16_t Data2;
		std::uint16_t Data3;
		std::uint8_t Data4[8];
	};
}

#endif
