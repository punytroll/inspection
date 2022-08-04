#include <exception>
#include <iostream>

#include "colors.h"
#include "exception_printing.h"

void Inspection::PrintException(const std::exception & Exception)
{
    std::cerr << Inspection::g_Yellow << Exception.what() << Inspection::g_Reset << std::endl;
    try
    {
        std::rethrow_if_nested(Exception);
    }
    catch(const std::exception & NestedException)
    {
        std::cerr << Inspection::g_Red << "Nested exception" << Inspection::g_Reset << ":" << std::endl;
        Inspection::PrintException(NestedException);
    }
}

void Inspection::PrintExceptions(const std::exception & Exception)
{
    std::cerr << Inspection::g_Red << "Caught an exception while processing" << Inspection::g_Reset << ":" << std::endl;
    Inspection::PrintException(Exception);
    std::cerr << std::endl;
}
