#ifndef COMMON_VALUES_H
#define COMMON_VALUES_H

#include <experimental/any>
#include <map>
#include <string>

class Values
{
public:
	void Add(const std::string & Name, const std::experimental::any & Value)
	{
		if(_Values.count(Name) == 0)
		{
			_Values[Name] = Value;
		}
		else
		{
			throw std::exception();
		}
	}
	
	const std::experimental::any & Get(const std::string & Name)
	{
		auto Iterator{_Values.find(Name)};
		
		if(Iterator != _Values.end())
		{
			return Iterator->second;
		}
		else
		{
			throw std::exception();
		}
	}
	
	bool Has(const std::string & Name)
	{
		return _Values.count(Name) == 1;
	}
	
	void Remove(const std::string & Name)
	{
		_Values.erase(Name);
	}
	
	void Replace(const std::string & Name, const std::experimental::any & Value)
	{
		auto Iterator{_Values.find(Name)};
		
		if(Iterator != _Values.end())
		{
			if(std::type_index(Value.type()) != std::type_index(Iterator->second.type()))
			{
				Iterator->second = Value;
			}
			else
			{
				throw std::exception();
			}
		}
		else
		{
			throw std::exception();
		}
	}
	
	void Update(const std::string & Name, const std::experimental::any & Value)
	{
		auto Iterator{_Values.find(Name)};
		
		if(Iterator != _Values.end())
		{
			if(std::type_index(Value.type()) == std::type_index(Iterator->second.type()))
			{
				Iterator->second = Value;
			}
			else
			{
				throw std::exception();
			}
		}
		else
		{
			throw std::exception();
		}
	}
private:
	std::map< std::string, std::experimental::any > _Values;
};

#endif
