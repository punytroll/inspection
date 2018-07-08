#ifndef INSPECTION_COMMON_READER_H
#define INSPECTION_COMMON_READER_H

#include <cstdint>
#include <iostream>

#include "length.h"

namespace Inspection
{
	class Buffer;
	
	class Reader
	{
	public:
		friend class Inspection::Buffer;
		
		Reader(Inspection::Buffer & Buffer);
		Reader(Inspection::Buffer & Buffer, const Inspection::Length & Length);
		Reader(Inspection::Buffer & Buffer, const Inspection::Length & OffsetInBuffer, const Inspection::Length & Length);
		Reader(Inspection::Reader & Reader, const Inspection::Length & Length);
		std::uint8_t Get0Bits(void);
		std::uint8_t Get1Bits(void);
		std::uint8_t Get2Bits(void);
		std::uint8_t Get3Bits(void);
		std::uint8_t Get4Bits(void);
		std::uint8_t Get5Bits(void);
		std::uint8_t Get6Bits(void);
		std::uint8_t Get7Bits(void);
		std::uint8_t Get8Bits(void);
		
		void AdvancePosition(const Inspection::Length & Offset)
		{
			if(_PositionInBuffer + Offset <= _BoundaryInBuffer)
			{
				_PositionInBuffer += Offset;
			}
			else
			{
				assert(false);
			}
		}
		
		const Inspection::Length & GetPositionInBuffer(void) const
		{
			return _PositionInBuffer;
		}
		
		Inspection::Length GetConsumedLength(void) const
		{
			return _PositionInBuffer - _OffsetInBuffer;
		}
		
		Inspection::Length GetRemainingLength(void) const
		{
			return _BoundaryInBuffer - _PositionInBuffer;
		}
		
		bool Has(const Inspection::Length & Length) const
		{
			return _PositionInBuffer + Length <= _BoundaryInBuffer;
		}
	private:
		Inspection::Length _BoundaryInBuffer;
		Inspection::Buffer & _Buffer;
		Inspection::Length _OffsetInBuffer;
		Inspection::Length _PositionInBuffer;
	};
}

#endif
