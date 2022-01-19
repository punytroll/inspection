#ifndef INSPECTION_COMMON_ASSERTION_H
#define INSPECTION_COMMON_ASSERTION_H

#include <cstdint>
#include <iostream>

#ifndef NDEBUG

/**
 * The abort()-call is not part of the log function, so that it is still situated inside the scope of the ASSERTION() caller.
 * This is necessary, because only an abort() will avoid the "control reaches end of non-void function" compiler warning/error.
 **/
#define ASSERTION(Expression) { if((Expression) == false) { __AssertionLog(#Expression, __FILE__, __LINE__, __func__); abort(); } }
#define ASSERTION_MESSAGE(Expression, Message) { if((Expression) == false) { __AssertionLog(#Expression, __FILE__, __LINE__, __func__, Message); abort(); } }

/* Use this assertion macro, whenever it should be impossible to reach the code, because of program logic or possible data. */
#define IMPOSSIBLE_CODE_REACHED(Message) { __Log("Impossible code reached", __FILE__, __LINE__, __func__, Message); abort(); }

/**
 * Use this assertion macro to indicate erroneous input data.
 * Even though, in production code, these errors should be handled with error reporting, possibly using exceptions, during development a short assertion might just be enough.
 **/
#define INVALID_INPUT_IF(Expression, Message) { if((Expression) == true) { __ExpressionLog("Invalid input", #Expression, __FILE__, __LINE__, __func__, Message); abort(); } }

/**
 * Use this assertion macro, to indicate places where the implementation is not yet complete.
 * Especially useful for unimplemented functions.
 **/
#define NOT_IMPLEMENTED(Message) { __Log("Not implemented", __FILE__, __LINE__, __func__, Message); abort(); }

/**
 * Use this assertion macro, if it is possible to reach the case value, but the case selection fails to address that case.
 * Use this for cases which might be subject to change due to future development.
 **/
#define UNEXPECTED_CASE(Message) { __Log("Unexpected case", __FILE__, __LINE__, __func__, Message); abort(); }

inline auto __AssertionLog(char const * ExpressionString, char const * File, std::uint64_t Line, const char * Function, const std::string & Message = "") -> void
{
	std::cerr << "Assertion failed:";
	if(Message.empty() == false)
	{
		std::cerr << " \"" << Message << '"';
	}
	std::cerr << "\n\tExpected: " << ExpressionString << "\n\tIn source: " << File << ":" << Line << "\n\tIn function: " << Function << '\n';
}

inline auto __ExpressionLog(char const * LogCase, char const * ExpressionString, char const * File, std::uint64_t Line, const char * Function, const std::string & Message = "") -> void
{
	std::cerr << LogCase << ": ";
	if(Message.empty() == false)
	{
		std::cerr << '"' << Message << '"';
	}
	std::cerr << "\n\tExpected: " << ExpressionString << "\n\tIn source: " << File << ":" << Line << "\n\tIn function: " << Function << '\n';
}

inline auto __Log(char const * LogCase, char const * File, std::uint64_t Line, char const * Function, std::string const & Message) -> void
{
	std::cerr << LogCase << ": ";
	if(Message.empty() == false)
	{
		std::cerr << '"' << Message << '"';
	}
	std::cerr << "\n\tIn source: " << File << ":" << Line << "\n\tIn function: " << Function << '\n';
}

#else
#define ASSERTION(Expression) ;
#define ASSERTION_MESSAGE(Expression, Message) ;
#define IMPOSSIBLE_CODE_REACHED(Message) ;
#define INVALID_INPUT_IF(Expression, Message) ;
#define NOT_IMPLEMENTED(Message) ;
#define UNEXPECTED_CASE(Message) ;
#endif

#endif
