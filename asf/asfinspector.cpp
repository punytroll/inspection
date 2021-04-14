#include <deque>
#include <memory>

#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class ASFInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_TypeRepository.Get({"ASF", "File"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendField("ASFFile", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			// finalization
			Result->SetSuccess(Continue);
			
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
