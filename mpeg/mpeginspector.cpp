#include <deque>

#include "../common/common.h"

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto MPEGStreamResult{Get_MPEG_1_Stream(Buffer)};
	
	MPEGStreamResult->GetValue()->SetName("MPEGStream");
	if(Buffer.GetPosition() < Buffer.GetLength())
	{
		MPEGStreamResult->SetSuccess(false);
	}
	
	return MPEGStreamResult;
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to MPEG-1 audio (ISO/IEC 11172-3)!" << std::endl;
	
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto Argument{0};
	
	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
