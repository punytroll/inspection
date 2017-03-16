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
				if(Character != Buffer.Get8Bits())
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
			Value = (0x01 & Buffer.Get1Bit()) == 0x01;
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
			Value = Buffer.Get7Bits();
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Value);
	}
	
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		std::uint16_t Result{0ul};
		
		if(Buffer.Has(0ull, 16) == true)
		{
			Result |= static_cast< std::uint16_t >(Buffer.Get8Bits()) << 8;
			Result |= static_cast< std::uint16_t >(Buffer.Get8Bits());
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Result);
	}
	
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		std::uint32_t Result{0ul};
		
		if(Buffer.Has(0ull, 20) == true)
		{
			Result |= static_cast< std::uint32_t >(Buffer.Get4Bits()) << 16;
			Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
			Result |= static_cast< std::uint32_t >(Buffer.Get8Bits());
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Result);
	}
	
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer)
	{
		auto Success{false};
		std::uint32_t Result{0ul};
		
		if(Buffer.Has(0ull, 24) == true)
		{
			Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
			Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
			Result |= static_cast< std::uint32_t >(Buffer.Get8Bits());
			Success = true;
		}
		
		return Inspection::MakeResult(Success, Result);
	}
}

#endif
