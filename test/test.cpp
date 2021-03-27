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
	std::uint8_t Buffer4[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00};
	std::uint8_t Buffer5[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
	
	{
		// read an unsigned integer with 0 bits from an empty reader
		Inspection::Buffer Buffer{Buffer0, Inspection::Length{0, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(Reader.GetReadPositionInInput() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.CalculateRemainingOutputLength() == Inspection::Length(0, 0));
	}
	{
		// read an unsigned integer with 0 bits from the start of a non-empty reader
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Get_UnsignedInteger_0Bit(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(Reader.GetReadPositionInInput() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.CalculateRemainingOutputLength() == Inspection::Length(1, 0));
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
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByLength
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByLength from a buffer that is too short
		Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTermination
		Inspection::Buffer Buffer{Buffer4, Inspection::Length{6, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTermination
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTermination from a buffer that is too short
		Inspection::Buffer Buffer{Buffer4, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTermination
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTermination
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{0, 2}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 2}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 2}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 2}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{7, 0}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == true);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	{
		// testing reading with Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength
		Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 4}};
		Inspection::Reader Reader{Buffer};
		auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
		
		assert(Result->GetSuccess() == false);
		assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 5}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 7}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{7, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint                                                  //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUTF8CodePointLatinSmallLetterA[] = {0x61};
		std::uint8_t BufferUTF8CodePointCopyrightSign[] = {0xc2, 0xa9};
		std::uint8_t BufferUTF8CodePointGreekCapitalLetterTheta[] = {0xce, 0x98};
		std::uint8_t BufferUTF8CodePointThaiCharacterDoDek[] = {0xe0, 0xb8, 0x94};
		std::uint8_t BufferUTF8CodePointEmojiComponentRedHair[] = {0xf0, 0x9f, 0xa6, 0xb0};
		
		{
			Inspection::Buffer Buffer{BufferUTF8CodePointLatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
		}
		{
			Inspection::Buffer Buffer{BufferUTF8CodePointCopyrightSign, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x000000a9);
		}
		{
			Inspection::Buffer Buffer{BufferUTF8CodePointGreekCapitalLetterTheta, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000398);
		}
		{
			Inspection::Buffer Buffer{BufferUTF8CodePointThaiCharacterDoDek, Inspection::Length{3, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000e14);
		}
		{
			Inspection::Buffer Buffer{BufferUTF8CodePointEmojiComponentRedHair, Inspection::Length{4, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x0001f9b0);
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUCS2Termination[] = {0x00, 0x00};
		
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{1, 7}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000000);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian                                     //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUCS2Termination[] = {0x00, 0x00};
		
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{1, 7}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUCS2Termination, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000000);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit                                                //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUTF16Termination[] = {0x00, 0x00};
		
		{
			Inspection::Buffer Buffer{BufferUTF16Termination, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16Termination, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result->GetValue()->GetData()) == 0x0000);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit                                                //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUTF16Termination[] = {0x00, 0x00};
		
		{
			Inspection::Buffer Buffer{BufferUTF16Termination, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16Termination, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result->GetValue()->GetData()) == 0x0000);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint                                               //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUTF16BELatinSmallLetterA[] = {0x00, 0x61};
		
		{
			Inspection::Buffer Buffer{BufferUTF16BELatinSmallLetterA, Inspection::Length{0, 5}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16BELatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16BELatinSmallLetterA, Inspection::Length{1, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16BELatinSmallLetterA, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint                                               //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferUTF16LatinSmallLetterA[] = {0x61, 0x00};
		
		{
			Inspection::Buffer Buffer{BufferUTF16LatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferUTF16LatinSmallLetterA, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint32_t>(Result->GetValue()->GetData()) == 0x00000061);
		}
	}
	std::cout << "All tests successfull." << std::endl;
}
