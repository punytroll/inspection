#ifndef COMMON_GETTER_REPOSITORY_H
#define COMMON_GETTER_REPOSITORY_H

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Inspection
{
	class Enumeration;
	class GetterDescriptor;
	class Module;
	class Result;
	class Reader;
	
	class GetterRepository
	{
	public:
		GetterRepository(void);
		~GetterRepository(void);
		std::unique_ptr< Inspection::Result > Get(const std::vector< std::string > & ModulePathParts, const std::string & GetterName, Inspection::Reader & Reader);
		Inspection::Enumeration * GetEnumeration(const std::vector< std::string > & ModulePathParts, const std::string & EnumerationIdentifier);
	private:
		Inspection::Enumeration * _GetOrLoadEnumeration(const std::vector< std::string > & ModulePathParts, const std::string & EnumerationName);
		Inspection::GetterDescriptor * _GetOrLoadGetterDescriptor(const std::vector< std::string > & ModulePathParts, const std::string & GetterName);
		Module * _GetOrLoadModule(const std::vector< std::string > & ModulePathParts);
		Module * _RootModule;
	};
	
	extern GetterRepository g_GetterRepository;
}

#endif
