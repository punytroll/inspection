#include "buffer.h"
#include "read_result.h"
#include "reader.h"

Inspection::Reader::Reader(Inspection::Buffer & Buffer) :
	Inspection::Reader{&Buffer, Inspection::Length{0, 0}, Buffer.GetLength(), Inspection::Reader::BitstreamType::MostSignificantBitFirst}
{
}

Inspection::Reader::Reader(Inspection::Reader & Reader) :
	Inspection::Reader{Reader._Buffer, Reader._PositionInBuffer, Reader._EndPositionInInput - Reader._PositionInBuffer, Reader._BitstreamType}
{
	assert(Reader._PositionInBuffer <= Reader._EndPositionInInput);
	assert(_EndPositionInInput <= Reader._EndPositionInInput);
}

Inspection::Reader::Reader(Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	Inspection::Reader{&Buffer, StartPositionInInput, Length, Inspection::Reader::BitstreamType::MostSignificantBitFirst}
{
}

Inspection::Reader::Reader(Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	Inspection::Reader{Reader._Buffer, StartPositionInInput, Length, Reader._BitstreamType}
{
}

Inspection::Reader::Reader(Inspection::Buffer * Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length, Inspection::Reader::BitstreamType BitstreamType) :
	_BitstreamType{BitstreamType},
	_Buffer{Buffer},
	_EndPositionInInput{StartPositionInInput + Length},
	_PositionInBuffer{StartPositionInInput},
	_StartPositionInInput{StartPositionInInput}
{
	assert((Buffer == nullptr) || (_StartPositionInInput <= Buffer->GetLength()));
	assert((Buffer == nullptr) || (_EndPositionInInput <= Buffer->GetLength()));
}

Inspection::Reader::Reader(Inspection::Reader & Reader, const Inspection::Length & Length) :
	Inspection::Reader{Reader._Buffer, Reader._PositionInBuffer, Length, Reader._BitstreamType}
{
	assert(Reader._PositionInBuffer + Length <= Reader._EndPositionInInput);
	assert(_EndPositionInInput <= Reader._EndPositionInInput);
}

std::uint8_t Inspection::Reader::Get0Bits(void)
{
	std::uint8_t Result{0};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get1Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 1}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (7 - _PositionInBuffer.GetBits())) & 0x01;
	}
	else
	{
		Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x01;
	}
	_PositionInBuffer += Inspection::Length{0, 1};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get2Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 2}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 7)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (6 - _PositionInBuffer.GetBits())) & 0x03;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 6)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (14 - _PositionInBuffer.GetBits()))) & 0x03;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 7)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x03;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x03;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 2};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get3Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 3}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 6)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (5 - _PositionInBuffer.GetBits())) & 0x07;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 5)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (13 - _PositionInBuffer.GetBits()))) & 0x07;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 6)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x07;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x07;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 3};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get4Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 4}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 5)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (4 - _PositionInBuffer.GetBits())) & 0x0f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 4)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (12 - _PositionInBuffer.GetBits()))) & 0x0f;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 5)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x0f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x0f;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 4};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get5Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 5}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 4)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (3 - _PositionInBuffer.GetBits())) & 0x1f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 3)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (11 - _PositionInBuffer.GetBits()))) & 0x1f;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 4)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x1f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x1f;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 5};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get6Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 6}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 3)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (2 - _PositionInBuffer.GetBits())) & 0x3f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 2)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (10 - _PositionInBuffer.GetBits()))) & 0x3f;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 3)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x3f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x3f;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 6};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get7Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 7}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() < 2)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> (1 - _PositionInBuffer.GetBits())) & 0x7f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 1)) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (9 - _PositionInBuffer.GetBits()))) & 0x7f;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() < 2)
		{
			Result = (*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) & 0x7f;
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0x7f;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 7};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get8Bits(void)
{
	assert(_Buffer != nullptr);
	assert(Has(Inspection::Length{0, 8}) == true);
	
	std::uint8_t Result;
	
	if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
	{
		if(_PositionInBuffer.GetBits() == 0)
		{
			Result = *(_Buffer->GetData() + _PositionInBuffer.GetBytes());
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (8 - _PositionInBuffer.GetBits()))) & 0xff;
		}
	}
	else
	{
		if(_PositionInBuffer.GetBits() == 0)
		{
			Result = *(_Buffer->GetData() + _PositionInBuffer.GetBytes());
		}
		else
		{
			Result = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0xff;
		}
	}
	_PositionInBuffer += Inspection::Length{0, 8};
	
	return Result;
}

bool Inspection::Reader::Read8Bits(ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.OutputLength = Inspection::Length{0, 8};
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 8}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 8};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_PositionInBuffer.GetBits() == 0)
				{
					ReadResult.Data = *(_Buffer->GetData() + _PositionInBuffer.GetBytes());
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) << _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) >> (8 - _PositionInBuffer.GetBits()))) & 0xff;
				}
			}
			else
			{
				if(_PositionInBuffer.GetBits() == 0)
				{
					ReadResult.Data = *(_Buffer->GetData() + _PositionInBuffer.GetBytes());
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes()) >> _PositionInBuffer.GetBits()) | ((*(_Buffer->GetData() + _PositionInBuffer.GetBytes() + 1)) << (8 - _PositionInBuffer.GetBits()))) & 0xff;
				}
			}
			_PositionInBuffer += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _PositionInBuffer;
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}
