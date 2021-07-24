#include <deque>

#include "inspector.h"
#include "result.h"
#include "type_repository.h"

namespace Inspection
{
	class MPEGInspector : public Inspection::Inspector
	{
	public:
		MPEGInspector(void) :
			_Seek{false}
		{
		}
		
		constexpr bool GetSeek(void) const noexcept
		{
			return _Seek;
		}
		
		constexpr void SetSeek(void) noexcept
		{
			_Seek = true;
		}
	protected:
		virtual std::unique_ptr< Inspection::Result > _Getter(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result = std::make_unique<Inspection::Result>();
			auto Continue = true;
			
			if(_Seek == false)
			{
				// reading
				if(Continue == true)
				{
					auto PartReader = Inspection::Reader{Reader};
					auto PartResult = Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {});
					
					Continue = PartResult->GetSuccess();
					Result->GetValue()->AppendField("MPEG.1.Stream", PartResult->ExtractValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
				}
			}
			else
			{
				auto Found = false;
				auto LastEnd = Inspection::Length{0, 0};
				
				// reading
				while((Continue == true) && (LastEnd < Buffer.GetLength()))
				{
					auto Start = _GetStartOfNextMPEGFrame(Buffer, LastEnd);
					
					if(Start != LastEnd)
					{
						_AppendOtherData(Result->GetValue(), Start - LastEnd);
					}
					if(Start != Buffer.GetLength())
					{
						Found = true;
						
						auto PartReader = Inspection::Reader{Reader, Start, Buffer.GetLength() - Start};
						auto PartResult = Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {});
						
						Continue = PartResult->GetSuccess();
						Result->GetValue()->AppendField("MPEG.1.Stream", PartResult->ExtractValue());
						LastEnd = Start + PartReader.GetConsumedLength();
					}
					else
					{
						LastEnd = Buffer.GetLength();
					}
				}
				Continue = (Continue == true) && (Found == true);
			}
			// finalization
			Result->SetSuccess((Continue == true) && ((_Seek == true) || (Reader.IsAtEnd() == true)));
			
			return Result;
		}
	private:
		Inspection::Length _GetStartOfNextMPEGFrame(const Inspection::Buffer & Buffer, const Inspection::Length & StartAt)
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
						if(Byte == 0xff)
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
						if(Byte > 0xe0)
						{
							Found = true;
						}
						else if(Byte == 0xff)
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
		
		bool _Seek;
	};
}

int main(int argc, char ** argv)
{
	auto Inspector = Inspection::MPEGInspector{};
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < NumberOfArguments)
	{
		auto Argument = std::string{argv[ArgumentIndex]};
		
		if(Argument == "--seek")
		{
			Inspector.SetSeek();
		}
		else
		{
			Inspector.PushPath(Argument);
		}
	}
	if(Inspector.GetSeek() == false)
	{
		std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
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
