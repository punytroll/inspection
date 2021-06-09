#include <cassert>
#include <sstream>
#include <string>
#include <vector>

#include "output_operators.h"
#include "query.h"

std::vector< std::string > Inspection::SplitString(const std::string & String, char Delimiter)
{
	std::vector< std::string > Result;
	auto BracketLevel{0};
	auto IsEscaped{false};
	std::string Part;
	
	for(auto Character : String)
	{
		if(Character == Delimiter)
		{
			if(IsEscaped == false)
			{
				if(BracketLevel == 0)
				{
					Result.push_back(Part);
					Part = "";
				}
				else
				{
					Part += Character;
				}
			}
			else
			{
				Part += Character;
				IsEscaped = false;
			}
		}
		else if(Character == '\\')
		{
			if(IsEscaped == true)
			{
				Part += Character;
			}
			IsEscaped = !IsEscaped;
		}
		else if(Character == '[')
		{
			Part += Character;
			if(IsEscaped == true)
			{
				IsEscaped = false;
			}
			else
			{
				BracketLevel += 1;
			}
		}
		else if(Character == ']')
		{
			Part += Character;
			if(IsEscaped == true)
			{
				IsEscaped = false;
			}
			else
			{
				assert(BracketLevel >= 0);
				BracketLevel -= 1;
			}
		}
		else
		{
			Part += Character;
		}
	}
	Result.push_back(Part);
	
	return Result;
}

bool Inspection::EvaluateTestQuery(std::shared_ptr< Inspection::Value > Value, const std::string & Query)
{
	auto QueryParts{Inspection::SplitString(Query, '/')};
	auto Result{false};
	
	for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
	{
		auto QueryPart{QueryParts[Index]};
		auto QueryPartSpecifications{SplitString(QueryPart, ':')};
		
		if(QueryPartSpecifications[0] == "field")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetField(QueryPartSpecifications[1]);
			}
		}
		else if(QueryPartSpecifications[0] == "data")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			Result = Output.str() == "true";
		}
		else if(QueryPartSpecifications[0] == "tag")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetTag(QueryPartSpecifications[1]);
			}
		}
		else if(QueryPartSpecifications[0] == "has-tag")
		{
			Result = Value->HasTag(QueryPartSpecifications[1]);
		}
		else if(QueryPartSpecifications[0] == "has-field")
		{
			Result = Value->HasField(QueryPartSpecifications[1]);
		}
		else if(QueryPartSpecifications[0] == "has-data")
		{
			return Value->GetData().has_value();
		}
		else if(QueryPartSpecifications[0] == "is-data")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			Result = Output.str() == QueryPartSpecifications[1];
		}
		else
		{
			assert(false);
		}
	}
	
	return Result;
}
