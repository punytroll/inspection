#include <fstream>

#include "file_handling.h"
#include "getter_descriptor.h"
#include "getter_repository.h"
#include "getters.h"
#include "not_implemented_exception.h"
#include "xml_parser.h"

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

Inspection::GetterRepository::~GetterRepository(void)
{
	for(auto ModulePair : _Modules)
	{
		delete ModulePair.second;
		ModulePair.second = nullptr;
	}
}

std::unique_ptr< Inspection::Result > Inspection::GetterRepository::Get(const std::vector< std::string > & ModulePathParts, const std::string & GetterName, Inspection::Reader & Reader)
{
	auto GetterDescriptor{_GetOrLoadGetterDescriptor(ModulePathParts, GetterName)};
	
	if(GetterDescriptor != nullptr)
	{
		return GetterDescriptor->Get(Reader);
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		Result->GetValue()->AppendTag("error", "Could not find/load the getter \"" + GetterName + "\".");
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

Inspection::GetterDescriptor * Inspection::GetterRepository::_GetOrLoadGetterDescriptor(const std::vector< std::string > & ModulePathParts, const std::string & GetterName)
{
	auto Module{_GetOrLoadModule(ModulePathParts)};
	
	assert(Module != nullptr);
	
	Inspection::GetterDescriptor * Result{nullptr};
	auto GetterDescriptorIterator{Module->_GetterDescriptors.find(GetterName)};
	
	if(GetterDescriptorIterator != Module->_GetterDescriptors.end())
	{
		Result = GetterDescriptorIterator->second;
	}
	else
	{
		auto GetterPath{Module->_Path + '/' + GetterName + ".xml"};
		
		if((FileExists(GetterPath) == true) && (IsRegularFile(GetterPath) == true))
		{
			std::ifstream InputFileStream{GetterPath};
			XMLParser Parser{InputFileStream};
			
			Parser.Parse();
			if(GetterPath == "/home/moebius/projects/inspection/common/getters/ASCII/String_Printable_EndedByTermination.xml")
			{
				Result = new Inspection::GetterDescriptor{};
				Result->SetHardcodedGetter(Inspection::Get_ASCII_String_Printable_EndedByTermination);
				Module->_GetterDescriptors.insert(std::make_pair(GetterName, Result));
			}
			else if(GetterPath == "/home/moebius/projects/inspection/common/getters/Buffers/UnsignedInteger_8Bit_EndedByLength.xml")
			{
				Result = new Inspection::GetterDescriptor{};
				Result->SetHardcodedGetter(Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength);
				Module->_GetterDescriptors.insert(std::make_pair(GetterName, Result));
			}
		}
	}
	
	return Result;
}

Inspection::Module * Inspection::GetterRepository::_GetOrLoadModule(const std::vector< std::string > & ModulePathParts)
{
	std::string ModulePath{"/home/moebius/projects/inspection/common/getters"};
	Inspection::Module * Result{nullptr};
	auto Modules{&_Modules};
	
	for(auto ModulePathPart : ModulePathParts)
	{
		ModulePath += '/' + ModulePathPart;
		
		auto ModuleIterator{Modules->find(ModulePathPart)};
		
		if(ModuleIterator != Modules->end())
		{
			Result = ModuleIterator->second;
			Modules = &(Result->_Modules);
		}
		else
		{
			if((FileExists(ModulePath) == true) && (IsDirectory(ModulePath) == true))
			{
				Result = new Module();
				Result->_Path = ModulePath;
				Modules->insert(std::make_pair(ModulePathPart, Result));
				Modules = &(Result->_Modules);
			}
			else
			{
				return nullptr;
			}
		}
	}
	
	return Result;
}
