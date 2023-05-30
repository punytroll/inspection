#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength                            //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_Buffer_UnsignedInteger_8Bit_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        
        const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
        
        assert(Data.size() == 5);
        assert(Data[0] == 0x61);
        assert(Data[1] == 0x62);
        assert(Data[2] == 0x63);
        assert(Data[3] == 0x64);
        assert(Data[4] == 0x65);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        
        const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
        
        assert(Data.size() == 4);
        assert(Data[0] == 0x61);
        assert(Data[1] == 0x62);
        assert(Data[2] == 0x63);
        assert(Data[3] == 0x64);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength                          //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x00, 0x00, 0x00, 0x00, 0x00};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        
        const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
        
        assert(Data.size() == 5);
        assert(Data[0] == 0x00);
        assert(Data[1] == 0x00);
        assert(Data[2] == 0x00);
        assert(Data[3] == 0x00);
        assert(Data[4] == 0x00);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        
        const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
        
        assert(Data.size() == 4);
        assert(Data[0] == 0x00);
        assert(Data[1] == 0x00);
        assert(Data[2] == 0x00);
        assert(Data[3] == 0x00);
    }
}
