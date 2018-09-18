#include <deque>
#include <memory>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>

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

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// seeking
	if(Continue == true)
	{
		auto Position{GetAPETAGOffset(Buffer.GetData(), Buffer.GetLength().GetBytes(), 0ul)};
		
		if(Position >= 0)
		{
			Buffer.SetPosition(Inspection::Length(Position, 0));
		}
		else
		{
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_APE_Tags(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		Result->GetValue()->SetName("APEv2 Tag");
	}
	// finalization
	Result->SetSuccess(Continue);
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
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
