#include <bitset>
#include <cassert>
#include <deque>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

#include "../common/5th.h"
#include "../common/file_handling.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////
std::unique_ptr< Inspection::Result > Get_APE_Identifier(Inspection::Buffer & Buffer);


std::unique_ptr< Inspection::Result > Get_APE_Identifier(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto State{0ul};
	
	while(Buffer.GetPosition() < Buffer.GetLength())
	{
		auto Byte{Buffer.Get8Bits()};
		
		switch(State)
		{
		case 0:
			{
				if(Byte == 'A')
				{
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
					Result->GetValue()->Append("Found", "APETAGEX"s);
					Result->SetSuccess(true);
					Buffer.SetPosition(Buffer.GetLength());
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
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto APETagResult(Get_APE_Identifier(Buffer));
	
	APETagResult->GetValue()->SetName("APE Tag");
	
	return APETagResult;
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
