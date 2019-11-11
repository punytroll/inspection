#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/result.h>

int main(void)
{
	std::uint8_t * Buffer0 = { };
	std::uint8_t * Buffer1 = { 0x00 };
	
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
	std::cout << "All tests successfull." << std::endl;
}
