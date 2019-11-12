#include <bitset>
#include <deque>

#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
#include <common/getters.h>
#include <common/result.h>

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	Inspection::Reader PartReader{Reader};
	
	PartReader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
	
	auto PartResult(Inspection::Get_Ogg_Stream(PartReader, {}));
	
	PartResult->GetValue()->SetName("OggStream");
	Reader.AdvancePosition(PartReader.GetConsumedLength());
	
	return PartResult;
}

int main(int argc, char ** argv)
{
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
		ReadItem(Paths.front(), Process, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
