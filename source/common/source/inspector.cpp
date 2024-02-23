#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <string_cast/string_cast.h>

#include "assertion.h"
#include "buffer.h"
#include "colors.h"
#include "inspector.h"
#include "internal_output_operators.h"
#include "output_operators.h"
#include "query.h"
#include "reader.h"

using namespace std::string_literals;

Inspection::Inspector::Inspector() :
    m_ReadFromStandardInput{false}
{
}

Inspection::Inspector::~Inspector(void)
{
}

std::uint_fast32_t Inspection::Inspector::GetPathCount(void) const
{
    return _Paths.size();
}

auto Inspection::Inspector::GetReadFromStandardInput() -> bool
{
    return m_ReadFromStandardInput;
}

bool Inspection::Inspector::Process(void)
{
    auto Result = true;
    
    if(m_ReadFromStandardInput == true)
    {
        auto MemoryBuffer = m_ReadBufferFromStandardInput();
        auto Buffer = Inspection::Buffer{&(*MemoryBuffer.cbegin()), Inspection::Length{MemoryBuffer.size(), 0}};
        
        m_ProcessBuffer(Buffer, "StandardInput");
    }
    else
    {
        for(auto Path : _Paths)
        {
            try
            {
                Result &= _ProcessPath(std::filesystem::directory_entry(Path));
            }
            catch(std::exception const & Exception)
            {
                std::cerr << Exception << '\n';
                Result = false;
            }
        }
    }
    
    return Result;
}

void Inspection::Inspector::PushPath(const std::filesystem::path & Path)
{
    _Paths.push_back(Path);
}

auto Inspection::Inspector::m_ReadBufferFromStandardInput() -> std::vector<std::uint8_t>
{
    std::freopen(nullptr, "rb", stdin);
    
    auto BytesRead = std::vector<std::uint8_t>::size_type{0};
    auto BufferSize = std::vector<std::uint8_t>::size_type{1024};
    auto Buffer = std::vector<std::uint8_t>{};
    
    do
    {
        Buffer.resize(BufferSize);
        std::cin.read(reinterpret_cast<char *>(&(*(Buffer.begin() + BytesRead))), BufferSize - BytesRead);
        BytesRead += std::cin.gcount();
        BufferSize *= 2;
    } while(BytesRead == Buffer.size());
    Buffer.resize(BytesRead);
    
    return Buffer;
}

auto Inspection::Inspector::SetReadFromStandardInput() -> void
{
    m_ReadFromStandardInput = true;
}

auto Inspection::Inspector::m_ProcessBuffer(const Inspection::Buffer & Buffer, std::string_view Name) -> bool
{
    auto FileResult = std::make_unique<Inspection::Result>();
    
    FileResult->GetValue()->SetName(Inspection::g_BrightGreen + std::string{Name} + Inspection::g_BrightWhite);
    
    auto LengthTag = FileResult->GetValue()->AddTag("length", Buffer.GetLength());
    
    LengthTag->AddTag("unit", "bytes and bits"s);
    
    auto InnerResult = _Getter(Buffer);
    
    FileResult->GetValue()->Extend(InnerResult->ExtractValue());
    FileResult->SetSuccess(InnerResult->GetSuccess());
    _Writer(FileResult);
    
    return FileResult->GetSuccess();
}

bool Inspection::Inspector::_ProcessPath(const std::filesystem::directory_entry & DirectoryEntry)
{
    auto Result = true;
    
    if(DirectoryEntry.exists() == true)
    {
        if(DirectoryEntry.is_directory() == true)
        {
            for(auto & DirectoryEntry : std::filesystem::recursive_directory_iterator(DirectoryEntry))
            {
                if(DirectoryEntry.is_regular_file() == true)
                {
                    Result &= _ProcessFile(DirectoryEntry);
                }
            }
        }
        else if(DirectoryEntry.is_regular_file() == true)
        {
            Result &= _ProcessFile(DirectoryEntry);
        }
        else
        {
            throw std::runtime_error('"' + DirectoryEntry.path().string() + "\" is no file or directory!");
            Result = false;
        }
    }
    else
    {
        throw std::runtime_error('"' + DirectoryEntry.path().string() + "\" does not exist!");
        Result = false;
    }
    
    return Result;
}

bool Inspection::Inspector::_ProcessFile(const std::filesystem::directory_entry & DirectoryEntry)
{
    auto Result = false;
    auto & Path{DirectoryEntry.path()};
    auto FileDescriptor{open(Path.c_str(), O_RDONLY)};
    
    if(FileDescriptor == -1)
    {
        throw std::runtime_error("Could not open the file \"" + Path.string() + "\".");
    }
    else
    {
        auto FileSize{DirectoryEntry.file_size()};
        auto Address{static_cast< std::uint8_t * >(mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0))};
        
        if(Address == MAP_FAILED)
        {
            throw std::runtime_error("Could not map the file \"" + Path.string() + "\" into memory.");
        }
        else
        {
            auto Buffer = Inspection::Buffer{Address, Inspection::Length(FileSize, 0)};
            
            Result = m_ProcessBuffer(Buffer, Path.string());
            munmap(Address, FileSize);
        }
        close(FileDescriptor);
    }
    
    return Result;
}

