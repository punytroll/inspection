#include <deque>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
#include <common/getters.h>
#include <common/result.h>

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	
	Result->GetValue()->SetName("ID3Tags");
	
	std::unique_ptr< Inspection::Result > ID3v1TagResult;
	
	if(Buffer.GetLength() >= Inspection::Length(128ull, 0))
	{
		Buffer.SetPosition(Buffer.GetLength() - Inspection::Length(128ull, 0));
		
		Inspection::Reader FieldReader{Buffer};
		
		ID3v1TagResult = Get_ID3_1_Tag(FieldReader);
		if(ID3v1TagResult->GetSuccess() == true)
		{
			if(ID3v1TagResult->GetValue()->HasValue("AlbumTrack") == true)
			{
				Result->GetValue()->AppendValue("ID3v1.1", ID3v1TagResult->GetValue());
			}
			else
			{
				Result->GetValue()->AppendValue("ID3v1", ID3v1TagResult->GetValue());
			}
		}
	}
	Buffer.SetPosition(Inspection::Length(0ull, 0));
	
	Inspection::Reader FieldReader{Buffer};
	auto ID3v2TagResult{Get_ID3_2_Tag(FieldReader)};
	
	Result->GetValue()->AppendValue("ID3v2", ID3v2TagResult->GetValue());
	Result->SetSuccess(((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess());
	Buffer.SetPosition(Buffer.GetLength());
	Inspection::FinalizeResult(Result, Buffer);
	
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
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}

	return 0;
}
