#ifndef COMMON_5TH_VALUE_PRINTING_H
#define COMMON_5TH_VALUE_PRINTING_H

#include <iostream>
#include <string>

#include "../any_printing.h"
#include "value.h"

namespace Inspection
{
	const std::string g_Blue{"\033[34m"};
	const std::string g_Green{"\033[32m"};
	const std::string g_DarkGray{"\033[90m"};
	const std::string g_DarkYellow{"\033[33m"};
	const std::string g_LightGray{"\033[37m"};
	const std::string g_LightYellow{"\033[93m"};
	const std::string g_Red{"\033[31m"};
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
			std::cout << g_LightYellow << Value->GetAny() << g_White;
		}
		if(Value->GetTags().empty() == false)
		{
			auto First{true};
			
			std::cout << " (" << g_DarkGray;
			for(auto & Tag : Value->GetTags())
			{
				if(First == false)
				{
					std::cout << g_White << ", " << g_DarkGray;
				}
				if(Tag->GetName().empty() == false)
				{
					std::cout << g_LightGray << Tag->GetName() << g_DarkGray;
				}
				if((Tag->GetName().empty() == false) && (Tag->GetAny().empty() == false))
				{
					std::cout << '=' << g_DarkYellow;
				}
				if(Tag->GetAny().empty() == false)
				{
					std::cout << Tag->GetAny();
				}
				if((Tag->GetValues().size() > 0) || (Tag->GetTags().size() > 0))
				{
					throw std::exception();
				}
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
