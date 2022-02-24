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
    class ExecutionContext;
    class Reader;
    class Result;
    class TypeRepository;
    
    namespace TypeDefinition
    {
        class Part;
        
        class Type
        {
        public:
            static auto Load(XML::Element const * Element, std::vector<std::string> const & PathParts) -> std::unique_ptr<Inspection::TypeDefinition::Type>;
            static auto Load(XML::Element const * Element, std::vector<std::string> const & PathParts, TypeRepository * TypeRepository) -> std::unique_ptr<Inspection::TypeDefinition::Type>;
        public:
            ~Type();
            auto Get(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>;
            auto GetPathParts() const -> std::vector<std::string> const &;
            auto SetTypeRepository(Inspection::TypeRepository & TypeRepository) -> void;
        private:
            Type(std::vector<std::string> const & PathParts, TypeRepository * TypeRepository);
            std::function<std::unique_ptr<Inspection::Result> (Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters)> m_HardcodedGetter;
            std::unique_ptr<Inspection::TypeDefinition::Part> m_Part;
            std::vector<std::string> m_PathParts;
            Inspection::TypeRepository * m_TypeRepository;
        };
    }
}

#endif
