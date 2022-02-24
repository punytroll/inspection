#ifndef COMMON_TYPE_REPOSITORY_H
#define COMMON_TYPE_REPOSITORY_H

#include <any>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Inspection
{
    class Result;
    class Reader;
    
    namespace TypeDefinition
    {
        class Module;
        class Type;
    }
    
    class TypeRepository
    {
    public:
        TypeRepository(void);
        ~TypeRepository(void);
        auto Get(std::vector<std::string> const & PathParts, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>;
        auto GetType(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Type const *;
    private:
        auto m_GetOrLoadType(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Type *;
        auto m_GetOrLoadModule(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Module *;
        std::unique_ptr<Inspection::TypeDefinition::Module> m_RootModule;
    };
    
    extern TypeRepository g_TypeRepository;
}

#endif
