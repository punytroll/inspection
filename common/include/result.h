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
		Result(const Inspection::Length & Offset) :
			_Offset(Offset),
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
		
		const std::list< std::shared_ptr< Inspection::Value > > & GetValues(void)
		{
			return _Value->GetValues();
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
			throw std::invalid_argument("Unknown sub value \"" + Name + "\".");
		}
		
		const Inspection::Length & GetLength(void) const
		{
			assert(_Value != nullptr);
			
			return _Value->GetLength();
		}
		
		const Inspection::Length & GetOffset(void) const
		{
			return _Offset;
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
		Inspection::Length _Offset;
		bool _Success;
		std::shared_ptr< Value > _Value;
	};
	
	inline std::unique_ptr< Inspection::Result > InitializeResult(const Inspection::Buffer & Buffer)
	{
		return std::make_unique< Inspection::Result >(Buffer.GetPosition());
	}
	
	inline std::unique_ptr< Inspection::Result > InitializeResult(const Inspection::Reader & Reader)
	{
		return std::make_unique< Inspection::Result >(Reader.GetPositionInBuffer());
	}
	
	inline void FinalizeResult(std::unique_ptr< Inspection::Result > & Result, const Inspection::Buffer & Buffer)
	{
		Result->GetValue()->SetOffset(Result->GetOffset());
		Result->GetValue()->SetLength(Buffer.GetPosition() - Result->GetValue()->GetOffset());
	}
	
	inline void FinalizeResult(std::unique_ptr< Inspection::Result > & Result, const Inspection::Reader & Reader)
	{
		Result->GetValue()->SetOffset(Result->GetOffset());
		Result->GetValue()->SetLength(Reader.GetPositionInBuffer() - Result->GetValue()->GetOffset());
	}
}

#endif
