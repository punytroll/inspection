#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace FLAC
{
	enum class MetaDataBlockType
	{
		StreamInfo,
		Padding,
		Application,
		SeekTable,
		VorbisComment,
		CueSheet,
		Picture,
		Reserved,
		Invalid
	};
}

std::ostream & operator<<(std::ostream & OStream, FLAC::MetaDataBlockType Value)
{
	switch(Value)
	{
	case FLAC::MetaDataBlockType::StreamInfo:
		{
			return OStream << "StreamInfo";
		}
	case FLAC::MetaDataBlockType::Padding:
		{
			return OStream << "Padding";
		}
	case FLAC::MetaDataBlockType::Application:
		{
			return OStream << "Application";
		}
	case FLAC::MetaDataBlockType::SeekTable:
		{
			return OStream << "SeekTable";
		}
	case FLAC::MetaDataBlockType::VorbisComment:
		{
			return OStream << "VorbisComment";
		}
	case FLAC::MetaDataBlockType::CueSheet:
		{
			return OStream << "CueSheet";
		}
	case FLAC::MetaDataBlockType::Picture:
		{
			return OStream << "Picture";
		}
	case FLAC::MetaDataBlockType::Reserved:
		{
			return OStream << "Reserved";
		}
	case FLAC::MetaDataBlockType::Invalid:
	default:
		{
			return OStream << "Invalid";
		}
	}
}

