#ifndef COMMON_EXPLODE_H
#define COMMON_EXPLODE_H

#include <sstream>
#include <string>

template < class OutputIterator >
void explode(const std::string & String, char Separator, OutputIterator Iterator)
{
	std::istringstream Buffer(String);
    std::string Temporary;

    while(std::getline(Buffer, Temporary, Separator))
	{
        *Iterator++ = Temporary;
	}
}

#endif
