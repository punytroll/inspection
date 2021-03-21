#ifndef INSPECTION_COMMON_EXCEPTION_PRINTING_H
#define INSPECTION_COMMON_EXCEPTION_PRINTING_H

namespace std
{
	class exception;
}

namespace Inspection
{
	void PrintException(const std::exception & Exception);
	void PrintExceptions(const std::exception & Exception);
}

#endif
