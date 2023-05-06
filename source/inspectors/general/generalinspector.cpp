#include <deque>
#include <sstream>
#include <string>
#include <vector>

#include <string_cast/string_cast.h>

#include <common/getters.h>
#include <common/helper.h>
#include <common/inspector.h>
#include <common/query.h>
#include <common/result.h>
#include <common/type.h>
#include <common/type_repository.h>

using namespace std::string_literals;

void AppendUnkownContinuation(Inspection::Value * Value, Inspection::Reader & Reader)
{
    auto ErrorValue = Value->AppendField("error", "Unknown continuation."s);
    
    ErrorValue->AddTag("position", to_string_cast(Reader.GetReadPositionInInput()));
    ErrorValue->AddTag("remaining length", to_string_cast(Reader.CalculateRemainingInputLength()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// the GeneralInspector is able to recognize these following forms:                              //
//                                                                                               //
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
//     - OggStream                                                                               //
//     - WavPackBlocks                                                                           //
//     - WavPackBlocks APEv2Tag                                                                  //
//     - RIFFFile                                                                                //
//                                                                                               //
///////////////////////////////////////////////////////////////////////////////////////////////////

namespace Inspection
{
    class GeneralInspector : public Inspection::Inspector
    {
    public:
        GeneralInspector(void) :
            _TopLevel{false}
        {
        }
        
        void SetQuery(const std::string & Query)
        {
            _Query = Query;
        }
        
        void SetTopLevel(bool TopLevel)
        {
            _TopLevel = TopLevel;
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
                if(_TopLevel == true)
                {
                    for(auto & TopLevelField : Result->GetValue()->GetFields())
                    {
                        TopLevelField->ClearFields();
                    }
                }
                Inspection::Inspector::_Writer(Result);
            }
        }
    private:
        std::unique_ptr<Inspection::Result> _GetGeneral(const Inspection::Buffer & Buffer)
        {
            auto Result = std::make_unique<Inspection::Result>();
            auto MSBFReader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
            auto PartReader = Inspection::Reader{MSBFReader};
            auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v2_Tag"});
            auto PartResult = PartType->Get(PartReader, {});
            
            if(PartResult->GetSuccess() == true)
            {
                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                if(MSBFReader.IsAtEnd() == true)
                {
                    Result->SetSuccess(true);
                }
                else
                {
                    auto PartReader = Inspection::Reader{MSBFReader};
                    auto PartType = Inspection::g_TypeRepository.GetType({"FLAC", "Stream"});
                    auto PartResult = PartType->Get(PartReader, {});
                    
                    if(PartResult->GetSuccess() == true)
                    {
                        Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                        MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                        if(MSBFReader.IsAtEnd() == true)
                        {
                            Result->SetSuccess(true);
                        }
                        else
                        {
                            auto PartReader = Inspection::Reader{MSBFReader};
                            auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                            auto PartResult = PartType->Get(PartReader, {});
                            
                            if(PartResult->GetSuccess() == true)
                            {
                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                if(MSBFReader.IsAtEnd() == true)
                                {
                                    Result->SetSuccess(true);
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                            else
                            {
                                
                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                            }
                        }
                    }
                    else
                    {
                        auto PartReader = Inspection::Reader{MSBFReader};
                        auto PartType = Inspection::g_TypeRepository.GetType({"MPEG", "1", "Stream"});
                        auto PartResult = PartType->Get(PartReader, {});
                        
                        if(PartResult->GetSuccess() == true)
                        {
                            Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                            MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                            if(MSBFReader.IsAtEnd() == true)
                            {
                                Result->SetSuccess(true);
                            }
                            else
                            {
                                auto PartReader = Inspection::Reader{MSBFReader};
                                auto PartType = Inspection::g_TypeRepository.GetType({"APE", "Tag"});
                                auto PartResult = PartType->Get(PartReader, {});
                                
                                if(PartResult->GetSuccess() == true)
                                {
                                    Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                    MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                    if(MSBFReader.IsAtEnd() == true)
                                    {
                                        Result->SetSuccess(true);
                                    }
                                    else
                                    {
                                        auto PartReader = Inspection::Reader{MSBFReader};
                                        auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                                        auto PartResult = PartType->Get(PartReader, {});
                                        
                                        if(PartResult->GetSuccess() == true)
                                        {
                                            Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                            MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                            if(MSBFReader.IsAtEnd() == true)
                                            {
                                                Result->SetSuccess(true);
                                            }
                                            else
                                            {
                                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                            }
                                        }
                                        else
                                        {
                                            AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                        }
                                    }
                                }
                                else
                                {
                                    auto PartReader = Inspection::Reader{MSBFReader};
                                    auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                                    auto PartResult = PartType->Get(PartReader, {});
                                    
                                    if(PartResult->GetSuccess() == true)
                                    {
                                        Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                        MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                        if(MSBFReader.IsAtEnd() == true)
                                        {
                                            Result->SetSuccess(true);
                                        }
                                        else
                                        {
                                            AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                        }
                                    }
                                    else
                                    {
                                        AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                    }
                                }
                            }
                        }
                        else
                        {
                            auto PartReader = Inspection::Reader{MSBFReader};
                            auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                            auto PartResult = PartType->Get(PartReader, {});
                            
                            if(PartResult->GetSuccess() == true)
                            {
                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                if(MSBFReader.IsAtEnd() == true)
                                {
                                    Result->SetSuccess(true);
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                            else
                            {
                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                            }
                        }
                    }
                }
            }
            else
            {
                auto PartReader = Inspection::Reader{MSBFReader};
                auto PartType = Inspection::g_TypeRepository.GetType({"MPEG", "1", "Stream"});
                auto PartResult = PartType->Get(PartReader, {});
                
                if(PartResult->GetSuccess() == true)
                {
                    Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                    MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                    if(MSBFReader.IsAtEnd() == true)
                    {
                        Result->SetSuccess(true);
                    }
                    else
                    {
                        auto PartReader = Inspection::Reader{MSBFReader};
                        auto PartType = Inspection::g_TypeRepository.GetType({"APE", "Tag"});
                        auto PartResult = PartType->Get(PartReader, {});
                        
                        if(PartResult->GetSuccess() == true)
                        {
                            Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                            MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                            if(MSBFReader.IsAtEnd() == true)
                            {
                                Result->SetSuccess(true);
                            }
                            else
                            {
                                auto PartReader = Inspection::Reader{MSBFReader};
                                auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                                auto PartResult = PartType->Get(PartReader, {});
                                
                                if(PartResult->GetSuccess() == true)
                                {
                                    Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                    MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                    if(MSBFReader.IsAtEnd() == true)
                                    {
                                        Result->SetSuccess(true);
                                    }
                                    else
                                    {
                                        AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                    }
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                        }
                        else
                        {
                            auto PartReader = Inspection::Reader{MSBFReader};
                            auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                            auto PartResult = PartType->Get(PartReader, {});
                            
                            if(PartResult->GetSuccess() == true)
                            {
                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                if(MSBFReader.IsAtEnd() == true)
                                {
                                    Result->SetSuccess(true);
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                            else
                            {
                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                            }
                        }
                    }
                }
                else
                {
                    auto PartReader = Inspection::Reader{MSBFReader};
                    auto PartType = Inspection::g_TypeRepository.GetType({"APE", "Tag"});
                    auto PartResult = PartType->Get(PartReader, {});
                    
                    if(PartResult->GetSuccess() == true)
                    {
                        Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                        MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                        if(MSBFReader.IsAtEnd() == true)
                        {
                            Result->SetSuccess(true);
                        }
                        else
                        {
                            auto PartReader = Inspection::Reader{MSBFReader};
                            auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                            auto PartResult = PartType->Get(PartReader, {});
                            
                            if(PartResult->GetSuccess() == true)
                            {
                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                if(MSBFReader.IsAtEnd() == true)
                                {
                                    Result->SetSuccess(true);
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                            else
                            {
                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                            }
                        }
                    }
                    else
                    {
                        auto PartReader = Inspection::Reader{MSBFReader};
                        auto PartType = Inspection::g_TypeRepository.GetType({"ID3", "v1", "Tag"});
                        auto PartResult = PartType->Get(PartReader, {});
                        
                        if(PartResult->GetSuccess() == true)
                        {
                            Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                            MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                            if(MSBFReader.IsAtEnd() == true)
                            {
                                Result->SetSuccess(true);
                            }
                            else
                            {
                                AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                            }
                        }
                        else
                        {
                            auto PartReader = Inspection::Reader{MSBFReader};
                            auto PartType = Inspection::g_TypeRepository.GetType({"FLAC", "Stream"});
                            auto PartResult = PartType->Get(PartReader, {});
                            
                            if(PartResult->GetSuccess() == true)
                            {
                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                if(MSBFReader.IsAtEnd() == true)
                                {
                                    Result->SetSuccess(true);
                                }
                                else
                                {
                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                }
                            }
                            else
                            {
                                auto PartReader = Inspection::Reader{MSBFReader};
                                auto PartType = Inspection::g_TypeRepository.GetType({"ASF", "File"});
                                auto PartResult = PartType->Get(PartReader, {});
                                
                                if(PartResult->GetSuccess() == true)
                                {
                                    Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                    MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                    if(MSBFReader.IsAtEnd() == true)
                                    {
                                        Result->SetSuccess(true);
                                    }
                                    else
                                    {
                                        AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                    }
                                }
                                else
                                {
                                    auto PartReader = Inspection::Reader{MSBFReader};
                                    auto PartType = Inspection::g_TypeRepository.GetType({"Apple", "AppleDouble_File"});
                                    auto PartResult = PartType->Get(PartReader, {});
                                    
                                    if(PartResult->GetSuccess() == true)
                                    {
                                        Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                        MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                        if(MSBFReader.IsAtEnd() == true)
                                        {
                                            Result->SetSuccess(true);
                                        }
                                        else
                                        {
                                            AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                        }
                                    }
                                    else
                                    {
                                        auto LSBFReader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
                                        
                                        LSBFReader.SetBitstreamType(Inspection::Reader::BitstreamType::LeastSignificantBitFirst);
                                        
                                        auto PartReader = Inspection::Reader{LSBFReader};
                                        auto PartType = Inspection::g_TypeRepository.GetType({"Ogg", "Stream"});
                                        auto PartResult = PartType->Get(PartReader, {});
                                        
                                        if(PartResult->GetSuccess() == true)
                                        {
                                            Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                            LSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                            if(LSBFReader.IsAtEnd() == true)
                                            {
                                                Result->SetSuccess(true);
                                            }
                                            else
                                            {
                                                AppendUnkownContinuation(Result->GetValue(), LSBFReader);
                                            }
                                        }
                                        else
                                        {
                                            auto PartReader = Inspection::Reader{MSBFReader};
                                            auto PartType = Inspection::g_TypeRepository.GetType({"WavPack", "Blocks"});
                                            auto PartResult = PartType->Get(PartReader, {});
                                            
                                            if(PartResult->GetSuccess() == true)
                                            {
                                                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                                MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                                if(MSBFReader.IsAtEnd() == true)
                                                {
                                                    Result->SetSuccess(true);
                                                }
                                                else
                                                {
                                                    auto PartReader = Inspection::Reader{MSBFReader};
                                                    auto PartType = Inspection::g_TypeRepository.GetType({"APE", "Tag"});
                                                    auto PartResult = PartType->Get(PartReader, {});
                                                    
                                                    if(PartResult->GetSuccess() == true)
                                                    {
                                                        Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                                        MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                                        if(MSBFReader.IsAtEnd() == true)
                                                        {
                                                            Result->SetSuccess(true);
                                                        }
                                                        else
                                                        {
                                                            AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                                        }
                                                    }
                                                    else
                                                    {
                                                        AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                                    }
                                                }
                                            }
                                            else
                                            {
                                                auto PartReader = Inspection::Reader{MSBFReader};
                                                auto PartType = Inspection::g_TypeRepository.GetType({"RIFF", "Chunk"});
                                                auto PartResult = PartType->Get(PartReader, {});
                                                
                                                if(PartResult->GetSuccess() == true)
                                                {
                                                    Result->GetValue()->AppendField(Inspection::JoinWithSeparator(PartType->GetPathParts(), "."), PartResult->ExtractValue());
                                                    MSBFReader.AdvancePosition(PartReader.GetConsumedLength());
                                                    if(MSBFReader.IsAtEnd() == true)
                                                    {
                                                        Result->SetSuccess(true);
                                                    }
                                                    else
                                                    {
                                                        AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                                    }
                                                }
                                                else
                                                {
                                                    AppendUnkownContinuation(Result->GetValue(), MSBFReader);
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            return Result;
        }

        std::unique_ptr<Inspection::Result> _GetAsSpecificTypeSequence(const Inspection::Buffer & Buffer)
        {
            auto Reader = Inspection::Reader{Buffer, Inspection::Length{0, 0}, Buffer.GetLength()};
            
            Reader.SetBitstreamType(Inspection::Reader::BitstreamType::MostSignificantBitFirst);
            
            auto Result = std::make_unique<Inspection::Result>();
            auto Continue = true;
            
            for(auto Index = 0ul; Index < _TypeSequence.size(); ++Index)
            {
                auto & TypeParts = _TypeSequence[Index];
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::g_TypeRepository.Get(TypeParts, PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField(Inspection::JoinWithSeparator(TypeParts, "."), PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            if(Continue == true)
            {
                if(Reader.IsAtEnd() == false)
                {
                    auto MoreDataValue = _AppendLengthField(Result->GetValue(), "MoreData", Reader.CalculateRemainingInputLength());
                    
                    MoreDataValue->AddTag("error", "Additional data after the parsed data."s);
                    Continue = false;
                }
            }
            // finalization
            Result->SetSuccess(Continue);
            
            return Result;
        }
        
        bool _TopLevel;
        std::vector<std::vector<std::string>> _TypeSequence;
        std::string _Query;
    };
}

int main(int argc, char ** argv)
{
    auto Inspector = Inspection::GeneralInspector{};
    auto TypesPrefix = "--types="s;
    auto QueryPrefix = "--query="s;
    auto NumberOfArguments = argc;
    auto ArgumentIndex = 0;
    auto Result = 0;
    
    while(++ArgumentIndex < NumberOfArguments)
    {
        auto Argument = std::string{argv[ArgumentIndex]};
        
        if(Argument.compare(0, QueryPrefix.size(), QueryPrefix) == 0)
        {
            auto Query = Argument.substr(QueryPrefix.size());
            
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
        else if(Argument == "--top-level")
        {
            Inspector.SetTopLevel(true);
        }
        else if(Argument.compare(0, TypesPrefix.size(), TypesPrefix) == 0)
        {
            auto Types = Argument.substr(TypesPrefix.size());
            auto TypesParts = Inspection::SplitString(Types, ';');
            
            for(auto & TypeParts : TypesParts)
            {
                Inspector.PushType(Inspection::SplitString(TypeParts, '/'));
            }
        }
        else if(Argument == "--read-stdin")
        {
            if(Inspector.GetReadFromStandardInput() == false)
            {
                Inspector.SetReadFromStandardInput();
            }
            else
            {
                Result = 1;
            }
        }
        else
        {
            Inspector.PushPath(Argument);
        }
    }
    if((Result != 0) || ((Inspector.GetPathCount() == 0) && (Inspector.GetReadFromStandardInput() == false)))
    {
        std::cerr << "Usage: " << argv[0] << " [<paths>|--] [options]\n";
        std::cerr << "    --read-stdin        read from standard input\n";
        std::cerr << "    --query=<query>     query for some specific data\n";
        std::cerr << "    --top-level         prints only the top-level fields\n";
        std::cerr << "    --types=<type>,...  forces interpretation as a sequence of specific types\n";
        std::cerr << "    --verbose           more appended fields on some type getters\n";
        if(Result == 0)
        {
            Result = 1;
        }
    }
    if(Result == 0)
    {
        Result = ((Inspector.Process() == true) ? (0) : (1));
    }
    
    return Result;
}
