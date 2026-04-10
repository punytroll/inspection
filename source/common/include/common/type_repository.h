#ifndef COMMON_TYPE_REPOSITORY_H
#define COMMON_TYPE_REPOSITORY_H

#include <any>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Inspection
{
    class Result;
    class Reader;
    class Type;
    
    namespace TypeDefinition
    {
        class Module;
        class Type;
    }
    
    class TypeRepository
    {
    public:
        static auto GetInstance() -> Inspection::TypeRepository &;
        ~TypeRepository();
        auto Get(std::vector<std::string> const & PathParts, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>;
        auto GetType(std::vector<std::string> const & PathParts) -> Inspection::Type const *;
    private:
        TypeRepository(std::filesystem::path TypeLibraryPath);
        auto m_GetOrLoadType(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Type *;
        auto m_GetOrLoadModule(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Module *;
        static std::unique_ptr<Inspection::TypeRepository> m_Instance;
        std::unique_ptr<Inspection::TypeDefinition::Module> m_RootModule;
    };
}

#endif
