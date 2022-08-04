#include <deque>
#include <memory>
#include <string>

#include <common/buffer.h>
#include <common/inspector.h>
#include <common/reader.h>
#include <common/result.h>
#include <common/type_repository.h>

using namespace std::string_literals;

namespace Inspection
{
	class APEInspector : public Inspection::Inspector
	{
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			auto Found = false;
			auto LastEnd = Inspection::Length{0, 0};
			
			// reading
			while((Continue == true) && (LastEnd < Buffer.GetLength()))
			{
				auto Start = _GetStartOfNextAPETAGS(Buffer, LastEnd);
				
				if(Start != LastEnd)
				{
					_AppendOtherData(Result->GetValue(), Start - LastEnd);
				}
				if(Start != Buffer.GetLength())
				{
					auto PartReader = Inspection::Reader{Reader, Start, Buffer.GetLength() - Start};
					auto PartResult = Inspection::g_TypeRepository.Get({"APE", "Tag"}, PartReader, {});
					
					Found = true;
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendField("APEv2Tag", PartResult->ExtractValue());
					LastEnd = Start + PartReader.GetConsumedLength();
				}
				else
				{
					LastEnd = Buffer.GetLength();
				}
			}
			// finalization
			Result->SetSuccess((Continue == true) && (Found == true));
			
			return Result;
		}
	private:
		Inspection::Length _GetStartOfNextAPETAGS(const Inspection::Buffer & Buffer, const Inspection::Length & StartAt)
		{
			auto Found{false};
			auto Start{0ul};
			auto Position{StartAt.GetBytes()};
			
			if(StartAt.GetBits() != 0)
			{
				Position += 1;
			}
			
			auto State{0ul};
			auto Length{Buffer.GetLength().GetBytes()};
			
			while((Found == false) && (Position < Length))
			{
				auto Byte{Buffer.GetData()[Position]};
				
				switch(State)
				{
				case 0:
					{
						if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 1:
					{
						if(Byte == 'P')
						{
							State = 2;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 2:
					{
						if(Byte == 'E')
						{
							State = 3;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 3:
					{
						if(Byte == 'T')
						{
							State = 4;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 4:
					{
						if(Byte == 'A')
						{
							State = 5;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 5:
					{
						if(Byte == 'G')
						{
							State = 6;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 6:
					{
						if(Byte == 'E')
						{
							State = 7;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				case 7:
					{
						if(Byte == 'X')
						{
							Found = true;
						}
						else if(Byte == 'A')
						{
							Start = Position;
							State = 1;
						}
						else
						{
							State = 0;
						}
						
						break;
					}
				}
				Position += 1;
			}
			if(Found == true)
			{
				return Inspection::Length{Start, 0};
			}
			else
			{
				return Buffer.GetLength();
			}
		}
	};
}

int main(int argc, char ** argv)
{
	Inspection::APEInspector Inspector;
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
		Result = ((Inspector.Process() == true) ? (0) : (1));
	}
	
	return Result;
}
