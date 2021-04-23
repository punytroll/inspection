#include <deque>
#include <string>

#include "getters.h"
#include "inspector.h"
#include "result.h"
#include "type_repository.h"

using namespace std::string_literals;

namespace Inspection
{
	class ID3Inspector : public Inspection::Inspector
	{
	public:
		enum class Mode
		{
			ID3v1Only,
			ID3v2Only,
			AllValid
		};
		
		ID3Inspector(void) :
			_Mode{Inspection::ID3Inspector::Mode::AllValid}
		{
		}
		
		void SetID3v1Only(void)
		{
			_Mode = Inspection::ID3Inspector::Mode::ID3v1Only;
		}
		
		void SetID3v2Only(void)
		{
			_Mode = Inspection::ID3Inspector::Mode::ID3v2Only;
		}
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			switch(_Mode)
			{
			case Inspection::ID3Inspector::Mode::ID3v1Only:
				{
					if(Buffer.GetLength() >= Inspection::Length{128, 0})
					{
						auto PartReader = Inspection::Reader{Reader, Buffer.GetLength() - Inspection::Length{128, 0}, Inspection::Length{128, 0}};
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
						Reader.AdvancePosition(PartReader.GetConsumedLength());
					}
					else
					{
						Result->GetValue()->AddTag("error", "Not enough data for an ID3v1 tag."s);
						Continue = false;
					}
					
					break;
				}
			case Inspection::ID3Inspector::Mode::ID3v2Only:
				{
					auto PartReader = Inspection::Reader{Reader};
					auto PartResult = Inspection::Get_ID3_2_Tag(PartReader, {});
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendField("ID3v2", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					
					break;
				}
			case Inspection::ID3Inspector::Mode::AllValid:
				{
					// ID3v2
					{
						auto PartReader = Inspection::Reader{Reader};
						auto PartResult = Inspection::Get_ID3_2_Tag(PartReader, {});
						
						Continue = PartResult->GetSuccess();
						if(Continue == true)
						{
							Result->GetValue()->AppendField("ID3v2", PartResult->GetValue());
						}
						Reader.AdvancePosition(PartReader.GetConsumedLength());
					}
					// ID3v1
					{
						if(Buffer.GetLength() >= Inspection::Length{128, 0})
						{
							auto PartReader = Inspection::Reader{Reader, Buffer.GetLength() - Inspection::Length{128, 0}, Inspection::Length{128, 0}};
							auto PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
							
							Continue = PartResult->GetSuccess();
							if(Continue == true)
							{
								if(PartResult->GetValue()->HasField("AlbumTrack") == true)
								{
									Result->GetValue()->AppendField("ID3v1.1", PartResult->GetValue());
								}
								else
								{
									Result->GetValue()->AppendField("ID3v1", PartResult->GetValue());
								}
							}
							Reader.AdvancePosition(PartReader.GetConsumedLength());
						}
					}
					
					break;
				}
			}
			// finalization
			Result->SetSuccess(Continue);
			
			return Result;
		}
	private:
		Inspection::ID3Inspector::Mode _Mode;
	};
}

int main(int argc, char **argv)
{
	auto Inspector = Inspection::ID3Inspector{};
	auto NumberOfArguments = argc;
	auto ArgumentIndex = 0;

	while(++ArgumentIndex < NumberOfArguments)
	{
		auto Argument = std::string{argv[ArgumentIndex]};
		
		if(Argument == "--id3v1-only")
		{
			Inspector.SetID3v1Only();
		}
		else if(Argument == "--id3v2-only")
		{
			Inspector.SetID3v2Only();
		}
		else
		{
			Inspector.PushPath(Argument);
		}
	}
	
	auto Result = 0;
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " [--id3v1-only|--id3v2-only] <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Inspector.Process();
	}

	return Result;
}
