#ifndef COMMON_5TH_RESULT_H
#define COMMON_5TH_RESULT_H

#include <iostream>

#include "length.h"
#include "value.h"

namespace Inspection
{
	class Result
	{
	public:
		Result(void) :
			_Success(false)
		{
		}
		
		Result(bool Success, const Inspection::Length & Offset) :
			_Offset(Offset),
			_Success(Success),
			_Value(Inspection::MakeValue(""))
		{
		}
		
		Result(bool Success, std::shared_ptr< Value > Value) :
			_Success(Success),
			_Value(Value)
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
			
			throw new std::exception();
		}
		
		const std::experimental::any & GetAny(void)
		{
			return _Value->GetAny();
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
			
			throw new std::exception();
		}
		
		const Inspection::Length & GetOffset(void) const
		{
			return _Offset;
		}
		
		void SetSuccess(bool Success)
		{
			_Success = Success;
		}
		
		void SetValue(std::shared_ptr< Value > Value)
		{
			_Value = Value;
		}
	private:
		Inspection::Length _Offset;
		bool _Success;
		std::shared_ptr< Value > _Value;
	};
	
	inline std::unique_ptr< Inspection::Result > InitializeResult(bool Success, const Inspection::Buffer & Buffer)
	{
		return std::make_unique< Inspection::Result >(Success, Buffer.GetPosition());
	}
	
	inline void FinalizeResult(std::unique_ptr< Inspection::Result > & Result, const Inspection::Buffer & Buffer)
	{
		if(Result->GetSuccess() == true)
		{
			Result->GetValue()->SetOffset(Result->GetOffset());
			Result->GetValue()->SetLength(Buffer.GetPosition() - Result->GetValue()->GetOffset());
		}
	}
}

#endif
