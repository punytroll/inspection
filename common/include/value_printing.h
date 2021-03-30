#ifndef INSPECTION_COMMON_VALUE_PRINTING_H
#define INSPECTION_COMMON_VALUE_PRINTING_H

#include <iostream>
#include <string>

#include "any_printing.h"
#include "colors.h"
#include "value.h"

namespace Inspection
{
	inline void PrintValue(std::shared_ptr< Inspection::Value > Value, const std::string & Indentation = "")
	{
		auto HeaderLine{(Value->GetName().empty() == false) || (Value->GetData().has_value() == true) || (Value->GetTags().empty() == false)};
		
		if(HeaderLine == true)
		{
			std::cout << g_White << Indentation;
		}
		if(Value->GetName().empty() == false)
		{
			if(Value->GetName() == "error")
			{
				std::cout << g_BrightRed;
			}
			else
			{
				std::cout << g_BrightWhite;
			}
			std::cout << Value->GetName();
		}
		if((Value->GetName().empty() == false) && (Value->GetData().has_value() == true))
		{
			std::cout << g_White << ": ";
		}
		if(Value->GetData().has_value() == true)
		{
			if((Value->GetName().empty() == false) && (Value->GetName() == "error"))
			{
				std::cout << g_BrightWhite;
			}
			else
			{
				std::cout << g_BrightCyan;
			}
			std::cout << Value->GetData();
		}
		if(Value->GetTags().empty() == false)
		{
			auto First{true};
			
			std::cout << g_White <<" {" << g_BrightBlack;
			for(auto & Tag : Value->GetTags())
			{
				if(First == false)
				{
					std::cout << g_White << ", " << g_BrightBlack;
				}
				if(Tag->GetName().empty() == false)
				{
					if(Tag->GetName() == "error")
					{
						std::cout << g_BrightRed;
					}
					else if(Tag->GetData().has_value() == false)
					{
						std::cout << g_BrightBlack;
					}
					else
					{
						std::cout << g_Yellow;
					}
					std::cout << Tag->GetName();
				}
				if((Tag->GetName().empty() == false) && (Tag->GetData().has_value() == true))
				{
					std::cout << g_White << '=';
				}
				if(Tag->GetData().has_value() == true)
				{
					if(Tag->GetData().type() == typeid(nullptr))
					{
						std::cout << g_Green;
					}
					else
					{
						std::cout << g_BrightBlack;
					}
					std::cout << Tag->GetData();
				}
				if(Tag->GetFields().size() > 0)
				{
					throw std::exception();
				}
				if(Tag->GetTags().size() > 0)
				{
					std::cout << g_White << " {" << g_BrightBlack;
					
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
								std::cout << g_BrightRed;
							}
							std::cout << SubTag->GetName();
						}
						std::cout << g_BrightBlack;
						if((SubTag->GetName().empty() == false) && (SubTag->GetData().has_value() == true))
						{
							std::cout << '=';
						}
						if(SubTag->GetData().has_value() == true)
						{
							std::cout << SubTag->GetData();
						}
						if(SubTag->GetFields().size() > 0)
						{
							throw std::exception();
						}
						if(SubTag->GetTags().size() > 0)
						{
							std::cout << g_White << " {" << g_BrightBlack;
							
							auto FirstSubSubTag{true};
							
							for(auto SubSubTag : SubTag->GetTags())
							{
								if(FirstSubSubTag == false)
								{
									std::cout << ", ";
								}
								if(SubSubTag->GetName().empty() == false)
								{
									if(SubSubTag->GetName() == "error")
									{
										std::cout << g_BrightRed;
									}
									std::cout << SubSubTag->GetName();
								}
								std::cout << g_BrightBlack;
								if((SubSubTag->GetName().empty() == false) && (SubSubTag->GetData().has_value() == true))
								{
									std::cout << '=';
								}
								if(SubSubTag->GetData().has_value() == true)
								{
									std::cout << SubSubTag->GetData();
								}
								if(SubSubTag->GetFields().size() > 0)
								{
									throw std::exception();
								}
								if(SubSubTag->GetTags().size() > 0)
								{
									throw std::exception();
								}
								FirstSubSubTag = false;
							}
							std::cout << g_White << '}' << g_BrightBlack;
						}
						FirstSubTag = false;
					}
					std::cout << g_White << '}' << g_BrightBlack;
				}
				First = false;
			}
			std::cout << g_White << '}';
		}
		
		auto SubIndentation{Indentation};
		
		if(HeaderLine == true)
		{
			std::cout << std::endl;
			SubIndentation += "    ";
		}
		if(Value->GetFieldCount() > 0)
		{
			for(auto & SubValue : Value->GetFields())
			{
				PrintValue(SubValue, SubIndentation);
			}
		}
		std::cout << g_Reset;
	}
}

#endif
