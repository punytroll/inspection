#ifndef INSPECTION_COMMON_RESULT_H
#define INSPECTION_COMMON_RESULT_H

#include "buffer.h"
#include "length.h"
#include "reader.h"
#include "value.h"

namespace Inspection
{
	class Result
	{
	public:
		Result(void) :
			_Success(false),
			_Value(std::make_shared< Inspection::Value >())
		{
		}
		
		~Result(void)
		{
		}
		
		bool GetSuccess(void) const
		{
			return _Success;
		}
		
		std::shared_ptr< Value > & GetValue(void)
		{
			return _Value;
		}
		
		const std::shared_ptr< Value > & GetValue(const std::string & Name)
		{
			for(auto & Value : _Value->GetValues())
			{
				if(Value->GetName() == Name)
				{
					return Value;
				}
			}
			throw std::invalid_argument("Unknown sub value \"" + Name + "\".");
		}
		
		const std::experimental::any & GetAny(const std::string & Name)
		{
			for(auto & Value : _Value->GetValues())
			{
				if(Value->GetName() == Name)
				{
					return Value->GetAny();
				}
			}
			throw std::invalid_argument("Unknown sub value \"" + Name + "\".");
		}
		
		void SetSuccess(bool Success)
		{
			_Success = Success;
		}
		
		std::shared_ptr< Value > SetValue(std::shared_ptr< Value > Value)
		{
			_Value = Value;
			
			return _Value;
		}
	private:
		bool _Success;
		std::shared_ptr< Value > _Value;
	};
	
	inline std::unique_ptr< Inspection::Result > InitializeResult(const Inspection::Buffer & Buffer)
	{
		return std::make_unique< Inspection::Result >();
	}
	
	inline std::unique_ptr< Inspection::Result > InitializeResult(const Inspection::Reader & Reader)
	{
		return std::make_unique< Inspection::Result >();
	}
	
	inline void FinalizeResult(std::unique_ptr< Inspection::Result > & Result, const Inspection::Buffer & Buffer)
	{
	}
	
	inline void FinalizeResult(std::unique_ptr< Inspection::Result > & Result, const Inspection::Reader & Reader)
	{
	}
}

#endif
