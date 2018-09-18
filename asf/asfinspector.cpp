#include <deque>
#include <memory>

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
		auto FieldResult{Get_ASF_File(FieldReader)};
		auto FieldValue{Result->SetValue(FieldResult->GetValue())};
		
		UpdateState(Continue, Buffer, FieldResult, FieldReader);
		Result->GetValue()->SetName("ASFFile");
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
