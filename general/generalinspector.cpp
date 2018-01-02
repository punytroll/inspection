#include <deque>

#include "../common/common.h"

using namespace std::string_literals;

///////////////////////////////////////////////////////////////////////////////////////////////////
// the following functions parses these forms:                                                   //
// - ID3v2Tag                                                                                    //
// - ID3v2Tag FLACStream                                                                         //
// - ID3v2Tag FLACStream ID3v1Tag                                                                //
// - ID3v2Tag MPEG1Stream                                                                        //
// - ID3v2Tag MPEG1Stream APEv2Tag                                                               //
// - ID3v2Tag MPEG1Stream APEv2Tag ID3v1Tag                                                      //
// - ID3v2Tag MPEG1Stream ID3v1Tag                                                               //
// - ID3v2Tag ID3v1Tag                                                                           //
// - MPEG1Stream                                                                                 //
// - MPEG1Stream APEv2Tag                                                                        //
// - MPEG1Stream APEv2Tag ID3v1Tag                                                               //
// - MPEG1Stream ID3v1Tag                                                                        //
// - APEv2Tag                                                                                    //
// - APEv2Tag ID3v1Tag                                                                           //
// - ID3v1Tag                                                                                    //
// - FLACStream                                                                                  //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > ProcessBuffer(Inspection::Buffer & Buffer)
{
	auto Result{Inspection::InitializeResult(Buffer)};
	auto Start{Buffer.GetPosition()};
	std::unique_ptr< Inspection::Result > PartialResult;
	
	PartialResult = Get_ID3_2_Tag(Buffer);
	if(PartialResult->GetSuccess() == true)
	{
		Start = Buffer.GetPosition();
		Result->GetValue()->AppendValue("ID3v2Tag", PartialResult->GetValue());
		if(Buffer.GetPosition() == Buffer.GetLength())
		{
			Result->SetSuccess(true);
		}
		else
		{
			PartialResult = Get_FLAC_Stream(Buffer, false);
			if(PartialResult->GetSuccess() == true)
			{
				Start = Buffer.GetPosition();
				Result->GetValue()->AppendValue("FLACStream", PartialResult->GetValue());
				if(Buffer.GetPosition() == Buffer.GetLength())
				{
					Result->SetSuccess(true);
				}
				else
				{
					PartialResult = Get_ID3_1_Tag(Buffer);
					if(PartialResult->GetSuccess() == true)
					{
						Start = Buffer.GetPosition();
						if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
						{
							Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
						}
						if(Buffer.GetPosition() == Buffer.GetLength())
						{
							Result->SetSuccess(true);
						}
						else
						{
							Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
			}
			else
			{
				Buffer.SetPosition(Start);
				PartialResult = Get_MPEG_1_Stream(Buffer);
				if(PartialResult->GetSuccess() == true)
				{
					Start = Buffer.GetPosition();
					Result->GetValue()->AppendValue("MPEG1Stream", PartialResult->GetValue());
					if(Buffer.GetPosition() == Buffer.GetLength())
					{
						Result->SetSuccess(true);
					}
					else
					{
						PartialResult = Get_APE_Tags(Buffer);
						if(PartialResult->GetSuccess() == true)
						{
							Start = Buffer.GetPosition();
							Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
							if(Buffer.GetPosition() == Buffer.GetLength())
							{
								Result->SetSuccess(true);
							}
							else
							{
								PartialResult = Get_ID3_1_Tag(Buffer);
								if(PartialResult->GetSuccess() == true)
								{
									Start = Buffer.GetPosition();
									if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
									{
										Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
									}
									else
									{
										Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
									}
									if(Buffer.GetPosition() == Buffer.GetLength())
									{
										Result->SetSuccess(true);
									}
									else
									{
										Result->GetValue()->AppendValue("error", "Unknown continuation."s);
									}
								}
								else
								{
									Buffer.SetPosition(Start);
									Result->GetValue()->AppendValue("error", "Unknown continuation."s);
								}
							}
						}
						else
						{
							Buffer.SetPosition(Start);
							PartialResult = Get_ID3_1_Tag(Buffer);
							if(PartialResult->GetSuccess() == true)
							{
								Start = Buffer.GetPosition();
								if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
								{
									Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
								}
								else
								{
									Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
								}
								if(Buffer.GetPosition() == Buffer.GetLength())
								{
									Result->SetSuccess(true);
								}
								else
								{
									Result->GetValue()->AppendValue("error", "Unknown continuation."s);
								}
							}
							else
							{
								Buffer.SetPosition(Start);
								Result->GetValue()->AppendValue("error", "Unknown continuation."s);
							}
						}
					}
				}
				else
				{
					Buffer.SetPosition(Start);
					PartialResult = Get_ID3_1_Tag(Buffer);
					if(PartialResult->GetSuccess() == true)
					{
						Start = Buffer.GetPosition();
						if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
						{
							Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
						}
						if(Buffer.GetPosition() == Buffer.GetLength())
						{
							Result->SetSuccess(true);
						}
						else
						{
							Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
			}
		}
	}
	else
	{
		Buffer.SetPosition(Start);
		PartialResult = Get_MPEG_1_Stream(Buffer);
		if(PartialResult->GetSuccess() == true)
		{
			Start = Buffer.GetPosition();
			Result->GetValue()->AppendValue("MPEG1Stream", PartialResult->GetValue());
			if(Buffer.GetPosition() == Buffer.GetLength())
			{
				Result->SetSuccess(true);
			}
			else
			{
				PartialResult = Get_APE_Tags(Buffer);
				if(PartialResult->GetSuccess() == true)
				{
					Start = Buffer.GetPosition();
					Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
					if(Buffer.GetPosition() == Buffer.GetLength())
					{
						Result->SetSuccess(true);
					}
					else
					{
						PartialResult = Get_ID3_1_Tag(Buffer);
						if(PartialResult->GetSuccess() == true)
						{
							Start = Buffer.GetPosition();
							if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
							{
								Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
							}
							else
							{
								Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
							}
							if(Buffer.GetPosition() == Buffer.GetLength())
							{
								Result->SetSuccess(true);
							}
							else
							{
								Result->GetValue()->AppendValue("error", "Unknown continuation."s);
							}
						}
						else
						{
							Buffer.SetPosition(Start);
							Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
				}
				else
				{
					Buffer.SetPosition(Start);
					PartialResult = Get_ID3_1_Tag(Buffer);
					if(PartialResult->GetSuccess() == true)
					{
						Start = Buffer.GetPosition();
						if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
						{
							Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
						}
						if(Buffer.GetPosition() == Buffer.GetLength())
						{
							Result->SetSuccess(true);
						}
						else
						{
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
			}
		}
		else
		{
			Buffer.SetPosition(Start);
			PartialResult = Get_APE_Tags(Buffer);
			if(PartialResult->GetSuccess() == true)
			{
				Start = Buffer.GetPosition();
				Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
				if(Buffer.GetPosition() == Buffer.GetLength())
				{
					Result->SetSuccess(true);
				}
				else
				{
					PartialResult = Get_ID3_1_Tag(Buffer);
					if(PartialResult->GetSuccess() == true)
					{
						Start = Buffer.GetPosition();
						if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
						{
							Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
						}
						if(Buffer.GetPosition() == Buffer.GetLength())
						{
							Result->SetSuccess(true);
						}
						else
						{
							Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
			}
			else
			{
				Buffer.SetPosition(Start);
				PartialResult = Get_ID3_1_Tag(Buffer);
				if(PartialResult->GetSuccess() == true)
				{
					Start = Buffer.GetPosition();
					if(PartialResult->GetValue()->HasValue("AlbumTrack") == true)
					{
						Result->GetValue()->AppendValue("ID3v1.1Tag", PartialResult->GetValue());
					}
					else
					{
						Result->GetValue()->AppendValue("ID3v1Tag", PartialResult->GetValue());
					}
					if(Buffer.GetPosition() == Buffer.GetLength())
					{
						Result->SetSuccess(true);
					}
					else
					{
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
				else
				{
					Buffer.SetPosition(Start);
					PartialResult = Get_FLAC_Stream(Buffer, false);
					if(PartialResult->GetSuccess() == true)
					{
						Start = Buffer.GetPosition();
						Result->GetValue()->AppendValue("FLACStream", PartialResult->GetValue());
						if(Buffer.GetPosition() == Buffer.GetLength())
						{
							Result->SetSuccess(true);
						}
						else
						{
							Result->GetValue()->AppendValue("error", "Unknown continuation."s);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						Result->GetValue()->AppendValue("error", "Unknown continuation."s);
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Buffer);
	
	return Result;
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
		ReadItem(Paths.front(), ProcessBuffer);
		Paths.pop_front();
	}
	
	return 0;
}
