#include "buffer.h"
#include "read_result.h"
#include "reader.h"

Inspection::Reader::Reader(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	_BufferCore{std::make_unique<Inspection::Reader::BufferCore>(Buffer, StartPositionInInput, StartPositionInInput + Length, Inspection::Reader::BitstreamType::MostSignificantBitFirst)}
{
}

Inspection::Reader::Reader(const Inspection::Reader & Reader)
{
	if(Reader._BufferCore != nullptr)
	{
		assert(Reader._BufferCore->_ReadPositionInInput <= Reader._BufferCore->_EndPositionInInput);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, Reader._BufferCore->_ReadPositionInInput, Reader._BufferCore->_EndPositionInInput, Reader._BufferCore->_BitstreamType);
		assert(_BufferCore->_StartPositionInInput <= _BufferCore->_Buffer.GetLength());
		assert(_BufferCore->_EndPositionInInput <= _BufferCore->_Buffer.GetLength());
		assert(_BufferCore->_EndPositionInInput <= Reader._BufferCore->_EndPositionInInput);
	}
	else
	{
		assert(false);
	}
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & Length)
{
	if(Reader._BufferCore != nullptr)
	{
		assert(Reader._BufferCore->_ReadPositionInInput + Length <= Reader._BufferCore->_EndPositionInInput);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, Reader._BufferCore->_ReadPositionInInput, Reader._BufferCore->_ReadPositionInInput + Length, Reader._BufferCore->_BitstreamType);
		assert(_BufferCore->_EndPositionInInput <= Reader._BufferCore->_EndPositionInInput);
		assert(_BufferCore->_StartPositionInInput <= _BufferCore->_Buffer.GetLength());
		assert(_BufferCore->_EndPositionInInput <= _BufferCore->_Buffer.GetLength());
	}
	else
	{
		assert(false);
	}
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length)
{
	if(Reader._BufferCore != nullptr)
	{
		assert(Reader._BufferCore->_ReadPositionInInput + Length <= Reader._BufferCore->_EndPositionInInput);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, StartPositionInInput, StartPositionInInput + Length, Reader._BufferCore->_BitstreamType);
		assert(_BufferCore->_EndPositionInInput <= Reader._BufferCore->_EndPositionInInput);
		assert(_BufferCore->_StartPositionInInput <= _BufferCore->_Buffer.GetLength());
		assert(_BufferCore->_EndPositionInInput <= _BufferCore->_Buffer.GetLength());
	}
	else
	{
		assert(false);
	}
}

void Inspection::Reader::AdvancePosition(const Inspection::Length & Offset)
{
	if(_BufferCore != nullptr)
	{
		assert(_BufferCore->_ReadPositionInInput + Offset <= _BufferCore->_EndPositionInInput);
		_BufferCore->_ReadPositionInInput += Offset;
	}
	else
	{
		assert(false);
	}
}

const Inspection::Buffer & Inspection::Reader::GetBuffer(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_Buffer;
	}
	else
	{
		assert(false);
	}
}

const Inspection::Length & Inspection::Reader::GetReadPositionInInput(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInInput;
	}
	else
	{
		assert(false);
	}
}

Inspection::Length Inspection::Reader::GetConsumedLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInInput - _BufferCore->_StartPositionInInput;
	}
	else
	{
		assert(false);
	}
}

Inspection::Length Inspection::Reader::CalculateRemainingOutputLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_EndPositionInInput - _BufferCore->_ReadPositionInInput;
	}
	else
	{
		assert(false);
	}
}

Inspection::Length Inspection::Reader::CalculateRemainingInputLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_EndPositionInInput - _BufferCore->_ReadPositionInInput;
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Has(const Inspection::Length & Length) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInInput + Length <= _BufferCore->_EndPositionInInput;
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::HasRemaining(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInInput < _BufferCore->_EndPositionInInput;
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::IsAtEnd(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInInput == _BufferCore->_EndPositionInInput;
	}
	else
	{
		assert(false);
	}
}

