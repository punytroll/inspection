#include <deque>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	
	Result->GetValue()->SetName("ID3Tags");
	
	std::unique_ptr< Inspection::Result > ID3v1TagResult;
	
	if(Reader.GetCompleteLength() >= Inspection::Length(128, 0))
	{
		Reader.SetPosition(Reader.GetCompleteLength() - Inspection::Length(128, 0));
		
		Inspection::Reader PartReader{Reader};
		
		ID3v1TagResult = Get_ID3_1_Tag(PartReader);
		if(ID3v1TagResult->GetSuccess() == true)
		{
			if(ID3v1TagResult->GetValue()->HasField("AlbumTrack") == true)
			{
				Result->GetValue()->AppendField("ID3v1.1", ID3v1TagResult->GetValue());
			}
			else
			{
				Result->GetValue()->AppendField("ID3v1", ID3v1TagResult->GetValue());
			}
		}
	}
	Reader.SetPosition(Inspection::Length(0, 0));
	
	Inspection::Reader PartReader{Reader};
	auto ID3v2TagResult{Get_ID3_2_Tag(PartReader)};
	
	Result->GetValue()->AppendField("ID3v2", ID3v2TagResult->GetValue());
	Result->SetSuccess(((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess());
	Reader.SetPosition(Reader.GetCompleteLength());
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

int main(int argc, char **argv)
{
	std::deque< std::string > Paths;
	unsigned int Arguments(argc);
	unsigned int Argument(0);

	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	// processing
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), Process, DefaultWriter);
		Paths.pop_front();
	}

	return 0;
}
