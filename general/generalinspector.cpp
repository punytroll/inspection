#include <deque>
#include <string>
#include <vector>

#include <common/any_printing.h>
#include <common/buffer.h>
#include <common/file_handling.h>
#include <common/getter_repository.h>
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
				IsEscaped = false;
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

void AppendUnkownContinuation(std::shared_ptr< Inspection::Value > Value, Inspection::Reader & Reader)
{
	auto ErrorValue{Value->AppendField("error", "Unknown continuation."s)};
	
	ErrorValue->AddTag("position", to_string_cast(Reader.GetPositionInBuffer()));
	ErrorValue->AddTag("remaining length", to_string_cast(Reader.GetRemainingLength()));
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
// - OggStream                                                                                   //
// - AppleSingle                                                                                 //
// - RIFFFile                                                                                    //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr< Inspection::Result > Process(Inspection::Reader & Reader)
{
	auto Result{Inspection::InitializeResult(Reader)};
	Inspection::Reader PartReader{Reader};
	auto PartResult{Get_ID3_2_Tag(PartReader)};
	
	if(PartResult->GetSuccess() == true)
	{
		Result->GetValue()->AppendField("ID3v2Tag", PartResult->GetValue());
		Reader.AdvancePosition(PartReader.GetConsumedLength());
		if(Reader.IsAtEnd() == true)
		{
			Result->SetSuccess(true);
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			
			PartResult = Inspection::g_GetterRepository.Get({"FLAC", "Stream"}, PartReader, {});
			if(PartResult->GetSuccess() == true)
			{
				Result->GetValue()->AppendField("FLACStream", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				if(Reader.IsAtEnd() == true)
				{
					Result->SetSuccess(true);
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					
					PartResult = Get_ID3_1_Tag(PartReader);
					if(PartResult->GetSuccess() == true)
					{
						if(PartResult->GetValue()->HasField("AlbumTrack") == true)
						{
							Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
						}
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						if(Reader.IsAtEnd() == true)
						{
							Result->SetSuccess(true);
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
					else
					{
						
						AppendUnkownContinuation(Result->GetValue(), Reader);
					}
				}
			}
			else
			{
				Inspection::Reader PartReader{Reader};
				
				PartResult = Get_MPEG_1_Stream(PartReader, {});
				if(PartResult->GetSuccess() == true)
				{
					Result->GetValue()->AppendField("MPEG1Stream", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					if(Reader.IsAtEnd() == true)
					{
						Result->SetSuccess(true);
					}
					else
					{
						Inspection::Reader PartReader{Reader};
						auto PartResult{Inspection::g_GetterRepository.Get({"APE", "Tag"}, PartReader, {})};
						
						if(PartResult->GetSuccess() == true)
						{
							Result->GetValue()->AppendField("APEv2Tag", PartResult->GetValue());
							Reader.AdvancePosition(PartReader.GetConsumedLength());
							if(Reader.IsAtEnd() == true)
							{
								Result->SetSuccess(true);
							}
							else
							{
								Inspection::Reader PartReader{Reader};
								
								PartResult = Get_ID3_1_Tag(PartReader);
								if(PartResult->GetSuccess() == true)
								{
									if(PartResult->GetValue()->HasField("AlbumTrack") == true)
									{
										Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
									}
									else
									{
										Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
									}
									Reader.AdvancePosition(PartReader.GetConsumedLength());
									if(Reader.IsAtEnd() == true)
									{
										Result->SetSuccess(true);
									}
									else
									{
										AppendUnkownContinuation(Result->GetValue(), Reader);
									}
								}
								else
								{
									AppendUnkownContinuation(Result->GetValue(), Reader);
								}
							}
						}
						else
						{
							Inspection::Reader PartReader{Reader};
							
							PartResult = Get_ID3_1_Tag(PartReader);
							if(PartResult->GetSuccess() == true)
							{
								if(PartResult->GetValue()->HasField("AlbumTrack") == true)
								{
									Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
								}
								else
								{
									Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
								}
								Reader.AdvancePosition(PartReader.GetConsumedLength());
								if(Reader.IsAtEnd() == true)
								{
									Result->SetSuccess(true);
								}
								else
								{
									AppendUnkownContinuation(Result->GetValue(), Reader);
								}
							}
							else
							{
								AppendUnkownContinuation(Result->GetValue(), Reader);
							}
						}
					}
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					
					PartResult = Get_ID3_1_Tag(PartReader);
					if(PartResult->GetSuccess() == true)
					{
						if(PartResult->GetValue()->HasField("AlbumTrack") == true)
						{
							Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
						}
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						if(Reader.IsAtEnd() == true)
						{
							Result->SetSuccess(true);
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
					else
					{
						AppendUnkownContinuation(Result->GetValue(), Reader);
					}
				}
			}
		}
	}
	else
	{
		Inspection::Reader PartReader{Reader};
		
		PartResult = Get_MPEG_1_Stream(PartReader, {});
		if(PartResult->GetSuccess() == true)
		{
			Result->GetValue()->AppendField("MPEG1Stream", PartResult->GetValue());
			Reader.AdvancePosition(PartReader.GetConsumedLength());
			if(Reader.IsAtEnd() == true)
			{
				Result->SetSuccess(true);
			}
			else
			{
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_GetterRepository.Get({"APE", "Tag"}, PartReader, {})};
				
				if(PartResult->GetSuccess() == true)
				{
					Result->GetValue()->AppendField("APEv2Tag", PartResult->GetValue());
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					if(Reader.IsAtEnd() == true)
					{
						Result->SetSuccess(true);
					}
					else
					{
						Inspection::Reader PartReader{Reader};
						
						PartResult = Get_ID3_1_Tag(PartReader);
						if(PartResult->GetSuccess() == true)
						{
							if(PartResult->GetValue()->HasField("AlbumTrack") == true)
							{
								Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
							}
							else
							{
								Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
							}
							Reader.AdvancePosition(PartReader.GetConsumedLength());
							if(Reader.IsAtEnd() == true)
							{
								Result->SetSuccess(true);
							}
							else
							{
								AppendUnkownContinuation(Result->GetValue(), Reader);
							}
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					
					PartResult = Get_ID3_1_Tag(PartReader);
					if(PartResult->GetSuccess() == true)
					{
						if(PartResult->GetValue()->HasField("AlbumTrack") == true)
						{
							Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
						}
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						if(Reader.IsAtEnd() == true)
						{
							Result->SetSuccess(true);
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
					else
					{
						AppendUnkownContinuation(Result->GetValue(), Reader);
					}
				}
			}
		}
		else
		{
			Inspection::Reader PartReader{Reader};
			auto PartResult{Inspection::g_GetterRepository.Get({"APE", "Tag"}, PartReader, {})};
			
			if(PartResult->GetSuccess() == true)
			{
				Result->GetValue()->AppendField("APEv2Tag", PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
				if(Reader.IsAtEnd() == true)
				{
					Result->SetSuccess(true);
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					
					PartResult = Get_ID3_1_Tag(PartReader);
					if(PartResult->GetSuccess() == true)
					{
						if(PartResult->GetValue()->HasField("AlbumTrack") == true)
						{
							Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
						}
						else
						{
							Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
						}
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						if(Reader.IsAtEnd() == true)
						{
							Result->SetSuccess(true);
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
					else
					{
						AppendUnkownContinuation(Result->GetValue(), Reader);
					}
				}
			}
			else
			{
				Inspection::Reader PartReader{Reader};
				
				PartResult = Get_ID3_1_Tag(PartReader);
				if(PartResult->GetSuccess() == true)
				{
					if(PartResult->GetValue()->HasField("AlbumTrack") == true)
					{
						Result->GetValue()->AppendField("ID3v1.1Tag", PartResult->GetValue());
					}
					else
					{
						Result->GetValue()->AppendField("ID3v1Tag", PartResult->GetValue());
					}
					Reader.AdvancePosition(PartReader.GetConsumedLength());
					if(Reader.IsAtEnd() == true)
					{
						Result->SetSuccess(true);
					}
					else
					{
						AppendUnkownContinuation(Result->GetValue(), Reader);
					}
				}
				else
				{
					Inspection::Reader PartReader{Reader};
					
					PartResult = Inspection::g_GetterRepository.Get({"FLAC", "Stream"}, PartReader, {});
					if(PartResult->GetSuccess() == true)
					{
						Result->GetValue()->AppendField("FLACStream", PartResult->GetValue());
						Reader.AdvancePosition(PartReader.GetConsumedLength());
						if(Reader.IsAtEnd() == true)
						{
							Result->SetSuccess(true);
						}
						else
						{
							AppendUnkownContinuation(Result->GetValue(), Reader);
						}
					}
					else
					{
						Inspection::Reader PartReader{Reader};
						
						PartResult = Inspection::g_GetterRepository.Get({"ASF", "File"}, PartReader, {});
						if(PartResult->GetSuccess() == true)
						{
							Result->GetValue()->AppendField("ASFFile", PartResult->GetValue());
							Reader.AdvancePosition(PartReader.GetConsumedLength());
							if(Reader.IsAtEnd() == true)
							{
								Result->SetSuccess(true);
							}
							else
							{
								AppendUnkownContinuation(Result->GetValue(), Reader);
							}
						}
						else
						{
							Inspection::Reader PartReader{Reader};
							auto PartResult{Inspection::Get_Ogg_Stream(PartReader, {})};
							
							if(PartResult->GetSuccess() == true)
							{
								Result->GetValue()->AppendField("OggStream", PartResult->GetValue());
								Reader.AdvancePosition(PartReader.GetConsumedLength());
								if(Reader.IsAtEnd() == true)
								{
									Result->SetSuccess(true);
								}
								else
								{
									AppendUnkownContinuation(Result->GetValue(), Reader);
								}
							}
							else
							{
								Inspection::Reader PartReader{Reader};
								auto PartResult{Inspection::g_GetterRepository.Get({"Apple", "AppleDouble_File"}, PartReader, {})};
								
								if(PartResult->GetSuccess() == true)
								{
									Result->GetValue()->AppendField("AppleDoubleFile", PartResult->GetValue());
									Reader.AdvancePosition(PartReader.GetConsumedLength());
									if(Reader.IsAtEnd() == true)
									{
										Result->SetSuccess(true);
									}
									else
									{
										AppendUnkownContinuation(Result->GetValue(), Reader);
									}
								}
								else
								{
									Inspection::Reader PartReader{Reader};
									auto PartResult{Get_RIFF_Chunk(PartReader, {})};
									
									if(PartResult->GetSuccess() == true)
									{
										Result->GetValue()->AppendField("RIFFChunk", PartResult->GetValue());
										Result->GetValue()->SetName("RIFFFile");
										Reader.AdvancePosition(PartReader.GetConsumedLength());
										if(Reader.IsAtEnd() == true)
										{
											Result->SetSuccess(true);
										}
										else
										{
											AppendUnkownContinuation(Result->GetValue(), Reader);
										}
									}
									else
									{
										AppendUnkownContinuation(Result->GetValue(), Reader);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

std::unique_ptr< Inspection::Result > ProcessWithSpecificGetter(Inspection::Reader & Reader, const std::vector< std::string > & Getter)
{
	auto Result{Inspection::InitializeResult(Reader)};
	Inspection::Reader PartReader{Reader};
	auto PartResult{Inspection::g_GetterRepository.Get(Getter, PartReader, {})};
	
	Result->SetSuccess(PartResult->GetSuccess());
	Result->GetValue()->AppendField(Getter.back(), PartResult->GetValue());
	Reader.AdvancePosition(PartReader.GetConsumedLength());
	Inspection::FinalizeResult(Result, Reader);
	
	return Result;
}

bool EvaluateTestQuery(std::shared_ptr< Inspection::Value > Value, const std::string & Query)
{
	auto QueryParts{SplitString(Query, '/')};
	auto Result{false};
	
	for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
	{
		auto QueryPart{QueryParts[Index]};
		auto QueryPartSpecifications{SplitString(QueryPart, ':')};
		
		if(QueryPartSpecifications[0] == "field")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetField(QueryPartSpecifications[1]);
			}
		}
		else if(QueryPartSpecifications[0] == "data")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			Result = Output.str() == "true";
		}
		else if(QueryPartSpecifications[0] == "tag")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetTag(QueryPartSpecifications[1]);
			}
		}
		else if(QueryPartSpecifications[0] == "has-tag")
		{
			Result = Value->HasTag(QueryPartSpecifications[1]);
		}
		else if(QueryPartSpecifications[0] == "has-field")
		{
			Result = Value->HasField(QueryPartSpecifications[1]);
		}
		else if(QueryPartSpecifications[0] == "has-data")
		{
			return Value->GetData().empty() == false;
		}
		else if(QueryPartSpecifications[0] == "is-data")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			Result = Output.str() == QueryPartSpecifications[1];
		}
		else
		{
			assert(false);
		}
	}
	
	return Result;
}

void QueryWriter(std::unique_ptr< Inspection::Result > & Result, const std::string & Query)
{
	auto QueryParts{SplitString(Query.substr(1), '/')};
	auto Value{Result->GetValue()};
	
	for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
	{
		auto QueryPart{QueryParts[Index]};
		auto QueryPartSpecifications{SplitString(QueryPart, ':')};
		
		if(QueryPartSpecifications[0] == "field")
		{
			if(QueryPartSpecifications.size() == 2)
			{
				Value = Value->GetField(QueryPartSpecifications[1]);
			}
			else if(QueryPartSpecifications.size() == 3)
			{
				auto TestQuery{QueryPartSpecifications[2].substr(1, QueryPartSpecifications[2].size() - 2)};
				std::shared_ptr< Inspection::Value > MatchingField;
				
				for(auto Field : Value->GetFields())
				{
					if((Field->GetName() == QueryPartSpecifications[1]) && (EvaluateTestQuery(Field, TestQuery) == true))
					{
						MatchingField = Field;
						
						break;
					}
				}
				if(MatchingField == nullptr)
				{
					throw std::invalid_argument("The test \"" + TestQuery + "\" could not be satisfied by any field.");
				}
				else
				{
					Value = MatchingField;
				}
			}
			if(Index + 1 == QueryParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(QueryPartSpecifications[0] == "data")
		{
			if(QueryPartSpecifications.size() == 1)
			{
				std::cout << Value->GetData();
			}
			else
			{
				throw std::invalid_argument("The \"data\" query part specification does not accept any arguments.");
			}
		}
		else if(QueryPartSpecifications[0] == "tag")
		{
			Value = Value->GetTag(QueryPartSpecifications[1]);
			if(Index + 1 == QueryParts.size())
			{
				PrintValue(Value);
			}
		}
		else if(QueryPartSpecifications[0] == "has-tag")
		{
			if(Value->HasTag(QueryPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "has-field")
		{
			if(Value->HasField(QueryPartSpecifications[1]) == true)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "has-data")
		{
			if(Value->GetData().empty() == false)
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "is-value")
		{
			std::stringstream Output;
			
			Output << Value->GetData();
			if(Output.str() == QueryPartSpecifications[1])
			{
				std::cout << "true";
			}
			else
			{
				std::cout << "false";
			}
		}
		else if(QueryPartSpecifications[0] == "type")
		{
			assert(QueryPartSpecifications.size() == 1);
			std::cout << GetTypeName(Value->GetData().type());
		}
		else
		{
			assert(false);
		}
	}
}

int main(int argc, char ** argv)
{
	std::string GetterPrefix{"--getter="};
	std::string Getter;
	std::string QueryPrefix{"--query="};
	std::string Query;
	std::deque< std::string > Paths;
	auto Arguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < Arguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument.compare(0, QueryPrefix.size(), QueryPrefix) == 0)
		{
			Query = Argument.substr(QueryPrefix.size());
			if((Query.size() > 0) && (Query[0] != '/'))
			{
				std::cerr << "A --query has to be given as an absolute path." << std::endl;
				
				return 1;
			}
		}
		else if(Argument == "--verbose")
		{
			g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples = true;
		}
		else if(Argument.compare(0, GetterPrefix.size(), GetterPrefix) == 0)
		{
			Getter = Argument.substr(GetterPrefix.size());
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
		std::function< void (std::unique_ptr< Inspection::Result > &, Inspection::Reader &) > Writer{DefaultWriter};
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader &) > Processor{Process};
		
		if(Query != "")
		{
			Writer = std::bind(QueryWriter, std::placeholders::_1, Query);
		}
		if(Getter != "")
		{
			auto GetterParts{SplitString(Getter, '/')};
			
			Processor = std::bind(ProcessWithSpecificGetter, std::placeholders::_1, GetterParts);
		}
		ReadItem(Paths.front(), Processor, Writer);
		Paths.pop_front();
	}
	
	return 0;
}
