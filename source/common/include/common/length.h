#ifndef INSPECTION_COMMON_LENGTH_H
#define INSPECTION_COMMON_LENGTH_H

#include <cstdint>
#include <sstream>
#include <string>

#include "assertion.h"

namespace Inspection
{
    class Length
    {
    public:
        Length() :
            Inspection::Length{0, 0}
        {
        }
        
        Length(Inspection::Length const & Other) :
            m_Bits{Other.m_Bits},
            m_Bytes{Other.m_Bytes}
        {
        }
        
        Length(std::uint64_t Bytes, std::uint64_t Bits) :
            m_Bits{Bits},
            m_Bytes{Bytes}
        {
            m_Normalize();
        }
        
        auto GetBytes() const -> std::uint64_t
        {
            return m_Bytes;
        }
        
        auto GetBits() const -> std::uint64_t
        {
            return m_Bits;
        }
        
        auto GetTotalBits() const -> std::uint64_t
        {
            return m_Bytes * 8 + m_Bits;
        }
        
        auto Reset() -> void
        {
            m_Bytes = 0;
            m_Bits = 0;
        }
        
        auto Set(std::uint64_t Bytes, std::uint64_t Bits) -> void
        {
            m_Bytes = Bytes;
            m_Bits = Bits;
            m_Normalize();
        }
        
        auto operator-(Inspection::Length const & Other) const -> Inspection::Length
        {
            ASSERTION(*this >= Other);
            
            auto Bits =  static_cast<std::uint8_t>(0);
            auto Bytes = m_Bytes - Other.m_Bytes;
            
            if(m_Bits >= Other.m_Bits)
            {
                Bits = m_Bits - Other.m_Bits;
            }
            else
            {
                Bits = (m_Bits + 8) - Other.m_Bits;
                Bytes -= 1;
            }
            
            return Inspection::Length{Bytes, Bits};
        }
        
        auto operator-=(Inspection::Length const & Other) -> Inspection::Length &
        {
            ASSERTION(*this >= Other);
            if(m_Bits >= Other.m_Bits)
            {
                m_Bits -= Other.m_Bits;
                m_Bytes -= Other.m_Bytes;
            }
            else
            {
                m_Bits += 8 - Other.m_Bits;
                m_Bytes -= Other.m_Bytes + 1;
            }
            
            return *this;
        }
        
        auto operator+(Inspection::Length const & Other) const -> Inspection::Length
        {
            return Inspection::Length{m_Bytes + Other.m_Bytes, m_Bits + Other.m_Bits};
        }
        
        auto operator+=(Inspection::Length const & Other) -> Inspection::Length &
        {
            m_Bytes += Other.m_Bytes;
            m_Bits += Other.m_Bits;
            m_Normalize();
            
            return *this;
        }
        
        auto operator=(Inspection::Length const & Other) -> Inspection::Length &
        {
            if(&Other != this)
            {
                m_Bytes  = Other.m_Bytes;
                m_Bits = Other.m_Bits;
            }
            
            return *this;
        }
        
        auto operator<=(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes < Other.m_Bytes) || ((m_Bytes == Other.m_Bytes) && (m_Bits <= Other.m_Bits));
        }
        
        auto operator<(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes < Other.m_Bytes) || ((m_Bytes == Other.m_Bytes) && (m_Bits < Other.m_Bits));
        }
        
        auto operator>=(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes > Other.m_Bytes) || ((m_Bytes == Other.m_Bytes) && (m_Bits >= Other.m_Bits));
        }
        
        auto operator>(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes > Other.m_Bytes) || ((m_Bytes == Other.m_Bytes) && (m_Bits > Other.m_Bits));
        }
        
        auto operator==(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes == Other.m_Bytes) && (m_Bits == Other.m_Bits);
        }
        
        auto operator!=(Inspection::Length const & Other) const -> bool
        {
            return (m_Bytes != Other.m_Bytes) || (m_Bits != Other.m_Bits);
        }
    private:
        auto m_Normalize() -> void
        {
            auto BytesInBits = m_Bits / 8;
            
            if(BytesInBits > 0)
            {
                m_Bits = m_Bits % 8;
                m_Bytes += BytesInBits;
            }
        }
        
        std::uint64_t m_Bits;
        std::uint64_t m_Bytes;
    };
}

#endif
