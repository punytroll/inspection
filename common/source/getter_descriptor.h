#ifndef INSPECTION_COMMON_GETTER_DESCRIPTOR_H
#define INSPECTION_COMMON_GETTER_DESCRIPTOR_H

#include <functional>
#include <memory>

namespace Inspection
{
	class Reader;
	class Result;
	
	class GetterDescriptor
	{
	public:
		void SetHardcodedGetter(std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > HardcodedGetter);
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader);
	private:
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
	};
}

#endif
