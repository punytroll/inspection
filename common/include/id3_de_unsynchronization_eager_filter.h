#ifndef INSPECTION_COMMON_ID3_DE_UNSYNCHRONIZATION_EAGER_FILTER_H
#define INSPECTION_COMMON_ID3_DE_UNSYNCHRONIZATION_EAGER_FILTER_H

#include <vector>

namespace Inspection
{
	class Buffer;
	class Length;
	
	class ID3DeUnsynchronizationEagerFilter
	{
	public:
		ID3DeUnsynchronizationEagerFilter(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & LengthInBuffer);
	private:
		std::vector<std::uint8_t> _Output;
	};
}

#endif
