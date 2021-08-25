#include "assertion.h"
#include "buffer.h"
#include "id3_de_unsynchronization_eager_filter.h"
#include "read_result.h"
#include "reader.h"

Inspection::Reader::Reader(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	_BufferCore{std::make_unique<Inspection::Reader::BufferCore>(Buffer, StartPositionInInput, StartPositionInInput + Length, Inspection::Reader::BitstreamType::MostSignificantBitFirst)}
{
}

Inspection::Reader::Reader(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter) :
	_ID3DeUnsynchronizationEagerFilterCore{std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(ID3DeUnsynchronizationEagerFilter)}
{
	_ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput = Inspection::Length{0, 0};
	_ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput = ID3DeUnsynchronizationEagerFilter.GetOutputLength();
	_ID3DeUnsynchronizationEagerFilterCore->_ProducedLengthInOutput = Inspection::Length{0, 0};
}

Inspection::Reader::Reader(const Inspection::Reader & Reader)
{
	if(Reader._BufferCore != nullptr)
	{
		ASSERTION(Reader._BufferCore->_ReadPositionInBuffer <= Reader._BufferCore->_EndPositionInBuffer);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, Reader._BufferCore->_ReadPositionInBuffer, Reader._BufferCore->_EndPositionInBuffer, Reader._BufferCore->_BitstreamType);
		ASSERTION(_BufferCore->_StartPositionInBuffer <= _BufferCore->_Buffer.GetLength());
		ASSERTION(_BufferCore->_EndPositionInBuffer <= _BufferCore->_Buffer.GetLength());
		ASSERTION(_BufferCore->_EndPositionInBuffer <= Reader._BufferCore->_EndPositionInBuffer);
	}
	else if(Reader._ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		_ID3DeUnsynchronizationEagerFilterCore = std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(Reader._ID3DeUnsynchronizationEagerFilterCore->_ID3DeUnsynchronizationEagerFilter);
		_ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput = Reader._ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput;
		_ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput = Reader._ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput;
		_ID3DeUnsynchronizationEagerFilterCore->_ProducedLengthInOutput = Inspection::Length{0, 0};
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & Length)
{
	if(Reader._BufferCore != nullptr)
	{
		ASSERTION(Reader._BufferCore->_ReadPositionInBuffer + Length <= Reader._BufferCore->_EndPositionInBuffer);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, Reader._BufferCore->_ReadPositionInBuffer, Reader._BufferCore->_ReadPositionInBuffer + Length, Reader._BufferCore->_BitstreamType);
		ASSERTION(_BufferCore->_EndPositionInBuffer <= Reader._BufferCore->_EndPositionInBuffer);
		ASSERTION(_BufferCore->_StartPositionInBuffer <= _BufferCore->_Buffer.GetLength());
		ASSERTION(_BufferCore->_EndPositionInBuffer <= _BufferCore->_Buffer.GetLength());
	}
	else if(Reader._ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		_ID3DeUnsynchronizationEagerFilterCore = std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(Reader._ID3DeUnsynchronizationEagerFilterCore->_ID3DeUnsynchronizationEagerFilter);
		_ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput = Reader._ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput;
		_ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput = Reader._ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput + Length;
		_ID3DeUnsynchronizationEagerFilterCore->_ProducedLengthInOutput = Inspection::Length{0, 0};
		ASSERTION(_ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput <= _ID3DeUnsynchronizationEagerFilterCore->_ID3DeUnsynchronizationEagerFilter.GetOutputLength());
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length)
{
	if(Reader._BufferCore != nullptr)
	{
		ASSERTION(Reader._BufferCore->_ReadPositionInBuffer + Length <= Reader._BufferCore->_EndPositionInBuffer);
		_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader._BufferCore->_Buffer, StartPositionInInput, StartPositionInInput + Length, Reader._BufferCore->_BitstreamType);
		ASSERTION(_BufferCore->_EndPositionInBuffer <= Reader._BufferCore->_EndPositionInBuffer);
		ASSERTION(_BufferCore->_StartPositionInBuffer <= _BufferCore->_Buffer.GetLength());
		ASSERTION(_BufferCore->_EndPositionInBuffer <= _BufferCore->_Buffer.GetLength());
	}
	else if(Reader._ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("creating a new reader from a reader with an ID3DeUnsynchronizationEagerFilterCore, given a start position and a length");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

void Inspection::Reader::AdvancePosition(const Inspection::Length & Offset)
{
	if(_BufferCore != nullptr)
	{
		ASSERTION(_BufferCore->_ReadPositionInBuffer + Offset <= _BufferCore->_EndPositionInBuffer);
		_BufferCore->_ReadPositionInBuffer += Offset;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		ASSERTION(_ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput + Offset <= _ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput);
		_ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput += Offset;
		_ID3DeUnsynchronizationEagerFilterCore->_ProducedLengthInOutput += Offset;
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

const Inspection::Buffer & Inspection::Reader::GetBuffer(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_Buffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("GetBuffer() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

const Inspection::Length & Inspection::Reader::GetReadPositionInInput(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("GetReadPositionInInput() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Length Inspection::Reader::GetConsumedLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInBuffer - _BufferCore->_StartPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->_ProducedLengthInOutput;
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Length Inspection::Reader::CalculateRemainingOutputLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_EndPositionInBuffer - _BufferCore->_ReadPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput - _ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput;
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Length Inspection::Reader::CalculateRemainingInputLength(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_EndPositionInBuffer - _BufferCore->_ReadPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("CalculateRemainingInputLength() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Has(const Inspection::Length & Length) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInBuffer + Length <= _BufferCore->_EndPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Has() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::HasRemaining(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInBuffer < _BufferCore->_EndPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput < _ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput;
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::IsAtEnd(void) const
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->_ReadPositionInBuffer == _BufferCore->_EndPositionInBuffer;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->_ReadPositionInFilterOutput == _ID3DeUnsynchronizationEagerFilterCore->_EndPositionInFilterOutput;
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

void Inspection::Reader::SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType)
{
	if(_BufferCore != nullptr)
	{
		ASSERTION(_BufferCore->_ReadPositionInBuffer.GetBits() == 0);
		_BufferCore->_BitstreamType = BitstreamType;
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("SetBitstreamType() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read0Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read0Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read0Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read1Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read1Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->Read1Bits(ReadResult);
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read2Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read2Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read2Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read3Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read3Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read3Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read4Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read4Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->Read4Bits(ReadResult);
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read5Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read5Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read5Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read6Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read6Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read6Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read7Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read7Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		NOT_IMPLEMENTED("Read7Bits() from an ID3DeUnsynchronizationEagerFilterCore");
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

bool Inspection::Reader::Read8Bits(Inspection::ReadResult & ReadResult)
{
	if(_BufferCore != nullptr)
	{
		return _BufferCore->Read8Bits(ReadResult);
	}
	else if(_ID3DeUnsynchronizationEagerFilterCore != nullptr)
	{
		return _ID3DeUnsynchronizationEagerFilterCore->Read8Bits(ReadResult);
	}
	else
	{
		UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
	}
}

Inspection::Reader::BufferCore::BufferCore(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & EndPositionInBuffer, Inspection::Reader::BitstreamType BitstreamType) :
	_BitstreamType{BitstreamType},
	_Buffer{Buffer},
	_EndPositionInBuffer{EndPositionInBuffer},
	_ReadPositionInBuffer{StartPositionInBuffer},
	_StartPositionInBuffer{StartPositionInBuffer}
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
	if(_ReadPositionInBuffer + Inspection::Length{0, 1} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 1};
		ReadResult.OutputLength = Inspection::Length{0, 1};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (7 - _ReadPositionInBuffer.GetBits())) & 0x01;
		}
		else
		{
			ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x01;
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
		
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read2Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 2};
	if(_ReadPositionInBuffer + Inspection::Length{0, 2} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 2};
		ReadResult.OutputLength = Inspection::Length{0, 2};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 7)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (6 - _ReadPositionInBuffer.GetBits())) & 0x03;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 6)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (14 - _ReadPositionInBuffer.GetBits()))) & 0x03;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 7)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x03;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x03;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read3Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 3};
	if(_ReadPositionInBuffer + Inspection::Length{0, 3} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 3};
		ReadResult.OutputLength = Inspection::Length{0, 3};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 6)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (5 - _ReadPositionInBuffer.GetBits())) & 0x07;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 5)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (13 - _ReadPositionInBuffer.GetBits()))) & 0x07;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 6)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x07;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x07;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read4Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 4};
	if(_ReadPositionInBuffer + Inspection::Length{0, 4} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 4};
		ReadResult.OutputLength = Inspection::Length{0, 4};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 5)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (4 - _ReadPositionInBuffer.GetBits())) & 0x0f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 4)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (12 - _ReadPositionInBuffer.GetBits()))) & 0x0f;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 5)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x0f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x0f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read5Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 5};
	if(_ReadPositionInBuffer + Inspection::Length{0, 5} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 5};
		ReadResult.OutputLength = Inspection::Length{0, 5};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 4)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (3 - _ReadPositionInBuffer.GetBits())) & 0x1f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 3)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (11 - _ReadPositionInBuffer.GetBits()))) & 0x1f;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 4)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x1f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x1f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read6Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 6};
	if(_ReadPositionInBuffer + Inspection::Length{0, 6} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 6};
		ReadResult.OutputLength = Inspection::Length{0, 6};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 3)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (2 - _ReadPositionInBuffer.GetBits())) & 0x3f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 2)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (10 - _ReadPositionInBuffer.GetBits()))) & 0x3f;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 3)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x3f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x3f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read7Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 7};
	if(_ReadPositionInBuffer + Inspection::Length{0, 7} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 7};
		ReadResult.OutputLength = Inspection::Length{0, 7};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() < 2)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> (1 - _ReadPositionInBuffer.GetBits())) & 0x7f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << (_ReadPositionInBuffer.GetBits() - 1)) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (9 - _ReadPositionInBuffer.GetBits()))) & 0x7f;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() < 2)
			{
				ReadResult.Data = (*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) & 0x7f;
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0x7f;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read8Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 8};
	if(_ReadPositionInBuffer + Inspection::Length{0, 8} <= _EndPositionInBuffer)
	{
		ReadResult.InputLength = Inspection::Length{0, 8};
		ReadResult.OutputLength = Inspection::Length{0, 8};
		if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
		{
			if(_ReadPositionInBuffer.GetBits() == 0)
			{
				ReadResult.Data = *(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes());
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) << _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) >> (8 - _ReadPositionInBuffer.GetBits()))) & 0xff;
			}
		}
		else
		{
			if(_ReadPositionInBuffer.GetBits() == 0)
			{
				ReadResult.Data = *(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes());
			}
			else
			{
				ReadResult.Data = ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes()) >> _ReadPositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _ReadPositionInBuffer.GetBytes() + 1)) << (8 - _ReadPositionInBuffer.GetBits()))) & 0xff;
			}
		}
		ReadResult.Success = true;
		_ReadPositionInBuffer += ReadResult.InputLength;
	}
	else
	{
		ReadResult.InputLength = _EndPositionInBuffer - _ReadPositionInBuffer;
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::ID3DeUnsynchronizationEagerFilterCore(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter) :
	_ID3DeUnsynchronizationEagerFilter{ID3DeUnsynchronizationEagerFilter}
{
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read1Bits(Inspection::ReadResult & ReadResult)
{
	auto Result = _ID3DeUnsynchronizationEagerFilter.Read1Bits(_ReadPositionInFilterOutput, ReadResult);
	
	if(Result == true)
	{
		_ReadPositionInFilterOutput += ReadResult.OutputLength;
		_ProducedLengthInOutput += ReadResult.OutputLength;
	}
	
	return Result;
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read4Bits(Inspection::ReadResult & ReadResult)
{
	auto Result = _ID3DeUnsynchronizationEagerFilter.Read4Bits(_ReadPositionInFilterOutput, ReadResult);
	
	if(Result == true)
	{
		_ReadPositionInFilterOutput += ReadResult.OutputLength;
		_ProducedLengthInOutput += ReadResult.OutputLength;
	}
	
	return Result;
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read8Bits(Inspection::ReadResult & ReadResult)
{
	auto Result = _ID3DeUnsynchronizationEagerFilter.Read8Bits(_ReadPositionInFilterOutput, ReadResult);
	
	if(Result == true)
	{
		_ReadPositionInFilterOutput += ReadResult.OutputLength;
		_ProducedLengthInOutput += ReadResult.OutputLength;
	}
	
	return Result;
}
