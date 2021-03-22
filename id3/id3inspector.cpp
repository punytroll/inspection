#include <deque>
#include <string>

#include "getters.h"
#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class ID3Inspector : public Inspection::Inspector
	{
	public:
		ID3Inspector(void) :
			_ID3v1Only{false}
		{
		}
		
		void SetID3v1Only(void)
		{
			_ID3v1Only = true;
		}
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters)
		{
			auto Result{Inspection::InitializeResult(Reader)};
			auto Continue{true};
			
			Result->GetValue()->SetName("ID3Tags");
			
			if(_ID3v1Only == true)
			{
				Reader.SetPosition(Reader.GetCompleteLength() - Inspection::Length(128, 0));
				
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				if(PartResult->GetValue()->HasField("AlbumTrack") == true)
				{
					Result->GetValue()->AppendField("ID3v1.1", PartResult->GetValue());
				}
				else
				{
					Result->GetValue()->AppendField("ID3v1", PartResult->GetValue());
				}
			}
			else
			{
				std::unique_ptr< Inspection::Result > ID3v1TagResult;
				
				if(Reader.GetCompleteLength() >= Inspection::Length(128, 0))
				{
					Reader.SetPosition(Reader.GetCompleteLength() - Inspection::Length(128, 0));
					
					Inspection::Reader PartReader{Reader};
					
					ID3v1TagResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
					if(ID3v1TagResult->GetSuccess() == true)
					{
						if(ID3v1TagResult->GetValue()->HasField("AlbumTrack") == true)
						{
							Result->GetValue()->AppendField("ID3v1.1", ID3v1TagResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendField("ID3v1", ID3v1TagResult->GetValue());
						}
					}
				}
				Reader.SetPosition(Inspection::Length(0, 0));
				
				Inspection::Reader PartReader{Reader};
				auto ID3v2TagResult{Inspection::Get_ID3_2_Tag(PartReader, {})};
				
				Result->GetValue()->AppendField("ID3v2", ID3v2TagResult->GetValue());
				Continue = ((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess();
			}
			Reader.SetPosition(Reader.GetCompleteLength());
			// finalization
			Result->SetSuccess(Continue);
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
	private:
		bool _ID3v1Only;
	};
}

int main(int argc, char **argv)
{
	Inspection::ID3Inspector Inspector;
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};

	while(++ArgumentIndex < NumberOfArguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument == "--id3v1-only")
		{
			Inspector.SetID3v1Only();
		}
		else
		{
			Inspector.PushPath(Argument);
		}
	}
	
	int Result{0};
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " [--id3v1-only] <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Inspector.Process();
	}

	return Result;
}
