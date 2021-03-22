#include <deque>
#include <memory>
#include <string>

#include "buffer.h"
#include "inspector.h"
#include "reader.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class APEInspector : public Inspection::Inspector
	{
	protected:
		virtual std::tuple< Inspection::Length, Inspection::Length > _GetStartAndEnd(const Inspection::Buffer & Buffer, const Inspection::Length & StartAt) override
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
				return {{Start, 0}, Buffer.GetLength()};
			}
			else
			{
				return {Buffer.GetLength(), Buffer.GetLength()};
			}
		}
		
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters)
		{
			auto Result{Inspection::InitializeResult(Reader)};
			auto Continue{Reader.HasRemaining()};
			
			// reading
			if(Continue == true)
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_TypeRepository.Get({"APE", "Tag"}, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->SetValue(PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				Result->GetValue()->SetName("APEv2 Tag");
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
		Inspector.Process();
	}
	
	return Result;
}
