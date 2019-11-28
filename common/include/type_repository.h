#ifndef COMMON_TYPE_REPOSITORY_H
#define COMMON_TYPE_REPOSITORY_H

#include <experimental/any>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Inspection
{
	class Module;
	class Result;
	class Reader;
	class Type;
	
	class TypeRepository
	{
	public:
		TypeRepository(void);
		~TypeRepository(void);
		std::unique_ptr< Inspection::Result > Get(const std::vector< std::string > & PathParts, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		const Inspection::Type * GetType(const std::vector< std::string > & PathParts);
	private:
		Inspection::Type * _GetOrLoadType(const std::vector< std::string > & PathParts);
		Module * _GetOrLoadModule(const std::vector< std::string > & PathParts);
		Module * _RootModule;
	};
	
	extern TypeRepository g_TypeRepository;
}

#endif
