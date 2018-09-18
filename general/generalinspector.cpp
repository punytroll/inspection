#include <deque>
#include <string>
#include <vector>

#include <common/any_printing.h>
#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getters.h>
#include <common/result.h>
#include <common/value_printing.h>

using namespace std::string_literals;

std::vector< std::string > SplitString(const std::string & String, char Delimiter)
{
	std::vector< std::string > Result;
	auto BracketLevel{0};
	auto IsEscaped{false};
	std::string Part;
	
	for(auto Character : String)
	{
		if(Character == Delimiter)
		{
			if(IsEscaped == false)
			{
				if(BracketLevel == 0)
				{
					Result.push_back(Part);
					Part = "";
				}
				else
				{
					Part += Character;
				}
			}
			else
			{
				Part += Character;
			}
		}
		else if(Character == '\\')
		{
			if(IsEscaped == true)
			{
				Part += Character;
			}
			IsEscaped = !IsEscaped;
		}
		else if(Character == '[')
		{
			Part += Character;
			if(IsEscaped == true)
			{
				IsEscaped = false;
			}
			else
			{
				BracketLevel += 1;
			}
		}
		else if(Character == ']')
		{
			Part += Character;
			if(IsEscaped == true)
			{
				IsEscaped = false;
			}
			else
			{
				assert(BracketLevel >= 0);
				BracketLevel -= 1;
			}
		}
		else
		{
			Part += Character;
		}
	}
	Result.push_back(Part);
	
	return Result;
}

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
	Inspection::Reader FieldReader{Buffer};
	
	PartialResult = Get_ID3_2_Tag(FieldReader);
	if(PartialResult->GetSuccess() == true)
	{
		Buffer.SetPosition(FieldReader);
		Start = Buffer.GetPosition();
		Result->GetValue()->AppendValue("ID3v2Tag", PartialResult->GetValue());
		if(Buffer.GetPosition() == Buffer.GetLength())
		{
			Result->SetSuccess(true);
		}
		else
		{
			Inspection::Reader FieldReader{Buffer};
			
			PartialResult = Get_FLAC_Stream(FieldReader);
			if(PartialResult->GetSuccess() == true)
			{
				Buffer.SetPosition(FieldReader);
				Start = Buffer.GetPosition();
				Result->GetValue()->AppendValue("FLACStream", PartialResult->GetValue());
				if(Buffer.GetPosition() == Buffer.GetLength())
				{
					Result->SetSuccess(true);
				}
				else
				{
					Inspection::Reader FieldReader{Buffer};
					
					PartialResult = Get_ID3_1_Tag(FieldReader);
					if(PartialResult->GetSuccess() == true)
					{
						Buffer.SetPosition(FieldReader);
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
				
				Inspection::Reader FieldReader{Buffer};
				
				PartialResult = Get_MPEG_1_Stream(FieldReader);
				if(PartialResult->GetSuccess() == true)
				{
					Buffer.SetPosition(FieldReader);
					Start = Buffer.GetPosition();
					Result->GetValue()->AppendValue("MPEG1Stream", PartialResult->GetValue());
					if(Buffer.GetPosition() == Buffer.GetLength())
					{
						Result->SetSuccess(true);
					}
					else
					{
						Inspection::Reader FieldReader{Buffer};
						
						PartialResult = Get_APE_Tags(FieldReader);
						if(PartialResult->GetSuccess() == true)
						{
							Buffer.SetPosition(FieldReader);
							Start = Buffer.GetPosition();
							Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
							if(Buffer.GetPosition() == Buffer.GetLength())
							{
								Result->SetSuccess(true);
							}
							else
							{
								Inspection::Reader FieldReader{Buffer};
								
								PartialResult = Get_ID3_1_Tag(FieldReader);
								if(PartialResult->GetSuccess() == true)
								{
									Buffer.SetPosition(FieldReader);
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
							
							Inspection::Reader FieldReader{Buffer};
							
							PartialResult = Get_ID3_1_Tag(FieldReader);
							if(PartialResult->GetSuccess() == true)
							{
								Buffer.SetPosition(FieldReader);
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
					
					Inspection::Reader FieldReader{Buffer};
					
					PartialResult = Get_ID3_1_Tag(FieldReader);
					if(PartialResult->GetSuccess() == true)
					{
						Buffer.SetPosition(FieldReader);
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
		
		Inspection::Reader FieldReader{Buffer};
		
		PartialResult = Get_MPEG_1_Stream(FieldReader);
		if(PartialResult->GetSuccess() == true)
		{
			Buffer.SetPosition(FieldReader);
			Start = Buffer.GetPosition();
			Result->GetValue()->AppendValue("MPEG1Stream", PartialResult->GetValue());
			if(Buffer.GetPosition() == Buffer.GetLength())
			{
				Result->SetSuccess(true);
			}
			else
			{
				Inspection::Reader FieldReader{Buffer};
				
				PartialResult = Get_APE_Tags(FieldReader);
				if(PartialResult->GetSuccess() == true)
				{
					Buffer.SetPosition(FieldReader);
					Start = Buffer.GetPosition();
					Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
					if(Buffer.GetPosition() == Buffer.GetLength())
					{
						Result->SetSuccess(true);
					}
					else
					{
						Inspection::Reader FieldReader{Buffer};
						
						PartialResult = Get_ID3_1_Tag(FieldReader);
						if(PartialResult->GetSuccess() == true)
						{
							Buffer.SetPosition(FieldReader);
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
					
					Inspection::Reader FieldReader{Buffer};
					
					PartialResult = Get_ID3_1_Tag(FieldReader);
					if(PartialResult->GetSuccess() == true)
					{
						Buffer.SetPosition(FieldReader);
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
			
			Inspection::Reader FieldReader{Buffer};
			
			PartialResult = Get_APE_Tags(FieldReader);
			if(PartialResult->GetSuccess() == true)
			{
				Buffer.SetPosition(FieldReader);
				Start = Buffer.GetPosition();
				Result->GetValue()->AppendValue("APEv2Tag", PartialResult->GetValue());
				if(Buffer.GetPosition() == Buffer.GetLength())
				{
					Result->SetSuccess(true);
				}
				else
				{
					Inspection::Reader FieldReader{Buffer};
					
					PartialResult = Get_ID3_1_Tag(FieldReader);
					if(PartialResult->GetSuccess() == true)
					{
						Buffer.SetPosition(FieldReader);
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
				
				Inspection::Reader FieldReader{Buffer};
				
				PartialResult = Get_ID3_1_Tag(FieldReader);
				if(PartialResult->GetSuccess() == true)
				{
					Buffer.SetPosition(FieldReader);
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
					
					Inspection::Reader FieldReader{Buffer};
					
					PartialResult = Get_FLAC_Stream(FieldReader);
					if(PartialResult->GetSuccess() == true)
					{
						Buffer.SetPosition(FieldReader);
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
						
						Inspection::Reader FieldReader{Buffer};
						
						PartialResult = Get_ASF_File(FieldReader);
						if(PartialResult->GetSuccess() == true)
						{
							Buffer.SetPosition(FieldReader);
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
							Buffer.SetPosition(FieldReader);
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

bool EvaluateTestPath(std::shared_ptr< Inspection::Value > Value, const std::string & TestPath)
{
	auto FilterParts{SplitString(TestPath, '/')};
	auto Result{false};
	
	for(auto Index = 0ul; Index < FilterParts.size(); ++Index)
	{
		auto FilterPart{FilterParts[Index]};
		auto FilterPartSpecifications{SplitString(FilterPart, ':')};
		
		if(FilterPartSpecifications[0] == "sub")
		{
			if(FilterPartSpecifications.size() == 2)
			{
				Value = Value->GetValue(FilterPartSpecifications[1]);
			}
		}
		else if(FilterPartSpecifications[0] == "value")
		{
			std::stringstream Output;
			
			Output << Value->GetAny();
			Result = Output.str() == "true";
		}
		else if(FilterPartSpecifications[0] == "tag")
		{
			if(FilterPartSpecifications.size() == 2)
			{
				Value = Value->GetTag(FilterPartSpecifications[1]);
			}
		}
		else if(FilterPartSpecifications[0] == "has-tag")
		{
			Result = Value->HasTag(FilterPartSpecifications[1]);
		}
		else if(FilterPartSpecifications[0] == "has-sub")
		{
			Result = Value->HasValue(FilterPartSpecifications[1]);
		}
		else if(FilterPartSpecifications[0] == "is-value")
		{
			std::stringstream Output;
			
			Output << Value->GetAny();
			Result = Output.str() == FilterPartSpecifications[1];
		}
	}
	
	return Result;
}

void FilterWriter(std::unique_ptr< Inspection::Result > & Result, const std::string & FilterPath)
{
	auto FilterParts{SplitString(FilterPath.substr(1), '/')};
	auto Value{Result->GetValue()};
	
	for(auto Index = 0ul; Index < FilterParts.size(); ++Index)
	{
		auto FilterPart{FilterParts[Index]};
		auto FilterPartSpecifications{SplitString(FilterPart, ':')};
		
		if(FilterPartSpecifications[0] == "sub")
		{
			if(FilterPartSpecifications.size() == 2)
			{
				Value = Value->GetValue(FilterPartSpecifications[1]);
			}
			else if(FilterPartSpecifications.size() == 3)
			{
				auto TestPath{FilterPartSpecifications[2].substr(1, FilterPartSpecifications[2].size() - 2)};
				std::shared_ptr< Inspection::Value > MatchingValue;
				
				for(auto PartValue : Value->GetValues())
				{
					if((PartValue->GetName() == FilterPartSpecifications[1]) && (EvaluateTestPath(PartValue, TestPath) == true))
					{
						MatchingValue = PartValue;
						
						break;
					}
				}
				if(MatchingValue == nullptr)
				{
					throw std::invalid_argument("The test \"" + TestPath + "\" could not be satisfied by any sub value.");
				}
				else
				{
					Value = MatchingValue;
				}
			}
			if(Index + 1 == FilterParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(FilterPartSpecifications[0] == "value")
		{
			std::cout << Value->GetAny();
		}
		else if(FilterPartSpecifications[0] == "tag")
		{
			Value = Value->GetTag(FilterPartSpecifications[1]);
			if(Index + 1 == FilterParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(FilterPartSpecifications[0] == "has-tag")
		{
			if(Value->HasTag(FilterPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(FilterPartSpecifications[0] == "has-sub")
		{
			if(Value->HasValue(FilterPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(FilterPartSpecifications[0] == "is-value")
		{
			std::stringstream Output;
			
			Output << Value->GetAny();
			if(Output.str() == FilterPartSpecifications[1])
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
	}
}

int main(int argc, char ** argv)
{
	std::string ValuePrefix{"--value="};
	std::string ValuePath;
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < Arguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument.compare(0, ValuePrefix.size(), ValuePrefix) == 0)
		{
			ValuePath = Argument.substr(ValuePrefix.size());
			if((ValuePath.size() > 0) && (ValuePath[0] != '/'))
			{
				std::cerr << "A --value has to be given as an absolute path." << std::endl;
				
				return 1;
			}
		}
		else if(Argument == "--verbose")
		{
			g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples = true;
		}
		else
		{
			Paths.push_back(Argument);
		}
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		if(ValuePath != "")
		{
			ReadItem(Paths.front(), ProcessBuffer, std::bind(FilterWriter, std::placeholders::_1, ValuePath));
		}
		else
		{
			ReadItem(Paths.front(), ProcessBuffer, DefaultWriter);
		}
		Paths.pop_front();
	}
	
	return 0;
}
