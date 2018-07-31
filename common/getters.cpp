#include <bitset>
#include <sstream>
#include <vector>

#include "buffer.h"
#include "getters.h"
#include "guid.h"
#include "helper.h"
#include "id3_helper.h"
#include "not_implemented_exception.h"
#include "reader.h"
#include "string_cast.h"
#include "unknown_value_exception.h"

using namespace std::string_literals;

void Inspection::UpdateState(bool & Continue, std::unique_ptr< Inspection::Result > & FieldResult)
{
	Continue = FieldResult->GetSuccess();
}

void Inspection::UpdateState(bool & Continue, Inspection::Buffer & Buffer, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader)
{
	UpdateState(Continue, FieldResult);
	if(Continue == true)
	{
		Buffer.SetPosition(FieldReader);
	}
}

void Inspection::UpdateState(bool & Continue, Inspection::Reader & Reader, std::unique_ptr< Inspection::Result > & FieldResult, const Inspection::Reader & FieldReader)
{
	UpdateState(Continue, FieldResult);
	if(Continue == true)
	{
		Reader.AdvancePosition(FieldReader.GetConsumedLength());
	}
}

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

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Getter, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	auto ElementIndex{0ull};
	
	while(true)
	{
		auto Start{Buffer.GetPosition()};
		
		if(Start < Boundary)
		{
			auto ElementResult{Getter(Buffer)};
			
			if(ElementResult->GetSuccess() == true)
			{
				auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
				
				ElementValue->AppendTag("array index", static_cast< std::uint64_t> (ElementIndex));
				ElementIndex++;
			}
			else
			{
				Result->GetValue()->AppendValue(ElementResult->GetValue());
				Result->GetValue()->PrependTag("ended by failure"s);
				Buffer.SetPosition(Start);
				
				break;
			}
		}
		else
		{
			Result->GetValue()->PrependTag("ended by length"s);
			
			break;
		}
	}
	Result->GetValue()->PrependTag("number of elements", static_cast< std::uint64_t> (ElementIndex));
	Result->GetValue()->PrependTag("array"s);
	Result->SetSuccess(true);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByNumberOfElements(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &) > Getter, std::uint64_t NumberOfElements)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ElementIndex{0ull};
	
	while(true)
	{
		if(ElementIndex < NumberOfElements)
		{
			auto ElementResult{Getter(Buffer)};
			auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
			
			ElementValue->AppendTag("array index", static_cast< std::uint64_t> (ElementIndex));
			ElementIndex++;
			if(ElementResult->GetSuccess() == false)
			{
				Result->GetValue()->PrependTag("ended by failure"s);
				
				break;
			}
		}
		else
		{
			Result->GetValue()->PrependTag("ended by number of elements"s);
			Result->SetSuccess(true);
			
			break;
		}
	}
	Result->GetValue()->PrependTag("number of elements", static_cast< std::uint64_t> (NumberOfElements));
	Result->GetValue()->PrependTag("array"s);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByNumberOfElements_PassArrayIndex(Inspection::Buffer & Buffer, std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, std::uint64_t) > Getter, std::uint64_t NumberOfElements)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ElementIndex{0ull};
	
	while(true)
	{
		if(ElementIndex < NumberOfElements)
		{
			auto ElementResult{Getter(Buffer, ElementIndex)};
			auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
			
			ElementValue->AppendTag("array index", static_cast< std::uint64_t> (ElementIndex));
			ElementIndex++;
			if(ElementResult->GetSuccess() == false)
			{
				Result->GetValue()->PrependTag("ended by failure"s);
				
				break;
			}
		}
		else
		{
			Result->GetValue()->PrependTag("ended by number of elements"s);
			Result->SetSuccess(true);
			
			break;
		}
	}
	Result->GetValue()->PrependTag("number of elements", static_cast< std::uint64_t> (NumberOfElements));
	Result->GetValue()->PrependTag("array"s);
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
					Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters) + "th character was alpha numeric but did not match the template.");
					
					break;
				}
			}
			else
			{
				Result->SetSuccess(false);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character was not alpha numeric.");
				
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(UnsignedInteger16BitResult->GetValue());
	if(UnsignedInteger16BitResult->GetSuccess() == true)
	{
		auto UnsignedInteger16Bit{std::experimental::any_cast< std::uint16_t >(UnsignedInteger16BitResult->GetAny())};
		
		if(UnsignedInteger16Bit == 0x0000)
		{
			Result->GetValue()->PrependTag("value", false);
			Result->SetSuccess(true);
		}
		else if(UnsignedInteger16Bit == 0x0001)
		{
			Result->GetValue()->PrependTag("value", true);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Boolean_32Bit_LittleEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(UnsignedInteger32BitResult->GetValue());
	if(UnsignedInteger32BitResult->GetSuccess() == true)
	{
		auto UnsignedInteger32Bit{std::experimental::any_cast< std::uint32_t >(UnsignedInteger32BitResult->GetAny())};
		
		if(UnsignedInteger32Bit == 0x00000000)
		{
			Result->GetValue()->PrependTag("value", false);
			Result->SetSuccess(true);
		}
		else if(UnsignedInteger32Bit == 0x00000001)
		{
			Result->GetValue()->PrependTag("value", true);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecEntry(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TypeResult{Get_ASF_CodecEntryType(Buffer)};
	
	TypeResult->GetValue()->AppendValue("Type", TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto CodecNameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("CodecNameLength", CodecNameLengthResult->GetValue());
		if(CodecNameLengthResult->GetSuccess() == true)
		{
			auto CodecNameLength{std::experimental::any_cast< std::uint16_t >(CodecNameLengthResult->GetAny())};
			auto CodecNameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecNameLength)};
			
			Result->GetValue()->AppendValue("CodecName", CodecNameResult->GetValue());
			if(CodecNameResult->GetSuccess() == true)
			{
				auto CodecDescriptionLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("CodecDescriptionLength", CodecDescriptionLengthResult->GetValue());
				if(CodecDescriptionLengthResult->GetSuccess() == true)
				{
					auto CodecDescriptionLength{std::experimental::any_cast< std::uint16_t >(CodecDescriptionLengthResult->GetAny())};
					auto CodecDescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecDescriptionLength)};
					
					Result->GetValue()->AppendValue("CodecDescription", CodecDescriptionResult->GetValue());
					if(CodecDescriptionResult->GetSuccess() == true)
					{
						auto CodecInformationLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->AppendValue("CodecInformation Length", CodecInformationLengthResult->GetValue());
						if(CodecInformationLengthResult->GetSuccess() == true)
						{
							auto CodecInformationLength{std::experimental::any_cast< std::uint16_t >(CodecInformationLengthResult->GetAny())};
							auto CodecInformationResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, CodecInformationLength)};
							
							Result->GetValue()->AppendValue("CodecInformation", CodecInformationResult->GetValue());
							if(CodecInformationResult->GetSuccess() == true)
							{
								Result->SetSuccess(true);
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecEntryType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(TypeResult->GetValue());
	if(TypeResult->GetSuccess() == true)
	{
		auto Type{std::experimental::any_cast< std::uint16_t >(TypeResult->GetAny())};
		
		if(Type == 0x0001)
		{
			Result->GetValue()->PrependTag("interpretation", "Video Codec"s);
		}
		else if(Type == 0x0002)
		{
			Result->GetValue()->PrependTag("interpretation", "Audio Codec"s);
		}
		else if(Type == 0xffff)
		{
			Result->GetValue()->PrependTag("interpretation", "Unknown Codec"s);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "<no interpretation>"s);
		}
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedGUIDResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->AppendValue("Reserved", ReservedGUIDResult->GetValue());
	if((ReservedGUIDResult->GetSuccess() == true) && (std::experimental::any_cast< Inspection::GUID >(ReservedGUIDResult->GetAny()) == Inspection::g_ASF_Reserved2GUID))
	{
		auto CodecEntriesCountResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("CodecEntriesCount", CodecEntriesCountResult->GetValue());
		if(CodecEntriesCountResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			
			auto CodecEntries(std::make_shared< Inspection::Value >());
			
			Result->GetValue()->AppendValue("CodecEntries", CodecEntries);
			
			auto CodecEntriesCount{std::experimental::any_cast< std::uint32_t >(CodecEntriesCountResult->GetAny())};
			
			for(auto CodecEntryIndex = 0ul; CodecEntryIndex < CodecEntriesCount; ++CodecEntryIndex)
			{
				auto CodecEntryResult{Get_ASF_CodecEntry(Buffer)};
				
				CodecEntries->AppendValue(CodecEntryResult->GetValue());
				if(CodecEntryResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CompatibilityObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ProfileResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->GetValue()->AppendValue("Profile", ProfileResult->GetValue());
	if(ProfileResult->GetSuccess() == true)
	{
		auto Profile{std::experimental::any_cast< std::uint8_t >(ProfileResult->GetAny())};
		
		if(Profile == 0x02)
		{
			auto ModeResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("Mode", ModeResult->GetValue());
			if(ModeResult->GetSuccess() == true)
			{
				auto Mode{std::experimental::any_cast< std::uint8_t >(ModeResult->GetAny())};
				
				if(Mode == 0x01)
				{
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto TitleLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("TitleLength", TitleLengthResult->GetValue());
	if(TitleLengthResult->GetSuccess() == true)
	{
		auto AuthorLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("AuthorLength", AuthorLengthResult->GetValue());
		if(AuthorLengthResult->GetSuccess() == true)
		{
			auto CopyrightLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("CopyrightLength", CopyrightLengthResult->GetValue());
			if(CopyrightLengthResult->GetSuccess() == true)
			{
				auto DescriptionLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("DescriptionLength", DescriptionLengthResult->GetValue());
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					auto RatingLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("RatingLength", RatingLengthResult->GetValue());
					if(RatingLengthResult->GetSuccess() == true)
					{
						auto TitleLength{std::experimental::any_cast< std::uint16_t >(TitleLengthResult->GetAny())};
						auto TitleResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, TitleLength)};
						
						Result->GetValue()->AppendValue("Title", TitleResult->GetValue());
						if(TitleResult->GetSuccess() == true)
						{
							auto AuthorLength{std::experimental::any_cast< std::uint16_t >(AuthorLengthResult->GetAny())};
							auto AuthorResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, AuthorLength)};
							
							Result->GetValue()->AppendValue("Author", AuthorResult->GetValue());
							if(AuthorResult->GetSuccess() == true)
							{
								auto CopyrightLength{std::experimental::any_cast< std::uint16_t >(CopyrightLengthResult->GetAny())};
								auto CopyrightResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, CopyrightLength)};
								
								Result->GetValue()->AppendValue("Copyright", CopyrightResult->GetValue());
								if(CopyrightResult->GetSuccess() == true)
								{
									auto DescriptionLength{std::experimental::any_cast< std::uint16_t >(DescriptionLengthResult->GetAny())};
									auto DescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, DescriptionLength)};
									
									Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
									if(DescriptionResult->GetSuccess() == true)
									{
										auto RatingLength{std::experimental::any_cast< std::uint16_t >(RatingLengthResult->GetAny())};
										auto RatingResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, RatingLength)};
										
										Result->GetValue()->AppendValue("Rating", RatingResult->GetValue());
										Result->SetSuccess(RatingResult->GetSuccess());
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_DataObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		
		if(GUID == Inspection::g_ASF_DataObjectGUID)
		{
			auto DataObjectDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(Size, 0) - ObjectHeaderResult->GetLength())};
			
			Result->GetValue()->AppendValue("Data", DataObjectDataResult->GetValue());
			Result->SetSuccess(DataObjectDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_DataType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataTypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(DataTypeResult->GetValue());
	if(DataTypeResult->GetSuccess() == true)
	{
		auto DataType{std::experimental::any_cast< std::uint16_t >(DataTypeResult->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->PrependTag("interpretation", "Unicode string"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->PrependTag("interpretation", "Byte array"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->PrependTag("interpretation", "Boolean"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 32bit"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 64bit"s);
			Result->SetSuccess(true);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 16bit"s);
			Result->SetSuccess(true);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("NameLength", NameLengthResult->GetValue());
	if(NameLengthResult->GetSuccess() == true)
	{
		auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
		auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, NameLength)};
		
		Result->GetValue()->AppendValue("Name", NameResult->GetValue());
		if(NameResult->GetSuccess() == true)
		{
			auto ValueDataTypeResult{Get_ASF_DataType(Buffer)};
			
			Result->GetValue()->AppendValue("ValueDataType", ValueDataTypeResult->GetValue());
			if(ValueDataTypeResult->GetSuccess() == true)
			{
				auto ValueLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("ValueLength", ValueLengthResult->GetValue());
				if(ValueLengthResult->GetSuccess() == true)
				{
					auto ValueLength{std::experimental::any_cast< std::uint16_t >(ValueLengthResult->GetAny())};
					auto ValueDataType{std::experimental::any_cast< const std::string & >(ValueDataTypeResult->GetValue()->GetTagAny("interpretation"))};
					auto Name{std::experimental::any_cast< const std::string & >(NameResult->GetAny())};
					auto DataValueResult{Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Buffer, ValueLength, ValueDataType, Name)};
					
					Result->GetValue()->AppendValue("Value", DataValueResult->GetValue());
					Result->SetSuccess(DataValueResult->GetSuccess());
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType, const std::string & Name)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(DataType == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
		if(Name == "WM/MediaPrimaryClassID")
		{
			auto String{std::experimental::any_cast< const std::string & >(Result->GetAny())};
			auto GUID{Inspection::Get_GUID_FromString_WithCurlyBraces(String)};
			
			Result->GetValue()->AppendValue("GUID", GUID);
			Result->GetValue("GUID")->AppendTag("guid"s);
			Result->GetValue("GUID")->AppendTag("string"s);
			
			auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
			
			Result->GetValue("GUID")->AppendTag("interpretation", GUIDInterpretation);
		}
	}
	else if(DataType == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(DataType == "Boolean")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
		if(Name == "WM/EncodingTime")
		{
			auto UnsignedInteger64Bit{std::experimental::any_cast< std::uint64_t >(Result->GetAny())};
			auto DateTime{Inspection::Get_DateTime_FromMicrosoftFileTime(UnsignedInteger64Bit)};
			
			Result->GetValue()->AppendValue("DateTime", DateTime);
			Result->GetValue("DateTime")->AppendTag("date and time"s);
			Result->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
		}
	}
	else if(DataType == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ContentDescriptorsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("ContentDescriptorsCount", ContentDescriptorsCountResult->GetValue());
	if(ContentDescriptorsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto ContentDescriptorsCount{std::experimental::any_cast< std::uint16_t >(ContentDescriptorsCountResult->GetAny())};
		
		for(auto ContentDescriptorsIndex = 0; ContentDescriptorsIndex < ContentDescriptorsCount; ++ContentDescriptorsIndex)
		{
			auto ContentDescriptorResult{Get_ASF_ExtendedContentDescription_ContentDescriptor(Buffer)};
			
			Result->GetValue()->AppendValue("ContentDescriptor", ContentDescriptorResult->GetValue());
			if(ContentDescriptorResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(FlagsResult->GetAny())};
		
		Result->GetValue()->AppendValue("[0] Reliable", Flags[0]);
		Result->GetValue()->AppendValue("[1] Seekable", Flags[1]);
		Result->GetValue()->AppendValue("[2] No Cleanpoints", Flags[2]);
		Result->GetValue()->AppendValue("[3] Resend Live Cleanpoints", Flags[3]);
		Result->GetValue()->AppendValue("[4-31] Reserved", false);
		Result->SetSuccess(true);
		for(auto Index = 4; Index < 32; ++Index)
		{
			Result->SetSuccess(Result->GetSuccess() & ~Flags[Index]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StartTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("StartTime", StartTimeResult->GetValue());
	Result->GetValue("StartTime")->PrependTag("milliseconds"s);
	if(StartTimeResult->GetSuccess() == true)
	{
		auto EndTimeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("EndTime", EndTimeResult->GetValue());
		Result->GetValue("EndTime")->PrependTag("milliseconds"s);
		if(EndTimeResult->GetSuccess() == true)
		{
			auto DataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("DataBitrate", DataBitrateResult->GetValue());
			Result->GetValue("DataBitrate")->PrependTag("bits per second"s);
			if(DataBitrateResult->GetSuccess() == true)
			{
				auto BufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("BufferSize", BufferSizeResult->GetValue());
				Result->GetValue("BufferSize")->PrependTag("milliseconds"s);
				if(BufferSizeResult->GetSuccess() == true)
				{
					auto InitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("InitialBufferFullness", InitialBufferFullnessResult->GetValue());
					Result->GetValue("InitialBufferFullness")->PrependTag("milliseconds"s);
					if(InitialBufferFullnessResult->GetSuccess() == true)
					{
						auto AlternateDataBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->AppendValue("AlternateDataBitrate", AlternateDataBitrateResult->GetValue());
						Result->GetValue("AlternateDataBitrate")->PrependTag("bits per second"s);
						if(AlternateDataBitrateResult->GetSuccess() == true)
						{
							auto AlternateBufferSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->AppendValue("AlternateBufferSize", AlternateBufferSizeResult->GetValue());
							Result->GetValue("AlternateBufferSize")->PrependTag("milliseconds"s);
							if(AlternateBufferSizeResult->GetSuccess() == true)
							{
								auto AlternateInitialBufferFullnessResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
								
								Result->GetValue()->AppendValue("AlternateInitialBufferFullness", AlternateInitialBufferFullnessResult->GetValue());
								Result->GetValue("AlternateInitialBufferFullness")->PrependTag("milliseconds"s);
								if(AlternateInitialBufferFullnessResult->GetSuccess() == true)
								{
									auto MaximumObjectSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
									
									Result->GetValue()->AppendValue("MaximumObjectSize", MaximumObjectSizeResult->GetValue());
									if(MaximumObjectSizeResult->GetSuccess() == true)
									{
										auto FlagsResult{Get_ASF_ExtendedStreamPropertiesObject_Flags(Buffer)};
										
										Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
										if(FlagsResult->GetSuccess() == true)
										{
											auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
											
											Result->GetValue()->AppendValue("StreamNumber", StreamNumberResult->GetValue());
											if(StreamNumberResult->GetSuccess() == true)
											{
												auto StreamLanguageIndexResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
												
												Result->GetValue()->AppendValue("StreamLanguageIndex", StreamLanguageIndexResult->GetValue());
												if(StreamLanguageIndexResult->GetSuccess() == true)
												{
													auto AverageTimePerFrameResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
													
													Result->GetValue()->AppendValue("AverageTimePerFrame", AverageTimePerFrameResult->GetValue());
													if(AverageTimePerFrameResult->GetSuccess() == true)
													{
														auto StreamNameCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
														
														Result->GetValue()->AppendValue("StreamNameCount", StreamNameCountResult->GetValue());
														if(StreamNameCountResult->GetSuccess() == true)
														{
															auto PayloadExtensionSystemCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
															
															Result->GetValue()->AppendValue("PayloadExtensionSystemCount", PayloadExtensionSystemCountResult->GetValue());
															if(PayloadExtensionSystemCountResult->GetSuccess() == true)
															{
																auto StreamNameCount{std::experimental::any_cast< std::uint16_t >(StreamNameCountResult->GetAny())};
																auto PayloadExtensionSystemCount{std::experimental::any_cast< std::uint16_t >(PayloadExtensionSystemCountResult->GetAny())};
																
																if((StreamNameCount == 0) && (PayloadExtensionSystemCount == 0))
																{
																	if(Buffer.GetPosition() < Boundary)
																	{
																		auto StreamPropertiesObjectResult{Get_ASF_StreamPropertiesObject(Buffer)};
																		
																		Result->GetValue()->AppendValue("StreamPropertiesObject", StreamPropertiesObjectResult->GetValue());
																		Result->SetSuccess(StreamPropertiesObjectResult->GetSuccess());
																	}
																	else
																	{
																		Result->SetSuccess(true);
																	}
																}
																else
																{
																	throw std::runtime_error("Not implemented.");
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
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_GUID(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->SetValue(GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		auto GUIDInterpretation{Get_GUID_Interpretation(std::experimental::any_cast< Inspection::GUID >(GUIDResult->GetAny()))};
		
		Result->GetValue()->PrependTag("interpretation", GUIDInterpretation);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedField1Result{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->AppendValue("ReservedField1", ReservedField1Result->GetValue());
	if(ReservedField1Result->GetSuccess() == true)
	{
		auto Reserved1Field{std::experimental::any_cast< Inspection::GUID >(ReservedField1Result->GetAny())};
		
		if(Reserved1Field == Inspection::g_ASF_Reserved1GUID)
		{
			auto ReservedField2Result{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("ReservedField2", ReservedField2Result->GetValue());
			if(ReservedField2Result->GetSuccess() == true)
			{
				auto ReservedField2{std::experimental::any_cast< std::uint16_t >(ReservedField2Result->GetAny())};
				
				if(ReservedField2 == 0x0006)
				{
					auto HeaderExtensionDataSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("HeaderExtensionDataSize", HeaderExtensionDataSizeResult->GetValue());
					if(HeaderExtensionDataSizeResult->GetSuccess() == true)
					{
						auto HeaderExtensionDataSize{std::experimental::any_cast< std::uint32_t >(HeaderExtensionDataSizeResult->GetAny())};
						Inspection::Length Boundary{Buffer.GetPosition() + HeaderExtensionDataSize};
						
						Result->SetSuccess(true);
						while(Buffer.GetPosition() < Boundary)
						{
							auto AdditionalExtendedHeaderObjectResult{Get_ASF_Object(Buffer)};
							
							Result->GetValue()->AppendValue("AdditionalExtendedHeader", AdditionalExtendedHeaderObjectResult->GetValue());
							if(AdditionalExtendedHeaderObjectResult->GetSuccess() == false)
							{
								Result->SetSuccess(false);
								
								break;
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_File(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto HeaderObjectResult{Get_ASF_HeaderObject(Buffer)};
	
	Result->GetValue()->AppendValue("HeaderObject", HeaderObjectResult->GetValue());
	if(HeaderObjectResult->GetSuccess() == true)
	{
		auto DataObjectResult{Get_ASF_DataObject(Buffer)};
		
		Result->GetValue()->AppendValue("DataObject", DataObjectResult->GetValue());
		if(DataObjectResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
			while(Buffer.GetPosition() < Buffer.GetLength())
			{
				auto ObjectResult{Get_ASF_Object(Buffer)};
				
				Result->GetValue()->AppendValue("Object", ObjectResult->GetValue());
				if(ObjectResult->GetSuccess() == false)
				{
					Result->SetSuccess(false);
					
					break;
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_FilePropertiesFlags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_BitSet_32Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(FlagsResult->GetAny())};
		
		Result->GetValue()->AppendValue("[0] Broadcast", Flags[0]);
		Result->GetValue()->AppendValue("[1] Seekable", Flags[1]);
		Result->GetValue()->AppendValue("[2-31] Reserved", false);
		Result->SetSuccess(true);
		for(auto Index = 2; Index < 32; ++Index)
		{
			Result->SetSuccess(Result->GetSuccess() & ~Flags[Index]);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_FilePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FileIDResult{Get_GUID_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("FileID", FileIDResult->GetValue());
	if(FileIDResult->GetSuccess() == true)
	{
		auto FileSizeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("FileSize", FileSizeResult->GetValue());
		if(FileSizeResult->GetSuccess() == true)
		{
			auto CreationDateResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("CreationDate", CreationDateResult->GetValue());
			if(CreationDateResult->GetSuccess() == true)
			{
				auto CreationDate{std::experimental::any_cast< std::uint64_t >(CreationDateResult->GetAny())};
				
				Result->GetValue("CreationDate")->AppendValue("DateTime", Inspection::Get_DateTime_FromMicrosoftFileTime(CreationDate));
				Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("date and time"s);
				Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
				
				auto DataPacketsCountResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("DataPacketsCount", DataPacketsCountResult->GetValue());
				if(DataPacketsCountResult->GetSuccess() == true)
				{
					auto PlayDurationResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("PlayDuration", PlayDurationResult->GetValue());
					if(PlayDurationResult->GetSuccess() == true)
					{
						auto SendDurationResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->AppendValue("SendDuration", SendDurationResult->GetValue());
						if(SendDurationResult->GetSuccess() == true)
						{
							auto PrerollResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->AppendValue("Preroll", PrerollResult->GetValue());
							if(PrerollResult->GetSuccess() == true)
							{
								auto FlagsResult{Get_ASF_FilePropertiesFlags(Buffer)};
								
								Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
								if(FlagsResult->GetSuccess() == true)
								{
									auto MinimumDataPacketSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
									
									Result->GetValue()->AppendValue("MinimumDataPacketSize", MinimumDataPacketSizeResult->GetValue());
									if(MinimumDataPacketSizeResult->GetSuccess() == true)
									{
										auto MaximumDataPacketSizeResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
										
										Result->GetValue()->AppendValue("MaximumDataPacketSize", MaximumDataPacketSizeResult->GetValue());
										if(MaximumDataPacketSizeResult->GetSuccess() == true)
										{
											auto MaximumBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
											
											Result->GetValue()->AppendValue("MaximumBitrate", MaximumBitrateResult->GetValue());
											if(MaximumBitrateResult->GetSuccess() == true)
											{
												Result->SetSuccess(true);
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_HeaderObjectData(Buffer)};
			
			Result->GetValue()->AppendValues(HeaderObjectDataResult->GetValue()->GetValues());
			if(HeaderObjectDataResult->GetSuccess() ==true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto NumberOfHeaderObjects{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("Number of header objects", NumberOfHeaderObjects->GetValue());
	if(NumberOfHeaderObjects->GetSuccess() == true)
	{
		auto Reserved1{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Reserved1", Reserved1->GetValue());
		if((Reserved1->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved1->GetAny()) == 0x01))
		{
			auto Reserved2{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("Reserved2", Reserved2->GetValue());
			if((Reserved2->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(Reserved2->GetAny()) == 0x02))
			{
				Result->SetSuccess(true);
				
				auto HeaderObjects(std::make_shared< Inspection::Value >());
				
				Result->GetValue()->AppendValue("HeaderObjects", HeaderObjects);
				
				auto NumberOfHeaderObjectsValue{std::experimental::any_cast< std::uint32_t >(NumberOfHeaderObjects->GetAny())};
				
				for(auto ObjectIndex = 0ul; ObjectIndex < NumberOfHeaderObjectsValue; ++ObjectIndex)
				{
					auto HeaderObject{Get_ASF_Object(Buffer)};
					
					HeaderObjects->AppendValue("Object", HeaderObject->GetValue());
					if(HeaderObject->GetSuccess() == false)
					{
						Result->SetSuccess(false);
						
						break;
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_IndexPlaceholderObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Length == Inspection::Length(10ull, 0))
	{
		auto DataResult{Get_Bits_Unset_EndedByLength(Buffer, Length)};
		
		if(DataResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("Data", DataResult->GetValue());
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_LanguageIDRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageIDLengthResult{Get_UnsignedInteger_8Bit(Buffer)};
	
	Result->GetValue()->AppendValue("LanguageIDLength", LanguageIDLengthResult->GetValue());
	if(LanguageIDLengthResult->GetSuccess() == true)
	{
		auto LanguageIDLength{std::experimental::any_cast< std::uint8_t >(LanguageIDLengthResult->GetAny())};
		auto LanguageIDResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, LanguageIDLength)};
		
		Result->GetValue()->AppendValue("LanguageID", LanguageIDResult->GetValue());
		Result->SetSuccess(LanguageIDResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageIDRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("LanguageIDRecordsCount", LanguageIDRecordsCountResult->GetValue());
	if(LanguageIDRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto LanguageIDRecordsCount{std::experimental::any_cast< std::uint16_t >(LanguageIDRecordsCountResult->GetAny())};
		
		for(auto LanguageIDRecordIndex = 0; LanguageIDRecordIndex < LanguageIDRecordsCount; ++LanguageIDRecordIndex)
		{
			auto LanguageIDRecordResult{Get_ASF_LanguageIDRecord(Buffer)};
			
			Result->GetValue()->AppendValue("LanguageIDRecord", LanguageIDRecordResult->GetValue());
			if(LanguageIDRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LanguageListIndexResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("LanguageListIndex", LanguageListIndexResult->GetValue());
	if(LanguageListIndexResult->GetSuccess() == true)
	{
		auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("NameLength", NameLengthResult->GetValue());
			if(NameLengthResult->GetSuccess() == true)
			{
				auto DataTypeResult{Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Buffer)};
				
				Result->GetValue()->AppendValue("DataType", DataTypeResult->GetValue());
				if(DataTypeResult->GetSuccess() == true)
				{
					auto DataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("DataLength", DataLengthResult->GetValue());
					if(DataLengthResult->GetSuccess() == true)
					{
						auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
						auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length(NameLength, 0))};
						
						Result->GetValue()->AppendValue("Name", NameResult->GetValue());
						if(NameResult->GetSuccess() == true)
						{
							auto DataLength{std::experimental::any_cast< std::uint32_t >(DataLengthResult->GetAny())};
							auto DataType{std::experimental::any_cast< const std::string & >(DataTypeResult->GetValue()->GetTagAny("interpretation"))};
							auto DataResult{Get_ASF_MetadataLibrary_DescriptionRecord_Data(Buffer, DataType, Inspection::Length(DataLength, 0))};
							
							Result->GetValue()->AppendValue("Data", DataResult->GetValue());
							Result->SetSuccess(DataResult->GetSuccess());
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Buffer & Buffer, const std::string & Type, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Type == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
	}
	else if(Type == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(Type == "Boolean")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
	}
	else if(Type == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else if(Type == "GUID")
	{
		assert(Length == Inspection::Length(16ull, 0));
		
		auto GUIDResult{Get_GUID_LittleEndian(Buffer)};
		
		Result->SetValue(GUIDResult->GetValue());
		Result->SetSuccess(GUIDResult->GetSuccess());
		
		auto GUID{std::experimental::any_cast< const Inspection::GUID & >(GUIDResult->GetAny())};
		auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
		
		Result->GetValue()->AppendTag("interpretation", GUIDInterpretation);
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DataTypeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(DataTypeResult->GetValue());
	if(DataTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DataType{std::experimental::any_cast< std::uint16_t >(DataTypeResult->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->AppendTag("interpretation", "Unicode string"s);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->AppendTag("interpretation", "Byte array"s);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->AppendTag("interpretation", "Boolean"s);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 32bit"s);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 64bit"s);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->AppendTag("interpretation", "Unsigned integer 16bit"s);
		}
		else if(DataType == 0x0006)
		{
			Result->GetValue()->AppendTag("interpretation", "GUID"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "<no interpretation>"s);
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibraryObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DescriptionRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("DescriptionRecordsCount", DescriptionRecordsCountResult->GetValue());
	if(DescriptionRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(DescriptionRecordsCountResult->GetAny())};
		
		for(auto DescriptionRecordsIndex = 0; DescriptionRecordsIndex < DescriptionRecordsCount; ++DescriptionRecordsIndex)
		{
			auto DescriptionRecordResult{Get_ASF_MetadataLibrary_DescriptionRecord(Buffer)};
			
			Result->GetValue()->AppendValue("DescriptionRecord", DescriptionRecordResult->GetValue());
			if(DescriptionRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(2ull, 0))};
	
	Result->GetValue()->AppendValue("Reserved", ReservedResult->GetValue());
	if(ReservedResult->GetSuccess() == true)
	{
		auto StreamNumberResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto NameLengthResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("NameLength", NameLengthResult->GetValue());
			if(NameLengthResult->GetSuccess() == true)
			{
				auto DataTypeResult{Get_ASF_DataType(Buffer)};
				
				Result->GetValue()->AppendValue("DataType", DataTypeResult->GetValue());
				if(DataTypeResult->GetSuccess() == true)
				{
					auto DataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("DataLength", DataLengthResult->GetValue());
					if(DataLengthResult->GetSuccess() == true)
					{
						auto NameLength{std::experimental::any_cast< std::uint16_t >(NameLengthResult->GetAny())};
						auto NameResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, NameLength)};
						
						Result->GetValue()->AppendValue("Name", NameResult->GetValue());
						if(NameResult->GetSuccess() == true)
						{
							auto DataLength{std::experimental::any_cast< std::uint32_t >(DataLengthResult->GetAny())};
							auto DataType{std::experimental::any_cast< std::string >(DataTypeResult->GetValue()->GetTagAny("interpretation"))};
							auto DataValueResult{Get_ASF_MetadataObject_DescriptionRecord_Data(Buffer, DataLength, DataType)};
							
							Result->GetValue()->AppendValue("Data", DataValueResult->GetValue());
							Result->SetSuccess(DataValueResult->GetSuccess());
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(DataType == "Unicode string")
	{
		auto UnicodeStringResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
		
		Result->SetValue(UnicodeStringResult->GetValue());
		Result->SetSuccess(UnicodeStringResult->GetSuccess());
	}
	else if(DataType == "Byte array")
	{
		auto ByteArrayResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
		
		Result->SetValue(ByteArrayResult->GetValue());
		Result->SetSuccess(ByteArrayResult->GetSuccess());
	}
	else if(DataType == "Boolean")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto BooleanResult{Get_ASF_Boolean_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(BooleanResult->GetValue());
		Result->SetSuccess(BooleanResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 32bit")
	{
		assert(Length == Inspection::Length(4ull, 0));
		
		auto UnsignedInteger32BitResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger32BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger32BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 64bit")
	{
		assert(Length == Inspection::Length(8ull, 0));
		
		auto UnsignedInteger64BitResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger64BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger64BitResult->GetSuccess());
	}
	else if(DataType == "Unsigned integer 16bit")
	{
		assert(Length == Inspection::Length(2ull, 0));
		
		auto UnsignedInteger16BitResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->SetValue(UnsignedInteger16BitResult->GetValue());
		Result->SetSuccess(UnsignedInteger16BitResult->GetSuccess());
	}
	else
	{
		Result->GetValue()->SetAny("<unknown type>"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto DescriptionRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("DescriptionRecordsCount", DescriptionRecordsCountResult->GetValue());
	if(DescriptionRecordsCountResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(DescriptionRecordsCountResult->GetAny())};
		
		for(auto DescriptionRecordsIndex = 0; DescriptionRecordsIndex < DescriptionRecordsCount; ++DescriptionRecordsIndex)
		{
			auto DescriptionRecordResult{Get_ASF_MetadataObject_DescriptionRecord(Buffer)};
			
			Result->GetValue()->AppendValue("DescriptionRecord", DescriptionRecordResult->GetValue());
			if(DescriptionRecordResult->GetSuccess() == false)
			{
				Result->SetSuccess(false);
				
				break;
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Object(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->SetValue(ObjectHeaderResult->GetValue());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto Size{std::experimental::any_cast< std::uint64_t >(ObjectHeaderResult->GetAny("Size"))};
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		std::unique_ptr< Inspection::Result > ObjectDataResult;
		
		if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_FilePropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_FilePropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_StreamPropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_CodecListObjectGUID)
		{
			ObjectDataResult = Get_ASF_CodecListObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_HeaderExtensionObjectGUID)
		{
			ObjectDataResult = Get_ASF_HeaderExtensionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_LanguageListObjectGUID)
		{
			ObjectDataResult = Get_ASF_LanguageListObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_ExtendedStreamPropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_ExtendedStreamPropertiesObjectData(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_MetadataObjectGUID)
		{
			ObjectDataResult = Get_ASF_MetadataObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_CompatibilityObjectGUID)
		{
			ObjectDataResult = Get_ASF_CompatibilityObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_IndexPlaceholderObjectGUID)
		{
			ObjectDataResult = Get_ASF_IndexPlaceholderObjectData(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_PaddingObjectGUID)
		{
			auto Length{Inspection::Length(Size) - ObjectHeaderResult->GetValue()->GetLength()};
			
			ObjectDataResult = Get_Bits_Unset_EndedByLength(Buffer, Length);
			Result->GetValue()->AppendValue("Data", ObjectDataResult->GetValue());
		}
		else if(GUID == Inspection::g_ASF_ExtendedContentDescriptionObjectGUID)
		{
			ObjectDataResult = Get_ASF_ExtendedContentDescriptionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_StreamBitratePropertiesObjectGUID)
		{
			ObjectDataResult = Get_ASF_StreamBitratePropertiesObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_ContentDescriptionObjectGUID)
		{
			ObjectDataResult = Get_ASF_ContentDescriptionObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else if(GUID == Inspection::g_ASF_MetadataLibraryObjectGUID)
		{
			ObjectDataResult = Get_ASF_MetadataLibraryObjectData(Buffer);
			Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
		}
		else
		{
			ObjectDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(Size) - ObjectHeaderResult->GetLength());
			Result->GetValue()->AppendValue("Data", ObjectDataResult->GetValue());
		}
		if(ObjectDataResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ObjectHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto GUIDResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->AppendValue("GUID", GUIDResult->GetValue());
	if(GUIDResult->GetSuccess() == true)
	{
		auto SizeResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		if(SizeResult->GetSuccess() == true)
		{
			Result->SetSuccess(true);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FlagsResult{Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Buffer)};
	
	Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		auto AverageBitrateResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("AverageBitrate", AverageBitrateResult->GetValue());
		Result->SetSuccess(AverageBitrateResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{Buffer.GetPosition()};
	auto FlagsResult{Get_BitSet_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Buffer.SetPosition(Position);
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
		
		auto StreamNumberResult{Get_UnsignedInteger_7Bit(Buffer)};
		
		Result->GetValue()->AppendValue("[0-6] StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0ull, 9))};
			
			Result->GetValue()->AppendValue("[7-15] Reserved", ReservedResult->GetValue());
			Result->SetSuccess(ReservedResult->GetSuccess());
		}
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitratePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto BitrateRecordsCountResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
	
	Result->GetValue()->AppendValue("BitrateRecordsCount", BitrateRecordsCountResult->GetValue());
	if(BitrateRecordsCountResult->GetSuccess() == true)
	{
		auto BitrateRecordsCount{std::experimental::any_cast< std::uint16_t >(BitrateRecordsCountResult->GetAny())};
		
		for(auto BitrateRecordsIndex = 0; BitrateRecordsIndex < BitrateRecordsCount; ++BitrateRecordsIndex)
		{
			auto BitrateRecordResult{Get_ASF_StreamBitrateProperties_BitrateRecord(Buffer)};
			
			Result->GetValue()->AppendValue("BitrateRecord", BitrateRecordResult->GetValue());
			Result->SetSuccess(BitrateRecordResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{Buffer.GetPosition()};
	auto FlagsResult{Get_BitSet_16Bit_LittleEndian(Buffer)};
	
	Result->SetValue(FlagsResult->GetValue());
	if(FlagsResult->GetSuccess() == true)
	{
		Buffer.SetPosition(Position);
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
		
		auto StreamNumberResult{Get_UnsignedInteger_7Bit(Buffer)};
		
		Result->GetValue()->AppendValue("[0-6] StreamNumber", StreamNumberResult->GetValue());
		if(StreamNumberResult->GetSuccess() == true)
		{
			auto ReservedResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("[7-14] Reserved", ReservedResult->GetValue());
			if((ReservedResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint8_t >(ReservedResult->GetAny()) == 0x00))
			{
				auto EncryptedContentFlagResult{Get_Boolean_1Bit(Buffer)};
				
				Result->GetValue()->AppendValue("[15] EncryptedContentFlag", EncryptedContentFlagResult->GetValue());
				if(EncryptedContentFlagResult->GetSuccess() == true)
				{
					Result->SetSuccess(true);
				}
			}
		}
		Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FormatTagResult{Get_Microsoft_WaveFormat_FormatTag(Buffer)};
	
	Result->GetValue()->AppendValue("FormatTag", FormatTagResult->GetValue());
	if(FormatTagResult->GetSuccess() == true)
	{
		auto NumberOfChannelsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("NumberOfChannels", NumberOfChannelsResult->GetValue());
		if(NumberOfChannelsResult->GetSuccess() == true)
		{
			auto SamplesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("SamplesPerSecond", SamplesPerSecondResult->GetValue());
			if(SamplesPerSecondResult->GetSuccess() == true)
			{
				auto AverageNumberOfBytesPerSecondResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("AverageNumberOfBytesPerSecond", AverageNumberOfBytesPerSecondResult->GetValue());
				if(AverageNumberOfBytesPerSecondResult->GetSuccess() == true)
				{
					auto BlockAlignmentResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("BlockAlignment", BlockAlignmentResult->GetValue());
					Result->GetValue("BlockAlignment")->PrependTag("block size in bytes"s);
					if(BlockAlignmentResult->GetSuccess() == true)
					{
						auto BitsPerSampleResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
						
						Result->GetValue()->AppendValue("BitsPerSample", BitsPerSampleResult->GetValue());
						if(BitsPerSampleResult->GetSuccess() == true)
						{
							auto CodecSpecificDataSizeResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->AppendValue("CodecSpecificDataSize", CodecSpecificDataSizeResult->GetValue());
							Result->GetValue("CodecSpecificDataSize")->PrependTag("bytes"s);
							if(CodecSpecificDataSizeResult->GetSuccess() == true)
							{
								auto FormatTag{std::experimental::any_cast< const std::string & >(FormatTagResult->GetValue()->GetTagAny("constant name"))};
								auto CodecSpecificDataSize{std::experimental::any_cast< std::uint16_t >(CodecSpecificDataSizeResult->GetAny())};
								std::unique_ptr< Inspection::Result > CodecSpecificDataResult;
								
								if(FormatTag == "WAVE_FORMAT_WMAUDIO2")
								{
									CodecSpecificDataResult = Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Buffer, CodecSpecificDataSize);
								}
								else
								{
									CodecSpecificDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(CodecSpecificDataSize));
								}
								Result->GetValue()->AppendValue("CodecSpecificData", CodecSpecificDataResult->GetValue());
								Result->SetSuccess(CodecSpecificDataResult->GetSuccess());
								Result->GetValue()->AppendTag("AudioMedia"s);
								Result->GetValue()->AppendTag("WAVEFORMATEX"s);
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Length == Inspection::Length(10ull, 0))
	{
		auto SamplesPerBlockResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
		
		Result->GetValue()->AppendValue("SamplesPerBlock", SamplesPerBlockResult->GetValue());
		if(SamplesPerBlockResult->GetSuccess() == true)
		{
			auto EncodeOptionsResult{Get_UnsignedInteger_16Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("EncodeOptions", EncodeOptionsResult->GetValue());
			if(EncodeOptionsResult->GetSuccess() == true)
			{
				auto SuperBlockAlignResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("SuperBlockAlign", SuperBlockAlignResult->GetValue());
				Result->SetSuccess(SuperBlockAlignResult->GetSuccess());
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamPropertiesObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto ObjectHeaderResult{Get_ASF_ObjectHeader(Buffer)};
	
	Result->GetValue()->AppendValues(ObjectHeaderResult->GetValue()->GetValues());
	if(ObjectHeaderResult->GetSuccess() == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(ObjectHeaderResult->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
		{
			auto HeaderObjectDataResult{Get_ASF_StreamPropertiesObjectData(Buffer)};
			
			Result->GetValue()->AppendValues(HeaderObjectDataResult->GetValue()->GetValues());
			if(HeaderObjectDataResult->GetSuccess() ==true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamPropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto StreamTypeResult{Get_ASF_GUID(Buffer)};
	
	Result->GetValue()->AppendValue("StreamType", StreamTypeResult->GetValue());
	if(StreamTypeResult->GetSuccess() == true)
	{
		auto ErrorCorrectionTypeResult{Get_ASF_GUID(Buffer)};
		
		Result->GetValue()->AppendValue("ErrorCorrectionType", ErrorCorrectionTypeResult->GetValue());
		if(ErrorCorrectionTypeResult->GetSuccess() == true)
		{
			auto TimeOffsetResult{Get_UnsignedInteger_64Bit_LittleEndian(Buffer)};
			
			Result->GetValue()->AppendValue("TimeOffset", TimeOffsetResult->GetValue());
			if(TimeOffsetResult->GetSuccess() == true)
			{
				auto TypeSpecificDataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
				
				Result->GetValue()->AppendValue("TypeSpecificDataLength", TypeSpecificDataLengthResult->GetValue());
				if(TypeSpecificDataLengthResult->GetSuccess() == true)
				{
					auto ErrorCorrectionDataLengthResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
					
					Result->GetValue()->AppendValue("ErrorCorrectionDataLength", ErrorCorrectionDataLengthResult->GetValue());
					if(ErrorCorrectionDataLengthResult->GetSuccess() == true)
					{
						auto FlagsResult{Get_ASF_StreamProperties_Flags(Buffer)};
						
						Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
						if(FlagsResult->GetSuccess() == true)
						{
							auto ReservedResult{Get_UnsignedInteger_32Bit_LittleEndian(Buffer)};
							
							Result->GetValue()->AppendValue("Reserved", ReservedResult->GetValue());
							if(ReservedResult->GetSuccess() == true)
							{
								auto TypeSpecificDataLength{std::experimental::any_cast< std::uint32_t >(TypeSpecificDataLengthResult->GetAny())};
								auto StreamType{std::experimental::any_cast< Inspection::GUID >(StreamTypeResult->GetAny())};
								std::unique_ptr< Inspection::Result > TypeSpecificDataResult;
								
								if(StreamType == Inspection::g_ASF_AudioMediaGUID)
								{
									TypeSpecificDataResult = Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Buffer, TypeSpecificDataLength);
								}
								else
								{
									TypeSpecificDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, TypeSpecificDataLength);
								}
								Result->GetValue()->AppendValue("TypeSpecificData", TypeSpecificDataResult->GetValue());
								if(TypeSpecificDataResult->GetSuccess() == true)
								{
									auto ErrorCorrectionDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, std::experimental::any_cast< std::uint32_t >(ErrorCorrectionDataLengthResult->GetAny()))};
									
									Result->GetValue()->AppendValue("ErrorCorrectionData", ErrorCorrectionDataResult->GetValue());
									if(ErrorCorrectionDataResult->GetSuccess() == true)
									{
										Result->SetSuccess(true);
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

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_Set_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		while(Reader.Has(Inspection::Length{0, 1}) == true)
		{
			if(Reader.Get1Bits() == 0x00)
			{
				Continue = false;
				
				break;
			}
		}
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("set data"s);
		Result->GetValue()->AppendTag(to_string_cast(Reader.GetConsumedLength()) + " bytes and bits"s);
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_Unset_UntilByteAlignment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Bits{Buffer.GetPosition().GetBits()};
	
	if(Bits == 0)
	{
		Result = Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 0));
	}
	else
	{
		Result = Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 8 - Bits));
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->AppendTag("until byte alignment"s);
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

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 8}) == true)
	{
		Result->SetSuccess(true);
		
		std::bitset< 8 > Value;
		auto Byte1{Reader.Get8Bits()};
		
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
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_ApplicationBlock_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	auto RegisteredApplicationIdentifierStart{Buffer.GetPosition()};
	auto RegisteredApplicationIdentifierResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	auto RegisteredApplicationIdentifierValue{Result->GetValue()->AppendValue("RegisteredApplicationIdentifier", RegisteredApplicationIdentifierResult->GetValue())};
	
	if(RegisteredApplicationIdentifierResult->GetSuccess() == true)
	{
		auto ApplicationDataStart{Buffer.GetPosition()};
		
		Buffer.SetPosition(RegisteredApplicationIdentifierStart);
		RegisteredApplicationIdentifierResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length(4ul, 0));
		if(RegisteredApplicationIdentifierResult->GetSuccess() == true)
		{
			RegisteredApplicationIdentifierValue->AppendTag("bytes", RegisteredApplicationIdentifierResult->GetAny());
		}
		Buffer.SetPosition(RegisteredApplicationIdentifierStart);
		RegisteredApplicationIdentifierResult = Get_ASCII_String_Printable_EndedByLength(Buffer, Inspection::Length(4ul, 0));
		if(RegisteredApplicationIdentifierResult->GetSuccess() == true)
		{
			RegisteredApplicationIdentifierValue->AppendTag("string interpretation", RegisteredApplicationIdentifierResult->GetAny());
		}
		Buffer.SetPosition(ApplicationDataStart);
		
		auto ApplicationDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		
		Result->GetValue()->AppendValue("ApplicationData", ApplicationDataResult->GetValue());
		Result->SetSuccess(ApplicationDataResult->GetSuccess());
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame(Inspection::Buffer & Buffer, std::uint8_t NumberOfChannelsByStream)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto FrameHeaderResult{Get_FLAC_Frame_Header(Buffer)};
		auto FrameHeaderValue{Result->GetValue()->AppendValue("Header", FrameHeaderResult->GetValue())};
		
		Continue = FrameHeaderResult->GetSuccess();
		if(FrameHeaderResult->GetSuccess() == true)
		{
			auto NumberOfChannelsByFrame{std::experimental::any_cast< std::uint8_t >(FrameHeaderValue->GetValue("ChannelAssignment")->GetTagAny("value"))};
			
			if(NumberOfChannelsByStream != NumberOfChannelsByFrame)
			{
				Result->GetValue()->AppendTag("error", "The number of channels from the stream (" + to_string_cast(NumberOfChannelsByStream) + ") does not match the number of channels from the frame (" + to_string_cast(NumberOfChannelsByFrame) + ").");
				Continue = false;
			}
		}
	}
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint16_t >(Result->GetValue("Header")->GetValue("BlockSize")->GetTagAny("value"))};
		auto BitsPerSample{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValue("SampleSize")->GetTagAny("value"))};
		auto ChannelAssignment{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValueAny("ChannelAssignment"))};
		
		for(auto SubFrameIndex = 0; (Continue == true) && (SubFrameIndex < NumberOfChannelsByStream); ++SubFrameIndex)
		{
			std::unique_ptr< Inspection::Result > SubframeResult;
			
			if(((SubFrameIndex == 0) && (ChannelAssignment == 0x09)) || ((SubFrameIndex == 1) && ((ChannelAssignment == 0x08) || (ChannelAssignment == 0x0a))))
			{
				SubframeResult = Get_FLAC_Subframe(Buffer, BlockSize, BitsPerSample + 1);
			}
			else
			{
				SubframeResult = Get_FLAC_Subframe(Buffer, BlockSize, BitsPerSample);
			}
			Result->GetValue()->AppendValue("Subframe", SubframeResult->GetValue());
			Continue = SubframeResult->GetSuccess();
		}
	}
	if(Continue == true)
	{
		auto PaddingResult{Get_Bits_Unset_UntilByteAlignment(Buffer)};
		
		Result->GetValue()->AppendValue("Padding", PaddingResult->GetValue());
		Continue = PaddingResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto FrameFooterResult{Get_FLAC_Frame_Footer(Buffer)};
		
		Result->GetValue()->AppendValue("Footer", FrameFooterResult->GetValue());
		Continue = FrameFooterResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Footer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto CRC16Result{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("CRC-16", CRC16Result->GetValue());
		Continue = CRC16Result->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto SyncCodeResult{Get_UnsignedInteger_14Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("SyncCode", SyncCodeResult->GetValue());
		Continue = (SyncCodeResult->GetSuccess() == true) && (std::experimental::any_cast< std::uint16_t >(SyncCodeResult->GetAny()) == 0x3ffe);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 1))};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	//reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 1))};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockingStrategy", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto BlockingStrategyValue{Result->GetValue("BlockingStrategy")};
		auto BlockingStrategy{std::experimental::any_cast< std::uint8_t >(BlockingStrategyValue->GetAny())};
		
		if(BlockingStrategy == 0x00)
		{
			BlockingStrategyValue->AppendTag("interpretation", "fixed-blocksize stream; frame header encodes the frame number"s);
		}
		else if(BlockingStrategy == 0x01)
		{
			BlockingStrategyValue->AppendTag("interpretation", "variable-blocksize stream; frame header encodes the sample number"s);
		}
		else
		{
			assert(false);
		}
	}
	
	std::uint8_t BlockSize;
	
	if(Continue == true)
	{
		auto BlockSizeResult{Get_UnsignedInteger_4Bit(Buffer)};
		auto BlockSizeValue{Result->GetValue()->AppendValue("BlockSize", BlockSizeResult->GetValue())};
		
		Continue = BlockSizeResult->GetSuccess();
		if(BlockSizeResult->GetSuccess() == true)
		{
			BlockSize = std::experimental::any_cast< std::uint8_t >(BlockSizeResult->GetAny());
			if(BlockSize == 0x00)
			{
				BlockSizeValue->AppendTag("reserved"s);
				BlockSizeValue->AppendTag("error", "The block size 0 MUST NOT be used."s);
				Continue = false;
			}
			else if(BlockSize == 0x01)
			{
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(192));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
			else if((BlockSize > 0x01) && (BlockSize <= 0x05))
			{
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(576 * (1 << (BlockSize - 2))));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
			else if(BlockSize == 0x06)
			{
				BlockSizeValue->AppendTag("interpretation", "get 8bit (blocksize - 1) from end of header"s);
			}
			else if(BlockSize == 0x07)
			{
				BlockSizeValue->AppendTag("interpretation", "get 16bit (blocksize - 1) from end of header"s);
			}
			else if((BlockSize > 0x07) && (BlockSize < 0x10))
			{
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(256 * (1 << (BlockSize - 8))));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
			else
			{
				assert(false);
			}
		}
	}
	
	std::uint8_t SampleRate;
	
	if(Continue == true)
	{
		auto SampleRateResult{Get_UnsignedInteger_4Bit(Buffer)};
		auto SampleRateValue{Result->GetValue()->AppendValue("SampleRate", SampleRateResult->GetValue())};
		
		Continue = SampleRateResult->GetSuccess();
		if(SampleRateResult->GetSuccess() == true)
		{
			SampleRate = std::experimental::any_cast< std::uint8_t >(SampleRateResult->GetAny());
		}
	}
	if(Continue == true)
	{
		auto ChannelAssignmentResult{Get_UnsignedInteger_4Bit(Buffer)};
		auto ChannelAssignmentValue{Result->GetValue()->AppendValue("ChannelAssignment", ChannelAssignmentResult->GetValue())};
		
		Continue = ChannelAssignmentResult->GetSuccess();
		if(ChannelAssignmentResult->GetSuccess() == true)
		{
			auto ChannelAssignment{std::experimental::any_cast< std::uint8_t >(ChannelAssignmentResult->GetAny())};
			
			if(ChannelAssignment < 0x08)
			{
				ChannelAssignmentValue->AppendTag("value", static_cast< std::uint8_t >(ChannelAssignment + 1));
				ChannelAssignmentValue->AppendTag("interpretation", "independent channels"s);
				switch(ChannelAssignment)
				{
				case 0:
					{
						ChannelAssignmentValue->AppendTag("assignment", "mono"s);
						
						break;
					}
				case 1:
					{
						ChannelAssignmentValue->AppendTag("assignment", "left, right"s);
						
						break;
					}
				case 2:
					{
						ChannelAssignmentValue->AppendTag("assignment", "left, right, center"s);
						
						break;
					}
				case 3:
					{
						ChannelAssignmentValue->AppendTag("assignment", "front left, front right, back left, back right"s);
						
						break;
					}
				case 4:
					{
						ChannelAssignmentValue->AppendTag("assignment", "front left, front right, front center, back/surround left, back/surround right"s);
						
						break;
					}
				case 5:
					{
						ChannelAssignmentValue->AppendTag("assignment", "front left, front right, front center, LFE, back/surround left, back/surround right"s);
						
						break;
					}
				case 6:
					{
						ChannelAssignmentValue->AppendTag("assignment", "front left, front right, front center, LFE, back center, side left, side right"s);
						
						break;
					}
				case 7:
					{
						ChannelAssignmentValue->AppendTag("assignment", "front left, front right, front center, LFE, back left, back right, side left, side right"s);
						
						break;
					}
				default:
					{
						assert(false);
					}
				}
			}
			else if(ChannelAssignment == 0x08)
			{
				ChannelAssignmentValue->AppendTag("value", static_cast< std::uint8_t >(2));
				ChannelAssignmentValue->AppendTag("interpretation", "left/side stereo"s);
				ChannelAssignmentValue->AppendTag("assignment", "channel 0 is the left channel, channel 1 is the side (difference) channel"s);
			}
			else if(ChannelAssignment == 0x09)
			{
				ChannelAssignmentValue->AppendTag("value", static_cast< std::uint8_t >(2));
				ChannelAssignmentValue->AppendTag("interpretation", "right/side stereo"s);
				ChannelAssignmentValue->AppendTag("assignment", "channel 0 is the side (difference) channel, channel 1 is the right channel"s);
			}
			else if(ChannelAssignment == 0x0a)
			{
				ChannelAssignmentValue->AppendTag("value", static_cast< std::uint8_t >(2));
				ChannelAssignmentValue->AppendTag("interpretation", "mid/side stereo"s);
				ChannelAssignmentValue->AppendTag("assignment", "channel 0 is the mid (average) channel, channel 1 is the side (difference) channel"s);
			}
			else
			{
				ChannelAssignmentValue->AppendTag("reserved"s);
				ChannelAssignmentValue->AppendTag("error", "The channel assignment " + to_string_cast(ChannelAssignment) + " MUST NOT be used.");
			}
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 3))};
		auto FieldResult{Get_UnsignedInteger_3Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SampleSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto SampleSizeValue{Result->GetValue("SampleSize")};
		auto SampleSize{std::experimental::any_cast< std::uint8_t >(SampleSizeValue->GetAny())};
		
		if(SampleSize == 0x00)
		{
			SampleSizeValue->AppendTag("interpretation", "get from STREAMINFO metadata block"s);
		}
		else if(SampleSize == 0x01)
		{
			SampleSizeValue->AppendTag("value", static_cast< std::uint8_t >(8));
			SampleSizeValue->AppendTag("unit", "bits"s);
		}
		else if(SampleSize == 0x02)
		{
			SampleSizeValue->AppendTag("value", static_cast< std::uint8_t >(12));
			SampleSizeValue->AppendTag("unit", "bits"s);
		}
		else if(SampleSize == 0x03)
		{
			SampleSizeValue->AppendTag("reserved"s);
			SampleSizeValue->AppendTag("error", "The block size 0 MUST NOT be used."s);
			Continue = false;
		}
		else if(SampleSize == 0x04)
		{
			SampleSizeValue->AppendTag("value", static_cast< std::uint8_t >(16));
			SampleSizeValue->AppendTag("unit", "bits"s);
		}
		else if(SampleSize == 0x05)
		{
			SampleSizeValue->AppendTag("value", static_cast< std::uint8_t >(24));
			SampleSizeValue->AppendTag("unit", "bits"s);
		}
		else if(SampleSize == 0x06)
		{
			SampleSizeValue->AppendTag("value", static_cast< std::uint8_t >(32));
			SampleSizeValue->AppendTag("unit", "bits"s);
		}
		else if(SampleSize == 0x07)
		{
			SampleSizeValue->AppendTag("reserved"s);
			SampleSizeValue->AppendTag("error", "The block size 0 MUST NOT be used."s);
			Continue = false;
		}
		else
		{
			assert(false);
		}
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 1))};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto BlockingStrategy{std::experimental::any_cast< std::uint8_t >(Result->GetAny("BlockingStrategy"))};
		std::unique_ptr< Inspection::Result > CodedNumberResult;
		
		if(BlockingStrategy == 0x00)
		{
			CodedNumberResult = Get_UnsignedInteger_31Bit_UTF_8_Coded(Buffer);
			CodedNumberResult->GetValue()->SetName("FrameNumber");
		}
		else if(BlockingStrategy == 0x01)
		{
			CodedNumberResult = Get_UnsignedInteger_36Bit_UTF_8_Coded(Buffer);
			CodedNumberResult->GetValue()->SetName("SampleNumber");
		}
		Result->GetValue()->AppendValue(CodedNumberResult->GetValue());
		Continue = CodedNumberResult->GetSuccess();
	}
	if(Continue == true)
	{
		if(BlockSize == 0x06)
		{
			auto BlockSizeResult{Get_UnsignedInteger_8Bit(Buffer)};
			
			Result->GetValue()->AppendValue("BlockSizeExplicit", BlockSizeResult->GetValue());
			Continue = BlockSizeResult->GetSuccess();
			if(BlockSizeResult->GetSuccess() == true)
			{
				auto BlockSizeValue{Result->GetValue("BlockSize")};
				
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(std::experimental::any_cast< std::uint8_t >(BlockSizeResult->GetAny())));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
		}
		else if(BlockSize == 0x07)
		{
			auto BlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("BlockSizeExplicit", BlockSizeResult->GetValue());
			Continue = BlockSizeResult->GetSuccess();
			if(BlockSizeResult->GetSuccess() == true)
			{
				auto BlockSizeValue{Result->GetValue("BlockSize")};
				
				BlockSizeValue->AppendTag("value", std::experimental::any_cast< std::uint16_t >(BlockSizeResult->GetAny()));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
		}
	}
	if(Continue == true)
	{
		if(SampleRate == 0xc0)
		{
			throw NotImplementedException("get 8bit sample rate in Hz");
		}
		else if(SampleRate == 0xd0)
		{
			throw NotImplementedException("get 16bit sample rate in Hz");
		}
		else if(SampleRate == 0xe0)
		{
			throw NotImplementedException("get 8bit sample rate in tens of Hz");
		}
	}
	if(Continue == true)
	{
		auto CRC8Result{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("CRC-8", CRC8Result->GetValue());
		Continue = CRC8Result->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->AppendValue("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetTagAny("interpretation"))};
		auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(MetaDataBlockHeaderResult->GetAny("Length"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->AppendValue("Data", StreamInfoBlockDataResult->GetValue());
			Result->SetSuccess(StreamInfoBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto PaddingBlockDataResult{Get_Bits_Unset_EndedByLength(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->AppendValue("Data", PaddingBlockDataResult->GetValue());
			Result->SetSuccess(PaddingBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Application")
		{
			auto ApplicationBlockDataResult{Get_FLAC_ApplicationBlock_Data(Buffer, MetaDataBlockDataLength)};
			
			Result->GetValue()->AppendValue("Data", ApplicationBlockDataResult->GetValue());
			Result->SetSuccess(ApplicationBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			if(MetaDataBlockDataLength % 18 == 0)
			{
				auto SeekTableBlockDataResult{Get_FLAC_SeekTableBlock_Data(Buffer, MetaDataBlockDataLength / 18)};
				
				Result->GetValue()->AppendValue("Data", SeekTableBlockDataResult->GetValue());
				Result->SetSuccess(SeekTableBlockDataResult->GetSuccess());
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto VorbisCommentBlockDataResult{Get_FLAC_VorbisCommentBlock_Data(Buffer)};
			
			Result->GetValue()->AppendValue("Data", VorbisCommentBlockDataResult->GetValue());
			Result->SetSuccess(VorbisCommentBlockDataResult->GetSuccess());
		}
		else if(MetaDataBlockType == "Picture")
		{
			auto PictureBlockDataResult{Get_FLAC_PictureBlock_Data(Buffer)};
			
			Result->GetValue()->AppendValue("Data", PictureBlockDataResult->GetValue());
			Result->SetSuccess(PictureBlockDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto LastMetaDataBlockResult{Get_Boolean_1Bit(Buffer)};
	
	Result->GetValue()->AppendValue("LastMetaDataBlock", LastMetaDataBlockResult->GetValue());
	if(LastMetaDataBlockResult->GetSuccess() == true)
	{
		auto MetaDataBlockTypeResult{Get_FLAC_MetaDataBlock_Type(Buffer)};
		
		Result->GetValue()->AppendValue("BlockType", MetaDataBlockTypeResult->GetValue());
		if(MetaDataBlockTypeResult->GetSuccess() == true)
		{
			auto LengthResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("Length", LengthResult->GetValue());
			if(LengthResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockTypeResult{Get_UnsignedInteger_7Bit(Buffer)};
	
	Result->SetValue(MetaDataBlockTypeResult->GetValue());
	if(MetaDataBlockTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockTypeResult->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Result->GetValue()->AppendTag("interpretation", "StreamInfo"s);
		}
		else if(NumericValue == 0x01)
		{
			Result->GetValue()->AppendTag("interpretation", "Padding"s);
		}
		else if(NumericValue == 0x02)
		{
			Result->GetValue()->AppendTag("interpretation", "Application"s);
		}
		else if(NumericValue == 0x03)
		{
			Result->GetValue()->AppendTag("interpretation", "SeekTable"s);
		}
		else if(NumericValue == 0x04)
		{
			Result->GetValue()->AppendTag("interpretation", "VorbisComment"s);
		}
		else if(NumericValue == 0x05)
		{
			Result->GetValue()->AppendTag("interpretation", "CueSheet"s);
		}
		else if(NumericValue == 0x06)
		{
			Result->GetValue()->AppendTag("interpretation", "Picture"s);
		}
		else if(NumericValue == 0xff)
		{
			Result->GetValue()->AppendTag("interpretation", "Invalid"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "Reserved"s);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_PictureBlock_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
	
	Result->SetValue(PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		Result->SetSuccess(true);
		
		auto PictureType{std::experimental::any_cast< std::uint32_t >(PictureTypeResult->GetAny())};
		
		if(PictureType == 0)
		{
			Result->GetValue()->AppendTag("interpretation", "Other"s);
		}
		else if(PictureType == 1)
		{
			Result->GetValue()->AppendTag("interpretation", "32x32 pixels 'file icon' (PNG only)"s);
		}
		else if(PictureType == 2)
		{
			Result->GetValue()->AppendTag("interpretation", "Other file icon"s);
		}
		else if(PictureType == 3)
		{
			Result->GetValue()->AppendTag("interpretation", "Cover (front)"s);
		}
		else if(PictureType == 4)
		{
			Result->GetValue()->AppendTag("interpretation", "Cover (back)"s);
		}
		else if(PictureType == 5)
		{
			Result->GetValue()->AppendTag("interpretation", "Leaflet page"s);
		}
		else if(PictureType == 6)
		{
			Result->GetValue()->AppendTag("interpretation", "Media (e.g. label side of CD"s);
		}
		else if(PictureType == 7)
		{
			Result->GetValue()->AppendTag("interpretation", "Lead artist/lead performer/soloist"s);
		}
		else if(PictureType == 8)
		{
			Result->GetValue()->AppendTag("interpretation", "Artist/performer"s);
		}
		else if(PictureType == 9)
		{
			Result->GetValue()->AppendTag("interpretation", "Conductor"s);
		}
		else if(PictureType == 10)
		{
			Result->GetValue()->AppendTag("interpretation", "Band/Orchestra"s);
		}
		else if(PictureType == 11)
		{
			Result->GetValue()->AppendTag("interpretation", "Composer"s);
		}
		else if(PictureType == 12)
		{
			Result->GetValue()->AppendTag("interpretation", "Lyricist/text writer"s);
		}
		else if(PictureType == 13)
		{
			Result->GetValue()->AppendTag("interpretation", "Recording Location"s);
		}
		else if(PictureType == 14)
		{
			Result->GetValue()->AppendTag("interpretation", "During recording"s);
		}
		else if(PictureType == 15)
		{
			Result->GetValue()->AppendTag("interpretation", "During performance"s);
		}
		else if(PictureType == 16)
		{
			Result->GetValue()->AppendTag("interpretation", "Movie/video screen capture"s);
		}
		else if(PictureType == 17)
		{
			Result->GetValue()->AppendTag("interpretation", "A bright coloured fish"s);
		}
		else if(PictureType == 18)
		{
			Result->GetValue()->AppendTag("interpretation", "Illustration"s);
		}
		else if(PictureType == 19)
		{
			Result->GetValue()->AppendTag("interpretation", "Band/artist logotype"s);
		}
		else if(PictureType == 20)
		{
			Result->GetValue()->AppendTag("interpretation", "Publisher/Studio logotype"s);
		}
		else
		{
			Result->SetSuccess(false);
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_PictureBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto PictureTypeResult{Get_FLAC_PictureBlock_PictureType(Buffer)};
	
	Result->GetValue()->AppendValue("PictureType", PictureTypeResult->GetValue());
	if(PictureTypeResult->GetSuccess() == true)
	{
		auto MIMETypeLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("MIMETypeLength", MIMETypeLengthResult->GetValue());
		if(MIMETypeLengthResult->GetSuccess() == true)
		{
			auto MIMETypeLength{std::experimental::any_cast< std::uint32_t >(MIMETypeLengthResult->GetAny())};
			auto MIMETypeResult{Get_ASCII_String_Printable_EndedByLength(Buffer, Inspection::Length(MIMETypeLength, 0))};
			
			Result->GetValue()->AppendValue("MIMType", MIMETypeResult->GetValue());
			if(MIMETypeResult->GetSuccess() == true)
			{
				auto DescriptionLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
				
				Result->GetValue()->AppendValue("DescriptionLength", DescriptionLengthResult->GetValue());
				if(DescriptionLengthResult->GetSuccess() == true)
				{
					auto DescriptionLength{std::experimental::any_cast< std::uint32_t >(DescriptionLengthResult->GetAny())};
					auto DescriptionResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, DescriptionLength)};
					
					Result->GetValue()->AppendValue("Description", DescriptionResult->GetValue());
					if(DescriptionResult->GetSuccess() == true)
					{
						auto PictureWidthInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
						
						Result->GetValue()->AppendValue("PictureWidthInPixels", PictureWidthInPixelsResult->GetValue());
						if(PictureWidthInPixelsResult->GetSuccess() == true)
						{
							auto PictureHeightInPixelsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
							
							Result->GetValue()->AppendValue("PictureHeightInPixels", PictureHeightInPixelsResult->GetValue());
							if(PictureHeightInPixelsResult->GetSuccess() == true)
							{
								auto BitsPerPixelResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
								
								Result->GetValue()->AppendValue("BitsPerPixel", BitsPerPixelResult->GetValue());
								if(BitsPerPixelResult->GetSuccess() == true)
								{
									auto NumberOfColorsResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
									
									Result->GetValue()->AppendValue("NumberOfColors", NumberOfColorsResult->GetValue());
									if(NumberOfColorsResult->GetSuccess() == true)
									{
										auto PictureDataLengthResult{Get_UnsignedInteger_32Bit_BigEndian(Buffer)};
										
										Result->GetValue()->AppendValue("PictureDataLength", PictureDataLengthResult->GetValue());
										if(PictureDataLengthResult->GetSuccess() == true)
										{
											auto PictureDataLength{std::experimental::any_cast< std::uint32_t >(PictureDataLengthResult->GetAny())};
											auto PictureDataResult{Get_Bits_SetOrUnset_EndedByLength(Buffer, Inspection::Length(PictureDataLength, 0))};
											
											Result->GetValue()->AppendValue("PictureData", PictureDataResult->GetValue());
											Result->SetSuccess(PictureDataResult->GetSuccess());
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

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_SeekTableBlock_Data(Inspection::Buffer & Buffer, std::uint32_t NumberOfSeekPoints)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->SetSuccess(true);
	for(auto SeekPointIndex = 0ul; SeekPointIndex < NumberOfSeekPoints; ++SeekPointIndex)
	{
		auto SeekPointResult{Get_FLAC_SeekTableBlock_SeekPoint(Buffer)};
		
		Result->GetValue()->AppendValue("SeekPoint", SeekPointResult->GetValue());
		if(SeekPointResult->GetSuccess() == false)
		{
			Result->SetSuccess(false);
			
			break;
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_SeekTableBlock_SeekPoint(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto SampleNumberOfFirstSampleInTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
	
	Result->GetValue()->AppendValue("SampleNumberOfFirstSampleInTargetFrame", SampleNumberOfFirstSampleInTargetFrameResult->GetValue());
	if(SampleNumberOfFirstSampleInTargetFrameResult->GetSuccess() == true)
	{
		auto ByteOffsetOfTargetFrameResult{Get_UnsignedInteger_64Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("ByteOffsetOfTargetFrame", ByteOffsetOfTargetFrameResult->GetValue());
		if(ByteOffsetOfTargetFrameResult->GetSuccess() == true)
		{
			auto NumberOfSamplesInTargetFrameResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
			
			Result->GetValue()->AppendValue("NumberOfSamplesInTargetFrame", NumberOfSamplesInTargetFrameResult->GetValue());
			Result->SetSuccess(NumberOfSamplesInTargetFrameResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Stream(Inspection::Buffer & Buffer, bool OnlyStreamHeader)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto FLACStreamMarkerResult{Get_ASCII_String_Alphabetical_EndedByTemplateLength(Buffer, "fLaC")};
		
		Result->GetValue()->AppendValue("FLAC stream marker", FLACStreamMarkerResult->GetValue());
		Continue = FLACStreamMarkerResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		Result->GetValue()->AppendValue("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
		Continue = FLACStreamInfoBlockResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto LastMetaDataBlock{std::experimental::any_cast< bool >(Result->GetValue("StreamInfoBlock")->GetValue("Header")->GetValueAny("LastMetaDataBlock"))};
		
		while((LastMetaDataBlock == false) && (Continue == true))
		{
			auto MetaDataBlockResult{Get_FLAC_MetaDataBlock(Buffer)};
			
			Result->GetValue()->AppendValue("MetaDataBlock", MetaDataBlockResult->GetValue());
			Continue = MetaDataBlockResult->GetSuccess();
			if(MetaDataBlockResult->GetSuccess() == true)
			{
				LastMetaDataBlock = std::experimental::any_cast< bool >(MetaDataBlockResult->GetValue("Header")->GetValueAny("LastMetaDataBlock"));
			}
		}
	}
	if((Continue == true) && (OnlyStreamHeader == false))
	{
		auto NumberOfChannels{std::experimental::any_cast< std::uint8_t >(Result->GetValue("StreamInfoBlock")->GetValue("Data")->GetValueAny("NumberOfChannels")) + 1};
		auto FramesResult{Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Buffer, std::bind(Get_FLAC_Frame, std::placeholders::_1, NumberOfChannels), Buffer.GetLength() - Buffer.GetPosition())};
		auto FramesValue{Result->GetValue()->AppendValue("Frames", FramesResult->GetValue())};
		
		Continue = FramesResult->GetSuccess();
		for(auto FrameValue : FramesValue->GetValues())
		{
			FrameValue->SetName("Frame");
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto MetaDataBlockHeaderResult{Get_FLAC_MetaDataBlock_Header(Buffer)};
	
	Result->GetValue()->AppendValue("Header", MetaDataBlockHeaderResult->GetValue());
	if(MetaDataBlockHeaderResult->GetSuccess() == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(MetaDataBlockHeaderResult->GetValue("BlockType")->GetTagAny("interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto StreamInfoBlockDataResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			
			Result->GetValue()->AppendValue("Data", StreamInfoBlockDataResult->GetValue());
			Result->SetSuccess(StreamInfoBlockDataResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock_BitsPerSample(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_5Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetAny()) + 1));
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto MinimumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("MinimumBlockSize", MinimumBlockSizeResult->GetValue());
		Continue = MinimumBlockSizeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto MaximumBlockSizeResult{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("MaximumBlockSize", MaximumBlockSizeResult->GetValue());
		Continue = MaximumBlockSizeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto MinimumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("MinimumFrameSize", MinimumFrameSizeResult->GetValue());
		Continue = MinimumFrameSizeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto MaximumFrameSizeResult{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("MaximumFrameSize", MaximumFrameSizeResult->GetValue());
		Continue = MaximumFrameSizeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto SampleRateResult{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
		
		Result->GetValue()->AppendValue("SampleRate", SampleRateResult->GetValue());
		Continue = SampleRateResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 3))};
		auto FieldResult{Get_FLAC_StreamInfoBlock_NumberOfChannels(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 5))};
		auto FieldResult{Get_FLAC_StreamInfoBlock_BitsPerSample(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 36))};
		auto FieldResult{Get_UnsignedInteger_36Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TotalSamplesPerChannel", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto MD5SignatureOfUnencodedAudioDataResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, 16)};
		
		Result->GetValue()->AppendValue("MD5SignatureOfUnencodedAudioData", MD5SignatureOfUnencodedAudioDataResult->GetValue());
		Continue = MD5SignatureOfUnencodedAudioDataResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock_NumberOfChannels(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_3Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("interpretation", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetAny()) + 1));
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	std::shared_ptr< Inspection::Value > SubframeHeaderValue;
	
	if(Continue == true)
	{
		auto SubframeHeaderResult{Get_FLAC_Subframe_Header(Buffer)};
		
		SubframeHeaderValue = Result->GetValue()->AppendValue("Header", SubframeHeaderResult->GetValue());
		Continue = SubframeHeaderResult->GetSuccess();
	}
	if(Continue == true)
	{
		try
		{
			auto SubframeType{std::experimental::any_cast< const std::string & >(SubframeHeaderValue->GetValue("Type")->GetTagAny("interpretation"))};
			
			std::unique_ptr< Inspection::Result > SubframeDataResult;
			
			if(SubframeType == "SUBFRAME_CONSTANT")
			{
				SubframeDataResult = Get_FLAC_Subframe_Data_Constant(Buffer, BitsPerSample);
			}
			else if(SubframeType == "SUBFRAME_FIXED")
			{
				auto Order{static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(SubframeHeaderValue->GetValue("Type")->GetValueAny("Order")))};
				
				SubframeDataResult = Get_FLAC_Subframe_Data_Fixed(Buffer, FrameBlockSize, BitsPerSample, Order);
			}
			else if(SubframeType == "SUBFRAME_LPC")
			{
				auto Order{static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(SubframeHeaderValue->GetValue("Type")->GetValueAny("Order")) + 1)};
				
				SubframeDataResult = Get_FLAC_Subframe_Data_LPC(Buffer, FrameBlockSize, BitsPerSample, Order);
			}
			else
			{
				throw Inspection::NotImplementedException(SubframeType);
			}
			Result->GetValue()->AppendValue("Data", SubframeDataResult->GetValue());
			Continue = SubframeDataResult->GetSuccess();
		}
		catch(std::invalid_argument & Exception)
		{
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Constant(Inspection::Buffer & Buffer, std::uint8_t BitsPerSample)
{
	return Get_UnsignedInteger_BigEndian(Buffer, BitsPerSample);
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Fixed(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto WarmUpSamplesResult{Get_UnsignedIntegers_BigEndian(Buffer, BitsPerSample, PredictorOrder)};
		
		Result->GetValue()->AppendValue("WarmUpSamples", WarmUpSamplesResult->GetValue());
		Continue = WarmUpSamplesResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto ResidualResult{Get_FLAC_Subframe_Residual(Buffer, FrameBlockSize, PredictorOrder)};
		
		Result->GetValue()->AppendValue("Residual", ResidualResult->GetValue());
		Continue = ResidualResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_LPC(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto WarmUpSamplesResult{Get_UnsignedIntegers_BigEndian(Buffer, BitsPerSample, PredictorOrder)};
		
		Result->GetValue()->AppendValue("WarmUpSamples", WarmUpSamplesResult->GetValue());
		Continue = WarmUpSamplesResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto QuantizedLinearPredictorCoefficientsPrecisionResult{Get_UnsignedInteger_4Bit(Buffer)};
		auto QuantizedLinearPredictorCoefficientsPrecisionValue{Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientsPrecision", QuantizedLinearPredictorCoefficientsPrecisionResult->GetValue())};
		
		Continue = QuantizedLinearPredictorCoefficientsPrecisionResult->GetSuccess();
		if(QuantizedLinearPredictorCoefficientsPrecisionResult->GetSuccess() == true)
		{
			auto QuantizedLinearPredictorCoefficientsPrecision{std::experimental::any_cast< std::uint8_t >(QuantizedLinearPredictorCoefficientsPrecisionResult->GetAny())};
			
			if(QuantizedLinearPredictorCoefficientsPrecision < 15)
			{
				QuantizedLinearPredictorCoefficientsPrecisionValue->AppendTag("value", static_cast< std::uint8_t >(QuantizedLinearPredictorCoefficientsPrecision + 1));
			}
			else
			{
				QuantizedLinearPredictorCoefficientsPrecisionValue->AppendTag("error", "The percision MUST NOT be 15."s);
				Continue = false;
			}
		}
	}
	if(Continue == true)
	{
		auto QuantizedLinearPredictorCoefficientShiftResult{Get_SignedInteger_5Bit(Buffer)};
		
		Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientShift", QuantizedLinearPredictorCoefficientShiftResult->GetValue());
		Continue = QuantizedLinearPredictorCoefficientShiftResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto PredictorCoefficientsResult{Get_SignedIntegers_BigEndian(Buffer, std::experimental::any_cast< std::uint8_t >(Result->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->GetTagAny("value")), PredictorOrder)};
		
		Result->GetValue()->AppendValue("PredictorCoefficients", PredictorCoefficientsResult->GetValue());
		Continue = PredictorCoefficientsResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto ResidualResult{Get_FLAC_Subframe_Residual(Buffer, FrameBlockSize, PredictorOrder)};
		
		Result->GetValue()->AppendValue("Residual", ResidualResult->GetValue());
		Continue = ResidualResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto PaddingBitResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 1))};
		
		Result->GetValue()->AppendValue("PaddingBit", PaddingBitResult->GetValue());
		Continue = PaddingBitResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto TypeResult{Get_FLAC_Subframe_Type(Buffer)};
		
		Result->GetValue()->AppendValue("Type", TypeResult->GetValue());
		Continue = TypeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto WastedBitsPerSampleFlagResult{Get_Boolean_1Bit(Buffer)};
		
		Result->GetValue()->AppendValue("WastedBitsPerSampleFlag", WastedBitsPerSampleFlagResult->GetValue());
		
		auto WastedBitsPerSampleFlag{std::experimental::any_cast< bool >(WastedBitsPerSampleFlagResult->GetAny())};
		
		if(WastedBitsPerSampleFlag == true)
		{
			throw Inspection::NotImplementedException("Wasted bits are not implemented yet!");
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_FLAC_Subframe_Residual_CodingMethod(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodingMethod", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto CodingMethod{std::experimental::any_cast< std::uint8_t >(Result->GetAny("CodingMethod"))};
		std::unique_ptr< Inspection::Result > CodedResidualResult;
		
		if(CodingMethod == 0x00)
		{
			CodedResidualResult = Get_FLAC_Subframe_Residual_Rice(Buffer, FrameBlockSize, PredictorOrder);
			CodedResidualResult->GetValue()->AppendTag("Rice"s);
		}
		else if(CodingMethod == 0x01)
		{
			CodedResidualResult = Get_FLAC_Subframe_Residual_Rice2(Buffer, FrameBlockSize, PredictorOrder);
			CodedResidualResult->GetValue()->AppendTag("Rice2"s);
		}
		Result->GetValue()->AppendValue("CodedResidual", CodedResidualResult->GetValue());
		Continue = CodedResidualResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_CodingMethod(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto CodingMethod{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(CodingMethod == 0x00)
		{
			Result->GetValue()->AppendTag("interpretation", "partitioned Rice coding with 4-bit Rice parameter"s);
		}
		else if(CodingMethod == 0x01)
		{
			Result->GetValue()->AppendTag("interpretation", "partitioned Rice coding with 5-bit Rice parameter"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "<reserved>"s);
			Result->GetValue()->AppendTag("error", "This coding method MUST NOT be used for the residual."s);
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto PartitionOrderResult{Get_UnsignedInteger_4Bit(Buffer)};
		auto PartitionOrderValue{Result->GetValue()->AppendValue("PartitionOrder", PartitionOrderResult->GetValue())};
		
		Continue = PartitionOrderResult->GetSuccess();
		if(PartitionOrderResult->GetSuccess() == true)
		{
			auto NumberOfPartitions{static_cast< std::uint16_t >(1 << std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValueAny("PartitionOrder")))};
			
			PartitionOrderValue->AppendTag("number of partitions", NumberOfPartitions);
		}
	}
	if(Continue == true)
	{
		auto NumberOfPartitions{std::experimental::any_cast< std::uint16_t >(Result->GetValue("PartitionOrder")->GetTagAny("number of partitions"))};
		auto PartitionsResult{Get_Array_EndedByNumberOfElements_PassArrayIndex(Buffer, std::bind(Get_FLAC_Subframe_Residual_Rice_Partition, std::placeholders::_1, std::placeholders::_2, FrameBlockSize / NumberOfPartitions, PredictorOrder), NumberOfPartitions)};
		auto PartitionsValue{Result->GetValue()->AppendValue("Partitions", PartitionsResult->GetValue())};
		
		for(auto PartitionValue : PartitionsValue->GetValues())
		{
			PartitionValue->SetName("Partition");
		}
		Continue = PartitionsResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Buffer & Buffer, std::uint64_t ArrayIndex, std::uint32_t NumberOfSamples, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto RiceParameterResult{Get_UnsignedInteger_4Bit(Buffer)};
		
		Result->GetValue()->AppendValue("RiceParameter", RiceParameterResult->GetValue());
		Continue = RiceParameterResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto RiceParameter{std::experimental::any_cast< std::uint8_t >(Result->GetAny("RiceParameter"))};
		std::unique_ptr< Inspection::Result > SamplesResult;
		
		if(ArrayIndex == 0)
		{
			SamplesResult = Get_Array_EndedByNumberOfElements(Buffer, std::bind(Get_SignedInteger_32Bit_RiceEncoded, std::placeholders::_1, RiceParameter), NumberOfSamples - PredictorOrder);
		}
		else
		{
			SamplesResult = Get_Array_EndedByNumberOfElements(Buffer, std::bind(Get_SignedInteger_32Bit_RiceEncoded, std::placeholders::_1, RiceParameter), NumberOfSamples);
		}
		// Result->GetValue()->AppendValues(SamplesResult->GetValue()->GetValues());
		Continue = SamplesResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice2(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	throw Inspection::NotImplementedException("Get_FLAC_Subframe_Residual_Rice2");
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto SubframeTypeResult{Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Buffer, Inspection::Length(0, 6))};
		
		Result->SetValue(SubframeTypeResult->GetValue());
		Continue = SubframeTypeResult->GetSuccess();
		if(SubframeTypeResult->GetSuccess() == true)
		{
			auto SubframeType{std::experimental::any_cast< std::uint8_t >(SubframeTypeResult->GetAny())};
			
			switch(SubframeType)
			{
			case 0:
				{
					Result->GetValue()->AppendTag("interpretation", "SUBFRAME_LPC"s);
					
					auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 5))};
					auto FieldResult{Get_UnsignedInteger_5Bit(FieldReader)};
					auto FieldValue{Result->GetValue()->AppendValue("Order", FieldResult->GetValue())};
					
					UpdateState(Continue, Buffer, FieldResult, FieldReader);
					if(FieldResult->GetSuccess() == true)
					{
						auto Order{std::experimental::any_cast< std::uint8_t >(FieldValue->GetAny())};
						
						FieldValue->AppendTag("value", static_cast< std::uint8_t >(Order + 1));
					}
					
					break;
				}
			case 2:
				{
					auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 3))};
					auto FieldResult{Get_UnsignedInteger_3Bit(FieldReader)};
					auto FieldValue{Result->GetValue()->AppendValue("Order", FieldResult->GetValue())};
					
					UpdateState(Continue, Buffer, FieldResult, FieldReader);
					if(FieldResult->GetSuccess() == true)
					{
						auto Order{std::experimental::any_cast< std::uint8_t >(FieldResult->GetAny())};
						
						FieldValue->AppendTag("value", static_cast< std::uint8_t >(Order));
						if(Order < 5)
						{
							Result->GetValue()->AppendTag("interpretation", "SUBFRAME_FIXED"s);
						}
						else
						{
							Result->GetValue()->AppendTag("reserved");
							Result->GetValue()->AppendTag("error", "The subframe type is SUBFRAME_FIXED, and the order " + to_string_cast(Order) + " MUST NOT be used.");
							Continue = false;
						}
					}
					
					break;
				}
			case 1:
			case 3:
			case 4:
				{
					Result->GetValue()->AppendTag("reserved"s);
					Result->GetValue()->AppendTag("error", "This subframe type MUST NOT be used."s);
					Continue = false;
					
					break;
				}
			case 5:
				{
					throw Inspection::NotImplementedException("SUBFRAME_VERBATIM");
				}
			case 6:
				{
					Result->GetValue()->AppendTag("interpretation", "SUBFRAME_CONSTANT"s);
					
					break;
				}
			default:
				{
					assert(false);
				}
			}
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer)
{
	return Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetAny())};
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
			Continue ^= ~Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
				Result->GetValue()->PrependTag("claimed size", to_string_cast(Size));
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "The frame size is claimed larger than the actually handled size."s);
			}
			else if(BodyResult->GetLength() < Size)
			{
				Result->GetValue()->PrependTag("claimed size", to_string_cast(Size));
				Result->GetValue()->PrependTag("handled size", to_string_cast(BodyResult->GetLength()));
				Result->GetValue()->PrependTag("error", "The frame size is claimed smaller than the actually handled size."s);
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
	auto Continue{true};
	
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_IEEE_60559_2011_binary32(Buffer)};
		
		Result->GetValue()->AppendValue("PeakAmplitude", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 16))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment(FieldReader)};
		
		Result->GetValue()->AppendValue("TrackReplayGainAdjustment", FieldResult->GetValue());
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 16))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment(FieldReader)};
		
		Result->GetValue()->AppendValue("AlbumReplayGainAdjustment", FieldResult->GetValue());
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	Result->SetSuccess(Continue);
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
			auto FlagsResult{Get_ID3_2_3_Frame_Header_Flags(Buffer)};
			
			Result->GetValue()->AppendValue("Flags", FlagsResult->GetValue());
			Result->SetSuccess(FlagsResult->GetSuccess());
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_16Bit_BigEndian(Buffer)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		Continue = FieldResult->GetSuccess();
		if(FieldResult->GetSuccess() == true)
		{
			const std::bitset< 16 > & Flags{std::experimental::any_cast< const std::bitset< 16 > & >(FieldValue->GetAny())};
			std::shared_ptr< Inspection::Value > FlagValue;
			
			FlagValue = Result->GetValue()->AppendValue("TagAlterPreservation", Flags[15]);
			FlagValue->AppendTag("bit index", 15);
			FlagValue->AppendTag("bit name", "a"s);
			if(Flags[15] == true)
			{
				FlagValue->AppendTag("interpretation", "Frame should be discarded."s);
			}
			else
			{
				FlagValue->AppendTag("interpretation", "Frame should be preserved."s);
			}
			FlagValue = Result->GetValue()->AppendValue("FileAlterPreservation", Flags[14]);
			FlagValue->AppendTag("bit index", 14);
			FlagValue->AppendTag("bit name", "b"s);
			if(Flags[14] == true)
			{
				FlagValue->AppendTag("interpretation", "Frame should be discarded."s);
			}
			else
			{
				FlagValue->AppendTag("interpretation", "Frame should be preserved."s);
			}
			FlagValue = Result->GetValue()->AppendValue("ReadOnly", Flags[13]);
			FlagValue->AppendTag("bit index", 13);
			FlagValue->AppendTag("bit name", "c"s);
			FlagValue = Result->GetValue()->AppendValue("Reserved", false);
			for(auto FlagIndex = 8; FlagIndex <= 12; ++FlagIndex)
			{
				FlagValue->AppendTag("bit index", FlagIndex);
				Continue = Continue && ~Flags[FlagIndex];
			}
			FlagValue = Result->GetValue()->AppendValue("Compression", Flags[7]);
			FlagValue->AppendTag("bit index", 7);
			FlagValue->AppendTag("bit name", "i"s);
			if(Flags[7] == true)
			{
				FlagValue->AppendTag("interpretation", "Frame is compressed using ZLIB with 4 bytes for 'decompressed size' appended to the frame header."s);
				FlagValue->AppendTag("error", "Frame compression is not yet implemented!");
			}
			else
			{
				FlagValue->AppendTag("interpretation", "Frame is not compressed."s);
			}
			FlagValue = Result->GetValue()->AppendValue("Encryption", Flags[6]);
			FlagValue->AppendTag("bit index", 6);
			FlagValue->AppendTag("bit name", "j"s);
			if(Flags[6] == true)
			{
				FlagValue->AppendTag("interpretation", "Frame is encrypted."s);
				FlagValue->AppendTag("error", "Frame encryption is not yet implemented!");
			}
			else
			{
				FlagValue->AppendTag("interpretation", "Frame is not encrypted."s);
			}
			FlagValue = Result->GetValue()->AppendValue("GroupingIdentity", Flags[5]);
			FlagValue->AppendTag("bit index", 5);
			FlagValue->AppendTag("bit name", "k"s);
			if(Flags[5] == true)
			{
				FlagValue->AppendTag("interpretation", "Frame contains group information."s);
				FlagValue->AppendTag("error", "Frame grouping is not yet implemented!");
			}
			else
			{
				FlagValue->AppendTag("interpretation", "Frame does not contain group information."s);
			}
			FlagValue = Result->GetValue()->AppendValue("Reserved", false);
			for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
			{
				FlagValue->AppendTag("bit index", FlagIndex);
				Continue = Continue && ~Flags[FlagIndex];
			}
		}
	}
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetAny())};
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
			Continue ^= ~Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		Continue = SizeResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto NumberOfFlagBytesResult{Get_UnsignedInteger_8Bit(Buffer)};
		
		Result->GetValue()->AppendValue("NumberOfFlagBytes", NumberOfFlagBytesResult->GetValue());
		Continue = NumberOfFlagBytesResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto NumberOfFlagBytes{std::experimental::any_cast< std::uint8_t >(Result->GetAny("NumberOfFlagBytes"))};
		
		if(NumberOfFlagBytes != 0x01)
		{
			Result->GetValue()->AppendTag("error", "According to the standard, the number of flag bytes must be equal to 1."s);
			Result->GetValue()->AppendTag("standard", "ID3 2.4"s);
			Continue = false;
		}
	}
	if(Continue == true)
	{
		auto ExtendedHeaderFlagsResult{Get_ID3_2_4_Tag_ExtendedHeader_Flags(Buffer)};
		
		Result->GetValue()->AppendValue("ExtendedFlags", ExtendedHeaderFlagsResult->GetValue());
		Continue = ExtendedHeaderFlagsResult->GetSuccess();
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue("ExtendedFlags")->GetValueAny("TagIsAnUpdate")) == true)
		{
			auto TagIsAnUpdateDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Buffer)};
			
			Result->GetValue()->AppendValue("TagIsAnUpdateData", TagIsAnUpdateDataResult->GetValue());
			Continue = TagIsAnUpdateDataResult->GetSuccess();
		}
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue("ExtendedFlags")->GetValueAny("CRCDataPresent")) == true)
		{
			auto CRCDataPresentDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Buffer)};
			
			Result->GetValue()->AppendValue("CRCDataPresentData", CRCDataPresentDataResult->GetValue());
			Continue = CRCDataPresentDataResult->GetSuccess();
		}
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue("ExtendedFlags")->GetValueAny("TagRestrictions")) == true)
		{
			auto TagRestrictionsDataResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Buffer)};
			
			Result->GetValue()->AppendValue("TagRestrictionsData", TagRestrictionsDataResult->GetValue());
			Continue = TagRestrictionsDataResult->GetSuccess();
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
		
		Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
		Continue = HeaderResult->GetSuccess();
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< std::uint8_t >(Result->GetAny("Size")) == 0x01)
		{
			auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
			auto FieldResult{Get_BitSet_8Bit(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Restrictions", FieldResult->GetValue())};
			
			FieldValue->AppendTag("error", "This program is missing the interpretation of the tag restriction flags."s); 
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The size of the tag restriction flags is not equal to 1."s); 
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{1, 0}}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("synchsafe"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetAny())};
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
			Continue ^= ~Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 8}}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetAny())};
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
			Continue ^= ~Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("standard", "Hydrogenaudio ReplayGain"s);
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Reader, Inspection::Length(0, 3))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_NameCode(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameCode", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Reader, Inspection::Length(0, 3))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_OriginatorCode(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OriginatorCode", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Reader, Inspection::Length(0, 1))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_SignBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SignBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader(Reader, Inspection::Length(0, 9))};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ReplayGainAdjustment", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto SignBit{std::experimental::any_cast< std::uint8_t >(Result->GetAny("SignBit"))};
		auto ReplayGainAdjustment{std::experimental::any_cast< float >(Result->GetValue("ReplayGainAdjustment")->GetTagAny("interpretation"))};
		
		if(SignBit == 0x01)
		{
			ReplayGainAdjustment *= -1.0f;
		}
		Result->GetValue()->PrependTag("interpretation", to_string_cast(ReplayGainAdjustment) + " dB");
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_NameCode(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_3Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto NameCode{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(NameCode == 0x00)
		{
			Result->GetValue()->PrependTag("interpretation", "not set"s);
		}
		else if(NameCode == 0x01)
		{
			Result->GetValue()->PrependTag("interpretation", "track gain adjustment"s);
		}
		else if(NameCode == 0x02)
		{
			Result->GetValue()->PrependTag("interpretation", "album gain adjustment"s);
		}
		else
		{
			Result->GetValue()->PrependTag("error", "This value is unknown and MUST NOT be used."s);
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_OriginatorCode(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_3Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto OriginatorCode{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(OriginatorCode == 0x00)
		{
			Result->GetValue()->PrependTag("interpretation", "unspecified"s);
		}
		else if(OriginatorCode == 0x01)
		{
			Result->GetValue()->PrependTag("interpretation", "pre-set by artist/producer/mastering engineer"s);
		}
		else if(OriginatorCode == 0x02)
		{
			Result->GetValue()->PrependTag("interpretation", "set by user"s);
		}
		else if(OriginatorCode == 0x03)
		{
			Result->GetValue()->PrependTag("interpretation", "determined automatically"s);
		}
		else
		{
			Result->GetValue()->PrependTag("error", "This value is unknown and MUST NOT be used."s);
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_9Bit_BigEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto ReplayGainAdjustment{static_cast< float >(std::experimental::any_cast< std::uint16_t >(Result->GetAny()))};
		
		Result->GetValue()->PrependTag("interpretation", ReplayGainAdjustment / 10.0f);
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_ReplayGainAdjustment_SignBit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_1Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto SignBit{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(SignBit == 0x00)
		{
			Result->GetValue()->PrependTag("interpretation", "positive gain (boost)"s);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "negative gain (attenuation)"s);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
	auto Continue{true};
	
	if(Continue == true)
	{
		auto FileIdentifierResult{Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Buffer, "ID3")};
		
		Result->GetValue()->AppendValue("FileIdentifier", FileIdentifierResult->GetValue());
		Continue = FileIdentifierResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{1, 0}}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MajorVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{1, 0}}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RevisionNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(Result->GetAny("MajorVersion"))};
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
			Continue = FlagsResult->GetSuccess();
		}
		else
		{
			Continue = false;
		}
	}
	if(Continue == true)
	{
		auto SizeResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Buffer)};
		
		Result->GetValue()->AppendValue("Size", SizeResult->GetValue());
		Continue = SizeResult->GetSuccess();
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == true)
		{
			if(Reader.Get1Bits() == 0x00)
			{
				std::uint8_t First{Reader.Get7Bits()};
				
				Result->GetValue()->SetAny(First);
				Result->GetValue()->AppendTag("integer"s);
				Result->GetValue()->AppendTag("unsigned"s);
				Result->GetValue()->AppendTag("7bit value"s);
				Result->GetValue()->AppendTag("8bit field"s);
				Result->GetValue()->AppendTag("synchsafe"s);
			}
			else
			{
				Continue = false;
			}
		}
		else
		{
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 12}}};
		auto FieldResult{Get_Bits_Set_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FrameSync", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_MPEG_1_FrameHeader_AudioVersionID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioVersionID", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_MPEG_1_FrameHeader_LayerDescription(Buffer)};
		
		Result->GetValue()->AppendValue("LayerDescription", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_MPEG_1_FrameHeader_ProtectionBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ProtectionBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		auto FieldResult{Get_MPEG_1_FrameHeader_BitRateIndex(Buffer, LayerDescription)};
		
		Result->GetValue()->AppendValue("BitRateIndex", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_MPEG_1_FrameHeader_SamplingFrequency(Buffer)};
		
		Result->GetValue()->AppendValue("SamplingFrequency", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_MPEG_1_FrameHeader_PaddingBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PaddingBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PrivateBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		auto FieldResult{Get_MPEG_1_FrameHeader_Mode(Buffer, LayerDescription)};
		
		Result->GetValue()->AppendValue("Mode", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		auto Mode{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Mode"))};
		auto FieldResult{Get_MPEG_1_FrameHeader_ModeExtension(Buffer, LayerDescription, Mode)};
		
		Result->GetValue()->AppendValue("ModeExtension", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_MPEG_1_FrameHeader_Copyright(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Copyright", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 1}}};
		auto FieldResult{Get_MPEG_1_FrameHeader_OriginalHome(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Original/Home", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_MPEG_1_FrameHeader_Emphasis(Buffer)};
		
		Result->GetValue()->AppendValue("Emphasis", FieldResult->GetValue());
		Continue = FieldResult->GetSuccess();
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto AudioVersionID{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(AudioVersionID == 0x01)
		{
			Result->GetValue()->PrependTag("MPEG Version 1 (ISO/IEC 11172-3)"s);
		}
		else
		{
			Result->GetValue()->PrependTag("<reserved>");
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Copyright(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Copyright{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(Copyright == 0x00)
		{
			Result->GetValue()->PrependTag("copyright", false);
		}
		else if(Copyright == 0x01)
		{
			Result->GetValue()->PrependTag("copyright", true);
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Emphasis(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Emphasis{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(Emphasis == 0x00)
		{
			Result->GetValue()->PrependTag("no emphasis"s);
		}
		else if(Emphasis == 0x01)
		{
			Result->GetValue()->PrependTag("50/15 microsec. emphasis"s);
		}
		else if(Emphasis == 0x02)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
			Continue = false;
		}
		else if(Emphasis == 0x03)
		{
			Result->GetValue()->PrependTag("CCITT J.17"s);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(LayerDescription == 0x00)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
			Continue = false;
		}
		else if(LayerDescription == 0x01)
		{
			Result->GetValue()->PrependTag("Layer III"s);
		}
		else if(LayerDescription == 0x02)
		{
			Result->GetValue()->PrependTag("Layer II"s);
		}
		else if(LayerDescription == 0x03)
		{
			Result->GetValue()->PrependTag("Layer I"s);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Mode(Inspection::Buffer & Buffer, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Mode{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(Mode == 0x00)
		{
			Result->GetValue()->PrependTag("stereo"s);
		}
		else if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo)"s);
			}
			else if(LayerDescription == 0x01)
			{
				Result->GetValue()->PrependTag("joint stereo (intensity_stereo and/or ms_stereo)"s);
			}
			else
			{
				Continue = false;
			}
		}
		else if(Mode == 0x02)
		{
			Result->GetValue()->PrependTag("dual_channel"s);
		}
		else if(Mode == 0x03)
		{
			Result->GetValue()->PrependTag("single_channel"s);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Buffer & Buffer, std::uint8_t LayerDescription, std::uint8_t Mode)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto ModeExtension{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("subbands 4-31 in intensity_stereo, bound==4"s);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("subbands 8-31 in intensity_stereo, bound==8"s);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("subbands 12-31 in intensity_stereo, bound==12"s);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("subbands 16-31 in intensity_stereo, bound==16"s);
				}
			}
			else if(LayerDescription == 0x01)
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->PrependTag("ms_stereo", "off"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "off"s);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->PrependTag("ms_stereo", "on"s);
					Result->GetValue()->PrependTag("intensity_stereo", "on"s);
				}
			}
		}
		else
		{
			Result->GetValue()->PrependTag("<ignored>"s);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto OriginalHome{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(OriginalHome == 0x00)
		{
			Result->GetValue()->PrependTag("original", false);
		}
		else if(OriginalHome == 0x01)
		{
			Result->GetValue()->PrependTag("original", true);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_PaddingBit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto OriginalHome{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(OriginalHome == 0x00)
		{
			Result->GetValue()->PrependTag("padding", false);
		}
		else if(OriginalHome == 0x01)
		{
			Result->GetValue()->PrependTag("padding", true);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ProtectionBit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Reader, Inspection::Length{0, 1}}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto OriginalHome{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(OriginalHome == 0x00)
		{
			Result->GetValue()->PrependTag("redundancy", false);
		}
		else if(OriginalHome == 0x01)
		{
			Result->GetValue()->PrependTag("redundancy", true);
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	//reading
	if(Continue == true)
	{
		auto FieldReader{Inspection::Reader{Buffer, Inspection::Length{0, 2}}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto SamplingFrequency{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(SamplingFrequency == 0x00)
		{
			Result->GetValue()->PrependTag("numeric", 44100u);
			Result->GetValue()->PrependTag("44.1 kHz"s);
		}
		else if(SamplingFrequency == 0x01)
		{
			Result->GetValue()->PrependTag("numeric", 48000u);
			Result->GetValue()->PrependTag("48 kHz"s);
		}
		else if(SamplingFrequency == 0x02)
		{
			Result->GetValue()->PrependTag("numeric", 32000u);
			Result->GetValue()->PrependTag("32 kHz"s);
		}
		else if(SamplingFrequency == 0x03)
		{
			Result->GetValue()->PrependTag("<reserved>"s);
			Continue = false;
		}
	}
	Result->SetSuccess(Continue);
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

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits)
{
	switch(Bits)
	{
	case 1:
		{
			return Get_SignedInteger_1Bit(Buffer);
		}
	case 12:
		{
			return Get_SignedInteger_12Bit_BigEndian(Buffer);
		}
	default:
		{
			throw NotImplementedException("Reading " + to_string_cast(Bits) + " bits as a signed integer is not yet implemented in the generic function.");
		}
	}
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_1Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 1) == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Buffer.Get1Bits() << 7) >> 7)};
		
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("1bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_5Bit(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 5) == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Buffer.Get5Bits() << 3) >> 3)};
		
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("5bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_12Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 12) == true)
	{
		std::int16_t Value{0};
		
		Value |= static_cast< std::int16_t >(static_cast< std::int16_t >(Buffer.Get4Bits() << 12) >> 4);
		Value |= static_cast< std::int16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("12bit"s);
		Result->SetSuccess(true);
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

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_RiceEncoded(Inspection::Buffer & Buffer, std::uint8_t RiceParameter)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	if(Continue == true)
	{
		auto MostSignificantBitsResult{Get_UnsignedInteger_32Bit_AlternativeUnary(Buffer)};
		
		Result->GetValue()->AppendValue("MostSignificantBits", MostSignificantBitsResult->GetValue());
		Continue = MostSignificantBitsResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto LeastSignificantBitsResult{Get_UnsignedInteger_BigEndian(Buffer, RiceParameter)};
		
		Result->GetValue()->AppendValue("LeastSignificantBits", LeastSignificantBitsResult->GetValue());
		Continue = LeastSignificantBitsResult->GetSuccess();
	}
	if(Continue == true)
	{
		auto MostSignificantBits{std::experimental::any_cast< std::uint32_t >(Result->GetAny("MostSignificantBits"))};
		std::uint32_t LeastSignificantBits;
		
		if(Result->GetAny("LeastSignificantBits").type() == typeid(std::uint8_t))
		{
			LeastSignificantBits = std::experimental::any_cast< std::uint8_t >(Result->GetAny("LeastSignificantBits"));
		}
		else if(Result->GetAny("LeastSignificantBits").type() == typeid(std::uint16_t))
		{
			LeastSignificantBits = std::experimental::any_cast< std::uint16_t >(Result->GetAny("LeastSignificantBits"));
		}
		
		auto Value{MostSignificantBits << RiceParameter | LeastSignificantBits};
		
		if((Value & 0x00000001) == 0x00000001)
		{
			Result->GetValue()->SetAny(static_cast< std::int32_t >(-(Value >> 1)- 1));
		}
		else
		{
			Result->GetValue()->SetAny(static_cast< std::int32_t >(Value >> 1));
		}
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedIntegers_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Buffer, std::bind(Inspection::Get_SignedInteger_BigEndian, std::placeholders::_1, Bits), NumberOfElements);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits)
{
	switch(Bits)
	{
	case 0:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 0))};
			auto FieldResult{Get_UnsignedInteger_0Bit(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 1:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 1))};
			auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 2:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 2))};
			auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 3:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 3))};
			auto FieldResult{Get_UnsignedInteger_3Bit(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 4:
		{
			return Get_UnsignedInteger_4Bit(Buffer);
		}
	case 5:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 5))};
			auto FieldResult{Get_UnsignedInteger_5Bit(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 6:
		{
			return Get_UnsignedInteger_6Bit(Buffer);
		}
	case 7:
		{
			return Get_UnsignedInteger_7Bit(Buffer);
		}
	case 8:
		{
			return Get_UnsignedInteger_8Bit(Buffer);
		}
	case 9:
		{
			auto FieldReader{Inspection::Reader(Buffer, Inspection::Length(0, 9))};
			auto FieldResult{Get_UnsignedInteger_9Bit_BigEndian(FieldReader)};
			
			Buffer.SetPosition(FieldReader);
			
			return FieldResult;
		}
	case 10:
		{
			return Get_UnsignedInteger_10Bit_BigEndian(Buffer);
		}
	case 11:
		{
			return Get_UnsignedInteger_11Bit_BigEndian(Buffer);
		}
	case 12:
		{
			return Get_UnsignedInteger_12Bit_BigEndian(Buffer);
		}
	case 13:
		{
			return Get_UnsignedInteger_13Bit_BigEndian(Buffer);
		}
	case 14:
		{
			return Get_UnsignedInteger_14Bit_BigEndian(Buffer);
		}
	case 15:
		{
			return Get_UnsignedInteger_15Bit_BigEndian(Buffer);
		}
	case 16:
		{
			return Get_UnsignedInteger_16Bit_BigEndian(Buffer);
		}
	case 17:
		{
			return Get_UnsignedInteger_17Bit_BigEndian(Buffer);
		}
	default:
		{
			throw NotImplementedException("Reading " + to_string_cast(Bits) + " bits as an unsigned integer is not yet implemented in the generic function.");
		}
	}
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_0Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	Result->GetValue()->SetAny(Reader.Get0Bits());
	Result->GetValue()->AppendTag("integer"s);
	Result->GetValue()->AppendTag("unsigned"s);
	Result->GetValue()->AppendTag("0bit"s);
	Result->SetSuccess(true);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_1Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 1}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get1Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("1bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_2Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 2}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get2Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("2bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_3Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 3}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get3Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("3bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_5Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 5}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get5Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("5bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Boundary{Buffer.GetPosition() + Length};
	std::uint8_t Value{0ul};
	
	while(true)
	{
		if(Buffer.GetPosition() < Boundary)
		{
			if(Buffer.Has(Inspection::Length(0, 1)) == true)
			{
				auto Bit{Buffer.Get1Bits()};
				
				if(Bit == 0x00)
				{
					Value += 1;
				}
				else
				{
					Result->SetSuccess(true);
					Result->GetValue()->AppendTag(to_string_cast(Value + 1) + "bit"s);
					
					break;
				}
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The unary coded unsigned integer could not be completed before the boundary because the buffer ran out."s);
				
				break;
			}
		}
		else
		{
			Result->SetSuccess(true);
			Result->GetValue()->AppendTag(to_string_cast(Value) + "bit"s);
			Result->GetValue()->AppendTag("ended by boundary"s);
			
			break;
		}
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("alternative unary"s);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 9}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get1Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("9bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_10Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 10) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get2Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("10bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_11Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 11) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get3Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("11bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_12Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 12) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get4Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("12bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_13Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 13) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get5Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("13bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_14Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 14) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get6Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("14bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_15Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 15) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Buffer.Get7Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("15bit"s);
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_17Bit_BigEndian(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(0ull, 17) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Buffer.Get1Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Buffer.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("17bit"s);
		Result->GetValue()->AppendTag("big endian"s);
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Buffer & Buffer)
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
		else if((First & 0xfc) == 0xf8)
		{
			if(Buffer.Has(4ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				auto Fifth{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x03) << 24)| static_cast< std::uint32_t >((Second & 0x3f) << 18) | static_cast< std::uint32_t >((Third & 0x3f) << 12) | static_cast< std::uint32_t >((Fourth & 0x3f) << 6) | static_cast< std::uint32_t >(Fifth & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
		else if((First & 0xfe) == 0xfc)
		{
			if(Buffer.Has(5ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				auto Fifth{Buffer.Get8Bits()};
				auto Sixth{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x01) << 30)| static_cast< std::uint32_t >((Second & 0x3f) << 24) | static_cast< std::uint32_t >((Third & 0x3f) << 18) | static_cast< std::uint32_t >((Fourth & 0x3f) << 12) | static_cast< std::uint32_t >((Fifth & 0x3f) << 6) | static_cast< std::uint32_t >(Sixth & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	std::uint32_t Value{0ul};
	
	while(true)
	{
		if(Buffer.Has(Inspection::Length(0, 1)) == true)
		{
			auto Bit{Buffer.Get1Bits()};
			
			if(Bit == 0x00)
			{
				Value += 1;
			}
			else
			{
				Result->GetValue()->SetAny(Value);
				Result->GetValue()->AppendTag("integer"s);
				Result->GetValue()->AppendTag("unsigned"s);
				Result->GetValue()->AppendTag(to_string_cast(Value + 1) + "bit"s);
				Result->GetValue()->AppendTag("alternative unary"s);
				Result->SetSuccess(true);
				
				break;
			}
		}
		else
		{
			break;
		}
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length(0ull, 36)) == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Reader.Get4Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("36bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Buffer & Buffer)
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
		else if((First & 0xfc) == 0xf8)
		{
			if(Buffer.Has(4ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				auto Fifth{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x03) << 24)| static_cast< std::uint32_t >((Second & 0x3f) << 18) | static_cast< std::uint32_t >((Third & 0x3f) << 12) | static_cast< std::uint32_t >((Fourth & 0x3f) << 6) | static_cast< std::uint32_t >(Fifth & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
		else if((First & 0xfe) == 0xfc)
		{
			if(Buffer.Has(5ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				auto Fifth{Buffer.Get8Bits()};
				auto Sixth{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x01) << 30)| static_cast< std::uint32_t >((Second & 0x3f) << 24) | static_cast< std::uint32_t >((Third & 0x3f) << 18) | static_cast< std::uint32_t >((Fourth & 0x3f) << 12) | static_cast< std::uint32_t >((Fifth & 0x3f) << 6) | static_cast< std::uint32_t >(Sixth & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
		else if((First & 0xff) == 0xfe)
		{
			if(Buffer.Has(6ull, 0) == true)
			{
				auto Second{Buffer.Get8Bits()};
				auto Third{Buffer.Get8Bits()};
				auto Fourth{Buffer.Get8Bits()};
				auto Fifth{Buffer.Get8Bits()};
				auto Sixth{Buffer.Get8Bits()};
				auto Seventh{Buffer.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80) && ((Seventh & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((Second & 0x3f) << 30) | static_cast< std::uint32_t >((Third & 0x3f) << 24) | static_cast< std::uint32_t >((Fourth & 0x3f) << 18) | static_cast< std::uint32_t >((Fifth & 0x3f) << 12) | static_cast< std::uint32_t >((Sixth & 0x3f) << 6) | static_cast< std::uint32_t >(Seventh & 0x3f));
					Result->SetSuccess(true);
				}
			}
		}
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_BigEndian(Inspection::Buffer & Buffer, std::uint8_t Bits, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Buffer, std::bind(Inspection::Get_UnsignedInteger_BigEndian, std::placeholders::_1, Bits), NumberOfElements);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_16Bit_BigEndian(Inspection::Buffer & Buffer, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Buffer, Get_UnsignedInteger_16Bit_BigEndian, NumberOfElements);
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
