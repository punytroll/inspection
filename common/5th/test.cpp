#include <cassert>

#include "buffer.h"
#include "getters.h"

std::uint8_t g_Data[16];
Inspection::Buffer * g_Buffer(nullptr);

void SetupDataAndBuffer(void)
{
	g_Data[0] = 0xde;
	g_Data[1] = 0xad;
	g_Data[2] = 0xbe;
	g_Data[3] = 0xef;
	g_Data[4] = 0xde;
	g_Data[5] = 0xad;
	g_Data[6] = 0xbe;
	g_Data[7] = 0xef;
	g_Data[8] = 0xde;
	g_Data[9] = 0xad;
	g_Data[10] = 0xbe;
	g_Data[11] = 0xef;
	g_Data[12] = 0xde;
	g_Data[13] = 0xad;
	g_Data[14] = 0xbe;
	g_Data[15] = 0xef;
	delete g_Buffer;
	g_Buffer = new Inspection::Buffer(g_Data, 16);
}

void TestStartingPosition(void)
{
	SetupDataAndBuffer();
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test8Bits(void)
{
	SetupDataAndBuffer();
	// four bytes
	assert(g_Buffer->Get8Bits() == 0xde);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	assert(g_Buffer->Get8Bits() == 0xad);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	assert(g_Buffer->Get8Bits() == 0xbe);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	assert(g_Buffer->Get8Bits() == 0xef);
}

void Test8BitsOffsetOne(void)
{
	SetupDataAndBuffer();
	// first bit
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	// three bytes
	assert(g_Buffer->Get8Bits() == 0xbd);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get8Bits() == 0x5b);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get8Bits() == 0x7d);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	// remaining seven bits
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 4);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test1Bit(void)
{
	SetupDataAndBuffer();
	// 'd'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'e'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'a'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'd'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'b'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'e'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'e'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bit() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'f'
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bit() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 4);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test7Bits(void)
{
	SetupDataAndBuffer();
	assert(g_Buffer->Get7Bits() == 0x6f);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get7Bits() == 0x2b);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get7Bits() == 0x37);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get7Bits() == 0x6e);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	assert(g_Buffer->Get7Bits() == 0x7e);
	assert(g_Buffer->GetPosition().GetBytes() == 4);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get7Bits() == 0x7a);
	assert(g_Buffer->GetPosition().GetBytes() == 5);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get7Bits() == 0x5b);
	assert(g_Buffer->GetPosition().GetBytes() == 6);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get7Bits() == 0x3e);
	assert(g_Buffer->GetPosition().GetBytes() == 7);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test_Get_SignedInteger_32Bit_BigEndian(void)
{
	SetupDataAndBuffer();
	
	auto Result{Get_SignedInteger_32Bit_BigEndian(*g_Buffer)};
	
	assert(Result->GetSuccess() == true);
	assert(std::experimental::any_cast< std::int32_t >(Result->GetAny()) == -559038737);
}

void Test_Get_SignedInteger_32Bit_LittleEndian(void)
{
	SetupDataAndBuffer();
	
	auto Result{Get_SignedInteger_32Bit_LittleEndian(*g_Buffer)};
	
	assert(Result->GetSuccess() == true);
	assert(std::experimental::any_cast< std::int32_t >(Result->GetAny()) == -272716322);
}

int main(int argc, char ** argv)
{
	TestStartingPosition();
	Test8Bits();
	Test1Bit();
	Test8BitsOffsetOne();
	Test7Bits();
	Test_Get_SignedInteger_32Bit_BigEndian();
	Test_Get_SignedInteger_32Bit_LittleEndian();
	
	return 0;
}
