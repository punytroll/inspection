#ifndef INSPECTION_COMMON_ENUMERATION_H
#define INSPECTION_COMMON_ENUMERATION_H

#include <string>
#include <vector>

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
		std::string BaseType;
		std::vector< Inspection::Enumeration::Element * > Elements;
	};
}

#endif
