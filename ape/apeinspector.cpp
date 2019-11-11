#include <deque>
#include <memory>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// helper function to find the position of an embedded APETAG                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::int64_t GetAPETAGPosition(const Inspection::Buffer & Buffer)
{
	std::int64_t Result{-1};
	auto Start{0ul};
	auto Position{0ul};
	auto State{0ul};
	auto Length{Buffer.GetLength().GetBytes()};
	
	while((Result == -1) && (Position < Length))
	{
		auto Byte{Buffer.GetData()[Position]};
		
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

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// seeking
	if(Continue == true)
	{
		auto Position{GetAPETAGPosition(Reader.GetBuffer())};
		
		if(Position >= 0)
		{
			Reader.SetPosition(Inspection::Length(Position, 0));
		}
		else
		{
			Continue = false;
		}
	}
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Inspection::g_GetterRepository.Get({"APE", "Tag"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
		Result->GetValue()->SetName("APEv2 Tag");
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
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
		ReadItem(Paths.front(), Process, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
