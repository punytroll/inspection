#include <deque>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Continue{true};
	
	// reading
	if(Continue == true)
	{
		Inspection::Reader FieldReader{Buffer};
		auto FieldResult{Get_MPEG_1_Stream(FieldReader, {})};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		FieldResult->GetValue()->SetName("MPEGStream");
	}
	// verification
	if(Continue == true)
	{
		if(Buffer.GetPosition() < Buffer.GetLength())
		{
			Continue = false;
		}
	}
	// finalization
	Result->SetSuccess(Continue);
	Inspection::FinalizeResult(Result, Buffer);
	
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
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
