#include <sstream>
#include <string>
#include <vector>

#include "assertion.h"
#include "output_operators.h"
#include "query.h"

std::vector<std::string> Inspection::SplitString(const std::string & String, char Delimiter)
{
	auto Result = std::vector<std::string>{};
	auto BracketLevel = 0;
	auto IsEscaped = false;
	auto Part = std::string{};
	
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
				ASSERTION(BracketLevel >= 0);
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

bool Inspection::EvaluateTestQuery(Inspection::Value * Value, const std::string & Query)
{
	auto QueryParts = Inspection::SplitString(Query, '/');
	auto Result = false;
	
	for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
	{
		auto QueryPart = QueryParts[Index];
		auto QueryPartSpecifications = SplitString(QueryPart, ':');
		
		if(QueryPartSpecifications[0] == "field")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetField(QueryPartSpecifications[1]);
			}
		}
		else if(QueryPartSpecifications[0] == "data")
		{
			auto Output = std::stringstream{};
			
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
			ASSERTION(QueryPartSpecifications.size() == 1);
			Result = Value->GetData().has_value();
		}
		else if(QueryPartSpecifications[0] == "is-data")
		{
			auto Output = std::stringstream{};
			
			Output << Value->GetData();
			Result = Output.str() == QueryPartSpecifications[1];
		}
		else
		{
			ASSERTION(false);
		}
	}
	
	return Result;
}
