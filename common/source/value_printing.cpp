#include <iostream>
#include <string>

#include "colors.h"
#include "output_operators.h"
#include "value.h"
#include "value_printing.h"

static void PrintValue(Inspection::Value & Value, const std::string & Indentation)
{
	auto HeaderLine = (Value.GetName().empty() == false) || (Value.GetData().has_value() == true) || (Value.GetTags().empty() == false);
	
	if(HeaderLine == true)
	{
		std::cout << Inspection::g_White << Indentation;
	}
	if(Value.GetName().empty() == false)
	{
		if(Value.GetName() == "error")
		{
			std::cout << Inspection::g_BrightRed;
		}
		else
		{
			std::cout << Inspection::g_BrightWhite;
		}
		std::cout << Value.GetName();
	}
	if((Value.GetName().empty() == false) && (Value.GetData().has_value() == true))
	{
		std::cout << Inspection::g_White << ": ";
	}
	if(Value.GetData().has_value() == true)
	{
		if((Value.GetName().empty() == false) && (Value.GetName() == "error"))
		{
			std::cout << Inspection::g_BrightWhite;
		}
		else
		{
			std::cout << Inspection::g_BrightCyan;
		}
		Inspection::operator<<(std::cout, Value.GetData());
	}
	if(Value.GetTags().empty() == false)
	{
		auto First = true;
		
		std::cout << Inspection::g_White <<" {" << Inspection::g_BrightBlack;
		for(auto & Tag : Value.GetTags())
		{
			if(First == false)
			{
				std::cout << Inspection::g_White << ", " << Inspection::g_BrightBlack;
			}
			if(Tag->GetName().empty() == false)
			{
				if(Tag->GetName() == "error")
				{
					std::cout << Inspection::g_BrightRed;
				}
				else if(Tag->GetData().has_value() == false)
				{
					std::cout << Inspection::g_BrightBlack;
				}
				else
				{
					std::cout << Inspection::g_Yellow;
				}
				std::cout << Tag->GetName();
			}
			if((Tag->GetName().empty() == false) && (Tag->GetData().has_value() == true))
			{
				std::cout << Inspection::g_White << '=';
			}
			if(Tag->GetData().has_value() == true)
			{
				if(Tag->GetData().type() == typeid(nullptr))
				{
					std::cout << Inspection::g_Green;
				}
				else
				{
					std::cout << Inspection::g_BrightBlack;
				}
				Inspection::operator<<(std::cout, Tag->GetData());
			}
			if(Tag->GetFields().size() > 0)
			{
				throw std::exception();
			}
			if(Tag->GetTags().size() > 0)
			{
				std::cout << Inspection::g_White << " {" << Inspection::g_BrightBlack;
				
				auto FirstSubTag = true;
				
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
							std::cout << Inspection::g_BrightRed;
						}
						std::cout << SubTag->GetName();
					}
					std::cout << Inspection::g_BrightBlack;
					if((SubTag->GetName().empty() == false) && (SubTag->GetData().has_value() == true))
					{
						std::cout << '=';
					}
					if(SubTag->GetData().has_value() == true)
					{
						Inspection::operator<<(std::cout, SubTag->GetData());
					}
					if(SubTag->GetFields().size() > 0)
					{
						throw std::exception();
					}
					if(SubTag->GetTags().size() > 0)
					{
						std::cout << Inspection::g_White << " {" << Inspection::g_BrightBlack;
						
						auto FirstSubSubTag = true;
						
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
									std::cout << Inspection::g_BrightRed;
								}
								std::cout << SubSubTag->GetName();
							}
							std::cout << Inspection::g_BrightBlack;
							if((SubSubTag->GetName().empty() == false) && (SubSubTag->GetData().has_value() == true))
							{
								std::cout << '=';
							}
							if(SubSubTag->GetData().has_value() == true)
							{
								Inspection::operator<<(std::cout, SubSubTag->GetData());
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
						std::cout << Inspection::g_White << '}' << Inspection::g_BrightBlack;
					}
					FirstSubTag = false;
				}
				std::cout << Inspection::g_White << '}' << Inspection::g_BrightBlack;
			}
			First = false;
		}
		std::cout << Inspection::g_White << '}';
	}
	
	auto SubIndentation = Indentation;
	
	if(HeaderLine == true)
	{
		std::cout << std::endl;
		SubIndentation += "    ";
	}
	if(Value.GetFieldCount() > 0)
	{
		for(auto & SubValue : Value.GetFields())
		{
			PrintValue(*SubValue, SubIndentation);
		}
	}
	std::cout << Inspection::g_Reset;
}

void Inspection::PrintValue(Inspection::Value & Value)
{
	::PrintValue(Value, "");
}
