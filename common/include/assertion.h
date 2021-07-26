#ifndef INSPECTION_COMMON_ASSERTION_H
#define INSPECTION_COMMON_ASSERTION_H

#include <cstdint>
#include <iostream>

#ifndef NDEBUG

/**
 * The abort()-call is not part of the log function, so that it is still situated inside the scope of the ASSERTION() caller.
 * This is necessary, because only an abort() will avoid the "control reaches end of non-void function" compiler warning/error.
 **/
#define ASSERTION(Expression) if((Expression) == false) { __AssertionLog(#Expression, __FILE__, __LINE__, __func__); abort(); }
#define ASSERTION_MESSAGE(Expression, Message) if((Expression) == false) { __AssertionLog(#Expression, __FILE__, __LINE__, __func__, Message); abort(); }
#define NOT_IMPLEMENTED(Message) __NotImplementedLog(__FILE__, __LINE__, __func__, Message); abort();

inline void __AssertionLog(const char * ExpressionString, const char * File, std::uint64_t Line, const char * Function, const std::string & Message = "")
{
	std::cerr << "Assertion failed:";
	if(Message.empty() == false)
	{
		std::cerr << "\"" << Message << "\"";
	}
	std::cerr << "\n\tExpected: " << ExpressionString << "\n\tIn source: " << File << ":" << Line << "\n\tIn function: " << Function << '\n';
}

inline void __NotImplementedLog(const std::string & File, std::uint64_t Line, const std::string & Function, const std::string & Message)
{
	std::cerr << "Not implemented:";
	if(Message.empty() == false)
	{
		std::cerr << "\"" << Message << "\"";
	}
	std::cerr << "\n\tIn source: " << File << ":" << Line << "\n\tIn function: " << Function << '\n';
}

#else
#define ASSERTION(Expression) ;
#define ASSERTION_MESSAGE(Expression, Message) ;
#define NOT_IMPLEMENTED(Message) ;
#endif

#endif
