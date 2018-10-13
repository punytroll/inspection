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
			class Tag
			{
			public:
				std::string Name;
				std::string Type;
				std::string Value;
			};
			
			~Element(void);
			std::string BaseValue;
			std::vector< Inspection::Enumeration::Element::Tag * > Tags;
		};
		
		Enumeration(void);
		~Enumeration(void);
		void Load(const std::string & Path);
		void Load(const XML::Element * EnumerationElement);
		std::string BaseType;
		std::vector< Inspection::Enumeration::Element * > Elements;
		Inspection::Enumeration::Element * FallbackElement;
	};
}

#endif