void Inspection::Reader::SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType)
{
	if(_BufferCore != nullptr)
	{
		assert(_BufferCore->_ReadPositionInInput.GetBits() == 0);
		_BufferCore->_BitstreamType = BitstreamType;
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read0Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read0Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read1Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read1Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read2Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read2Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read3Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read3Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read4Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read4Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read5Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read5Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read6Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read6Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read7Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read7Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

bool Inspection::Reader::Read8Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read8Bits(ReadResult);
	}
	else
	{
		assert(false);
	}
}

Inspection::Reader::BufferCore::BufferCore(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & EndPositionInInput, Inspection::Reader::BitstreamType BitstreamType) :
	_BitstreamType{BitstreamType},
	_Buffer{Buffer},
	_EndPositionInInput{EndPositionInInput},
	_ReadPositionInInput{StartPositionInInput},
	_StartPositionInInput{StartPositionInInput}
{
}

bool Inspection::Reader::BufferCore::Read0Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = true;
	ReadResult.Data = 0;
	ReadResult.RequestedLength = Inspection::Length{0, 0};
	ReadResult.InputLength = Inspection::Length{0, 0};
	ReadResult.OutputLength = Inspection::Length{0, 0};
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read1Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 1};
	if(_ReadPositionInInput + Inspection::Length{0, 1} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 1};
		ReadResult.OutputLength = Inspection::Length{0, 1};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (7 - _ReadPositionInInput.GetBits())) & 0x01;
		}
		else
		{
			ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x01;
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
		
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read2Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 2};
	if(_ReadPositionInInput + Inspection::Length{0, 2} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 2};
		ReadResult.OutputLength = Inspection::Length{0, 2};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 7)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (6 - _ReadPositionInInput.GetBits())) & 0x03;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 6)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (14 - _ReadPositionInInput.GetBits()))) & 0x03;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 7)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x03;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x03;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read3Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 3};
	if(_ReadPositionInInput + Inspection::Length{0, 3} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 3};
		ReadResult.OutputLength = Inspection::Length{0, 3};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 6)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (5 - _ReadPositionInInput.GetBits())) & 0x07;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 5)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (13 - _ReadPositionInInput.GetBits()))) & 0x07;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 6)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x07;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x07;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read4Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 4};
	if(_ReadPositionInInput + Inspection::Length{0, 4} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 4};
		ReadResult.OutputLength = Inspection::Length{0, 4};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 5)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (4 - _ReadPositionInInput.GetBits())) & 0x0f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 4)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (12 - _ReadPositionInInput.GetBits()))) & 0x0f;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 5)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x0f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x0f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read5Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 5};
	if(_ReadPositionInInput + Inspection::Length{0, 5} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 5};
		ReadResult.OutputLength = Inspection::Length{0, 5};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 4)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (3 - _ReadPositionInInput.GetBits())) & 0x1f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 3)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (11 - _ReadPositionInInput.GetBits()))) & 0x1f;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 4)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x1f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x1f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read6Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 6};
	if(_ReadPositionInInput + Inspection::Length{0, 6} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 6};
		ReadResult.OutputLength = Inspection::Length{0, 6};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 3)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (2 - _ReadPositionInInput.GetBits())) & 0x3f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 2)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (10 - _ReadPositionInInput.GetBits()))) & 0x3f;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 3)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x3f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x3f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read7Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 7};
	if(_ReadPositionInInput + Inspection::Length{0, 7} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 7};
		ReadResult.OutputLength = Inspection::Length{0, 7};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() < 2)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> (1 - _ReadPositionInInput.GetBits())) & 0x7f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 1)) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (9 - _ReadPositionInInput.GetBits()))) & 0x7f;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() < 2)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x7f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x7f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read8Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 8};
	if(_ReadPositionInInput + Inspection::Length{0, 8} <= _EndPositionInInput)
	{
		ReadResult.InputLength = Inspection::Length{0, 8};
		ReadResult.OutputLength = Inspection::Length{0, 8};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInInput.GetBits() == 0)
			{
				ReadResult.Data = *(_Buffer.GetData() + _ReadPositionInInput.GetBytes());
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) << _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (8 - _ReadPositionInInput.GetBits()))) & 0xff;
			}
		}
		else
		{
			if(_ReadPositionInInput.GetBits() == 0)
			{
				ReadResult.Data = *(_Buffer.GetData() + _ReadPositionInInput.GetBytes());
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0xff;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInInput += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}
