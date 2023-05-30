#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/reader.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_Character                                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_Character() -> void
{
    std::uint8_t RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA[] = {0x61};
    std::uint8_t RawBuffer_Not_ISO_IEC_8859_1_1998_Character[] = {0x87};
    
    {
        auto Buffer = Inspection::Buffer{RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA, Inspection::Length{0, 5}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {});
        
        assert(Result->GetSuccess() == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBuffer_Not_ISO_IEC_8859_1_1998_Character, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("data") == true);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength                             //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination                        //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTermination() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{5, 0}, Inspection::Length{0, 2}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{5, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength                //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength() -> void
{
    std::uint8_t Buffer5[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
    auto Buffer = Inspection::Buffer{Buffer5, Inspection::Length{7, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 2}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 2}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 2}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength             //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength() -> void
{
    std::uint8_t Buffer5[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
    auto Buffer = Inspection::Buffer{Buffer5, Inspection::Length{7, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 4}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{7, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength     //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength() -> void
{
    std::uint8_t Buffer5[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
    auto Buffer = Inspection::Buffer{Buffer5, Inspection::Length{7, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 2}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 5}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{6, 7}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{7, 0}};
        auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
}
