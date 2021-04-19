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
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			Result->GetValue()->SetName("ID3Tags");
			
			if(_ID3v1Only == true)
			{
				auto PartReader = Inspection::Reader{Reader, Buffer.GetLength() - Inspection::Length{128, 0}};
				auto PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
				
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
				
				if(Buffer.GetLength() >= Inspection::Length(128, 0))
				{
					auto PartReader = Inspection::Reader{Reader, Buffer.GetLength() - Inspection::Length{128, 0}};
					
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
				
				Inspection::Reader PartReader{Reader};
				auto ID3v2TagResult{Inspection::Get_ID3_2_Tag(PartReader, {})};
				
				Result->GetValue()->AppendField("ID3v2", ID3v2TagResult->GetValue());
				Continue = ((ID3v1TagResult != nullptr) && (ID3v1TagResult->GetSuccess() == true)) || ID3v2TagResult->GetSuccess();
			}
			// finalization
			Result->SetSuccess(Continue);
			
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
