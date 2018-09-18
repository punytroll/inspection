#ifndef INSPECTION_COMMON_VALUE_PRINTING_H
#define INSPECTION_COMMON_VALUE_PRINTING_H

#include <iostream>
#include <string>

#include "any_printing.h"
#include "value.h"

namespace Inspection
{
	const std::string g_DarkBlue{"\033[34m"};
	const std::string g_DarkCyan{"\033[36m"};
	const std::string g_DarkGray{"\033[90m"};
	const std::string g_DarkGreen{"\033[32m"};
	const std::string g_DarkYellow{"\033[33m"};
	const std::string g_LightBlue{"\033[94m"};
	const std::string g_LightCyan{"\033[96m"};
	const std::string g_LightGray{"\033[37m"};
	const std::string g_LightRed{"\033[91m"};
	const std::string g_LightYellow{"\033[93m"};
	const std::string g_White{"\033[97m"};
	const std::string g_Reset{"\033[0m"};
	
	inline void PrintValue(std::shared_ptr< Inspection::Value > Value, const std::string & Indentation = "")
	{
		auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetAny().empty() == false) || (Value->GetTags().empty() == false)};
		
		if(HeaderLine == true)
		{
			std::cout << g_LightGray << Indentation;
		}
		if(Value->GetName().empty() == false)
		{
			if(Value->GetName() == "error")
			{
				std::cout << g_LightRed;
			}
			else
			{
				std::cout << g_White;
			}
			std::cout << Value->GetName();
		}
		if((Value->GetName().empty() == false) && (Value->GetAny().empty() == false))
		{
			std::cout << g_LightGray << ": ";
		}
		if(Value->GetAny().empty() == false)
		{
			if((Value->GetName().empty() == false) && (Value->GetName() == "error"))
			{
				std::cout << g_White;
			}
			else
			{
				std::cout << g_LightCyan;
			}
			std::cout << Value->GetAny();
		}
		if(Value->GetTags().empty() == false)
		{
			auto First{true};
			
			std::cout << g_LightGray <<" {" << g_DarkGray;
			for(auto & Tag : Value->GetTags())
			{
				if(First == false)
				{
					std::cout << g_LightGray << ", " << g_DarkGray;
				}
				if(Tag->GetName().empty() == false)
				{
					if(Tag->GetName() == "error")
					{
						std::cout << g_LightRed;
					}
					else if(Tag->GetAny().empty() == true)
					{
						std::cout << g_DarkGray;
					}
					else
					{
						std::cout << g_DarkYellow;
					}
					std::cout << Tag->GetName();
				}
				if((Tag->GetName().empty() == false) && (Tag->GetAny().empty() == false))
				{
					std::cout << g_LightGray << '=';
				}
				if(Tag->GetAny().empty() == false)
				{
					std::cout << g_DarkGray << Tag->GetAny();
				}
				if(Tag->GetValues().size() > 0)
				{
					throw std::exception();
				}
				if(Tag->GetTags().size() > 0)
				{
					std::cout << g_LightGray << " {" << g_DarkGray;
					
					auto FirstSubTag{true};
					
					for(auto SubTag : Tag->GetTags())
					{
						if(FirstSubTag == false)
						{
							std::cout << ", ";
						}
						if(SubTag->GetName().empty() == false)
						{
							if(SubTag->GetName() == "error")
							{
								std::cout << g_LightRed;
							}
							std::cout << SubTag->GetName();
						}
						std::cout << g_DarkGray;
						if((SubTag->GetName().empty() == false) && (SubTag->GetAny().empty() == false))
						{
							std::cout << '=';
						}
						if(SubTag->GetAny().empty() == false)
						{
							std::cout << SubTag->GetAny();
						}
						if((SubTag->GetValues().size() > 0) || (SubTag->GetTags().size() > 0))
						{
							throw std::exception();
						}
						FirstSubTag = false;
					}
					std::cout << g_LightGray << '}' << g_DarkGray;
				}
				First = false;
			}
			std::cout << g_LightGray << '}';
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
		std::cout << g_Reset;
	}
}

#endif