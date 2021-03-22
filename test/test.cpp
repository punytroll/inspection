#include <cassert>

#include "buffer.h"
#include "getters.h"
#include "result.h"

int main(void)
{
	std::uint8_t Buffer0[] = {};
	std::uint8_t Buffer1[] = {0x00};
	std::uint8_t Buffer2[] = {0x61, 0x62, 0x63, 0x64, 0x65};
	std::uint8_t Buffer3[] = {0x00, 0x00, 0x00, 0x00, 0x00};
	
	{
		// read an unsigned integer with 0 bits from an empty reader
		Inspection::Buffer Buffer{Buffer0, Inspection::Length{0, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(Reader.GetPositionInBuffer() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(0, 0));
	}
	{
		// read an unsigned integer with 0 bits from the start of a non-empty reader
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(Reader.GetPositionInBuffer() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(1, 0));
	}
	{
		// testing reading with Get_ASCII_String_Alphabetic_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ASCII_String_Alphabetic_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ASCII_String_AlphaNumeric_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ASCII_String_AlphaNumeric_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ASCII_String_AlphaNumericOrSpace_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ASCII_String_AlphaNumericOrSpace_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ASCII_String_Printable_EndedByInvalidOrLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ASCII_String_Printable_EndedByInvalidOrLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ASCII_String_Printable_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ASCII_String_Printable_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_Buffer_UnsignedInteger_8Bit_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader, {})};
		
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
		// testing reading with Get_Buffer_UnsignedInteger_8Bit_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		
		const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
		
		assert(Data.size() == 4);
		assert(Data[0] == 0x61);
		assert(Data[1] == 0x62);
		assert(Data[2] == 0x63);
		assert(Data[3] == 0x64);
	}
	{
		// testing reading with Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength
		Inspection::Buffer Buffer{Buffer3, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Reader, {})};
		
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
		// testing reading with Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer3, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		
		const auto & Data{std::any_cast<const std::vector<std::uint8_t> &>(Result->GetValue()->GetData())};
		
		assert(Data.size() == 4);
		assert(Data[0] == 0x00);
		assert(Data[1] == 0x00);
		assert(Data[2] == 0x00);
		assert(Data[3] == 0x00);
	}
	std::cout << "All tests successfull." << std::endl;
}
