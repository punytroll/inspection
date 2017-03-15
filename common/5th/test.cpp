#include <cassert>
#include <iomanip>
#include <iostream>

#include "buffer.h"

void TestStartingPosition(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 0);
}

void Test8Bits(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// four bytes
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0xde})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0xad})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0xbe})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0xef})));
}

void Test8BitsOffsetOne(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// first bit
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 1);
	// three bytes
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0xbd})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0x5d})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get8Bits() == std::array< std::uint8_t, 1>({0x7d})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 1);
	// remaining seven bits
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 4);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 4);
	assert(Buffer.GetPosition().GetBits() == 0);
}

void Test1Bit(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// 'd'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'e'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'a'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'd'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'b'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'e'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'e'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'f'
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert(Buffer.GetPosition().GetBytes() == 4);
	assert(Buffer.GetPosition().GetBits() == 0);
}

void Test7Bits(void)
{
	std::uint8_t Data[8];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	Data[4] = 0xde;
	Data[5] = 0xad;
	Data[6] = 0xbe;
	Data[7] = 0xef;
	
	Inspection::Buffer Buffer{Data, 8};
	
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x6f})));
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x2b})));
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x37})));
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x6e})));
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 4);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x7e})));
	assert(Buffer.GetPosition().GetBytes() == 4);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x7a})));
	assert(Buffer.GetPosition().GetBytes() == 5);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x5b})));
	assert(Buffer.GetPosition().GetBytes() == 6);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert((Buffer.Get7Bits() == std::array< std::uint8_t, 1>({0x3e})));
	assert(Buffer.GetPosition().GetBytes() == 7);
	assert(Buffer.GetPosition().GetBits() == 0);
}

void Test24Bits(void)
{
	std::uint8_t Data[12];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	Data[4] = 0xde;
	Data[5] = 0xad;
	Data[6] = 0xbe;
	Data[7] = 0xef;
	Data[8] = 0xde;
	Data[9] = 0xad;
	Data[10] = 0xbe;
	Data[11] = 0xef;
	
	Inspection::Buffer Buffer{Data, 12};
	
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xde, 0xad, 0xbe})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xef, 0xde, 0xad})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xbe, 0xef, 0xde})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xad, 0xbe, 0xef})));
}

void Test24BitsWithOffsets(void)
{
	std::uint8_t Data[12];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	Data[4] = 0xde;
	Data[5] = 0xad;
	Data[6] = 0xbe;
	Data[7] = 0xef;
	Data[8] = 0xde;
	Data[9] = 0xad;
	Data[10] = 0xbe;
	Data[11] = 0xef;
	
	Inspection::Buffer Buffer{Data, 12};
	
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xbd, 0x5b, 0x7d})));
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xbf, 0x7a, 0xb6})));
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x01})));
	assert((Buffer.Get24Bits() == std::array< std::uint8_t, 3 >({0xf7, 0x7e, 0xf5})));
	assert((Buffer.Get1Bit() == std::array< std::uint8_t, 1>({0x00})));
}

int main(int argc, char ** argv)
{
	TestStartingPosition();
	Test8Bits();
	Test1Bit();
	Test8BitsOffsetOne();
	Test7Bits();
	Test24Bits();
	Test24BitsWithOffsets();
	
	return 0;
}
