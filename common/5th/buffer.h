#ifndef COMMON_5TH_BUFFER_H
#define COMMON_5TH_BUFFER_H

#include <array>

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
		
		std::array< std::uint8_t, 1 > Get1Bit(void)
		{
			assert(Has(0ull, 1) == true);
			
			std::array< std::uint8_t, 1 > Result;
			
			Result[0] = (*(_Data + _Position.GetBytes()) >> (7 - _Position.GetBits())) & 0x01;
			_Position += Inspection::Length(0ull, 1);
			
			return Result;
		}
		
		std::array< std::uint8_t, 1 > Get7Bits(void)
		{
			assert(Has(0ull, 7) == true);
			
			std::array< std::uint8_t, 1 > Result;
			
			if(_Position.GetBits() < 2)
			{
				Result[0] = (*(_Data + _Position.GetBytes()) >> (1 - _Position.GetBits())) & 0x7f;
			}
			else
			{
				Result[0] = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 1)) + ((*(_Data + _Position.GetBytes() + 1ull)) >> (9 - _Position.GetBits()))) & 0x7f;
			}
			_Position += Inspection::Length(0ull, 7);
			
			return Result;
		}
		
		std::array< std::uint8_t, 1 > Get8Bits(void)
		{
			assert(Has(1ull, 0) == true);
			
			std::array< std::uint8_t, 1 > Result;
			
			if(_Position.GetBits() == 0)
			{
				Result[0] = _Data[_Position.GetBytes()];
			}
			else
			{
				Result[0] = (*(_Data + _Position.GetBytes()) << _Position.GetBits()) + ((*(_Data + _Position.GetBytes() + 1ull) >> (8 - _Position.GetBits())) & ((1 << _Position.GetBits()) - 1));
			}
			_Position += Inspection::Length(1ull, 0);
			
			return Result;
		}
		
		std::array< std::uint8_t, 3 > Get24Bits(void)
		{
			assert(Has(0ull, 24) == true);
			
			std::array< std::uint8_t, 3 > Result;
			
			Result[0] = Get8Bits()[0];
			Result[1] = Get8Bits()[0];
			Result[2] = Get8Bits()[0];
			
			return Result;
		}
	private:
		const std::uint8_t * _Data;
		Length _Length;
		Length _Position;
	};
}

#endif
