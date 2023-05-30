#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian                       //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian() -> void
{
    std::uint8_t BufferUCS2Termination[] = {0x00, 0x00};
    auto Buffer = Inspection::Buffer{BufferUCS2Termination, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 7}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000000);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian                    //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian() -> void
{
    std::uint8_t BufferUCS2Termination[] = {0x00, 0x00};
    auto Buffer = Inspection::Buffer{BufferUCS2Termination, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 7}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000000);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint() -> void
{
    std::uint8_t BufferUTF8CodePointLatinSmallLetterA[] = {0x61};
    std::uint8_t BufferUTF8CodePointCopyrightSign[] = {0xc2, 0xa9};
    std::uint8_t BufferUTF8CodePointGreekCapitalLetterTheta[] = {0xce, 0x98};
    std::uint8_t BufferUTF8CodePointThaiCharacterDoDek[] = {0xe0, 0xb8, 0x94};
    std::uint8_t BufferUTF8CodePointEmojiComponentRedHair[] = {0xf0, 0x9f, 0xa6, 0xb0};
    
    {
        auto Buffer = Inspection::Buffer{BufferUTF8CodePointLatinSmallLetterA, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
    }
    {
        auto Buffer = Inspection::Buffer{BufferUTF8CodePointCopyrightSign, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x000000a9);
    }
    {
        auto Buffer = Inspection::Buffer{BufferUTF8CodePointGreekCapitalLetterTheta, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000398);
    }
    {
        auto Buffer = Inspection::Buffer{BufferUTF8CodePointThaiCharacterDoDek, Inspection::Length{3, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000e14);
    }
    {
        auto Buffer = Inspection::Buffer{BufferUTF8CodePointEmojiComponentRedHair, Inspection::Length{4, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x0001f9b0);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint                              //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint() -> void
{
    std::uint8_t BufferUTF16BELatinSmallLetterA[] = {0x00, 0x61};
    auto Buffer = Inspection::Buffer{BufferUTF16BELatinSmallLetterA, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{0, 5}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 2}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint                              //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint() -> void
{
    std::uint8_t BufferUTF16LatinSmallLetterA[] = {0x61, 0x00};
    auto Buffer = Inspection::Buffer{BufferUTF16LatinSmallLetterA, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit                               //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit() -> void
{
    std::uint8_t BufferUTF16Termination[] = {0x00, 0x00};
    auto Buffer = Inspection::Buffer{BufferUTF16Termination, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result->GetValue()->GetData()) == 0x0000);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit                               //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit() -> void
{
    std::uint8_t BufferUTF16Termination[] = {0x00, 0x00};
    auto Buffer = Inspection::Buffer{BufferUTF16Termination, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        auto Result = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint16_t>(Result->GetValue()->GetData()) == 0x0000);
    }
}
