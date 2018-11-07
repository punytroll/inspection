#include <bitset>
#include <experimental/optional>
#include <functional>
#include <sstream>
#include <vector>

#include "buffer.h"
#include "getter_repository.h"
#include "getters.h"
#include "guid.h"
#include "helper.h"
#include "id3_helper.h"
#include "not_implemented_exception.h"
#include "reader.h"
#include "string_cast.h"
#include "unknown_value_exception.h"

using namespace std::string_literals;

bool g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples{false};

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

std::shared_ptr< Inspection::Value > AppendLength(std::shared_ptr< Inspection::Value > Value, Inspection::Length Length)
{
	auto Tag{std::make_shared< Inspection::Value >()};
	
	Tag->SetName("length");
	Tag->SetAny(Length);
	Tag->AddTag("unit", "bytes and bits"s);
	Value->AddTag(Tag);
	
	return Tag;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Readers & Getters                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Flags(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
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
		const std::bitset<32> & TagsFlags{std::experimental::any_cast< const std::bitset<32> & >(Result->GetValue()->GetAny())};
		auto FlagValue{Result->GetValue()->AppendValue("TagOrItemIsReadOnly", TagsFlags[0])};
		
		FlagValue->AddTag("bit index", "0"s);
		if((TagsFlags[1] == false) && (TagsFlags[2] == false))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(0));
			FlagValue->AddTag("interpretation", "Item contains text information coded in UTF-8"s);
		}
		else if((TagsFlags[1] == true) && (TagsFlags[2] == false))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(1));
			FlagValue->AddTag("interpretation", "Item contains binary information"s);
		}
		else if((TagsFlags[1] == false) && (TagsFlags[2] == true))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(2));
			FlagValue->AddTag("interpretation", "Item is a locator of external stored information"s);
		}
		else if((TagsFlags[1] == true) && (TagsFlags[2] == true))
		{
			FlagValue = Result->GetValue()->AppendValue("ItemValueType", static_cast< std::uint8_t >(3));
			FlagValue->AddTag("interpretation", "<reserved>"s);
		}
		FlagValue->AddTag("bit indices", "1 to 2"s);
		
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
			FlagValue->AddTag("bit indices", "3 to 28"s);
		}
		else
		{
			Result->GetValue()->AddTag("error", "All bits 3 to 28 must be unset."s);
		}
		FlagValue = Result->GetValue()->AppendValue("Type", TagsFlags[29]);
		if(TagsFlags[29] == true)
		{
			FlagValue->AddTag("interpretation", "This is the header, not the footer"s);
		}
		else
		{
			FlagValue->AddTag("interpretation", "This is the footer, not the header"s);
		}
		FlagValue->AddTag("bit index", 29);
		FlagValue = Result->GetValue()->AppendValue("TagContainsAFooter", TagsFlags[30]);
		FlagValue->AddTag("bit index", 30);
		FlagValue = Result->GetValue()->AppendValue("TagContainsAHeader", TagsFlags[31]);
		FlagValue->AddTag("bit index", 31);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Tag(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"APE", "HeaderOrFooter"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("APETagsHeader", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto PartReader{Reader};
		auto ItemCount{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("APETagsHeader")->GetValue("ItemCount")->GetAny())};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"APE", "Item"}}, {"ElementName", "APETagsItem"s}, {"NumberOfElements", static_cast< std::uint64_t >(ItemCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("APETagsItems", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"APE", "HeaderOrFooter"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("APETagsFooter", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_APE_Item(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemValueSize", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_APE_Flags(Reader, {})};
		auto FieldValue{Result->GetValue()->AppendValue("ItemFlags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASCII_String_Printable_EndedByTermination(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ItemKey", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto ItemValueType{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("ItemFlags")->GetValue("ItemValueType")->GetAny())};
		
		if(ItemValueType == 0)
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("ItemValueSize")->GetAny()), 0}};
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(FieldReader, {})};
			auto FieldValue{Result->GetValue()->AppendValue("ItemValue", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else
		{
			throw Inspection::NotImplementedException("Can only interpret UTF-8 item values.");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	Result->GetValue()->AddTag("at least one element"s);
	// reading
	if(Continue == true)
	{
		std::uint64_t ElementIndex{0};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Getter(PartReader)};
			
			Continue = PartResult->GetSuccess();
			if(Continue == true)
			{
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				Result->GetValue()->AppendValue(PartResult->GetValue());
				PartResult->GetValue()->AddTag("array index", ElementIndex++);
			}
			else
			{
				Continue = true;
				
				break;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		else
		{
			Result->GetValue()->AddTag("ended by failure"s);
		}
		if(ElementIndex == 0)
		{
			Result->GetValue()->AddTag("error", "The array contains no elements, although at least one is required."s);
			Continue = false;
		}
		Result->GetValue()->AddTag("number of elements", ElementIndex);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	// reading
	if(Continue == true)
	{
		std::uint64_t ElementIndex{0};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			Inspection::Reader FieldReader{Reader};
			auto FieldResult{Getter(FieldReader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				Reader.AdvancePosition(FieldReader.GetConsumedLength());
				
				auto FieldValue{Result->GetValue()->AppendValue(FieldResult->GetValue())};
				
				FieldValue->AddTag("array index", ElementIndex++);
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		else
		{
			Result->GetValue()->AddTag("ended by failure"s);
		}
		Result->GetValue()->AddTag("number of elements", ElementIndex);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByLength(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &, const std::unordered_map< std::string, std::experimental::any > &) > Getter)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	// reading
	if(Continue == true)
	{
		std::uint64_t ElementIndex{0};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			Inspection::Reader ElementReader{Reader};
			auto ElementResult{Getter(ElementReader, {})};
			
			Continue = ElementResult->GetSuccess();
			
			auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
			
			ElementValue->AddTag("array index", ElementIndex++);
			Reader.AdvancePosition(ElementReader.GetConsumedLength());
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		else
		{
			Result->GetValue()->AddTag("ended by failure"s);
		}
		Result->GetValue()->AddTag("number of elements", ElementIndex);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByNumberOfElements(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	static const std::unordered_map< std::string, std::experimental::any > DefaultElementParameters{};
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	// reading
	if(Continue == true)
	{
		std::experimental::optional< std::string > ElementName;
		const auto * ElementParameters{&DefaultElementParameters};
		auto ElementParametersIterator{Parameters.find("ElementParameters")};
		
		if(ElementParametersIterator != Parameters.end())
		{
			ElementParameters = &(std::experimental::any_cast< const std::unordered_map< std::string, std::experimental::any > & >(ElementParametersIterator->second));
		}
		
		auto ElementNameIterator{Parameters.find("ElementName")};
		
		if(ElementNameIterator != Parameters.end())
		{
			ElementName = std::experimental::any_cast< std::string >(ElementNameIterator->second);
		}
		
		auto & Getter{std::experimental::any_cast< const std::vector< std::string > & >(Parameters.at("Getter"))};
		auto NumberOfElements{std::experimental::any_cast< std::uint64_t >(Parameters.at("NumberOfElements"))};
		std::uint64_t ElementIndex{0};
		
		while(true)
		{
			if(ElementIndex < NumberOfElements)
			{
				Inspection::Reader ElementReader{Reader};
				auto ElementResult{g_GetterRepository.Get(Getter, ElementReader, *ElementParameters)};
				std::shared_ptr< Inspection::Value > ElementValue;
				
				if(ElementName)
				{
					ElementValue = Result->GetValue()->AppendValue(ElementName.value(), ElementResult->GetValue());
				}
				else
				{
					ElementValue = Result->GetValue()->AppendValue(ElementResult->GetValue());
				}
				
				ElementValue->AddTag("array index", ElementIndex++);
				UpdateState(Continue, Reader, ElementResult, ElementReader);
				if(Continue == false)
				{
					Result->GetValue()->AddTag("ended by failure"s);
					
					break;
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by number of elements"s);
				
				break;
			}
		}
		Result->GetValue()->AddTag("number of elements", NumberOfElements);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByNumberOfElements_PassArrayIndex(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &, std::uint64_t) > Getter, std::uint64_t NumberOfElements)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	// reading
	if(Continue == true)
	{
		std::uint64_t ElementIndex{0};
		
		while(true)
		{
			if(ElementIndex < NumberOfElements)
			{
				auto FieldResult{Getter(Reader, ElementIndex)};
				auto FieldValue{Result->GetValue()->AppendValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
				FieldValue->AddTag("array index", ElementIndex++);
				if(Continue == false)
				{
					Result->GetValue()->AddTag("ended by failure"s);
					
					break;
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by number of elements"s);
				
				break;
			}
		}
		Result->GetValue()->AddTag("number of elements", NumberOfElements);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Array_EndedByPredicate(Inspection::Reader & Reader, std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Getter, std::function< bool (std::shared_ptr< Inspection::Value > PartValue) > EndedPredicate)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("array"s);
	Result->GetValue()->AddTag("ended by predicate"s);
	// reading
	if(Continue == true)
	{
		std::uint64_t ElementIndex{0};
		
		while(Continue == true)
		{
			Inspection::Reader ElementReader{Reader};
			auto ElementResult{Getter(ElementReader)};
			
			Continue = ElementResult->GetSuccess();
			
			auto ElementValue{Result->GetValue()->AppendValue(ElementResult->GetValue())};
			
			ElementValue->AddTag("array index", ElementIndex++);
			Reader.AdvancePosition(ElementReader.GetConsumedLength());
			if(Continue == true)
			{
				if(EndedPredicate(ElementValue) == true)
				{
					break;
				}
			}
		}
		Result->GetValue()->AddTag("number of elements", ElementIndex);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_Character_Alphabetic(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("character");
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphabetic");
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
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
	
	Result->GetValue()->AddTag("character");
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphanumeric");
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
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
	
	Result->GetValue()->AddTag("character");
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphanumeric or space");
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphabetic"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphabetic ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphanumeric"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphanumeric"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{TemplateString.size(), 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length must be at least " + to_string_cast(Inspection::Length{TemplateString.size(), 0}) + ", the length of the template string.");
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
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric ASCII character.");
				Continue = false;
				
				break;
			}
		}
		if(Continue == true)
		{
			Result->GetValue()->AddTag("ended by template"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters"s);
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphanumeric or space"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric or space ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("printable"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				Result->GetValue()->AddTag("ended by invalid character '" + to_string_cast(Character) + '\'');
				Reader.MoveBackPosition(Inspection::Length{1, 0});
				
				break;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASCII_String_Printable_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("printable"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not a printable ASCII character.");
				Continue = false;
			}
		}
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("printables only"s);
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
				Result->GetValue()->AddTag("ended by termination"s);
				Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				
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

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CodecListObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ASF_GUID(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Reserved", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("Reserved")->GetAny()) == Inspection::g_ASF_Reserved2GUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("CodecEntriesCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto CodecEntriesCount{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("CodecEntriesCount")->GetAny())};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "CodecList", "CodecEntry"}}, {"ElementName", "CodecEntry"s}, {"NumberOfElements", static_cast< std::uint64_t >(CodecEntriesCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("CodecEntries", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Profile")->GetAny()) == 0x02;
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
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Mode")->GetAny()) == 0x01;
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_CreationDate(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_64Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpreting
	if(Continue == true)
	{
		auto CreationDate{std::experimental::any_cast< std::uint64_t >(Result->GetValue()->GetAny())};
		
		Result->GetValue()->AppendValue("DateTime", Inspection::Get_DateTime_FromMicrosoftFileTime(CreationDate));
		Result->GetValue()->GetValue("DateTime")->AddTag("date and time"s);
		Result->GetValue()->GetValue("DateTime")->AddTag("from Microsoft filetime"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_DataObject(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ASF", "ObjectHeader"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("GUID")->GetAny())};
		
		Continue = GUID == Inspection::g_ASF_DataObjectGUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint64_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0} - Reader.GetConsumedLength()};
		auto PartResult{Get_Data_SetOrUnset_EndedByLength(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Data", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("NameLength", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("NameLength")->GetAny()), 0}};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(FieldReader, {})};
		auto FieldValue{Result->GetValue()->AppendValue("Name", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ASF", "ExtendedContentDescription", "ContentDescriptor_ValueDataType"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ValueDataType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ValueLength", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("ValueLength")->GetAny()), 0}};
		auto FieldResult{Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(FieldReader, std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("ValueDataType")->GetTag("interpretation")->GetAny()), std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Name")->GetAny()))};
		auto FieldValue{Result->GetValue()->AppendValue("Value", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Reader & Reader, const std::string & DataType, const std::string & Name)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Reader, {})};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				if(Name == "WM/MediaPrimaryClassID")
				{
					auto String{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetAny())};
					auto GUID{Inspection::Get_GUID_FromString_WithCurlyBraces(String)};
					
					Result->GetValue()->AppendValue("GUID", GUID);
					Result->GetValue()->GetValue("GUID")->AddTag("guid"s);
					Result->GetValue()->GetValue("GUID")->AddTag("string"s);
					
					auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
					
					Result->GetValue()->GetValue("GUID")->AddTag("interpretation", GUIDInterpretation);
				}
			}
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{g_GetterRepository.Get({"ASF", "Boolean_32Bit_LittleEndian"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->SetValue(PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
			{
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{8, 0})
			{
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
				// interpretation
				if(Continue == true)
				{
					if(Name == "WM/EncodingTime")
					{
						auto UnsignedInteger64Bit{std::experimental::any_cast< std::uint64_t >(Result->GetValue()->GetAny())};
						auto DateTime{Inspection::Get_DateTime_FromMicrosoftFileTime(UnsignedInteger64Bit)};
						
						Result->GetValue()->AppendValue("DateTime", DateTime);
						Result->GetValue()->GetValue("DateTime")->AddTag("date and time"s);
						Result->GetValue()->GetValue("DateTime")->AddTag("from Microsoft filetime"s);
					}
				}
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{2, 0})
			{
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedContentDescriptionObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ContentDescriptorsCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto ContentDescriptorsCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("ContentDescriptorsCount")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(Reader, {{"Getter", std::vector< std::string >{"ASF", "ExtendedContentDescription", "ContentDescriptor"s}}, {"ElementName", "ContentDescriptor"s}, {"NumberOfElements", static_cast< std::uint64_t >(ContentDescriptorsCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ContentDescriptors", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Reader & Reader)
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
		const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetValue()->GetAny())};
		
		Result->GetValue()->AppendValue("[0] Reliable", Flags[0]);
		Result->GetValue()->AppendValue("[1] Seekable", Flags[1]);
		Result->GetValue()->AppendValue("[2] No Cleanpoints", Flags[2]);
		Result->GetValue()->AppendValue("[3] Resend Live Cleanpoints", Flags[3]);
		Result->GetValue()->AppendValue("[4-31] Reserved", false);
		for(auto Index = 4; Index < 32; ++Index)
		{
			Continue &= !Flags[Index];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StartTime", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("EndTime", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "bits per second"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("BufferSize", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("InitialBufferFullness", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateDataBitrate", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "bits per second"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateBufferSize", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("AlternateInitialBufferFullness", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "milliseconds"s);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("MaximumObjectSize", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_ExtendedStreamPropertiesObject_Flags(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamLanguageIndex", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageTimePerFrame", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamNameCount", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("PayloadExtensionSystemCount", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		auto StreamNameCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("StreamNameCount")->GetAny())};
		auto PayloadExtensionSystemCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("PayloadExtensionSystemCount")->GetAny())};
		
		if((StreamNameCount != 0) || (PayloadExtensionSystemCount != 0))
		{
			throw Inspection::NotImplementedException("StreamNameCount != 0 || PayloadExtensionSystemCount != 0");
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.HasRemaining() == true)
		{
			auto FieldResult{Get_ASF_StreamPropertiesObject(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("StreamPropertiesObject", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		auto GUIDInterpretation{Get_GUID_Interpretation(std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetAny()))};
		
		Result->GetValue()->AddTag("interpretation", GUIDInterpretation);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderExtensionObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ASF_GUID(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ReservedField1", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("ReservedField1")->GetAny()) == Inspection::g_ASF_Reserved1GUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ReservedField2", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("ReservedField2")->GetAny()) == 0x0006;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("HeaderExtensionDataSize", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("HeaderExtensionDataSize")->GetAny()), 0}};
		auto PartResult{Get_Array_EndedByLength(PartReader, Get_ASF_Object)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("AdditionalExtendedHeaders", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("AdditionalExtendedHeader");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_File(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_ASF_HeaderObject(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("HeaderObject", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_ASF_DataObject(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("DataObject", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByLength(PartReader, Get_ASF_Object)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Objects", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Object");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_FileProperties_Flags(Inspection::Reader & Reader)
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
	//~ if(Continue == true)
	//~ {
		//~ const std::bitset< 32 > & Flags{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetValue()->GetAny())};
		
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObject(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ASF", "ObjectHeader"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("GUID")->GetAny()) == Inspection::g_ASF_HeaderObjectGUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint64_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0} - Reader.GetConsumedLength()};
		auto PartResult{Get_ASF_HeaderObjectData(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_HeaderObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfHeaderObjects", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved1", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Reserved1")->GetAny()) == 0x01;
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved2", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Reserved2")->GetAny()) == 0x02;
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfHeaderObjects{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("NumberOfHeaderObjects")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "Object"}}, {"ElementName", "HeaderObject"s}, {"NumberOfElements", static_cast< std::uint64_t >(NumberOfHeaderObjects)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("HeaderObjects", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_IndexPlaceholderObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength() != Inspection::Length{10, 0})
		{
			Result->GetValue()->AddTag("error", "The available length needs to be exactly " + to_string_cast(Inspection::Length{10, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Data_Unset_EndedByLength(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_LanguageListObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("LanguageIDRecordsCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto LanguageIDRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("LanguageIDRecordsCount")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "LanguageList", "LanguageIDRecord"}}, {"ElementName", "LanguageIDRecord"s}, {"NumberOfElements", static_cast< std::uint64_t >(LanguageIDRecordsCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("LanguageIDRecords", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	auto & DataType{std::experimental::any_cast< const std::string & >(Parameters.at("DataType"))};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Reader, {})};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{2, 0})
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{g_GetterRepository.Get({"ASF", "Boolean_16Bit_LittleEndian"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->SetValue(PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
			{
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{8, 0})
			{
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{2, 0})
			{
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "GUID")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{16, 0})
			{
				auto FieldResult{Get_GUID_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
				// interpretation
				if(Continue == true)
				{
					auto GUID{std::experimental::any_cast< const Inspection::GUID & >(Result->GetValue()->GetAny())};
					auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
					
					Result->GetValue()->AddTag("interpretation", GUIDInterpretation);
				}
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{16, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataLibraryObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("DescriptionRecordsCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("DescriptionRecordsCount")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "MetadataLibrary", "DescriptionRecord"}}, {"ElementName", "DescriptionRecord"s}, {"NumberOfElements", static_cast< std::uint64_t >(DescriptionRecordsCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("DescriptionRecords", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Metadata_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	auto & DataType{std::experimental::any_cast< const std::string & >(Parameters.at("DataType"))};
	
	// reading
	if(Continue == true)
	{
		if(DataType == "Unicode string")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Reader, {})};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Byte array")
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(DataType == "Boolean")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{2, 0})
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{g_GetterRepository.Get({"ASF", "Boolean_16Bit_LittleEndian"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->SetValue(PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 32bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
			{
				auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{4, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 64bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{8, 0})
			{
				auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{8, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else if(DataType == "Unsigned integer 16bit")
		{
			if(Reader.GetRemainingLength() == Inspection::Length{2, 0})
			{
				auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
			else
			{
				Result->GetValue()->AddTag("error", "The length of the data should be " + to_string_cast(Inspection::Length{2, 0}) + ", not " + to_string_cast(Reader.GetRemainingLength()) + ".");
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_MetadataObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("DescriptionRecordsCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto DescriptionRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("DescriptionRecordsCount")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "Metadata", "DescriptionRecord"}}, {"ElementName", "DescriptionRecord"s}, {"NumberOfElements", static_cast< std::uint64_t >(DescriptionRecordsCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("DescriptionRecords", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_Object(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ASF", "ObjectHeader"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Length Size{std::experimental::any_cast< std::uint64_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0};
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("GUID")->GetAny())};
		
		if(GUID == Inspection::g_ASF_CompatibilityObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_CompatibilityObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_HeaderObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_HeaderObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_FilePropertiesObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{g_GetterRepository.Get({"ASF", "ObjectData", "FileProperties"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_StreamPropertiesObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_CodecListObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_CodecListObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_HeaderExtensionObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_HeaderExtensionObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_LanguageListObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_LanguageListObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_ExtendedStreamPropertiesObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_ExtendedStreamPropertiesObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_MetadataObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_MetadataObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_IndexPlaceholderObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_IndexPlaceholderObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_PaddingObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_Data_Unset_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Data", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_ExtendedContentDescriptionObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_ExtendedContentDescriptionObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_StreamBitratePropertiesObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_StreamBitratePropertiesObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_ContentDescriptionObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{g_GetterRepository.Get({"ASF", "ObjectData", "ContentDescription"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(GUID == Inspection::g_ASF_MetadataLibraryObjectGUID)
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_ASF_MetadataLibraryObjectData(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader, Size - Reader.GetConsumedLength()};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Data", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
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
		//~ auto ReservedResult{Get_Data_Unset_EndedByLength(Buffer, Inspection::Length(0ull, 9))};
		
		//~ Result->GetValue()->AppendValue("[7-15] Reserved", ReservedResult->GetValue());
		//~ Continue = ReservedResult->GetSuccess();
	//~ }
	//~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamBitratePropertiesObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_16Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BitrateRecordsCount", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto BitrateRecordsCount{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("BitrateRecordsCount")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"ASF", "StreamBitrateProperties", "BitrateRecord"}}, {"ElementName", "BitrateRecord"s}, {"NumberOfElements", static_cast< std::uint64_t >(BitrateRecordsCount)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BitrateRecords", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
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
		//~ Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValueAny("[7-14] Reserved")) == 0x00;
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("AudioMedia"s);
	Result->GetValue()->AddTag("WAVEFORMATEX"s);
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"Microsoft_WaveFormat_FormatTag"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FormatTag", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfChannels", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("SamplesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("AverageNumberOfBytesPerSecond", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockAlignment", FieldResult->GetValue())};
		
		FieldValue->AddTag("unit", "bytes"s);
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FliedValue{Result->GetValue()->AppendValue("BitsPerSample", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_16Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("CodecSpecificDataSize", FieldResult->GetValue())};
		
		FieldValue->AddTag("unit", "bytes"s);
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FormatTag{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("FormatTag")->GetTag("constant name")->GetAny())};
		
		if(FormatTag == "WAVE_FORMAT_WMAUDIO2")
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("CodecSpecificDataSize")->GetAny()), 0}};
			auto PartResult{g_GetterRepository.Get({"ASF", "StreamProperties", "TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("CodecSpecificData", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("CodecSpecificDataSize")->GetAny()), 0}};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("CodecSpecificData", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamPropertiesObject(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ASF", "ObjectHeader"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("GUID")->GetAny()) == Inspection::g_ASF_StreamPropertiesObjectGUID;
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ASF_StreamPropertiesObjectData(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ASF_StreamPropertiesObjectData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_GUID(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_GUID(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ErrorCorrectionType", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_64Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("TimeOffset", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificDataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ErrorCorrectionDataLength", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASF_StreamProperties_Flags(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto StreamType{std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("StreamType")->GetAny())};
		
		if(StreamType == Inspection::g_ASF_AudioMediaGUID)
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("TypeSpecificDataLength")->GetAny()), 0}};
			auto FieldResult{Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("TypeSpecificDataLength")->GetAny()), 0}};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("TypeSpecificData", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
	}
	// reading
	if(Continue == true)
	{
		auto ErrorCorrectionType{std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetValue("ErrorCorrectionType")->GetAny())};
		
		if(ErrorCorrectionType == Inspection::g_ASF_AudioSpreadGUID)
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("ErrorCorrectionDataLength")->GetAny()), 0}};
			auto PartResult{g_GetterRepository.Get({"ASF", "StreamProperties", "ErrorCorrectionData_AudioSpread"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("ErrorCorrectionData", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("ErrorCorrectionDataLength")->GetAny()), 0}};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("ErrorCorrectionData", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("bitset"s);
	Result->GetValue()->AddTag("4bit"s);
	Result->GetValue()->AddTag("most significant bit first"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 4}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 4}) + ".");
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
		Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{Byte1});
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
	
	Result->GetValue()->AddTag("bitset"s);
	Result->GetValue()->AddTag("8bit"s);
	Result->GetValue()->AddTag("least significant bit first"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 8}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 8}) + ".");
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
		Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{Byte1});
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
	
	Result->GetValue()->AddTag("bitset"s);
	Result->GetValue()->AddTag("16bit"s);
	Result->GetValue()->AddTag("big endian"s);
	Result->GetValue()->AddTag("least significant bit first per byte"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
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
		Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{Byte1, Byte2});
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
	
	Result->GetValue()->AddTag("bitset"s);
	Result->GetValue()->AddTag("16bit"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("least significant bit first per byte"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
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
		Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{Byte1, Byte2});
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
	
	Result->GetValue()->AddTag("bitset"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("least significant bit first per byte"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
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
		Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{Byte1, Byte2, Byte3, Byte4});
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
	
	Result->GetValue()->AddTag("boolean"s);
	Result->GetValue()->AddTag("1bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 1}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Bit{Reader.Get1Bits()};
		
		Result->GetValue()->SetAny((0x01 & Bit) == 0x01);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("buffer"s);
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("8bit values"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::vector< std::uint8_t > Value;
		
		while(Reader.HasRemaining() == true)
		{
			Value.push_back(Reader.Get8Bits());
		}
		Result->GetValue()->SetAny(Value);
		AppendLength(Result->GetValue(), Reader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("buffer"s);
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("8bit values"s);
	Result->GetValue()->AddTag("zeroed");
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::vector< std::uint8_t > Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Byte{Reader.Get8Bits()};
			
			Value.push_back(Byte);
			if(Byte != 0x00)
			{
				Result->GetValue()->AddTag("error", "The " + to_string_cast(Value.size()) + "th byte was not zeroed.");
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value);
		AppendLength(Result->GetValue(), Reader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_Set_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("set data"s);
	// reading
	if(Continue == true)
	{
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			Continue = Reader.Get1Bits() == 0x01;
		}
	}
	// interpretation
	if(Continue == true)
	{
		AppendLength(Result->GetValue(), Reader.GetConsumedLength());
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_SetOrUnset_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("any data"s);
	
	auto RemainingLength{Reader.GetRemainingLength()};
	
	Reader.AdvancePosition(RemainingLength);
	AppendLength(Result->GetValue(), RemainingLength);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_SetOrUnset_Until16BitAlignment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("any data"s);
	Result->GetValue()->AddTag("until 16bit alignment");
	
	Inspection::Length LengthUntil16BitAlignment;
	auto OutOfAlignmentBits{Reader.GetPositionInBuffer().GetTotalBits() % 16};
	
	if(OutOfAlignmentBits > 0)
	{
		LengthUntil16BitAlignment.Set(0, 16 - OutOfAlignmentBits);
		if(Reader.Has(LengthUntil16BitAlignment) == true)
		{
			Reader.AdvancePosition(LengthUntil16BitAlignment);
		}
		else
		{
			Result->GetValue()->AddTag("error", "The next 16bit alignment is not inside the reader's available data length.");
		}
	}
	AppendLength(Result->GetValue(), LengthUntil16BitAlignment);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_Unset_Until16BitAlignment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("unset data"s);
	Result->GetValue()->AddTag("until 16bit alignment");
	
	Inspection::Length LengthUntil16BitAlignment;
	auto OutOfAlignmentBits{Reader.GetPositionInBuffer().GetTotalBits() % 16};
	
	if(OutOfAlignmentBits > 0)
	{
		LengthUntil16BitAlignment.Set(0, 16 - OutOfAlignmentBits);
		if(Reader.Has(LengthUntil16BitAlignment) == true)
		{
			while((Continue == true) && (Reader.GetConsumedLength() < LengthUntil16BitAlignment))
			{
				Continue = Reader.Get1Bits() == 0x00;
			}
			if(Continue == false)
			{
				Result->GetValue()->AddTag("error", "Only " + to_string_cast(Reader.GetConsumedLength()) + " bytes and bits could be read as unset data.");
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The next 16bit alignment is not inside the reader's available data length.");
		}
	}
	AppendLength(Result->GetValue(), LengthUntil16BitAlignment);
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_Unset_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("unset data"s);
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
		AppendLength(Result->GetValue(), Reader.GetConsumedLength());
	}
	else
	{
		Result->GetValue()->AddTag("error", "Of the requested " + to_string_cast(Reader.GetCompleteLength()) + " bytes and bits, only " + to_string_cast(Reader.GetConsumedLength()) + " could be read as unset data.");
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Data_Unset_Until8BitAlignment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, static_cast< std::uint8_t >((8 - Reader.GetPositionInBuffer().GetBits()) % 8)}};
		auto FieldResult{Get_Data_Unset_EndedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
		Result->GetValue()->AddTag("until 8bit alignment"s);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_ApplicationBlock_Data(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("RegisteredApplicationIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{4, 0}};
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
		
		Result->GetValue()->GetValue("RegisteredApplicationIdentifier")->AddTag("bytes", FieldResult->GetValue()->GetAny());
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{4, 0}};
		auto FieldResult{Get_ASCII_String_Printable_EndedByLength(FieldReader, {})};
		
		UpdateState(Continue, FieldResult);
		if(Continue == true)
		{
			Reader.AdvancePosition(FieldReader.GetConsumedLength());
			Result->GetValue()->GetValue("RegisteredApplicationIdentifier")->AddTag("string interpretation", FieldResult->GetValue()->GetAny());
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("ApplicationData", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame(Inspection::Reader & Reader, std::uint8_t NumberOfChannelsByStream)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Frame_Header(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// inspect
	if(Continue == true)
	{
		auto NumberOfChannelsByFrame{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("ChannelAssignment")->GetTag("value")->GetAny())};
		
		if(NumberOfChannelsByStream != NumberOfChannelsByFrame)
		{
			Result->GetValue()->AddTag("error", "The number of channels from the stream (" + to_string_cast(NumberOfChannelsByStream) + ") does not match the number of channels from the frame (" + to_string_cast(NumberOfChannelsByFrame) + ").");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("Header")->GetValue("BlockSize")->GetTag("value")->GetAny())};
		auto BitsPerSample{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("SampleSize")->GetTag("value")->GetAny())};
		auto ChannelAssignment{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("ChannelAssignment")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements_PassArrayIndex(PartReader, std::bind(Get_FLAC_Subframe_CalculateBitsPerSample, std::placeholders::_1, std::placeholders::_2, BlockSize, BitsPerSample, ChannelAssignment), NumberOfChannelsByStream)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Subframes", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Subframe");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_Data_Unset_Until8BitAlignment(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Padding", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Frame_Footer"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Footer", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Frame_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_14Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("SyncCode", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("SyncCode")->GetAny()) == 0x3ffe;
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_1Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		// verification
		if(Continue == true)
		{
			Continue = std::experimental::any_cast< std::uint8_t >(FieldValue->GetAny()) == 0x00;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Frame_Header_BlockingStrategy"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BlockingStrategy", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_4Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("BlockSize", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("BlockSize")->GetAny())};
		
		if(BlockSize == 0x00)
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("reserved"s);
			Result->GetValue()->GetValue("BlockSize")->AddTag("error", "The block size 0 MUST NOT be used."s);
			Continue = false;
		}
		else if(BlockSize == 0x01)
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("value", static_cast< std::uint16_t >(192));
			Result->GetValue()->GetValue("BlockSize")->AddTag("unit", "samples"s);
		}
		else if((BlockSize > 0x01) && (BlockSize <= 0x05))
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("value", static_cast< std::uint16_t >(576 * (1 << (BlockSize - 2))));
			Result->GetValue()->GetValue("BlockSize")->AddTag("unit", "samples"s);
		}
		else if(BlockSize == 0x06)
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("interpretation", "get 8bit (blocksize - 1) from end of header"s);
		}
		else if(BlockSize == 0x07)
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("interpretation", "get 16bit (blocksize - 1) from end of header"s);
		}
		else if((BlockSize > 0x07) && (BlockSize < 0x10))
		{
			Result->GetValue()->GetValue("BlockSize")->AddTag("value", static_cast< std::uint16_t >(256 * (1 << (BlockSize - 8))));
			Result->GetValue()->GetValue("BlockSize")->AddTag("unit", "samples"s);
		}
		else
		{
			assert(false);
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Frame_Header_SampleRate"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SampleRate", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Frame_Header_ChannelAssignment"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ChannelAssignment", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Frame_Header_SampleSize"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SampleSize", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_1Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		// verification
		if(Continue == true)
		{
			Continue = std::experimental::any_cast< std::uint8_t >(FieldValue->GetAny()) == 0x00;
		}
	}
	// reading
	if(Continue == true)
	{
		auto BlockingStrategy{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("BlockingStrategy")->GetAny())};
		
		if(BlockingStrategy == 0x00)
		{
			auto FieldResult{Get_UnsignedInteger_31Bit_UTF_8_Coded(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("FrameNumber", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(BlockingStrategy == 0x01)
		{
			auto FieldResult{Get_UnsignedInteger_36Bit_UTF_8_Coded(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("SampleNumber", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AddTag("error", "Unknown blocking strategy value " + to_string_cast(BlockingStrategy) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto BlockSize{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("BlockSize")->GetAny())};
		
		if(BlockSize == 0x06)
		{
			auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("BlockSizeExplicit", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				auto BlockSizeValue{Result->GetValue()->GetValue("BlockSize")};
				
				BlockSizeValue->AddTag("value", static_cast< std::uint16_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("BlockSizeExplicit")->GetAny()) + 1));
				BlockSizeValue->AddTag("unit", "samples"s);
			}
		}
		else if(BlockSize == 0x07)
		{
			auto FieldResult{Get_UnsignedInteger_16Bit_BigEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("BlockSizeExplicit", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				auto BlockSizeValue{Result->GetValue()->GetValue("BlockSize")};
				
				BlockSizeValue->AddTag("value", static_cast< std::uint16_t >(std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("BlockSizeExplicit")->GetAny()) + 1));
				BlockSizeValue->AddTag("unit", "samples"s);
			}
		}
	}
	// reading
	if(Continue == true)
	{
		auto SampleRate{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("SampleRate")->GetAny())};
		
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
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("CRC-8", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_MetaDataBlock(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "MetaDataBlock_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Header", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		const std::string & MetaDataBlockType{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Header")->GetValue("BlockType")->GetTag("interpretation")->GetAny())};
		
		if(MetaDataBlockType == "StreamInfo")
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
			auto PartResult{g_GetterRepository.Get({"FLAC", "StreamInfoBlock_Data"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Data", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(MetaDataBlockType == "Padding")
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
			auto FieldResult{Get_Data_Unset_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(MetaDataBlockType == "Application")
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
			auto FieldResult{Get_FLAC_ApplicationBlock_Data(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(MetaDataBlockType == "SeekTable")
		{
			auto MetaDataBlockDataLength{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny())};
			
			if(MetaDataBlockDataLength % 18 == 0)
			{
				Inspection::Reader PartReader{Reader, Inspection::Length{MetaDataBlockDataLength, 0}};
				auto PartResult{Get_FLAC_SeekTableBlock_Data(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Continue = false;
			}
		}
		else if(MetaDataBlockType == "VorbisComment")
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
			auto FieldResult{Get_FLAC_VorbisCommentBlock_Data(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(MetaDataBlockType == "Picture")
		{
			Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
			auto PartResult{g_GetterRepository.Get({"FLAC", "PictureBlock_Data"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Data", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_SeekTableBlock_Data(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByLength(PartReader, std::bind(&Inspection::GetterRepository::Get, &g_GetterRepository, std::vector< std::string >({"FLAC", "SeekTableBlock_SeekPoint"}), std::placeholders::_1, std::unordered_map< std::string, std::experimental::any >()))};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SeekPoints", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("SeekPoint");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Stream(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_FLAC_Stream_Header(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfChannels{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("StreamInfoBlock")->GetValue("Data")->GetValue("NumberOfChannels")->GetTag("value")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, std::bind(Get_FLAC_Frame, std::placeholders::_1, NumberOfChannels))};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Frames", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Frame");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Stream_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "fLaC"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FLAC stream marker", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_StreamInfoBlock(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StreamInfoBlock", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto LastMetaDataBlock{std::experimental::any_cast< bool >(Result->GetValue()->GetValue("StreamInfoBlock")->GetValue("Header")->GetValue("LastMetaDataBlock")->GetAny())};
		
		if(LastMetaDataBlock == false)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Array_EndedByPredicate(PartReader, Get_FLAC_MetaDataBlock, [](std::shared_ptr< Inspection::Value > PartValue) { return std::experimental::any_cast< bool >(PartValue->GetValue("Header")->GetValue("LastMetaDataBlock")->GetAny()); })};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("MetaDataBlocks", PartResult->GetValue());
			for(auto PartValue : PartResult->GetValue()->GetValues())
			{
				PartValue->SetName("MetaDataBlock");
			}
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_StreamInfoBlock(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "MetaDataBlock_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Header", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		if(std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Header")->GetValue("BlockType")->GetTag("interpretation")->GetAny()) != "StreamInfo")
		{
			Result->GetValue()->AddTag("error", "The block type of the meta data block is not \"StreamInfo\"."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Length")->GetAny()), 0}};
		auto PartResult{g_GetterRepository.Get({"FLAC", "StreamInfoBlock_Data"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Data", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		Result->GetValue()->AddTag("value", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny()) + 1));
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		Result->GetValue()->AddTag("value", static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny()) + 1));
	}
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Header(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	if(Continue == true)
	{
		auto SubframeType{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Header")->GetValue("Type")->GetTag("interpretation")->GetAny())};
		
		if(SubframeType == "SUBFRAME_CONSTANT")
		{
			auto FieldResult{Get_FLAC_Subframe_Data_Constant(Reader, BitsPerSample)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(SubframeType == "SUBFRAME_FIXED")
		{
			auto FieldResult{Get_FLAC_Subframe_Data_Fixed(Reader, FrameBlockSize, BitsPerSample, std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("Type")->GetValue("Order")->GetAny()))};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(SubframeType == "SUBFRAME_LPC")
		{
			auto FieldResult{Get_FLAC_Subframe_Data_LPC(Reader, FrameBlockSize, BitsPerSample, static_cast< std::uint8_t >(std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("Type")->GetValue("Order")->GetAny()) + 1))};
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_CalculateBitsPerSample(Inspection::Reader & Reader, std::uint64_t SubFrameIndex, std::uint16_t BlockSize, std::uint8_t BitsPerSample, std::uint8_t ChannelAssignment)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(((SubFrameIndex == 0) && (ChannelAssignment == 0x09)) || ((SubFrameIndex == 1) && ((ChannelAssignment == 0x08) || (ChannelAssignment == 0x0a))))
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_FLAC_Subframe(PartReader, BlockSize, BitsPerSample + 1)};
			
			Continue = PartResult->GetSuccess();
			Result->SetValue(PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_FLAC_Subframe(PartReader, BlockSize, BitsPerSample)};
			
			Continue = PartResult->GetSuccess();
			Result->SetValue(PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Constant(Inspection::Reader & Reader, std::uint8_t BitsPerSample)
{
	return Get_UnsignedInteger_BigEndian(Reader, {{"Bits", BitsPerSample}});
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_Fixed(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedIntegers_BigEndian(Reader, BitsPerSample, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("WarmUpSamples", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Residual(Reader, FrameBlockSize, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("Residual", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Data_LPC(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedIntegers_BigEndian(Reader, BitsPerSample, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("WarmUpSamples", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_4Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientsPrecision", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto QuantizedLinearPredictorCoefficientsPrecision{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->GetAny())};
		
		if(QuantizedLinearPredictorCoefficientsPrecision < 15)
		{
			Result->GetValue()->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->AddTag("value", static_cast< std::uint8_t >(QuantizedLinearPredictorCoefficientsPrecision + 1));
		}
		else
		{
			Result->GetValue()->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->AddTag("error", "The percision MUST NOT be 15."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_SignedInteger_5Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("QuantizedLinearPredictorCoefficientShift", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_SignedIntegers_BigEndian(Reader, std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("QuantizedLinearPredictorCoefficientsPrecision")->GetTag("value")->GetAny()), PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("PredictorCoefficients", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Residual(Reader, FrameBlockSize, PredictorOrder)};
		auto FieldValue{Result->GetValue()->AppendValue("Residual", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 1}};
		auto FieldResult{Get_Data_Unset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("PaddingBit", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_FLAC_Subframe_Type(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Type", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_Boolean_1Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("WastedBitsPerSampleFlag", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		auto WastedBitsPerSampleFlag{std::experimental::any_cast< bool >(Result->GetValue()->GetValue("WastedBitsPerSampleFlag")->GetAny())};
		
		if(WastedBitsPerSampleFlag == true)
		{
			throw Inspection::NotImplementedException("Wasted bits are not implemented yet!");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"FLAC", "Subframe_Residual_CodingMethod"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("CodingMethod", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto CodingMethod{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("CodingMethod")->GetAny())};
		
		if(CodingMethod == 0x00)
		{
			auto FieldResult{Get_FLAC_Subframe_Residual_Rice(Reader, FrameBlockSize, PredictorOrder)};
			auto FieldValue{Result->GetValue()->AppendValue("CodedResidual", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				FieldValue->AddTag("Rice"s);
			}
		}
		else if(CodingMethod == 0x01)
		{
			auto FieldResult{Get_FLAC_Subframe_Residual_Rice2(Reader, FrameBlockSize, PredictorOrder)};
			auto FieldValue{Result->GetValue()->AppendValue("CodedResidual", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				FieldValue->AddTag("Rice2"s);
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

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_4Bit(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("PartitionOrder", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto NumberOfPartitions{static_cast< std::uint16_t >(1 << std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("PartitionOrder")->GetAny()))};
		
		Result->GetValue()->GetValue("PartitionOrder")->AddTag("number of partitions", NumberOfPartitions);
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfPartitions{std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("PartitionOrder")->GetTag("number of partitions")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements_PassArrayIndex(PartReader, std::bind(Get_FLAC_Subframe_Residual_Rice_Partition, std::placeholders::_1, std::placeholders::_2, FrameBlockSize / NumberOfPartitions, PredictorOrder), NumberOfPartitions)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Partitions", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Partition");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Reader & Reader, std::uint64_t ArrayIndex, std::uint32_t NumberOfSamples, std::uint8_t PredictorOrder)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_4Bit(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("RiceParameter", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto Rice{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("RiceParameter")->GetAny())};
		
		if(ArrayIndex == 0)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"Number", "Integer", "Signed", "32Bit_RiceEncoded"}}, {"ElementParameters", std::unordered_map< std::string, std::experimental::any >{{"Rice", Rice}}}, {"NumberOfElements", static_cast< std::uint64_t >(NumberOfSamples - PredictorOrder)}})};
			
			Continue = PartResult->GetSuccess();
			if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
			{
				Result->GetValue()->AppendValue("Samples", PartResult->GetValue());
			}
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"Number", "Integer", "Signed", "32Bit_RiceEncoded"}}, {"ElementParameters", std::unordered_map< std::string, std::experimental::any >{{"Rice", Rice}}}, {"NumberOfElements", static_cast< std::uint64_t > (NumberOfSamples)}})};
			
			Continue = PartResult->GetSuccess();
			if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
			{
				Result->GetValue()->AppendValue("Samples", PartResult->GetValue());
			}
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Residual_Rice2(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t PredictorOrder)
{
	throw Inspection::NotImplementedException("Get_FLAC_Subframe_Residual_Rice2");
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_Subframe_Type(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{0, 6}};
		auto FieldResult{Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto SubframeType{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny())};
		
		switch(SubframeType)
		{
		case 0:
			{
				Result->GetValue()->AddTag("interpretation", "SUBFRAME_LPC"s);
				
				auto FieldResult{Get_UnsignedInteger_5Bit(Reader)};
				auto FieldValue{Result->GetValue()->AppendValue("Order", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
				// interpretation
				if(Continue == true)
				{
					auto Order{std::experimental::any_cast< std::uint8_t >(FieldValue->GetAny())};
					
					FieldValue->AddTag("value", static_cast< std::uint8_t >(Order + 1));
				}
				
				break;
			}
		case 2:
			{
				Result->GetValue()->AddTag("interpretation", "SUBFRAME_FIXED"s);
				
				auto FieldResult{Get_UnsignedInteger_3Bit(Reader)};
				auto FieldValue{Result->GetValue()->AppendValue("Order", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
				// interpretation and verification
				if(Continue == true)
				{
					auto Order{std::experimental::any_cast< std::uint8_t >(FieldResult->GetValue()->GetAny())};
					
					FieldValue->AddTag("value", static_cast< std::uint8_t >(Order));
					if(Order >= 5)
					{
						Result->GetValue()->AddTag("reserved");
						Result->GetValue()->AddTag("error", "The subframe type is SUBFRAME_FIXED, and the order " + to_string_cast(Order) + " MUST NOT be used.");
						Continue = false;
					}
				}
				
				break;
			}
		case 1:
		case 3:
		case 4:
			{
				Result->GetValue()->AddTag("reserved"s);
				Result->GetValue()->AddTag("error", "This subframe type MUST NOT be used."s);
				Continue = false;
				
				break;
			}
		case 5:
			{
				throw Inspection::NotImplementedException("SUBFRAME_VERBATIM");
			}
		case 6:
			{
				Result->GetValue()->AddTag("interpretation", "SUBFRAME_CONSTANT"s);
				
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
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_FLAC_VorbisCommentBlock_Data(Inspection::Reader & Reader)
{
	return Get_Vorbis_CommentHeader_WithoutFramingFlag(Reader);
}

std::unique_ptr< Inspection::Result > Inspection::Get_GUID_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{16, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{16, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->AddTag("guid"s);
		Result->GetValue()->AddTag("binary"s);
		Result->GetValue()->AddTag("little endian"s);
		
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_1_Tag(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_String_ASCII_Alphabetic_ByTemplate(PartReader, {{"Template", "TAG"s}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Identifier", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{30, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Title", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{30, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Artist", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{30, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Album", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{4, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Year", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{30, 0}};
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(FieldReader)};
		
		UpdateState(Continue, FieldResult);
		if(Continue == true)
		{
			auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
			
			Reader.AdvancePosition(FieldReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{29, 0}};
			FieldResult = Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(FieldReader);
			
			auto FieldValue{Result->GetValue()->AppendValue("Comment", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
			// reading
			if(Continue == true)
			{
				auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
				auto FieldValue{Result->GetValue()->AppendValue("AlbumTrack", FieldResult->GetValue())};
				
				UpdateState(Continue, FieldResult);
			}
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_1_Genre(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Genre", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_1_Genre(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto GenreNumber{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny())};
		
		try
		{
			auto Genre{Inspection::Get_ID3_1_Genre(GenreNumber)};
			
			Result->GetValue()->AddTag("interpretation", Genre);
			Result->GetValue()->AddTag("standard", "ID3v1"s);
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			try
			{
				auto Genre{Inspection::Get_ID3_1_Winamp_Genre(GenreNumber)};
				
				Result->GetValue()->AddTag("interpretation", Genre);
				Result->GetValue()->AddTag("standard", "Winamp extension"s);
			}
			catch(Inspection::UnknownValueException & Exception)
			{
				Result->GetValue()->AddTag("interpretation", nullptr);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "Frame_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldStart{Reader.GetConsumedLength()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Identifier")->GetAny())};
		auto ClaimedSize{Inspection::Length(std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0)};
		
		if(Identifier == "COM")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "FrameBody", "COM"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "PIC")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "FrameBody", "PIC"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if((Identifier == "TAL") || (Identifier == "TCM") || (Identifier == "TCP") || (Identifier == "TEN") || (Identifier == "TP1") || (Identifier == "TP2") || (Identifier == "TPA") || (Identifier == "TRK") || (Identifier == "TT1") || (Identifier == "TT2") || (Identifier == "TYE"))
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "FrameBody", "T__"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "TCO")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{Get_ID3_2_2_Frame_Body_TCO(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "UFI")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "FrameBody", "UFI"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		
		auto HandledSize{Reader.GetConsumedLength() - FieldStart};
		
		if(HandledSize > ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		else if(HandledSize < ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed smaller than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		Reader.SetPosition(FieldStart + ClaimedSize);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Frame_Body_TCO(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.2", "FrameBody", "T__"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetAny())};
		auto Interpretation{GetContentTypeInterpretation2_3(Information)};
		
		if(std::get<0>(Interpretation) == true)
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", std::get<1>(Interpretation));
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Language(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader Alternative1Reader{Reader, Inspection::Length{3, 0}};
		auto Alternative1Result{Get_ISO_639_2_1998_Code(Alternative1Reader)};
		
		UpdateState(Continue, Alternative1Result);
		if(Continue == true)
		{
			Reader.AdvancePosition(Alternative1Reader.GetRemainingLength());
			Result->SetValue(Alternative1Result->GetValue());
		}
		else
		{
			Inspection::Reader Alternative2Reader{Reader, Inspection::Length{3, 0}};
			auto Alternative2Result{Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Alternative2Reader)};
			
			UpdateState(Continue, Reader, Alternative2Result, Alternative2Reader);
			Result->SetValue(Alternative2Result->GetValue());
			if(Continue == true)
			{
				Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->AddTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_Tag_Header_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_8Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetValue()->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AddTag("bit index", 7);
		Flag = Result->GetValue()->AppendValue("Compression", Flags[6]);
		Flag->AddTag("bit index", 6);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AddTag("bit index", 5);
		Flag->AddTag("bit index", 4);
		Flag->AddTag("bit index", 3);
		Flag->AddTag("bit index", 2);
		Flag->AddTag("bit index", 1);
		Flag->AddTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 5; ++FlagIndex)
		{
			Continue &= !Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AddTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AddTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "Frame_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldStart{Reader.GetConsumedLength()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Identifier")->GetAny())};
		auto ClaimedSize{Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0}};
		
		if(Identifier == "APIC")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "APIC"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "COMM")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "COMM"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "GEOB")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "GEOB"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "MCDI")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_MCDI(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "PCNT")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_PCNT(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "POPM")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_POPM(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "PRIV")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_PRIV(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}

		else if(Identifier == "RGAD")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "RGAD"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCOP") || (Identifier == "TDAT") || (Identifier == "TDRC") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIME") || (Identifier == "TIT1") || (Identifier == "TIT2") || (Identifier == "TIT3") || (Identifier == "TLEN") || (Identifier == "TMED") || (Identifier == "TOAL") || (Identifier == "TOFN") || (Identifier == "TOPE") || (Identifier == "TOWN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPE3") || (Identifier == "TPE4") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TRDA") || (Identifier == "TSIZ") || (Identifier == "TSO2") || (Identifier == "TSOA") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TSST") || (Identifier == "TYER"))
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "TCMP")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_TCMP(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TCON")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_TCON(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TFLT")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_TFLT(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TLAN")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_TLAN(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TSRC")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_3_Frame_Body_TSRC(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TXXX")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "TXXX"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "UFID")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "UFID"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "USLT")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "USLT"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if((Identifier == "WCOM") || (Identifier == "WOAF") || (Identifier == "WOAR"))
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "W___"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "WXXX")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "WXXX"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		
		auto HandledSize{Reader.GetConsumedLength() - FieldStart};
		
		if(HandledSize > ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		else if(HandledSize < ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed smaller than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		Reader.SetPosition(FieldStart + ClaimedSize);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_MCDI(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader Alternative1Reader{Reader};
		auto Alternative1Result{g_GetterRepository.Get({"IEC_60908_1999", "TableOfContents"}, Alternative1Reader, {})};
		
		UpdateState(Continue, Alternative1Result);
		if(Continue == true)
		{
			Result->SetValue(Alternative1Result->GetValue());
			Reader.AdvancePosition(Alternative1Reader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader Alternative2Reader{Reader};
			auto Alternative2Result{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Alternative2Reader)};
			
			UpdateState(Continue, Reader, Alternative2Result, Alternative2Reader);
			if(Continue == true)
			{
				Result->GetValue()->AppendValue("String", Alternative2Result->GetValue());
				Result->GetValue()->GetValue("String")->AddTag("error", "The content of an \"MCDI\" frame should be a binary compact disc table of contents, but is a unicode string encoded with UCS-2 in little endian."s);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_PCNT(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(Reader.GetRemainingLength() < Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			Result->GetValue()->GetValue("Counter")->AddTag("standard", "ID3 2.3"s);
			Result->GetValue()->GetValue("Counter")->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			Continue = false;
		}
		else if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
		{
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(Reader.GetRemainingLength() > Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			Result->GetValue()->GetValue("Counter")->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			Continue = false;
		}
	}
	// reading
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_POPM(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("EMailToUser", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Rating", FieldResult->GetValue())};
		
		FieldValue->AddTag("standard", "ID3 2.3"s);
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Rating{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Rating")->GetAny())};
		
		if(Rating > 0)
		{
			Result->GetValue()->GetValue("Rating")->AddTag("interpretation", to_string_cast(Rating) + " / 255");
		}
		else
		{
			Result->GetValue()->GetValue("Rating")->AddTag("interpretation", nullptr);
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.IsAtEnd() == true)
		{
			auto FieldValue{std::make_shared< Inspection::Value >()};
			
			FieldValue->SetName("Counter");
			FieldValue->AddTag("omitted"s);
			Result->GetValue()->AppendValue(FieldValue);
		}
		else if(Reader.GetRemainingLength() < Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			Result->GetValue()->GetValue("Counter")->AddTag("standard", "ID3 2.3"s);
			Result->GetValue()->GetValue("Counter")->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			UpdateState(Continue, FieldResult);
			Continue = false;
		}
		else if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
		{
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(Reader.GetRemainingLength() > Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			Result->GetValue()->GetValue("Counter")->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			UpdateState(Continue, FieldResult);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_PRIV(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("OwnerIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		const std::string & OwnerIdentifier{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("OwnerIdentifier")->GetAny())};
		
		if(OwnerIdentifier == "AverageLevel")
		{
			auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("AverageLevel", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "PeakValue")
		{
			auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("PeakValue", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/MediaClassPrimaryID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("MediaClassPrimaryID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/MediaClassSecondaryID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("MediaClassSecondaryID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/WMCollectionGroupID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("CollectionGroupID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/WMCollectionID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("CollectionID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/WMContentID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("ContentID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "ZuneAlbumArtistMediaID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneAlbumArtistMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "ZuneAlbumMediaID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneAlbumMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "ZuneCollectionID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneCollectionID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "ZuneMediaID")
		{
			auto FieldResult{Get_ID3_GUID(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("ZuneMediaID", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/Provider")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Reader, {})};
			auto FieldValue{Result->GetValue()->AppendValue("Provider", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(OwnerIdentifier == "WM/UniqueFileIdentifier")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Reader, {})};
			auto FieldValue{Result->GetValue()->AppendValue("UniqueFileIdentifier", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("PrivateData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCMP(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetTag("value")->GetAny())};
		
		if(Information == "1")
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", "yes, this is part of a compilation"s);
		}
		else if(Information == "0")
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", "no, this is not part of a compilation"s);
		}
		else
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", nullptr);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TCON(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetTag("value")->GetAny())};
		auto Interpretation{GetContentTypeInterpretation2_3(Information)};
		
		if(std::get<0>(Interpretation) == true)
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", std::get<1>(Interpretation));
		}
	}
	// finalization
	Result->SetSuccess(true);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TFLT(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->GetValue("Information")->AddTag("standard", "ID3 2.3"s);
		
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetTag("value")->GetAny())};
		
		try
		{
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", Get_ID3_2_3_FileType_Interpretation(Information));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			if(Information == "/3")
			{
				Result->GetValue()->GetValue("Information")->AddTag("error", "The file type could not be interpreted strictly according to the standard, but this seems plausible."s);
				Result->GetValue()->GetValue("Information")->AddTag("interpretation", "MPEG 1/2 layer III");
			}
			else
			{
				Result->GetValue()->GetValue("Information")->AddTag("error", "The file type could not be interpreted."s);
				Result->GetValue()->GetValue("Information")->AddTag("interpretation", nullptr);
			}
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TLAN(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		try
		{
			auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetTag("value")->GetAny())};
			
			Result->GetValue()->GetValue("Information")->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", Inspection::Get_LanguageName_From_ISO_639_2_1998_Code(Information));
		}
		catch(...)
		{
			Result->GetValue()->GetValue("Information")->AddTag("standard", "ID3 2.3"s);
			Result->GetValue()->GetValue("Information")->AddTag("error", "The language frame needs to contain a three letter code from ISO 639-2:1998 (alpha-3)."s);
			Result->GetValue()->GetValue("Information")->AddTag("interpretation", nullptr);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Body_TSRC(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.3", "FrameBody", "T___"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto Information{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Information")->GetTag("value")->GetAny())};
		
		if(Information.length() == 12)
		{
			Result->GetValue()->GetValue("Information")->AddTag("standard", "ISRC Bulletin 2015/01"s);
			Result->GetValue()->GetValue("Information")->AddTag("DesignationCode", Information.substr(7, 5));
			Result->GetValue()->GetValue("Information")->AddTag("YearOfReference", Information.substr(5, 2));
			Result->GetValue()->GetValue("Information")->AddTag("RegistrantCode", Information.substr(2, 3));
			
			std::string CountryCode{Information.substr(0, 2)};
			auto CountryCodeValue{Result->GetValue()->GetValue("Information")->AddTag("CountryCode", CountryCode)};
			
			try
			{
				CountryCodeValue->AddTag("standard", "ISO 3166-1 alpha-2"s);
				CountryCodeValue->AddTag("interpretation", Inspection::Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(CountryCode));
			}
			catch(Inspection::UnknownValueException & Exception)
			{
				CountryCodeValue->AddTag("standard", "ISRC Bulletin 2015/01"s);
				CountryCodeValue->AddTag("error", "The ISRC string needs to contain a two letter country code from ISO 3166-1 alpha-2."s);
				CountryCodeValue->AddTag("interpretation", nullptr);
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->GetValue("Information")->AddTag("standard", "ID3 2.3"s);
			Result->GetValue()->GetValue("Information")->AddTag("error", "The TSRC frame needs to contain a twelve letter ISRC code from ISRC Bulletin 2015/01."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Frame_Header_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_16Bit_BigEndian(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 16 > & Flags{std::experimental::any_cast< const std::bitset< 16 > & >(Result->GetValue()->GetAny())};
		std::shared_ptr< Inspection::Value > FlagValue;
		
		FlagValue = Result->GetValue()->AppendValue("TagAlterPreservation", Flags[15]);
		FlagValue->AddTag("bit index", 15);
		FlagValue->AddTag("bit name", "a"s);
		if(Flags[15] == true)
		{
			FlagValue->AddTag("interpretation", "Frame should be discarded."s);
		}
		else
		{
			FlagValue->AddTag("interpretation", "Frame should be preserved."s);
		}
		FlagValue = Result->GetValue()->AppendValue("FileAlterPreservation", Flags[14]);
		FlagValue->AddTag("bit index", 14);
		FlagValue->AddTag("bit name", "b"s);
		if(Flags[14] == true)
		{
			FlagValue->AddTag("interpretation", "Frame should be discarded."s);
		}
		else
		{
			FlagValue->AddTag("interpretation", "Frame should be preserved."s);
		}
		FlagValue = Result->GetValue()->AppendValue("ReadOnly", Flags[13]);
		FlagValue->AddTag("bit index", 13);
		FlagValue->AddTag("bit name", "c"s);
		FlagValue = Result->GetValue()->AppendValue("Reserved", false);
		for(auto FlagIndex = 8; FlagIndex <= 12; ++FlagIndex)
		{
			FlagValue->AddTag("bit index", FlagIndex);
			Continue &= !Flags[FlagIndex];
		}
		FlagValue = Result->GetValue()->AppendValue("Compression", Flags[7]);
		FlagValue->AddTag("bit index", 7);
		FlagValue->AddTag("bit name", "i"s);
		if(Flags[7] == true)
		{
			FlagValue->AddTag("interpretation", "Frame is compressed using ZLIB with 4 bytes for 'decompressed size' appended to the frame header."s);
			FlagValue->AddTag("error", "Frame compression is not yet implemented!");
		}
		else
		{
			FlagValue->AddTag("interpretation", "Frame is not compressed."s);
		}
		FlagValue = Result->GetValue()->AppendValue("Encryption", Flags[6]);
		FlagValue->AddTag("bit index", 6);
		FlagValue->AddTag("bit name", "j"s);
		if(Flags[6] == true)
		{
			FlagValue->AddTag("interpretation", "Frame is encrypted."s);
			FlagValue->AddTag("error", "Frame encryption is not yet implemented!");
		}
		else
		{
			FlagValue->AddTag("interpretation", "Frame is not encrypted."s);
		}
		FlagValue = Result->GetValue()->AppendValue("GroupingIdentity", Flags[5]);
		FlagValue->AddTag("bit index", 5);
		FlagValue->AddTag("bit name", "k"s);
		if(Flags[5] == true)
		{
			FlagValue->AddTag("interpretation", "Frame contains group information."s);
			FlagValue->AddTag("error", "Frame grouping is not yet implemented!");
		}
		else
		{
			FlagValue->AddTag("interpretation", "Frame does not contain group information."s);
		}
		FlagValue = Result->GetValue()->AppendValue("Reserved", false);
		for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
		{
			FlagValue->AddTag("bit index", FlagIndex);
			Continue &= !Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_Language(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader Alternative1Reader{Reader, Inspection::Length{3, 0}};
		auto Alternative1Result{Get_ISO_639_2_1998_Code(Alternative1Reader)};
		
		UpdateState(Continue, Alternative1Result);
		if(Continue == true)
		{
			Result->SetValue(Alternative1Result->GetValue());
			Reader.AdvancePosition(Alternative1Reader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader Alternative2Reader{Reader, Inspection::Length{3, 0}};
			auto Alternative2Result{Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Alternative2Reader)};
			
			UpdateState(Continue, Reader, Alternative2Result, Alternative2Reader);
			if(Continue == true)
			{
				Result->SetValue(Alternative2Result->GetValue());
				Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
				Result->GetValue()->AddTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
			}
			else
			{
				Result->GetValue()->AddTag("error", "Could not read a language for ID3v2.3."s);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetValue()->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AddTag("bit index", 7);
		Flag->AddTag("bit name", "a"s);
		Flag = Result->GetValue()->AppendValue("ExtendedHeader", Flags[6]);
		Flag->AddTag("bit index", 6);
		Flag->AddTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("ExperimentalIndicator", Flags[5]);
		Flag->AddTag("bit index", 5);
		Flag->AddTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AddTag("bit index", 4);
		Flag->AddTag("bit index", 3);
		Flag->AddTag("bit index", 2);
		Flag->AddTag("bit index", 1);
		Flag->AddTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
		{
			Continue &= !Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetValue("String")->GetAny());
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetValue("String")->GetAny());
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "Frame_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->AddTag("content", Result->GetValue()->GetValue("Identifier")->GetTag("interpretation")->GetAny());
		
		auto FieldStart{Reader.GetConsumedLength()};
		const std::string & Identifier{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Identifier")->GetAny())};
		auto ClaimedSize{Inspection::Length(std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Size")->GetAny()), 0)};
		
		if(Identifier == "APIC")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "APIC"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "COMM")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "COMM"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "MCDI")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "MCDI"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "POPM")
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_4_Frame_Body_POPM(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "PRIV")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "PRIV"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCON") || (Identifier == "TCOP") || (Identifier == "TDRC") || (Identifier == "TDRL") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIT2") || (Identifier == "TLAN") || (Identifier == "TLEN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TYER"))
		{
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_ID3_2_4_Frame_Body_T___(FieldReader)};
			
			Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else if(Identifier == "TCMP")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{Get_ID3_2_4_Frame_Body_TCMP(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "TXXX")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "TXXX"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "UFID")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "UFID"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "USLT")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "USLT"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "WCOM")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "W___"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(Identifier == "WXXX")
		{
			Inspection::Reader PartReader{Reader, ClaimedSize};
			auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "FrameBody", "WXXX"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
			
			Inspection::Reader FieldReader{Reader, ClaimedSize};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("Data", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		
		auto HandledSize{Reader.GetConsumedLength() - FieldStart};
		
		if(HandledSize > ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		else if(HandledSize < ClaimedSize)
		{
			Result->GetValue()->AddTag("error", "The frame size is claimed smaller than the actually handled size."s);
			Result->GetValue()->AddTag("claimed size", to_string_cast(ClaimedSize));
			Result->GetValue()->AddTag("handled size", to_string_cast(HandledSize));
		}
		Reader.SetPosition(FieldStart + ClaimedSize);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_POPM(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("EMailToUser", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Rating", FieldResult->GetValue())};
		
		FieldValue->AddTag("standard", "ID3 2.3"s);
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Rating{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Rating")->GetAny())};
		
		if(Rating > 0)
		{
			Result->GetValue()->GetValue("Rating")->AddTag("interpretation", to_string_cast(Rating) + " / 255");
		}
		else
		{
			Result->GetValue()->GetValue("Rating")->AddTag("interpretation", nullptr);
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.IsAtEnd() == true)
		{
			auto CounterValue{std::make_shared< Inspection::Value >()};
			
			CounterValue->SetName("Counter");
			CounterValue->AddTag("omitted"s);
			Result->GetValue()->AppendValue(CounterValue);
		}
		else if(Reader.GetRemainingLength() < Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			FieldValue->AddTag("standard", "ID3 2.4"s);
			FieldValue->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
			UpdateState(Continue, FieldResult);
			Continue = false;
		}
		else if(Reader.GetRemainingLength() == Inspection::Length{4, 0})
		{
			auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(Reader.GetRemainingLength() > Inspection::Length{4, 0})
		{
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Counter", FieldResult->GetValue())};
			
			FieldValue->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
			UpdateState(Continue, FieldResult);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_T___(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "TextEncoding"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("TextEncoding", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByLength(PartReader, std::bind(Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength, std::placeholders::_1, std::unordered_map< std::string, std::experimental::any >{{"TextEncoding", Result->GetValue()->GetValue("TextEncoding")->GetAny()}}))};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Informations", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Information");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Frame_Body_TCMP(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Frame_Body_T___(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		for(auto PartValue : Result->GetValue()->GetValue("Informations")->GetValues())
		{
			auto Information{std::experimental::any_cast< const std::string & >(PartValue->GetTag("value")->GetAny())};
			
			if(Information == "1")
			{
				PartValue->AddTag("interpretation", "yes, this is part of a compilation"s);
			}
			else if(Information == "0")
			{
				PartValue->AddTag("interpretation", "no, this is not part of a compilation"s);
			}
			else
			{
				PartValue->AddTag("error", "The value \"" + Information + "\" could not interpreted.");
				PartValue->AddTag("interpretation", nullptr);
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Language(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{3, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
			Continue = false;
		}
	}
	// reader
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{3, 0}};
		auto FieldResult{Get_ISO_639_2_1998_Code(FieldReader)};
		
		UpdateState(Continue, FieldResult);
		if(Continue == true)
		{
			Result->SetValue(FieldResult->GetValue());
			Reader.AdvancePosition(FieldReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{3, 0}};
			auto FieldResult{Get_ASCII_String_Alphabetic_EndedByLength(FieldReader)};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->SetValue(FieldResult->GetValue());
				Reader.AdvancePosition(FieldReader.GetConsumedLength());
				
				const std::string & Code{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetAny())};
				
				if(Code == "XXX")
				{
					Result->GetValue()->AddTag("standard", "ID3 2.4"s);
					Result->GetValue()->AddTag("interpretation", "the language is unknown"s);
				}
				else
				{
					Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
					Result->GetValue()->AddTag("error", "The language code \"" + Code + "\" is unknown."s);
					Continue = false;
				}
			}
			else
			{
				Inspection::Reader FieldReader{Reader, Inspection::Length{3, 0}};
				auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(FieldReader)};
				auto FieldValue{Result->SetValue(FieldResult->GetValue())};
				
				UpdateState(Continue, Reader, FieldResult, FieldReader);
				// interpretation
				if(Continue == true)
				{
					Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
					Result->GetValue()->AddTag("error", "The language code consists of three null bytes. Although common, this is not valid."s);
				}
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("NumberOfFlagBytes", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		auto NumberOfFlagBytes{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("NumberOfFlagBytes")->GetAny())};
		
		if(NumberOfFlagBytes != 0x01)
		{
			Result->GetValue()->AddTag("error", "According to the standard, the number of flag bytes must be equal to 1."s);
			Result->GetValue()->AddTag("standard", "ID3 2.4"s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader_Flags(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ExtendedFlags", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue()->GetValue("ExtendedFlags")->GetValue("TagIsAnUpdate")->GetAny()) == true)
		{
			auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("TagIsAnUpdateData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue()->GetValue("ExtendedFlags")->GetValue("CRCDataPresent")->GetAny()) == true)
		{
			auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("CRCDataPresentData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	if(Continue == true)
	{
		if(std::experimental::any_cast< bool >(Result->GetValue()->GetValue("ExtendedFlags")->GetValue("TagRestrictions")->GetAny()) == true)
		{
			auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("TagRestrictionsData", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_CRCDataPresent(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Size")->GetAny()) == 0x05;
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("TotalFrameCRC", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Size")->GetAny()) == 0x00;
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Header"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		if(std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Size")->GetAny()) != 0x01)
		{
			Result->GetValue()->AddTag("error", "The size of the tag restriction flags is not equal to 1."s); 
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Restrictions", FieldResult->GetValue())};
		
		FieldValue->AddTag("error", "This program is missing the interpretation of the tag restriction flags."s); 
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_8Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		Result->GetValue()->AddTag("synchsafe"s);
		
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetValue()->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Reserved", Flags[7])};
		
		Flag->AddTag("bit index", 7);
		Flag = Result->GetValue()->AppendValue("TagIsAnUpdate", Flags[6]);
		Flag->AddTag("bit index", 6);
		Flag->AddTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("CRCDataPresent", Flags[5]);
		Flag->AddTag("bit index", 5);
		Flag->AddTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("TagRestrictions", Flags[4]);
		Flag->AddTag("bit index", 4);
		Flag->AddTag("bit name", "d"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AddTag("bit index", 3);
		Flag->AddTag("bit index", 2);
		Flag->AddTag("bit index", 1);
		Flag->AddTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Continue &= !Flags[FlagIndex];
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
		auto FieldResult{Get_BitSet_8Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 8 > & Flags{std::experimental::any_cast< const std::bitset< 8 > & >(Result->GetValue()->GetAny())};
		auto Flag{Result->GetValue()->AppendValue("Unsynchronization", Flags[7])};
		
		Flag->AddTag("bit index", 7);
		Flag->AddTag("bit name", "a"s);
		Flag = Result->GetValue()->AppendValue("ExtendedHeader", Flags[6]);
		Flag->AddTag("bit index", 6);
		Flag->AddTag("bit name", "b"s);
		Flag = Result->GetValue()->AppendValue("ExperimentalIndicator", Flags[5]);
		Flag->AddTag("bit index", 5);
		Flag->AddTag("bit name", "c"s);
		Flag = Result->GetValue()->AppendValue("FooterPresent", Flags[4]);
		Flag->AddTag("bit index", 4);
		Flag->AddTag("bit name", "d"s);
		Flag = Result->GetValue()->AppendValue("Reserved", false);
		Flag->AddTag("bit index", 3);
		Flag->AddTag("bit index", 2);
		Flag->AddTag("bit index", 1);
		Flag->AddTag("bit index", 0);
		for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
		{
			Continue &= !Flags[FlagIndex];
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetValue("String")->GetAny());
			}
		}
		else if(TextEncoding == 0x02)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x03)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto TextEncoding{std::experimental::any_cast< std::uint8_t >(Parameters.at("TextEncoding"))};
		
		if(TextEncoding == 0x00)
		{
			auto FieldResult{Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x01)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetValue("String")->GetAny());
			}
		}
		else if(TextEncoding == 0x02)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
			}
		}
		else if(TextEncoding == 0x03)
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->SetValue(FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
			// interpretation
			if(Continue == true)
			{
				Result->GetValue()->AddTag("value", FieldResult->GetValue()->GetAny());
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

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_Tag(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_2_Tag_Header(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("TagHeader", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("TagHeader")->GetValue("MajorVersion")->GetAny())};
		auto Size{Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("TagHeader")->GetValue("Size")->GetAny()), 0}};
		
		if(MajorVersion == 0x02)
		{
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader, Size};
				auto PartResult{Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, Get_ID3_2_2_Frame)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Frames", PartResult->GetValue());
				for(auto PartValue : PartResult->GetValue()->GetValues())
				{
					PartValue->SetName("Frame");
				}
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				Size -= PartReader.GetConsumedLength();
			}
			// reading
			if(Continue == true)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_Data_Unset_EndedByLength(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Padding", PartResult->GetValue());
					if(Continue == true)
					{
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						Size -= PartReader.GetConsumedLength();
					}
				}
			}
			// reading
			if(Continue == false)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_ID3_2_2_Frame(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Frame", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					Size -= PartReader.GetConsumedLength();
				}
			}
		}
		else if(MajorVersion == 0x03)
		{
			// reading
			if(Continue == true)
			{
				if(std::experimental::any_cast< bool >(Result->GetValue()->GetValue("TagHeader")->GetValue("Flags")->GetValue("ExtendedHeader")->GetAny()) == true)
				{
					throw Inspection::NotImplementedException("ID3 2.3 extended header");
				}
			}
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader, Size};
				auto PartResult{Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, Get_ID3_2_3_Frame)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Frames", PartResult->GetValue());
				for(auto PartValue : PartResult->GetValue()->GetValues())
				{
					PartValue->SetName("Frame");
				}
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				Size -= PartReader.GetConsumedLength();
			}
			// reading
			if(Continue == true)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_Data_Unset_EndedByLength(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Padding", PartResult->GetValue());
					if(Continue == true)
					{
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						Size -= PartReader.GetConsumedLength();
					}
				}
			}
			// reading
			if(Continue == false)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_ID3_2_3_Frame(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Frame", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					Size -= PartReader.GetConsumedLength();
				}
			}
		}
		else if(MajorVersion == 0x04)
		{
			// reading
			if(Continue == true)
			{
				if(std::experimental::any_cast< bool >(Result->GetValue()->GetValue("TagHeader")->GetValue("Flags")->GetValue("ExtendedHeader")->GetAny()) == true)
				{
					Inspection::Reader FieldReader{Reader, Size};
					auto FieldResult{Get_ID3_2_4_Tag_ExtendedHeader(FieldReader)};
					auto FieldValue{Result->GetValue()->AppendValue("ExtendedHeader", FieldResult->GetValue())};
					
					UpdateState(Continue, Reader, FieldResult, FieldReader);
					Size -= FieldReader.GetConsumedLength();
				}
			}
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader, Size};
				auto PartResult{Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, Get_ID3_2_4_Frame)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Frames", PartResult->GetValue());
				for(auto PartValue : PartResult->GetValue()->GetValues())
				{
					PartValue->SetName("Frame");
				}
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				Size -= PartReader.GetConsumedLength();
			}
			// reading
			if(Continue == true)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_Data_Unset_EndedByLength(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Padding", PartResult->GetValue());
					if(Continue == true)
					{
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						Size -= PartReader.GetConsumedLength();
					}
				}
			}
			// reading
			if(Continue == false)
			{
				if(Size > Inspection::Length{0, 0})
				{
					Inspection::Reader PartReader{Reader, Size};
					auto PartResult{Get_ID3_2_4_Frame(PartReader)};
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendValue("Frame", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					Size -= PartReader.GetConsumedLength();
				}
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "Unknown major version \"" + to_string_cast(MajorVersion) + "\".");
			Continue = false;
		}
		// verification
		if(Continue == true)
		{
			if(Size > Inspection::Length{0, 0})
			{
				Result->GetValue()->AddTag("error", "There are " + to_string_cast(Size) + " bytes and bits remaining.");
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_2_Tag_Header(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ASCII_String_AlphaNumeric_EndedByTemplateLength(Reader, "ID3")};
		auto FieldValue{Result->GetValue()->AppendValue("FileIdentifier", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("MajorVersion", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("RevisionNumber", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto MajorVersion{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("MajorVersion")->GetAny())};
		
		if(MajorVersion == 0x02)
		{
			auto FieldResult{Get_ID3_2_2_Tag_Header_Flags(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MajorVersion == 0x03)
		{
			auto FieldResult{Get_ID3_2_3_Tag_Header_Flags(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(MajorVersion == 0x04)
		{
			auto FieldResult{Get_ID3_2_4_Tag_Header_Flags(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("Flags", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else
		{
			Result->GetValue()->AddTag("error", "The major version of the tag (" + to_string_cast(MajorVersion) + ") cannot be handled!"s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Size", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_ReplayGainAdjustment(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("standard", "Hydrogenaudio ReplayGain"s);
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "ReplayGainAdjustment_NameCode"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("NameCode", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "ReplayGainAdjustment_OriginatorCode"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("OriginatorCode", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"ID3", "ReplayGainAdjustment_SignBit"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SignBit", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ID3_ReplayGainAdjustment_ReplayGainAdjustment(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ReplayGainAdjustment", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto SignBit{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("SignBit")->GetAny())};
		auto ReplayGainAdjustment{std::experimental::any_cast< float >(Result->GetValue()->GetValue("ReplayGainAdjustment")->GetTag("interpretation")->GetAny())};
		
		if(SignBit == 0x01)
		{
			ReplayGainAdjustment *= -1.0f;
		}
		Result->GetValue()->AddTag("interpretation", to_string_cast(ReplayGainAdjustment) + " dB");
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_ReplayGainAdjustment_ReplayGainAdjustment(Inspection::Reader & Reader)
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
		auto ReplayGainAdjustment{static_cast< float >(std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetAny()))};
		
		Result->GetValue()->AddTag("interpretation", ReplayGainAdjustment / 10.0f);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("7bit value"s);
	Result->GetValue()->AddTag("8bit field"s);
	Result->GetValue()->AddTag("synchsafe"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 8}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 8}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.Get1Bits() == 0x00)
		{
			std::uint8_t First{Reader.Get7Bits()};
			
			Result->GetValue()->SetAny(First);
		}
		else
		{
			Continue = false;
			Result->GetValue()->AddTag("error", "The unsigned integer should start with an unset bit."s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("28bit value"s);
	Result->GetValue()->AddTag("32bit field"s);
	Result->GetValue()->AddTag("synchsafe"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
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
					}
					else
					{
						Result->GetValue()->AddTag("error", "The fourth byte of the unsigned integer should start with an unset bit."s);
						Continue = false;
					}
				}
				else
				{
					Result->GetValue()->AddTag("error", "The third byte of the unsigned integer should start with an unset bit."s);
					Continue = false;
				}
			}
			else
			{
				Result->GetValue()->AddTag("error", "The second byte of the unsigned integer should start with an unset bit."s);
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The first byte of the unsigned integer should start with an unset bit."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ID3_UnsignedInteger_32Bit_SynchSafe_40Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("32bit value"s);
	Result->GetValue()->AddTag("40bit field"s);
	Result->GetValue()->AddTag("synchsafe"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 40}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 40}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
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
						}
						else
						{
							Result->GetValue()->AddTag("error", "The fifth byte of the unsigned integer should start with an unset bit."s);
							Continue = false;
						}
					}
					else
					{
						Result->GetValue()->AddTag("error", "The fourth byte of the unsigned integer should start with an unset bit."s);
						Continue = false;
					}
				}
				else
				{
					Result->GetValue()->AddTag("error", "The third byte of the unsigned integer should start with an unset bit."s);
					Continue = false;
				}
			}
			else
			{
				Result->GetValue()->AddTag("error", "The second byte of the unsigned integer should start with an unset bit."s);
				Continue = false;
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The first byte of the unsigned integer should start with an unset bit."s);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
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
		try
		{
			const Inspection::GUID & GUID{std::experimental::any_cast< const Inspection::GUID & >(Result->GetValue()->GetAny())};
			
			Result->GetValue()->AddTag("interpretation", Inspection::Get_GUID_Interpretation(GUID));
		}
		catch(Inspection::UnknownValueException & Exception)
		{
			Result->GetValue()->AddTag("interpretation", nullptr);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Track(Reader, {})};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = (Result->GetValue()->GetValue("Number")->HasTag("interpretation") == true) && (std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("Number")->GetTag("interpretation")->GetAny()) == "Lead-Out");
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{1, 0}};
		auto FieldResult{Get_Data_Unset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_4Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ADR", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// verification
	if(Continue == true)
	{
		Continue = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("ADR")->GetAny());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_Track_Control(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Control", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_8Bit(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Number", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Number{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Number")->GetAny())};
		
		if(Number == 0xaa)
		{
			Result->GetValue()->GetValue("Number")->AddTag("interpretation", "Lead-Out"s);
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{1, 0}};
		auto FieldResult{Get_Data_Unset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("Reserved", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_BigEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("StartAddress", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_BitSet_4Bit_MostSignificantBitFirst(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 4 > & Control{std::experimental::any_cast< const std::bitset< 4 > & >(Result->GetValue()->GetAny())};
		
		if(Control[1] == true)
		{
			if(Control[0] == true)
			{
				Continue = false;
				
				auto Value{Result->GetValue()->AppendValue("Reserved", true)};
				
				Value->AddTag("error", "The track type is \"Data\" so this bit must be off.");
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

std::unique_ptr< Inspection::Result > Inspection::Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FirstTrackNumber{std::experimental::any_cast< std::uint8_t >(Parameters.at("FirstTrackNumber"))};
		auto LastTrackNumber{std::experimental::any_cast< std::uint8_t >(Parameters.at("LastTrackNumber"))};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"IEC_60908_1999", "TableOfContents_Track"}}, {"ElementName", "Track"s}, {"NumberOfElements", static_cast< std::uint64_t >(LastTrackNumber - FirstTrackNumber + 1)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Tracks", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_IEC_60908_1999_TableOfContents_LeadOutTrack(Reader)};
		
		Result->GetValue()->AppendValue("LeadOutTrack", FieldResult->GetValue());
		UpdateState(Continue, FieldResult);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{3, 0}) + ".");
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
		Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
		
		const std::string & Code{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetAny())};
		
		try
		{
			Result->GetValue()->AddTag("interpretation", Get_LanguageName_From_ISO_639_2_1998_Code(Code));
		}
		catch(...)
		{
			Result->GetValue()->AddTag("error", "The code \"" + Code + "\" is unknown.");
			Result->GetValue()->AddTag("interpretation", nullptr);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_Character(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("character"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto Byte{Reader.Get8Bits()};
		
		Result->GetValue()->AppendValue("byte", Byte);
		if(Is_ISO_IEC_8859_1_1998_Character(Byte) == true)
		{
			Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Byte));
		}
		else
		{
			Result->GetValue()->AddTag("error", "The character is not an ISO/IEC 8859-1:1998 character."s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
			
			if(Is_ISO_IEC_8859_1_1998_Character(Character) == true)
			{
				NumberOfCharacters += 1;
				Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Character);
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
				Continue = false;
			}
		}
		if(NumberOfCharacters == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
				if(NumberOfCharacters == 0)
				{
					Result->GetValue()->AddTag("empty"s);
				}
				Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
				Result->GetValue()->AddTag("ended by termination"s);
				
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		auto EndedByTermination{false};
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto Byte{Reader.Get8Bits()};
			
			if(Byte == 0x00)
			{
				EndedByTermination = true;
				
				break;
			}
			else if(Is_ISO_IEC_8859_1_1998_Character(Byte) == true)
			{
				NumberOfCharacters += 1;
				Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(Byte);
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
				Continue = false;
			}
		}
		if(NumberOfCharacters == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
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
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
					Result->GetValue()->AddTag("ended by error"s);
					Result->GetValue()->AddTag("error", "After the first termination byte only terminations are allowed, but the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not."s);
					Continue = false;
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not an ISO/IEC 8859-1:1998 character or termination.");
				Continue = false;
			}
		}
		if(NumberOfCharacters > 0)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		else
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(NumberOfTerminations > 0)
		{
			Result->GetValue()->AddTag("ended by termination"s);
			if(Reader.IsAtEnd() == true)
			{
				Result->GetValue()->AddTag(to_string_cast(NumberOfTerminations) + " terminations until length");
			}
		}
		else
		{
			Result->GetValue()->AddTag("ended by length"s);
			Result->GetValue()->AddTag("error", "The string must be ended by at least one termination."s);
			Continue = false;
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
	Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
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
					Result->GetValue()->AddTag("ended by error"s);
					Result->GetValue()->AddTag("error", "After the first termination byte only terminations are allowed, but the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not."s);
					Continue = false;
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not an ISO/IEC 8859-1:1998 character or termination.");
				Continue = false;
			}
		}
		if(NumberOfCharacters > 0)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
		}
		else
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(NumberOfTerminations > 0)
		{
			Result->GetValue()->AddTag("ended by termination"s);
			if(Reader.IsAtEnd() == true)
			{
				Result->GetValue()->AddTag(to_string_cast(NumberOfTerminations) + " terminations until length");
			}
		}
		else
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == true)
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{2, 0}};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			
			Result->SetValue(FieldResult->GetValue());
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
		else
		{
			Result->GetValue()->AddTag("error", "At least " + to_string_cast(Inspection::Length{2, 0}) + " bytes and bits are necessary to read a byte order mark.");
			Continue = false;
		}
	}
	// verification
	if(Continue == true)
	{
		const std::vector< std::uint8_t > & Bytes{std::experimental::any_cast< const std::vector< std::uint8_t > & >(Result->GetValue()->GetAny())};
		
		if((Bytes[0] == 0xfe) && (Bytes[1] == 0xff))
		{
			Result->GetValue()->AddTag("interpretation", "BigEndian"s);
		}
		else if((Bytes[0] == 0xff) && (Bytes[1] == 0xfe))
		{
			Result->GetValue()->AddTag("interpretation", "LittleEndian"s);
		}
		else
		{
			Result->GetValue()->AddTag("error", "The byte combination is not a valid byte order mark."s);
			Result->GetValue()->AddTag("interpretation", nullptr);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		auto CodePoint{static_cast< std::uint32_t >((static_cast< std::uint32_t >(First) << 8) | static_cast< std::uint32_t >(Second))};
		
		Result->GetValue()->AppendValue("codepoint", CodePoint);
		if(Continue == true)
		{
			Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint));
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		auto CodePoint{static_cast< std::uint32_t >((static_cast< std::uint32_t >(Second) << 8) | static_cast< std::uint32_t >(First))};
		
		Result->GetValue()->AppendValue("codepoint", CodePoint);
		if(Continue == true)
		{
			Result->GetValue()->SetAny(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint));
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint32_t >((static_cast< std::uint32_t >(First) << 8) | static_cast< std::uint32_t >(Second)));
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint32_t >((static_cast< std::uint32_t >(Second) << 8) | static_cast< std::uint32_t >(First)));
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AddTag("empty"s);
					}
					Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
					Result->GetValue()->AddTag("ended by termination"s);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AddTag("empty"s);
					}
					Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
					Result->GetValue()->AddTag("ended by termination"s);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ByteOrderMark", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("ByteOrderMark")->GetTag("interpretation")->GetAny())};
		
		if(ByteOrderMark == "BigEndian")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UCS-2"s);
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("ByteOrderMark", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("ByteOrderMark")->GetTag("interpretation")->GetAny())};
		
		if(ByteOrderMark == "BigEndian")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Reader)};
			auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
			
			UpdateState(Continue, FieldResult);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{1, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{1, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-8"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			NumberOfCodePoints += 1;
			Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny()));
			if(Continue == false)
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points"s);
		if(Reader.IsAtEnd() == true)
		{
			Result->GetValue()->AddTag("ended by length"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-8"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AddTag("empty"s);
					}
					Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
					Result->GetValue()->AddTag("ended by termination"s);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
				Continue = false;
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-8"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength().GetBits() != 0)
		{
			Result->GetValue()->AddTag("error", "The available length must be an integer multiple of bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
	
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{2, 0}};
		auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
		
		Result->SetValue(FieldResult->GetValue());
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// verification
	if(Continue == true)
	{
		const std::vector< std::uint8_t > & Bytes{std::experimental::any_cast< const std::vector< std::uint8_t > & >(Result->GetValue()->GetAny())};
		
		if((Bytes[0] == 0xfe) && (Bytes[1] == 0xff))
		{
			Result->GetValue()->AddTag("interpretation", "BigEndian"s);
		}
		else if((Bytes[0] == 0xff) && (Bytes[1] == 0xfe))
		{
			Result->GetValue()->AddTag("interpretation", "LittleEndian"s);
		}
		else
		{
			Result->GetValue()->AddTag("error", "The byte combination is not a valid byte order mark."s);
			Result->GetValue()->AddTag("interpretation", nullptr);
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ByteOrderMark", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("ByteOrderMark")->GetTag("interpretation")->GetAny())};
		
		if(ByteOrderMark == "BigEndian")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(PartReader)};
		
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("String", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(PartReader)};
		
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("String", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ByteOrderMark", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto ByteOrderMark{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("ByteOrderMark")->GetTag("interpretation")->GetAny())};
		
		if(ByteOrderMark == "BigEndian")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(PartReader)};
		
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("String", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(ByteOrderMark == "LittleEndian")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(PartReader, {})};
		
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("String", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FirstCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader)};
		
		UpdateState(Continue, FirstCodeUnitResult);
		if(Continue == true)
		{
			auto FirstCodeUnit{std::experimental::any_cast< std::uint16_t >(FirstCodeUnitResult->GetValue()->GetAny())};
			
			if((FirstCodeUnit < 0xd800) || (FirstCodeUnit >= 0xe000))
			{
				std::uint32_t Value{FirstCodeUnit};
				
				Result->GetValue()->SetAny(Value);
			}
			else if((FirstCodeUnit >= 0xd800) && (FirstCodeUnit < 0xdc00))
			{
				auto SecondCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader)};
				
				UpdateState(Continue, SecondCodeUnitResult);
				if(Continue == true)
				{
					auto SecondCodeUnit{std::experimental::any_cast< std::uint16_t >(SecondCodeUnitResult->GetValue()->GetAny())};
					
					if((SecondCodeUnit >= 0xdc00) && (SecondCodeUnit < 0xe000))
					{
						std::uint32_t Value{(static_cast< std::uint32_t >(FirstCodeUnit - 0xd800) << 10) | static_cast< std::uint32_t >(SecondCodeUnit - 0xdc00)};
						
						Result->GetValue()->SetAny(Value);
					}
					else
					{
						Continue = false;
					}
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint16_t >(static_cast< std::uint16_t >(First << 8) | static_cast< std::uint16_t >(Second)));
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("big endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// verification
	if(Continue == true)
	{
		if((Reader.GetRemainingLength().GetBits() != 0) && (Reader.GetRemainingLength().GetBytes() % 2 != 0))
		{
			Result->GetValue()->AddTag("error", "The available length must be a multiple of two bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader)};
			
			Continue = PartResult->GetSuccess();
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(PartResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AddTag("emtpy"s);
					}
					Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
					Result->GetValue()->AddTag("ended by termination"s);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("big endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// verification
	if(Continue == true)
	{
		if((Reader.GetRemainingLength().GetBits() != 0) && (Reader.GetRemainingLength().GetBytes() % 2 != 0))
		{
			Result->GetValue()->AddTag("error", "The available length must be a multiple of two bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FirstCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader)};
		
		UpdateState(Continue, FirstCodeUnitResult);
		if(Continue == true)
		{
			auto FirstCodeUnit{std::experimental::any_cast< std::uint16_t >(FirstCodeUnitResult->GetValue()->GetAny())};
			
			if((FirstCodeUnit < 0xd800) || (FirstCodeUnit >= 0xe000))
			{
				std::uint32_t Value{FirstCodeUnit};
				
				Result->GetValue()->SetAny(Value);
			}
			else if((FirstCodeUnit >= 0xd800) && (FirstCodeUnit < 0xdc00))
			{
				auto SecondCodeUnitResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader)};
				
				UpdateState(Continue, SecondCodeUnitResult);
				if(Continue == true)
				{
					auto SecondCodeUnit{std::experimental::any_cast< std::uint16_t >(SecondCodeUnitResult->GetValue()->GetAny())};
					
					if((SecondCodeUnit >= 0xdc00) && (SecondCodeUnit < 0xe000))
					{
						std::uint32_t Value{(static_cast< std::uint32_t >(FirstCodeUnit - 0xd800) << 10) | static_cast< std::uint32_t >(SecondCodeUnit - 0xdc00)};
						
						Result->GetValue()->SetAny(Value);
					}
					else
					{
						Continue = false;
					}
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

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{2, 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{2, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto First{Reader.Get8Bits()};
		auto Second{Reader.Get8Bits()};
		
		Result->GetValue()->SetAny(static_cast< std::uint16_t >(static_cast< std::uint16_t >(First) | static_cast< std::uint16_t >(Second << 8)));
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// verification
	if(Continue == true)
	{
		if((Reader.GetRemainingLength().GetBits() != 0) && (Reader.GetRemainingLength().GetBytes() % 2 != 0))
		{
			Result->GetValue()->AddTag("error", "The available length must be a multiple of two bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader)};
			
			Continue = PartResult->GetSuccess();
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(PartResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					if(NumberOfCodePoints == 0)
					{
						Result->GetValue()->AddTag("emtpy"s);
					}
					Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
					Result->GetValue()->AddTag("ended by termination"s);
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
			}
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// verification
	if(Continue == true)
	{
		if((Reader.GetRemainingLength().GetBits() != 0) && (Reader.GetRemainingLength().GetBytes() % 2 != 0))
		{
			Result->GetValue()->AddTag("error", "The available length must be a multiple of two bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					if(Reader.HasRemaining() == true)
					{
						Result->GetValue()->AddTag("error", "The termination must be the last code point in the availble length."s);
						Continue = false;
					}
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
				Continue = false;
			}
		}
		if(EndedByTermination == false)
		{
			Result->GetValue()->AddTag("error", "The string must be ended by a termination."s);
			Continue = false;
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// verification
	if(Continue == true)
	{
		if((Reader.GetRemainingLength().GetBits() != 0) && (Reader.GetRemainingLength().GetBytes() % 2 != 0))
		{
			Result->GetValue()->AddTag("error", "The available length must be a multiple of two bytes, without additional bits."s);
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		auto EndedByTermination{false};
		auto NumberOfCodePoints{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					
					break;
				}
				else
				{
					NumberOfCodePoints += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
				Continue = false;
			}
		}
		if(NumberOfCodePoints == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
	Result->GetValue()->AddTag("encoding", "UTF-16"s);
	Result->GetValue()->AddTag("little endian"s);
	Result->GetValue()->AddTag("without byte order mark"s);
	// no verification, because not ended by length
	// reading
	if(Continue == true)
	{
		auto NumberOfCodePoints{std::experimental::any_cast< std::uint64_t >(Parameters.at("NumberOfCodePoints"))};
		auto EndedByTermination{false};
		auto CodePointIndex{0ul};
		std::stringstream Value;
		
		while((Continue == true) && (Reader.HasRemaining() == true) && (CodePointIndex < NumberOfCodePoints))
		{
			auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader)};
			
			UpdateState(Continue, FieldResult);
			if(Continue == true)
			{
				auto CodePoint{std::experimental::any_cast< std::uint32_t >(FieldResult->GetValue()->GetAny())};
				
				if(CodePoint == 0x00000000)
				{
					EndedByTermination = true;
					if(CodePointIndex + 1 != NumberOfCodePoints)
					{
						Result->GetValue()->AddTag("error", "With the termination code point, the string must contain exactly " + to_string_cast(NumberOfCodePoints) + " code points."s);
						Continue = false;
					}
					
					break;
				}
				else
				{
					CodePointIndex += 1;
					Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
				}
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(CodePointIndex + 1) + "th code point is not a valid UTF-16 code point.");
				Continue = false;
			}
		}
		if(EndedByTermination == false)
		{
			Result->GetValue()->AddTag("error", "The string must be ended by a termination."s);
			Continue = false;
		}
		if(CodePointIndex == 0)
		{
			Result->GetValue()->AddTag("empty"s);
		}
		if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag(to_string_cast(CodePointIndex) + " code points + termination");
		}
		else
		{
			Result->GetValue()->AddTag(to_string_cast(CodePointIndex) + " code points");
		}
		if(Reader.IsAtEnd() == true)
		{
			if(EndedByTermination == true)
			{
				Result->GetValue()->AddTag("ended by termination and length"s);
			}
			else
			{
				Result->GetValue()->AddTag("ended by length"s);
			}
		}
		else if(EndedByTermination == true)
		{
			Result->GetValue()->AddTag("ended by termination"s);
		}
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("floating point"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("standard", "ISO/IEC/IEEE-60559:2011 binary32"s);
	// verification
	if(Continue == true)
	{
		if(Reader.GetRemainingLength() < Inspection::Length{4, 0})
		{
			Result->GetValue()->AddTag("error", "The available length needs to be exactly " + to_string_cast(Inspection::Length{4, 0}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint8_t Data[4];
		
		Data[0] = Reader.Get8Bits();
		Data[1] = Reader.Get8Bits();
		Data[2] = Reader.Get8Bits();
		Data[3] = Reader.Get8Bits();
		Result->GetValue()->SetAny(*reinterpret_cast< const float * const >(Data));
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_Frame(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_MPEG_1_FrameHeader(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Header", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// reading
	if(Continue == true)
	{
		auto ProtectionBit{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("ProtectionBit")->GetAny())};
		
		if(ProtectionBit == 0x00)
		{
			Inspection::Reader FieldReader{Reader, Inspection::Length{0, 16}};
			auto FieldResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(FieldReader)};
			auto FieldValue{Result->GetValue()->AppendValue("ErrorCheck", FieldResult->GetValue())};
			
			UpdateState(Continue, Reader, FieldResult, FieldReader);
		}
	}
	// reading
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("LayerDescription")->GetAny())};
		auto BitRate{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("BitRateIndex")->GetTag("value")->GetAny())};
		auto SamplingFrequency{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("SamplingFrequency")->GetTag("value")->GetAny())};
		auto PaddingBit{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("Header")->GetValue("PaddingBit")->GetAny())};
		auto FrameLength{0ul};
		
		if(LayerDescription == 0x03)
		{
			FrameLength = (12 * BitRate / SamplingFrequency + PaddingBit) * 4;
		}
		else if((LayerDescription == 0x01) || (LayerDescription == 0x02))
		{
			FrameLength = 144 * BitRate / SamplingFrequency + PaddingBit;
		}
		
		Inspection::Reader FieldReader{Reader, Inspection::Length{FrameLength, 0} - Reader.GetConsumedLength()};
		auto FieldResult{Get_Data_SetOrUnset_EndedByLength(FieldReader)};
		auto FieldValue{Result->GetValue()->AppendValue("AudioData", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{0, 12}};
		auto PartResult{Get_Data_Set_EndedByLength(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FrameSync", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_AudioVersionID"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("AudioVersionID", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_LayerDescription"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("LayerDescription", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_ProtectionBit"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ProtectionBit", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_MPEG_1_FrameHeader_BitRateIndex(PartReader, {{"LayerDescription", Result->GetValue()->GetValue("LayerDescription")->GetAny()}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("BitRateIndex", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_SamplingFrequency"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("SamplingFrequency", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_PaddingBit"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("PaddingBit", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_1Bit(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("PrivateBit", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_MPEG_1_FrameHeader_Mode(PartReader, {{"LayerDescription", Result->GetValue()->GetValue("LayerDescription")->GetAny()}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Mode", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_MPEG_1_FrameHeader_ModeExtension(PartReader, {{"LayerDescription", Result->GetValue()->GetValue("LayerDescription")->GetAny()}, {"Mode", Result->GetValue()->GetValue("Mode")->GetAny()}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("ModeExtension", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_Copyright"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Copyright", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_OriginalHome"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Original/Home", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"MPEG", "1", "FrameHeader_Emphasis"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Emphasis", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
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
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Parameters.at("LayerDescription"))};
		auto BitRateIndex{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny())};
		
		if(LayerDescription == 0x03)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->AddTag("interpretation", "free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->AddTag("value", 32000u);
				Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->AddTag("value", 64000u);
				Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->AddTag("value", 96000u);
				Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->AddTag("value", 128000u);
				Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->AddTag("value", 160000u);
				Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->AddTag("value", 192000u);
				Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->AddTag("value", 224000u);
				Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->AddTag("value", 256000u);
				Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->AddTag("value", 288000u);
				Result->GetValue()->AddTag("interpretation", "288 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->AddTag("value", 320000u);
				Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->AddTag("value", 352000u);
				Result->GetValue()->AddTag("interpretation", "352 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->AddTag("value", 384000u);
				Result->GetValue()->AddTag("interpretation", "384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->AddTag("value", 416000u);
				Result->GetValue()->AddTag("interpretation", "416 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->AddTag("value", 448000u);
				Result->GetValue()->AddTag("interpretation", "448 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
				Result->GetValue()->AddTag("interpretation", nullptr);
				Continue = false;
			}
		}
		else if(LayerDescription == 0x02)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->AddTag("interpretation", "free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->AddTag("value", 32000u);
				Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->AddTag("value", 48000u);
				Result->GetValue()->AddTag("interpretation", "48 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->AddTag("value", 56000u);
				Result->GetValue()->AddTag("interpretation", "56 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->AddTag("value", 64000u);
				Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->AddTag("value", 80000u);
				Result->GetValue()->AddTag("interpretation", "80 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->AddTag("value", 96000u);
				Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->AddTag("value", 112000u);
				Result->GetValue()->AddTag("interpretation", "112 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->AddTag("value", 128000u);
				Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->AddTag("value", 160000u);
				Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->AddTag("value", 192000u);
				Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->AddTag("value", 224000u);
				Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->AddTag("value", 256000u);
				Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->AddTag("value", 320000u);
				Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->AddTag("value", 384000u);
				Result->GetValue()->AddTag("interpretation", "384 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
				Result->GetValue()->AddTag("interpretation", nullptr);
				Continue = false;
			}
		}
		else if(LayerDescription == 0x01)
		{
			if(BitRateIndex == 0x00)
			{
				Result->GetValue()->AddTag("interpretation", "free format"s);
			}
			else if(BitRateIndex == 0x01)
			{
				Result->GetValue()->AddTag("value", 32000u);
				Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
			}
			else if(BitRateIndex == 0x02)
			{
				Result->GetValue()->AddTag("value", 40000u);
				Result->GetValue()->AddTag("interpretation", "40 kbit/s"s);
			}
			else if(BitRateIndex == 0x03)
			{
				Result->GetValue()->AddTag("value", 48000u);
				Result->GetValue()->AddTag("interpretation", "48 kbit/s"s);
			}
			else if(BitRateIndex == 0x04)
			{
				Result->GetValue()->AddTag("value", 56000u);
				Result->GetValue()->AddTag("interpretation", "56 kbit/s"s);
			}
			else if(BitRateIndex == 0x05)
			{
				Result->GetValue()->AddTag("value", 64000u);
				Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
			}
			else if(BitRateIndex == 0x06)
			{
				Result->GetValue()->AddTag("value", 80000u);
				Result->GetValue()->AddTag("interpretation", "80 kbit/s"s);
			}
			else if(BitRateIndex == 0x07)
			{
				Result->GetValue()->AddTag("value", 96000u);
				Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
			}
			else if(BitRateIndex == 0x08)
			{
				Result->GetValue()->AddTag("value", 112000u);
				Result->GetValue()->AddTag("interpretation", "112 kbit/s"s);
			}
			else if(BitRateIndex == 0x09)
			{
				Result->GetValue()->AddTag("value", 128000u);
				Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
			}
			else if(BitRateIndex == 0x0a)
			{
				Result->GetValue()->AddTag("value", 160000u);
				Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
			}
			else if(BitRateIndex == 0x0b)
			{
				Result->GetValue()->AddTag("value", 192000u);
				Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
			}
			else if(BitRateIndex == 0x0c)
			{
				Result->GetValue()->AddTag("value", 224000u);
				Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
			}
			else if(BitRateIndex == 0x0d)
			{
				Result->GetValue()->AddTag("value", 256000u);
				Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
			}
			else if(BitRateIndex == 0x0e)
			{
				Result->GetValue()->AddTag("value", 320000u);
				Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
			}
			else if(BitRateIndex == 0x0f)
			{
				Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
				Result->GetValue()->AddTag("interpretation", nullptr);
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

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_Mode(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_2Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Parameters.at("LayerDescription"))};
		auto Mode{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny())};
		
		if(Mode == 0x00)
		{
			Result->GetValue()->AddTag("interpretation", "stereo"s);
		}
		else if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				Result->GetValue()->AddTag("interpretation", "joint stereo (intensity_stereo)"s);
			}
			else if(LayerDescription == 0x01)
			{
				Result->GetValue()->AddTag("interpretation", "joint stereo (intensity_stereo and/or ms_stereo)"s);
			}
			else
			{
				// LayerDescription is a 2-bit value. Value 0 is reserved, 1, 2 and 3 are handled above. Otherwise the program is corrupt.
				assert(false);
			}
		}
		else if(Mode == 0x02)
		{
			Result->GetValue()->AddTag("interpretation", "dual_channel"s);
		}
		else if(Mode == 0x03)
		{
			Result->GetValue()->AddTag("interpretation", "single_channel"s);
		}
		else
		{
			// every 2-bit value is either 0, 1, 2 or 3 ... otherwise the program is corrupt.
			assert(false);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_2Bit(Reader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
	}
	// interpretation
	if(Continue == true)
	{
		auto Mode{std::experimental::any_cast< std::uint8_t >(Parameters.at("Mode"))};
		auto LayerDescription{std::experimental::any_cast< std::uint8_t >(Parameters.at("LayerDescription"))};
		auto ModeExtension{std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetAny())};
		
		if(Mode == 0x01)
		{
			if((LayerDescription == 0x03) || (LayerDescription == 0x02))
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->AddTag("subbands 4-31 in intensity_stereo, bound==4"s);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->AddTag("subbands 8-31 in intensity_stereo, bound==8"s);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->AddTag("subbands 12-31 in intensity_stereo, bound==12"s);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->AddTag("subbands 16-31 in intensity_stereo, bound==16"s);
				}
				else
				{
					// every 2-bit value is either 0, 1, 2 or 3 ... otherwise the program is corrupt.
					assert(false);
				}
			}
			else if(LayerDescription == 0x01)
			{
				if(ModeExtension == 0x00)
				{
					Result->GetValue()->AddTag("ms_stereo", "off"s);
					Result->GetValue()->AddTag("intensity_stereo", "off"s);
				}
				else if(ModeExtension == 0x01)
				{
					Result->GetValue()->AddTag("ms_stereo", "off"s);
					Result->GetValue()->AddTag("intensity_stereo", "on"s);
				}
				else if(ModeExtension == 0x02)
				{
					Result->GetValue()->AddTag("ms_stereo", "on"s);
					Result->GetValue()->AddTag("intensity_stereo", "off"s);
				}
				else if(ModeExtension == 0x03)
				{
					Result->GetValue()->AddTag("ms_stereo", "on"s);
					Result->GetValue()->AddTag("intensity_stereo", "on"s);
				}
				else
				{
					// every 2-bit value is either 0, 1, 2 or 3 ... otherwise the program is corrupt.
					assert(false);
				}
			}
			else
			{
				// LayerDescription is a 2-bit value. Value 0 is reserved, 1, 2 and 3 are handled above. Otherwise the program is corrupt.
				assert(false);
			}
		}
		else
		{
			Result->GetValue()->AddTag("<ignored>"s);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_MPEG_1_Stream(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, Get_MPEG_1_Frame)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("MPEGFrames", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("MPEGFrame");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_RIFF_Chunk(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkHeader"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Header", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto PartStart{Reader.GetConsumedLength()};
		auto ClaimedSize{Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Header")->GetValue("Size")->GetAny()), 0}};
		
		if(Reader.Has(ClaimedSize) == true)
		{
			auto ChunkIdentifier{std::experimental::any_cast< std::string >(Result->GetValue()->GetValue("Header")->GetValue("Identifier")->GetAny())};
			
			if(ChunkIdentifier == "RIFF")
			{
				Inspection::Reader PartReader{Reader, ClaimedSize};
				auto PartResult{Get_RIFF_RIFF_ChunkData(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else if(ChunkIdentifier == "inst")
			{
				Inspection::Reader PartReader{Reader, ClaimedSize};
				auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkData", "inst"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else if(ChunkIdentifier == "fact")
			{
				Inspection::Reader PartReader{Reader, ClaimedSize};
				auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkData", "fact"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else if(ChunkIdentifier == "fmt ")
			{
				Inspection::Reader PartReader{Reader, ClaimedSize};
				auto PartResult{Get_RIFF_ChunkData_fmt_(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			else
			{
				Inspection::Reader PartReader{Reader, ClaimedSize};
				auto PartResult{Get_Data_SetOrUnset_EndedByLength(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Data", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			
			auto HandledSize{Reader.GetConsumedLength() - PartStart};
			
			if(HandledSize < ClaimedSize)
			{
				auto OutOfAlignmentBits{Reader.GetPositionInBuffer().GetTotalBits() % 16};
				
				if(HandledSize + Inspection::Length{0, OutOfAlignmentBits} == ClaimedSize)
				{
					Result->GetValue()->AddTag("error", "The chunk data size is claimed larger than the actually handled size, because the padding was erroneously included in the chunk data size."s);
				}
				else
				{
					Result->GetValue()->AddTag("error", "The chunk data size is claimed larger than the actually handled size."s);
				}
				Result->GetValue()->AddTag("claimed size", ClaimedSize);
				Result->GetValue()->AddTag("handled size", HandledSize);
			}
		}
		else
		{
			Result->GetValue()->AddTag("error", "The RIFF chunk claims to have a length of " + to_string_cast(ClaimedSize) + " bytes and bits but only " + to_string_cast(Reader.GetRemainingLength()) + " bytes and bits are available.");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		if(Reader.HasRemaining() == true)
		{
			auto OutOfAlignmentBits{Reader.GetPositionInBuffer().GetTotalBits() % 16};
			
			if(OutOfAlignmentBits > 0)
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Get_Data_Unset_Until16BitAlignment(PartReader)};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendValue("Padding", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_RIFF_ChunkData_fmt_(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkData", "fmt__CommonFields"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		const std::string & FormatTag{std::experimental::any_cast< const std::string & >(Result->GetValue()->GetValue("FormatTag")->GetTag("constant name")->GetAny())};
		
		if(FormatTag == "WAVE_FORMAT_PCM")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkData", "fmt__FormatSpecificFields_PCM"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else if(FormatTag == "WAVE_FORMAT_EXTENSIBLE")
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{g_GetterRepository.Get({"RIFF", "ChunkData", "fmt__FormatSpecificFields_Extensible"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValues(PartResult->GetValue()->GetValues());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader)};
			
			Continue = PartResult->GetSuccess();
			Result->GetValue()->AppendValue("Rest", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
			Result->GetValue()->AddTag("error", "Unknown format tag " + FormatTag + ".");
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_BitSet_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		const std::bitset< 32 > & ChannelMask{std::experimental::any_cast< const std::bitset< 32 > & >(Result->GetValue()->GetAny())};
		
		if(ChannelMask[0] == true)
		{
			Result->GetValue()->AppendValue("[0]", "SPEAKER_FRONT_LEFT"s);
		}
		if(ChannelMask[1] == true)
		{
			Result->GetValue()->AppendValue("[1]", "SPEAKER_FRONT_RIGHT"s);
		}
		if(ChannelMask[2] == true)
		{
			Result->GetValue()->AppendValue("[2]", "SPEAKER_FRONT_CENTER"s);
		}
		if(ChannelMask[3] == true)
		{
			Result->GetValue()->AppendValue("[3]", "SPEAKER_LOW_FREQUENCY"s);
		}
		if(ChannelMask[4] == true)
		{
			Result->GetValue()->AppendValue("[4]", "SPEAKER_BACK_LEFT"s);
		}
		if(ChannelMask[5] == true)
		{
			Result->GetValue()->AppendValue("[5]", "SPEAKER_BACK_RIGHT"s);
		}
		if(ChannelMask[6] == true)
		{
			Result->GetValue()->AppendValue("[6]", "SPEAKER_FRONT_LEFT_OF_CENTER"s);
		}
		if(ChannelMask[7] == true)
		{
			Result->GetValue()->AppendValue("[7]", "SPEAKER_FRONT_RIGHT_OF_CENTER"s);
		}
		if(ChannelMask[8] == true)
		{
			Result->GetValue()->AppendValue("[8]", "SPEAKER_BACK_CENTER"s);
		}
		if(ChannelMask[9] == true)
		{
			Result->GetValue()->AppendValue("[9]", "SPEAKER_SIDE_LEFT"s);
		}
		if(ChannelMask[10] == true)
		{
			Result->GetValue()->AppendValue("[10]", "SPEAKER_SIDE_RIGHT"s);
		}
		if(ChannelMask[11] == true)
		{
			Result->GetValue()->AppendValue("[11]", "SPEAKER_TOP_CENTER"s);
		}
		if(ChannelMask[12] == true)
		{
			Result->GetValue()->AppendValue("[12]", "SPEAKER_TOP_FRONT_LEFT"s);
		}
		if(ChannelMask[13] == true)
		{
			Result->GetValue()->AppendValue("[13]", "SPEAKER_TOP_FRONT_CENTER"s);
		}
		if(ChannelMask[14] == true)
		{
			Result->GetValue()->AppendValue("[14]", "SPEAKER_TOP_FRONT_RIGHT"s);
		}
		if(ChannelMask[15] == true)
		{
			Result->GetValue()->AppendValue("[15]", "SPEAKER_TOP_BACK_LEFT"s);
		}
		if(ChannelMask[16] == true)
		{
			Result->GetValue()->AppendValue("[16]", "SPEAKER_TOP_BACK_CENTER"s);
		}
		if(ChannelMask[17] == true)
		{
			Result->GetValue()->AppendValue("[17]", "SPEAKER_TOP_BACK_RIGHT"s);
		}
		for(auto BitIndex = 18; BitIndex < 31; ++BitIndex)
		{
			Continue &= !ChannelMask[BitIndex];
		}
		if(ChannelMask[31] == true)
		{
			Result->GetValue()->AppendValue("[31]", "SPEAKER_ALL"s);
		}
		/// @todo there are the following bit combinations which have special names
		//~ #define SPEAKER_MONO             (SPEAKER_FRONT_CENTER)
		//~ #define SPEAKER_STEREO           (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
		//~ #define SPEAKER_2POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
		//~ #define SPEAKER_SURROUND         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
		//~ #define SPEAKER_QUAD             (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_4POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_5POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
		//~ #define SPEAKER_7POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
		//~ #define SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
		//~ #define SPEAKER_7POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_GUID_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// interpretation
	if(Continue == true)
	{
		auto GUID{std::experimental::any_cast< Inspection::GUID >(Result->GetValue()->GetAny())};
		
		if(GUID == g_KSDATAFORMAT_SUBTYPE_PCM)
		{
			Result->GetValue()->AddTag("interpretation", "KSDATAFORMAT_SUBTYPE_PCM"s);
		}
		else
		{
			Result->GetValue()->AddTag("interpretation", nullptr);
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_RIFF_RIFF_ChunkData(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{4, 0}};
		auto PartResult{Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("FormType", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByLength(PartReader, Get_RIFF_Chunk)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Chunks", PartResult->GetValue());
		for(auto PartValue : PartResult->GetValue()->GetValues())
		{
			PartValue->SetName("Chunk");
		}
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	auto Bits{std::experimental::any_cast< std::uint8_t >(Parameters.at("Bits"))};
	
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
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("1bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 1}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Reader.Get1Bits() << 7) >> 7)};
		
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_5Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("5bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 5}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 5}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(static_cast< std::int8_t >(Reader.Get5Bits() << 3) >> 3)};
		
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("8bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 8}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 8}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int8_t Value{static_cast< std::int8_t >(Reader.Get8Bits())};
		
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_12Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("12bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 12}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 12}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int16_t Value{0};
		
		Value |= static_cast< std::int16_t >(static_cast< std::int16_t >(Reader.Get4Bits() << 12) >> 4);
		Value |= static_cast< std::int16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("signed"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::int32_t Value{0l};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 24;
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_SignedInteger_32Bit_RiceEncoded(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
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
		auto Rice{std::experimental::any_cast< std::uint8_t >(Parameters.at("Rice"))};
		Inspection::Reader FieldReader{Reader};
		auto FieldResult{Get_UnsignedInteger_BigEndian(FieldReader, {{"Bits", Rice}})};
		auto FieldValue{Result->GetValue()->AppendValue("LeastSignificantBits", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// interpretation
	if(Continue == true)
	{
		auto MostSignificantBits{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("MostSignificantBits")->GetAny())};
		std::uint32_t LeastSignificantBits;
		
		if(Result->GetValue()->GetValue("LeastSignificantBits")->GetAny().type() == typeid(std::uint8_t))
		{
			LeastSignificantBits = std::experimental::any_cast< std::uint8_t >(Result->GetValue()->GetValue("LeastSignificantBits")->GetAny());
		}
		else if(Result->GetValue()->GetValue("LeastSignificantBits")->GetAny().type() == typeid(std::uint16_t))
		{
			LeastSignificantBits = std::experimental::any_cast< std::uint16_t >(Result->GetValue()->GetValue("LeastSignificantBits")->GetAny());
		}
		
		auto Rice{std::experimental::any_cast< std::uint8_t >(Parameters.at("Rice"))};
		auto Value{MostSignificantBits << Rice | LeastSignificantBits};
		
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
	return Get_Array_EndedByNumberOfElements(Reader, {{"Getter", std::vector< std::string >{"Number", "Integer", "Signed", "BigEndian"}}, {"ElementParameters", std::unordered_map< std::string, std::experimental::any >{{"Bits", Bits}}}, {"NumberOfElements", NumberOfElements}});
}

std::unique_ptr< Inspection::Result > Inspection::Get_String_ASCII_Alphabetic_ByTemplate(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto & Template{std::experimental::any_cast< const std::string & >(Parameters.at("Template"))};
	auto Continue{true};
	
	Result->GetValue()->AddTag("string"s);
	Result->GetValue()->AddTag("character set", "ASCII"s);
	Result->GetValue()->AddTag("encoding", "ASCII"s);
	Result->GetValue()->AddTag("alphabetic"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{Template.size(), 0}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length must be at least " + to_string_cast(Inspection::Length{Template.size(), 0}) + ", the length of the template string.");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::stringstream Value;
		auto NumberOfCharacters{0ul};
		
		for(auto TemplateCharacter : Template)
		{
			auto Character{Reader.Get8Bits()};
			
			if((Is_ASCII_Character_Alphabetic(Character) == true) && (TemplateCharacter == Character))
			{
				NumberOfCharacters += 1;
				Value << Character;
			}
			else
			{
				Result->GetValue()->AddTag("ended by error"s);
				Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphabetic ASCII character.");
				Continue = false;
			}
		}
		if(Continue == true)
		{
			Result->GetValue()->AddTag("ended by template"s);
		}
		Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters"s);
		Result->GetValue()->SetAny(Value.str());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	auto Bits{std::experimental::any_cast< std::uint8_t >(Parameters.at("Bits"))};
	
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
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("0bit"s);
	Result->GetValue()->SetAny(Reader.Get0Bits());
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_1Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("1bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 1}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 1}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get1Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_2Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("2bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 2}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 2}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get2Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_3Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("3bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 3}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 3}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get3Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_4Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("4bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 4}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 4}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get4Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_5Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("5bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 5}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 5}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get5Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_6Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("6bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 6}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 6}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get6Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_7Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("7bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 7}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 7}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get7Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("8bit"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 8}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 8}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Result->GetValue()->SetAny(Reader.Get8Bits());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	std::uint8_t Value{0ul};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("alternative unary"s);
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
				Result->GetValue()->AddTag(to_string_cast(Value + 1) + "bit"s);
				
				break;
			}
		}
		else
		{
			Result->SetSuccess(true);
			Result->GetValue()->AddTag(to_string_cast(Value) + "bit"s);
			Result->GetValue()->AddTag("ended by boundary"s);
			
			break;
		}
	}
	if(Result->GetSuccess() == true)
	{
		Result->GetValue()->SetAny(Value);
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("9bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 9}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 9}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get1Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_10Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("10bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 10}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 10}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get2Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_11Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("11bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 11}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 11}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get3Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_12Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("12bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 12}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 12}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get4Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_13Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("13bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 13}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 13}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get5Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_14Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("14bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 14}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 14}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get6Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_15Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("15bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 15}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 15}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint16_t Value{0ul};
		
		Value |= static_cast< std::uint16_t >(Reader.Get7Bits()) << 8;
		Value |= static_cast< std::uint16_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("16bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
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
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("16bit"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 16}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 16}) + ".");
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_17Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("17bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 17}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 17}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get1Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_20Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("20bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 20}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 20}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get4Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_24Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("24bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 24}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 24}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0ul};
		
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint32_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
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
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("alternative unary"s);
	// reading
	if(Continue == true)
	{
		std::uint32_t Value{0};
		
		while(Continue == true)
		{
			if(Reader.HasRemaining() == true)
			{
				auto Bit{Reader.Get1Bits()};
				
				if(Bit == 0x00)
				{
					Value += 1;
				}
				else
				{
					Result->GetValue()->SetAny(Value);
					AppendLength(Result->GetValue(), Reader.GetConsumedLength());
					
					break;
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

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_32Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
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
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("32bit"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 32}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 32}) + ".");
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("36bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 36}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 36}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		std::uint64_t Value{0ull};
		
		Value |= static_cast< std::uint64_t >(Reader.Get4Bits()) << 32;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 24;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 16;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits()) << 8;
		Value |= static_cast< std::uint64_t >(Reader.Get8Bits());
		Result->GetValue()->SetAny(Value);
	}
	// finalization
	Result->SetSuccess(Continue);
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
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("64bit"s);
	Result->GetValue()->AddTag("big endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 64}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 64}) + ".");
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->AddTag("integer"s);
	Result->GetValue()->AddTag("unsigned"s);
	Result->GetValue()->AddTag("64bit"s);
	Result->GetValue()->AddTag("little endian"s);
	// verification
	if(Continue == true)
	{
		if(Reader.Has(Inspection::Length{0, 64}) == false)
		{
			Result->GetValue()->AddTag("error", "The available length needs to be at least " + to_string_cast(Inspection::Length{0, 64}) + ".");
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
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_BigEndian(Inspection::Reader & Reader, std::uint8_t Bits, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Reader, {{"Getter", std::vector< std::string >{"Number", "Integer", "Unsigned", "BigEndian"}}, {"ElementParameters", std::unordered_map< std::string, std::experimental::any >{{"Bits", Bits}}}, {"NumberOfElements", NumberOfElements}});
}

std::unique_ptr< Inspection::Result > Inspection::Get_UnsignedIntegers_16Bit_BigEndian(Inspection::Reader & Reader, std::uint64_t NumberOfElements)
{
	return Get_Array_EndedByNumberOfElements(Reader, {{"Getter", std::vector< std::string >{"Number", "Integer", "Unsigned", "16Bit_BigEndian"}}, {"NumberOfElements", NumberOfElements}});
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer)
{
	assert(Buffer.GetBitstreamType() == Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
	
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_Vorbis_CommentHeader_WithoutFramingFlag(FieldReader)};
		
		Result->GetValue()->AppendValues(FieldResult->GetValue()->GetValues());
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
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
		Continue = std::experimental::any_cast< bool >(Result->GetValue()->GetValue("FramingFlag")->GetAny());
	}
	//finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_UserComment(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		auto FieldResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		auto FieldValue{Result->GetValue()->AppendValue("Length", FieldResult->GetValue())};
		
		UpdateState(Continue, FieldResult);
		FieldValue->AddTag("unit", "bytes"s);
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("Length")->GetAny()), 0}};
		auto FieldResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(FieldReader, {})};
		auto FieldValue{Result->GetValue()->AppendValue("String", FieldResult->GetValue())};
		
		UpdateState(Continue, Reader, FieldResult, FieldReader);
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > Inspection::Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(Reader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("VendorLength", PartResult->GetValue());
		Result->GetValue()->GetValue("VendorLength")->AddTag("unit", "bytes"s);
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader, Inspection::Length{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("VendorLength")->GetAny()), 0}};
		auto PartResult{Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("Vendor", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_UnsignedInteger_32Bit_LittleEndian(PartReader)};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("UserCommentListLength", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// reading
	if(Continue == true)
	{
		auto UserCommentListLength{std::experimental::any_cast< std::uint32_t >(Result->GetValue()->GetValue("UserCommentListLength")->GetAny())};
		Inspection::Reader PartReader{Reader};
		auto PartResult{Get_Array_EndedByNumberOfElements(PartReader, {{"Getter", std::vector< std::string >{"Vorbis", "CommentHeader_UserComment"}}, {"ElementName", "UserComment"s}, {"NumberOfElements", static_cast< std::uint64_t >(UserCommentListLength)}})};
		
		Continue = PartResult->GetSuccess();
		Result->GetValue()->AppendValue("UserCommentList", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}
