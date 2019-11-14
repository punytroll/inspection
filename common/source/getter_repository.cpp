#include <experimental/iterator>
#include <numeric>

#include "file_handling.h"
#include "getter_descriptor.h"
#include "getter_repository.h"

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
	Inspection::GetterRepository g_GetterRepository;

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
			for(auto GetterDescriptorPair : _GetterDescriptors)
			{
				delete GetterDescriptorPair.second;
				GetterDescriptorPair.second = nullptr;
			}
		}
		
		std::map< std::string, GetterDescriptor * > _GetterDescriptors;
		std::map< std::string, Module * > _Modules;
		std::string _Path;
	};
}

Inspection::GetterRepository::GetterRepository(void) :
	_RootModule{new Module{}}
{
	_RootModule->_Path = "/home/moebius/projects/inspection/common/getters";
}

Inspection::GetterRepository::~GetterRepository(void)
{
	delete _RootModule;
	_RootModule = nullptr;
}

std::unique_ptr< Inspection::Result > Inspection::GetterRepository::Get(const std::vector< std::string > & PathParts, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto GetterDescriptor{_GetOrLoadGetterDescriptor(PathParts)};
	
	if(GetterDescriptor != nullptr)
	{
		return GetterDescriptor->Get(Reader, Parameters);
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		Result->GetValue()->AddTag("error", "Could not find/load the getter \"" + JoinSeparated(PathParts, "/") + "\".");
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

Inspection::GetterDescriptor * Inspection::GetterRepository::_GetOrLoadGetterDescriptor(const std::vector< std::string > & PathParts)
{
	auto Module{_GetOrLoadModule(std::vector< std::string >{PathParts.begin(), PathParts.end() - 1})};
	Inspection::GetterDescriptor * Result{nullptr};
	
	if(Module != nullptr)
	{
		auto GetterDescriptorIterator{Module->_GetterDescriptors.find(PathParts.back())};
		
		if(GetterDescriptorIterator != Module->_GetterDescriptors.end())
		{
			Result = GetterDescriptorIterator->second;
		}
		else
		{
			auto GetterPath{Module->_Path + '/' + PathParts.back() + ".getter"};
			
			if((FileExists(GetterPath) == true) && (IsRegularFile(GetterPath) == true))
			{
				Result = new Inspection::GetterDescriptor{this};
				Result->LoadGetterDescription(GetterPath);
				Module->_GetterDescriptors.insert(std::make_pair(PathParts.back(), Result));
			}
		}
	}
	else
	{
		std::cerr << "Getting or loading the module at ";
		std::copy(std::begin(PathParts), std::end(PathParts), std::experimental::make_ostream_joiner(std::cerr, "/"));
		std::cerr << " failed!" << std::endl;
	}
	
	return Result;
}

Inspection::Module * Inspection::GetterRepository::_GetOrLoadModule(const std::vector< std::string > & ModulePathParts)
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
