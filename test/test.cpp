#include <cassert>

#include "buffer.h"
#include "getters.h"
#include "result.h"

int main(void)
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Reader.Read2Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Reader.Read3Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Reader.Read6Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Reader.Read7Bits()                                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_0Bit                                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_String_Alphabetic_EndedByLength                                     //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength                                   //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_String_Printable_EndedByLength                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65};
		auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
		
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{5, 0}};
			auto Result = Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {});
			
			assert(Result->GetSuccess() == true);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcde");
		}
		{
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
			auto Result = Inspection::Get_ASCII_String_Printable_EndedByLength(Reader, {});
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength                                 //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength                          //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength                                  //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00, 0x00};
		auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{7, 0}};
		
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
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x61, 0x62, 0x63, 0x64, 0x65, 0x00};
		auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{6, 0}};
		
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
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Inspection::Length{4, 4}};
			auto Result = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Reader, {});
			
			assert(Result->GetSuccess() == false);
			assert(std::any_cast<const std::string &>(Result->GetValue()->GetData()) == "abcd");
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength                     //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength                  //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength          //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian                         //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit                                    //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit                                    //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint                                   //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint                                   //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_Character_Alphabetic                                                //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_Character_AlphaNumeric                                              //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ASCII_Character_AlphaNumericOrSpace                                       //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_SignedInteger_1Bit                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85};
		
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 0}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_1Bit(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result1 = Inspection::Get_SignedInteger_1Bit(Reader, {});
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -0x01);
			
			auto Result2 = Inspection::Get_SignedInteger_1Bit(Reader, {});
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == 0x00);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_5Bit                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x85, 0xc3, 0x4f, 0x11, 0x0f};
		
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 3}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{5, 0}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result1 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == -0x10);
			
			auto Result2 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -0x09);
			
			auto Result3 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result3->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result3->GetValue()->GetData()) == 0x01);
			
			auto Result4 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result4->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result4->GetValue()->GetData()) == -0x0c);
			
			auto Result5 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result5->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result5->GetValue()->GetData()) == -0x02);
			
			auto Result6 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result6->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result6->GetValue()->GetData()) == 0x04);
			
			auto Result7 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result7->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result7->GetValue()->GetData()) == 0x08);
			
			auto Result8 = Inspection::Get_SignedInteger_5Bit(Reader, {});
			
			assert(Result8->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result8->GetValue()->GetData()) == 0x0f);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_8Bit                                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x64, 0xc3};
		
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 5}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_8Bit(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{2, 0}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result1 = Inspection::Get_SignedInteger_8Bit(Reader, {});
			
			assert(Result1->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result1->GetValue()->GetData()) == 0x64);
			
			auto Result2 = Inspection::Get_SignedInteger_8Bit(Reader, {});
			
			assert(Result2->GetSuccess() == true);
			assert(std::any_cast<std::int8_t>(Result2->GetValue()->GetData()) == -0x3d);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_SignedInteger_12Bit_BigEndian                                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
	{
		std::uint8_t RawBuffer[] = {0x64, 0x30};
		
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 2}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{0, 4}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 0}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 2}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
			
			assert(Result->GetSuccess() == false);
		}
		{
			auto Buffer = Inspection::Buffer{RawBuffer, Inspection::Length{1, 4}};
			auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
			auto Result = Inspection::Get_SignedInteger_12Bit_BigEndian(Reader, {});
			
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_UnsignedInteger_36Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	// Inspection::Get_ISO_IEC_8859_1_1998_Character                                             //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_5Bit                                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_6Bit                                                      //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_9Bit_BigEndian                                            //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_10Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_11Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_12Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_13Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_14Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_16Bit_BigEndian                                           //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_UnsignedInteger_16Bit_LittleEndian                                        //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit                                   //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	// Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian                       //
	///////////////////////////////////////////////////////////////////////////////////////////////
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
	std::cout << "All tests successful." << std::endl;
}
