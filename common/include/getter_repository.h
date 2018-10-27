#ifndef COMMON_GETTER_REPOSITORY_H
#define COMMON_GETTER_REPOSITORY_H

#include <experimental/any>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
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
		std::unique_ptr< Inspection::Result > Get(const std::vector< std::string > & PathParts, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		Inspection::Enumeration * GetEnumeration(const std::vector< std::string > & PathParts);
	private:
		Inspection::Enumeration * _GetOrLoadEnumeration(const std::vector< std::string > & PathParts);
		Inspection::GetterDescriptor * _GetOrLoadGetterDescriptor(const std::vector< std::string > & PathParts);
		Module * _GetOrLoadModule(const std::vector< std::string > & PathParts);
		Module * _RootModule;
	};
	
	extern GetterRepository g_GetterRepository;
}

#endif
