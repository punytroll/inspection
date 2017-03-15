#ifndef COMMON_5TH_GETTERS_H
#define COMMON_5TH_GETTERS_H

#include "buffer.h"
#include "result.h"

namespace Inspection
{
	std::unique_ptr< Inspection::Result > Get_ASCII_AlphaStringEndedByLength(Inspection::Buffer & Buffer, const std::string & String)
	{
		auto Success{false};
		auto Value{std::string("")};
		
		if(Buffer.Has(String.length(), 0) == true)
		{
			Success = true;
			for(auto Character : String)
			{
				if(Character != Buffer.Get8Bits()[0])
				{
					Success = false;
					
					break;
				}
				else
				{
					Value += Character;
				}
			}
		}
		
		return Inspection::MakeResult(Success, Value);
	}
	
	std::unique_ptr< Inspection::Result > Get_Boolean_OneBit(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		auto Value{false};
		
		if(Buffer.Has(0ull, 1) == true)
		{
			Value = (0x01 & Buffer.Get1Bit()[0]) == 0x01;
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Value);
	}
	
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		std::uint8_t Value{0};
		
		if(Buffer.Has(0ull, 7) == true)
		{
			Value = Buffer.Get7Bits()[0];
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Value);
	}
	
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		std::uint32_t Result{0ul};
		
		if(Buffer.Has(0ull, 24) == true)
		{
			auto Value{Buffer.Get24Bits()};
			
			Result |= static_cast< std::uint32_t >(Value[0]) << 16;
			Result |= static_cast< std::uint32_t >(Value[1]) << 8;
			Result |= static_cast< std::uint32_t >(Value[2]);
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Result);
	}
}

#endif
