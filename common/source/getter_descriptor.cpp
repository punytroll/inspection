#include "getter_descriptor.h"
#include "not_implemented_exception.h"
#include "result.h"

void Inspection::GetterDescriptor::SetHardcodedGetter(std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > HardcodedGetter)
{
	_HardcodedGetter = HardcodedGetter;
}

std::unique_ptr< Inspection::Result > Inspection::GetterDescriptor::Get(Inspection::Reader & Reader)
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
