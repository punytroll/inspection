#ifndef COMMON_GETTER_REPOSITORY_H
#define COMMON_GETTER_REPOSITORY_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Inspection
{
	class Module;
	class Result;
	class Reader;
	
	class GetterRepository
	{
	public:
		~GetterRepository(void);
		void RegisterHardcodedGetter(const std::vector< std::string > & ModulePath, const std::string & GetterName, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > Getter);
		std::unique_ptr< Inspection::Result > Get(const std::vector< std::string > & ModulePath, const std::string & GetterName, Inspection::Reader & Reader);
	private:
		Module * GetModule(const std::vector< std::string > & ModulePath);
		Module * _GetOrLoadModule(const std::vector< std::string > & ModulePathParts);
		std::map< std::string, Module * > _Modules;
	};
	
	extern GetterRepository g_GetterRepository;
	
	void InitializeGetterRepository(void);
}

#endif