#include "../common/any_printing.h"
#include "../common/file_handling.h"
#include "../common/5th/buffer.h"
#include "../common/5th/getters.h"
#include "../common/5th/result.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockHeader(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockType(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer);
std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlockData(Inspection::Buffer & Buffer);

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockHeader(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto LastMetaDataBlock{Get_Boolean_OneBit(Buffer)};
	
	if(LastMetaDataBlock->GetSuccess() == true)
	{
		Value->Append("LastMetaDataBlock", LastMetaDataBlock->GetValue());
		
		auto MetaDataBlockType{Get_FLAC_MetaDataBlockType(Buffer)};
		
		if(MetaDataBlockType->GetSuccess() == true)
		{
			Value->Append("BlockType", MetaDataBlockType->GetValue());
			
			auto Length{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(Length->GetSuccess() == true)
			{
				Value->Append("Length", Length->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_MetaDataBlockType(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockType{Get_UnsignedInteger_7Bit(Buffer)};
	
	if(MetaDataBlockType->GetSuccess() == true)
	{
		Success = true;
		Value->SetAny(MetaDataBlockType->GetAny());
		
		auto NumericValue{std::experimental::any_cast< std::uint8_t >(MetaDataBlockType->GetAny())};
		
		if(NumericValue == 0x00)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::StreamInfo);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Padding);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Application);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::SeekTable);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::VorbisComment);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::CueSheet);
		}
		else if(NumericValue == 0x01)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Picture);
		}
		else if(NumericValue == 0xff)
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Invalid);
		}
		else
		{
			Value->Append("Interpretation", FLAC::MetaDataBlockType::Reserved);
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_Stream(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto FLACStreamMarkerResult{Get_ASCII_AlphaStringEndedByLength(Buffer, "fLaC")};
	
	if(FLACStreamMarkerResult->GetSuccess() == true)
	{
		// auto LastMetaDataBlock{false};
		
		Value->Append("FLAC stream marker", FLACStreamMarkerResult->GetValue());
		
		auto FLACStreamInfoBlockResult{Get_FLAC_StreamInfoBlock(Buffer)};
		
		if(FLACStreamInfoBlockResult->GetSuccess() == true)
		{
			// LastMetaDataBlock = std::experimental::any_cast< bool >(FLACStreamInfoBlockResult->GetAny("LastMetaDataBlock"));
			Value->Append("StreamInfoBlock", FLACStreamInfoBlockResult->GetValue());
			Success = true;
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlock(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MetaDataBlockHeader{Get_FLAC_MetaDataBlockHeader(Buffer)};
	
	if(MetaDataBlockHeader->GetSuccess() == true)
	{
		auto MetaDataBlockType{std::experimental::any_cast< FLAC::MetaDataBlockType >(MetaDataBlockHeader->GetValue("BlockType")->GetAny("Interpretation"))};
		
		if(MetaDataBlockType == FLAC::MetaDataBlockType::StreamInfo)
		{
			Value->Append("Header", MetaDataBlockHeader->GetValue());
			
			auto StreamInfoBlockData{Get_FLAC_StreamInfoBlockData(Buffer)};
			
			if(StreamInfoBlockData->GetSuccess() == true)
			{
				Value->Append("Data", StreamInfoBlockData->GetValue());
				Success = true;
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

std::unique_ptr< Inspection::Result > Get_FLAC_StreamInfoBlockData(Inspection::Buffer & Buffer)
{
	auto Success{false};
	auto Value{std::make_shared< Inspection::Value >()};
	auto MinimumBlockSize{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
	
	if(MinimumBlockSize->GetSuccess() == true)
	{
		Value->Append("MinimumBlockSize", MinimumBlockSize->GetValue());
		
		auto MaximumBlockSize{Get_UnsignedInteger_16Bit_BigEndian(Buffer)};
		
		if(MaximumBlockSize->GetSuccess() == true)
		{
			Value->Append("MaximumBlockSize", MaximumBlockSize->GetValue());
			
			auto MinimumFrameSize{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
			
			if(MinimumFrameSize->GetSuccess() == true)
			{
				Value->Append("MinimumFrameSize", MinimumFrameSize->GetValue());
				
				auto MaximumFrameSize{Get_UnsignedInteger_24Bit_BigEndian(Buffer)};
				
				if(MaximumFrameSize->GetSuccess() == true)
				{
					Value->Append("MaximumFrameSize", MaximumFrameSize->GetValue());
					
					auto SampleRate{Get_UnsignedInteger_20Bit_BigEndian(Buffer)};
					
					if(SampleRate->GetSuccess() == true)
					{
						Value->Append("SampleRate", SampleRate->GetValue());
						Success = true;
					}
				}
			}
		}
	}
	
	return Inspection::MakeResult(Success, Value);
}

void PrintValue(const std::string & Indentation, std::shared_ptr< Inspection::Value > Value)
{
	auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false)};
	
	if(HeaderLine == true)
	{
		std::cout << Indentation;
	}
	if(Value->GetName().empty() == false)
	{
		std::cout << Value->GetName() << ": ";
	}
	if(Value->GetAny().empty() == false)
	{
		std::cout << Value->GetAny();
	}
	
	auto SubIndentation{Indentation};
	
	if(HeaderLine == true)
	{
		std::cout << std::endl;
		SubIndentation += "    ";
	}
	if(Value->GetCount() > 0)
	{
		for(auto & SubValue : Value->GetValues())
		{
			PrintValue(SubIndentation, SubValue);
		}
	}
}

void PrintValue(std::shared_ptr< Inspection::Value > Value)
{
	PrintValue("", Value);
}

void ReadFile(const std::string & Path)
{
	auto FileDescriptor{open(Path.c_str(), O_RDONLY)};
	
	if(FileDescriptor == -1)
	{
		std::cerr << "Could not open the file \"" << Path << "\"." << std::endl;
	}
	else
	{
		std::int64_t FileSize{GetFileSize(Path)};
		
		if(FileSize != -1)
		{
			auto Address{reinterpret_cast< std::uint8_t * >(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0))};
			
			if(Address == MAP_FAILED)
			{
				std::cerr << "Could not map the file \"" + Path + "\" into memory." << std::endl;
			}
			else
			{
				Inspection::Buffer Buffer{Address, Inspection::Length(FileSize, 0)};
				auto FLACStream{Get_FLAC_Stream(Buffer)};
				
				if(FLACStream->GetSuccess() == true)
				{
					PrintValue(FLACStream->GetValue());
				}
				else
				{
					std::cerr << "The file does not start with FLACStream object." << std::endl;
				}
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
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
		ReadItem(Paths.front());
		Paths.pop_front();
	}
	
	return 0;
}
