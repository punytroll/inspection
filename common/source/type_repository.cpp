#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "helper.h"
#include "result.h"
#include "type.h"
#include "type_repository.h"
#include "type_library_path.h"
#include "xml_puny_dom.h"

namespace Inspection
{
	Inspection::TypeRepository g_TypeRepository;
    
    namespace TypeDefinition
    {
        class Module
        {
        public:
            Module(std::string const & Path) :
                Path{Path}
            {
            }
            
            std::map<std::string, std::unique_ptr<Inspection::TypeDefinition::Type>> Types;
            std::map<std::string, std::unique_ptr<Inspection::TypeDefinition::Module>> Modules;
            std::string Path;
        };
    }
}

Inspection::TypeRepository::TypeRepository(void) :
	m_RootModule{std::make_unique<Inspection::TypeDefinition::Module>(g_TypeLibraryPath)}
{
}

Inspection::TypeRepository::~TypeRepository(void)
{
}

auto Inspection::TypeRepository::Get(std::vector<std::string> const & PathParts, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>
{
	return m_GetOrLoadType(PathParts)->Get(Reader, Parameters);
}

auto Inspection::TypeRepository::GetType(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Type const *
{
	return m_GetOrLoadType(PathParts);
}

auto Inspection::TypeRepository::m_GetOrLoadType(std::vector<std::string> const & PathParts) -> Inspection::TypeDefinition::Type *
{
	auto Module = m_GetOrLoadModule(std::vector<std::string>{PathParts.begin(), PathParts.end() - 1});
	auto Result = (Inspection::TypeDefinition::Type *)nullptr;
	
	if(Module != nullptr)
	{
		auto TypeIterator = Module->Types.find(PathParts.back());
		
		if(TypeIterator != Module->Types.end())
		{
			Result = TypeIterator->second.get();
		}
		else
		{
			auto TypePath = Module->Path + '/' + PathParts.back() + ".type";
			
			if((std::filesystem::exists(TypePath) == true) && (std::filesystem::is_regular_file(TypePath) == true))
			{
				auto Type = std::unique_ptr<Inspection::TypeDefinition::Type>{};
                
				try
				{
					auto InputFileStream = std::ifstream{TypePath};
                    auto Document = XML::Document{InputFileStream};
                    auto DocumentElement = Document.GetDocumentElement();
                    
                    ASSERTION(DocumentElement != nullptr);
                    ASSERTION(DocumentElement->GetName() == "type");
                    Type = Inspection::TypeDefinition::Type::Load(DocumentElement, PathParts, *this);
                    Result = Type.get();
                    Module->Types.insert(std::make_pair(PathParts.back(), std::move(Type)));
				}
				catch(std::domain_error & Exception)
				{
					std::throw_with_nested(std::runtime_error("Type path: " + TypePath));
				}
			}
			else
			{
				throw std::runtime_error("Could not find the type file \"" + TypePath + "\".");
			}
		}
	}
	else
	{
		std::cerr << "Could not find/load the module at \"" + Inspection::JoinWithSeparator(PathParts, "/") + "\"." << std::endl;
	}
	
	return Result;
}

auto Inspection::TypeRepository::m_GetOrLoadModule(std::vector<std::string> const & ModulePathParts) -> Inspection::TypeDefinition::Module *
{
	auto Result = m_RootModule.get();
	
	for(auto ModulePathPart : ModulePathParts)
	{
		auto ModulePath = Result->Path + '/' + ModulePathPart;
		auto ModuleIterator = Result->Modules.find(ModulePathPart);
		
		if(ModuleIterator != Result->Modules.end())
		{
			Result = ModuleIterator->second.get();
		}
		else
		{
			if((std::filesystem::exists(ModulePath) == true) && (std::filesystem::is_directory(ModulePath) == true))
			{
                auto ParentModule = Result;
				auto NewModule = std::make_unique<Inspection::TypeDefinition::Module>(ModulePath);
				
				Result = NewModule.get();
				ParentModule->Modules.insert(std::make_pair(ModulePathPart, std::move(NewModule)));
			}
			else
			{
				throw std::runtime_error("Could not find the module directory \"" + ModulePath + "\".");
			}
		}
	}
	
	return Result;
}
