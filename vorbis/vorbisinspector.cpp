#include <bitset>
#include <deque>

#include <common/getters.h>
#include <common/inspector.h>
#include <common/result.h>

namespace Inspection
{
	class VorbisInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters)
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult(Inspection::Get_Ogg_Stream(PartReader, {}));
			
			PartResult->GetValue()->SetName("OggStream");
			Reader.AdvancePosition(PartReader.GetConsumedLength());
			
			return PartResult;
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
