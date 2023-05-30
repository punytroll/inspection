#include <cassert>

#include <common/buffer.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Reader::Read2Bits()                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::ReaderRead2Bits() -> void
{
    std::uint8_t RawBuffer[] = {0xf3, 0x24};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x01);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        
        Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
        
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x01);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read2Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 2}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 2}));
        assert(ReadResult.Data == 0x00);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Reader::Read3Bits()                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::ReaderRead3Bits() -> void
{
    std::uint8_t RawBuffer[] = {0xf3, 0x24, 0x76};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{3, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{3, 0}};
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x07);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x04);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x06);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x01);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x06);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x06);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{3, 0}};
        
        Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
        
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x06);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x03);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x02);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x04);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x05);
        
        assert(Reader.Read3Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 3}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 3}));
        assert(ReadResult.Data == 0x03);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Reader::Read6Bits()                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::ReaderRead6Bits() -> void
{
    std::uint8_t RawBuffer[] = {0xf3, 0x24};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read6Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 6}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 6}));
        assert(ReadResult.Data == 0x3c);
        
        assert(Reader.Read6Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 6}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 6}));
        assert(ReadResult.Data == 0x32);
        
        assert(Reader.Read6Bits(ReadResult) == false);
        assert(ReadResult.Success == false);
        assert((ReadResult.InputLength == Inspection::Length{0, 4}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 0}));
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{2, 0}};
        
        Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
        
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read6Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 6}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 6}));
        assert(ReadResult.Data == 0x33);
        
        assert(Reader.Read6Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 6}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 6}));
        assert(ReadResult.Data == 0x13);
        
        assert(Reader.Read6Bits(ReadResult) == false);
        assert(ReadResult.Success == false);
        assert((ReadResult.InputLength == Inspection::Length{0, 4}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 0}));
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Reader::Read7Bits()                                                      //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::ReaderRead7Bits() -> void
{
    std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{4, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 0}};
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x42);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x70);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x69);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x71);
        
        assert(Reader.Read7Bits(ReadResult) == false);
        assert(ReadResult.Success == false);
        assert((ReadResult.InputLength == Inspection::Length{0, 4}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 0}));
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 0}};
        
        Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
        
        Inspection::ReadResult ReadResult;
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x05);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x07);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x3f);
        
        assert(Reader.Read7Bits(ReadResult) == true);
        assert(ReadResult.Success == true);
        assert((ReadResult.InputLength == Inspection::Length{0, 7}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 7}));
        assert(ReadResult.Data == 0x0a);
        
        assert(Reader.Read7Bits(ReadResult) == false);
        assert(ReadResult.Success == false);
        assert((ReadResult.InputLength == Inspection::Length{0, 4}));
        assert((ReadResult.OutputLength == Inspection::Length{0, 0}));
    }
}
