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
        Result() :
            m_Success{false},
            m_Value{std::make_unique<Inspection::Value>()}
        {
        }
        
        ~Result()
        {
        }
        
        auto GetSuccess() const -> bool
        {
            return m_Success;
        }
        
        auto ExtractValue() -> std::unique_ptr<Inspection::Value>
        {
            return std::move(m_Value);
        }
        
        auto GetValue() -> Inspection::Value *
        {
            return m_Value.get();
        }
        
        auto SetSuccess(bool Success) -> void
        {
            m_Success = Success;
        }
    private:
        bool m_Success;
        std::unique_ptr<Inspection::Value> m_Value;
    };
}

#endif
