#include <bitset>
#include <sstream>
#include <vector>

#include "buffer.h"
#include "getters.h"
#include "guid.h"
#include "helper.h"
#include "id3_helper.h"
#include "not_implemented_exception.h"
#include "string_cast.h"
#include "unknown_value_exception.h"

using namespace std::string_literals;

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto APETagsHeaderResult{Get_APE_Tags_HeaderOrFooter(Buffer)};
	
	Result->GetValue()->AppendValue("APETagsHeader", APETagsHeaderResult->GetValue());
	if(APETagsHeaderResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto ItemCount{std::experimental::any_cast< std::uint32_t >(APETagsHeaderResult->GetAny("ItemCount"))};
		
		for(auto ItemIndex = 0ul; ItemIndex < ItemCount; ++ItemIndex)
		{
			auto APETagsItemResult{Get_APE_Tags_Item(Buffer)};
			
			Result->GetValue()->AppendValue("APETagsItem", APETagsItemResult->GetValue());
			if(APETagsItemResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		
		auto APETagsFooterResult{Get_APE_Tags_HeaderOrFooter(Buffer)};
		
		Result->GetValue()->AppendValue("APETagsFooter", APETagsFooterResult->GetValue());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Start{Buffer.GetPosition()};
	auto TagsFlagsResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(TagsFlagsResult->GetValue());
	if(TagsFlagsResult->GetSuccess() == true)
	{
		const std::bitset<32> & TagsFlags{std::experimental::any_cast< const std::bitset<32> & >(TagsFlagsResult->GetAny())};
		auto FlagValue{Result->GetValue()->AppendValue("TagOrItemIsReadOnly", TagsFlags[0])};
		
		FlagValue->AppendTag("bit index", "0"s);
		if((TagsFlags[1] == false) && (TagsFlags[2] == false))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(0));
			FlagValue->AppendTag("interpretation", "Item contains text information coded in UTF-8"s);
		}
		else if((TagsFlags[1] == true) && (TagsFlags[2] == false))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(1));
			FlagValue->AppendTag("interpretation", "Item contains binary information"s);
		}
		else if((TagsFlags[1] == false) && (TagsFlags[2] == true))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(2));
			FlagValue->AppendTag("interpretation", "Item is a locator of external stored information"s);
		}
		else if((TagsFlags[1] == true) && (TagsFlags[2] == true))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(3));
			FlagValue->AppendTag("interpretation", "<reserved>"s);
		}
		FlagValue->AppendTag("bit indices", "1 to 2"s);
		
		auto AllFalse{true};
		
		for(auto BitIndex = 3; BitIndex < 29; ++BitIndex)
		{
			if(TagsFlags[BitIndex] == true)
			{
				AllFalse = false;
				
				break;
			}
		}
		if(AllFalse == true)
		{
			FlagValue = Result->GetValue()->AppendValue("Undefined", false);
			FlagValue->AppendTag("bit indices", "3 to 28"s);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "All bits 3 to 28 must be unset.");
		}
		FlagValue = Result->GetValue()->AppendValue("Type", TagsFlags[29]);
		if(TagsFlags[29] == true)
		{
			FlagValue->AppendTag("interpretation", "This is the header, not the footer"s);
		}
		else
		{
			FlagValue->AppendTag("interpretation", "This is the footer, not the header"s);
		}
		FlagValue->AppendTag("bit index", 29);
		FlagValue = Result->GetValue()->AppendValue("TagContainsAFooter", TagsFlags[30]);
		FlagValue->AppendTag("bit index", 30);
		FlagValue = Result->GetValue()->AppendValue("TagContainsAHeader", TagsFlags[31]);
		FlagValue->AppendTag("bit index", 31);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PreambleResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "APETAGEX")};
	
	Result->GetValue()->AppendValue("Preamble", PreambleResult->GetValue());
	if(PreambleResult->GetSuccess() == true)
	{
		auto VersionNumberResult{Get_APE_Tags_HeaderOrFooter_VersionNumber(Buffer)};
		
		Result->GetValue()->AppendValue("VersionNumber", VersionNumberResult->GetValue());
		if(VersionNumberResult->GetSuccess() == true)
		{
			auto TagSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("TagSize", TagSizeResult->GetValue());
			if(TagSizeResult->GetSuccess() == true)
			{
				auto ItemCountResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("ItemCount", ItemCountResult->GetValue());
				if(ItemCountResult->GetSuccess() == true)
				{
					auto TagsFlagsResult{Get_APE_Tags_Flags(Buffer)};
					
					Result->GetValue()->AppendValue("TagsFlags", TagsFlagsResult->GetValue());
					if(TagsFlagsResult->GetSuccess() == true)
					{
						auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(8, 0))};
						
						Result->GetValue()->AppendValue("Reserved", ReservedResult->GetValue());
						Result->SetSuccess(ReservedResult->GetSuccess());
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto VersionNumberResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(VersionNumberResult->GetValue());
	if(VersionNumberResult->GetSuccess() == true)
	{
		auto VersionNumber{std::experimental::any_cast< std::uint32_t >(VersionNumberResult->GetAny())};
		
		if(VersionNumber == 1000)
		{
			Result->GetValue()->AppendTag("interpretation", "1.000 (old)"s);
			Result->SetSuccess(true);
		}
		else if(VersionNumber == 2000)
		{
			Result->GetValue()->AppendTag("interpretation", "2.000 (new)"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "<unknown>"s);
			Result->GetValue()->AppendTag("error", "Unknown version number " + to_string_cast(VersionNumber) + ".");
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_Item(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ItemValueSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("ItemValueSize", ItemValueSizeResult->GetValue());
	if(ItemValueSizeResult->GetSuccess() == true)
	{
		auto ItemFlagsResult{Get_APE_Tags_Flags(Buffer)};
		
		Result->GetValue()->AppendValue("ItemFlags", ItemFlagsResult->GetValue());
		if(ItemFlagsResult->GetSuccess() == true)
		{
			auto ItemKeyResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
			
			Result->GetValue()->AppendValue("ItemKey", ItemKeyResult->GetValue());
			if(ItemKeyResult->GetSuccess() == true)
			{
				auto ItemValueType{std::experimental::any_cast< std::uint8_t >(ItemFlagsResult->GetAny("ItemValueType"))};
				
				if(ItemValueType == 0)
				{
					auto ItemValueSize{std::experimental::any_cast< std::uint32_t >(ItemValueSizeResult->GetAny())};
					auto ItemValueResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, Inspection::Length(ItemValueSize, 0))};
					
					Result->GetValue()->AppendValue("ItemValue", ItemValueResult->GetValue());
					Result->SetSuccess(ItemValueResult->GetSuccess());
				}
				else
				{
					throw Inspection::NotImplementedException("Can only interpret UTF-8 item values.");
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_Alphabetical(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_AlphaNumeric(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto Character{Buffer.Get8Bits()};
		
		if((Is_ASCII_Character_Alphabetical(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true))
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetical_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(Length) == true)
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("alphabetical"s);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto  CharacterResult{Get_ASCII_Character_Alphabetical(Buffer)};
			
			if(CharacterResult->GetSuccess() == false)
			{
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny());
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					Result->SetSuccess(true);
					
					break;
				}
			}
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetical_EndedByTemplateLength(Inspection::Buffer & Buffer, const std::string & TemplateString)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(TemplateString.length(), 0) == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		Result->SetSuccess(true);
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("alphabetical"s);
		for(auto TemplateCharacter : TemplateString)
		{
			auto CharacterResult{Get_ASCII_Character_Alphabetical(Buffer)};
			
			if(CharacterResult->GetSuccess() == true)
			{
				auto Character{std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny())};
				
				NumberOfCharacters += 1;
				Value << Character;
				if(TemplateCharacter != Character)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		if(Result->GetSuccess() == true)
		{
			Result->GetValue()->AppendTag("ended by template"s);
		}
		else
		{
			Result->GetValue()->AppendTag("ended by error"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters"s);
		Result->GetValue()->SetAny(Value.str());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(Length) == true)
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("alphanumeric"s);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto  CharacterResult{Get_ASCII_Character_AlphaNumeric(Buffer)};
			
			if(CharacterResult->GetSuccess() == false)
			{
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny());
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					Result->SetSuccess(true);
					
					break;
				}
			}
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Inspection::Buffer & Buffer, const std::string & TemplateString)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(TemplateString.length(), 0) == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		Result->SetSuccess(true);
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("alpha numeric"s);
		for(auto TemplateCharacter : TemplateString)
		{
			auto CharacterResult{Get_ASCII_Character_AlphaNumeric(Buffer)};
			
			if(CharacterResult->GetSuccess() == true)
			{
				auto Character{std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny())};
				
				NumberOfCharacters += 1;
				Value << Character;
				if(TemplateCharacter != Character)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		if(Result->GetSuccess() == true)
		{
			Result->GetValue()->AppendTag("ended by template"s);
		}
		else
		{
			Result->GetValue()->AppendTag("ended by error"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters"s);
		Result->GetValue()->SetAny(Value.str());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	if(Buffer.Has(Length) == true)
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("alphanumeric or space"s);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		
		while(Buffer.GetPosition() < Boundary)
		{
			auto CharacterResult{Get_ASCII_Character_AlphaNumericOrSpace(Buffer)};
			
			if(CharacterResult->GetSuccess() == false)
			{
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny());
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					Result->SetSuccess(true);
				}
			}
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("ASCII"s);
		Result->GetValue()->AppendTag("printables only"s);
		Result->SetSuccess(true);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		std::stringstream Value;
		
		while(true)
		{
			auto Character{Buffer.Get8Bits()};
			
			if(Is_ASCII_Character_Printable(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Character;
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					
					break;
				}
			}
			else
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("printables only"s);
	Result->SetSuccess(true);
	
	auto NumberOfCharacters{0ul};
	std::stringstream Value;
	
	while(Buffer.Has(1ull, 0) == true)
	{
		auto Character{Buffer.Get8Bits()};
		
		if(Character == 0x00)
		{
			Result->GetValue()->AppendTag("ended by termination"s);
			Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
			
			break;
		}
		else if(Is_ASCII_Character_Printable(Character) == true)
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
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_Set_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->SetSuccess(true);
		Buffer.SetPosition(Buffer.GetPosition() + Length);
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	assert(Length.GetBits() == 0);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("ASCII printable"s);
	if(Buffer.Has(Length) == true)
	{
		Result->SetSuccess(true);
		
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

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 4) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 4 > Value;
		auto Byte1{Buffer.Get4Bits()};
		
		Value[0] = (Byte1 & 0x08) == 0x08;
		Value[1] = (Byte1 & 0x04) == 0x04;
		Value[2] = (Byte1 & 0x02) == 0x02;
		Value[3] = (Byte1 & 0x01) == 0x01;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("4bit"s);
		Result->GetValue()->AppendTag("most significant bit first"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_8Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_16Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 16) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 16 > Value;
		auto Byte1{Buffer.Get8Bits()};
		
		Value[8] = (Byte1 & 0x01) == 0x01;
		Value[9] = (Byte1 & 0x02) == 0x02;
		Value[10] = (Byte1 & 0x04) == 0x04;
		Value[11] = (Byte1 & 0x08) == 0x08;
		Value[12] = (Byte1 & 0x10) == 0x10;
		Value[13] = (Byte1 & 0x20) == 0x20;
		Value[14] = (Byte1 & 0x40) == 0x40;
		Value[15] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Buffer.Get8Bits()};
		
		Value[0] = (Byte2 & 0x01) == 0x01;
		Value[1] = (Byte2 & 0x02) == 0x02;
		Value[2] = (Byte2 & 0x04) == 0x04;
		Value[3] = (Byte2 & 0x08) == 0x08;
		Value[4] = (Byte2 & 0x10) == 0x10;
		Value[5] = (Byte2 & 0x20) == 0x20;
		Value[6] = (Byte2 & 0x40) == 0x40;
		Value[7] = (Byte2 & 0x80) == 0x80;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_1_Tag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TagIdentifierResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "TAG")};
	
	Result->GetValue()->AppendValue("Identifier", TagIdentifierResult->GetValue());
	if(TagIdentifierResult->GetSuccess() == true)
	{
		auto TitelResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
		
		Result->GetValue()->AppendValue("Title", TitelResult->GetValue());
		if(TitelResult->GetSuccess() == true)
		{
			auto ArtistResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
			
			Result->GetValue()->AppendValue("Artist", ArtistResult->GetValue());
			if(ArtistResult->GetSuccess() == true)
			{
				auto AlbumResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
				
				Result->GetValue()->AppendValue("Album", AlbumResult->GetValue());
				if(AlbumResult->GetSuccess() == true)
				{
					auto YearResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(4ull, 0))};
					
					Result->GetValue()->AppendValue("Year", YearResult->GetValue());
					if(YearResult->GetSuccess() == true)
					{
						auto StartOfComment{Buffer.GetPosition()};
						auto CommentResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length(30ull, 0))};
						auto Continue{false};
						
						if(CommentResult->GetSuccess() == true)
						{
							Result->GetValue()->AppendValue("Comment", CommentResult->GetValue());
							Continue = true;
						}
						else
						{
							Buffer.SetPosition(StartOfComment);
							CommentResult = Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Buffer, Inspection::Length(29ull, 0));
							Result->GetValue()->AppendValue("Comment", CommentResult->GetValue());
							if(CommentResult->GetSuccess() == true)
							{
								auto AlbumTrackResult{Get_UnsignedInteger_8Bit(Buffer)};
								
								Result->GetValue()->AppendValue("AlbumTrack", AlbumTrackResult->GetValue());
								Continue = AlbumTrackResult->GetSuccess();
							}
						}
						if(Continue == true)
						{
							auto GenreResult{Get_UnsignedInteger_8Bit(Buffer)};
							
							Result->GetValue()->AppendValue("Genre", GenreResult->GetValue());
							if(GenreResult->GetSuccess() == true)
							{
								Result->SetSuccess(true);
								
								auto GenreNumber{std::experimental::any_cast< std::uint8_t >(GenreResult->GetAny())};
								
								try
								{
									auto Genre{Inspection::Get_ID3_1_Genre(GenreNumber)};
									
									Result->GetValue("Genre")->PrependTag("interpretation", Genre);
									Result->GetValue("Genre")->PrependTag("standard", "ID3v1"s);
								}
								catch(Inspection::UnknownValueException & Exception)
								{
									try
									{
										auto Genre{Inspection::Get_ID3_1_Winamp_Genre(GenreNumber)};
										
										Result->GetValue("Genre")->PrependTag("interpretation", Genre);
										Result->GetValue("Genre")->PrependTag("standard", "Winamp extension"s);
									}
									catch(Inspection::UnknownValueException & Exception)
									{
										Result->GetValue("Genre")->PrependTag("interpretation", "<unrecognized>"s);
									}
								}
							}
						}
						Result->SetSuccess(true);
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_2_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::unique_ptr< Inspection::Result > BodyResult;
		
		if(Identifier == "COM")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_COM(Buffer, Size);
		}
		else if(Identifier == "PIC")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_PIC(Buffer, Size);
		}
		else if((Identifier == "TAL") || (Identifier == "TCM") || (Identifier == "TCO") || (Identifier == "TCP") || (Identifier == "TEN") || (Identifier == "TP1") || (Identifier == "TP2") || (Identifier == "TPA") || (Identifier == "TRK") || (Identifier == "TT1") || (Identifier == "TT2") || (Identifier == "TYE"))
		{
			BodyResult = Get_ID3_2_2_Frame_Body_T__(Buffer, Size);
		}
		else if(Identifier == "UFI")
		{
			BodyResult = Get_ID3_2_2_Frame_Body_UFI(Buffer, Size);
		}
		if(BodyResult)
		{
			if(Start + Size > Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "Frame size is stated larger than the handled size."s);
			}
			else if(Start + Size < Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "Handled size is larger than the stated frame size."s);
			}
			Result->GetValue()->AppendValues(BodyResult->GetValue()->GetValues());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		else
		{
			Result->SetSuccess(false);
		}
		Buffer.SetPosition(Start + Size);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_COM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_2_Language(Buffer)};
		
		Result->GetValue()->AppendValue("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ShortContentDescriptionResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto ImageFormatResult{Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Buffer)};
		
		Result->GetValue()->AppendValue("ImageFormat", ImageFormatResult->GetValue());
		if(ImageFormatResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_2_Frame_Body_PIC_PictureType(Buffer)};
			
			Result->GetValue()->AppendValue("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->AppendValue("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ImageFormatResult{Get_ISO_IEC_8859_1_1998_String_EndedByLength(Buffer, Inspection::Length(3ull, 0))};
	
	Result->SetValue(ImageFormatResult->GetValue());
	if(ImageFormatResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & ImageFormat{std::experimental::any_cast< const std::string & >(ImageFormatResult->GetAny())};
		
		if(ImageFormat == "-->")
		{
			Result->GetValue()->PrependTag("mime-type", "application/x-url"s);
		}
		else if(ImageFormat == "PNG")
		{
			Result->GetValue()->PrependTag("mime-type", "image/png"s);
		}
		else if(ImageFormat == "JPG")
		{
			Result->GetValue()->PrependTag("mime-type", "image/jpeg"s);
		}
		else
		{
			Result->GetValue()->PrependTag("mime-type", "<unrecognized>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v2"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_T__(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_2_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Information", InformationResult->GetValue());
		Result->SetSuccess(InformationResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_UFI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ID3_2_2_Frame_Header_Identifier(Buffer)};
	
	Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		Result->SetSuccess(SizeResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(3ull, 0))};
	
	Result->SetValue(IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Get_ID3_2_2_FrameIdentifier_Interpretation(Identifier));
			Result->GetValue()->PrependTag("standard", "ID3 2.2"s);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Identifier == "TCP")
			{
				Result->GetValue()->PrependTag("interpretation", "Compilation"s);
				Result->GetValue()->PrependTag("standard", "<from the internet>"s);
				Result->GetValue()->PrependTag("error", "This frame is not officially defined for tag version 2.2 but has been seen used nonetheless."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Unknown frame identifier \"" + Identifier + "\".");
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_2_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("Frame", FrameResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(Start);
			
			auto PaddingResult{Get_Bits_Unset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("Padding", PaddingResult->GetValue());
			if(PaddingResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				Buffer.SetPosition(Boundary);
			}
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		Result->SetValue(FieldResult->GetValue());
		if(FieldResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag = Result->GetValue()->AppendValue("Compression", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 5; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_8859_1_1998_StringResult->GetSuccess());
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_8859_1_1998_StringResult->GetSuccess());
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		Result->SetSuccess(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_3_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > BodyHandler;
		
		if(Identifier == "APIC")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_APIC;
		}
		else if(Identifier == "COMM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_COMM;
		}
		else if(Identifier == "GEOB")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_GEOB;
		}
		else if(Identifier == "MCDI")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_MCDI;
		}
		else if(Identifier == "PCNT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_PCNT;
		}
		else if(Identifier == "POPM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_POPM;
		}
		else if(Identifier == "PRIV")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_PRIV;
		}
		else if(Identifier == "RGAD")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_RGAD;
		}
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCOP") || (Identifier == "TDAT") || (Identifier == "TDRC") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIME") || (Identifier == "TIT1") || (Identifier == "TIT2") || (Identifier == "TIT3") || (Identifier == "TLEN") || (Identifier == "TMED") || (Identifier == "TOAL") || (Identifier == "TOFN") || (Identifier == "TOPE") || (Identifier == "TOWN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPE3") || (Identifier == "TPE4") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TRDA") || (Identifier == "TSIZ") || (Identifier == "TSO2") || (Identifier == "TSOA") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TSST") || (Identifier == "TYER"))
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_T___;
		}
		else if(Identifier == "TCMP")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TCMP;
		}
		else if(Identifier == "TCON")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TCON;
		}
		else if(Identifier == "TFLT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TFLT;
		}
		else if(Identifier == "TLAN")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TLAN;
		}
		else if(Identifier == "TSRC")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TSRC;
		}
		else if(Identifier == "TXXX")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_TXXX;
		}
		else if(Identifier == "UFID")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_UFID;
		}
		else if(Identifier == "USLT")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_USLT;
		}
		else if(Identifier == "WCOM")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WOAF")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WOAR")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_W___;
		}
		else if(Identifier == "WXXX")
		{
			BodyHandler = Get_ID3_2_3_Frame_Body_WXXX;
		}
		if(BodyHandler != nullptr)
		{
			auto BodyResult{BodyHandler(Buffer, Size)};
			
			if(Size > BodyResult->GetLength())
			{
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated larger than the actually handled size."s);
			}
			else if(BodyResult->GetLength() < Size)
			{
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated smaller than the actually handled size."s);
			}
			Result->GetValue()->AppendValues(BodyResult->GetValue()->GetValues());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			auto BodyResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size)};
			
			Result->GetValue()->AppendValue("Data", BodyResult->GetValue());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		Buffer.SetPosition(Start + Size);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_3_Frame_Body_APIC_MIMEType(Buffer)};
		
		Result->GetValue()->AppendValue("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_3_Frame_Body_APIC_PictureType(Buffer)};
			
			Result->GetValue()->AppendValue("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->AppendValue("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	/// @todo As per [ID3 2.3.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	Result->SetValue(MIMETypeResult->GetValue());
	Result->SetSuccess(MIMETypeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v3"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto LanguageResult{Get_ID3_2_3_Language(Buffer)};
		
		Result->GetValue()->AppendValue("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto ShortContentDescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_GEOB(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Buffer)};
		
		Result->GetValue()->AppendValue("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto FileNameResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("FileName", FileNameResult->GetValue());
			if(FileNameResult->GetSuccess() == true)
			{
				auto ContentDescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->AppendValue("ContentDescription", ContentDescriptionResult->GetValue());
				if(ContentDescriptionResult->GetSuccess() == true)
				{
					auto EncapsulatedObjectResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->AppendValue("EncapsulatedObject", EncapsulatedObjectResult->GetValue());
					Result->SetSuccess(EncapsulatedObjectResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	Result->SetValue(MIMETypeResult->GetValue());
	if(MIMETypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
	}
	else
	{
		Result->GetValue()->PrependTag("error", "This field could not be interpreted as a terminated ASCII string of printable characters."s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TableOfContentsResult{Get_IEC_60908_1999_TableOfContents(Buffer)};
	
	if(TableOfContentsResult->GetSuccess() == true)
	{
		Result->SetValue(TableOfContentsResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Result->GetOffset());
		
		auto MCDIStringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Buffer, Length)};
		
		if(MCDIStringResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("String", MCDIStringResult->GetValue());
			Result->GetValue("String")->PrependTag("error", "The content of an \"MCDI\" frame should be a binary compact disc table of contents, but is a unicode string encoded with UCS-2 in little endian."s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_PCNT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
	{
		auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
		Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
		Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
	}
	else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
	{
		auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
		Result->SetSuccess(CounterResult->GetSuccess());
	}
	else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
	{
		auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
		Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EMailToUserResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("EMailToUser", EMailToUserResult->GetValue());
	if(EMailToUserResult->GetSuccess() == true)
	{
		auto RatingResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Rating", RatingResult->GetValue());
		if(RatingResult->GetSuccess() == true)
		{
			auto Rating{std::experimental::any_cast< std::uint8_t >(RatingResult->GetAny())};
			
			if(Rating == 0)
			{
				Result->GetValue("Rating")->PrependTag("standard", "ID3 2.3"s);
				Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
			}
			if(Buffer.GetPosition() == Boundary)
			{
				auto CounterValue{std::make_shared< Inspection::Value >()};
				
				CounterValue->SetName("Counter");
				CounterValue->AppendTag("omitted"s);
				Result->GetValue()->AppendValue(CounterValue);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
				Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
			{
				auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->SetSuccess(CounterResult->GetSuccess());
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_PRIV(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		const std::string & OwnerIdentifier{std::experimental::any_cast< const std::string & >(OwnerIdentifierResult->GetAny())};
		std::string PRIVDataName;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > PRIVDataHandler;
		
		if(OwnerIdentifier == "AverageLevel")
		{
			PRIVDataHandler = std::bind(Inspection::Get_UnsignedInteger_32Bit_LittleEndian, std::placeholders::_1);
			PRIVDataName = "AverageLevel";
		}
		else if(OwnerIdentifier == "PeakValue")
		{
			PRIVDataHandler = std::bind(Inspection::Get_UnsignedInteger_32Bit_LittleEndian, std::placeholders::_1);
			PRIVDataName = "PeakValue";
		}
		else if(OwnerIdentifier == "WM/MediaClassPrimaryID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "MediaClassPrimaryID";
		}
		else if(OwnerIdentifier == "WM/MediaClassSecondaryID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "MediaClassSecondaryID";
		}
		else if(OwnerIdentifier == "WM/Provider")
		{
			PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
			PRIVDataName = "Provider";
		}
		else if(OwnerIdentifier == "WM/UniqueFileIdentifier")
		{
			PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
			PRIVDataName = "UniqueFileIdentifier";
		}
		else if(OwnerIdentifier == "WM/WMCollectionGroupID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "CollectionGroupID";
		}
		else if(OwnerIdentifier == "WM/WMCollectionID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "CollectionID";
		}
		else if(OwnerIdentifier == "WM/WMContentID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ContentID";
		}
		else if(OwnerIdentifier == "ZuneAlbumArtistMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneAlbumMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneCollectionID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneAlbumArtistMediaID";
		}
		else if(OwnerIdentifier == "ZuneMediaID")
		{
			PRIVDataHandler = Get_ID3_GUID;
			PRIVDataName = "ZuneMediaID";
		}
		else
		{
			PRIVDataHandler = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
			PRIVDataName = "PrivateData";
		}
		
		auto PRIVDataResult{PRIVDataHandler(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue(PRIVDataName, PRIVDataResult->GetValue());
		Result->SetSuccess(PRIVDataResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_RGAD(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PeakAmplitudeResult{Get_ISO_IEC_IEEE_60559_2011_binary32(Buffer)};
	
	Result->GetValue()->AppendValue("PeakAmplitude", PeakAmplitudeResult->GetValue());
	if(PeakAmplitudeResult->GetSuccess() == true)
	{
		auto TrackReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment(Buffer)};
		
		Result->GetValue()->AppendValue("TrackReplayGainAdjustment", TrackReplayGainAdjustmentResult->GetValue());
		if(TrackReplayGainAdjustmentResult->GetSuccess() == true)
		{
			auto AlbumReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment(Buffer)};
			
			Result->GetValue()->AppendValue("AlbumReplayGainAdjustment", AlbumReplayGainAdjustmentResult->GetValue());
			Result->SetSuccess(AlbumReplayGainAdjustmentResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Information", InformationResult->GetValue());
		Result->SetSuccess(InformationResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCMP(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		if(Information == "1")
		{
			Result->GetValue("Information")->PrependTag("interpretation", "yes, this is part of a comilation"s);
		}
		else if(Information == "0")
		{
			Result->GetValue("Information")->PrependTag("interpretation", "no, this is not part of a compilation"s);
		}
		else
		{
			Result->GetValue("Information")->PrependTag("interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCON(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		auto Interpretation{GetContentTypeInterpretation2_3(Information)};
		
		if(std::get<0>(Interpretation) == true)
		{
			Result->GetValue("Information")->PrependTag("interpretation", std::get<1>(Interpretation));
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TFLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		std::string Interpretation;
		
		Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
		try
		{
			Interpretation = Get_ID3_2_3_FileType_Interpretation(Information);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Information == "/3")
			{
				Interpretation = "MPEG 1/2 layer III";
				Result->GetValue("Information")->PrependTag("error", "The file type could not be interpreted strictly according to the standard, but this seems plausible."s);
			}
			else
			{
				Interpretation = "unkown";
				Result->GetValue("Information")->PrependTag("error", "The file type could not be interpreted."s);
			}
		}
		Result->GetValue("Information")->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TLAN(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		try
		{
			auto Interpretation{Inspection::Get_LanguageName_From_ISO_639_2_1998_Code(Information)};
			
			Result->GetValue("Information")->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue("Information")->PrependTag("interpretation", Interpretation);
		}
		catch(...)
		{
			Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
			Result->GetValue("Information")->PrependTag("error", "The language frame needs to contain a three letter code from ISO 639-2:1998 (alpha-3)."s);
			Result->GetValue("Information")->PrependTag("interpretation", "<unkown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TSRC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameT___BodyResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
	
	Result->SetValue(FrameT___BodyResult->GetValue());
	if(FrameT___BodyResult->GetSuccess() == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(FrameT___BodyResult->GetValue("Information")->GetTagAny("string"))};
		
		if(Information.length() == 12)
		{
			Result->GetValue("Information")->PrependTag("standard", "ISRC Bulletin 2015/01"s);
			Result->GetValue("Information")->PrependTag("DesignationCode", Information.substr(7, 5));
			Result->GetValue("Information")->PrependTag("YearOfReference", Information.substr(5, 2));
			Result->GetValue("Information")->PrependTag("RegistrantCode", Information.substr(2, 3));
			
			std::string CountryCode{Information.substr(0, 2)};
			auto CountryCodeValue{Result->GetValue("Information")->PrependTag("CountryCode", CountryCode)};
			
			try
			{
				CountryCodeValue->PrependTag("standard", "ISO 3166-1 alpha-2"s);
				CountryCodeValue->PrependTag("interpretation", Inspection::Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(CountryCode));
				Result->SetSuccess(true);
			}
			catch(Inspection::UnknownValueException & Exception)
			{
				CountryCodeValue->PrependTag("standard", "ISRC Bulletin 2015/01"s);
				CountryCodeValue->PrependTag("error", "The ISRC string needs to contain a two letter country code from ISO 3166-1 alpha-2."s);
				CountryCodeValue->PrependTag("interpretation", "<unkown>"s);
			}
		}
		else
		{
			Result->GetValue("Information")->PrependTag("standard", "ID3 2.3"s);
			Result->GetValue("Information")->PrependTag("error", "The TSRC frame needs to contain a twelve letter ISRC code from ISRC Bulletin 2015/01."s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptionResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
		if(DescriptionResult->GetSuccess() == true)
		{
			auto ValueResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("Value", ValueResult->GetValue());
			Result->SetSuccess(ValueResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_3_Language(Buffer)};
		
		Result->GetValue()->AppendValue("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ContentDescriptorResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == true)
			{
				auto LyricsTextResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Lyrics/Text", LyricsTextResult->GetValue());
				Result->SetSuccess(LyricsTextResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
	
	Result->GetValue()->AppendValue("URL", URLResult->GetValue());
	Result->SetSuccess(URLResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_3_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptorResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->AppendValue("Descriptor", DescriptorResult->GetValue());
		if(DescriptorResult->GetSuccess() == true)
		{
			auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("URL", URLResult->GetValue());
			Result->SetSuccess(URLResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ID3_2_3_Frame_Header_Identifier(Buffer)};
	
	Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		if(SizeResult->GetSuccess() == true)
		{
			auto FlagsResult{Get_BitSet_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
			Result->SetSuccess(FlagsResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(4ull, 0))};
	
	Result->SetValue(IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Get_ID3_2_3_FrameIdentifier_Interpretation(Identifier));
			Result->GetValue()->PrependTag("standard", "ID3 2.3"s);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Identifier == "RGAD")
			{
				Result->GetValue()->PrependTag("interpretation", "Replay gain adjustment"s);
				Result->GetValue()->PrependTag("standard", "Hydrogenaudio ReplayGain"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard frame which is acknowledged as an 'in the wild' tag by id3.org."s);
			}
			else if(Identifier == "TCMP")
			{
				Result->GetValue()->PrependTag("interpretation", "Part of a compilation"s);
				Result->GetValue()->PrependTag("standard", "iTunes"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate whether a title is a part of a compilation."s);
			}
			else if(Identifier == "TDRC")
			{
				Result->GetValue()->PrependTag("interpretation", "Recording time"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TDTG")
			{
				Result->GetValue()->PrependTag("interpretation", "Tagging time"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSST")
			{
				Result->GetValue()->PrependTag("interpretation", "Set subtitle"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSOA")
			{
				Result->GetValue()->PrependTag("interpretation", "Album sort order"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSOP")
			{
				Result->GetValue()->PrependTag("interpretation", "Performer sort order"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It has only been introduced with tag version 2.4."s);
			}
			else if(Identifier == "TSO2")
			{
				Result->GetValue()->PrependTag("interpretation", "Album artist sort order"s);
				Result->GetValue()->PrependTag("standard", "iTunes"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.3. It is a non-standard text frame added by iTunes to indicate the album artist sort order."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Unkown frame identifier \"" + Identifier + "\"."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_3_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("Frame", FrameResult->GetValue());
		}
		else
		{
			auto SavePosition{Buffer.GetPosition()};
			
			Buffer.SetPosition(Start);
			
			auto PaddingResult{Get_Bits_Unset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			
			if(PaddingResult->GetSuccess() == true)
			{
				Result->GetValue()->AppendValue("Padding", PaddingResult->GetValue());
				
				break;
			}
			else
			{
				Result->GetValue()->AppendValue("Frame", FrameResult->GetValue());
				Buffer.SetPosition(SavePosition);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		Result->SetValue(FieldResult->GetValue());
		if(FieldResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag->AppendTag("bit name", "a"s);
		Flag = Result->GetValue()->AppendValue("ExtendedHeader", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("ExperimentalIndicator", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", Result->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		if(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", Result->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", Result->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto ISO_IEC_10646_1_1993_UCS_2_StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetValue());
		if(ISO_IEC_10646_1_1993_UCS_2_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", Result->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Frame_Header(Buffer)};
	
	Result->SetValue(HeaderResult->GetValue());
	if(HeaderResult->GetSuccess() == true)
	{
		Result->GetValue()->PrependTag("content", HeaderResult->GetValue("Identifier")->GetTagAny("interpretation"));
		
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(HeaderResult->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(HeaderResult->GetAny("Size")), 0)};
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > BodyHandler;
		
		if(Identifier == "APIC")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_APIC;
		}
		else if(Identifier == "COMM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_COMM;
		}
		else if(Identifier == "MCDI")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_MCDI;
		}
		else if(Identifier == "POPM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_POPM;
		}
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCON") || (Identifier == "TCOP") || (Identifier == "TDRC") || (Identifier == "TDRL") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIT2") || (Identifier == "TLAN") || (Identifier == "TLEN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TSSE") || (Identifier == "TYER"))
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_T___;
		}
		else if(Identifier == "TXXX")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_TXXX;
		}
		else if(Identifier == "UFID")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_UFID;
		}
		else if(Identifier == "USLT")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_USLT;
		}
		else if(Identifier == "WCOM")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_W___;
		}
		else if(Identifier == "WXXX")
		{
			BodyHandler = Get_ID3_2_4_Frame_Body_WXXX;
		}
		if(BodyHandler != nullptr)
		{
			auto BodyResult{BodyHandler(Buffer, Size)};
			
			if(Start + Size > Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated larger than the actual handled size."s);
			}
			else if(Start + Size < Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the acutal handled size is larger than the stated frame size."s);
			}
			Result->GetValue()->AppendValues(BodyResult->GetValue()->GetValues());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			auto BodyResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size)};
			
			Result->GetValue()->AppendValue("Data", BodyResult->GetValue());
			Result->SetSuccess(BodyResult->GetSuccess());
		}
		Buffer.SetPosition(Start + Size);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto MIMETypeResult{Get_ID3_2_4_Frame_Body_APIC_MIMEType(Buffer)};
		
		Result->GetValue()->AppendValue("MIMEType", MIMETypeResult->GetValue());
		if(MIMETypeResult->GetSuccess() == true)
		{
			auto PictureTypeResult{Get_ID3_2_4_Frame_Body_APIC_PictureType(Buffer)};
			
			Result->GetValue()->AppendValue("PictureType", PictureTypeResult->GetValue());
			if(PictureTypeResult->GetSuccess() == true)
			{
				auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
				auto DescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
				
				Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
				if(DescriptionResult->GetSuccess() == true)
				{
					auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
					
					Result->GetValue()->AppendValue("PictureData", PictureDataResult->GetValue());
					Result->SetSuccess(PictureDataResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MIMETypeResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
	/// @todo As per [ID3 2.4.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	Result->SetValue(MIMETypeResult->GetValue());
	Result->SetSuccess(MIMETypeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(PictureTypeResult->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3v4"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
			Result->SetSuccess(true);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_4_Language(Buffer)};
		
		Result->GetValue()->AppendValue("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ShortContentDescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("ShortContentDescription", ShortContentDescriptionResult->GetValue());
			if(ShortContentDescriptionResult->GetSuccess() == true)
			{
				auto CommentResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Comment", CommentResult->GetValue());
				Result->SetSuccess(CommentResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TableOfContentsResult{Get_IEC_60908_1999_TableOfContents(Buffer)};
	
	Result->SetValue(TableOfContentsResult->GetValue());
	Result->SetSuccess(TableOfContentsResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EMailToUserResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("EMailToUser", EMailToUserResult->GetValue());
	if(EMailToUserResult->GetSuccess() == true)
	{
		auto RatingResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Rating", RatingResult->GetValue());
		if(RatingResult->GetSuccess() == true)
		{
			auto Rating{std::experimental::any_cast< std::uint8_t >(RatingResult->GetAny())};
			
			if(Rating == 0)
			{
				Result->GetValue("Rating")->PrependTag("standard", "ID3 2.3"s);
				Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
			}
			if(Buffer.GetPosition() == Boundary)
			{
				auto CounterValue{std::make_shared< Inspection::Value >()};
				
				CounterValue->SetName("Counter");
				CounterValue->AppendTag("omitted"s);
				Result->GetValue()->AppendValue(CounterValue);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) > Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
				Result->GetValue("Counter")->PrependTag("standard", "ID3 2.4"s);
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) == Boundary)
			{
				auto CounterResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->SetSuccess(CounterResult->GetSuccess());
			}
			else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
			{
				auto CounterResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Counter", CounterResult->GetValue());
				Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto InformationResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Information[0]", InformationResult->GetValue());
		if(InformationResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			
			auto InformationIndex{1ul};
			
			while(Buffer.GetPosition() < Boundary)
			{
				InformationResult = Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition());
				Result->GetValue()->AppendValue("Information[" + to_string_cast(InformationIndex) + "]", InformationResult->GetValue());
				if(InformationResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
				InformationIndex += 1;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptionResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
		if(DescriptionResult->GetSuccess() == true)
		{
			auto ValueResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("Value", ValueResult->GetValue());
			Result->SetSuccess(ValueResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OwnerIdentifierResult{Get_ASCII_String_Printable_EndedByTermination(Buffer)};
	
	Result->GetValue()->AppendValue("OwnerIdentifier", OwnerIdentifierResult->GetValue());
	if(OwnerIdentifierResult->GetSuccess() == true)
	{
		auto IdentifierResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
		Result->SetSuccess(IdentifierResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto LanguageResult{Get_ID3_2_4_Language(Buffer)};
		
		Result->GetValue()->AppendValue("Language", LanguageResult->GetValue());
		if(LanguageResult->GetSuccess() == true)
		{
			auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
			auto ContentDescriptorResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
			
			Result->GetValue()->AppendValue("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == true)
			{
				auto LyricsTextResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
				
				Result->GetValue()->AppendValue("Lyrics/Text", LyricsTextResult->GetValue());
				Result->SetSuccess(LyricsTextResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
	
	Result->GetValue()->AppendValue("URL", URLResult->GetValue());
	Result->SetSuccess(URLResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_ID3_2_4_TextEncoding(Buffer)};
	
	Result->GetValue()->AppendValue("TextEncoding", TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		auto DescriptorResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		
		Result->GetValue()->AppendValue("Descriptor", DescriptorResult->GetValue());
		if(DescriptorResult->GetSuccess() == true)
		{
			auto URLResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Boundary - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("URL", URLResult->GetValue());
			Result->SetSuccess(URLResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ID3_2_4_Frame_Header_Identifier(Buffer)};
	
	Result->GetValue()->AppendValue("Identifier", IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		if(SizeResult->GetSuccess() == true)
		{
			auto FlagsResult{Get_BitSet_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
			Result->SetSuccess(FlagsResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto IdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByLength(Buffer, Inspection::Length(4ull, 0))};
	
	Result->SetValue(IdentifierResult->GetValue());
	if(IdentifierResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(IdentifierResult->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Get_ID3_2_4_FrameIdentifier_Interpretation(Identifier));
			Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Identifier == "TYER")
			{
				Result->GetValue()->PrependTag("interpretation", "Year"s);
				Result->GetValue()->PrependTag("standard", "ID3 2.3"s);
				Result->GetValue()->PrependTag("error", "This frame is not defined in tag version 2.4. It has only been valid until tag version 2.3."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Unkown frame identifier \"" + Identifier + "\"."s);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_4_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("Frame", FrameResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(Start);
			
			auto PaddingResult{Get_Bits_Unset_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			
			if(PaddingResult->GetSuccess() == true)
			{
				Result->GetValue()->AppendValue("Padding", PaddingResult->GetValue());
			}
			else
			{
				Result->GetValue()->AppendValue("Frame", FrameResult->GetValue());
				Result->SetSuccess(false);
				Buffer.SetPosition(Boundary);
			}
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Language(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FieldResult{Get_ISO_639_2_1998_Code(Buffer)};
	
	if(FieldResult->GetSuccess() == true)
	{
		Result->SetValue(FieldResult->GetValue());
		Result->SetSuccess(true);
	}
	else
	{
		Buffer.SetPosition(Start);
		FieldResult = Get_ASCII_String_Alphabetical_EndedByLength(Buffer, Inspection::Length(3ull, 0));
		if(FieldResult->GetSuccess() == true)
		{
			Result->SetValue(FieldResult->GetValue());
			
			const std::string & Code{std::experimental::any_cast< const std::string & >(FieldResult->GetAny())};
			
			if(Code == "XXX")
			{
				Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
				Result->GetValue()->PrependTag("interpretation", "<unknown>"s);
				Result->SetSuccess(true);
			}
			else
			{
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code \"" + Code + "\" is unknown."s);
			}
		}
		else
		{
			Buffer.SetPosition(Start);
			FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length(3ull, 0));
			Result->SetValue(FieldResult->GetValue());
			if(FieldResult->GetSuccess() == true)
			{
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
	
	Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
	if(SizeResult->GetSuccess() == true)
	{
		auto NumberOfFlagBytesResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("NumberOfFlagBytes", NumberOfFlagBytesResult->GetValue());
		if(NumberOfFlagBytesResult->GetSuccess() == true)
		{
			auto NumberOfFlagBytes{std::experimental::any_cast< std::uint8_t >(NumberOfFlagBytesResult->GetAny())};
			
			if(NumberOfFlagBytes == 0x01)
			{
				auto ExtendedHeaderFlagsResult{Get_ID3_2_4_Tag_ExtendedHeader_Flags(Buffer)};
				
				Result->GetValue()->AppendValue("ExtendedFlags", ExtendedHeaderFlagsResult->GetValue());
				if(ExtendedHeaderFlagsResult->GetSuccess() == true)
				{
					Result->SetSuccess(true);
					if(Result->GetSuccess() == true)
					{
						auto TagIsAnUpdate{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("TagIsAnUpdate"))};
						
						if(TagIsAnUpdate == true)
						{
							auto TagIsAnUpdateDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Buffer)};
							
							Result->GetValue()->AppendValue("TagIsAnUpdateData", TagIsAnUpdateDataResult->GetValue());
							Result->SetSuccess(TagIsAnUpdateDataResult->GetSuccess());
						}
					}
					if(Result->GetSuccess() == true)
					{
						auto CRCDataPresent{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("CRCDataPresent"))};
						
						if(CRCDataPresent == true)
						{
							auto CRCDataPresentDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Buffer)};
							
							Result->GetValue()->AppendValue("CRCDataPresentData", CRCDataPresentDataResult->GetValue());
							Result->SetSuccess(CRCDataPresentDataResult->GetSuccess());
						}
					}
					if(Result->GetSuccess() == true)
					{
						auto TagRestrictions{std::experimental::any_cast< bool >(ExtendedHeaderFlagsResult->GetAny("TagRestrictions"))};
						
						if(TagRestrictions == true)
						{
							auto TagRestrictionsDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Buffer)};
							
							Result->GetValue()->AppendValue("TagRestrictionsData", TagRestrictionsDataResult->GetValue());
							Result->SetSuccess(TagRestrictionsDataResult->GetSuccess());
						}
					}
				}
			}
			else
			{
				Result->SetSuccess(false);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
	if((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x05))
	{
		auto TotalFrameCRCResult{Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Buffer)};
		
		Result->GetValue()->AppendValue("TotalFrameCRC", TotalFrameCRCResult->GetValue());
		Result->SetSuccess(TotalFrameCRCResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
	Result->SetSuccess((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x00));
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
	
	Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
	if((HeaderResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("Size")) == 0x01))
	{
		auto RestrictionResult{Get_BitSet_8Bit(Buffer)};
		auto Value{Result->GetValue()->AppendValue("Restrictions", RestrictionResult->GetValue())};
		
		Value->AppendTag("error", "This program is missing the interpretation of the restriction flags."s); 
		Result->SetSuccess(RestrictionResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SizeResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
	
	Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
	Result->SetSuccess(SizeResult->GetSuccess());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		Result->GetValue()->AppendTag("synchsafe"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Reserved", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag = Result->GetValue()->AppendValue("TagIsAnUpdate", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("CRCDataPresent", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("TagRestrictions", Flags[4]);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit name", "d"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_8Bit(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(FlagsResult->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AppendTag("bit index", 7);
		Flag->AppendTag("bit name", "a"s);
		Flag = Result->GetValue()->AppendValue("ExtendedHeader", Flags[6]);
		Flag->AppendTag("bit index", 6);
		Flag->AppendTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("ExperimentalIndicator", Flags[5]);
		Flag->AppendTag("bit index", 5);
		Flag->AppendTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("FooterPresent", Flags[4]);
		Flag->AppendTag("bit index", 4);
		Flag->AppendTag("bit name", "d"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AppendTag("bit index", 3);
		Flag->AppendTag("bit index", 2);
		Flag->AppendTag("bit index", 1);
		Flag->AppendTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Result->SetSuccess(Result->GetSuccess() ^ ~Flags[FlagIndex]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TextEncodingResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->SetValue(TextEncodingResult->GetValue());
	if(TextEncodingResult->GetSuccess() == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(TextEncodingResult->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UTF-16"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x02)
		{
			Result->GetValue()->PrependTag("name", "UTF-16BE"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
		else if(TextEncoding == 0x03)
		{
			Result->GetValue()->PrependTag("name", "UTF-8"s);
			Result->GetValue()->PrependTag("standard", "RFC 2279"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Buffer)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", ISO_IEC_8859_1_1998_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto UTF_16_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_16_StringResult->GetValue());
		if(UTF_16_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", UTF_16_StringResult->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x02)
	{
		auto UTF_16_BE_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_16_BE_StringResult->GetValue());
		if(UTF_16_BE_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_16_BE_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x03)
	{
		auto UTF_8_StringResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Buffer)};
		
		Result->SetValue(UTF_8_StringResult->GetValue());
		if(UTF_8_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_8_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(TextEncoding == 0x00)
	{
		auto ISO_IEC_8859_1_1998_StringResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(ISO_IEC_8859_1_1998_StringResult->GetValue());
		if(ISO_IEC_8859_1_1998_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", ISO_IEC_8859_1_1998_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x01)
	{
		auto UTF_16_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_16_StringResult->GetValue());
		if(UTF_16_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->ClearTags();
			Result->GetValue()->PrependTag("string", UTF_16_StringResult->GetAny("String"));
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x02)
	{
		auto UTF_16_BE_StringResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_16_BE_StringResult->GetValue());
		if(UTF_16_BE_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_16_BE_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	else if(TextEncoding == 0x03)
	{
		auto UTF_8_StringResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Buffer, Length)};
		
		Result->SetValue(UTF_8_StringResult->GetValue());
		if(UTF_8_StringResult->GetSuccess() == true)
		{
			Result->GetValue()->PrependTag("string", UTF_8_StringResult->GetAny());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameCodeResult{Get_ID3_2_ReplayGainAdjustment_NameCode(Buffer)};
	
	Result->GetValue()->AppendValue("NameCode", NameCodeResult->GetValue());
	if(NameCodeResult->GetSuccess() == true)
	{
		auto OriginatorCodeResult{Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Buffer)};
		
		Result->GetValue()->AppendValue("OriginatorCode", OriginatorCodeResult->GetValue());
		if(OriginatorCodeResult->GetSuccess() == true)
		{
			auto SignBitResult{Get_ID3_2_ReplayGainAdjustment_SignBit(Buffer)};
			
			Result->GetValue()->AppendValue("SignBit", SignBitResult->GetValue());
			if(SignBitResult->GetSuccess() == true)
			{
				auto ReplayGainAdjustmentResult{Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Buffer)};
				
				Result->GetValue()->AppendValue("ReplayGainAdjustment", ReplayGainAdjustmentResult->GetValue());
				if(ReplayGainAdjustmentResult->GetSuccess() == true)
				{
					auto SignBit{std::experimental::any_cast< std::uint8_t >(SignBitResult->GetAny())};
					auto ReplayGainAdjustment{std::experimental::any_cast< float >(ReplayGainAdjustmentResult->GetValue()->GetTagAny("Interpretation"))};
					
					if(SignBit == 0x01)
					{
						ReplayGainAdjustment *= -1.0f;
					}
					Result->GetValue()->PrependTag("Standard", "Hydrogenaudio ReplayGain"s);
					Result->GetValue()->PrependTag("Interpretation", to_string_cast(ReplayGainAdjustment) + " dB");
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_NameCode(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameCodeResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(NameCodeResult->GetValue());
	if(NameCodeResult->GetSuccess() == true)
	{
		auto NameCode{std::experimental::any_cast< std::uint8_t >(NameCodeResult->GetAny())};
		
		if(NameCode == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "not set"s);
			Result->SetSuccess(true);
		}
		else if(NameCode == 0x01)
		{
			Result->GetValue()->PrependTag("Interpretation", "track gain adjustment"s);
			Result->SetSuccess(true);
		}
		else if(NameCode == 0x02)
		{
			Result->GetValue()->PrependTag("Interpretation", "album gain adjustment"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OriginatorCodeResult{Get_UnsignedInteger_3Bit(Buffer)};
	
	Result->SetValue(OriginatorCodeResult->GetValue());
	if(OriginatorCodeResult->GetSuccess() == true)
	{
		auto OriginatorCode{std::experimental::any_cast< std::uint8_t >(OriginatorCodeResult->GetAny())};
		
		if(OriginatorCode == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "unspecified"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x01)
		{
			Result->GetValue()->PrependTag("Interpretation", "pre-set by artist/producer/mastering engineer"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x02)
		{
			Result->GetValue()->PrependTag("Interpretation", "set by user"s);
			Result->SetSuccess(true);
		}
		else if(OriginatorCode == 0x03)
		{
			Result->GetValue()->PrependTag("Interpretation", "determined automatically"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "<unknown>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReplayGainAdjustmentResult{Get_UnsignedInteger_9Bit_BigEndian(Buffer)};
	
	Result->SetValue(ReplayGainAdjustmentResult->GetValue());
	if(ReplayGainAdjustmentResult->GetSuccess() == true)
	{
		float ReplayGainAdjustment{static_cast< float >(std::experimental::any_cast< std::uint16_t >(ReplayGainAdjustmentResult->GetAny()))};
		
		Result->GetValue()->PrependTag("Interpretation", ReplayGainAdjustment / 10.0f);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_SignBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SignBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(SignBitResult->GetValue());
	if(SignBitResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto SignBit{std::experimental::any_cast< std::uint8_t >(SignBitResult->GetAny())};
		
		if(SignBit == 0x00)
		{
			Result->GetValue()->PrependTag("Interpretation", "positive gain (boost)"s);
		}
		else
		{
			Result->GetValue()->PrependTag("Interpretation", "negative gain (attenuation)"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_Tag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TagHeaderResult{Get_ID3_2_Tag_Header(Buffer)};
	
	Result->GetValue()->AppendValues(TagHeaderResult->GetValue()->GetValues());
	if(TagHeaderResult->GetSuccess() == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(TagHeaderResult->GetAny("MajorVersion"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(TagHeaderResult->GetAny("Size")), 0)};
		
		if(MajorVersion == 0x02)
		{
			auto FramesResult{Get_ID3_2_2_Frames(Buffer, Size)};
			
			Result->GetValue()->AppendValues(FramesResult->GetValue()->GetValues());
			Result->SetSuccess(FramesResult->GetSuccess());
		}
		else if(MajorVersion == 0x03)
		{
			if((TagHeaderResult->GetValue("Flags")->HasValue("ExtendedHeader") == true) && (std::experimental::any_cast< bool >(TagHeaderResult->GetValue("Flags")->GetValueAny("ExtendedHeader")) == true))
			{
				throw Inspection::NotImplementedException("ID3 2.3 extended header");
			}
			
			auto FramesResult{Get_ID3_2_3_Frames(Buffer, Size)};
			
			Result->GetValue()->AppendValues(FramesResult->GetValue()->GetValues());
			Result->SetSuccess(FramesResult->GetSuccess());
		}
		else if(MajorVersion == 0x04)
		{
			std::unique_ptr< Inspection::Result > ExtendedHeaderResult;
			
			if((TagHeaderResult->GetValue("Flags")->HasValue("ExtendedHeader") == true) && (std::experimental::any_cast< bool >(TagHeaderResult->GetValue("Flags")->GetValueAny("ExtendedHeader")) == true))
			{
				ExtendedHeaderResult = Get_ID3_2_4_Tag_ExtendedHeader(Buffer);
				Result->GetValue()->AppendValue("ExtendedHeader", ExtendedHeaderResult->GetValue());
				Result->SetSuccess(ExtendedHeaderResult->GetSuccess());
				Size -= ExtendedHeaderResult->GetLength();
			}
			if((ExtendedHeaderResult == nullptr) || (ExtendedHeaderResult->GetSuccess() == true))
			{
				auto FramesResult{Get_ID3_2_4_Frames(Buffer, Size)};
				
				Result->GetValue()->AppendValues(FramesResult->GetValue()->GetValues());
				Result->SetSuccess(FramesResult->GetSuccess());
			}
		}
		else
		{
			Result->GetValue()->PrependTag("error", "Unknown major version \"" + to_string_cast(MajorVersion) + "\".");
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_Tag_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FileIdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Buffer, "ID3")};
	
	Result->GetValue()->AppendValue("FileIdentifier", FileIdentifierResult->GetValue());
	if(FileIdentifierResult->GetSuccess() == true)
	{
		auto MajorVersionResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("MajorVersion", MajorVersionResult->GetValue());
		if(MajorVersionResult->GetSuccess() == true)
		{
			auto RevisionNumberResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("RevisionNumber", RevisionNumberResult->GetValue());
			if(RevisionNumberResult->GetSuccess() == true)
			{
				auto MajorVersion{std::experimental::any_cast< std::uint8_t >(MajorVersionResult->GetAny())};
				std::unique_ptr< Inspection::Result > FlagsResult;
				
				if(MajorVersion == 0x02)
				{
					FlagsResult = Get_ID3_2_2_Tag_Header_Flags(Buffer);
				}
				else if(MajorVersion == 0x03)
				{
					FlagsResult = Get_ID3_2_3_Tag_Header_Flags(Buffer);
				}
				else if(MajorVersion == 0x04)
				{
					FlagsResult = Get_ID3_2_4_Tag_Header_Flags(Buffer);
				}
				if(FlagsResult)
				{
					Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
					if(FlagsResult->GetSuccess() == true)
					{
						auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
						
						Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
						Result->SetSuccess(SizeResult->GetSuccess());
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(1ull, 0)) == true)
	{
		if(Buffer.Get1Bits() == 0x00)
		{
			std::uint8_t First{Buffer.Get7Bits()};
			
			Result->GetValue()->SetAny(First);
			Result->GetValue()->AppendTag("integer"s);
			Result->GetValue()->AppendTag("unsigned"s);
			Result->GetValue()->AppendTag("7bit value"s);
			Result->GetValue()->AppendTag("8bit field"s);
			Result->GetValue()->AppendTag("synchsafe"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(4ull, 0)) == true)
	{
		if(Buffer.Get1Bits() == 0x00)
		{
			std::uint32_t First{Buffer.Get7Bits()};
			
			if(Buffer.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Buffer.Get7Bits()};
				
				if(Buffer.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Buffer.Get7Bits()};
					
					if(Buffer.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Buffer.Get7Bits()};
						
						Result->GetValue()->SetAny((First << 21) | (Second << 14) | (Third << 7) | (Fourth));
						Result->GetValue()->AppendTag("integer"s);
						Result->GetValue()->AppendTag("unsigned"s);
						Result->GetValue()->AppendTag("28bit value"s);
						Result->GetValue()->AppendTag("32bit field"s);
						Result->GetValue()->AppendTag("synchsafe"s);
						Result->SetSuccess(true);
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Inspection::Length(5ull, 0)) == true)
	{
		if(Buffer.Get4Bits() == 0x00)
		{
			std::uint32_t First{Buffer.Get4Bits()};
			
			if(Buffer.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Buffer.Get7Bits()};
				
				if(Buffer.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Buffer.Get7Bits()};
					
					if(Buffer.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Buffer.Get7Bits()};
						
						if(Buffer.Get1Bits() == 0x00)
						{
							std::uint32_t Fifth{Buffer.Get7Bits()};
							
							Result->GetValue()->SetAny((First << 28) | (Second << 21) | (Third << 14) | (Fourth << 7) | Fifth);
							Result->GetValue()->AppendTag("integer"s);
							Result->GetValue()->AppendTag("unsigned"s);
							Result->GetValue()->AppendTag("32bit value"s);
							Result->GetValue()->AppendTag("40bit field"s);
							Result->GetValue()->AppendTag("synchsafe"s);
							Result->SetSuccess(true);
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_GUID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const Inspection::GUID & GUID{std::experimental::any_cast< const Inspection::GUID & >(GUIDResult->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Inspection::Get_GUID_Interpretation(GUID));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Result->GetValue()->PrependTag("interpretation", "<unknown GUID>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("standard", "IEC 60908:1999"s);
	Result->GetValue()->AppendTag("name", "Compact Disc Digital Audio"s);
	
	auto HeaderResult{Get_IEC_60908_1999_TableOfContents_Header(Buffer)};
	
	Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
	if(HeaderResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto FirstTrackNumber{std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("FirstTrackNumber"))};
		auto LastTrackNumber{std::experimental::any_cast< std::uint8_t >(HeaderResult->GetAny("LastTrackNumber"))};
		auto TracksResult{Get_IEC_60908_1999_TableOfContents_Tracks(Buffer, FirstTrackNumber, LastTrackNumber)};
		
		Result->GetValue()->AppendValues(TracksResult->GetValue()->GetValues());
		Result->SetSuccess(TracksResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataLengthResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	Result->GetValue()->AppendValue("DataLength", DataLengthResult->GetValue());
	if(DataLengthResult->GetSuccess() == true)
	{
		auto FirstTrackNumberResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("FirstTrackNumber", FirstTrackNumberResult->GetValue());
		if(FirstTrackNumberResult->GetSuccess() == true)
		{
			auto LastTrackNumberResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("LastTrackNumber", LastTrackNumberResult->GetValue());
			Result->SetSuccess(LastTrackNumberResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TrackResult{Get_IEC_60908_1999_TableOfContents_Track(Buffer)};
	
	Result->SetValue(TrackResult->GetValue());
	Result->SetSuccess((TrackResult->GetSuccess() == true) && (TrackResult->GetValue("Number")->HasTag("interpretation") == true) && (std::experimental::any_cast< const std::string & >(TrackResult->GetValue("Number")->GetTagAny("interpretation")) == "Lead-Out"));
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Reserved1Result{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 8))};
	
	Result->GetValue()->AppendValue("Reserved", Reserved1Result->GetValue());
	if(Reserved1Result->GetSuccess() == true)
	{
		auto ADRResult{Get_UnsignedInteger_4Bit(Buffer)};
		
		Result->GetValue()->AppendValue("ADR", ADRResult->GetValue());
		if((ADRResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(ADRResult->GetAny())))
		{
			auto ControlResult{Get_IEC_60908_1999_TableOfContents_Track_Control(Buffer)};
			
			Result->GetValue()->AppendValue("Control", ControlResult->GetValue());
			if(ControlResult->GetSuccess() == true)
			{
				auto NumberResult{Get_UnsignedInteger_8Bit(Buffer)};
				
				Result->GetValue()->AppendValue("Number", NumberResult->GetValue());
				if(NumberResult->GetSuccess() == true)
				{
					auto Number{std::experimental::any_cast< std::uint8_t >(NumberResult->GetAny())};
					
					if(Number == 0xaa)
					{
						Result->GetValue("Number")->PrependTag("interpretation", "Lead-Out"s);
					}
					
					auto Reserved2Result{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 8))};
					
					Result->GetValue()->AppendValue("Reserved", Reserved2Result->GetValue());
					if(Reserved2Result->GetSuccess() == true)
					{
						auto StartAddressResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
						
						Result->GetValue()->AppendValue("StartAddress", StartAddressResult->GetValue());
						Result->SetSuccess(StartAddressResult->GetSuccess());
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ControlResult{Get_BitSet_4Bit_MostSignificantBitFirst(Buffer)};
	
	Result->SetValue(ControlResult->GetValue());
	if(ControlResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		const std::bitset< 4 > & Control{std::experimental::any_cast< const std::bitset< 4 > & >(ControlResult->GetAny())};
		
		if(Control[1] == true)
		{
			if(Control[0] == true)
			{
				Result->SetSuccess(false);
				
				auto Value{Result->GetValue()->AppendValue("Reserved", true)};
				
				Value->AppendTag("error", "The track type is \"Data\" so this bit must be off.");
			}
			else
			{
				Result->GetValue()->AppendValue("Reserved", false);
			}
			Result->GetValue()->AppendValue("TrackType", "Data"s);
			Result->GetValue()->AppendValue("DigitalCopyProhibited", !Control[2]);
			if(Control[3] == true)
			{
				Result->GetValue()->AppendValue("DataRecorded", "incrementally"s);
			}
			else
			{
				Result->GetValue()->AppendValue("DataRecorded", "uninterrupted"s);
			}
		}
		else
		{
			if(Control[0] == true)
			{
				Result->GetValue()->AppendValue("NumberOfChannels", 4);
			}
			else
			{
				Result->GetValue()->AppendValue("NumberOfChannels", 2);
			}
			Result->GetValue()->AppendValue("TrackType", "Audio"s);
			Result->GetValue()->AppendValue("DigitalCopyProhibited", !Control[2]);
			Result->GetValue()->AppendValue("PreEmphasis", Control[3]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Buffer & Buffer, std::uint8_t FirstTrackNumber, std::uint8_t LastTrackNumber)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	for(auto TrackNumber = FirstTrackNumber; TrackNumber <= LastTrackNumber; ++TrackNumber)
	{
		auto TrackResult{Get_IEC_60908_1999_TableOfContents_Track(Buffer)};
		auto TrackValue{Result->GetValue()->AppendValue("Track", TrackResult->GetValue())};
		
		if(TrackResult->GetSuccess() == true)
		{
			auto TrackNumber{std::experimental::any_cast< std::uint8_t >(TrackResult->GetAny("Number"))};
			
			TrackValue->SetName("Track " + to_string_cast(TrackNumber));
		}
		else
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	if(Result->GetSuccess() == true)
	{
		auto LeadOutTrackResult{Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Buffer)};
		
		Result->GetValue()->AppendValue("LeadOutTrack", LeadOutTrackResult->GetValue());
		Result->SetSuccess(LeadOutTrackResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_639_2_1998_Code(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodeResult{Get_ASCII_String_Alphabetical_EndedByLength(Buffer, Inspection::Length(3ull, 0))};
	
	Result->SetValue(CodeResult->GetValue());
	if(CodeResult->GetSuccess() == true)
	{
		const std::string & Code{std::experimental::any_cast< const std::string & >(CodeResult->GetAny())};
		
		try
		{
			auto Interpretation{Get_LanguageName_From_ISO_639_2_1998_Code(Code)};
			
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("interpretation", Interpretation);
			Result->SetSuccess(true);
		}
		catch(...)
		{
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto Character{Buffer.Get8Bits()};
		
		Result->GetValue()->AppendValue("byte", Character);
		if(Is_ISO_IEC_8859_1_1998_Character(Character) == true)
		{
			Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Character));
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
		Result->SetSuccess(true);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
				{
					NumberOfCharacters += 1;
					Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
					if(Buffer.GetPosition() == Boundary)
					{
						Result->GetValue()->AppendTag("ended by length"s);
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
						Result->SetSuccess(true);
						
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	
	auto NumberOfCharacters{0ul};
	
	while(true)
	{
		auto CharacterResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
		
		if(CharacterResult->GetSuccess() == true)
		{
			NumberOfCharacters += 1;
			Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
		}
		else
		{
			auto Byte{std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny("byte"))};
			
			if(Byte == 0x00)
			{
				Result->GetValue()->AppendTag("ended by termination"s);
				Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				Result->SetSuccess(true);
			}
			
			break;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
		Result->SetSuccess(true);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
				{
					NumberOfCharacters += 1;
					Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
					if(Buffer.GetPosition() == Boundary)
					{
						Result->GetValue()->AppendTag("ended by length"s);
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
						Result->SetSuccess(true);
						
						break;
					}
				}
				else
				{
					auto Byte{std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny("byte"))};
					
					if(Byte == 0x00)
					{
						if(Buffer.GetPosition() == Boundary)
						{
							Result->GetValue()->AppendTag("ended by termination and length"s);
						}
						else
						{
							Result->GetValue()->AppendTag("ended by termination"s);
						}
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
						Result->SetSuccess(true);
					}
					
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	if(Buffer.GetPosition() < Boundary)
	{
		auto NumberOfCharacters{0ul};
		auto NumberOfTerminations{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
				{
					NumberOfCharacters += 1;
					Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
				}
				else
				{
					auto Byte{std::experimental::any_cast< std::uint8_t >(CharacterResult->GetAny("byte"))};
					
					if(Byte == 0x00)
					{
						if(NumberOfTerminations == 0)
						{
							Result->GetValue()->AppendTag("ended by termination"s);
							if(NumberOfCharacters > 0)
							{
								Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
							}
							else
							{
								Result->GetValue()->AppendTag("empty"s);
							}
						}
						NumberOfTerminations += 1;
						if(Buffer.GetPosition() == Boundary)
						{
							Result->GetValue()->AppendTag("until length"s);
							Result->GetValue()->AppendTag(to_string_cast(NumberOfTerminations) + " terminations");
							Result->SetSuccess(true);
							
							break;
						}
					}
					else
					{
						break;
					}
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		auto NumberOfTerminations{0ul};
		
		while(true)
		{
			auto Position{Buffer.GetPosition()};
			auto CharacterResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
			
			if((Buffer.GetPosition() <= Boundary) && (CharacterResult->GetSuccess() == true))
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
					Result->SetSuccess(true);
					
					break;
				}
			}
			else
			{
				Buffer.SetPosition(Position);
				
				break;
			}
		}
		while(Buffer.GetPosition() < Boundary)
		{
			auto Position{Buffer.GetPosition()};
			auto Byte{Buffer.Get8Bits()};
			
			if(Byte == 0x00)
			{
				if(NumberOfTerminations == 0)
				{
					Result->GetValue()->AppendTag("ended by termination"s);
					Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
				}
				NumberOfTerminations += 1;
				if(Buffer.GetPosition() == Boundary)
				{
					Result->GetValue()->AppendTag("ended by length"s);
					Result->GetValue()->AppendTag(to_string_cast(NumberOfTerminations) + " terminations");
					Result->SetSuccess(true);
					
					break;
				}
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto BytesResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 2ull)};
		
		Result->SetValue(BytesResult->GetValue());
		if(BytesResult->GetSuccess() == true)
		{
			const std::vector< std::uint8_t > & Bytes{std::experimental::any_cast< const std::vector< std::uint8_t > & >(BytesResult->GetAny())};
			
			if((Bytes[0] == 0xfe) && (Bytes[1] == 0xff))
			{
				Result->GetValue()->PrependTag("interpretation", "BigEndian"s);
				Result->SetSuccess(true);
			}
			else if((Bytes[0] == 0xff) && (Bytes[1] == 0xfe))
			{
				Result->GetValue()->PrependTag("interpretation", "LittleEndian"s);
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodePointResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Buffer)};
	
	if(CodePointResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendValue("CodePoint", CodePointResult->GetValue());
		Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodePointResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Buffer)};
	
	if(CodePointResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendValue("CodePoint", CodePointResult->GetValue());
		Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		auto Second{Buffer.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint32_t >((static_cast< std::uint32_t >(First) << 8) | static_cast< std::uint32_t >(Second)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		auto Second{Buffer.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint32_t >((static_cast< std::uint32_t >(Second) << 8) | static_cast< std::uint32_t >(First)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AppendTag("big endian"s);
	
	auto NumberOfCharacters{0ul};
	
	while(true)
	{
		auto CharacterResult{Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Buffer)};
		
		if(CharacterResult->GetSuccess() == true)
		{
			auto CodePoint{std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint"))};
			
			if(CodePoint == 0x00000000)
			{
				Result->GetValue()->AppendTag("ended by termination"s);
				Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				Result->SetSuccess(true);
				
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
			}
		}
		else
		{
			break;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AppendTag("big endian"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
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
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
						Result->SetSuccess(true);
						
						break;
					}
					else
					{
						NumberOfCharacters += 1;
						Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
						if(Buffer.GetPosition() == Boundary)
						{
							Result->GetValue()->AppendTag("ended by boundary"s);
							Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
							Result->SetSuccess(true);
							
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AppendTag("little endian"s);
	
	auto NumberOfCharacters{0ul};
	
	while(true)
	{
		auto CharacterResult{Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Buffer)};
		
		if(CharacterResult->GetSuccess() == true)
		{
			auto CodePoint{std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint"))};
			
			if(CodePoint == 0x00000000)
			{
				Result->GetValue()->AppendTag("ended by termination"s);
				Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				Result->SetSuccess(true);
				
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
			}
		}
		else
		{
			break;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AppendTag("little endian"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
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
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
						Result->SetSuccess(true);
						
						break;
					}
					else
					{
						NumberOfCharacters += 1;
						Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
						if(Buffer.GetPosition() == Boundary)
						{
							Result->GetValue()->AppendTag("ended by boundary"s);
							Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
							Result->SetSuccess(true);
							
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	
	auto ByteOrderMarkResult{Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Buffer)};
	
	Result->GetValue()->AppendValue("ByteOrderMark", ByteOrderMarkResult->GetValue());
	if(ByteOrderMarkResult->GetSuccess() == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(ByteOrderMarkResult->GetValue()->GetTagAny("interpretation"))};
		
		if(ByteOrderMark == "BigEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Buffer)};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Buffer)};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto StartPosition{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UCS-2"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	
	auto ByteOrderMarkResult{Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Buffer)};
	
	Result->GetValue()->AppendValue("ByteOrderMark", ByteOrderMarkResult->GetValue());
	if(ByteOrderMarkResult->GetSuccess() == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(ByteOrderMarkResult->GetValue()->GetTagAny("interpretation"))};
		
		if(ByteOrderMark == "BigEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Buffer, StartPosition + Length - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Buffer, StartPosition + Length - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodePointResult{Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Buffer)};
	
	Result->GetValue()->AppendValue("CodePoint", CodePointResult->GetValue());
	if(CodePointResult->GetSuccess() == true)
	{
		auto CodePoint{std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())};
		
		Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(1ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		
		if((First & 0x80) == 0x00)
		{
			Result->GetValue()->SetAny(static_cast< std::uint32_t >(First));
			Result->SetSuccess(true);
		}
		else if((First & 0xe0) == 0xc0)
		{
			if(Buffer.Has(1ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				
				if((Second & 0xc0) == 0x80)
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x1f) << 6) | static_cast< std::uint32_t >(Second & 0x3f));
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
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x0f) << 12)| static_cast< std::uint32_t >((Second & 0x3f) << 6) | static_cast< std::uint32_t >(Third & 0x3f));
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
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x07) << 18)| static_cast< std::uint32_t >((Second & 0x3f) << 12) | static_cast< std::uint32_t >((Third & 0x3f) << 6) | static_cast< std::uint32_t >(Fourth & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(Length) == true)
	{
		Result->GetValue()->AppendTag("string"s);
		Result->GetValue()->AppendTag("UTF-8"s);
		
		auto Boundary{Buffer.GetPosition() + Length};
		auto NumberOfCharacters{0ul};
		std::string String;
		
		while(Buffer.GetPosition() < Boundary)
		{
			auto Character{Get_ISO_IEC_10646_1_1993_UTF_8_Character(Buffer)};
			
			if(Character->GetSuccess() == true)
			{
				NumberOfCharacters += 1;
				String += std::experimental::any_cast< std::string >(Character->GetAny());
			}
			else
			{
				break;
			}
		}
		Result->GetValue()->SetAny(String);
		if(Buffer.GetPosition() == Boundary)
		{
			Result->GetValue()->AppendTag("ended by length"s);
			Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UTF-8"s);
	
	std::stringstream Value;
	auto NumberOfCharacters{0ul};
	
	while(true)
	{
		auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_8_Character(Buffer)};
		
		if(CharacterResult->GetSuccess() == true)
		{
			auto CodePoint{std::experimental::any_cast< std::uint32_t >(CharacterResult->GetAny("CodePoint"))};
			
			if(CodePoint == 0x00000000)
			{
				Result->GetValue()->AppendTag("ended by termination"s);
				Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				Result->SetSuccess(true);
				
				break;
			}
			else
			{
				NumberOfCharacters += 1;
				Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
			}
		}
		else
		{
			break;
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UTF-8"s);
	if(Buffer.GetPosition() == Boundary)
	{
		Result->GetValue()->AppendTag("ended by length"s);
		Result->GetValue()->AppendTag("empty"s);
	}
	else
	{
		auto NumberOfCharacters{0ul};
		
		while(true)
		{
			auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_8_Character(Buffer)};
			
			if(Buffer.GetPosition() <= Boundary)
			{
				if(CharacterResult->GetSuccess() == true)
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
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
						Result->SetSuccess(true);
						
						break;
					}
					else
					{
						NumberOfCharacters += 1;
						Value << std::experimental::any_cast< const std::string & >(CharacterResult->GetAny());
						if(Buffer.GetPosition() == Boundary)
						{
							Result->GetValue()->AppendTag("ended by boundary"s);
							Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
							Result->SetSuccess(true);
							
							break;
						}
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				break;
			}
		}
	}
	Result->GetValue()->SetAny(Value.str());
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto BytesResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 2ull)};
		
		Result->SetValue(BytesResult->GetValue());
		if(BytesResult->GetSuccess() == true)
		{
			const std::vector< std::uint8_t > & Bytes{std::experimental::any_cast< const std::vector< std::uint8_t > & >(BytesResult->GetAny())};
			
			if((Bytes[0] == 0xfe) && (Bytes[1] == 0xff))
			{
				Result->GetValue()->PrependTag("interpretation", "BigEndian"s);
				Result->SetSuccess(true);
			}
			else if((Bytes[0] == 0xff) && (Bytes[1] == 0xfe))
			{
				Result->GetValue()->PrependTag("interpretation", "LittleEndian"s);
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer)
{
	throw NotImplementedException("Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination()");
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto StartPosition{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("UTF-16"s);
	Result->GetValue()->AppendTag("ISO/IEC 10646-1:1993"s);
	
	auto ByteOrderMarkResult{Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Buffer)};
	
	Result->GetValue()->AppendValue("ByteOrderMark", ByteOrderMarkResult->GetValue());
	if(ByteOrderMarkResult->GetSuccess() == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(ByteOrderMarkResult->GetValue()->GetTagAny("interpretation"))};
		
		if(ByteOrderMark == "BigEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, StartPosition + Length - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			auto StringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, StartPosition + Length - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("String", StringResult->GetValue());
			Result->SetSuccess(StringResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodePointResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Buffer)};
	
	if(CodePointResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendValue("CodePoint", CodePointResult->GetValue());
		Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FirstCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Buffer)};
	
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
			auto SecondCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Buffer)};
			
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		auto Second{Buffer.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint16_t >(static_cast< std::uint16_t >(First << 8) | static_cast< std::uint16_t >(Second)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Buffer & Buffer)
{
	throw NotImplementedException("Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination()");
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBytes() % 2 == 0);
	assert(Length.GetBits() == 0);
	
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	auto NumberOfCodePoints{0ul};
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("big endian"s);
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
			auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_Character(Buffer)};
			
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_Character(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CodePointResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Buffer)};
	
	if(CodePointResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendValue("CodePoint", CodePointResult->GetValue());
		Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(CodePointResult->GetAny())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FirstCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Buffer)};
	
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
			auto SecondCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Buffer)};
			
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto First{Buffer.Get8Bits()};
		auto Second{Buffer.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint16_t >(static_cast< std::uint16_t >(First) | (static_cast< std::uint16_t >(Second) << 8)));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBytes() % 2 == 0);
	assert(Length.GetBits() == 0);
	
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("little endian"s);
	Result->GetValue()->AppendTag("without byte order mark"s);
	
	auto NumberOfCodePoints{0ul};
	
	while(Buffer.GetPosition() < Boundary)
	{
		auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_Character(Buffer)};
		
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	assert(Length.GetBytes() % 2 == 0);
	assert(Length.GetBits() == 0);
	
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
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
			auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_Character(Buffer)};
			
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Buffer & Buffer, std::uint64_t NumberOfCodePoints)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	std::stringstream Value;
	std::uint64_t CodePointIndex{0ull};
	
	Result->GetValue()->AppendTag("UTF16"s);
	Result->GetValue()->AppendTag("little endian"s);
	Result->GetValue()->AppendTag("without byte order mark"s);
	while(true)
	{
		auto CharacterResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_Character(Buffer)};
		
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(4ull, 0))};
	
	if(DataResult->GetSuccess() == true)
	{
		Result->GetValue()->SetAny(*reinterpret_cast< const float * const >(&(std::experimental::any_cast< const std::vector< std::uint8_t > & >(DataResult->GetAny()).front())));
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Microsoft_WaveFormat_FormatTag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
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
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_UNKNOWN"));
				Result->GetValue()->AppendTag("description", std::string("Unknown or invalid format tag"));
				
				break;
			}
		case 0x0001:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_PCM"));
				Result->GetValue()->AppendTag("description", std::string("Pulse Code Modulation"));
				
				break;
			}
		case 0x0002:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_ADPCM"));
				Result->GetValue()->AppendTag("description", std::string("Microsoft Adaptive Differental PCM"));
				
				break;
			}
		case 0x0003:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_IEEE_FLOAT"));
				Result->GetValue()->AppendTag("description", std::string("32-bit floating-point"));
				
				break;
			}
		case 0x0055:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_MPEGLAYER3"));
				Result->GetValue()->AppendTag("description", std::string("ISO/MPEG Layer3"));
				
				break;
			}
		case 0x0092:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_DOLBY_AC3_SPDIF"));
				Result->GetValue()->AppendTag("description", std::string("Dolby Audio Codec 3 over S/PDIF"));
				
				break;
			}
		case 0x0161:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_WMAUDIO2"));
				Result->GetValue()->AppendTag("description", std::string("Windows Media Audio Standard (Versions 7, 8, and 9 Series)"));
				
				break;
			}
		case 0x0162:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_WMAUDIO3"));
				Result->GetValue()->AppendTag("description", std::string("Windows Media Audio Professional (9 Series)"));
				
				break;
			}
		case 0x0163:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_WMAUDIO_LOSSLESS"));
				Result->GetValue()->AppendTag("description", std::string("Windows Media Audio Lossless (9 Series)"));
				
				break;
			}
		case 0x0164:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_WMASPDIF"));
				Result->GetValue()->AppendTag("description", std::string("Windows Media Audio over S/PDIF"));
				
				break;
			}
		case 0xFFFE:
			{
				Result->GetValue()->AppendTag("constant name", std::string("WAVE_FORMAT_EXTENSIBLE"));
				Result->GetValue()->AppendTag("description", std::string("All new audio formats"));
				
				break;
			}
		default:
			{
				Result->GetValue()->AppendTag("constant name", std::string("<no interpretation>"));
				Result->GetValue()->AppendTag("description", std::string("<no interpretation>"));
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_Frame(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameHeaderResult{Get_MPEG_1_FrameHeader(Buffer)};
	
	Result->GetValue()->AppendValue("Header", FrameHeaderResult->GetValue());
	if(FrameHeaderResult->GetSuccess() == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("ProtectionBit"))};
		auto Continue{true};
		
		if(ProtectionBit == 0x00)
		{
			auto ErrorCheckResult{Get_BitSet_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("ErrorCheck", ErrorCheckResult->GetValue());
			Continue = ErrorCheckResult->GetSuccess();
		}
		if(Continue == true)
		{
			auto LayerDescription{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("LayerDescription"))};
			auto BitRate{std::experimental::any_cast< std::uint32_t >(FrameHeaderResult->GetValue("BitRateIndex")->GetTagAny("numeric"))};
			auto SamplingFrequency{std::experimental::any_cast< std::uint32_t >(FrameHeaderResult->GetValue("SamplingFrequency")->GetTagAny("numeric"))};
			auto PaddingBit{std::experimental::any_cast< std::uint8_t >(FrameHeaderResult->GetAny("PaddingBit"))};
			auto FrameLength{0ul};
			
			if(LayerDescription == 0x03)
			{
				FrameLength = (12 * BitRate / SamplingFrequency + PaddingBit) * 4;
			}
			else if((LayerDescription == 0x01) || (LayerDescription == 0x02))
			{
				FrameLength = 144 * BitRate / SamplingFrequency + PaddingBit;
			}
			
			auto AudioDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(FrameLength) + Start - Buffer.GetPosition())};
			
			Result->GetValue()->AppendValue("AudioData", AudioDataResult->GetValue());
			Result->SetSuccess(AudioDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameSyncResult{Get_Bits_Set_EndedByLength(Buffer, Inspection::Length(0ull, 12))};
	
	Result->GetValue()->AppendValue("FrameSync", FrameSyncResult->GetValue());
	if(FrameSyncResult->GetSuccess() == true)
	{
		auto AudioVersionIDResult{Get_MPEG_1_FrameHeader_AudioVersionID(Buffer)};
		
		Result->GetValue()->AppendValue("AudioVersionID", AudioVersionIDResult->GetValue());
		if(AudioVersionIDResult->GetSuccess() == true)
		{
			auto LayerDescriptionResult{Get_MPEG_1_FrameHeader_LayerDescription(Buffer)};
			
			Result->GetValue()->AppendValue("LayerDescription", LayerDescriptionResult->GetValue());
			if(LayerDescriptionResult->GetSuccess() == true)
			{
				auto ProtectionBitResult{Get_MPEG_1_FrameHeader_ProtectionBit(Buffer)};
				
				Result->GetValue()->AppendValue("ProtectionBit", ProtectionBitResult->GetValue());
				if(ProtectionBitResult->GetSuccess() == true)
				{
					auto LayerDescription{std::experimental::any_cast< std::uint8_t >(LayerDescriptionResult->GetAny())};
					auto BitRateIndexResult{Get_MPEG_1_FrameHeader_BitRateIndex(Buffer, LayerDescription)};
					
					Result->GetValue()->AppendValue("BitRateIndex", BitRateIndexResult->GetValue());
					if(BitRateIndexResult->GetSuccess() == true)
					{
						auto SamplingFrequencyResult{Get_MPEG_1_FrameHeader_SamplingFrequency(Buffer)};
						
						Result->GetValue()->AppendValue("SamplingFrequency", SamplingFrequencyResult->GetValue());
						if(SamplingFrequencyResult->GetSuccess() == true)
						{
							auto PaddingBitResult{Get_MPEG_1_FrameHeader_PaddingBit(Buffer)};
							
							Result->GetValue()->AppendValue("PaddingBit", PaddingBitResult->GetValue());
							if(PaddingBitResult->GetSuccess() == true)
							{
								auto PrivateBitResult{Get_UnsignedInteger_1Bit(Buffer)};
								
								Result->GetValue()->AppendValue("PrivateBit", PrivateBitResult->GetValue());
								if(PrivateBitResult->GetSuccess() == true)
								{
									auto ModeResult{Get_MPEG_1_FrameHeader_Mode(Buffer, LayerDescription)};
									
									Result->GetValue()->AppendValue("Mode", ModeResult->GetValue());
									if(ModeResult->GetSuccess() == true)
									{
										auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
										auto ModeExtensionResult{Get_MPEG_1_FrameHeader_ModeExtension(Buffer, LayerDescription, Mode)};
										
										Result->GetValue()->AppendValue("ModeExtension", ModeExtensionResult->GetValue());
										if(ModeExtensionResult->GetSuccess() == true)
										{
											auto CopyrightResult{Get_MPEG_1_FrameHeader_Copyright(Buffer)};
											
											Result->GetValue()->AppendValue("Copyright", CopyrightResult->GetValue());
											if(CopyrightResult->GetSuccess() == true)
											{
												auto OriginalHomeResult{Get_MPEG_1_FrameHeader_OriginalHome(Buffer)};
												
												Result->GetValue()->AppendValue("Original/Home", OriginalHomeResult->GetValue());
												if(OriginalHomeResult->GetSuccess() == true)
												{
													auto EmphasisResult{Get_MPEG_1_FrameHeader_Emphasis(Buffer)};
													
													Result->GetValue()->AppendValue("Emphasis", EmphasisResult->GetValue());
													Result->SetSuccess(EmphasisResult->GetSuccess());
												}
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto AudioVersionIDResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(AudioVersionIDResult->GetValue());
	if(AudioVersionIDResult->GetSuccess() == true)
	{
		auto AudioVersionID{std::experimental::any_cast< std::uint8_t >(AudioVersionIDResult->GetAny())};
		
		if(AudioVersionID == 0x01)
		{
			Result->GetValue()->PrependTag("MPEG Version 1 (ISO/IEC 11172-3)"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("<reserved>");
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Buffer & Buffer, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitRateIndexResult{Get_UnsignedInteger_4Bit(Buffer)};
	
	Result->SetValue(BitRateIndexResult->GetValue());
	if(BitRateIndexResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto BitRateIndex{std::experimental::any_cast< std::uint8_t >(BitRateIndexResult->GetAny())};
		
		if(LayerDescription == 0x03)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 288000u);
				Result->GetValue()->PrependTag("288 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 352000u);
				Result->GetValue()->PrependTag("352 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 384000u);
				Result->GetValue()->PrependTag("384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 416000u);
				Result->GetValue()->PrependTag("416 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 448000u);
				Result->GetValue()->PrependTag("448 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else if(LayerDescription == 0x02)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 48000u);
				Result->GetValue()->PrependTag("48 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 56000u);
				Result->GetValue()->PrependTag("56 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 80000u);
				Result->GetValue()->PrependTag("80 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 112000u);
				Result->GetValue()->PrependTag("112 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 384000u);
				Result->GetValue()->PrependTag("384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else if(LayerDescription == 0x01)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->PrependTag("free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->PrependTag("numeric", 32000u);
				Result->GetValue()->PrependTag("32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->PrependTag("numeric", 40000u);
				Result->GetValue()->PrependTag("40 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->PrependTag("numeric", 48000u);
				Result->GetValue()->PrependTag("48 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->PrependTag("numeric", 56000u);
				Result->GetValue()->PrependTag("56 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->PrependTag("numeric", 64000u);
				Result->GetValue()->PrependTag("64 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->PrependTag("numeric", 80000u);
				Result->GetValue()->PrependTag("80 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->PrependTag("numeric", 96000u);
				Result->GetValue()->PrependTag("96 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->PrependTag("numeric", 112000u);
				Result->GetValue()->PrependTag("112 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->PrependTag("numeric", 128000u);
				Result->GetValue()->PrependTag("128 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->PrependTag("numeric", 160000u);
				Result->GetValue()->PrependTag("160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->PrependTag("numeric", 192000u);
				Result->GetValue()->PrependTag("192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->PrependTag("numeric", 224000u);
				Result->GetValue()->PrependTag("224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->PrependTag("numeric", 256000u);
				Result->GetValue()->PrependTag("256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->PrependTag("numeric", 320000u);
				Result->GetValue()->PrependTag("320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->PrependTag("<reserved>"s);
				Result->SetSuccess(false);
			}
		}
		else
		{
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Copyright(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CopyrightResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(CopyrightResult->GetValue());
	if(CopyrightResult->GetSuccess() == true)
	{
		auto Copyright{std::experimental::any_cast< std::uint8_t >(CopyrightResult->GetAny())};
		
		if(Copyright == 0x00)
		{
			Result->GetValue()->PrependTag("copyright", false);
			Result->SetSuccess(true);
		}
		else if(Copyright == 0x01)
		{
			Result->GetValue()->PrependTag("copyright", true);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Emphasis(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto EmphasisResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(EmphasisResult->GetValue());
	if(EmphasisResult->GetSuccess() == true)
	{
		auto Emphasis{std::experimental::any_cast< std::uint8_t >(EmphasisResult->GetAny())};
		
		if(Emphasis == 0x00)
		{
			Result->GetValue()->PrependTag("no emphasis"s);
			Result->SetSuccess(true);
		}
		else if(Emphasis == 0x01)
		{
			Result->GetValue()->PrependTag("50/15 microsec. emphasis"s);
			Result->SetSuccess(true);
		}
		else if(Emphasis == 0x02)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
		else if(Emphasis == 0x03)
		{
			Result->GetValue()->PrependTag("CCITT J.17"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LayerDescriptionResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(LayerDescriptionResult->GetValue());
	if(LayerDescriptionResult->GetSuccess() == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(LayerDescriptionResult->GetAny())};
		
		if(LayerDescription == 0x00)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
		else if(LayerDescription == 0x01)
		{
			Result->GetValue()->PrependTag("Layer III"s);
			Result->SetSuccess(true);
		}
		else if(LayerDescription == 0x02)
		{
			Result->GetValue()->PrependTag("Layer II"s);
			Result->SetSuccess(true);
		}
		else if(LayerDescription == 0x03)
		{
			Result->GetValue()->PrependTag("Layer I"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Mode(Inspection::Buffer & Buffer, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ModeResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(ModeResult->GetValue());
	if(ModeResult->GetSuccess() == true)
	{
		auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
		
		if(Mode == 0x00)
		{
			Result->GetValue()->PrependTag("stereo"s);
			Result->SetSuccess(true);
		}
		else if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo)"s);
				Result->SetSuccess(true);
			}
			else if(LayerDescription == 0x01)
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo and/or ms_stereo)"s);
				Result->SetSuccess(true);
			}
		}
		else if(Mode == 0x02)
		{
			Result->GetValue()->PrependTag("dual_channel"s);
			Result->SetSuccess(true);
		}
		else if(Mode == 0x03)
		{
			Result->GetValue()->PrependTag("single_channel"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Buffer & Buffer, std::uint8_t LayerDescription, std::uint8_t Mode)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ModeExtensionResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(ModeExtensionResult->GetValue());
	if(ModeExtensionResult->GetSuccess() == true)
	{
		auto ModeExtension{std::experimental::any_cast< std::uint8_t >(ModeExtensionResult->GetAny())};
		
		if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("subbands 4-31 in intensity_stereo, bound==4"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("subbands 8-31 in intensity_stereo, bound==8"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("subbands 12-31 in intensity_stereo, bound==12"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("subbands 16-31 in intensity_stereo, bound==16"s);
					Result->SetSuccess(true);
				}
			}
			else if(LayerDescription == 0x01)
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
					Result->SetSuccess(true);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
					Result->SetSuccess(true);
				}
			}
		}
		else
		{
			Result->GetValue()->PrependTag("<ignored>"s);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto OriginalHomeResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(OriginalHomeResult->GetValue());
	if(OriginalHomeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto OriginalHome{std::experimental::any_cast< std::uint8_t >(OriginalHomeResult->GetAny())};
		
		if(OriginalHome == 0x00)
		{
			Result->GetValue()->PrependTag("original", false);
		}
		else if(OriginalHome == 0x01)
		{
			Result->GetValue()->PrependTag("original", true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PaddingBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(PaddingBitResult->GetValue());
	if(PaddingBitResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto PaddingBit{std::experimental::any_cast< std::uint8_t >(PaddingBitResult->GetAny())};
		
		if(PaddingBit == 0x00)
		{
			Result->GetValue()->PrependTag("padding", false);
		}
		else if(PaddingBit == 0x01)
		{
			Result->GetValue()->PrependTag("padding", true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ProtectionBitResult{Get_UnsignedInteger_1Bit(Buffer)};
	
	Result->SetValue(ProtectionBitResult->GetValue());
	if(ProtectionBitResult->GetSuccess() == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(ProtectionBitResult->GetAny())};
		
		if(ProtectionBit == 0x00)
		{
			Result->GetValue()->PrependTag("redundancy", true);
			Result->SetSuccess(true);
		}
		else if(ProtectionBit == 0x01)
		{
			Result->GetValue()->PrependTag("redundancy", false);
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SamplingFrequencyResult{Get_UnsignedInteger_2Bit(Buffer)};
	
	Result->SetValue(SamplingFrequencyResult->GetValue());
	if(SamplingFrequencyResult->GetSuccess() == true)
	{
		auto SamplingFrequency{std::experimental::any_cast< std::uint8_t >(SamplingFrequencyResult->GetAny())};
		
		if(SamplingFrequency == 0x00)
		{
			Result->GetValue()->PrependTag("numeric", 44100u);
			Result->GetValue()->PrependTag("44.1 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x01)
		{
			Result->GetValue()->PrependTag("numeric", 48000u);
			Result->GetValue()->PrependTag("48 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x02)
		{
			Result->GetValue()->PrependTag("numeric", 32000u);
			Result->GetValue()->PrependTag("32 kHz"s);
			Result->SetSuccess(true);
		}
		else if(SamplingFrequency == 0x03)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_Stream(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(false);
	while(true)
	{
		auto Position{Buffer.GetPosition()};
		auto MPEGFrameResult{Get_MPEG_1_Frame(Buffer)};
		
		if(MPEGFrameResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("MPEGFrame", MPEGFrameResult->GetValue());
			Result->SetSuccess(true);
		}
		else
		{
			Buffer.SetPosition(Position);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_9Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 9) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get1Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("9bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	assert(Buffer.GetBitstreamType() == Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	auto CommentHeaderWithoutFramingFlag{Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer)};
	
	if(CommentHeaderWithoutFramingFlag->GetSuccess() == true)
	{
		Result->GetValue()->AppendValues(CommentHeaderWithoutFramingFlag->GetValue()->GetValues());
		
		auto FramingFlagResult{Get_Boolean_1Bit(Buffer)};
		
		Result->GetValue()->AppendValue("FramingFlag", FramingFlagResult->GetValue());
		if(FramingFlagResult->GetSuccess() == true)
		{
			Result->SetSuccess(std::experimental::any_cast< bool >(FramingFlagResult->GetAny()));
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UserCommentLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	auto UserCommentLengthValue{Result->GetValue()->AppendValue("Length", UserCommentLengthResult->GetValue())};
	
	UserCommentLengthValue->AppendTag("unit", "bytes"s);
	if(UserCommentLengthResult->GetSuccess() == true)
	{
		auto UserCommentLength{std::experimental::any_cast< std::uint32_t >(UserCommentLengthResult->GetAny())};
		auto UserCommentResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, UserCommentLength)};
		
		Result->GetValue()->AppendValue("String", UserCommentResult->GetValue());
		if(UserCommentResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UserCommentListLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	auto UserCommentListLengthValue{Result->GetValue()->AppendValue("Length", UserCommentListLengthResult->GetValue())};
	
	UserCommentListLengthValue->AppendTag("unit", "items"s);
	if(UserCommentListLengthResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto UserCommentListLength{std::experimental::any_cast< std::uint32_t >(UserCommentListLengthResult->GetAny())};
		
		for(auto Index = 0ul; Index < UserCommentListLength; ++Index)
		{
			auto UserCommentResult{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			
			Result->GetValue()->AppendValue("UserComment", UserCommentResult->GetValue());
			if(UserCommentResult->GetSuccess() == false)
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto VendorLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	auto VendorLengthValue{Result->GetValue()->AppendValue("VendorLength", VendorLengthResult->GetValue())};
	
	VendorLengthValue->AppendTag("unit", "bytes"s);
	if(VendorLengthResult->GetSuccess() == true)
	{
		auto VendorLength{std::experimental::any_cast< std::uint32_t >(VendorLengthResult->GetAny())};
		auto VendorResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, VendorLength)};
		
		Result->GetValue()->AppendValue("Vendor", VendorResult->GetValue());
		if(VendorResult->GetSuccess() == true)
		{
			auto UserCommentListResult{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
			
			Result->GetValue()->AppendValue("UserCommentList", UserCommentListResult->GetValue());
			Result->SetSuccess(UserCommentListResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}
