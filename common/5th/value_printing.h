#ifndef COMMON_5TH_VALUE_PRINTING_H
#define COMMON_5TH_VALUE_PRINTING_H

#include <iostream>
#include <string>

#include "../any_printing.h"
#include "value.h"

namespace Inspection
{
	void PrintValue(std::shared_ptr< Inspection::Value > Value, const std::string & Indentation = "")
	{
		auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false)};
		
		if(HeaderLine == true)
		{
			std::cout << Indentation;
		}
		if(Value->GetName().empty() == false)
		{
			std::cout << Value->GetName() << ": ";
		}
		if(Value->GetAny().empty() == false)
		{
			std::cout << Value->GetAny();
		}
		
		auto SubIndentation{Indentation};
		
		if(HeaderLine == true)
		{
			std::cout << std::endl;
			SubIndentation += "    ";
		}
		if(Value->GetCount() > 0)
		{
			for(auto & SubValue : Value->GetValues())
			{
				PrintValue(SubValue, SubIndentation);
			}
		}
	}
}

#endif
