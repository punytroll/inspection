#include <deque>

#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class MPEGInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			// reading
			if(Continue == true)
			{
				auto PartReader = Inspection::Reader{Reader};
				auto PartResult = Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {});
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendField("MPEGStream", PartResult->ExtractValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			// finalization
			Result->SetSuccess((Continue == true) && (Reader.IsAtEnd() == true));
			
			return Result;
		}
	};
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
	
	auto Inspector = Inspection::MPEGInspector{};
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < NumberOfArguments)
	{
		auto Argument = std::string{argv[ArgumentIndex]};
		
		Inspector.PushPath(Argument);
	}
	
	auto Result = 0;
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Result = ((Inspector.Process() == true) ? (0) : (1));
	}
	
	return Result;
}
