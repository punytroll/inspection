#ifndef INSPECTION_COMMON_ID3_DE_UNSYNCHRONIZATION_EAGER_FILTER_H
#define INSPECTION_COMMON_ID3_DE_UNSYNCHRONIZATION_EAGER_FILTER_H

#include <vector>

namespace Inspection
{
	class Buffer;
	class Length;
	class ReadResult;
	
	class ID3DeUnsynchronizationEagerFilter
	{
	public:
		ID3DeUnsynchronizationEagerFilter(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & LengthInBuffer);
		Inspection::Length GetOutputLength(void) const;
		bool Read8Bits(const Inspection::Length & ReadPositionInOutput, Inspection::ReadResult & ReadResult);
	private:
		std::vector<std::uint8_t> _Output;
	};
}

#endif
