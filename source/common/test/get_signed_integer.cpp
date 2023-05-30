#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_1Bit                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_1Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x85};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_1Bit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_1Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -0x01);
        
        auto Result2 = Inspection::Get_SignedInteger_1Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == 0x00);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_5Bit                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_5Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -16);
        
        auto Result2 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -9);
        
        auto Result3 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result3->GetValue()->GetData()) == 1);
        
        auto Result4 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result4->GetValue()->GetData()) == -12);
        
        auto Result5 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result5->GetValue()->GetData()) == -2);
        
        auto Result6 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result6->GetValue()->GetData()) == 4);
        
        auto Result7 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result7->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result7->GetValue()->GetData()) == 8);
        
        auto Result8 = Inspection::Get_SignedInteger_5Bit(Reader, {});
        
        assert(Result8->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result8->GetValue()->GetData()) == 15);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_7Bit                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_7Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -62);
        
        auto Result2 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -16);
        
        auto Result3 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result3->GetValue()->GetData()) == -23);
        
        auto Result4 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result4->GetValue()->GetData()) == -15);
        
        auto Result5 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result5->GetValue()->GetData()) == 8);
        
        auto Result6 = Inspection::Get_SignedInteger_7Bit(Reader, {});
        
        assert(Result6->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_8Bit                                                   //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_8Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x64, 0xc3};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 5}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_8Bit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_8Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == 0x64);
        
        auto Result2 = Inspection::Get_SignedInteger_8Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -0x3d);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_9Bit_BigEndian                                         //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_9Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 1}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == -245);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -245);
        
        auto Result2 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == -243);
        
        auto Result3 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == 120);
        
        auto Result4 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == -240);
        
        auto Result5 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == -19);
        
        auto Result6 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result6->GetValue()->GetData()) == -172);
        
        auto Result7 = Inspection::Get_SignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_10Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_10Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == -489);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{9, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -489);
        
        auto Result2 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == 52);
        
        auto Result3 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == -60);
        
        auto Result4 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == 271);
        
        auto Result5 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == 437);
        
        auto Result6 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result6->GetValue()->GetData()) == 286);
        
        auto Result7 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result7->GetValue()->GetData()) == -342);
        
        auto Result8 = Inspection::Get_SignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result8->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_11Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_11Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == -978);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{9, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -978);
        
        auto Result2 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == 211);
        
        auto Result3 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == -478);
        
        auto Result4 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == 246);
        
        auto Result5 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == -344);
        
        auto Result6 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result6->GetValue()->GetData()) == -86);
        
        auto Result7 = Inspection::Get_SignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_12Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_12Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == -1956);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -1956);
        
        auto Result2 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == 847);
        
        auto Result3 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == 272);
        
        auto Result4 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == -147);
        
        auto Result5 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == 1310);
        
        auto Result6 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result6->GetValue()->GetData()) == -1367);
        
        auto Result7 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result7->GetValue()->GetData()) == 843);
        
        auto Result8 = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result8->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_14Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_14Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 6}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == -7824);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -7824);
        
        auto Result2 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == -2831);
        
        auto Result3 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == 1085);
        
        auto Result4 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == -4783);
        
        auto Result5 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == -1366);
        
        auto Result6 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result6->GetValue()->GetData()) == 4939);
        
        auto Result7 = Inspection::Get_SignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_15Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_15Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -15647);
        
        auto Result2 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == -11324);
        
        auto Result3 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == 8685);
        
        auto Result4 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == -10978);
        
        auto Result5 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == -10935);
        
        auto Result6 = Inspection::Get_SignedInteger_15Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_16Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_16Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result1->GetValue()->GetData()) == -31293);
        
        auto Result2 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result2->GetValue()->GetData()) == 20241);
        
        auto Result3 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result3->GetValue()->GetData()) == 3949);
        
        auto Result4 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result4->GetValue()->GetData()) == 20970);
        
        auto Result5 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int16_t>(Result5->GetValue()->GetData()) == -22220);
        
        auto Result6 = Inspection::Get_SignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_17Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_17Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -62586);
        
        auto Result2 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == -50108);
        
        auto Result3 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == 31594);
        
        auto Result4 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == -57686);
        
        auto Result5 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result5->GetValue()->GetData()) == -55658);
        
        auto Result6 = Inspection::Get_SignedInteger_17Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_18Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_18Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -125171);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -125171);
        
        auto Result2 = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == 61712);
        
        auto Result3 = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == -9388);
        
        auto Result4 = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == 125609);
        
        auto Result5 = Inspection::Get_SignedInteger_18Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_19Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_19Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -250342);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -250342);
        
        auto Result2 = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == 246851);
        
        auto Result3 = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == -75101);
        
        auto Result4 = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == -87405);
        
        auto Result5 = Inspection::Get_SignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_20Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_20Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -500684);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -500684);
        
        auto Result2 = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == -61169);
        
        auto Result3 = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == 447774);
        
        auto Result4 = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == -349900);
        
        auto Result5 = Inspection::Get_SignedInteger_20Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_21Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_21Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 5}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -1001367);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -1001367);
        
        auto Result2 = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == -244675);
        
        auto Result3 = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == -612107);
        
        auto Result4 = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == 693067);
        
        auto Result5 = Inspection::Get_SignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_22Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_22Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2, 0x90};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 6}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -2002733);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{12, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -2002733);
        
        auto Result2 = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == -978698);
        
        auto Result3 = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == -702550);
        
        auto Result4 = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == -1493838);
        
        auto Result5 = Inspection::Get_SignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_23Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_23Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2, 0x90};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 7}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result->GetValue()->GetData()) == -4005465);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{12, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -4005465);
        
        auto Result2 = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == -3914789);
        
        auto Result3 = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == 2768213);
        
        auto Result4 = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result4->GetValue()->GetData()) == 1264425);
        
        auto Result5 = Inspection::Get_SignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_24Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_24Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_24Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -8010929);
        
        auto Result2 = Inspection::Get_SignedInteger_24Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == 1118061);
        
        auto Result3 = Inspection::Get_SignedInteger_24Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == 5368489);
        
        auto Result4 = Inspection::Get_SignedInteger_24Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_SignedInteger_25Bit_BigEndian                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_SignedInteger_25Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_SignedInteger_25Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result1->GetValue()->GetData()) == -16021858);
        
        auto Result2 = Inspection::Get_SignedInteger_25Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result2->GetValue()->GetData()) == 4472245);
        
        auto Result3 = Inspection::Get_SignedInteger_25Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::int32_t>(Result3->GetValue()->GetData()) == 9393481);
        
        auto Result4 = Inspection::Get_SignedInteger_25Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}
