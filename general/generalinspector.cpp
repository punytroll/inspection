#include <deque>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// 5th generation getters                                                                        //
///////////////////////////////////////////////////////////////////////////////////////////////////


std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Start{Buffer.GetPosition()};
	std::unique_ptr< Inspection::Result > PartialResult;
	
	PartialResult = Get_MPEG_1_Stream(Buffer);
	if(PartialResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendValue("MPEG1Stream", PartialResult->GetValue());
		if(Buffer.GetPosition() == Buffer.GetLength())
		{
			Result->SetSuccess(true);
		}
		else
		{
			PartialResult = Get_APE_Tags(Buffer);
			if(PartialResult->GetSuccess() == true)
			{
				Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
				if(Buffer.GetPosition() == Buffer.GetLength())
				{
					Result->SetSuccess(true);
				}
				else
				{
					throw Inspection::NotImplementedException("Get ID3v1 tag.");
				}
			}
			else
			{
				throw Inspection::NotImplementedException("Get ID3v1 tag.");
			}
		}
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
