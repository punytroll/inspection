#include <fstream>

#include "file_handling.h"
#include "getter_repository.h"
#include "getters.h"
#include "not_implemented_exception.h"
#include "xml_parser.h"

namespace Inspection
{
	Inspection::GetterRepository g_GetterRepository;
	
	void InitializeGetterRepository(void)
	{
		std::ifstream InputFileStream{"/home/moebius/projects/inspection/common/getters/ASCII.xml"};
		XMLParser Parser{InputFileStream};
		
		Parser.Parse();
		g_GetterRepository.RegisterHardcodedGetter(std::vector< std::string >{"ASCII"}, "String_Printable_EndedByTermination", Inspection::Get_ASCII_String_Printable_EndedByTermination);
		g_GetterRepository.RegisterHardcodedGetter(std::vector< std::string >{"Buffers"}, "UnsignedInteger_8Bit_EndedByLength", Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength);
	}

	class GetterDescriptor
	{
	public:
		void SetHardcodedGetter(std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > HardcodedGetter)
		{
			_HardcodedGetter = HardcodedGetter;
		}
		
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader)
		{
			if(_HardcodedGetter != nullptr)
			{
				return _HardcodedGetter(Reader);
			}
			else
			{
				throw Inspection::NotImplementedException{"Only hard coded getters work."};
			}
		}
	private:
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
	};

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

void Inspection::GetterRepository::RegisterHardcodedGetter(const std::vector< std::string > & ModulePath, const std::string & GetterName, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > Getter)
{
	auto Module{_GetOrLoadModule(ModulePath)};
	
	assert(Module != nullptr);
	
	auto GetterDescriptorIterator{Module->_GetterDescriptors.find(GetterName)};
	
	assert(GetterDescriptorIterator == Module->_GetterDescriptors.end());
	
	auto GetterDescriptor{new Inspection::GetterDescriptor{}};
	
	GetterDescriptor->SetHardcodedGetter(Getter);
	Module->_GetterDescriptors.insert(std::make_pair(GetterName, GetterDescriptor));
}

std::unique_ptr< Inspection::Result > Inspection::GetterRepository::Get(const std::vector< std::string > & ModulePath, const std::string & GetterName, Inspection::Reader & Reader)
{
	auto Module{_GetOrLoadModule(ModulePath)};
	
	assert(Module != nullptr);
	
	auto GetterDescriptorIterator{Module->_GetterDescriptors.find(GetterName)};
	
	if(GetterDescriptorIterator != Module->_GetterDescriptors.end())
	{
		return GetterDescriptorIterator->second->Get(Reader);
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		
		Result->GetValue()->AppendTag("error", "Could not find the getter \"" + GetterName + "\".");
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

Inspection::Module * Inspection::GetterRepository::_GetOrLoadModule(const std::vector< std::string > & ModulePathParts)
{
	std::string ModulePath{"/home/moebius/projects/inspection/common/getters/"};
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
