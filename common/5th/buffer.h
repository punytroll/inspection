#ifndef COMMON_5TH_BUFFER_H
#define COMMON_5TH_BUFFER_H

#include <cassert>

#include "length.h"

namespace Inspection
{
	class Buffer
	{
	public:
		Buffer(const std::uint8_t * Data, const Length & Length) :
			_Data(Data),
			_Length(Length),
			_Position(0ull, 0)
		{
		}
		
		const Inspection::Length & GetPosition(void) const
		{
			return _Position;
		}
		
		bool Has(std::uint64_t Bytes, std::uint8_t Bits)
		{
			return Has(Length(Bytes, Bits));
		}
		
		bool Has(const Length & Length)
		{
			return _Position + Length <= _Length;
		}
		
		std::uint8_t Get1Bit(void)
		{
			assert(Has(0ull, 1) == true);
			
			std::uint8_t Result;
			
			Result = (*(_Data + _Position.GetBytes()) >> (7 - _Position.GetBits())) & 0x01;
			_Position += Inspection::Length(0ull, 1);
			
			return Result;
		}
		
		std::uint8_t Get3Bits(void)
		{
			assert(Has(0ull, 3) == true);
			
			std::uint8_t Result;
			
			if(_Position.GetBits() < 6)
			{
				Result = (*(_Data + _Position.GetBytes()) >> (5 - _Position.GetBits())) & 0x07;
			}
			else
			{
				Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 5)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (13 - _Position.GetBits()))) & 0x07;
			}
			_Position += Inspection::Length(0ull, 3);
			
			return Result;
		}
		
		std::uint8_t Get4Bits(void)
		{
			assert(Has(0ull, 4) == true);
			
			std::uint8_t Result;
			
			if(_Position.GetBits() < 5)
			{
				Result = (*(_Data + _Position.GetBytes()) >> (4 - _Position.GetBits())) & 0x0f;
			}
			else
			{
				Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 4)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (12 - _Position.GetBits()))) & 0x0f;
			}
			_Position += Inspection::Length(0ull, 4);
			
			return Result;
		}
		
		std::uint8_t Get5Bits(void)
		{
			assert(Has(0ull, 5) == true);
			
			std::uint8_t Result;
			
			if(_Position.GetBits() < 4)
			{
				Result = (*(_Data + _Position.GetBytes()) >> (3 - _Position.GetBits())) & 0x07;
			}
			else
			{
				Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 3)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (11 - _Position.GetBits()))) & 0x1f;
			}
			_Position += Inspection::Length(0ull, 5);
			
			return Result;
		}
		
		std::uint8_t Get7Bits(void)
		{
			assert(Has(0ull, 7) == true);
			
			std::uint8_t Result;
			
			if(_Position.GetBits() < 2)
			{
				Result = (*(_Data + _Position.GetBytes()) >> (1 - _Position.GetBits())) & 0x7f;
			}
			else
			{
				Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 1)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (9 - _Position.GetBits()))) & 0x7f;
			}
			_Position += Inspection::Length(0ull, 7);
			
			return Result;
		}
		
		std::uint8_t Get8Bits(void)
		{
			assert(Has(1ull, 0) == true);
			
			std::uint8_t Result;
			
			if(_Position.GetBits() == 0)
			{
				Result = _Data[_Position.GetBytes()];
			}
			else
			{
				Result = (*(_Data + _Position.GetBytes()) << _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull) >> (8 - _Position.GetBits())) & ((1 << _Position.GetBits()) - 1));
			}
			_Position += Inspection::Length(1ull, 0);
			
			return Result;
		}
	private:
		const std::uint8_t * _Data;
		Length _Length;
		Length _Position;
	};
}

#endif
