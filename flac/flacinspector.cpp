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
				auto Start = _GetStartOfNextFLAC(Buffer, LastEnd);
				
				if(Start != LastEnd)
				{
					_AppendOtherData(Result->GetValue(), Start - LastEnd);
				}
				if(Start != Buffer.GetLength())
				{
					Found = true;
					
					auto PartReader = Inspection::Reader{Reader, Start, Buffer.GetLength() - Start};
					
					if(_WithFrames == true)
					{
						auto PartResult = Inspection::g_TypeRepository.Get({"FLAC", "Stream"}, PartReader, {});
						
						Continue = PartResult->GetSuccess();
						Result->GetValue()->AppendField("FLACStream", PartResult->GetValue());
						LastEnd = Start + PartReader.GetConsumedLength();
					}
					else
					{
						auto PartResult = Inspection::g_TypeRepository.Get({"FLAC", "Stream_Header"}, PartReader, {});
						
						Continue = PartResult->GetSuccess();
						Result->GetValue()->AppendField("FLACStreamHeader", PartResult->GetValue());
						LastEnd = Start + PartReader.GetConsumedLength();
					}
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
		Inspection::Length _GetStartOfNextFLAC(const Inspection::Buffer & Buffer, const Inspection::Length & StartAt)
		{
			auto Found = false;
			auto Start = 0ul;
			auto Position = StartAt.GetBytes();
			
			if(StartAt.GetBits() != 0)
			{
				Position += 1;
			}
			
			auto State = 0ul;
			auto Length = Buffer.GetLength().GetBytes();
			
			while((Found == false) && (Position < Length))
			{
				auto Byte = Buffer.GetData()[Position];
				
				switch(State)
				{
				case 0:
					{
						if(Byte == 'f')
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
						if(Byte == 'L')
						{
							State = 2;
						}
						else if(Byte == 'f')
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
						if(Byte == 'a')
						{
							State = 3;
						}
						else if(Byte == 'f')
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
						if(Byte == 'C')
						{
							Found = true;
						}
						else if(Byte == 'f')
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
