#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// helper function to find the position of an embedded APETAG                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::int64_t GetAPETAGOffset(const std::uint8_t * Buffer, std::uint64_t Length, std::uint64_t Offset)
{
	std::int64_t Result{-1};
	auto Start{0ul};
	auto Position{Offset};
	auto State{0ul};
	
	while((Result == -1) && (Position < Length))
	{
		auto Byte{Buffer[Position]};
		
		switch(State)
		{
		case 0:
			{
				if(Byte == 'A')
				{
					Start = Position;
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 1:
			{
				if(Byte == 'P')
				{
					State = 2;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 2:
			{
				if(Byte == 'E')
				{
					State = 3;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 3:
			{
				if(Byte == 'T')
				{
					State = 4;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 4:
			{
				if(Byte == 'A')
				{
					State = 5;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 5:
			{
				if(Byte == 'G')
				{
					State = 6;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 6:
			{
				if(Byte == 'E')
				{
					State = 7;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		case 7:
			{
				if(Byte == 'X')
				{
					Result = Start;
				}
				else if(Byte == 'A')
				{
					State = 1;
				}
				else
				{
					State = 0;
				}
				
				break;
			}
		}
		Position += 1;
	}
	
	return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_APE_Tags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_APE_Tags_Flags(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_APE_Tags_Item(Inspection::Buffer & Buffer);


std::unique_ptr< Inspection::Result > Get_APE_Tags(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Get_APE_Tags_Flags(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Get_APE_Tags_HeaderOrFooter_VersionNumber(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > Get_APE_Tags_Item(Inspection::Buffer & Buffer)
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

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Position{GetAPETAGOffset(Buffer.GetData(), Buffer.GetLength().GetBytes(), 0ul)};
	
	if(Position >= 0)
	{
		Buffer.SetPosition(Inspection::Length(Position, 0));
		
		auto APETagsResult{Get_APE_Tags(Buffer)};
		
		Result->SetValue(APETagsResult->GetValue());
		Result->SetSuccess(APETagsResult->GetSuccess());
		Result->GetValue()->SetName("APEv2 Tag");
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

int main(int argc, char ** argv)
{
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto Argument{0};
	
	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}
	
	return 0;
}
