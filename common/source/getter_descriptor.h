#ifndef INSPECTION_COMMON_GETTER_DESCRIPTOR_H
#define INSPECTION_COMMON_GETTER_DESCRIPTOR_H

#include <functional>
#include <memory>
#include <vector>

namespace Inspection
{
	class PartDescriptor;
	class Reader;
	class Result;
	
	class GetterDescriptor
	{
	public:
		~GetterDescriptor(void);
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader);
		void LoadGetterDescription(const std::string & GetterPath);
	private:
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
		std::vector< Inspection::PartDescriptor * > _PartDescriptors;
	};
}

#endif
