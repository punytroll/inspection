#ifndef COMMON_LENGTH_H
#define COMMON_LENGTH_H

#include <cstdint>

namespace Inspection
{
	class Length
	{
	public:
		Length(void) :
			Length(0ull, 0)
		{
		}
		
		Length(std::uint64_t Bytes) :
			Length(Bytes, 0)
		{
		}
		
		Length(std::uint64_t Bytes, std::uint8_t Bits) :
			_Bits(Bits),
			_Bytes(Bytes)
		{
			_Normalize();
		}
		
		std::uint64_t GetBytes(void) const
		{
			return _Bytes;
		}
		
		std::uint8_t GetBits(void) const
		{
			return _Bits;
		}
		
		void Reset(void)
		{
			_Bytes = 0ull;
			_Bits = 0;
		}
		
		void Set(std::uint64_t Bytes, std::uint8_t Bits)
		{
			_Bytes = Bytes;
			_Bits = Bits;
		}
		
		Length operator+(const Length & Length) const
		{
			return Inspection::Length(_Bytes + Length._Bytes, _Bits + Length._Bits);
		}
		
		Length & operator+=(const Length & Length)
		{
			_Bytes += Length._Bytes;
			_Bits += Length._Bits;
			_Normalize();
			
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
		
		bool operator==(const Length & Length) const
		{
			return (_Bytes == Length._Bytes) && (_Bits == Length._Bits);
		}
	private:
		void _Normalize(void)
		{
			while(_Bits >= 8)
			{
				_Bits -= 8;
				_Bytes += 1;
			}
		}
		
		std::uint8_t _Bits;
		std::uint64_t _Bytes;
	};
}

#endif
