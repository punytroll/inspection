#ifndef INSPECTION_COMMON_NOT_IMPLEMENTED_EXCEPTION_H
#define INSPECTION_COMMON_NOT_IMPLEMENTED_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace Inspection
{
	class NotImplementedException : public std::runtime_error
	{
	public:
		NotImplementedException(const std::string & Message)
			: std::runtime_error(Message)
		{
		}
	};
}

#endif
