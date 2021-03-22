#ifndef INSPECTION_COMMON_QUERY_H
#define INSPECTION_COMMON_QUERY_H

#include <string>
#include <vector>

#include "value.h"

namespace Inspection
{
	std::vector< std::string > SplitString(const std::string & String, char Delimiter);
	bool EvaluateTestQuery(std::shared_ptr< Inspection::Value > Value, const std::string & Query);
}

#endif
