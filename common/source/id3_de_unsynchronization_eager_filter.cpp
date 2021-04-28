#include <cstdint>

#include "buffer.h"
#include "id3_de_unsynchronization_eager_filter.h"
#include "length.h"

Inspection::ID3DeUnsynchronizationEagerFilter::ID3DeUnsynchronizationEagerFilter(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & LengthInBuffer)
{
	assert(StartPositionInBuffer.GetBits() == 0);
	assert(LengthInBuffer.GetBits() == 0);
	assert(StartPositionInBuffer <= Buffer.GetLength());
	assert(StartPositionInBuffer + LengthInBuffer <= Buffer.GetLength());
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
