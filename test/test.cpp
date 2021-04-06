#include <cassert>

#include "buffer.h"
#include "getters.h"
#include "result.h"
#include "value_printing.h"

int main(void)
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Reader.Read2Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0xf3, 0x24};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
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
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{2, 0}};
			Inspection::Reader Reader{Buffer};
			
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
	// Inspection::Reader.Read3Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0xf3, 0x24, 0x76};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{3, 0}};
			Inspection::Reader Reader{Buffer};
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
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{3, 0}};
			Inspection::Reader Reader{Buffer};
			
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_Alphabetic_EndedByLength                                     //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength                                   //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByInvalidOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_Printable_EndedByLength                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength                                 //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
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
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength                          //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
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
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength                                  //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer2, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer4, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer1, Inspection::Length{0, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
		}
		{
			Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "");
		}
		{
			Inspection::Buffer Buffer{Buffer4, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength                     //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength                  //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{5, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{6, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			Inspection::Buffer Buffer{Buffer5, Inspection::Length{7, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength          //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint                                      //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian                            //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian                         //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit                                    //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit                                    //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint                                   //
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint                                   //
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_Character_Alphabetic                                                //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
		
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{0, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_Alphabetic(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_Alphabetic(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_Character_AlphaNumeric                                              //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
		
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{0, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_AlphaNumeric(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_AlphaNumeric(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_Character_AlphaNumericOrSpace                                       //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t BufferASCIILatinSmallLetterA[] = {0x61};
		
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{0, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{BufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result->GetValue()->GetData()) == 0x61);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_5Bit                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 3}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result1{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -16);
			
			auto Result2{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -9);
			
			auto Result3{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result3->GetValue()->GetData()) == 1);
			
			auto Result4{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result4->GetValue()->GetData()) == -12);
			
			auto Result5{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result5->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result5->GetValue()->GetData()) == -2);
			
			auto Result6{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result6->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result6->GetValue()->GetData()) == 4);
			
			auto Result7{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result7->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result7->GetValue()->GetData()) == 8);
			
			auto Result8{Inspection::Get_SignedInteger_5Bit(Reader, {})};
			
			assert(Result8->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result8->GetValue()->GetData()) == 15);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_8Bit                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x64};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 5}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_8Bit(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_8Bit(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result->GetValue()->GetData()) == 0x64);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_12Bit_BigEndian                                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x64, 0x30};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::int16_t>(Result->GetValue()->GetData()) == 0x0643);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ASCII_String_Printable_EndedByTermination                                 //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBufferASCIILatinSmallLetterA[] = {0x61, 0x62, 0x00, 0x00};
		
		{
			Inspection::Buffer Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{0, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("ended by error") == true);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "a");
			assert(Result->GetValue()->HasTag("ended by error") == true);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{2, 6}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "ab");
			assert(Result->GetValue()->HasTag("ended by error") == true);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBufferASCIILatinSmallLetterA, Inspection::Length{3, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ASCII_String_Printable_EndedByTermination(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "ab");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_36Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x43, 0x62, 0xa6, 0x9e, 0x30};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 3}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 1}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{2, 6}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{4, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{4, 4}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_36Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<std::uint64_t>(Result->GetValue()->GetData()) == 0x00000004362a69e3);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_Character                                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA[] = {0x61};
		std::uint8_t RawBuffer_Not_ISO_IEC_8859_1_1998_Character[] = {0x87};
		
		{
			Inspection::Buffer Buffer{RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA, Inspection::Length{0, 5}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data length") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("remaining length") == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer_ISO_IEC_8859_1_1998_LatinSmallLetterA, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {})};
			
			assert(Result->GetSuccess() == true);
		}
		{
			Inspection::Buffer Buffer{RawBuffer_Not_ISO_IEC_8859_1_1998_Character, Inspection::Length{1, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_ISO_IEC_8859_1_1998_Character(Reader, {})};
			
			assert(Result->GetSuccess() == false);
			assert(Result->GetValue()->HasTag("error") == true);
			assert(Result->GetValue()->GetTag("error")->HasTag("data") == true);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_5Bit                                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{0, 2}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result1{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result1->GetValue()->GetData()) == 16);
			
			auto Result2{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result2->GetValue()->GetData()) == 23);
			
			auto Result3{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result3->GetValue()->GetData()) == 1);
			
			auto Result4{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result4->GetValue()->GetData()) == 20);
			
			auto Result5{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result5->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result5->GetValue()->GetData()) == 30);
			
			auto Result6{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result6->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result6->GetValue()->GetData()) == 4);
			
			auto Result7{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result7->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result7->GetValue()->GetData()) == 8);
			
			auto Result8{Inspection::Get_UnsignedInteger_5Bit(Reader, {})};
			
			assert(Result8->GetSuccess() == true);
			assert(std::any_cast<std::uint8_t>(Result8->GetValue()->GetData()) == 15);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_10Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{5, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result1{Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x0217);
			
			auto Result2{Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {})};
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x0034);
			
			auto Result3{Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x03c4);
			
			auto Result4{Inspection::Get_UnsignedInteger_10Bit_BigEndian(Reader, {})};
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x010f);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_11Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d, 0x51, 0xea, 0xa9, 0x34, 0xb2};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{11, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result1{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x042e);
			
			auto Result2{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x00d3);
			
			auto Result3{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0622);
			
			auto Result4{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x00f6);
			
			auto Result5{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result5->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result5->GetValue()->GetData()) == 0x06a8);
			
			auto Result6{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result6->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result6->GetValue()->GetData()) == 0x07aa);
			
			auto Result7{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result7->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result7->GetValue()->GetData()) == 0x0526);
			
			auto Result8{Inspection::Get_UnsignedInteger_11Bit_BigEndian(Reader, {})};
			
			assert(Result8->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result8->GetValue()->GetData()) == 0x04b2);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_12Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			
			auto Result1{Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x085c);
			
			auto Result2{Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x034f);
			
			auto Result3{Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0110);
			
			auto Result4{Inspection::Get_UnsignedInteger_12Bit_BigEndian(Reader, {})};
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result4->GetValue()->GetData()) == 0x0f6d);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_13Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f, 0x6d};
		
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{1, 1}};
			Inspection::Reader Reader{Buffer};
			auto Result{Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {})};
			
			assert(Result->GetSuccess() == false);
		}
		{
			Inspection::Buffer Buffer{RawBuffer, Inspection::Length{6, 0}};
			Inspection::Reader Reader{Buffer};
			auto Result1{Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {})};
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result1->GetValue()->GetData()) == 0x10b8);
			
			auto Result2{Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {})};
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result2->GetValue()->GetData()) == 0x0d3c);
			
			auto Result3{Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {})};
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::uint16_t>(Result3->GetValue()->GetData()) == 0x0887);
			
			auto Result4{Inspection::Get_UnsignedInteger_13Bit_BigEndian(Reader, {})};
			
			assert(Result4->GetSuccess() == false);
		}
	}
	std::cout << "All tests successful." << std::endl;
}
