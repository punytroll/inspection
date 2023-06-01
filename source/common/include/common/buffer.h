#ifndef INSPECTION_COMMON_BUFFER_H
#define INSPECTION_COMMON_BUFFER_H

#include "length.h"

namespace Inspection
{
    class Buffer
    {
    public:
        Buffer(std::uint8_t const * Data, Inspection::Length const & Length) :
            m_Data{Data},
            m_Length{Length}
        {
        }
        
        ~Buffer()
        {
            m_Data = nullptr;
        }
        
        auto GetData() const -> std::uint8_t const *
        {
            return m_Data;
        }
        
        auto GetLength() const -> Inspection::Length const &
        {
            return m_Length;
        }
    private:
        std::uint8_t const * m_Data;
        Inspection::Length m_Length;
    };
}

#endif
