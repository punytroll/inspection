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

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset<32> & TagsFlags{std::experimental::any_cast< const std::bitset<32> & >(Result->GetAny())};
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{8, 0}};
		auto FieldResult{Get_ASCII_String_Alphabetic_EndedByTemplateLength(FieldReader, "APETAGEX")};
		auto FieldValue{Result->GetValue()->AppendValue("Preamble", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_APE_Tags_HeaderOrFooter_VersionNumber(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("VersionNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TagSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_APE_Tags_Flags(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TagsFlags", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length{8, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto VersionNumber{std::experimental::any_cast< std::uint32_t >(Result->GetAny())};
		
		if(VersionNumber == 1000)
		{
			Result->GetValue()->AppendTag("interpretation", "1.000 (old)"s);
		}
		else if(VersionNumber == 2000)
		{
			Result->GetValue()->AppendTag("interpretation", "2.000 (new)"s);
		}
		else
		{
			Result->GetValue()->AppendTag("interpretation", "<unknown>"s);
			Result->GetValue()->AppendTag("error", "Unknown version number " + to_string_cast(VersionNumber) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tags_Item(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemValueSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_APE_Tags_Flags(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemFlags", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemKey", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto ItemValueType{std::experimental::any_cast< std::uint8_t >(Result->GetValue("ItemFlags")->GetValueAny("ItemValueType"))};
		
		if(ItemValueType == 0)
		{
			auto ItemValueSize{std::experimental::any_cast< std::uint32_t >(Result->GetAny("ItemValueSize"))};
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, Inspection::Length{ItemValueSize, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("ItemValue", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			throw Inspection::NotImplementedException("Can only interpret UTF-8 item values.");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByNumberOfElements(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter, std::uint64_t NumberOfElements)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto ElementIndex{0ull};
	auto Continue{true};
	
	while(true)
	{
		if(ElementIndex < NumberOfElements)
		{
			Inspection::Reader ElementReader{Reader};
			auto ElementResult{Getter(ElementReader)};
			auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
			
			ElementValue->AppendTag("array index", static_cast< std::uint64_t> (ElementIndex));
			ElementIndex++;
			UpdateState(Continue, Reader, ElementResult, ElementReader);
			if(Continue == false)
			{
				Result->GetValue()->PrependTag("ended by failure"s);
				
				break;
			}
		}
		else
		{
			Result->GetValue()->PrependTag("ended by number of elements"s);
			
			break;
		}
	}
	Result->GetValue()->PrependTag("number of elements", static_cast< std::uint64_t> (NumberOfElements));
	Result->GetValue()->PrependTag("array"s);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_Alphabetic(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Character{Reader.Get8Bits()};
		
		if(Is_ASCII_Character_Alphabetic(Character) == true)
		{
			Result->GetValue()->SetAny(Character);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_AlphaNumeric(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Character{Reader.Get8Bits()};
		
		if((Is_ASCII_Character_Alphabetic(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true))
		{
			Result->GetValue()->SetAny(Character);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Character{Reader.Get8Bits()};
		
		if((Is_ASCII_Character_Alphabetic(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true) || (Is_ASCII_Character_Space(Character) == true))
		{
			Result->GetValue()->SetAny(Character);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("alphabetic"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if(Is_ASCII_Character_Alphabetic(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphabetic ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Alphabetic_EndedByTemplateLength(Inspection::Reader & Reader, const std::string & TemplateString)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("alphabetic"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{TemplateString.size(), 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length must be at least " + to_string_cast(Inspection::Length{TemplateString.size(), 0}) + ", the length of the template string.");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		for(auto TemplateCharacter : TemplateString)
		{
			auto Character{Reader.Get8Bits()};
			
			if((Is_ASCII_Character_Alphabetic(Character) == true) && (TemplateCharacter == Character))
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphabetic ASCII character.");
				Continue = false;
			}
		}
		if(Continue == true)
		{
			Result->GetValue()->AppendTag("ended by template"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters"s);
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("alphanumeric"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if((Is_ASCII_Character_Alphabetic(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true))
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Inspection::Reader & Reader, const std::string & TemplateString)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("alphanumeric"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{TemplateString.size(), 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length must be at least " + to_string_cast(Inspection::Length{TemplateString.size(), 0}) + ", the length of the template string.");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		for(auto TemplateCharacter : TemplateString)
		{
			auto Character{Reader.Get8Bits()};
			
			if(((Is_ASCII_Character_Alphabetic(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true)) && (TemplateCharacter == Character))
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric ASCII character.");
				Continue = false;
			}
		}
		if(Continue == true)
		{
			Result->GetValue()->AppendTag("ended by template"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters"s);
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("alphanumeric or space"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if((Is_ASCII_Character_Alphabetic(Character) == true) || (Is_ASCII_Character_DecimalDigit(Character) == true) || (Is_ASCII_Character_Space(Character) == true))
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric or space ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("printable"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if(Is_ASCII_Character_Printable(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by invalid character '" + to_string_cast(Character) + '\'');
				Reader.MoveBackPosition(Inspection::Length{1, 0});
				
				break;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("printable"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if(Is_ASCII_Character_Printable(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not a printable ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ASCII"s);
	Result->GetValue()->AppendTag("printables only"s);
	//reading
	if(Continue == true)
	{
		auto NumberOfCharacters{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.Has(Inspection::Length{1, 0}) == true))
		{
			auto Character{Reader.Get8Bits()};
			
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
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	//finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Boolean_16Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto UnsignedInteger16Bit{std::experimental::any_cast< std::uint16_t >(Result->GetAny())};
		
		if(UnsignedInteger16Bit == 0x0000)
		{
			Result->GetValue()->PrependTag("value", false);
		}
		else if(UnsignedInteger16Bit == 0x0001)
		{
			Result->GetValue()->PrependTag("value", true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Boolean_32Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto UnsignedInteger32Bit{std::experimental::any_cast< std::uint32_t >(Result->GetAny())};
		
		if(UnsignedInteger32Bit == 0x00000000)
		{
			Result->GetValue()->PrependTag("value", false);
		}
		else if(UnsignedInteger32Bit == 0x00000001)
		{
			Result->GetValue()->PrependTag("value", true);
		}
		else
		{
			Result->GetValue()->PrependTag("value", "<no interpretation>"s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecEntry(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_ASF_CodecEntryType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Type", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecNameLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto CodecNameLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("CodecNameLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecNameLength)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecName", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecDescriptionLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto CodecDescriptionLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("CodecDescriptionLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Buffer, CodecDescriptionLength)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecInformationLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto CodecInformationLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("CodecInformationLength"))};
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{CodecInformationLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("CodecInformation", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecEntryType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Type{std::experimental::any_cast< std::uint16_t >(Result->GetAny())};
		
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_ASF_GUID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetAny("Reserved")) == Inspection::g_ASF_Reserved2GUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecEntriesCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto CodecEntries(std::make_shared< Inspection::Value >());
		
		Result->GetValue()->AppendValue("CodecEntries", CodecEntries);
		
		auto CodecEntriesCount{std::experimental::any_cast< std::uint32_t >(Result->GetAny("CodecEntriesCount"))};
		
		for(auto CodecEntryIndex = 0ul; (Continue == true) && (CodecEntryIndex < CodecEntriesCount); ++CodecEntryIndex)
		{
			auto FieldResult{Get_ASF_CodecEntry(Buffer)};
			auto FieldValue{CodecEntries->AppendValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CompatibilityObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Profile", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("Profile")) == 0x02;
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Mode", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("Mode")) == 0x01;
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TitleLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AuthorLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CopyrightLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DescriptionLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RatingLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TitleLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("TitleLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Inspection::Length{TitleLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Title", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto AuthorLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("AuthorLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Inspection::Length{AuthorLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Author", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto CopyrightLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("CopyrightLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Inspection::Length{CopyrightLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Copyright", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("DescriptionLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Inspection::Length{DescriptionLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto RatingLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("RatingLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Inspection::Length{RatingLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Rating", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_DataObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Start{Buffer.GetPosition()};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_ObjectHeader(Buffer)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetAny("GUID"))};
		
		Continue = GUID == Inspection::g_ASF_DataObjectGUID;
	}
	// reading
	if(Continue == true)
	{
		auto Size{std::experimental::any_cast< std::uint64_t >(Result->GetAny("Size"))};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{Size, 0} - (Buffer.GetPosition() - Start)};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_DataType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto DataType{std::experimental::any_cast< std::uint16_t >(Result->GetAny())};
		
		if(DataType == 0x0000)
		{
			Result->GetValue()->PrependTag("interpretation", "Unicode string"s);
		}
		else if(DataType == 0x0001)
		{
			Result->GetValue()->PrependTag("interpretation", "Byte array"s);
		}
		else if(DataType == 0x0002)
		{
			Result->GetValue()->PrependTag("interpretation", "Boolean"s);
		}
		else if(DataType == 0x0003)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 32bit"s);
		}
		else if(DataType == 0x0004)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 64bit"s);
		}
		else if(DataType == 0x0005)
		{
			Result->GetValue()->PrependTag("interpretation", "Unsigned integer 16bit"s);
		}
		else
		{
			Result->GetValue()->PrependTag("interpretation", "<no interpretation>"s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto NameLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("NameLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length{NameLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Name", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{2, 0}};
		auto FieldResult{Get_ASF_DataType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ValueDataType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ValueLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto ValueLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("ValueLength"))};
		auto ValueDataType{std::experimental::any_cast< const std::string & >(Result->GetValue("ValueDataType")->GetTagAny("interpretation"))};
		auto Name{std::experimental::any_cast< const std::string & >(Result->GetAny("Name"))};
		auto FieldResult{Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Buffer, Inspection::Length{ValueLength, 0}, ValueDataType, Name)};
		auto FieldValue{Result->GetValue()->AppendValue("Value", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType, const std::string & Name)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
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
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Length == Inspection::Length{4, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
				auto FieldResult{Get_ASF_Boolean_32Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Length == Inspection::Length{4, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Length == Inspection::Length{8, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
				// interpretation
				if(Continue == true)
				{
					if(Name == "WM/EncodingTime")
					{
						auto UnsignedInteger64Bit{std::experimental::any_cast< std::uint64_t >(Result->GetAny())};
						auto DateTime{Inspection::Get_DateTime_FromMicrosoftFileTime(UnsignedInteger64Bit)};
						
						Result->GetValue()->AppendValue("DateTime", DateTime);
						Result->GetValue("DateTime")->AppendTag("date and time"s);
						Result->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
					}
				}
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Length == Inspection::Length{2, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ContentDescriptorsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto ContentDescriptorsCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("ContentDescriptorsCount"))};
		
		for(auto ContentDescriptorsIndex = 0; (Continue == true) && (ContentDescriptorsIndex < ContentDescriptorsCount); ++ContentDescriptorsIndex)
		{
			auto FieldResult{Get_ASF_ExtendedContentDescription_ContentDescriptor(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("ContentDescriptor", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_BitSet_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetAny())};
		
		Result->GetValue()->AppendValue("[0] Reliable", Flags[0]);
		Result->GetValue()->AppendValue("[1] Seekable", Flags[1]);
		Result->GetValue()->AppendValue("[2] No Cleanpoints", Flags[2]);
		Result->GetValue()->AppendValue("[3] Resend Live Cleanpoints", Flags[3]);
		Result->GetValue()->AppendValue("[4-31] Reserved", false);
		for(auto Index = 4; Index < 32; ++Index)
		{
			Continue &= ~Flags[Index];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StartTime", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("EndTime", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "bits per second"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BufferSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("InitialBufferFullness", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateDataBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "bits per second"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateBufferSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateInitialBufferFullness", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->PrependTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumObjectSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_ExtendedStreamPropertiesObject_Flags(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamLanguageIndex", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageTimePerFrame", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNameCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PayloadExtensionSystemCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		auto StreamNameCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("StreamNameCount"))};
		auto PayloadExtensionSystemCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("PayloadExtensionSystemCount"))};
		
		if((StreamNameCount != 0) || (PayloadExtensionSystemCount != 0))
		{
			throw Inspection::NotImplementedException("StreamNameCount != 0 || PayloadExtensionSystemCount != 0");
		}
	}
	// reading
	if(Continue == true)
	{
		if(Buffer.GetPosition() < Boundary)
		{
			auto FieldResult{Get_ASF_StreamPropertiesObject(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("StreamPropertiesObject", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_GUID(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_GUID_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto GUIDInterpretation{Get_GUID_Interpretation(std::experimental::any_cast< Inspection::GUID >(Result->GetAny()))};
		
		Result->GetValue()->PrependTag("interpretation", GUIDInterpretation);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderExtensionObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_ASF_GUID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ReservedField1", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetAny("ReservedField1")) == Inspection::g_ASF_Reserved1GUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ReservedField2", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint16_t >(Result->GetAny("ReservedField2")) == 0x0006;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("HeaderExtensionDataSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto HeaderExtensionDataSize{std::experimental::any_cast< std::uint32_t >(Result->GetAny("HeaderExtensionDataSize"))};
		auto Boundary{Buffer.GetPosition() + Inspection::Length{HeaderExtensionDataSize, 0}};
		
		while((Continue == true) && (Buffer.GetPosition() < Boundary))
		{
			auto FieldResult{Get_ASF_Object(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("AdditionalExtendedHeader", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_BitSet_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	//~ if(Continue == true)
	//~ {
		//~ const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetAny())};
		
		//~ Result->GetValue()->AppendValue("[0] Broadcast", Flags[0]);
		//~ Result->GetValue()->AppendValue("[1] Seekable", Flags[1]);
		//~ Result->GetValue()->AppendValue("[2-31] Reserved", false);
		//~ for(auto Index = 2; Index < 32; ++Index)
		//~ {
			//~ Continue &= ~Flags[Index];
		//~ }
	//~ }
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_FilePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_GUID_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FileID", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FileSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CreationDate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpreting
	if(Continue == true)
	{
		auto CreationDate{std::experimental::any_cast< std::uint64_t >(Result->GetAny("CreationDate"))};
		
		Result->GetValue("CreationDate")->AppendValue("DateTime", Inspection::Get_DateTime_FromMicrosoftFileTime(CreationDate));
		Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("date and time"s);
		Result->GetValue("CreationDate")->GetValue("DateTime")->AppendTag("from Microsoft filetime"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataPacketsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PlayDuration", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SendDuration", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Preroll", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_FilePropertiesFlags(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MinimumDataPacketSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumDataPacketSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObject(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_ObjectHeader(Buffer)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			auto FieldResult{Get_ASF_HeaderObjectData(Buffer)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfHeaderObjects", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved1", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("Reserved1")) == 0x01;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved2", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("Reserved2")) == 0x02;
	}
	// reading
	if(Continue == true)
	{
		auto HeaderObjects(std::make_shared< Inspection::Value >());
		
		Result->GetValue()->AppendValue("HeaderObjects", HeaderObjects);
		
		auto NumberOfHeaderObjectsValue{std::experimental::any_cast< std::uint32_t >(Result->GetAny("NumberOfHeaderObjects"))};
		
		for(auto ObjectIndex = 0ul; (Continue == true) && (ObjectIndex < NumberOfHeaderObjectsValue); ++ObjectIndex)
		{
			auto FieldResult{Get_ASF_Object(Buffer)};
			auto FieldValue{HeaderObjects->AppendValue("Object", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("LanguageIDLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LanguageIDLength{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LanguageIDLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length{LanguageIDLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("LanguageID", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_LanguageListObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("LanguageIDRecordsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LanguageIDRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("LanguageIDRecordsCount"))};
		
		for(auto LanguageIDRecordIndex = 0; (Continue == true) && (LanguageIDRecordIndex < LanguageIDRecordsCount); ++LanguageIDRecordIndex)
		{
			auto FieldResult{Get_ASF_LanguageIDRecord(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("LanguageIDRecord", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("LanguageListIndex", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{2, 0}};
		auto FieldResult{Get_ASF_MetadataLibrary_DescriptionRecord_DataType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto NameLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("NameLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length{NameLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Name", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto DataLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("DataLength"))};
		auto DataType{std::experimental::any_cast< const std::string & >(Result->GetValue("DataType")->GetTagAny("interpretation"))};
		auto FieldResult{Get_ASF_MetadataLibrary_DescriptionRecord_Data(Buffer, DataType, Inspection::Length{DataLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Buffer & Buffer, const std::string & DataType, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Length == Inspection::Length{2, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
				auto FieldResult{Get_ASF_Boolean_16Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Length == Inspection::Length{4, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Length == Inspection::Length{8, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Length == Inspection::Length{2, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "GUID")
		{
			if(Length == Inspection::Length{16, 0})
			{
				Inspection::Reader FieldReader{Buffer, Length};
				auto FieldResult{Get_GUID_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
				// interpretation
				if(Continue == true)
				{
					auto GUID{std::experimental::any_cast< const Inspection::GUID & >(Result->GetAny())};
					auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
					
					Result->GetValue()->AppendTag("interpretation", GUIDInterpretation);
				}
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{16, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_DataType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto DataType{std::experimental::any_cast< std::uint16_t >(Result->GetAny())};
		
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
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibraryObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DescriptionRecordsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("DescriptionRecordsCount"))};
		
		for(auto DescriptionRecordsIndex = 0; (Continue == true) && (DescriptionRecordsIndex < DescriptionRecordsCount); ++DescriptionRecordsIndex)
		{
			auto FieldResult{Get_ASF_MetadataLibrary_DescriptionRecord(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("DescriptionRecord", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObject_DescriptionRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length{2, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{2, 0}};
		auto FieldResult{Get_ASF_DataType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto NameLength{std::experimental::any_cast< std::uint16_t >(Result->GetAny("NameLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Inspection::Length{NameLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Name", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto DataLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("DataLength"))};
		auto DataType{std::experimental::any_cast< std::string >(Result->GetValue("DataType")->GetTagAny("interpretation"))};
		auto FieldResult{Get_ASF_MetadataObject_DescriptionRecord_Data(Buffer, Inspection::Length{DataLength, 0}, DataType)};
		auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObject_DescriptionRecord_Data(Inspection::Buffer & Buffer, const Inspection::Length & Length, const std::string & DataType)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Length == Inspection::Length{2, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
				auto FieldResult{Get_ASF_Boolean_16Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Length == Inspection::Length{4, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Length == Inspection::Length{8, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Length == Inspection::Length{2, 0})
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Length) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DescriptionRecordsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("DescriptionRecordsCount"))};
		
		for(auto DescriptionRecordsIndex = 0; (Continue == true) && (DescriptionRecordsIndex < DescriptionRecordsCount); ++DescriptionRecordsIndex)
		{
			auto FieldResult{Get_ASF_MetadataObject_DescriptionRecord(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("DescriptionRecord", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Object(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_ObjectHeader(Buffer)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Length Size{std::experimental::any_cast< std::uint64_t >(Result->GetAny("Size")), 0};
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetAny("GUID"))};
		
		if(GUID == Inspection::g_ASF_CompatibilityObjectGUID)
		{
			Inspection::Reader FieldReader{Buffer, Size - Result->GetLength()};
			auto FieldResult{Get_ASF_CompatibilityObjectData(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
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
				ObjectDataResult = Get_ASF_ExtendedStreamPropertiesObjectData(Buffer, Size - Result->GetLength());
				Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
			}
			else if(GUID == Inspection::g_ASF_MetadataObjectGUID)
			{
				ObjectDataResult = Get_ASF_MetadataObjectData(Buffer);
				Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
			}
			else if(GUID == Inspection::g_ASF_IndexPlaceholderObjectGUID)
			{
				ObjectDataResult = Get_ASF_IndexPlaceholderObjectData(Buffer, Size - Result->GetLength());
				Result->GetValue()->AppendValues(ObjectDataResult->GetValue()->GetValues());
			}
			else if(GUID == Inspection::g_ASF_PaddingObjectGUID)
			{
				ObjectDataResult = Get_Bits_Unset_EndedByLength(Buffer, Size - Result->GetLength());
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
				ObjectDataResult = Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size - Result->GetLength());
				Result->GetValue()->AppendValue("Data", ObjectDataResult->GetValue());
			}
			if(ObjectDataResult->GetSuccess() == true)
			{
				Result->SetSuccess(true);
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ObjectHeader(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_ASF_GUID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("GUID", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{Buffer.GetPosition()};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_BitSet_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	//~ Buffer.SetPosition(Position);
	//~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	//~ if(Continue == true)
	//~ {
		//~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 7}}};
		//~ auto FieldResult{Get_UnsignedInteger_7Bit(FieldReader)};
		//~ auto FieldValue{Result->GetValue()->AppendValue("[0-6] StreamNumber", FieldResult->GetValue())};
		
		//~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
	//~ }
	//~ if(Continue == true)
	//~ {
		//~ auto ReservedResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0ull, 9))};
		
		//~ Result->GetValue()->AppendValue("[7-15] Reserved", ReservedResult->GetValue());
		//~ Continue = ReservedResult->GetSuccess();
	//~ }
	//~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitratePropertiesObjectData(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitrateRecordsCount", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto BitrateRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetAny("BitrateRecordsCount"))};
		
		for(auto BitrateRecordsIndex = 0; (Continue == true) && (BitrateRecordsIndex < BitrateRecordsCount); ++BitrateRecordsIndex)
		{
			auto FieldResult{Get_ASF_StreamBitrateProperties_BitrateRecord(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("BitrateRecord", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Start{Buffer.GetPosition()};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_BitSet_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	//~ Buffer.SetPosition(Start);
	//~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	//~ // reading
	//~ if(Continue == true)
	//~ {
		//~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 7}}};
		//~ auto FieldResult{Get_UnsignedInteger_7Bit(FieldReader)};
		//~ auto FieldValue{Result->GetValue()->AppendValue("[0-6] StreamNumber", FieldResult->GetValue())};
		
		//~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
	//~ }
	//~ // reading
	//~ if(Continue == true)
	//~ {
		//~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}}};
		//~ auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		//~ auto FieldValue{Result->GetValue()->AppendValue("[7-14] Reserved", FieldResult->GetValue())};
		
		//~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
	//~ }
	//~ // interpretation
	//~ if(Continue == true)
	//~ {
		//~ Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("[7-14] Reserved")) == 0x00;
	//~ }
	//~ // reading
	//~ if(Continue == true)
	//~ {
		//~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}}};
		//~ auto FieldResult{Get_Boolean_1Bit(FieldReader)};
		//~ auto FieldValue{Result->GetValue()->AppendValue("[15] EncryptedContentFlag", FieldResult->GetValue())};
		
		//~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
	//~ }
	//~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("AudioMedia"s);
	Result->GetValue()->AppendTag("WAVEFORMATEX"s);
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Microsoft_WaveFormat_FormatTag(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("FormatTag", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SamplesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageNumberOfBytesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockAlignment", FieldResult->GetValue())};
		
		FieldValue->PrependTag("unit", "bytes"s);
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FliedValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecSpecificDataSize", FieldResult->GetValue())};
		
		FieldValue->PrependTag("unit", "bytes"s);
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FormatTag{std::experimental::any_cast< const std::string & >(Result->GetValue("FormatTag")->GetTagAny("constant name"))};
		auto CodecSpecificDataSize{std::experimental::any_cast< std::uint16_t >(Result->GetAny("CodecSpecificDataSize"))};
		
		if(FormatTag == "WAVE_FORMAT_WMAUDIO2")
		{
			auto FieldResult{Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Buffer, Inspection::Length{CodecSpecificDataSize, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("CodecSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{CodecSpecificDataSize, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("CodecSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Length != Inspection::Length{10, 0})
		{
			Result->GetValue()->AppendTag("error", "The length must be " + to_string_cast(Inspection::Length{10, 0}) + ", not " + to_string_cast(Length) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SamplesPerBlock", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("EncodeOptions", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SuperBlockAlign", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_ASF_GUID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
		auto FieldResult{Get_ASF_GUID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ErrorCorrectionType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 64}};
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TimeOffset", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificDataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ErrorCorrectionDataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_StreamProperties_Flags(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TypeSpecificDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("TypeSpecificDataLength"))};
		auto StreamType{std::experimental::any_cast< Inspection::GUID >(Result->GetAny("StreamType"))};
		
		if(StreamType == Inspection::g_ASF_AudioMediaGUID)
		{
			auto FieldResult{Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Buffer, Inspection::Length{TypeSpecificDataLength, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{TypeSpecificDataLength, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// reading
	if(Continue == true)
	{
		auto ErrorCorrectionDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("ErrorCorrectionDataLength"))};
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{ErrorCorrectionDataLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("ErrorCorrectionData", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
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
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			Continue = Reader.Get1Bits() == 0x00;
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

std::unique_ptr< Inspection::Result > Inspection::Get_Bits_SetOrUnset_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Reader.AdvancePosition(Reader.GetRemainingLength());
	Result->GetValue()->AppendTag("any data"s);
	Result->GetValue()->AppendTag(to_string_cast(Reader.GetConsumedLength()) + " bytes and bits"s);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 4}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 4}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::bitset< 4 > Value;
		auto Byte1{Reader.Get4Bits()};
		
		Value[0] = (Byte1 & 0x08) == 0x08;
		Value[1] = (Byte1 & 0x04) == 0x04;
		Value[2] = (Byte1 & 0x02) == 0x02;
		Value[3] = (Byte1 & 0x01) == 0x01;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("bitset"s);
		Result->GetValue()->AppendTag("4bit"s);
		Result->GetValue()->AppendTag("most significant bit first"s);
		Result->GetValue()->AppendTag("data", std::vector< std::uint8_t >{Byte1});
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 8}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 8}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
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
		Result->GetValue()->AppendTag("least significant bit first"s);
		Result->GetValue()->AppendTag("data", std::vector< std::uint8_t >{Byte1});
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_16Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::bitset< 16 > Value;
		auto Byte1{Reader.Get8Bits()};
		
		Value[8] = (Byte1 & 0x01) == 0x01;
		Value[9] = (Byte1 & 0x02) == 0x02;
		Value[10] = (Byte1 & 0x04) == 0x04;
		Value[11] = (Byte1 & 0x08) == 0x08;
		Value[12] = (Byte1 & 0x10) == 0x10;
		Value[13] = (Byte1 & 0x20) == 0x20;
		Value[14] = (Byte1 & 0x40) == 0x40;
		Value[15] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Reader.Get8Bits()};
		
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
		Result->GetValue()->AppendTag("big endian"s);
		Result->GetValue()->AppendTag("least significant bit first per byte"s);
		Result->GetValue()->AppendTag("data", std::vector< std::uint8_t >{Byte1, Byte2});
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_16Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::bitset< 16 > Value;
		auto Byte1{Reader.Get8Bits()};
		
		Value[0] = (Byte1 & 0x01) == 0x01;
		Value[1] = (Byte1 & 0x02) == 0x02;
		Value[2] = (Byte1 & 0x04) == 0x04;
		Value[3] = (Byte1 & 0x08) == 0x08;
		Value[4] = (Byte1 & 0x10) == 0x10;
		Value[5] = (Byte1 & 0x20) == 0x20;
		Value[6] = (Byte1 & 0x40) == 0x40;
		Value[7] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Reader.Get8Bits()};
		
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
		Result->GetValue()->AppendTag("least significant bit first per byte"s);
		Result->GetValue()->AppendTag("data", std::vector< std::uint8_t >{Byte1, Byte2});
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_32Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::bitset< 32 > Value;
		auto Byte1{Reader.Get8Bits()};
		
		Value[0] = (Byte1 & 0x01) == 0x01;
		Value[1] = (Byte1 & 0x02) == 0x02;
		Value[2] = (Byte1 & 0x04) == 0x04;
		Value[3] = (Byte1 & 0x08) == 0x08;
		Value[4] = (Byte1 & 0x10) == 0x10;
		Value[5] = (Byte1 & 0x20) == 0x20;
		Value[6] = (Byte1 & 0x40) == 0x40;
		Value[7] = (Byte1 & 0x80) == 0x80;
		
		auto Byte2{Reader.Get8Bits()};
		
		Value[8] = (Byte2 & 0x01) == 0x01;
		Value[9] = (Byte2 & 0x02) == 0x02;
		Value[10] = (Byte2 & 0x04) == 0x04;
		Value[11] = (Byte2 & 0x08) == 0x08;
		Value[12] = (Byte2 & 0x10) == 0x10;
		Value[13] = (Byte2 & 0x20) == 0x20;
		Value[14] = (Byte2 & 0x40) == 0x40;
		Value[15] = (Byte2 & 0x80) == 0x80;
		
		auto Byte3{Reader.Get8Bits()};
		
		Value[16] = (Byte3 & 0x01) == 0x01;
		Value[17] = (Byte3 & 0x02) == 0x02;
		Value[18] = (Byte3 & 0x04) == 0x04;
		Value[19] = (Byte3 & 0x08) == 0x08;
		Value[20] = (Byte3 & 0x10) == 0x10;
		Value[21] = (Byte3 & 0x20) == 0x20;
		Value[22] = (Byte3 & 0x40) == 0x40;
		Value[23] = (Byte3 & 0x80) == 0x80;
		
		auto Byte4{Reader.Get8Bits()};
		
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
		Result->GetValue()->AppendTag("least significant bit first per byte"s);
		Result->GetValue()->AppendTag("data", std::vector< std::uint8_t >{Byte1, Byte2, Byte3, Byte4});
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Boolean_1Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 1}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Bit{Reader.Get1Bits()};
		
		Result->GetValue()->SetAny((0x01 & Bit) == 0x01);
		Result->GetValue()->AppendTag("boolean"s);
		Result->GetValue()->AppendTag("1bit"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RegisteredApplicationIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	
	auto ApplicationDataStart{Buffer.GetPosition()};
	
	// reading
	if(Continue == true)
	{
		Buffer.SetPosition(RegisteredApplicationIdentifierStart);
		
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{4, 0})};
		
		Result->GetValue("RegisteredApplicationIdentifier")->AppendTag("bytes", FieldResult->GetAny());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Buffer.SetPosition(RegisteredApplicationIdentifierStart);
		
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_Printable_EndedByLength(FieldReader)};
		
		if(FieldResult->GetSuccess() == true)
		{
			Result->GetValue("RegisteredApplicationIdentifier")->AppendTag("string interpretation", FieldResult->GetAny());
		}
	}
	// reading
	if(Continue == true)
	{
		Buffer.SetPosition(ApplicationDataStart);
		
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("ApplicationData", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame(Inspection::Buffer & Buffer, std::uint8_t NumberOfChannelsByStream)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Frame_Header(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// inspect
	if(Continue == true)
	{
		auto NumberOfChannelsByFrame{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValue("ChannelAssignment")->GetTagAny("value"))};
		
		if(NumberOfChannelsByStream != NumberOfChannelsByFrame)
		{
			Result->GetValue()->AppendTag("error", "The number of channels from the stream (" + to_string_cast(NumberOfChannelsByStream) + ") does not match the number of channels from the frame (" + to_string_cast(NumberOfChannelsByFrame) + ").");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint16_t >(Result->GetValue("Header")->GetValue("BlockSize")->GetTagAny("value"))};
		auto BitsPerSample{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValue("SampleSize")->GetTagAny("value"))};
		auto ChannelAssignment{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValueAny("ChannelAssignment"))};
		
		for(auto SubFrameIndex = 0; (Continue == true) && (SubFrameIndex < NumberOfChannelsByStream); ++SubFrameIndex)
		{
			if(((SubFrameIndex == 0) && (ChannelAssignment == 0x09)) || ((SubFrameIndex == 1) && ((ChannelAssignment == 0x08) || (ChannelAssignment == 0x0a))))
			{
				auto FieldResult{Get_FLAC_Subframe(Buffer, BlockSize, BitsPerSample + 1)};
				auto FieldValue{Result->GetValue()->AppendValue("Subframe", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				auto FieldResult{Get_FLAC_Subframe(Buffer, BlockSize, BitsPerSample)};
				auto FieldValue{Result->GetValue()->AppendValue("Subframe", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_UntilByteAlignment(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Padding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_FLAC_Frame_Footer(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Footer", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Footer(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("CRC-16", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 14}};
		auto FieldResult{Get_UnsignedInteger_14Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SyncCode", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint16_t >(Result->GetAny("SyncCode")) == 0x3ffe;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	//reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
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
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint8_t >(Result->GetAny("BlockSize"))};
		
		if(BlockSize == 0x00)
		{
			Result->GetValue("BlockSize")->AppendTag("reserved"s);
			Result->GetValue("BlockSize")->AppendTag("error", "The block size 0 MUST NOT be used."s);
			Continue = false;
		}
		else if(BlockSize == 0x01)
		{
			Result->GetValue("BlockSize")->AppendTag("value", static_cast< std::uint16_t >(192));
			Result->GetValue("BlockSize")->AppendTag("unit", "samples"s);
		}
		else if((BlockSize > 0x01) && (BlockSize <= 0x05))
		{
			Result->GetValue("BlockSize")->AppendTag("value", static_cast< std::uint16_t >(576 * (1 << (BlockSize - 2))));
			Result->GetValue("BlockSize")->AppendTag("unit", "samples"s);
		}
		else if(BlockSize == 0x06)
		{
			Result->GetValue("BlockSize")->AppendTag("interpretation", "get 8bit (blocksize - 1) from end of header"s);
		}
		else if(BlockSize == 0x07)
		{
			Result->GetValue("BlockSize")->AppendTag("interpretation", "get 16bit (blocksize - 1) from end of header"s);
		}
		else if((BlockSize > 0x07) && (BlockSize < 0x10))
		{
			Result->GetValue("BlockSize")->AppendTag("value", static_cast< std::uint16_t >(256 * (1 << (BlockSize - 8))));
			Result->GetValue("BlockSize")->AppendTag("unit", "samples"s);
		}
		else
		{
			assert(false);
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_FLAC_Frame_Header_SampleRate(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SampleRate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ChannelAssignment", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto ChannelAssignment{std::experimental::any_cast< std::uint8_t >(Result->GetAny("ChannelAssignment"))};
		
		if(ChannelAssignment < 0x08)
		{
			Result->GetValue("ChannelAssignment")->AppendTag("value", static_cast< std::uint8_t >(ChannelAssignment + 1));
			Result->GetValue("ChannelAssignment")->AppendTag("interpretation", "independent channels"s);
			switch(ChannelAssignment)
			{
			case 0:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "mono"s);
					
					break;
				}
			case 1:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "left, right"s);
					
					break;
				}
			case 2:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "left, right, center"s);
					
					break;
				}
			case 3:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "front left, front right, back left, back right"s);
					
					break;
				}
			case 4:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "front left, front right, front center, back/surround left, back/surround right"s);
					
					break;
				}
			case 5:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "front left, front right, front center, LFE, back/surround left, back/surround right"s);
					
					break;
				}
			case 6:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "front left, front right, front center, LFE, back center, side left, side right"s);
					
					break;
				}
			case 7:
				{
					Result->GetValue("ChannelAssignment")->AppendTag("assignment", "front left, front right, front center, LFE, back left, back right, side left, side right"s);
					
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
			Result->GetValue("ChannelAssignment")->AppendTag("value", static_cast< std::uint8_t >(2));
			Result->GetValue("ChannelAssignment")->AppendTag("interpretation", "left/side stereo"s);
			Result->GetValue("ChannelAssignment")->AppendTag("assignment", "channel 0 is the left channel, channel 1 is the side (difference) channel"s);
		}
		else if(ChannelAssignment == 0x09)
		{
			Result->GetValue("ChannelAssignment")->AppendTag("value", static_cast< std::uint8_t >(2));
			Result->GetValue("ChannelAssignment")->AppendTag("interpretation", "right/side stereo"s);
			Result->GetValue("ChannelAssignment")->AppendTag("assignment", "channel 0 is the side (difference) channel, channel 1 is the right channel"s);
		}
		else if(ChannelAssignment == 0x0a)
		{
			Result->GetValue("ChannelAssignment")->AppendTag("value", static_cast< std::uint8_t >(2));
			Result->GetValue("ChannelAssignment")->AppendTag("interpretation", "mid/side stereo"s);
			Result->GetValue("ChannelAssignment")->AppendTag("assignment", "channel 0 is the mid (average) channel, channel 1 is the side (difference) channel"s);
		}
		else
		{
			Result->GetValue("ChannelAssignment")->AppendTag("reserved"s);
			Result->GetValue("ChannelAssignment")->AppendTag("error", "The channel assignment " + to_string_cast(ChannelAssignment) + " MUST NOT be used.");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 3}};
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
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto BlockingStrategy{std::experimental::any_cast< std::uint8_t >(Result->GetAny("BlockingStrategy"))};
		
		if(BlockingStrategy == 0x00)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_UnsignedInteger_31Bit_UTF_8_Coded(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("FrameNumber", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(BlockingStrategy == 0x01)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_UnsignedInteger_36Bit_UTF_8_Coded(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("SampleNumber", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "Unknown blocking strategy value " + to_string_cast(BlockingStrategy) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint8_t >(Result->GetAny("BlockSize"))};
		
		if(BlockSize == 0x06)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
			auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("BlockSizeExplicit", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				auto BlockSizeValue{Result->GetValue("BlockSize")};
				
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(std::experimental::any_cast< std::uint8_t >(Result->GetAny("BlockSizeExplicit")) + 1));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
		}
		else if(BlockSize == 0x07)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
			auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("BlockSizeExplicit", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				auto BlockSizeValue{Result->GetValue("BlockSize")};
				
				BlockSizeValue->AppendTag("value", static_cast< std::uint16_t >(std::experimental::any_cast< std::uint16_t >(Result->GetAny("BlockSizeExplicit")) + 1));
				BlockSizeValue->AppendTag("unit", "samples"s);
			}
		}
	}
	// reading
	if(Continue == true)
	{
		auto SampleRate{std::experimental::any_cast< std::uint8_t >(Result->GetAny("SampleRate"))};
		
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
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CRC-8", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Header_SampleRate(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_4Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto SampleRate{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(SampleRate == 0x00)
		{
			Result->GetValue()->AppendTag("interpretation", "get from STREAMINFO metadata block"s);
		}
		else if(SampleRate == 0x01)
		{
			Result->GetValue()->AppendTag("value", 88.2f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x02)
		{
			Result->GetValue()->AppendTag("value", 176.4f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x03)
		{
			Result->GetValue()->AppendTag("value", 192.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x04)
		{
			Result->GetValue()->AppendTag("value", 8.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x05)
		{
			Result->GetValue()->AppendTag("value", 16.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x06)
		{
			Result->GetValue()->AppendTag("value", 22.05f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x07)
		{
			Result->GetValue()->AppendTag("value", 24.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x08)
		{
			Result->GetValue()->AppendTag("value", 32.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x09)
		{
			Result->GetValue()->AppendTag("value", 44.1f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x0A)
		{
			Result->GetValue()->AppendTag("value", 48.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x0B)
		{
			Result->GetValue()->AppendTag("value", 96.0f);
			Result->GetValue()->AppendTag("unit", "kHz"s);
		}
		else if(SampleRate == 0x0C)
		{
			Result->GetValue()->AppendTag("interpretation", "get 8 bit sample rate (in kHz) from end of header"s);
		}
		else if(SampleRate == 0x0D)
		{
			Result->GetValue()->AppendTag("interpretation", "get 16 bit sample rate (in Hz) from end of header"s);
		}
		else if(SampleRate == 0x0E)
		{
			Result->GetValue()->AppendTag("interpretation", "get 16 bit smaple rate (in tens of Hz) from end of header"s);
		}
		else if(SampleRate == 0x0F)
		{
			Result->GetValue()->AppendTag("interpretation", "invalid, to prevent sync-fooling string of 1s"s);
			Result->GetValue()->AppendTag("error", "The sample rate MUST NOT be a value of 16."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_FLAC_MetaDataBlock_Header(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(Result->GetValue("Header")->GetValue("BlockType")->GetTagAny("interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto FieldResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MetaDataBlockType == "Padding")
		{
			auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetValue("Header")->GetValueAny("Length"))};
			auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length{MetaDataBlockDataLength, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MetaDataBlockType == "Application")
		{
			auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetValue("Header")->GetValueAny("Length"))};
			auto FieldResult{Get_FLAC_ApplicationBlock_Data(Buffer, Inspection::Length{MetaDataBlockDataLength, 0})};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetValue("Header")->GetValueAny("Length"))};
			
			if(MetaDataBlockDataLength % 18 == 0)
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{MetaDataBlockDataLength, 0}};
				auto FieldResult{Get_FLAC_SeekTableBlock_Data(FieldReader, MetaDataBlockDataLength / 18)};
				auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
			else
			{
				Continue = false;
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			auto FieldResult{Get_FLAC_VorbisCommentBlock_Data(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MetaDataBlockType == "Picture")
		{
			auto FieldResult{Get_FLAC_PictureBlock_Data(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Boolean_1Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("LastMetaDataBlock", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_MetaDataBlock_Type(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_24Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Length", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock_Type(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_7Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_PictureBlock_PictureType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint32_t >(Result->GetAny())};
		
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
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_PictureBlock_Data(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_FLAC_PictureBlock_PictureType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MIMETypeLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto MIMETypeLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("MIMETypeLength"))};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{MIMETypeLength, 0}};
		auto FieldResult{Get_ASCII_String_Printable_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MIMEType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DescriptionLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("DescriptionLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, Inspection::Length{DescriptionLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureWidthInPixels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureHeightInPixels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitsPerPixel", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfColors", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureDataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto PictureDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("PictureDataLength"))};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{PictureDataLength, 0}};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureData", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_SeekTableBlock_Data(Inspection::Reader & Reader, std::uint32_t NumberOfSeekPoints)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		for(auto SeekPointIndex = 0ul; ((Continue == true) && (SeekPointIndex < NumberOfSeekPoints)); ++SeekPointIndex)
		{
			auto FieldResult{Get_FLAC_SeekTableBlock_SeekPoint(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("SeekPoint", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_SeekTableBlock_SeekPoint(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("SampleNumberOfFirstSampleInTargetFrame", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ByteOffsetOfTargetFrame", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfSamplesInTargetFrame", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Stream(Inspection::Buffer & Buffer, bool OnlyStreamHeader)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_Alphabetic_EndedByTemplateLength(FieldReader, "fLaC")};
		auto FieldValue{Result->GetValue()->AppendValue("FLAC stream marker", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_StreamInfoBlock(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamInfoBlock", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto LastMetaDataBlock{std::experimental::any_cast< bool >(Result->GetValue("StreamInfoBlock")->GetValue("Header")->GetValueAny("LastMetaDataBlock"))};
		
		while((Continue == true) && (LastMetaDataBlock == false))
		{
			auto FieldResult{Get_FLAC_MetaDataBlock(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("MetaDataBlock", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				LastMetaDataBlock = std::experimental::any_cast< bool >(FieldResult->GetValue("Header")->GetValueAny("LastMetaDataBlock"));
			}
		}
	}
	// reading
	if((Continue == true) && (OnlyStreamHeader == false))
	{
		auto NumberOfChannels{std::experimental::any_cast< std::uint8_t >(Result->GetValue("StreamInfoBlock")->GetValue("Data")->GetValueAny("NumberOfChannels")) + 1};
		auto FieldResult{Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Buffer, std::bind(Get_FLAC_Frame, std::placeholders::_1, NumberOfChannels), Buffer.GetLength() - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Frames", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		for(auto FrameValue : FieldValue->GetValues())
		{
			FrameValue->SetName("Frame");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_FLAC_MetaDataBlock_Header(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(Result->GetValue("Header")->GetValue("BlockType")->GetTagAny("interpretation"))};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			auto FieldResult{Get_FLAC_StreamInfoBlock_Data(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The BlockType of the meta data block is not \"StreamInfo\"."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MinimumBlockSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumBlockSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 24}};
		auto FieldResult{Get_UnsignedInteger_24Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MinimumFrameSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 24}};
		auto FieldResult{Get_UnsignedInteger_24Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumFrameSize", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 20}};
		auto FieldResult{Get_UnsignedInteger_20Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SampleRate", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 3}};
		auto FieldResult{Get_FLAC_StreamInfoBlock_NumberOfChannels(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 5}};
		auto FieldResult{Get_FLAC_StreamInfoBlock_BitsPerSample(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 36}};
		auto FieldResult{Get_UnsignedInteger_36Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TotalSamplesPerChannel", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{16, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("MD5SignatureOfUnencodedAudioData", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
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
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Header(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	if(Continue == true)
	{
		auto SubframeType{std::experimental::any_cast< const std::string & >(Result->GetValue("Header")->GetValue("Type")->GetTagAny("interpretation"))};
		
		if(SubframeType == "SUBFRAME_CONSTANT")
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_FLAC_Subframe_Data_Constant(FieldReader, BitsPerSample)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(SubframeType == "SUBFRAME_FIXED")
		{
			auto Order{static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValue("Type")->GetValueAny("Order")))};
			auto FieldResult{Get_FLAC_Subframe_Data_Fixed(Buffer, FrameBlockSize, BitsPerSample, Order)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(SubframeType == "SUBFRAME_LPC")
		{
			auto Order{static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValue("Type")->GetValueAny("Order")) + 1)};
			auto FieldResult{Get_FLAC_Subframe_Data_LPC(Buffer, FrameBlockSize, BitsPerSample, Order)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Constant(Inspection::Reader & Reader, std::uint8_t BitsPerSample)
{
	return Get_UnsignedInteger_BigEndian(Reader, BitsPerSample);
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Fixed(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_UnsignedIntegers_BigEndian(FieldReader, BitsPerSample, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("WarmUpSamples", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Residual(Buffer, FrameBlockSize, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("Residual", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_LPC(Inspection::Buffer & Buffer, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_UnsignedIntegers_BigEndian(FieldReader, BitsPerSample, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("WarmUpSamples", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientsPrecision", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto QuantizedLinearPredictorCoefficientsPrecision{std::experimental::any_cast< std::uint8_t >(Result->GetAny("QuantizedLinearPredictorCoefficientsPrecision"))};
		
		if(QuantizedLinearPredictorCoefficientsPrecision < 15)
		{
			Result->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->AppendTag("value", static_cast< std::uint8_t >(QuantizedLinearPredictorCoefficientsPrecision + 1));
		}
		else
		{
			Result->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->AppendTag("error", "The percision MUST NOT be 15."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 5}};
		auto FieldResult{Get_SignedInteger_5Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientShift", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_SignedIntegers_BigEndian(FieldReader, std::experimental::any_cast< std::uint8_t >(Result->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->GetTagAny("value")), PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("PredictorCoefficients", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Residual(Buffer, FrameBlockSize, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("Residual", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length(0, 1))};
		auto FieldValue{Result->GetValue()->AppendValue("PaddingBit", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Type(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Type", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_Boolean_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("WastedBitsPerSampleFlag", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto WastedBitsPerSampleFlag{std::experimental::any_cast< bool >(Result->GetAny("WastedBitsPerSampleFlag"))};
		
		if(WastedBitsPerSampleFlag == true)
		{
			throw Inspection::NotImplementedException("Wasted bits are not implemented yet!");
		}
	}
	// finalization
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
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 2}};
		auto FieldResult{Get_FLAC_Subframe_Residual_CodingMethod(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodingMethod", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto CodingMethod{std::experimental::any_cast< std::uint8_t >(Result->GetAny("CodingMethod"))};
		
		if(CodingMethod == 0x00)
		{
			auto FieldResult{Get_FLAC_Subframe_Residual_Rice(Buffer, FrameBlockSize, PredictorOrder)};
			auto FieldValue{Result->GetValue()->AppendValue("CodedResidual", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				FieldValue->AppendTag("Rice"s);
			}
		}
		else if(CodingMethod == 0x01)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_FLAC_Subframe_Residual_Rice2(FieldReader, FrameBlockSize, PredictorOrder)};
			auto FieldValue{Result->GetValue()->AppendValue("CodedResidual", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				FieldValue->AppendTag("Rice2"s);
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
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
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
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
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PartitionOrder", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto NumberOfPartitions{static_cast< std::uint16_t >(1 << std::experimental::any_cast< std::uint8_t >(Result->GetAny("PartitionOrder")))};
		
		Result->GetValue("PartitionOrder")->AppendTag("number of partitions", NumberOfPartitions);
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfPartitions{std::experimental::any_cast< std::uint16_t >(Result->GetValue("PartitionOrder")->GetTagAny("number of partitions"))};
		auto FieldResult{Get_Array_EndedByNumberOfElements_PassArrayIndex(Buffer, std::bind(Get_FLAC_Subframe_Residual_Rice_Partition, std::placeholders::_1, std::placeholders::_2, FrameBlockSize / NumberOfPartitions, PredictorOrder), NumberOfPartitions)};
		auto FieldValue{Result->GetValue()->AppendValue("Partitions", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		if(Continue == true)
		{
			for(auto PartitionValue : FieldValue->GetValues())
			{
				PartitionValue->SetName("Partition");
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Buffer & Buffer, std::uint64_t ArrayIndex, std::uint32_t NumberOfSamples, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RiceParameter", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto RiceParameter{std::experimental::any_cast< std::uint8_t >(Result->GetAny("RiceParameter"))};
		std::unique_ptr< Inspection::Result > SamplesResult;
		
		if(ArrayIndex == 0)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_Array_EndedByNumberOfElements(FieldReader, std::bind(Get_SignedInteger_32Bit_RiceEncoded, std::placeholders::_1, RiceParameter), NumberOfSamples - PredictorOrder)};
			
			//~ Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_Array_EndedByNumberOfElements(FieldReader, std::bind(Get_SignedInteger_32Bit_RiceEncoded, std::placeholders::_1, RiceParameter), NumberOfSamples)};
			
			//~ Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice2(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	throw Inspection::NotImplementedException("Get_FLAC_Subframe_Residual_Rice2");
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Type(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 6}};
		auto FieldResult{Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto SubframeType{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		switch(SubframeType)
		{
		case 0:
			{
				Result->GetValue()->AppendTag("interpretation", "SUBFRAME_LPC"s);
				
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 5}};
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
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 3}};
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_VorbisCommentBlock_Data(Inspection::Buffer & Buffer)
{
	return Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer);
}

std::unique_ptr< Inspection::Result > Inspection::Get_GUID_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength() != Inspection::Length{16, 0})
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be exactly " + to_string_cast(Inspection::Length{16, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("guid"s);
		Result->GetValue()->AppendTag("binary"s);
		Result->GetValue()->AppendTag("little endian"s);
		
		GUID Value;
		
		Value.Data1 = static_cast< std::uint32_t >(Reader.Get8Bits()) + (static_cast< std::uint32_t >(Reader.Get8Bits()) << 8) + (static_cast< std::uint32_t >(Reader.Get8Bits()) << 16) + (static_cast< std::uint32_t >(Reader.Get8Bits()) << 24);
		Value.Data2 = static_cast< std::uint32_t >(Reader.Get8Bits()) + (static_cast< std::uint32_t >(Reader.Get8Bits()) << 8);
		Value.Data3 = static_cast< std::uint32_t >(Reader.Get8Bits()) + (static_cast< std::uint32_t >(Reader.Get8Bits()) << 8);
		Value.Data4[0] = Reader.Get8Bits();
		Value.Data4[1] = Reader.Get8Bits();
		Value.Data4[2] = Reader.Get8Bits();
		Value.Data4[3] = Reader.Get8Bits();
		Value.Data4[4] = Reader.Get8Bits();
		Value.Data4[5] = Reader.Get8Bits();
		Value.Data4[6] = Reader.Get8Bits();
		Value.Data4[7] = Reader.Get8Bits();
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_1_Tag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ASCII_String_Alphabetic_EndedByTemplateLength(FieldReader, "TAG")};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length{30, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Title", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length{30, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Artist", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length{30, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Album", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length{4, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Year", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto AlternativeStart{Buffer.GetPosition()};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Buffer, Inspection::Length{30, 0})};
		
		if(FieldResult->GetSuccess() == true)
		{
			auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Buffer.SetPosition(AlternativeStart);
			
			Inspection::Reader FieldReader{Buffer, Inspection::Length{29, 0}};
			FieldResult = Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(FieldReader);
			
			auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// reading
			if(Continue == true)
			{
				Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
				auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
				auto FieldValue{Result->GetValue()->AppendValue("AlbumTrack", FieldResult->GetValue())};
				
				UpdateState(Continue, Buffer, FieldResult, FieldReader);
			}
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Genre", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto GenreNumber{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Genre"))};
		
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
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_2_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_2_Language(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Language", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ShortContentDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_2_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("ImageFormat", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_2_Frame_Body_PIC_PictureType(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureData", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC_ImageFormat(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::string & ImageFormat{std::experimental::any_cast< const std::string & >(Result->GetAny())};
		
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_PIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	//reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto PictureType{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		std::string Interpretation;
		
		Result->GetValue()->PrependTag("standard", "ID3 2.2"s);
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Continue = false;
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_T__(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_2_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Information", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_UFI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OwnerIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_2_Frame_Header_Identifier(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 24}};
		auto FieldResult{Get_UnsignedInteger_24Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumeric_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetAny())};
		
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
				Continue = false;
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frames(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto FrameIndex{0ul};
	
	Result->SetSuccess(true);
	while(Buffer.GetPosition() < Boundary)
	{
		auto Start{Buffer.GetPosition()};
		auto FrameResult{Get_ID3_2_2_Frame(Buffer)};
		
		if(FrameResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendValue("Frame[" + to_string_cast(FrameIndex++) + "]", FrameResult->GetValue());
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Buffer.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto AlternativeStart{Buffer.GetPosition()};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ISO_639_2_1998_Code(FieldReader)};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		if(Continue == true)
		{
			Result->SetValue(FieldResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(AlternativeStart);
			FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length{3, 0});
			UpdateState(Continue, FieldResult);
			Result->SetValue(FieldResult->GetValue());
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Tag_Header_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextEncoding(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AppendTag("standard", "ID3 2.2"s);
		
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The text encoding " + to_string_cast(TextEncoding) + " is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer, Length};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Frame_Header(Buffer)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(Result->GetAny("Size")), 0)};
		
		if(Identifier == "RGAD")
		{
			Inspection::Reader FieldReader{Buffer, Size};
			auto FieldResult{Get_ID3_2_3_Frame_Body_RGAD(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
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
				auto FieldResult{BodyHandler(Buffer, Size)};
				
				Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AppendTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
				
				auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size)};
				auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
		}
		
		auto HandledSize{Buffer.GetPosition() - Start};
		
		if(HandledSize > Size)
		{
			Result->GetValue()->PrependTag("claimed size", to_string_cast(Size));
			Result->GetValue()->PrependTag("handled size", to_string_cast(HandledSize));
			Result->GetValue()->PrependTag("error", "The frame size is claimed larger than the actually handled size."s);
		}
		else if(HandledSize < Size)
		{
			Result->GetValue()->PrependTag("claimed size", to_string_cast(Size));
			Result->GetValue()->PrependTag("handled size", to_string_cast(HandledSize));
			Result->GetValue()->PrependTag("error", "The frame size is claimed smaller than the actually handled size."s);
		}
		Buffer.SetPosition(Start + Size);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ID3_2_3_Frame_Body_APIC_MIMEType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MIMEType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_Frame_Body_APIC_PictureType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureData", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC_MIMEType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
		/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
		/// @todo As per [ID3 2.3.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_APIC_PictureType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->PrependTag("standard", "ID3 2.3"s);
		
		auto PictureType{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		std::string Interpretation;
		
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
			Continue = false;
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Language(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Language", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ShortContentDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_GEOB(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ID3_2_3_Frame_Body_GEOB_MIMEType(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MIMEType", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("FileName", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ContentDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("EncapsulatedObject", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_GEOB_MIMEType(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
		/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
		if(Continue == false)
		{
			Result->GetValue()->PrependTag("error", "This field could not be interpreted as a terminated ASCII string of printable characters."s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(Buffer.GetPosition() + Inspection::Length{4, 0} > Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
			Continue = false;
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} == Boundary)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} < Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			Continue = false;
		}
	}
	// reading
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("EMailToUser", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Rating", FieldResult->GetValue())};
		
		FieldValue->PrependTag("standard", "ID3 2.3"s);
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Rating{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Rating"))};
		
		if(Rating > 0)
		{
			Result->GetValue("Rating")->PrependTag("interpretation", to_string_cast(Rating) + " / 255");
		}
		else
		{
			Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
		}
	}
	// reading
	if(Continue == true)
	{
		if(Buffer.GetPosition() == Boundary)
		{
			auto FieldValue{std::make_shared< Inspection::Value >()};
			
			FieldValue->SetName("Counter");
			FieldValue->AppendTag("omitted"s);
			Result->GetValue()->AppendValue(FieldValue);
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} > Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			Result->GetValue("Counter")->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			Result->GetValue("Counter")->PrependTag("standard", "ID3 2.3"s);
			Continue = false;
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} == Boundary)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} < Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			Result->GetValue("Counter")->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_PRIV(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OwnerIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		const std::string & OwnerIdentifier{std::experimental::any_cast< const std::string & >(Result->GetAny("OwnerIdentifier"))};
		
		if(OwnerIdentifier == "AverageLevel")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
			auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("AverageLevel", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "PeakValue")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
			auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("PeakValue", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "WM/MediaClassPrimaryID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("MediaClassPrimaryID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "WM/MediaClassSecondaryID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("MediaClassSecondaryID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "WM/WMCollectionGroupID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("CollectionGroupID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "WM/WMCollectionID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("CollectionID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "WM/WMContentID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ContentID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "ZuneAlbumArtistMediaID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneAlbumArtistMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "ZuneAlbumMediaID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneAlbumMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "ZuneCollectionID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneCollectionID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(OwnerIdentifier == "ZuneMediaID")
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{16, 0}};
			auto FieldResult{Get_ID3_GUID(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
			std::string PRIVDataName;
			std::function< std::unique_ptr< Inspection::Result > (Inspection::Buffer &, const Inspection::Length &) > PRIVDataHandler;
			
			if(OwnerIdentifier == "WM/Provider")
			{
				PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
				PRIVDataName = "Provider";
			}
			else if(OwnerIdentifier == "WM/UniqueFileIdentifier")
			{
				PRIVDataHandler = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
				PRIVDataName = "UniqueFileIdentifier";
			}
			else
			{
				PRIVDataHandler = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
				PRIVDataName = "PrivateData";
			}
			
			auto FieldResult{PRIVDataHandler(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue(PRIVDataName, FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_RGAD(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 32}};
		auto FieldResult{Get_ISO_IEC_IEEE_60559_2011_binary32(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PeakAmplitude", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 16}};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TrackReplayGainAdjustment", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 16}};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlbumReplayGainAdjustment", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Information", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCMP(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue("Information")->GetTagAny("string"))};
		
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCON(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Frame_Body_T___(Buffer, Length)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue("Information")->GetTagAny("string"))};
		auto Interpretation{GetContentTypeInterpretation2_3(Information)};
		
		if(std::get<0>(Interpretation) == true)
		{
			Result->GetValue("Information")->PrependTag("interpretation", std::get<1>(Interpretation));
		}
	}
	// finalization
	Result->SetSuccess(true);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Value", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OwnerIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Language(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Language", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ContentDescriptor", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Lyrics/Text", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Length};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("URL", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_3_TextEncoding(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Descriptor", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("URL", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Frame_Header_Identifier(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_3_Frame_Header_Flags(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header_Flags(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_BitSet_16Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 16 > & Flags{std::experimental::any_cast< const std::bitset< 16 > & >(Result->GetAny())};
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumeric_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetAny())};
		
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
	// finalization
	Result->SetSuccess(Continue);
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Buffer.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto AlternativeStart{Buffer.GetPosition()};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ISO_639_2_1998_Code(FieldReader)};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		if(Continue == true)
		{
			Result->SetValue(FieldResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(AlternativeStart);
			FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length{3, 0});
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				Result->SetValue(FieldResult->GetValue());
				Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			}
			else
			{
				Result->GetValue()->PrependTag("error", "Could not read a language for ID3v2.3."s);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Tag_Header_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextEncoding(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->PrependTag("standard", "ID3 2.3"s);
		
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UCS-2"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
		}
		else
		{
			Result->GetValue()->PrependTag("error", "The text encoding " + to_string_cast(TextEncoding) + " is not known.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Buffer)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->ClearTags();
				Result->GetValue()->PrependTag("string", FieldResult->GetAny("String"));
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer, Length};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->ClearTags();
				Result->GetValue()->PrependTag("string", FieldResult->GetAny("String"));
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{10, 0}};
		auto FieldResult{Get_ID3_2_4_Frame_Header(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->PrependTag("content", Result->GetValue("Identifier")->GetTagAny("interpretation"));
		
		auto Start{Buffer.GetPosition()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetAny("Identifier"))};
		auto Size{Inspection::Length(std::experimental::any_cast< std::uint32_t >(Result->GetAny("Size")), 0)};
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
			auto FieldResult{BodyHandler(Buffer, Size)};
			
			if(Start + Size > Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the frame size is stated larger than the actual handled size."s);
			}
			else if(Start + Size < Buffer.GetPosition())
			{
				Result->GetValue()->PrependTag("error", "For the frame \"" + Identifier + "\", the acutal handled size is larger than the stated frame size."s);
			}
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Size)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		Buffer.SetPosition(Start + Size);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Frame_Body_APIC_MIMEType(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("MIMEType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Frame_Body_APIC_PictureType(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PictureData", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC_MIMEType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		/// @todo There are certain opportunities for at least validating the data! [RFC 2045]
		/// @todo As per [ID3 2.4.0], the value '-->' is also permitted to signal a URL [RFC 1738] in the picture data.
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_APIC_PictureType(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
		
		auto PictureType{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		std::string Interpretation;
		
		try
		{
			Interpretation = Get_ID3_2_PictureType_Interpretation(PictureType);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Interpretation = "unknown";
		}
		Result->GetValue()->PrependTag("interpretation", Interpretation);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_COMM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Language(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Language", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ShortContentDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_MCDI(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_IEC_60908_1999_TableOfContents(Buffer)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_POPM(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("EMailToUser", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Rating", FieldResult->GetValue())};
		
		FieldValue->PrependTag("standard", "ID3 2.3"s);
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Rating{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Rating"))};
		
		if(Rating > 0)
		{
			Result->GetValue("Rating")->PrependTag("interpretation", to_string_cast(Rating) + " / 255");
		}
		else
		{
			Result->GetValue("Rating")->PrependTag("interpretation", "unknown"s);
		}
	}
	// reading
	if(Continue == true)
	{
		if(Buffer.GetPosition() == Boundary)
		{
			auto CounterValue{std::make_shared< Inspection::Value >()};
			
			CounterValue->SetName("Counter");
			CounterValue->AppendTag("omitted"s);
			Result->GetValue()->AppendValue(CounterValue);
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} > Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			FieldValue->PrependTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			FieldValue->PrependTag("standard", "ID3 2.4"s);
			Continue = false;
		}
		else if(Buffer.GetPosition() + Inspection::Length{4, 0} == Boundary)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(Buffer.GetPosition() + Inspection::Length(4ul, 0) < Boundary)
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			FieldValue->PrependTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_T___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto InformationIndex{0ul};
		
		while((Continue == true) && (Buffer.GetPosition() < Boundary))
		{
			auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				Result->GetValue()->AppendValue("Information[" + to_string_cast(InformationIndex) + "]", FieldResult->GetValue());
			}
			InformationIndex += 1;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_TXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Description", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Value", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_UFID(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OwnerIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_USLT(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Language(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("Language", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("ContentDescriptor", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Buffer, TextEncoding, Boundary - Buffer.GetPosition())};
		auto FieldValue{Result->GetValue()->AppendValue("Lyrics/Text", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_W___(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Length};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("URL", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_WXXX(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_TextEncoding(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("TextEncoding", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny("TextEncoding"))};
		auto FieldResult{Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Buffer, TextEncoding)};
		auto FieldValue{Result->GetValue()->AppendValue("Descriptor", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Boundary - Buffer.GetPosition()};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("URL", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{10, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{10, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Frame_Header_Identifier(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Identifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_16Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Header_Identifier(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{4, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{4, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumeric_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetAny())};
		
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Buffer.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reader
	if(Continue == true)
	{
		auto AlternativeStart{Buffer.GetPosition()};
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ISO_639_2_1998_Code(FieldReader)};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		if(Continue == true)
		{
			Result->SetValue(FieldResult->GetValue());
		}
		else
		{
			Buffer.SetPosition(AlternativeStart);
			
			Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
			
			FieldResult = Get_ASCII_String_Alphabetic_EndedByLength(FieldReader);
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				Result->SetValue(FieldResult->GetValue());
				
				const std::string & Code{std::experimental::any_cast< const std::string & >(Result->GetAny())};
				
				if(Code == "XXX")
				{
					Result->GetValue()->PrependTag("standard", "ID3 2.4"s);
					Result->GetValue()->PrependTag("interpretation", "<unknown>"s);
				}
				else
				{
					Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
					Result->GetValue()->PrependTag("error", "The language code \"" + Code + "\" is unknown."s);
					Continue = false;
				}
			}
			else
			{
				Buffer.SetPosition(AlternativeStart);
				FieldResult = Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Buffer, Inspection::Length{3, 0});
				Result->SetValue(FieldResult->GetValue());
				UpdateState(Continue, FieldResult);
				// interpretation
				if(Continue == true)
				{
					Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
					Result->GetValue()->PrependTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
				}
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfFlagBytes", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
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
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader_Flags(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ExtendedFlags", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
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
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto HeaderResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Header(Buffer)};
		
		Result->GetValue()->AppendValues(HeaderResult->GetValue()->GetValues());
		Continue = HeaderResult->GetSuccess();
	}
	// interpretation
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("Size")) == 0x05;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 40}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("TotalFrameCRC", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
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
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
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
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_Header_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_BitSet_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextEncoding(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
		if(TextEncoding == 0x00)
		{
			Result->GetValue()->PrependTag("name", "Latin alphabet No. 1"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 8859-1:1998"s);
		}
		else if(TextEncoding == 0x01)
		{
			Result->GetValue()->PrependTag("name", "UTF-16"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
		}
		else if(TextEncoding == 0x02)
		{
			Result->GetValue()->PrependTag("name", "UTF-16BE"s);
			Result->GetValue()->PrependTag("standard", "RFC 2781"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
		}
		else if(TextEncoding == 0x03)
		{
			Result->GetValue()->PrependTag("name", "UTF-8"s);
			Result->GetValue()->PrependTag("standard", "RFC 2279"s);
			Result->GetValue()->PrependTag("standard", "ISO/IEC 10646-1:1993"s);
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTermination(Inspection::Buffer & Buffer, std::uint8_t TextEncoding)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Buffer)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->ClearTags();
				Result->GetValue()->PrependTag("string", FieldResult->GetAny("String"));
			}
		}
		else if(TextEncoding == 0x02)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Buffer)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x03)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Buffer)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccodingToEncoding_EndedByTerminationOrLength(Inspection::Buffer & Buffer, std::uint8_t TextEncoding, const Inspection::Length & Length)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(TextEncoding == 0x00)
		{
			Inspection::Reader FieldReader{Buffer, Length};
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->ClearTags();
				Result->GetValue()->PrependTag("string", FieldResult->GetAny("String"));
			}
		}
		else if(TextEncoding == 0x02)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else if(TextEncoding == 0x03)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Buffer, Length)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->PrependTag("string", FieldResult->GetAny());
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 3}};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_NameCode(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameCode", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 3}};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_OriginatorCode(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("OriginatorCode", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_ID3_2_ReplayGainAdjustment_SignBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SignBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 9}};
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
	// finalization
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
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{3, 0}};
		auto FieldResult{Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(FieldReader, "ID3")};
		auto FieldValue{Result->GetValue()->AppendValue("FileIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MajorVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_7Bit_SynchSafe_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RevisionNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(Result->GetAny("MajorVersion"))};
		
		if(MajorVersion == 0x02)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
			auto FieldResult{Get_ID3_2_2_Tag_Header_Flags(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(MajorVersion == 0x03)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
			auto FieldResult{Get_ID3_2_3_Tag_Header_Flags(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else if(MajorVersion == 0x04)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{1, 0}};
			auto FieldResult{Get_ID3_2_4_Tag_Header_Flags(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
		else
		{
			Result->GetValue()->AppendTag("error", "The major version of the tag (" + to_string_cast(MajorVersion) + ") cannot be handled!"s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
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
		if(Reader.Has(Inspection::Length{0, 8}) == true)
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_28Bit_SynchSafe_32Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 32}) == true)
	{
		if(Reader.Get1Bits() == 0x00)
		{
			std::uint32_t First{Reader.Get7Bits()};
			
			if(Reader.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Reader.Get7Bits()};
				
				if(Reader.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Reader.Get7Bits()};
					
					if(Reader.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Reader.Get7Bits()};
						
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_UnsignedInteger_32Bit_SynchSafe_40Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 40}) == true)
	{
		if(Reader.Get4Bits() == 0x00)
		{
			std::uint32_t First{Reader.Get4Bits()};
			
			if(Reader.Get1Bits() == 0x00)
			{
				std::uint32_t Second{Reader.Get7Bits()};
				
				if(Reader.Get1Bits() == 0x00)
				{
					std::uint32_t Third{Reader.Get7Bits()};
					
					if(Reader.Get1Bits() == 0x00)
					{
						std::uint32_t Fourth{Reader.Get7Bits()};
						
						if(Reader.Get1Bits() == 0x00)
						{
							std::uint32_t Fifth{Reader.Get7Bits()};
							
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_GUID(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{16, 0}};
		auto FieldResult{Get_GUID_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		const Inspection::GUID & GUID{std::experimental::any_cast< const Inspection::GUID & >(Result->GetAny())};
		
		try
		{
			Result->GetValue()->PrependTag("interpretation", Inspection::Get_GUID_Interpretation(GUID));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Result->GetValue()->PrependTag("interpretation", "<unknown GUID>"s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("standard", "IEC 60908:1999"s);
	Result->GetValue()->AppendTag("name", "Compact Disc Digital Audio"s);
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Header(FieldReader)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FirstTrackNumber{std::experimental::any_cast< std::uint8_t >(Result->GetAny("FirstTrackNumber"))};
		auto LastTrackNumber{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LastTrackNumber"))};
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Tracks(Buffer, FirstTrackNumber, LastTrackNumber)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FirstTrackNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("LastTrackNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Track(Buffer)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = (Result->GetValue("Number")->HasTag("interpretation") == true) && (std::experimental::any_cast< const std::string & >(Result->GetValue("Number")->GetTagAny("interpretation")) == "Lead-Out");
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length{0, 8})};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ADR", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetAny("ADR"));
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 4}};
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Track_Control(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Control", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}};
		auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Number", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto Number{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Number"))};
		
		if(Number == 0xaa)
		{
			Result->GetValue("Number")->PrependTag("interpretation", "Lead-Out"s);
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Bits_Unset_EndedByLength(Buffer, Inspection::Length{0, 8})};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("StartAddress", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 4}};
		auto FieldResult{Get_BitSet_4Bit_MostSignificantBitFirst(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 4 > & Control{std::experimental::any_cast< const std::bitset< 4 > & >(Result->GetAny())};
		
		if(Control[1] == true)
		{
			if(Control[0] == true)
			{
				Continue = false;
				
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_639_2_1998_Code(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{3, 0}};
		auto FieldResult{Get_ASCII_String_Alphabetic_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		const std::string & Code{std::experimental::any_cast< const std::string & >(Result->GetAny())};
		
		try
		{
			auto Interpretation{Get_LanguageName_From_ISO_639_2_1998_Code(Code)};
			
			Result->GetValue()->PrependTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->PrependTag("interpretation", Interpretation);
		}
		catch(...)
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("empty"s);
		}
		else
		{
			auto NumberOfCharacters{0ul};
			
			while((Continue == true) && (Reader.HasRemaining() == true))
			{
				auto Character{Reader.Get8Bits()};
				
				if(Is_ISO_IEC_8859_1_1998_Character(Character) == true)
				{
					NumberOfCharacters += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Character);
				}
				else
				{
					Result->GetValue()->AppendTag("ended by error"s);
					Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
					Continue = false;
				}
			}
			Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	//reading
	if(Continue == true)
	{
		auto NumberOfCharacters{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Character{Reader.Get8Bits()};
			
			if(Character == 0x00)
			{
				Result->GetValue()->AppendTag("ended by termination"s);
				Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				
				break;
			}
			else if(Is_ISO_IEC_8859_1_1998_Character(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Character);
			}
			else
			{
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	//finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AppendTag("ended by length"s);
			Result->GetValue()->AppendTag("empty"s);
		}
		else
		{
			auto NumberOfCharacters{0ul};
			
			while((Continue == true) && (Reader.HasRemaining() == true))
			{
				auto Byte{Reader.Get8Bits()};
				
				if(Byte == 0x00)
				{
					if(Reader.HasRemaining() == true)
					{
						Result->GetValue()->AppendTag("ended by termination"s);
					}
					else
					{
						Result->GetValue()->AppendTag("ended by termination and length"s);
					}
					Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters + termination");
					
					break;
				}
				else if(Is_ISO_IEC_8859_1_1998_Character(Byte) == true)
				{
					NumberOfCharacters += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Byte);
					if(Reader.IsAtEnd() == true)
					{
						Result->GetValue()->AppendTag("ended by length"s);
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
					}
				}
				else
				{
					Result->GetValue()->AppendTag("ended by error"s);
					Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
					Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
					Continue = false;
				}
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AppendTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		auto NumberOfTerminations{0ul};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Byte{Reader.Get8Bits()};
			
			if(Byte == 0x00)
			{
				NumberOfTerminations += 1;
			}
			else if(Is_ISO_IEC_8859_1_1998_Character(Byte) == true)
			{
				if(NumberOfTerminations == 0)
				{
					NumberOfCharacters += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Byte);
				}
				else
				{
					Result->GetValue()->AppendTag("ended by error"s);
					Result->GetValue()->AppendTag("error", "After the first termination byte only terminations are allowed, but the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not."s);
					Continue = false;
				}
			}
			else
			{
				Result->GetValue()->AppendTag("ended by error"s);
				Result->GetValue()->AppendTag("error", "The " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not an ISO/IEC 8859-1:1998 character or termination.");
				Continue = false;
			}
		}
		if(NumberOfCharacters > 0)
		{
			Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		else
		{
			Result->GetValue()->AppendTag("empty"s);
		}
		if(NumberOfTerminations > 0)
		{
			Result->GetValue()->AppendTag("ended by termination"s);
			if(Reader.IsAtEnd() == true)
			{
				Result->GetValue()->AppendTag(to_string_cast(NumberOfTerminations) + " terminations until length");
			}
		}
		else
		{
			Result->GetValue()->AppendTag("ended by length"s);
			Result->GetValue()->AppendTag("error", "The string must be ended by at least one termination."s);
			Continue = false;
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Buffer & Buffer, const Inspection::Length & Length)
{
	auto Boundary{Buffer.GetPosition() + Length};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	Result->GetValue()->AppendTag("string"s);
	Result->GetValue()->AppendTag("ISO/IEC 8859-1:1998"s);
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		
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
				auto FieldResult{Get_ISO_IEC_8859_1_1998_Character(Buffer)};
				
				if((Buffer.GetPosition() <= Boundary) && (FieldResult->GetSuccess() == true))
				{
					NumberOfCharacters += 1;
					Value << std::experimental::any_cast< const std::string & >(FieldResult->GetAny());
					if(Buffer.GetPosition() == Boundary)
					{
						Result->GetValue()->AppendTag("ended by length"s);
						Result->GetValue()->AppendTag(to_string_cast(NumberOfCharacters) + " characters");
						
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
						
						break;
					}
				}
				else
				{
					Continue = false;
				}
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	if(Buffer.Has(2ull, 0) == true)
	{
		auto BytesResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{2, 0})};
		
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
		auto BytesResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Buffer, Inspection::Length{2, 0})};
		
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength() != Inspection::Length{4, 0})
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be exactly " + to_string_cast(Inspection::Length{4, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::vector< std::uint8_t > Data;
		
		Data.push_back(Reader.Get8Bits());
		Data.push_back(Reader.Get8Bits());
		Data.push_back(Reader.Get8Bits());
		Data.push_back(Reader.Get8Bits());
		Result->GetValue()->SetAny(*reinterpret_cast< const float * const >(&(Data.front())));
		Result->GetValue()->AppendTag("floating point"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("standard", "ISO/IEC/IEEE-60559:2011 binary32"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Microsoft_WaveFormat_FormatTag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	if(Continue == true)
	{
		auto FormatTag{std::experimental::any_cast< std::uint16_t >(Result->GetAny())};
		
		switch(FormatTag)
		{
		case 0x0000:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_UNKNOWN"s);
				Result->GetValue()->AppendTag("description", "Unknown or invalid format tag"s);
				
				break;
			}
		case 0x0001:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_PCM"s);
				Result->GetValue()->AppendTag("description", "Pulse Code Modulation"s);
				
				break;
			}
		case 0x0002:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_ADPCM"s);
				Result->GetValue()->AppendTag("description", "Microsoft Adaptive Differental PCM"s);
				
				break;
			}
		case 0x0003:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_IEEE_FLOAT"s);
				Result->GetValue()->AppendTag("description", "32-bit floating-point"s);
				
				break;
			}
		case 0x0055:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_MPEGLAYER3"s);
				Result->GetValue()->AppendTag("description", "ISO/MPEG Layer3"s);
				
				break;
			}
		case 0x0092:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_DOLBY_AC3_SPDIF"s);
				Result->GetValue()->AppendTag("description", "Dolby Audio Codec 3 over S/PDIF"s);
				
				break;
			}
		case 0x0161:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_WMAUDIO2"s);
				Result->GetValue()->AppendTag("description", "Windows Media Audio Standard (Versions 7, 8, and 9 Series)"s);
				
				break;
			}
		case 0x0162:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_WMAUDIO3"s);
				Result->GetValue()->AppendTag("description", "Windows Media Audio Professional (9 Series)"s);
				
				break;
			}
		case 0x0163:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_WMAUDIO_LOSSLESS"s);
				Result->GetValue()->AppendTag("description", "Windows Media Audio Lossless (9 Series)"s);
				
				break;
			}
		case 0x0164:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_WMASPDIF"s);
				Result->GetValue()->AppendTag("description", "Windows Media Audio over S/PDIF"s);
				
				break;
			}
		case 0xFFFE:
			{
				Result->GetValue()->AppendTag("constant name", "WAVE_FORMAT_EXTENSIBLE"s);
				Result->GetValue()->AppendTag("description", "All new audio formats"s);
				
				break;
			}
		default:
			{
				Result->GetValue()->AppendTag("constant name", "<no interpretation>"s);
				Result->GetValue()->AppendTag("description", "<no interpretation>"s);
				
				break;
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_Frame(Inspection::Buffer & Buffer)
{
	auto Start{Buffer.GetPosition()};
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{4, 0}};
		auto FieldResult{Get_MPEG_1_FrameHeader(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValueAny("ProtectionBit"))};
		
		if(ProtectionBit == 0x00)
		{
			Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 16}};
			auto FieldResult{Get_BitSet_16Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ErrorCheck", FieldResult->GetValue())};
			
			UpdateState(Continue, Buffer, FieldResult, FieldReader);
		}
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValueAny("LayerDescription"))};
		auto BitRate{std::experimental::any_cast< std::uint32_t >(Result->GetValue("Header")->GetValue("BitRateIndex")->GetTagAny("numeric"))};
		auto SamplingFrequency{std::experimental::any_cast< std::uint32_t >(Result->GetValue("Header")->GetValue("SamplingFrequency")->GetTagAny("numeric"))};
		auto PaddingBit{std::experimental::any_cast< std::uint8_t >(Result->GetValue("Header")->GetValueAny("PaddingBit"))};
		auto FrameLength{0ul};
		
		if(LayerDescription == 0x03)
		{
			FrameLength = (12 * BitRate / SamplingFrequency + PaddingBit) * 4;
		}
		else if((LayerDescription == 0x01) || (LayerDescription == 0x02))
		{
			FrameLength = 144 * BitRate / SamplingFrequency + PaddingBit;
		}
		
		Inspection::Reader FieldReader{Buffer, Inspection::Length{FrameLength, 0} + Start - Buffer.GetPosition()};
		auto FieldResult{Get_Bits_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioData", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 12}};
		auto FieldResult{Get_Bits_Set_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FrameSync", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_MPEG_1_FrameHeader_AudioVersionID(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioVersionID", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_MPEG_1_FrameHeader_LayerDescription(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("LayerDescription", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_MPEG_1_FrameHeader_ProtectionBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ProtectionBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 4}};
		auto FieldResult{Get_MPEG_1_FrameHeader_BitRateIndex(FieldReader, LayerDescription)};
		auto FieldValue{Result->GetValue()->AppendValue("BitRateIndex", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_MPEG_1_FrameHeader_SamplingFrequency(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("SamplingFrequency", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_MPEG_1_FrameHeader_PaddingBit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PaddingBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PrivateBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_MPEG_1_FrameHeader_Mode(FieldReader, LayerDescription)};
		auto FieldValue{Result->GetValue()->AppendValue("Mode", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetAny("LayerDescription"))};
		auto Mode{std::experimental::any_cast< std::uint8_t >(Result->GetAny("Mode"))};
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_MPEG_1_FrameHeader_ModeExtension(FieldReader, LayerDescription, Mode)};
		auto FieldValue{Result->GetValue()->AppendValue("ModeExtension", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_MPEG_1_FrameHeader_Copyright(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Copyright", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_MPEG_1_FrameHeader_OriginalHome(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Original/Home", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_MPEG_1_FrameHeader_Emphasis(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Emphasis", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_AudioVersionID(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
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

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Reader & Reader, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 4}};
		auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto BitRateIndex{std::experimental::any_cast< std::uint8_t >(Result->GetAny())};
		
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
				Continue = false;
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
				Continue = false;
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
				Continue = false;
			}
		}
		else
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Copyright(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Emphasis(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_LayerDescription(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Mode(Inspection::Reader & Reader, std::uint8_t LayerDescription)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Reader & Reader, std::uint8_t LayerDescription, std::uint8_t Mode)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_OriginalHome(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
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
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
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
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
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

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_SamplingFrequency(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	//reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
		auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	switch(Bits)
	{
	case 1:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
			auto FieldResult{Get_SignedInteger_1Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 12:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 12}};
			auto FieldResult{Get_SignedInteger_12Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	default:
		{
			throw NotImplementedException("Reading " + to_string_cast(Bits) + " bits as a signed integer is not yet implemented in the generic function.");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_1Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 1}) == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Reader.Get1Bits() << 7) >> 7)};
		
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("1bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_5Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 5}) == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Reader.Get5Bits() << 3) >> 3)};
		
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("5bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_12Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 12}) == true)
	{
		std::int16_t Value{0};
		
		Value |= static_cast< std::int16_t >(static_cast< std::int16_t >(Reader.Get4Bits() << 12) >> 4);
		Value |= static_cast< std::int16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("12bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 32}) == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 32}) == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("signed"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("little endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_RiceEncoded(Inspection::Reader & Reader, std::uint8_t RiceParameter)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_UnsignedInteger_32Bit_AlternativeUnary(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("MostSignificantBits", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_UnsignedInteger_BigEndian(FieldReader, RiceParameter)};
		auto FieldValue{Result->GetValue()->AppendValue("LeastSignificantBits", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
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
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Reader, std::bind(Inspection::Get_SignedInteger_BigEndian, std::placeholders::_1, Bits), NumberOfElements);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	switch(Bits)
	{
	case 0:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 0}};
			auto FieldResult{Get_UnsignedInteger_0Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 1:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
			auto FieldResult{Get_UnsignedInteger_1Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 2:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 2}};
			auto FieldResult{Get_UnsignedInteger_2Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 3:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 3}};
			auto FieldResult{Get_UnsignedInteger_3Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 4:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 4}};
			auto FieldResult{Get_UnsignedInteger_4Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 5:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 5}};
			auto FieldResult{Get_UnsignedInteger_5Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 6:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 6}};
			auto FieldResult{Get_UnsignedInteger_6Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 7:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 7}};
			auto FieldResult{Get_UnsignedInteger_7Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 8:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 8}};
			auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 9:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 9}};
			auto FieldResult{Get_UnsignedInteger_9Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 10:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 10}};
			auto FieldResult{Get_UnsignedInteger_10Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 11:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 11}};
			auto FieldResult{Get_UnsignedInteger_11Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 12:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 12}};
			auto FieldResult{Get_UnsignedInteger_12Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 13:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 13}};
			auto FieldResult{Get_UnsignedInteger_13Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 14:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 14}};
			auto FieldResult{Get_UnsignedInteger_14Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 15:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 15}};
			auto FieldResult{Get_UnsignedInteger_15Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 16:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 16}};
			auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	case 17:
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 17}};
			auto FieldResult{Get_UnsignedInteger_17Bit_BigEndian(FieldReader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			
			break;
		}
	default:
		{
			throw NotImplementedException("Reading " + to_string_cast(Bits) + " bits as an unsigned integer is not yet implemented in the generic function.");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_4Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 4}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get4Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("4bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_6Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 6}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get6Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("6bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_7Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 7}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get7Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("7bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 8}) == true)
	{
		Result->GetValue()->SetAny(Reader.Get8Bits());
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("8bit"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	std::uint8_t Value{0ul};
	
	while(true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == true)
		{
			auto Bit{Reader.Get1Bits()};
			
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
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_10Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 10}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get2Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("10bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_11Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 11}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get3Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("11bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_12Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 12}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get4Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("12bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_13Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 13}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get5Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("13bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_14Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 14}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get6Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("14bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_15Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 15}) == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get7Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("15bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0};
		
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("big endian"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0};
		
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits()) << 8;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("16bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_17Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 17}) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get1Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("17bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_20Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 20}) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get4Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("20bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_24Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 24}) == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("24bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Reader.Has(Inspection::Length{1, 0}) == true)
	{
		auto First{Reader.Get8Bits()};
		
		if((First & 0x80) == 0x00)
		{
			Result->GetValue()->SetAny(static_cast< std::uint32_t >(First));
		}
		else if((First & 0xe0) == 0xc0)
		{
			if(Reader.Has(Inspection::Length{1, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				
				if((Second & 0xc0) == 0x80)
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x1f) << 6) | static_cast< std::uint32_t >(Second & 0x3f));
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
		else if((First & 0xf0) == 0xe0)
		{
			if(Reader.Has(Inspection::Length{2, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x0f) << 12)| static_cast< std::uint32_t >((Second & 0x3f) << 6) | static_cast< std::uint32_t >(Third & 0x3f));
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
		else if((First & 0xf8) == 0xf0)
		{
			if(Reader.Has(Inspection::Length{3, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x07) << 18)| static_cast< std::uint32_t >((Second & 0x3f) << 12) | static_cast< std::uint32_t >((Third & 0x3f) << 6) | static_cast< std::uint32_t >(Fourth & 0x3f));
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
		else if((First & 0xfc) == 0xf8)
		{
			if(Reader.Has(Inspection::Length{4, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				auto Fifth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x03) << 24)| static_cast< std::uint32_t >((Second & 0x3f) << 18) | static_cast< std::uint32_t >((Third & 0x3f) << 12) | static_cast< std::uint32_t >((Fourth & 0x3f) << 6) | static_cast< std::uint32_t >(Fifth & 0x3f));
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
		else if((First & 0xfe) == 0xfc)
		{
			if(Reader.Has(Inspection::Length{5, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				auto Fifth{Reader.Get8Bits()};
				auto Sixth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x01) << 30)| static_cast< std::uint32_t >((Second & 0x3f) << 24) | static_cast< std::uint32_t >((Third & 0x3f) << 18) | static_cast< std::uint32_t >((Fourth & 0x3f) << 12) | static_cast< std::uint32_t >((Fifth & 0x3f) << 6) | static_cast< std::uint32_t >(Sixth & 0x3f));
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	std::uint32_t Value{0};
	
	while(true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == true)
		{
			auto Bit{Reader.Get1Bits()};
			
			if(Bit == 0x00)
			{
				Value += 1;
			}
			else
			{
				Result->GetValue()->SetAny(Value);
				Result->GetValue()->AppendTag("integer"s);
				Result->GetValue()->AppendTag("unsigned"s);
				Result->GetValue()->AppendTag("length", Reader.GetConsumedLength());
				Result->GetValue()->AppendTag("alternative unary"s);
				
				break;
			}
		}
		else
		{
			Continue = false;
			
			break;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("big endian"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("32bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Reader.Has(Inspection::Length{1, 0}) == true)
	{
		auto First{Reader.Get8Bits()};
		
		if((First & 0x80) == 0x00)
		{
			Result->GetValue()->SetAny(static_cast< std::uint32_t >(First));
		}
		else if((First & 0xe0) == 0xc0)
		{
			if(Reader.Has(Inspection::Length{1, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				
				if((Second & 0xc0) == 0x80)
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x1f) << 6) | static_cast< std::uint32_t >(Second & 0x3f));
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
		else if((First & 0xf0) == 0xe0)
		{
			if(Reader.Has(Inspection::Length{2, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x0f) << 12)| static_cast< std::uint32_t >((Second & 0x3f) << 6) | static_cast< std::uint32_t >(Third & 0x3f));
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
		else if((First & 0xf8) == 0xf0)
		{
			if(Reader.Has(Inspection::Length{3, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x07) << 18)| static_cast< std::uint32_t >((Second & 0x3f) << 12) | static_cast< std::uint32_t >((Third & 0x3f) << 6) | static_cast< std::uint32_t >(Fourth & 0x3f));
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
		else if((First & 0xfc) == 0xf8)
		{
			if(Reader.Has(Inspection::Length{4, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				auto Fifth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x03) << 24)| static_cast< std::uint32_t >((Second & 0x3f) << 18) | static_cast< std::uint32_t >((Third & 0x3f) << 12) | static_cast< std::uint32_t >((Fourth & 0x3f) << 6) | static_cast< std::uint32_t >(Fifth & 0x3f));
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
		else if((First & 0xfe) == 0xfc)
		{
			if(Reader.Has(Inspection::Length{5, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				auto Fifth{Reader.Get8Bits()};
				auto Sixth{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((First & 0x01) << 30)| static_cast< std::uint32_t >((Second & 0x3f) << 24) | static_cast< std::uint32_t >((Third & 0x3f) << 18) | static_cast< std::uint32_t >((Fourth & 0x3f) << 12) | static_cast< std::uint32_t >((Fifth & 0x3f) << 6) | static_cast< std::uint32_t >(Sixth & 0x3f));
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
		else if((First & 0xff) == 0xfe)
		{
			if(Reader.Has(Inspection::Length{6, 0}) == true)
			{
				auto Second{Reader.Get8Bits()};
				auto Third{Reader.Get8Bits()};
				auto Fourth{Reader.Get8Bits()};
				auto Fifth{Reader.Get8Bits()};
				auto Sixth{Reader.Get8Bits()};
				auto Seventh{Reader.Get8Bits()};
				
				if(((Second & 0xc0) == 0x80) && ((Third & 0xc0) == 0x80) && ((Fourth & 0xc0) == 0x80) && ((Fifth & 0xc0) == 0x80) && ((Sixth & 0xc0) == 0x80) && ((Seventh & 0xc0) == 0x80))
				{
					Result->GetValue()->SetAny(static_cast< std::uint32_t >((Second & 0x3f) << 30) | static_cast< std::uint32_t >((Third & 0x3f) << 24) | static_cast< std::uint32_t >((Fourth & 0x3f) << 18) | static_cast< std::uint32_t >((Fifth & 0x3f) << 12) | static_cast< std::uint32_t >((Sixth & 0x3f) << 6) | static_cast< std::uint32_t >(Seventh & 0x3f));
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_64Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	if(Reader.Has(Inspection::Length{0, 64}) == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 56;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 48;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 40;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("64bit"s);
		Result->GetValue()->AppendTag("big endian"s);
		Result->SetSuccess(true);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 64}) == false)
		{
			Result->GetValue()->AppendTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 64}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint64_t Value{0};
		
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits());
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 40;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 48;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 56;
		Result->GetValue()->SetAny(Value);
		Result->GetValue()->AppendTag("integer"s);
		Result->GetValue()->AppendTag("unsigned"s);
		Result->GetValue()->AppendTag("64bit"s);
		Result->GetValue()->AppendTag("little endian"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Reader, std::bind(Inspection::Get_UnsignedInteger_BigEndian, std::placeholders::_1, Bits), NumberOfElements);
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_16Bit_BigEndian(Inspection::Reader & Reader, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Reader, Get_UnsignedInteger_16Bit_BigEndian, NumberOfElements);
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	assert(Buffer.GetBitstreamType() == Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Vorbis_CommentHeader_WithoutFramingFlag(Buffer)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}};
		auto FieldResult{Get_Boolean_1Bit(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("FramingFlag", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< bool >(Result->GetAny("FramingFlag"));
	}
	//finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Length", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->AppendTag("unit", "bytes"s);
	}
	// reading
	if(Continue == true)
	{
		auto Length{std::experimental::any_cast< std::uint32_t >(Result->GetAny("Length"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, Inspection::Length{Length, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Length", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->AppendTag("unit", "items"s);
	}
	// reading
	if(Continue == true)
	{
		auto Length{std::experimental::any_cast< std::uint32_t >(Result->GetAny("Length"))};
		
		for(auto Index = 0ul; (Continue == true) && (Index < Length); ++Index)
		{
			auto FieldResult{Get_Vorbis_CommentHeader_UserComment(Buffer)};
			auto FieldValue{Result->GetValue()->AppendValue("UserComment", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 32}};
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("VendorLength", FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldValue->AppendTag("unit", "bytes"s);
	}
	// reading
	if(Continue == true)
	{
		auto VendorLength{std::experimental::any_cast< std::uint32_t >(Result->GetAny("VendorLength"))};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Buffer, Inspection::Length{VendorLength, 0})};
		auto FieldValue{Result->GetValue()->AppendValue("Vendor", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Vorbis_CommentHeader_UserCommentList(Buffer)};
		auto FieldValue{Result->GetValue()->AppendValue("UserCommentList", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}
