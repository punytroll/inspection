#include <deque>
#include <memory>

#include <common/inspector.h>
#include <common/result.h>
#include <common/type_repository.h>

namespace Inspection
{
	class ASFInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters)
		{
			auto Result{Inspection::InitializeResult(Reader)};
			auto Continue{true};
			
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_TypeRepository.Get({"ASF", "File"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->SetValue(PartResult->GetValue());
				Result->GetValue()->SetName("ASFFile");
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			// finalization
			Result->SetSuccess(Continue);
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
	};
}

int main(int argc, char ** argv)
{
	Inspection::ASFInspector Inspector;
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < NumberOfArguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		Inspector.PushPath(Argument);
	}
	
	int Result{0};
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Inspector.Process();
	}
	
	return Result;
}
