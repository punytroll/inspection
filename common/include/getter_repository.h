#ifndef COMMON_GETTER_REPOSITORY_H
#define COMMON_GETTER_REPOSITORY_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Inspection
{
	class GetterDescriptor;
	class Module;
	class Result;
	class Reader;
	
	class GetterRepository
	{
	public:
		~GetterRepository(void);
		std::unique_ptr< Inspection::Result > Get(const std::vector< std::string > & ModulePathParts, const std::string & GetterName, Inspection::Reader & Reader);
	private:
		GetterDescriptor * _GetOrLoadGetterDescriptor(const std::vector< std::string > & ModulePathParts, const std::string & GetterName);
		Module * _GetOrLoadModule(const std::vector< std::string > & ModulePathParts);
		std::map< std::string, Module * > _Modules;
	};
	
	extern GetterRepository g_GetterRepository;
}

#endif
