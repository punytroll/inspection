#include <deque>
#include <memory>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
#include <common/getters.h>
#include <common/result.h>

bool g_WithFrames{false};

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		if(g_WithFrames == true)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Inspection::g_GetterRepository.Get({"FLAC", "Stream"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->SetValue(PartResult->GetValue());
			Result->GetValue()->SetName("FLACStream");
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Inspection::g_GetterRepository.Get({"FLAC", "Stream_Header"}, PartReader, {})};
			
			Continue = PartResult->GetSuccess();
			Result->SetValue(PartResult->GetValue());
			Result->GetValue()->SetName("FLACStream");
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
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
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < Arguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument == "--with-frames")
		{
			g_WithFrames = true;
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
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), Process, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
