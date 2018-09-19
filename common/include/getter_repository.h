#ifndef COMMON_GETTER_REPOSITORY_H
#define COMMON_GETTER_REPOSITORY_H

#include <map>
#include <string>

#include "getters.h"
#include "result.h"

class Getter
{
public:
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
	void RegisterGetter(const std::string & GetterName, Getter * Getter)
	{
		auto GetterIterator{_Getters.find(GetterName)};
		
		assert(GetterIterator == _Getters.end());
		_Getters.insert(std::make_pair(GetterName, Getter));
	}
	
	std::unique_ptr< Inspection::Result > Get(const std::string & GetterName, Inspection::Reader & Reader)
	{
		auto GetterIterator{_Getters.find(GetterName)};
		
		if(GetterIterator != _Getters.end())
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
	
	std::map< std::string, Getter * > _Getters;
};

class GetterRepository
{
public:
	void LoadFile(const std::string & FilePath);
	void RegisterHardcodedGetter(const std::string & ModuleName, const std::string & GetterName, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > Getter)
	{
		auto ModuleIterator{_Modules.find(ModuleName)};
		
		if(ModuleIterator == _Modules.end())
		{
			auto InsertResult{_Modules.insert(std::make_pair(ModuleName, new Module()))};
			
			ModuleIterator = InsertResult.first;
		}
		ModuleIterator->second->RegisterGetter(GetterName, new HardcodedGetter(Getter));
	}
	
	std::unique_ptr< Inspection::Result > Get(const std::string & ModuleName, const std::string & GetterName, Inspection::Reader & Reader)
	{
		auto ModuleIterator{_Modules.find(ModuleName)};
		
		if(ModuleIterator != _Modules.end())
		{
			return ModuleIterator->second->Get(GetterName, Reader);
		}
		else
		{
			auto Result{Inspection::InitializeResult(Reader)};
			
			Result->GetValue()->AppendTag("error", "Could not find the module \"" + ModuleName + "\".");
			Result->SetSuccess(false);
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
	}
private:
	std::map< std::string, Module * > _Modules;
};

extern GetterRepository g_GetterRepository;

inline void InitializeGetterRepository(void)
{
	g_GetterRepository.RegisterHardcodedGetter("ASCII", "String Printable EndedByTermination", Inspection::Get_ASCII_String_Printable_EndedByTermination);
}

#endif
