#ifndef INSPECTION_COMMON_RESULT_H
#define INSPECTION_COMMON_RESULT_H

#include <utility>

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
			_Value(std::make_unique<Inspection::Value>())
		{
		}
		
		~Result(void)
		{
		}
		
		bool GetSuccess(void) const
		{
			return _Success;
		}
		
		std::unique_ptr<Inspection::Value> ExtractValue(void)
		{
			return std::move(_Value);
		}
		
		Inspection::Value * GetValue(void)
		{
			return _Value.get();
		}
		
		void SetSuccess(bool Success)
		{
			_Success = Success;
		}
		
		void SetValue(std::unique_ptr<Inspection::Value> Value)
		{
			_Value = std::move(Value);
		}
	private:
		bool _Success;
		std::unique_ptr<Inspection::Value> _Value;
	};
}

#endif
