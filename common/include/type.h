#ifndef INSPECTION_COMMON_TYPE_H
#define INSPECTION_COMMON_TYPE_H

#include <any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace XML
{
	class Element;
}

namespace Inspection
{
    class Reader;
    class Result;
    class TypeRepository;
    
	namespace TypeDefinition
	{
        class Part;
        
		class Type
		{
		public:
			Type(std::vector<std::string> const & PathParts, TypeRepository & TypeRepository);
			~Type();
			auto Get(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>;
			auto GetPathParts() const -> const std::vector<std::string>;
			auto Load(std::istream & InputStream) -> void;
		private:
			auto m_LoadType(XML::Element const * TypeElement) -> void;
			std::function<std::unique_ptr<Inspection::Result> (Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters)> m_HardcodedGetter;
			std::unique_ptr<Inspection::TypeDefinition::Part> m_Part;
			std::vector<std::string> m_PathParts;
			Inspection::TypeRepository & m_TypeRepository;
		};
	}
}

#endif
