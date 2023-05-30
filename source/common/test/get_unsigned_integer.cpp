#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_0Bit                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_0Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x00};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{0, 0}};
        auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
        
        assert(Result->GetSuccess() == true);
        assert(Reader.GetReadPositionInInput() == Inspection::Length(0, 0));
        assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
        assert(Reader.CalculateRemainingOutputLength() == Inspection::Length(0, 0));
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
        
        assert(Result->GetSuccess() == true);
        assert(Reader.GetReadPositionInInput() == Inspection::Length(0, 0));
        assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
        assert(Reader.CalculateRemainingOutputLength() == Inspection::Length(1, 0));
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_5Bit                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_5Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result1->GetValue()->GetData()) == 0x10);
        
        auto Result2 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result2->GetValue()->GetData()) == 0x17);
        
        auto Result3 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result3->GetValue()->GetData()) == 0x01);
        
        auto Result4 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result4->GetValue()->GetData()) == 0x14);
        
        auto Result5 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result5->GetValue()->GetData()) == 0x1e);
        
        auto Result6 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result6->GetValue()->GetData()) == 0x04);
        
        auto Result7 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result7->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result7->GetValue()->GetData()) == 0x08);
        
        auto Result8 = Inspection::Get_UnsignedInteger_5Bit(Reader, {});
        
        assert(Result8->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result8->GetValue()->GetData()) == 0x0f);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_6Bit                                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_6Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{4, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result1->GetValue()->GetData()) == 0x21);
        
        auto Result2 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result2->GetValue()->GetData()) == 0x1c);
        
        auto Result3 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result3->GetValue()->GetData()) == 0x0d);
        
        auto Result4 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result4->GetValue()->GetData()) == 0x0f);
        
        auto Result5 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result5->GetValue()->GetData()) == 0x04);
        
        auto Result6 = Inspection::Get_UnsignedInteger_6Bit(Reader, {});
        
        assert(Result6->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_9Bit_BigEndian                                       //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_9Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x010b);
        
        auto Result2 = Inspection::Get_UnsignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x010d);
        
        auto Result3 = Inspection::Get_UnsignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0078);
        
        auto Result4 = Inspection::Get_UnsignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x0110);
        
        auto Result5 = Inspection::Get_UnsignedInteger_9Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_10Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_10Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x0217);
        
        auto Result2 = Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x0034);
        
        auto Result3 = Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x03c4);
        
        auto Result4 = Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x010f);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_11Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_11Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x042e);
        
        auto Result2 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x00d3);
        
        auto Result3 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0622);
        
        auto Result4 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x00f6);
        
        auto Result5 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result5->GetValue()->GetData()) == 0x06a8);
        
        auto Result6 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result6->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result6->GetValue()->GetData()) == 0x07aa);
        
        auto Result7 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result7->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result7->GetValue()->GetData()) == 0x0526);
        
        auto Result8 = Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {});
        
        assert(Result8->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result8->GetValue()->GetData()) == 0x04b2);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_12Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_12Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{6, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x085c);
        
        auto Result2 = Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x034f);
        
        auto Result3 = Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0110);
        
        auto Result4 = Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x0f6d);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_13Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_13Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 1}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{6, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x10b8);
        
        auto Result2 = Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x0d3c);
        
        auto Result3 = Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0887);
        
        auto Result4 = Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_14Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_14Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{6, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x2170);
        
        auto Result2 = Inspection::Get_UnsignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x34f1);
        
        auto Result3 = Inspection::Get_UnsignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x043d);
        
        auto Result4 = Inspection::Get_UnsignedInteger_14Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_16Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_16Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0xab};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x85c3);
        
        auto Result2 = Inspection::Get_UnsignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x4f11);
        
        auto Result3 = Inspection::Get_UnsignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0f6d);
        
        auto Result4 = Inspection::Get_UnsignedInteger_16Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_16Bit_LittleEndian                                   //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_16Bit_LittleEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0xab};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0xc385);
        
        auto Result2 = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x114f);
        
        auto Result3 = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x6d0f);
        
        auto Result4 = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_18Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_18Bit_BigEndian() -> void
{
   std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0xab};
   
   {
       auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
       auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
       auto Result1 = Inspection::Get_UnsignedInteger_18Bit_BigEndian(Reader, {});
       
       assert(Result1->GetSuccess() == true);
       assert(std::any_cast<std::uint32_t>(Result1->GetValue()->GetData()) == 0x2170d);
       
       auto Result2 = Inspection::Get_UnsignedInteger_18Bit_BigEndian(Reader, {});
       
       assert(Result2->GetSuccess() == true);
       assert(std::any_cast<std::uint32_t>(Result2->GetValue()->GetData()) == 0xf110);
       
       auto Result3 = Inspection::Get_UnsignedInteger_18Bit_BigEndian(Reader, {});
       
       assert(Result3->GetSuccess() == true);
       assert(std::any_cast<std::uint32_t>(Result3->GetValue()->GetData()) == 0x3db6a);
       
       auto Result4 = Inspection::Get_UnsignedInteger_18Bit_BigEndian(Reader, {});
       
       assert(Result4->GetSuccess() == false);
   }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_19Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_19Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0xab, 0x7e};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{8, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result1->GetValue()->GetData()) == 0x42e1a);
        
        auto Result2 = Inspection::Get_UnsignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result2->GetValue()->GetData()) == 0x3c443);
        
        auto Result3 = Inspection::Get_UnsignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result3->GetValue()->GetData()) == 0x6db56);
        
        auto Result4 = Inspection::Get_UnsignedInteger_19Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_21Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_21Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{11, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result1->GetValue()->GetData()) == 1095785);
        
        auto Result2 = Inspection::Get_UnsignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result2->GetValue()->GetData()) == 1852477);
        
        auto Result3 = Inspection::Get_UnsignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result3->GetValue()->GetData()) == 1485045);
        
        auto Result4 = Inspection::Get_UnsignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result4->GetValue()->GetData()) == 693067);
        
        auto Result5 = Inspection::Get_UnsignedInteger_21Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_22Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_22Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2, 0x79};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{12, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result1->GetValue()->GetData()) == 2191571);
        
        auto Result2 = Inspection::Get_UnsignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result2->GetValue()->GetData()) == 3215606);
        
        auto Result3 = Inspection::Get_UnsignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result3->GetValue()->GetData()) == 3491754);
        
        auto Result4 = Inspection::Get_UnsignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result4->GetValue()->GetData()) == 2700466);
        
        auto Result5 = Inspection::Get_UnsignedInteger_22Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_23Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_23Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2, 0x79};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{12, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_UnsignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result1->GetValue()->GetData()) == 4383143);
        
        auto Result2 = Inspection::Get_UnsignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result2->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result2->GetValue()->GetData()) == 4473819);
        
        auto Result3 = Inspection::Get_UnsignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result3->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result3->GetValue()->GetData()) == 2768213);
        
        auto Result4 = Inspection::Get_UnsignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result4->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result4->GetValue()->GetData()) == 1264423);
        
        auto Result5 = Inspection::Get_UnsignedInteger_23Bit_BigEndian(Reader, {});
        
        assert(Result5->GetSuccess() == false);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_UnsignedInteger_36Bit_BigEndian                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_UnsignedInteger_36Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x43, 0x62, 0xa6, 0x9e, 0x30};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 1}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 6}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{4, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{4, 4}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint64_t>(Result->GetValue()->GetData()) == 0x00000004362a69e3);
    }
}
