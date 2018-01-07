#include "buffer.h"
#include "reader.h"

Inspection::Reader::Reader(Inspection::Buffer & Buffer) :
	Reader(Buffer, Buffer.GetPosition(), Buffer.GetLength() - Buffer.GetPosition())
{
}

Inspection::Reader::Reader(Inspection::Buffer & Buffer, const Inspection::Length & Length) :
	Reader(Buffer, Buffer.GetPosition(), Length)
{
}

Inspection::Reader::Reader(Inspection::Buffer & Buffer, const Inspection::Length & OffsetInBuffer, const Inspection::Length & Length) :
	_BoundaryInBuffer(OffsetInBuffer + Length),
	_Buffer(Buffer),
	_OffsetInBuffer(OffsetInBuffer),
	_PositionInBuffer(OffsetInBuffer)
{
}

std::uint8_t Inspection::Reader::Get0Bits(void)
{
	std::uint8_t Result{0};
	
	return Result;
}

std::uint8_t Inspection::Reader::Get1Bits(void)
{
	std::uint8_t Result;
	
	Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (7 - _PositionInBuffer.GetBits())) & 0x01;
	_PositionInBuffer += Inspection::Length(0ull, 1);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get2Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 7)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (6 - _PositionInBuffer.GetBits())) & 0x03;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 6)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (14 - _PositionInBuffer.GetBits()))) & 0x03;
	}
	_PositionInBuffer += Inspection::Length(0ull, 2);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get3Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 6)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (5 - _PositionInBuffer.GetBits())) & 0x07;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 5)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (13 - _PositionInBuffer.GetBits()))) & 0x07;
	}
	_PositionInBuffer += Inspection::Length(0ull, 3);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get4Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 5)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (4 - _PositionInBuffer.GetBits())) & 0x0f;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 4)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (12 - _PositionInBuffer.GetBits()))) & 0x0f;
	}
	_PositionInBuffer += Inspection::Length(0ull, 4);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get5Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 4)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (3 - _PositionInBuffer.GetBits())) & 0x1f;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 3)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (11 - _PositionInBuffer.GetBits()))) & 0x1f;
	}
	_PositionInBuffer += Inspection::Length(0ull, 5);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get6Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 3)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (2 - _PositionInBuffer.GetBits())) & 0x3f;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 2)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (10 - _PositionInBuffer.GetBits()))) & 0x3f;
	}
	_PositionInBuffer += Inspection::Length(0ull, 6);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get7Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() < 2)
	{
		Result = (*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) >> (1 - _PositionInBuffer.GetBits())) & 0x7f;
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << (_PositionInBuffer.GetBits() - 1)) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (9 - _PositionInBuffer.GetBits()))) & 0x7f;
	}
	_PositionInBuffer += Inspection::Length(0ull, 7);
	
	return Result;
}

std::uint8_t Inspection::Reader::Get8Bits(void)
{
	std::uint8_t Result;
	
	if(_PositionInBuffer.GetBits() == 0)
	{
		Result = *(_Buffer.GetData() + _PositionInBuffer.GetBytes());
	}
	else
	{
		Result = ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes()) << _PositionInBuffer.GetBits()) | ((*(_Buffer.GetData() + _PositionInBuffer.GetBytes() + 1ull)) >> (8 - _PositionInBuffer.GetBits()))) & 0xff;
	}
	_PositionInBuffer += Inspection::Length(0ull, 8);
	
	return Result;
}
