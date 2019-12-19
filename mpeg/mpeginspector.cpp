#include <deque>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>
#include <common/type_repository.h>

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader PartReader{Reader};
		auto PartResult{Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {})};
		
		Continue = PartResult->GetSuccess();
		Result->SetValue(PartResult->GetValue());
		PartResult->GetValue()->SetName("MPEGStream");
		Reader.AdvancePosition(PartReader.GetConsumedLength());
	}
	// verification
	if(Continue == true)
	{
		Continue = Reader.IsAtEnd();
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
	
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
