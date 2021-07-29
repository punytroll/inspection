#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>

#include "helper.h"
#include "result.h"
#include "type.h"
#include "type_repository.h"
#include "type_library_path.h"

namespace Inspection
{
	Inspection::TypeRepository g_TypeRepository;

	class Module
	{
	public:
		~Module(void)
		{
			for(auto ModulePair : _Modules)
			{
				delete ModulePair.second;
				ModulePair.second = nullptr;
			}
			for(auto TypePair : _Types)
			{
				delete TypePair.second;
				TypePair.second = nullptr;
			}
		}
		
		std::map<std::string, Inspection::TypeDefinition::Type *> _Types;
		std::map<std::string, Module *> _Modules;
		std::string _Path;
	};
}

Inspection::TypeRepository::TypeRepository(void) :
	_RootModule{new Module{}}
{
	_RootModule->_Path = g_TypeLibraryPath;
}

Inspection::TypeRepository::~TypeRepository(void)
{
	delete _RootModule;
	_RootModule = nullptr;
}

const Inspection::TypeDefinition::Type * Inspection::TypeRepository::GetType(const std::vector<std::string> & PathParts)
{
	return _GetOrLoadType(PathParts);
}

std::unique_ptr<Inspection::Result> Inspection::TypeRepository::Get(const std::vector<std::string> & PathParts, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
	return _GetOrLoadType(PathParts)->Get(Reader, Parameters);
}

Inspection::TypeDefinition::Type * Inspection::TypeRepository::_GetOrLoadType(const std::vector<std::string> & PathParts)
{
	auto Module = _GetOrLoadModule(std::vector<std::string>{PathParts.begin(), PathParts.end() - 1});
	auto Result = (Inspection::TypeDefinition::Type *)nullptr;
	
	if(Module != nullptr)
	{
		auto TypeIterator = Module->_Types.find(PathParts.back());
		
		if(TypeIterator != Module->_Types.end())
		{
			Result = TypeIterator->second;
		}
		else
		{
			auto TypePath = Module->_Path + '/' + PathParts.back() + ".type";
			
			if((std::filesystem::exists(TypePath) == true) && (std::filesystem::is_regular_file(TypePath) == true))
			{
				Result = new Inspection::TypeDefinition::Type{PathParts, *this};
				try
				{
					auto InputFileStream = std::ifstream{TypePath};
					
					Result->Load(InputFileStream);
				}
				catch(std::domain_error & Exception)
				{
					std::throw_with_nested(std::runtime_error("Type path: " + TypePath));
				}
				Module->_Types.insert(std::make_pair(PathParts.back(), Result));
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

Inspection::Module * Inspection::TypeRepository::_GetOrLoadModule(const std::vector<std::string> & ModulePathParts)
{
	auto Result = _RootModule;
	
	for(auto ModulePathPart : ModulePathParts)
	{
		auto ModulePath = Result->_Path + '/' + ModulePathPart;
		auto ModuleIterator = Result->_Modules.find(ModulePathPart);
		
		if(ModuleIterator != Result->_Modules.end())
		{
			Result = ModuleIterator->second;
		}
		else
		{
			if((std::filesystem::exists(ModulePath) == true) && (std::filesystem::is_directory(ModulePath) == true))
			{
				auto NewModule = new Module{};
				
				NewModule->_Path = ModulePath;
				Result->_Modules.insert(std::make_pair(ModulePathPart, NewModule));
				Result = NewModule;
			}
			else
			{
				throw std::runtime_error("Could not find the module directory \"" + ModulePath + "\".");
			}
		}
	}
	
	return Result;
}
