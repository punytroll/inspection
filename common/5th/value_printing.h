#ifndef COMMON_5TH_VALUE_PRINTING_H
#define COMMON_5TH_VALUE_PRINTING_H

#include <iostream>
#include <string>

#include "../any_printing.h"
#include "value.h"

namespace Inspection
{
	const std::string g_Yellow{"\033[33;1m"};
	const std::string g_Green{"\033[32;1m"};
	const std::string g_Gray{"\033[30;1m"};
	const std::string g_Red{"\033[31;1m"};
	const std::string g_White{"\033[0m"};
	
	void PrintValue(std::shared_ptr< Inspection::Value > Value, const std::string & Indentation = "")
	{
		auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false)};
		
		if(HeaderLine == true)
		{
			std::cout << Indentation;
		}
		if(Value->GetName().empty() == false)
		{
			std::cout << Value->GetName();
		}
		if((Value->GetName().empty() == false) && (Value->GetAny().empty() == false))
		{
			std::cout << ": ";
		}
		if(Value->GetAny().empty() == false)
		{
			std::cout << g_Yellow << Value->GetAny() << g_White;
		}
		if(Value->GetTags().empty() == false)
		{
			auto First{true};
			
			std::cout << " (" << g_Gray;
			for(auto & Tag : Value->GetTags())
			{
				if(First == false)
				{
					std::cout << g_White << ", " << g_Gray;
				}
				std::cout << Tag;
				First = false;
			}
			std::cout << g_White << ')';
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
