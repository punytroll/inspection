#include <bitset>
#include <deque>

#include "getters.h"
#include "inspector.h"
#include "result.h"

namespace Inspection
{
	class VorbisInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			// reading
			if(Continue == true)
			{
				auto PartReader = Inspection::Reader{Reader};
				auto PartResult = Inspection::Get_Ogg_Stream(PartReader, {});
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendField("OggStream", PartResult->GetValue());
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
	Inspection::VorbisInspector Inspector;
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
