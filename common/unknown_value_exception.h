#ifndef INSPECTION_COMMON_UNKNOWN_VALUE_EXCEPTION_H
#define INSPECTION_COMMON_UNKNOWN_VALUE_EXCEPTIONH

namespace Inspection
{
	class UnknownValueException : public std::invalid_argument
	{
	public:
		UnknownValueException(const std::string & Value) :
			std::invalid_argument("The value \"" + Value + "\" is unkown.")
		{
		}
	};
}

#endif
