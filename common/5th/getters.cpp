#include <vector>

#include "buffer.h"
#include "getters.h"

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetical_EndedTemplateByLength(Inspection::Buffer & Buffer, const std::string & String)
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

std::unique_ptr< Inspection::Result > Inspection::Get_Boolean_OneBit(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Success{false};
	std::vector< std::uint8_t > Value;
	
	if(Buffer.Has(Length, 0) == true)
	{
		for(auto Index = 0ull; Index < Length; ++Index)
		{
			Value.push_back(Buffer.Get8Bits());
		}
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Success{false};
	std::shared_ptr< Inspection::Value > Value;
	auto Data{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
	
	if(Data->GetSuccess() == true)
	{
		Success = true;
		
		const std::vector< std::uint8_t > & DataValue{std::experimental::any_cast< const std::vector< std::uint8_t > & >(Data->GetAny())};
		
		for(auto Byte : DataValue)
		{
			if(Byte != 0x00)
			{
				Success = false;
				
				break;
			}
		}
		Value = Data->GetValue();
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_3Bit(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint8_t Value{0};
	
	if(Buffer.Has(0ull, 3) == true)
	{
		Value = Buffer.Get3Bits();
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_4Bit(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint8_t Value{0};
	
	if(Buffer.Has(0ull, 4) == true)
	{
		Value = Buffer.Get4Bits();
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_5Bit(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint8_t Value{0};
	
	if(Buffer.Has(0ull, 5) == true)
	{
		Value = Buffer.Get5Bits();
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint8_t Value{0};
	
	if(Buffer.Has(0ull, 8) == true)
	{
		Value = Buffer.Get8Bits();
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint32_t Result{0ul};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		Result |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Result |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24;
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Result);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::uint64_t Result{0ull};
	
	if(Buffer.Has(0ull, 36) == true)
	{
		Result |= static_cast< std::uint64_t >(Buffer.Get4Bits()) << 32;
		Result |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 24;
		Result |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 16;
		Result |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 8;
		Result |= static_cast< std::uint64_t >(Buffer.Get8Bits());
		Success = true;
	}
	
	return Inspection::MakeResult(Success, Result);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF8_Character(Inspection::Buffer & Buffer)
{
	auto Success{false};
	std::string Value;
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		
		Value += First;
		if((First & 0x80) == 0x00)
		{
			Success = true;
		}
		else if((First & 0xe0) == 0xc0)
		{
			if(Buffer.Has(1ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				
				if((Second & 0xc0) == 0x80)
				{
					Value += Second;
					Success = true;
				}
			}
		}
		else if((First & 0xf0) == 0xe0)
		{
			if(Buffer.Has(2ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80))
				{
					Value += Second;
					Value += Third;
					Success = true;
				}
			}
		}
		else if((First & 0xf8) == 0xf0)
		{
			if(Buffer.Has(3ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80))
				{
					Value += Second;
					Value += Third;
					Value += Fourth;
					Success = true;
				}
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF8_String_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Success{false};
	std::string Value;
	
	if(Buffer.Has(Length, 0) == true)
	{
		auto End{Buffer.GetPosition() + Inspection::Length(Length, 0)};
		
		while(Buffer.GetPosition() < End)
		{
			auto Character{Get_UTF8_Character(Buffer)};
			
			if(Character->GetSuccess() == true)
			{
				Value += std::experimental::any_cast< std::string >(Character->GetAny());
			}
			else
			{
				break;
			}
		}
		if(Buffer.GetPosition() == End)
		{
			Success = true;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}
