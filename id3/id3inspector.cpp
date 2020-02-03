#include <deque>
#include <string>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>
#include <common/type_repository.h>

using namespace std::string_literals;

auto g_ID3v1{false};

///////////////////////////////////////////////////////////////////////////////////////////////////
// application                                                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	Result->GetValue()->SetName("ID3Tags");
	
	if(g_ID3v1 == true)
	{
		Reader.SetPosition(Reader.GetCompleteLength() - Inspection::Length(128, 0));
		
		Inspection::Reader PartReader{Reader};
		auto PartResult{Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		if(PartResult->GetValue()->HasField("AlbumTrack") == true)
		{
			Result->GetValue()->AppendField("ID3v1.1", PartResult->GetValue());
		}
		else
		{
			Result->GetValue()->AppendField("ID3v1", PartResult->GetValue());
		}
	}
	else
	{
		std::unique_ptr< Inspection::Result > ID3v1TagResult;
		
		if(Reader.GetCompleteLength() >= Inspection::Length(128, 0))
		{
			Reader.SetPosition(Reader.GetCompleteLength() - Inspection::Length(128, 0));
			
			Inspection::Reader PartReader{Reader};
			
			ID3v1TagResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
		auto ID3v2TagResult{Inspection::Get_ID3_2_Tag(PartReader, {})};
		
		Result->GetValue()->AppendField("ID3v2", ID3v2TagResult->GetValue());
		Continue = ((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess();
	}
	Reader.SetPosition(Reader.GetCompleteLength());
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

int main(int argc, char **argv)
{
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto ArgumentIndex{0};

	while(++ArgumentIndex < Arguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument == "--id3v1")
		{
			g_ID3v1 = true;
		}
		else
		{
			Paths.push_back(Argument);
		}
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
