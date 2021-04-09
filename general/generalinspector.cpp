#include <deque>
#include <string>
#include <vector>

#include "any_printing.h"
#include "buffer.h"
#include "getters.h"
#include "inspector.h"
#include "query.h"
#include "result.h"
#include "type_repository.h"
#include "value_printing.h"

using namespace std::string_literals;

void AppendUnkownContinuation(std::shared_ptr< Inspection::Value > Value, Inspection::Reader & Reader)
{
	auto ErrorValue{Value->AppendField("error", "Unknown continuation."s)};
	
	ErrorValue->AddTag("position", to_string_cast(Reader.GetReadPositionInInput()));
	ErrorValue->AddTag("remaining length", to_string_cast(Reader.CalculateRemainingInputLength()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// the GeneralInspector is able to recognize these following forms:                              //
//                                                                                               //
// - most significant bit first:                                                                 //
//     - ID3v2Tag                                                                                //
//     - ID3v2Tag FLACStream                                                                     //
//     - ID3v2Tag FLACStream ID3v1Tag                                                            //
//     - ID3v2Tag MPEG1Stream                                                                    //
//     - ID3v2Tag MPEG1Stream APEv2Tag                                                           //
//     - ID3v2Tag MPEG1Stream APEv2Tag ID3v1Tag                                                  //
//     - ID3v2Tag MPEG1Stream ID3v1Tag                                                           //
//     - ID3v2Tag ID3v1Tag                                                                       //
//     - MPEG1Stream                                                                             //
//     - MPEG1Stream APEv2Tag                                                                    //
//     - MPEG1Stream APEv2Tag ID3v1Tag                                                           //
//     - MPEG1Stream ID3v1Tag                                                                    //
//     - APEv2Tag                                                                                //
//     - APEv2Tag ID3v1Tag                                                                       //
//     - ID3v1Tag                                                                                //
//     - FLACStream                                                                              //
//     - ASFFile                                                                                 //
//     - AppleSingle                                                                             //
//     - RIFFFile                                                                                //
//                                                                                               //
// - least significant bit first                                                                 //
//     - OggStream                                                                               //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Inspection
{
	class GeneralInspector : public Inspection::Inspector
	{
	public:
		void SetQuery(const std::string & Query)
		{
			_Query = Query;
		}
		
		void PushType(const std::vector<std::string> & Type)
		{
			_TypeSequence.emplace_back(Type);
		}
	protected:
		virtual std::unique_ptr<Inspection::Result> _Getter(const Inspection::Buffer & Buffer)
		{
			if(_TypeSequence.size() == 0)
			{
				return _GetGeneral(Buffer);
			}
			else
			{
				return _GetAsSpecificTypeSequence(Buffer);
			}
		}
		
		virtual void _Writer(std::unique_ptr<Inspection::Result> & Result) override
		{
			if(_Query != "")
			{
				_QueryWriter(Result->GetValue(), _Query);
			}
			else
			{
				Inspection::Inspector::_Writer(Result);
			}
		}
	private:
		std::unique_ptr<Inspection::Result> _GetGeneral(const Inspection::Buffer & Buffer)
		{
			auto Result = std::make_unique<Inspection::Result>();
			
			// reading most significant bit first
			if(Result->GetSuccess() == false)
			{
				auto Reader = Inspection::Reader{Buffer};
				
				Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
				Result = _GetGeneralMostSignificantBitFirst(Reader);
			}
			// reading least significant bit first
			if(Result->GetSuccess() == false)
			{
				auto Reader = Inspection::Reader{Buffer};
				
				Reader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
				Result = _GetGeneralLeastSignificantBitFirst(Reader);
			}
			
			return Result;
		}
		
		std::unique_ptr<Inspection::Result> _GetGeneralMostSignificantBitFirst(Inspection::Reader & Reader)
		{
			auto Result{Inspection::InitializeResult(Reader)};
			Inspection::Reader PartReader{Reader};
			auto PartResult{Inspection::Get_ID3_2_Tag(PartReader, {})};
			
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
					
					PartResult = Inspection::g_TypeRepository.Get({"FLAC", "Stream"}, PartReader, {});
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
							
							PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
						
						PartResult = Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {});
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
								auto PartResult{Inspection::g_TypeRepository.Get({"APE", "Tag"}, PartReader, {})};
								
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
										
										PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
									
									PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
							
							PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
				
				PartResult = Inspection::g_TypeRepository.Get({"MPEG", "1", "Stream"}, PartReader, {});
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
						auto PartResult{Inspection::g_TypeRepository.Get({"APE", "Tag"}, PartReader, {})};
						
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
								
								PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
							
							PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
					auto PartResult{Inspection::g_TypeRepository.Get({"APE", "Tag"}, PartReader, {})};
					
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
							
							PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
						
						PartResult = Inspection::g_TypeRepository.Get({"ID3", "v1", "Tag"}, PartReader, {});
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
							
							PartResult = Inspection::g_TypeRepository.Get({"FLAC", "Stream"}, PartReader, {});
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
								
								PartResult = Inspection::g_TypeRepository.Get({"ASF", "File"}, PartReader, {});
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
									auto PartResult{Inspection::g_TypeRepository.Get({"Apple", "AppleDouble_File"}, PartReader, {})};
									
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
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
		
		std::unique_ptr<Inspection::Result> _GetGeneralLeastSignificantBitFirst(Inspection::Reader & Reader)
		{
			auto Result = Inspection::InitializeResult(Reader);
			auto PartReader = Inspection::Reader{Reader};
			auto PartResult = Inspection::Get_Ogg_Stream(PartReader, {});
									
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
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}

		std::unique_ptr<Inspection::Result> _GetAsSpecificTypeSequence(const Inspection::Buffer & Buffer)
		{
			auto Reader = Inspection::Reader{Buffer};
			
			Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
			
			auto Result{Inspection::InitializeResult(Reader)};
			auto Continue{true};
			
			for(auto Index = 0ul; Index < _TypeSequence.size(); ++Index)
			{
				auto & TypeParts{_TypeSequence[Index]};
				Inspection::Reader PartReader{Reader};
				auto PartResult{Inspection::g_TypeRepository.Get(TypeParts, PartReader, {})};
				
				Continue = PartResult->GetSuccess();
				Result->GetValue()->AppendField(TypeParts.back(), PartResult->GetValue());
				Reader.AdvancePosition(PartReader.GetConsumedLength());
			}
			// finalization
			Result->SetSuccess(Continue);
			Inspection::FinalizeResult(Result, Reader);
			
			return Result;
		}
		
		std::vector<std::vector<std::string>> _TypeSequence;
		std::string _Query;
	};
}

int main(int argc, char ** argv)
{
	Inspection::GeneralInspector Inspector;
	std::string TypesPrefix{"--types="};
	std::string QueryPrefix{"--query="};
	auto NumberOfArguments{argc};
	auto ArgumentIndex{0};
	
	while(++ArgumentIndex < NumberOfArguments)
	{
		std::string Argument{argv[ArgumentIndex]};
		
		if(Argument.compare(0, QueryPrefix.size(), QueryPrefix) == 0)
		{
			auto Query{Argument.substr(QueryPrefix.size())};
			
			if((Query.size() > 0) && (Query[0] != '/'))
			{
				std::cerr << "A --query has to be given as an absolute path." << std::endl;
				
				return 1;
			}
			else
			{
				Inspector.SetQuery(Query);
			}
		}
		else if(Argument == "--verbose")
		{
			g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples = true;
		}
		else if(Argument.compare(0, TypesPrefix.size(), TypesPrefix) == 0)
		{
			auto Types{Argument.substr(TypesPrefix.size())};
			auto TypesParts{Inspection::SplitString(Types, ';')};
			
			for(auto & TypeParts : TypesParts)
			{
				Inspector.PushType(Inspection::SplitString(TypeParts, '/'));
			}
		}
		else
		{
			Inspector.PushPath(Argument);
		}
	}
	
	int Result{0};
	
	if(Inspector.GetPathCount() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;
		Result = 1;
	}
	else
	{
		Inspector.Process();
	}
	
	return Result;
}
