#include <experimental/iterator>
#include <numeric>

#include "file_handling.h"
#include "type.h"
#include "type_repository.h"

std::string JoinSeparated(const std::vector< std::string > & Strings, const std::string & Separator)
{
	if(Strings.empty() == true)
	{
		return std::string();
	}
	else
	{
		return std::accumulate(Strings.begin() + 1, Strings.end(), Strings[0], [](const std::string & Start, const std::string & String) { return Start + "/" + String; } );
	}
}

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
			for(auto TypePair : _TypeDescriptors)
			{
				delete TypePair.second;
				TypePair.second = nullptr;
			}
		}
		
		std::map< std::string, Type * > _TypeDescriptors;
		std::map< std::string, Module * > _Modules;
		std::string _Path;
	};
}

Inspection::TypeRepository::TypeRepository(void) :
	_RootModule{new Module{}}
{
	_RootModule->_Path = "/home/moebius/projects/inspection/common/types";
}

Inspection::TypeRepository::~TypeRepository(void)
{
	delete _RootModule;
	_RootModule = nullptr;
}

const Inspection::Type * Inspection::TypeRepository::GetType(const std::vector< std::string > & PathParts)
{
	return _GetOrLoadType(PathParts);
}

std::unique_ptr< Inspection::Result > Inspection::TypeRepository::Get(const std::vector< std::string > & PathParts, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Type{_GetOrLoadType(PathParts)};
	
	if(Type != nullptr)
	{
		return Type->Get(Reader, Parameters);
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		Result->GetValue()->AddTag("error", "Could not find/load the type \"" + JoinSeparated(PathParts, "/") + "\".");
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

Inspection::Type * Inspection::TypeRepository::_GetOrLoadType(const std::vector< std::string > & PathParts)
{
	auto Module{_GetOrLoadModule(std::vector< std::string >{PathParts.begin(), PathParts.end() - 1})};
	Inspection::Type * Result{nullptr};
	
	if(Module != nullptr)
	{
		auto TypeIterator{Module->_TypeDescriptors.find(PathParts.back())};
		
		if(TypeIterator != Module->_TypeDescriptors.end())
		{
			Result = TypeIterator->second;
		}
		else
		{
			auto TypePath{Module->_Path + '/' + PathParts.back() + ".type"};
			
			if((FileExists(TypePath) == true) && (IsRegularFile(TypePath) == true))
			{
				Result = new Inspection::Type{this};
				Result->Load(TypePath);
				Module->_TypeDescriptors.insert(std::make_pair(PathParts.back(), Result));
			}
		}
	}
	else
	{
		std::cerr << "Could not find/load the module at \"" + JoinSeparated(PathParts, "/") + "\"." << std::endl;
	}
	
	return Result;
}

Inspection::Module * Inspection::TypeRepository::_GetOrLoadModule(const std::vector< std::string > & ModulePathParts)
{
	Inspection::Module * Result{_RootModule};
	
	for(auto ModulePathPart : ModulePathParts)
	{
		auto ModulePath{Result->_Path + '/' + ModulePathPart};
		auto ModuleIterator{Result->_Modules.find(ModulePathPart)};
		
		if(ModuleIterator != Result->_Modules.end())
		{
			Result = ModuleIterator->second;
		}
		else
		{
			if((FileExists(ModulePath) == true) && (IsDirectory(ModulePath) == true))
			{
				auto NewModule{new Module{}};
				
				NewModule->_Path = ModulePath;
				Result->_Modules.insert(std::make_pair(ModulePathPart, NewModule));
				Result = NewModule;
			}
			else
			{
				return nullptr;
			}
		}
	}
	
	return Result;
}
