#include <deque>

#include "../common/common.h"

using namespace std::string_literals;

void AppendUnkownContinuation(std::shared_ptr< Inspection::Value > Value, Inspection::Buffer & Buffer)
{
	auto ErrorValue{Value->AppendValue("error", "Unknown continuation."s)};
	
	ErrorValue->AppendTag("position", to_string_cast(Buffer.GetPosition()));
	ErrorValue->AppendTag("length", to_string_cast(Buffer.GetLength()));
}

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
// - ASFFile                                                                                     //
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
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						AppendUnkownContinuation(Result->GetValue(), Buffer);
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
										AppendUnkownContinuation(Result->GetValue(), Buffer);
									}
								}
								else
								{
									Buffer.SetPosition(Start);
									AppendUnkownContinuation(Result->GetValue(), Buffer);
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
									AppendUnkownContinuation(Result->GetValue(), Buffer);
								}
							}
							else
							{
								Buffer.SetPosition(Start);
								AppendUnkownContinuation(Result->GetValue(), Buffer);
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
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						AppendUnkownContinuation(Result->GetValue(), Buffer);
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
								AppendUnkownContinuation(Result->GetValue(), Buffer);
							}
						}
						else
						{
							Buffer.SetPosition(Start);
							AppendUnkownContinuation(Result->GetValue(), Buffer);
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
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						AppendUnkownContinuation(Result->GetValue(), Buffer);
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
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						AppendUnkownContinuation(Result->GetValue(), Buffer);
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
						AppendUnkownContinuation(Result->GetValue(), Buffer);
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
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
					}
					else
					{
						Buffer.SetPosition(Start);
						PartialResult = Get_ASF_File(Buffer);
						if(PartialResult->GetSuccess() == true)
						{
							Start = Buffer.GetPosition();
							Result->GetValue()->AppendValue("ASFFile", PartialResult->GetValue());
							if(Buffer.GetPosition() == Buffer.GetLength())
							{
								Result->SetSuccess(true);
							}
							else
							{
								AppendUnkownContinuation(Result->GetValue(), Buffer);
							}
						}
						else
						{
							Buffer.SetPosition(Start);
							AppendUnkownContinuation(Result->GetValue(), Buffer);
						}
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
		ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		Paths.pop_front();
	}
	
	return 0;
}
