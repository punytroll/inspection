#include <cstdint>

#include "assertion.h"
#include "buffer.h"
#include "id3_de_unsynchronization_eager_filter.h"
#include "length.h"
#include "read_result.h"

Inspection::ID3DeUnsynchronizationEagerFilter::ID3DeUnsynchronizationEagerFilter(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & LengthInBuffer)
{
	ASSERTION(StartPositionInBuffer.GetBits() == 0);
	ASSERTION(LengthInBuffer.GetBits() == 0);
	ASSERTION(StartPositionInBuffer <= Buffer.GetLength());
	ASSERTION(StartPositionInBuffer + LengthInBuffer <= Buffer.GetLength());
	_Output.reserve(LengthInBuffer.GetBytes());
	
	auto Begin = Buffer.GetData() + StartPositionInBuffer.GetBytes();
	auto End = Buffer.GetData() + StartPositionInBuffer.GetBytes() + LengthInBuffer.GetBytes();
	auto LastByte = std::uint8_t{0};
	
	while(Begin != End)
	{
		if((LastByte == 0xff) && (*Begin == 0x00))
		{
			LastByte = *Begin;
			++Begin;
		}
		else
		{
			LastByte = *Begin;
			_Output.push_back(*Begin);
			++Begin;
		}
	}
}

Inspection::Length Inspection::ID3DeUnsynchronizationEagerFilter::GetOutputLength(void) const
{
	return Inspection::Length{_Output.size(), 0};
}

bool Inspection::ID3DeUnsynchronizationEagerFilter::Read1Bits(const Inspection::Length & ReadPositionInOutput, Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 1};
	ReadResult.InputLength = Inspection::Length{0, 0};
	if(ReadPositionInOutput + Inspection::Length{0, 1} <= GetOutputLength())
	{
		ReadResult.OutputLength = Inspection::Length{0, 1};
		ReadResult.Data = (_Output[ReadPositionInOutput.GetBytes()] >> (7 - ReadPositionInOutput.GetBits())) & 0x01;
		ReadResult.Success = true;
		
	}
	else
	{
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::ID3DeUnsynchronizationEagerFilter::Read4Bits(const Inspection::Length & ReadPositionInOutput, Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 4};
	ReadResult.InputLength = Inspection::Length{0, 0};
	if(ReadPositionInOutput + Inspection::Length{0, 4} <= GetOutputLength())
	{
		ReadResult.OutputLength = Inspection::Length{0, 4};
		if(ReadPositionInOutput.GetBits() < 5)
		{
			ReadResult.Data = (_Output[ReadPositionInOutput.GetBytes()] >> (4 - ReadPositionInOutput.GetBits())) & 0x0f;
		}
		else
		{
			ReadResult.Data = (_Output[ReadPositionInOutput.GetBytes()] << (ReadPositionInOutput.GetBits() - 4)) | ((_Output[ReadPositionInOutput.GetBytes() + 1] >> (12 - ReadPositionInOutput.GetBits())) & 0x0f);
		}
		ReadResult.Success = true;
		
	}
	else
	{
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}

bool Inspection::ID3DeUnsynchronizationEagerFilter::Read8Bits(const Inspection::Length & ReadPositionInOutput, Inspection::ReadResult & ReadResult)
{
	ReadResult.RequestedLength = Inspection::Length{0, 8};
	ReadResult.InputLength = Inspection::Length{0, 0};
	if(ReadPositionInOutput + Inspection::Length{0, 8} <= GetOutputLength())
	{
		ReadResult.OutputLength = Inspection::Length{0, 8};
		if(ReadPositionInOutput.GetBits() == 0)
		{
			ReadResult.Data = _Output[ReadPositionInOutput.GetBytes()];
		}
		else
		{
			ReadResult.Data = (_Output[ReadPositionInOutput.GetBytes()] << ReadPositionInOutput.GetBits()) | ((_Output[ReadPositionInOutput.GetBytes() + 1] >> (8 - ReadPositionInOutput.GetBits())) & 0xff);
		}
		ReadResult.Success = true;
	}
	else
	{
		ReadResult.OutputLength = Inspection::Length{0, 0};
		ReadResult.Data = 0;
		ReadResult.Success = false;
	}
	
	return ReadResult.Success;
}
