#include <cassert>

#include "../common/common.h"

int main(void)
{
	std::uint8_t * Buffer0 = { };
	std::uint8_t * Buffer1 = { 0x00 };
	
	{
		// read an unsigned integer with 0 bits from an empty reader
		Inspection::Buffer Buffer{Buffer0, Inspection::Length{0, 0}};
		Inspection::Reader Reader{Buffer, Inspection::Length{0, 0}};
		auto Result{Get_UnsignedInteger_0Bit(Reader)};
		
		assert(Result->GetSuccess() == true);
		assert(Result->GetLength() == Inspection::Length(0, 0));
		assert(Reader.GetPositionInBuffer() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(0, 0));
	}
	{
		// read an unsigned integer with 0 bits from the start of a non-empty reader
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer, Inspection::Length{1, 0}};
		auto Result{Get_UnsignedInteger_0Bit(Reader)};
		
		assert(Result->GetSuccess() == true);
		assert(Result->GetLength() == Inspection::Length(0, 0));
		assert(Reader.GetPositionInBuffer() == Inspection::Length(0, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(1, 0));
	}
	{
		// read an unsigned integer with 0 bits from inside a non-empty reader
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer, Inspection::Length{0, 4}, Inspection::Length{0, 4}};
		auto Result{Get_UnsignedInteger_0Bit(Reader)};
		
		assert(Result->GetSuccess() == true);
		assert(Result->GetLength() == Inspection::Length(0, 0));
		assert(Reader.GetPositionInBuffer() == Inspection::Length(0, 4));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(0, 4));
	}
	{
		// read an unsigned integer with 0 bits at the end a non-empty reader
		Inspection::Buffer Buffer{Buffer1, Inspection::Length{1, 0}};
		Inspection::Reader Reader{Buffer, Inspection::Length{1, 0}, Inspection::Length{0, 0}};
		auto Result{Get_UnsignedInteger_0Bit(Reader)};
		
		assert(Result->GetSuccess() == true);
		assert(Result->GetLength() == Inspection::Length(0, 0));
		assert(Reader.GetPositionInBuffer() == Inspection::Length(1, 0));
		assert(Reader.GetConsumedLength() == Inspection::Length(0, 0));
		assert(Reader.GetRemainingLength() == Inspection::Length(0, 0));
	}
	std::cout << "All tests successfull." << std::endl;
}