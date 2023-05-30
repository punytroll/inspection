#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit                              //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit() -> void
{
    std::uint8_t RawBuffer[] = {0x45, 0x8d};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result1 = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Reader, {});
        
        assert(Result1->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result1->GetValue()->GetData()) == 0x45);
        
        auto Result2 = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Reader, {});
        
        assert(Result2->GetSuccess() == false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian                  //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian() -> void
{
    std::uint8_t RawBuffer[] = {0x45, 0x33, 0x4f, 0x11, 0x80, 0x80, 0x80, 0x80};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{8, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x08ace791);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{1, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{2, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{3, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{4, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
}
