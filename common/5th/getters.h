#ifndef COMMON_5TH_GETTERS_H
#define COMMON_5TH_GETTERS_H

#include "buffer.h"
#include "result.h"

namespace Inspection
{
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
	
	std::unique_ptr< Inspection::Result > Get_ASCII_AlphaStringEndedByLength(Inspection::Buffer & Buffer, const std::string & String)
	{
		auto Success{false};
		auto Value{std::string("")};
		
		if(Buffer.Has(String.length(), 0) == true)
		{
			Success = true;
			for(auto Character : String)
			{
				if(Character != Buffer.Get1Byte())
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
}

#endif
