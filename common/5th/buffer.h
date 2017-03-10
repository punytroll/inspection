#ifndef COMMON_5TH_BUFFER_H
#define COMMON_5TH_BUFFER_H

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
			
			auto Result{(_Data[_Position.GetBytes()] >> (7 - _Position.GetBits())) & 0x01};
			
			_Position += Inspection::Length(0ull, 1);
			
			return Result;
		}
		
		std::uint8_t Get1Byte(void)
		{
			assert(Has(1ull, 0) == true);
			
			std::uint8_t Result{0};
			
			if(_Position.GetBits() == 0)
			{
				Result = _Data[_Position.GetBytes()];
			}
			else
			{
				Result = (_Data[_Position.GetBytes()] << _Position.GetBits()) + ((_Data[_Position.GetBytes() + 1] >> (8 - _Position.GetBits())) & ((1 << _Position.GetBits()) - 1));
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
