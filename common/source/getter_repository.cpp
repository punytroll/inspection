#include "getter_repository.h"
#include "getters.h"

namespace Inspection
{
	Inspection::GetterRepository g_GetterRepository;
	
	void InitializeGetterRepository(void)
	{
		g_GetterRepository.RegisterHardcodedGetter(std::vector< std::string >{"ASCII"}, "String Printable EndedByTermination", Inspection::Get_ASCII_String_Printable_EndedByTermination);
		g_GetterRepository.RegisterHardcodedGetter(std::vector< std::string >{"Buffers"}, "UnsignedInteger 8Bit EndedByLength", Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength);
	}
	
	class Getter
	{
	public:
		virtual ~Getter(void) = default;
		
		virtual std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader) = 0;

		bool GetSuccess(void) const
		{
			return _Continue;
		}
	protected:
		bool _Continue;
	};

	class HardcodedGetter : public Getter
	{
	public:
		HardcodedGetter(std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > Getter) :
			_Getter(Getter)
		{
		}
		
		virtual std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader) override
		{
			return _Getter(Reader);
		}
	private:
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _Getter;
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
			for(auto GetterPair : _Getters)
			{
				delete GetterPair.second;
				GetterPair.second = nullptr;
			}
		}
		
		void RegisterHardcodedGetter(const std::string & GetterName, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > Getter)
		{
			auto GetterIterator{_Getters.find(GetterName)};
			
			assert(GetterIterator == _Getters.end());
			_Getters.insert(std::make_pair(GetterName, new HardcodedGetter(Getter)));
		}
		
		std::map< std::string, Getter * > _Getters;
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
	auto Module{GetOrCreateModule(ModulePath)};
	
	assert(Module != nullptr);
	Module->RegisterHardcodedGetter(GetterName, Getter);
}

std::unique_ptr< Inspection::Result > Inspection::GetterRepository::Get(const std::vector< std::string > & ModulePath, const std::string & GetterName, Inspection::Reader & Reader)
{
	auto Module{GetModule(ModulePath)};
	
	if(Module != nullptr)
	{
		auto GetterIterator{Module->_Getters.find(GetterName)};
		
		if(GetterIterator != Module->_Getters.end())
		{
			return GetterIterator->second->Get(Reader);
		}
		else
		{
			auto Result{Inspection::InitializeResult(Reader)};
			
			Result->GetValue()->AppendTag("error", "Could not find the getter \"" + GetterName + "\".");
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		
		Result->GetValue()->AppendTag("error", "Could not find the module.");
		Result->SetSuccess(false);
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

Inspection::Module * Inspection::GetterRepository::GetModule(const std::vector< std::string > & ModulePath)
{
	Inspection::Module * Result{nullptr};
	auto Modules{&_Modules};
	
	for(auto ModulePathPart : ModulePath)
	{
		auto ModuleIterator{Modules->find(ModulePathPart)};
		
		if(ModuleIterator != Modules->end())
		{
			Result = ModuleIterator->second;
			Modules = &(Result->_Modules);
		}
		else
		{
			Result = nullptr;
			
			break;
		}
	}
	
	return Result;
}

Inspection::Module * Inspection::GetterRepository::GetOrCreateModule(const std::vector< std::string > & ModulePath)
{
	Inspection::Module * Result{nullptr};
	auto Modules{&_Modules};
	
	for(auto ModulePathPart : ModulePath)
	{
		auto ModuleIterator{Modules->find(ModulePathPart)};
		
		if(ModuleIterator != Modules->end())
		{
			Result = ModuleIterator->second;
			Modules = &(Result->_Modules);
		}
		else
		{
			Result = new Module();
			Modules->insert(std::make_pair(ModulePathPart, Result));
			Modules = &(Result->_Modules);
		}
	}
	
	return Result;
}
