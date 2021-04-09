#include <deque>
#include <memory>

#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class FLACInspector : public Inspection::Inspector
	{
	public:
		FLACInspector(void) :
			_WithFrames{false}
		{
		}
		
		void SetWithFrames(void)
		{
			_WithFrames = true;
		}
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
				if(_WithFrames == true)
				{
					Inspection::Reader PartReader{Reader};
					auto PartResult{Inspection::g_TypeRepository.Get({"FLAC", "Stream"}, PartReader, {})};
					
					Continue = PartResult->GetSuccess();
					Result->SetValue(PartResult->GetValue());
					Result->GetValue()->SetName("FLACStream");
					Reader.AdvancePosition(PartReader.GetConsumedLength());
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					auto PartResult{Inspection::g_TypeRepository.Get({"FLAC", "Stream_Header"}, PartReader, {})};
					
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
	private:
		bool _WithFrames;
	};
}

int main(int argc, char ** argv)
{
	Inspection::FLACInspector Inspector;
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < NumberOfArguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument == "--with-frames")
		{
			Inspector.SetWithFrames();
		}
		else
		{
			Inspector.PushPath(Argument);
		}
	}
	
	int Result{0};
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " [--with-frames] <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Inspector.Process();
	}
	
	return Result;
}
