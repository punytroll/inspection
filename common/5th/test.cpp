#include <cassert>
#include <iomanip>
#include <iostream>

#include "buffer.h"

void TestStartingPositionInDEADBEEF(void)
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

void TestBytesInDEADBEEF(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// four bytes
	assert(Buffer.Get1Byte() == 0xde);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert(Buffer.Get1Byte() == 0xad);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert(Buffer.Get1Byte() == 0xbe);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 0);
	assert(Buffer.Get1Byte() == 0xef);
}

void TestBytesOffsetOneInDEADBEEF(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// first bit
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 1);
	// three bytes
	assert(Buffer.Get1Byte() == 0xbd);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Byte() == 0x5b);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Byte() == 0x7d);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 1);
	// remaining seven bits
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 4);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 4);
	assert(Buffer.GetPosition().GetBits() == 0);
}

void TestBitsInDEADBEEF(void)
{
	std::uint8_t Data[4];
	
	Data[0] = 0xde;
	Data[1] = 0xad;
	Data[2] = 0xbe;
	Data[3] = 0xef;
	
	Inspection::Buffer Buffer{Data, 4};
	
	// 'd'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'e'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 0);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'a'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'd'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 1);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'b'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'e'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 2);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 0);
	// 'e'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 1);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 2);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 3);
	assert(Buffer.Get1Bit() == 0x00);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 4);
	// 'f'
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 5);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 6);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 3);
	assert(Buffer.GetPosition().GetBits() == 7);
	assert(Buffer.Get1Bit() == 0x01);
	assert(Buffer.GetPosition().GetBytes() == 4);
	assert(Buffer.GetPosition().GetBits() == 0);
}

int main(int argc, char ** argv)
{
	TestStartingPositionInDEADBEEF();
	TestBytesInDEADBEEF();
	TestBitsInDEADBEEF();
	TestBytesOffsetOneInDEADBEEF();
	
	return 0;
}
