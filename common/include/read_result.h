#ifndef INSPECTION_COMMON_READ_RESULT_H
#define INSPECTION_COMMON_READ_RESULT_H

namespace Inspection
{
	struct ReadResult
	{
		std::uint8_t Data;
		Inspection::Length InputLength;
		Inspection::Length OutputLength;
		bool Success;
	};
}

#endif
