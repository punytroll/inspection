#include <cassert>

#include "buffer.h"
#include "getters.h"

std::uint8_t g_Data[16];
Inspection::Buffer * g_Buffer(nullptr);

void SetupDataAndBufferDEADBEEF(void)
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

void SetupDataAndBufferVorbis(void)
{
	g_Data[0] = 0xfc;
	g_Data[1] = 0x48;
	g_Data[2] = 0xce;
	g_Data[3] = 0x06;
	g_Data[4] = 0x00;
	g_Data[5] = 0x00;
	g_Data[6] = 0x00;
	g_Data[7] = 0x00;
	g_Data[8] = 0x00;
	g_Data[9] = 0x00;
	g_Data[10] = 0x00;
	g_Data[11] = 0x00;
	g_Data[12] = 0x00;
	g_Data[13] = 0x00;
	g_Data[14] = 0x00;
	g_Data[15] = 0x00;
	delete g_Buffer;
	g_Buffer = new Inspection::Buffer(g_Data, Inspection::Length(0ull, 27));
	g_Buffer->SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
}

void TestStartingPosition(void)
{
	SetupDataAndBufferDEADBEEF();
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test8Bits(void)
{
	SetupDataAndBufferDEADBEEF();
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
	SetupDataAndBufferDEADBEEF();
	// first bit
	assert(g_Buffer->Get1Bits() == 0x01);
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
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 4);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test1Bit(void)
{
	SetupDataAndBufferDEADBEEF();
	// 'd'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'e'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 0);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'a'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'd'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 1);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'b'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'e'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 2);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 0);
	// 'e'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 1);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 2);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 3);
	assert(g_Buffer->Get1Bits() == 0x00);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 4);
	// 'f'
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 5);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 6);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 3);
	assert(g_Buffer->GetPosition().GetBits() == 7);
	assert(g_Buffer->Get1Bits() == 0x01);
	assert(g_Buffer->GetPosition().GetBytes() == 4);
	assert(g_Buffer->GetPosition().GetBits() == 0);
}

void Test7Bits(void)
{
	SetupDataAndBufferDEADBEEF();
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
	SetupDataAndBufferDEADBEEF();
	
	auto FieldReader{Inspection::Reader{*g_Buffer, Inspection::Length{0, 32}}};
	auto FieldResult{Get_SignedInteger_32Bit_BigEndian(FieldReader)};
	
	assert(FieldResult->GetSuccess() == true);
	assert(std::experimental::any_cast< std::int32_t >(FieldResult->GetAny()) == -559038737);
}

void Test_Get_SignedInteger_32Bit_LittleEndian(void)
{
	SetupDataAndBufferDEADBEEF();
	
	auto FieldReader{Inspection::Reader{*g_Buffer, Inspection::Length{0, 32}}};
	auto FieldResult{Get_SignedInteger_32Bit_LittleEndian(FieldReader)};
	
	assert(FieldResult->GetSuccess() == true);
	assert(std::experimental::any_cast< std::int32_t >(FieldResult->GetAny()) == -272716322);
}

void Test_Vorbis_Buffer(void)
{
	SetupDataAndBufferVorbis();
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 0));
	assert(g_Buffer->Get4Bits() == 0x0c);
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 4));
	assert(g_Buffer->Get3Bits() == 0x07);
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 7));
	assert(g_Buffer->Get7Bits() == 0x11);
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 14));
	assert(g_Buffer->Get8Bits() == 0x39);
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 22));
	assert(g_Buffer->Get5Bits() == 0x1b);
	assert(g_Buffer->GetPosition() == Inspection::Length(0ull, 27));
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
	Test_Vorbis_Buffer();
	
	return 0;
}
