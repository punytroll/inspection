#include <deque>

#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class MPEGInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
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
	};
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
	
	Inspection::MPEGInspector Inspector;
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
