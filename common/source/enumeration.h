#ifndef INSPECTION_COMMON_ENUMERATION_H
#define INSPECTION_COMMON_ENUMERATION_H

#include <string>
#include <vector>

namespace XML
{
	class Element;
}

namespace Inspection
{
	class Enumeration
	{
	public:
		class Element
		{
		public:
			std::string BaseValue;
			std::string TagName;
			std::string TagType;
			std::string TagValue;
		};
		
		~Enumeration(void);
		void Load(const std::string & Path);
		void Load(const XML::Element * EnumerationElement);
		std::string BaseType;
		std::vector< Inspection::Enumeration::Element * > Elements;
	};
}

#endif
