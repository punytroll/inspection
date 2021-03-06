#ifndef INSPECTION_COMMON_LENGTH_H
#define INSPECTION_COMMON_LENGTH_H

#include <cassert>
#include <cstdint>
#include <sstream>
#include <string>

#include "string_cast.h"

namespace Inspection
{
	class Length
	{
	public:
		Length(void) :
			Length{0, 0}
		{
		}
		
		Length(const Length & Length) :
			_Bits{Length._Bits},
			_Bytes{Length._Bytes}
		{
		}
		
		Length(std::uint64_t Bytes, std::uint64_t Bits) :
			_Bits{Bits},
			_Bytes{Bytes}
		{
			_Normalize();
		}
		
		std::uint64_t GetBytes(void) const
		{
			return _Bytes;
		}
		
		std::uint64_t GetBits(void) const
		{
			return _Bits;
		}
		
		std::uint64_t GetTotalBits(void) const
		{
			return _Bytes * 8 + _Bits;
		}
		
		void Reset(void)
		{
			_Bytes = 0;
			_Bits = 0;
		}
		
		void Set(std::uint64_t Bytes, std::uint64_t Bits)
		{
			_Bytes = Bytes;
			_Bits = Bits;
			_Normalize();
		}
		
		Length operator-(const Length & Length) const
		{
			assert(*this >= Length);
			
			std::uint8_t Bits;
			std::uint64_t Bytes{_Bytes - Length._Bytes};
			
			if(_Bits >= Length._Bits)
			{
				Bits = _Bits - Length._Bits;
			}
			else
			{
				Bits = (_Bits + 8) - Length._Bits;
				Bytes -= 1;
			}
			
			return Inspection::Length{Bytes, Bits};
		}
		
		Length & operator-=(const Length & Length)
		{
			assert(*this >= Length);
			if(_Bits >= Length._Bits)
			{
				_Bits -= Length._Bits;
				_Bytes -= Length._Bytes;
			}
			else
			{
				_Bits += 8 - Length._Bits;
				_Bytes -= Length._Bytes + 1;
			}
			return *this;
		}
		
		Length operator+(const Length & Length) const
		{
			return Inspection::Length{_Bytes + Length._Bytes, _Bits + Length._Bits};
		}
		
		Length & operator+=(const Length & Length)
		{
			_Bytes += Length._Bytes;
			_Bits += Length._Bits;
			_Normalize();
			
			return *this;
		}
		
		Length & operator=(const Length & Length)
		{
			if(&Length != this)
			{
				_Bytes  = Length._Bytes;
				_Bits = Length._Bits;
			}
			
			return *this;
		}
		
		bool operator<=(const Length & Length) const
		{
			return (_Bytes < Length._Bytes) || ((_Bytes == Length._Bytes) && (_Bits <= Length._Bits));
		}
		
		bool operator<(const Length & Length) const
		{
			return (_Bytes < Length._Bytes) || ((_Bytes == Length._Bytes) && (_Bits < Length._Bits));
		}
		
		bool operator>=(const Length & Length) const
		{
			return (_Bytes > Length._Bytes) || ((_Bytes == Length._Bytes) && (_Bits >= Length._Bits));
		}
		
		bool operator>(const Length & Length) const
		{
			return (_Bytes > Length._Bytes) || ((_Bytes == Length._Bytes) && (_Bits > Length._Bits));
		}
		
		bool operator==(const Length & Length) const
		{
			return (_Bytes == Length._Bytes) && (_Bits == Length._Bits);
		}
		
		bool operator!=(const Length & Length) const
		{
			return (_Bytes != Length._Bytes) || (_Bits != Length._Bits);
		}
	private:
		void _Normalize(void)
		{
			auto BytesInBits{_Bits / 8};
			
			if(BytesInBits > 0)
			{
				_Bits = _Bits % 8;
				_Bytes += BytesInBits;
			}
		}
		
		std::uint64_t _Bits;
		std::uint64_t _Bytes;
	};
}

#endif
