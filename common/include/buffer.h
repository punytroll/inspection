#ifndef INSPECTION_COMMON_BUFFER_H
#define INSPECTION_COMMON_BUFFER_H

#include "length.h"

namespace Inspection
{
	class Buffer
	{
	public:
		Buffer(const std::uint8_t * Data, const Inspection::Length & Length) :
			_Data{Data},
			_Length{Length}
		{
		}
		
		~Buffer(void)
		{
			_Data = nullptr;
		}
		
		const std::uint8_t * GetData(void) const
		{
			return _Data;
		}
		
		const Inspection::Length & GetLength(void) const
		{
			return _Length;
		}
	private:
		const std::uint8_t * _Data;
		Inspection::Length _Length;
	};
}

#endif
