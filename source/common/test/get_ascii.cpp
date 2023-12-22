#include <cassert>

#include <common/buffer.h>
#include <common/execution_context.h>
#include <common/getters.h>
#include <common/reader.h>
#include <common/type_repository.h>

#include "tests.h"


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_Character_Alphabetic                                           //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_Character_Alphabetic() -> void
{
    std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
    auto Buffer = Inspection::Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{0, 4}};
        auto Result = Inspection::Get_ASCII_Character_Alphabetic(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ASCII_Character_Alphabetic(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_Character_AlphaNumeric                                         //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_Character_AlphaNumeric() -> void
{
    std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
    auto Buffer = Inspection::Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{0, 4}};
        auto Result = Inspection::Get_ASCII_Character_AlphaNumeric(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ASCII_Character_AlphaNumeric(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_Character_AlphaNumericOrSpace                                  //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_Character_AlphaNumericOrSpace() -> void
{
    std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
    auto Buffer = Inspection::Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{0, 4}};
        auto Result = Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Reader, {});
        
        assert(Result->GetSuccess() == false);
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{1, 0}};
        auto Result = Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_String_Alphabetic_EndedByLength                                //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_String_Alphabetic_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength                              //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_String_AlphaNumeric_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength                       //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_String_Printable_EndedByLength                                 //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_String_Printable_EndedByLength() -> void
{
    std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
    auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
    
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
        auto Result = Inspection::ExecutionContext::Call(Inspection::Get_ASCII_String_Printable_EndedByLength, Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
    }
    {
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
        auto Result = Inspection::ExecutionContext::Call(Inspection::Get_ASCII_String_Printable_EndedByLength, Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
    }
}

	
///////////////////////////////////////////////////////////////////////////////////////////////
// TEST Inspection::Get_ASCII_String_Printable_EndedByTermination                            //
///////////////////////////////////////////////////////////////////////////////////////////////

auto Inspection::Test::Get_ASCII_String_Printable_EndedByTermination() -> void
{
    std::uint8_t RawBufferASCIILatinSmallLetterA[] = {0x61, 0x62, 0x00, 0x00};
    
    {
        auto Buffer = Inspection::Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{0, 2}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(Result->GetValue()->HasTag("ended by error") == true);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "a");
        assert(Result->GetValue()->HasTag("ended by error") == true);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{2, 6}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == false);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "ab");
        assert(Result->GetValue()->HasTag("ended by error") == true);
        assert(Result->GetValue()->HasTag("error") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("requested length") == true);
        assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
    }
    {
        auto Buffer = Inspection::Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{3, 0}};
        auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
        auto Result = Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {});
        
        assert(Result->GetSuccess() == true);
        assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "ab");
    }
}
