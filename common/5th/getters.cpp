#include <bitset>
#include <sstream>
#include <vector>

#include "../guid.h"
#include "../helper.h"
#include "../string_cast.h"
#include "buffer.h"
#include "getters.h"

using namespace std::string_literals;

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_Alphabetical(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto Character{Buffer.Get8Bits()};
		
		if(Is_ASCII_Character_Alphabetical(Character) == true)
		{
			Result->GetValue()->SetAny(Character);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto Character{Buffer.Get8Bits()};
		
		if((Is_ASCII_Character_Alphabetical(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true) || (Is_ASCII_Character_Space(Character) == true))
		{
			Result->GetValue()->SetAny(Character);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetical_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(Length, 0) == true)
	{
		Result->SetSuccess(true);
		
		auto CharacterIndex{0ull};
		
		while(CharacterIndex < Length)
		{
			auto  CharacterResult{Get_ASCII_Character_Alphabetical(Buffer)};
			
			Value << std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny());
			if(CharacterResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
			++CharacterIndex;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(Length, 0) == true)
	{
		Result->SetSuccess(true);
		
		auto CharacterIndex{0ull};
		
		while(CharacterIndex < Length)
		{
			auto CharacterResult{Get_ASCII_Character_AlphaNumericOrSpace(Buffer)};
			
			Value << std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny());
			if(CharacterResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
			++CharacterIndex;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetical_EndedByTemplateLength(Inspection::Buffer & Buffer, const std::string & TemplateString)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(TemplateString.length(), 0) == true)
	{
		Result->SetSuccess(true);
		for(auto TemplateCharacter : TemplateString)
		{
			auto BufferCharacter{Buffer.Get8Bits()};
			
			Value << BufferCharacter;
			if((TemplateCharacter != BufferCharacter) || (Is_ASCII_Character_Alphabetical(BufferCharacter) == false))
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if((Length.GetBits() == 0) && (Buffer.Has(Length) == true))
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("printables only"s);
		Result->SetSuccess(true);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		std::stringstream Value;
		
		while(Buffer.GetPosition() < Boundary)
		{
			auto Character{Buffer.Get8Bits()};
			
			if(Is_ASCII_Character_Printable(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		if(Buffer.GetPosition() == Boundary)
		{
			Result->GetValue()->AppendTag("ended by length"s);
			Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		Result->GetValue()->SetAny(Value.str());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_Set_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->SetSuccess(true);
		
		auto Boundary{Buffer.GetPosition() + Length};
		
		while(Buffer.GetPosition() < Boundary)
		{
			if(Buffer.Get1Bits() == 0x00)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("set data"s);
		Result->GetValue()->AppendTag(to_string_cast(Length) + " bytes and bits"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_SetOrUnset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->SetSuccess(true);
		
		auto Boundary{Buffer.GetPosition() + Length};
		
		while(Buffer.GetPosition() < Boundary)
		{
			if(Buffer.GetPosition().GetBits() == 0)
			{
				Buffer.Get8Bits();
			}
			else
			{
				Buffer.Get1Bits();
			}
		}
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("any data"s);
		Result->GetValue()->AppendTag(to_string_cast(Length) + " bytes and bits"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_Unset_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->SetSuccess(true);
		
		auto Boundary{Buffer.GetPosition() + Length};
		
		while(Buffer.GetPosition() < Boundary)
		{
			if(Buffer.Get1Bits() == 0x01)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("unset data"s);
		Result->GetValue()->AppendTag(to_string_cast(Length) + " bytes and bits"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(true, Buffer)};
	
	Result->GetValue()->AppendTag("ASCII printable"s);
	if((Length.GetBits() == 0) && (Buffer.Has(Length) == true))
	{
		auto Boundary{Buffer.GetPosition() + Length};
		std::stringstream Value;
		
		while(Buffer.GetPosition() < Boundary)
		{
			auto Character{Buffer.Get8Bits()};
			
			if(Is_ASCII_Character_Printable(Character) == true)
			{
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by invalid character '" + to_string_cast(Character) + '\'');
				Buffer.SetPosition(Buffer.GetPosition() - 1ull);
				
				break;
			}
		}
		if(Buffer.GetPosition() == Boundary)
		{
			Result->GetValue()->AppendTag("ended by boundary"s);
		}
		
		auto String{Value.str()};
		
		Result->GetValue()->SetAny(String);
		Result->GetValue()->AppendTag(to_string_cast(String.length()) + " characters long");
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_8Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 8) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 8 > Value;
		auto Byte1{Buffer.Get8Bits()};
		
		Value[0] = (Byte1 & 0x01) == 0x01;
		Value[1] = (Byte1 & 0x02) == 0x02;
		Value[2] = (Byte1 & 0x04) == 0x04;
		Value[3] = (Byte1 & 0x08) == 0x08;
		Value[4] = (Byte1 & 0x10) == 0x10;
		Value[5] = (Byte1 & 0x20) == 0x20;
		Value[6] = (Byte1 & 0x40) == 0x40;
		Value[7] = (Byte1 & 0x80) == 0x80;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("8bit"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 16) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 16 > Value;
		auto Byte1{Buffer.Get8Bits()};
		
		Value[0] = (Byte1 & 0x01) == 0x01;
		Value[1] = (Byte1 & 0x02) == 0x02;
		Value[2] = (Byte1 & 0x04) == 0x04;
		Value[3] = (Byte1 & 0x08) == 0x08;
		Value[4] = (Byte1 & 0x10) == 0x10;
		Value[5] = (Byte1 & 0x20) == 0x20;
		Value[6] = (Byte1 & 0x40) == 0x40;
		Value[7] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Buffer.Get8Bits()};
		
		Value[8] = (Byte2 & 0x01) == 0x01;
		Value[9] = (Byte2 & 0x02) == 0x02;
		Value[10] = (Byte2 & 0x04) == 0x04;
		Value[11] = (Byte2 & 0x08) == 0x08;
		Value[12] = (Byte2 & 0x10) == 0x10;
		Value[13] = (Byte2 & 0x20) == 0x20;
		Value[14] = (Byte2 & 0x40) == 0x40;
		Value[15] = (Byte2 & 0x80) == 0x80;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 32 > Value;
		auto Byte1{Buffer.Get8Bits()};
		
		Value[0] = (Byte1 & 0x01) == 0x01;
		Value[1] = (Byte1 & 0x02) == 0x02;
		Value[2] = (Byte1 & 0x04) == 0x04;
		Value[3] = (Byte1 & 0x08) == 0x08;
		Value[4] = (Byte1 & 0x10) == 0x10;
		Value[5] = (Byte1 & 0x20) == 0x20;
		Value[6] = (Byte1 & 0x40) == 0x40;
		Value[7] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Buffer.Get8Bits()};
		
		Value[8] = (Byte2 & 0x01) == 0x01;
		Value[9] = (Byte2 & 0x02) == 0x02;
		Value[10] = (Byte2 & 0x04) == 0x04;
		Value[11] = (Byte2 & 0x08) == 0x08;
		Value[12] = (Byte2 & 0x10) == 0x10;
		Value[13] = (Byte2 & 0x20) == 0x20;
		Value[14] = (Byte2 & 0x40) == 0x40;
		Value[15] = (Byte2 & 0x80) == 0x80;
		
		auto Byte3{Buffer.Get8Bits()};
		
		Value[16] = (Byte3 & 0x01) == 0x01;
		Value[17] = (Byte3 & 0x02) == 0x02;
		Value[18] = (Byte3 & 0x04) == 0x04;
		Value[19] = (Byte3 & 0x08) == 0x08;
		Value[20] = (Byte3 & 0x10) == 0x10;
		Value[21] = (Byte3 & 0x20) == 0x20;
		Value[22] = (Byte3 & 0x40) == 0x40;
		Value[23] = (Byte3 & 0x80) == 0x80;
		
		auto Byte4{Buffer.Get8Bits()};
		
		Value[24] = (Byte4 & 0x01) == 0x01;
		Value[25] = (Byte4 & 0x02) == 0x02;
		Value[26] = (Byte4 & 0x04) == 0x04;
		Value[27] = (Byte4 & 0x08) == 0x08;
		Value[28] = (Byte4 & 0x10) == 0x10;
		Value[29] = (Byte4 & 0x20) == 0x20;
		Value[30] = (Byte4 & 0x40) == 0x40;
		Value[31] = (Byte4 & 0x80) == 0x80;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Boolean_1Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 1) == true)
	{
		Result->GetValue()->SetAny((0x01 & Buffer.Get1Bits()) == 0x01);
		Result->GetValue()->AppendTag("boolean"s);
		Result->GetValue()->AppendTag("1bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		auto Boundary{Buffer.GetPosition() + Length};
		std::vector< std::uint8_t > Value;
		
		while(Buffer.GetPosition() < Boundary)
		{
			Value.push_back(Buffer.Get8Bits());
		}
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("buffer"s);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("8bit values"s);
		Result->GetValue()->AppendTag(to_string_cast(Length) + " bytes and bits");
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto Data{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
	
	if(Data->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::vector< std::uint8_t > & DataValue{std::experimental::any_cast< const std::vector< std::uint8_t > & >(Data->GetAny())};
		
		for(auto Byte : DataValue)
		{
			if(Byte != 0x00)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		Result->SetValue(Data->GetValue());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_GUID_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(16ull, 0) == true)
	{
		Result->GetValue()->AppendTag("guid"s);
		Result->GetValue()->AppendTag("binary"s);
		Result->GetValue()->AppendTag("little endian"s);
		
		GUID Value;
		
		Value.Data1 = static_cast< std::uint32_t >(Buffer.Get8Bits()) + (static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8) + (static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16) + (static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24);
		Value.Data2 = static_cast< std::uint32_t >(Buffer.Get8Bits()) + (static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8);
		Value.Data3 = static_cast< std::uint32_t >(Buffer.Get8Bits()) + (static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8);
		Value.Data4[0] = Buffer.Get8Bits();
		Value.Data4[1] = Buffer.Get8Bits();
		Value.Data4[2] = Buffer.Get8Bits();
		Value.Data4[3] = Buffer.Get8Bits();
		Value.Data4[4] = Buffer.Get8Bits();
		Value.Data4[5] = Buffer.Get8Bits();
		Value.Data4[6] = Buffer.Get8Bits();
		Value.Data4[7] = Buffer.Get8Bits();
		Result->GetValue()->SetAny(Value);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Microsoft_WaveFormat_FormatTag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto FormatTagResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FormatTagResult->GetValue());
	if(FormatTagResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto FormatTag{std::experimental::any_cast< std::uint16_t >(FormatTagResult->GetAny())};
		
		switch(FormatTag)
		{
		case 0x0000:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_UNKNOWN"));
				Result->GetValue()->Append("Description", std::string("Unknown or invalid format tag"));
				
				break;
			}
		case 0x0001:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_PCM"));
				Result->GetValue()->Append("Description", std::string("Pulse Code Modulation"));
				
				break;
			}
		case 0x0002:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_ADPCM"));
				Result->GetValue()->Append("Description", std::string("Microsoft Adaptive Differental PCM"));
				
				break;
			}
		case 0x0003:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_IEEE_FLOAT"));
				Result->GetValue()->Append("Description", std::string("32-bit floating-point"));
				
				break;
			}
		case 0x0055:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_MPEGLAYER3"));
				Result->GetValue()->Append("Description", std::string("ISO/MPEG Layer3"));
				
				break;
			}
		case 0x0092:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_DOLBY_AC3_SPDIF"));
				Result->GetValue()->Append("Description", std::string("Dolby Audio Codec 3 over S/PDIF"));
				
				break;
			}
		case 0x0161:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO2"));
				Result->GetValue()->Append("Description", std::string("Windows Media Audio Standard (Versions 7, 8, and 9 Series)"));
				
				break;
			}
		case 0x0162:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO3"));
				Result->GetValue()->Append("Description", std::string("Windows Media Audio Professional (9 Series)"));
				
				break;
			}
		case 0x0163:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_WMAUDIO_LOSSLESS"));
				Result->GetValue()->Append("Description", std::string("Windows Media Audio Lossless (9 Series)"));
				
				break;
			}
		case 0x0164:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_WMASPDIF"));
				Result->GetValue()->Append("Description", std::string("Windows Media Audio over S/PDIF"));
				
				break;
			}
		case 0xFFFE:
			{
				Result->GetValue()->Append("ConstantName", std::string("WAVE_FORMAT_EXTENSIBLE"));
				Result->GetValue()->Append("Description", std::string("All new audio formats"));
				
				break;
			}
		default:
			{
				Result->GetValue()->Append("ConstantName", std::string("<no interpretation>"));
				Result->GetValue()->Append("Description", std::string("<no interpretation>"));
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("little endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_1Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 1) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get1Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("1bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_2Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 2) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get2Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("2bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_3Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 3) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get3Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("3bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_4Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 4) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get4Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("4bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_5Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 5) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get5Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("5bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_6Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 6) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get6Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("6bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 7) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get7Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("7bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 8) == true)
	{
		Result->GetValue()->SetAny(Buffer.Get8Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("8bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 16) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 16) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits()) << 8;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("little endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 20) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get4Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("20bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 24) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("24bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 32) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 24;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("little endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 36) == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Buffer.Get4Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("36bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_64Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 64) == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 56;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 48;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 40;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("64bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(0ull, 64) == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits());
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 40;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 48;
		Value |= static_cast< std::uint64_t >(Buffer.Get8Bits()) << 56;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("64bit"s);
		Result->GetValue()->AppendTag("little endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF8_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		std::string Value;
		
		Value += First;
		if((First & 0x80) == 0x00)
		{
			Result->GetValue()->SetAny(Value);
			Result->SetSuccess(true);
		}
		else if((First & 0xe0) == 0xc0)
		{
			if(Buffer.Has(1ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				
				if((Second & 0xc0) == 0x80)
				{
					Value += Second;
					Result->GetValue()->SetAny(Value);
					Result->SetSuccess(true);
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
					Result->GetValue()->SetAny(Value);
					Result->SetSuccess(true);
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
					Result->GetValue()->SetAny(Value);
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF8_String_EndedByBoundary(Inspection::Buffer & Buffer, const Inspection::Length & Boundary)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(Boundary) == true)
	{
		auto End{Buffer.GetPosition() + Boundary};
		std::string Value;
		
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
			Result->GetValue()->SetAny(Value);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto CodePointResult{Get_UTF16LE_CodePoint(Buffer)};
	
	if(CodePointResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("CodePoint", CodePointResult->GetValue());
		Result->GetValue()->SetAny(Get_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_CodePoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto FirstCodeUnitResult{Get_UTF16LE_CodeUnit(Buffer)};
	
	if(FirstCodeUnitResult->GetSuccess() == true)
	{
		auto FirstCodeUnit{std::experimental::any_cast< std::uint16_t >(FirstCodeUnitResult->GetAny())};
		
		if((FirstCodeUnit < 0xd800) || (FirstCodeUnit >= 0xe000))
		{
			std::uint32_t Value{FirstCodeUnit};
			
			Result->GetValue()->SetAny(Value);
			Result->SetSuccess(true);
		}
		else if((FirstCodeUnit >= 0xd800) && (FirstCodeUnit < 0xdc00))
		{
			auto SecondCodeUnitResult{Get_UTF16LE_CodeUnit(Buffer)};
			
			if(SecondCodeUnitResult->GetSuccess() == true)
			{
				auto SecondCodeUnit{std::experimental::any_cast< std::uint16_t >(SecondCodeUnitResult->GetAny())};
				
				if((SecondCodeUnit >= 0xdc00) && (SecondCodeUnit < 0xe000))
				{
					std::uint32_t Value{(static_cast< std::uint32_t >(FirstCodeUnit - 0xd800) << 10) | static_cast< std::uint32_t >(SecondCodeUnit - 0xdc00)};
					
					Result->GetValue()->SetAny(Value);
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_CodeUnit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		Result->GetValue()->SetAny(static_cast< std::uint16_t >(static_cast< std::uint16_t >(Buffer.Get8Bits()) | (static_cast< std::uint16_t >(Buffer.Get8Bits()) << 8)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBytes() % 2 == 0);
	assert(Length.GetBits() == 0);
	
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("little endian"s);
	Result->GetValue()->AppendTag("without byte order mark"s);
	
	auto NumberOfCodePoints{0ul};
	
	while(Buffer.GetPosition() < Boundary)
	{
		auto CharacterResult{Get_UTF16LE_Character(Buffer)};
		
		if((Buffer.GetPosition() <= Boundary) && (CharacterResult->GetSuccess() == true))
		{
			auto CodePoint{std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint"))};
			
			if(CodePoint == 0x00000000)
			{
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by termination and boundary"s);
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AppendTag("empty"s);
					}
					Result->SetSuccess(true);
				}
				else
				{
					Result->GetValue()->AppendTag("ended by termination"s);
					
					break;
				}
			}
			else
			{
				NumberOfCodePoints += 1;
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by boundary"s);
				}
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
			}
		}
		else
		{
			break;
		}
	}
	Result->GetValue()->AppendTag(to_string_cast(NumberOfCodePoints) + " code points");
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBytes() % 2 == 0);
	assert(Length.GetBits() == 0);
	
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	auto NumberOfCodePoints{0ul};
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("little endian"s);
	Result->GetValue()->AppendTag("without byte order mark"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by boundary"s);
		Result->GetValue()->AppendTag("empty"s);
		Result->SetSuccess(true);
	}
	else
	{
		while(Buffer.GetPosition() < Boundary)
		{
			auto CharacterResult{Get_UTF16LE_Character(Buffer)};
			
			if((Buffer.GetPosition() <= Boundary) && (CharacterResult->GetSuccess() == true))
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint"))};
				
				if(CodePoint == 0x00000000)
				{
					if(Buffer.GetPosition() == Boundary)
					{
						Result->GetValue()->AppendTag("ended by termination and boundary"s);
					}
					else
					{
						Result->GetValue()->AppendTag("ended by termination"s);
					}
					Result->SetSuccess(true);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					if(Buffer.GetPosition() == Boundary)
					{
						Result->GetValue()->AppendTag("ended by boundary"s);
						Result->SetSuccess(true);
					}
					Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->AppendTag(to_string_cast(NumberOfCodePoints) + " code points");
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Buffer & Buffer, std::uint64_t NumberOfCodePoints)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	std::stringstream Value;
	std::uint64_t CodePointIndex{0ull};
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("little endian"s);
	Result->GetValue()->AppendTag("without byte order mark"s);
	while(true)
	{
		auto CharacterResult{Get_UTF16LE_Character(Buffer)};
		
		if(CharacterResult->GetSuccess() == true)
		{
			++CodePointIndex;
			if(std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint")) == 0x00000000)
			{
				if(CodePointIndex == NumberOfCodePoints)
				{
					Result->GetValue()->AppendTag("ended by termination and number of code points"s);
					Result->SetSuccess(true);
				}
				
				break;
			}
			else
			{
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
			}
		}
		else
		{
			break;
		}
	}
	Result->GetValue()->AppendTag(to_string_cast(NumberOfCodePoints) + " code points");
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	assert(Buffer.GetBitstreamType() == Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto CommentHeaderWithoutFramingFlag{Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer)};
	
	if(CommentHeaderWithoutFramingFlag->GetSuccess() == true)
	{
		Result->GetValue()->Append(CommentHeaderWithoutFramingFlag->GetValue()->GetValues());
		
		auto FramingFlagResult{Get_Boolean_1Bit(Buffer)};
		
		if(FramingFlagResult->GetSuccess() == true)
		{
			auto FramingFlag{std::experimental::any_cast< bool >(FramingFlagResult->GetAny())};
			
			Result->GetValue()->Append("FramingFlag", FramingFlagResult->GetValue());
			Result->SetSuccess(FramingFlag);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto UserCommentLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Length", UserCommentLengthResult->GetValue());
		
		auto UserCommentLength{std::experimental::any_cast< std::uint32_t >(UserCommentLengthResult->GetAny())};
		auto UserCommentResult{Get_UTF8_String_EndedByBoundary(Buffer, UserCommentLength)};
		
		if(UserCommentResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("String", UserCommentResult->GetValue());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto UserCommentListLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(UserCommentListLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("Length", UserCommentListLengthResult->GetValue());
		Result->SetSuccess(true);
		
		auto UserCommentListLength{std::experimental::any_cast< std::uint32_t >(UserCommentListLengthResult->GetAny())};
		
		for(std::uint32_t Index = 0ul; Index < UserCommentListLength; ++Index)
		{
			auto UserCommentResult{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			
			if(UserCommentResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("UserComment", UserCommentResult->GetValue());
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(false, Buffer)};
	auto VendorLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	if(VendorLengthResult->GetSuccess() == true)
	{
		Result->GetValue()->Append("VendorLength", VendorLengthResult->GetValue());
		
		auto VendorLength{std::experimental::any_cast< std::uint32_t >(VendorLengthResult->GetAny())};
		auto VendorResult{Get_UTF8_String_EndedByBoundary(Buffer, VendorLength)};
		
		if(VendorResult->GetSuccess() == true)
		{
			Result->GetValue()->Append("Vendor", VendorResult->GetValue());
			
			auto UserCommentListResult{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
			
			if(UserCommentListResult->GetSuccess() == true)
			{
				Result->GetValue()->Append("UserCommentList", UserCommentListResult->GetValue());
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}