void Inspection::Inspector::_Writer(std::unique_ptr<Inspection::Result> & Result)
{
    std::cout << *(Result->GetValue());
    if(Result->GetSuccess() == false)
    {
        std::cout << Inspection::g_BrightRed << "Parsing does not give valid." << Inspection::g_BrightWhite << std::endl;
    }
}

void Inspection::Inspector::_QueryWriter(Inspection::Value * Value, const std::string & Query)
{
    auto QueryParts{Inspection::SplitString(Query.substr(1), '/')};
    
    for(auto Index = 0ul; Index < QueryParts.size(); ++Index)
    {
        auto QueryPart{QueryParts[Index]};
        auto QueryPartSpecifications{Inspection::SplitString(QueryPart, ':')};
        
        if(QueryPartSpecifications[0] == "field")
        {
            if(QueryPartSpecifications.size() == 2)
            {
                Value = Value->GetField(QueryPartSpecifications[1]);
            }
            else if(QueryPartSpecifications.size() == 3)
            {
                auto MatchingField = static_cast<Inspection::Value *>(nullptr);
                
                if((QueryPartSpecifications[2][0] == '[') && (QueryPartSpecifications[2][QueryPartSpecifications[2].size() - 1] == ']'))
                {
                    auto TestQuery{QueryPartSpecifications[2].substr(1, QueryPartSpecifications[2].size() - 2)};
                    
                    for(auto & Field : Value->GetFields())
                    {
                        if((Field->GetName() == QueryPartSpecifications[1]) && (Inspection::EvaluateTestQuery(Field.get(), TestQuery) == true))
                        {
                            MatchingField = Field.get();
                            
                            break;
                        }
                    }
                }
                else
                {
                    auto WantedIndex{from_string_cast< std::uint64_t >(QueryPartSpecifications[2])};
                    std::uint64_t Index{0};
                    
                    for(auto & Field : Value->GetFields())
                    {
                        if(Field->GetName() == QueryPartSpecifications[1])
                        {
                            if(WantedIndex == Index)
                            {
                                MatchingField = Field.get();
                                
                                break;
                            }
                            else
                            {
                                ++Index;
                            }
                        }
                    }
                }
                if(MatchingField == nullptr)
                {
                    throw std::invalid_argument("The test \"" + QueryPartSpecifications[2] + "\" could not be satisfied by any field.");
                }
                else
                {
                    Value = MatchingField;
                }
            }
            if(Index + 1 == QueryParts.size())
            {
                std::cout << *Value;
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
                std::cout << *Value;
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
            if(QueryPartSpecifications.size() == 2)
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
            else if(QueryPartSpecifications.size() == 3)
            {
                auto MatchingField = static_cast<Inspection::Value *>(nullptr);
                
                if((QueryPartSpecifications[2][0] == '[') && (QueryPartSpecifications[2][QueryPartSpecifications[2].size() - 1] == ']'))
                {
                    auto TestQuery = QueryPartSpecifications[2].substr(1, QueryPartSpecifications[2].size() - 2);
                    
                    for(auto & Field : Value->GetFields())
                    {
                        if((Field->GetName() == QueryPartSpecifications[1]) && (Inspection::EvaluateTestQuery(Field.get(), TestQuery) == true))
                        {
                            MatchingField = Field.get();
                            
                            break;
                        }
                    }
                }
                else
                {
                    auto WantedIndex = from_string_cast<std::uint64_t>(QueryPartSpecifications[2]);
                    auto Index = static_cast<std::uint64_t>(0);
                    
                    for(auto & Field : Value->GetFields())
                    {
                        if(Field->GetName() == QueryPartSpecifications[1])
                        {
                            if(WantedIndex == Index)
                            {
                                MatchingField = Field.get();
                                
                                break;
                            }
                            else
                            {
                                ++Index;
                            }
                        }
                    }
                }
                if(MatchingField != nullptr)
                {
                    std::cout << "true";
                }
                else
                {
                    std::cout << "false";
                }
            }
        }
        else if(QueryPartSpecifications[0] == "has-data")
        {
            if(Value->GetData().has_value() == true)
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
            ASSERTION(QueryPartSpecifications.size() == 1);
            std::cout << Inspection::to_string(Value->GetData().type());
        }
        else
        {
            UNEXPECTED_CASE("QueryPartSpecifications[0] == \"" + QueryPartSpecifications[0] + '"');
        }
    }
}

Inspection::Value * Inspection::Inspector::_AppendOtherData(Inspection::Value * Value, const Inspection::Length & Length)
{
    return _AppendLengthField(Value, "OtherData", Length);
}

Inspection::Value * Inspection::Inspector::_AppendLengthField(Inspection::Value * Value, const std::string & FieldName, const Inspection::Length & Length)
{
    auto Result = Value->AppendField(FieldName);
    auto LengthTag = Result->AddTag("length", Length);
    
    LengthTag->AddTag("unit", "bytes and bits"s);
    
    return Result;
}
