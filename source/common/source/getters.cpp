#include <any>
#include <bitset>
#include <functional>
#include <optional>
#include <sstream>
#include <vector>

#include <string_cast/string_cast.h>

#include "assertion.h"
#include "buffer.h"
#include "getters.h"
#include "guid.h"
#include "helper.h"
#include "id3_de_unsynchronization_eager_filter.h"
#include "id3_helper.h"
#include "internal_output_operators.h"
#include "reader.h"
#include "type.h"
#include "type_repository.h"
#include "unknown_value_exception.h"

using namespace std::string_literals;

bool g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples{false};

Inspection::Value * AppendLengthTag(Inspection::Value * Value, const Inspection::Length & Length, const std::string & LengthName = "length"s)
{
    auto Result = Value->AddTag(LengthName, Length);
    
    Result->AddTag("unit", "bytes and bits"s);
    
    return Result;
}

Inspection::Value * AppendReadErrorTag(Inspection::Value * Value, const Inspection::ReadResult & ReadResult)
{
    ASSERTION(ReadResult.Success == false);
    
    auto Result = Value->AddTag("error", "Could not read enough data."s);
    
    AppendLengthTag(Result, ReadResult.RequestedLength, "requested length");
    AppendLengthTag(Result, ReadResult.InputLength, "remaining length");
    
    return Result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Readers & Getters                                                                             //
///////////////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Inspection::Result> Inspection::Get_Apple_AppleDouble_File(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"Apple", "AppleDouble_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Header", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & EntryDescriptorFields = Result->GetValue()->GetField("Header")->GetField("EntryDescriptors")->GetFields();
        
        if(EntryDescriptorFields.size() > 0)
        {
            auto EntriesField = Result->GetValue()->AppendField("Entries");
            
            EntriesField->AddTag("array"s);
            
            auto EntryDescriptorFieldIterator = std::begin(EntryDescriptorFields);
            auto FurthestReader = std::unique_ptr<Inspection::Reader>{};
            auto EntryIndex = 0;
            
            while((Continue == true) && (EntryDescriptorFieldIterator != std::end(EntryDescriptorFields)))
            {
                auto EntryDescriptorField = EntryDescriptorFieldIterator->get();
                auto EntryReader = std::make_unique<Inspection::Reader>(Reader, Inspection::Length{std::any_cast<std::uint32_t>(EntryDescriptorField->GetField("Offset")->GetData()), 0}, Inspection::Length{std::any_cast<std::uint32_t>(EntryDescriptorField->GetField("Length")->GetData()), 0});
                auto EntryResult = std::unique_ptr<Inspection::Result>{};
                
                switch(std::any_cast<std::uint32_t>(EntryDescriptorField->GetField("EntryID")->GetData()))
                {
                case 9:
                    {
                        EntryResult = Inspection::g_TypeRepository.GetType({"Apple", "AppleSingleDouble_Entry_FinderInfo"})->Get(*EntryReader, {});
                        
                        break;
                    }
                default:
                    {
                        EntryResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(*EntryReader, {});
                        
                        break;
                    }
                }
                Continue = EntryResult->GetSuccess();
                
                auto EntryField = EntriesField->AppendField("Entry", EntryResult->ExtractValue());
                
                EntryField->AddTag("element index in array", EntryIndex);
                if((Continue == true) && ((FurthestReader == nullptr) || (EntryReader->GetConsumedLength() > FurthestReader->GetConsumedLength())))
                {
                    FurthestReader = std::move(EntryReader);
                }
                ++EntryDescriptorFieldIterator;
                ++EntryIndex;
            }
            if(Continue == true)
            {
                EntriesField->AddTag("ended by number of elements"s);
            }
            else
            {
                EntriesField->AddTag("ended by failure"s);
            }
            EntriesField->AddTag("number of elements", EntryIndex);
            Reader.AdvancePosition(FurthestReader->GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("array"s);
    // reading
    if(Continue == true)
    {
        std::optional<std::string> ElementName;
        std::unordered_map<std::string, std::any> ElementParameters;
        auto ElementParametersIterator{Parameters.find("ElementParameters")};
        
        if(ElementParametersIterator != Parameters.end())
        {
            const auto & ElementParametersFromParameter{std::any_cast< const std::unordered_map<std::string, std::any> & >(ElementParametersIterator->second)};
            
            ElementParameters.insert(std::begin(ElementParametersFromParameter), std::end(ElementParametersFromParameter));
        }
        
        auto ElementNameIterator{Parameters.find("ElementName")};
        
        if(ElementNameIterator != Parameters.end())
        {
            ElementName = std::any_cast< std::string >(ElementNameIterator->second);
        }
        
        auto ElementType{std::any_cast< const Inspection::TypeDefinition::Type * >(Parameters.at("ElementType"))};
        std::uint64_t ElementIndexInArray{0};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            Inspection::Reader ElementReader{Reader};
            
            ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
            
            auto ElementResult{ElementType->Get(ElementReader, ElementParameters)};
            
            Continue = ElementResult->GetSuccess();
            if(Continue == true)
            {
                ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                if(ElementName)
                {
                    Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
                }
                else
                {
                    Result->GetValue()->AppendField(ElementResult->ExtractValue());
                }
                Reader.AdvancePosition(ElementReader.GetConsumedLength());
            }
            else
            {
                Continue = true;
                
                break;
            }
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        else
        {
            Result->GetValue()->AddTag("ended by failure"s);
        }
        if(ElementIndexInArray > 0)
        {
            Result->GetValue()->AddTag("at least one element"s);
        }
        else
        {
            Result->GetValue()->AddTag("error", "The array contains no elements, although at least one is required."s);
            Continue = false;
        }
        Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Array_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("array"s);
    // reading
    if(Continue == true)
    {
        std::optional<std::string> ElementName;
        std::unordered_map<std::string, std::any> ElementParameters;
        auto ElementParametersIterator{Parameters.find("ElementParameters")};
        
        if(ElementParametersIterator != Parameters.end())
        {
            const auto & ElementParametersFromParameter{std::any_cast< const std::unordered_map<std::string, std::any> & >(ElementParametersIterator->second)};
            
            ElementParameters.insert(std::begin(ElementParametersFromParameter), std::end(ElementParametersFromParameter));
        }
        
        auto ElementNameIterator{Parameters.find("ElementName")};
        
        if(ElementNameIterator != Parameters.end())
        {
            ElementName = std::any_cast< std::string >(ElementNameIterator->second);
        }
        
        auto ElementType{std::any_cast< const Inspection::TypeDefinition::Type * >(Parameters.at("ElementType"))};
        std::uint64_t ElementIndexInArray{0};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            Inspection::Reader ElementReader{Reader};
            
            ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
            
            auto ElementResult{ElementType->Get(ElementReader, ElementParameters)};
            
            Continue = ElementResult->GetSuccess();
            ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
            if(ElementName)
            {
                Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
            }
            else
            {
                Result->GetValue()->AppendField(ElementResult->ExtractValue());
            }
            Reader.AdvancePosition(ElementReader.GetConsumedLength());
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        else
        {
            Result->GetValue()->AddTag("ended by failure"s);
        }
        Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Array_EndedByNumberOfElements(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("array"s);
    // reading
    if(Continue == true)
    {
        std::optional<std::string> ElementName;
        std::unordered_map<std::string, std::any> ElementParameters;
        auto ElementParametersIterator{Parameters.find("ElementParameters")};
        
        if(ElementParametersIterator != Parameters.end())
        {
            const auto & ElementParametersFromParameter{std::any_cast< const std::unordered_map<std::string, std::any> & >(ElementParametersIterator->second)};
            
            ElementParameters.insert(std::begin(ElementParametersFromParameter), std::end(ElementParametersFromParameter));
        }
        
        auto ElementNameIterator{Parameters.find("ElementName")};
        
        if(ElementNameIterator != Parameters.end())
        {
            ElementName = std::any_cast< std::string >(ElementNameIterator->second);
        }
        
        auto ElementType{std::any_cast< const Inspection::TypeDefinition::Type * >(Parameters.at("ElementType"))};
        auto NumberOfElements{std::any_cast<std::uint64_t>(Parameters.at("NumberOfElements"))};
        std::uint64_t ElementIndexInArray{0};
        
        while(true)
        {
            if(ElementIndexInArray < NumberOfElements)
            {
                Inspection::Reader ElementReader{Reader};
                
                ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
                
                auto ElementResult{ElementType->Get(ElementReader, ElementParameters)};
                
                Continue = ElementResult->GetSuccess();
                ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                if(ElementName)
                {
                    Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
                }
                else
                {
                    Result->GetValue()->AppendField(ElementResult->ExtractValue());
                }
                Reader.AdvancePosition(ElementReader.GetConsumedLength());
                if(Continue == false)
                {
                    Result->GetValue()->AddTag("ended by failure"s);
                    
                    break;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by number of elements"s);
                
                break;
            }
        }
        Result->GetValue()->AddTag("number of elements", NumberOfElements);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Array_EndedByPredicate(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("array"s);
    // reading
    if(Continue == true)
    {
        std::optional<std::string> ElementName;
        std::unordered_map<std::string, std::any> ElementParameters;
        auto ElementParametersIterator{Parameters.find("ElementParameters")};
        
        if(ElementParametersIterator != Parameters.end())
        {
            const auto & ElementParametersFromParameter{std::any_cast< const std::unordered_map<std::string, std::any> & >(ElementParametersIterator->second)};
            
            ElementParameters.insert(std::begin(ElementParametersFromParameter), std::end(ElementParametersFromParameter));
        }
        
        auto ElementNameIterator{Parameters.find("ElementName")};
        
        if(ElementNameIterator != Parameters.end())
        {
            ElementName = std::any_cast< std::string >(ElementNameIterator->second);
        }
        
        auto EndPredicate = std::any_cast<std::function<bool (Inspection::Value *)>>(Parameters.at("EndPredicate"));
        const auto ElementType = std::any_cast<const Inspection::TypeDefinition::Type *>(Parameters.at("ElementType"));
        auto ElementIndexInArray = std::uint64_t{0};
        
        while(Continue == true)
        {
            Inspection::Reader ElementReader{Reader};
            
            ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
            
            auto ElementResult = ElementType->Get(ElementReader, ElementParameters);
            // required for predicate processing
            auto ElementValue = ElementResult->GetValue();
            
            Continue = ElementResult->GetSuccess();
            ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
            if(ElementName)
            {
                Result->GetValue()->AppendField(ElementName.value(), ElementResult->ExtractValue());
            }
            else
            {
                Result->GetValue()->AppendField(ElementResult->ExtractValue());
            }
            Reader.AdvancePosition(ElementReader.GetConsumedLength());
            if(Continue == true)
            {
                if(EndPredicate(ElementValue) == true)
                {
                    Result->GetValue()->AddTag("ended by predicate"s);
                    
                    break;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by failure"s);
            }
        }
        Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_Character_Alphabetic(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("character");
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphabetic");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            if(Is_ASCII_Character_Alphabetic(ReadResult.Data) == true)
            {
                Result->GetValue()->SetData(ReadResult.Data);
            }
            else
            {
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_Character_AlphaNumeric(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("character");
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphanumeric");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            if((Is_ASCII_Character_Alphabetic(ReadResult.Data) == true) || (Is_ASCII_Character_DecimalDigit(ReadResult.Data) == true))
            {
                Result->GetValue()->SetData(ReadResult.Data);
            }
            else
            {
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_Character_AlphaNumericOrSpace(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("character");
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphanumeric or space");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            if((Is_ASCII_Character_Alphabetic(ReadResult.Data) == true) || (Is_ASCII_Character_DecimalDigit(ReadResult.Data) == true) || (Is_ASCII_Character_Space(ReadResult.Data) == true))
            {
                Result->GetValue()->SetData(ReadResult.Data);
            }
            else
            {
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_String_Alphabetic_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphabetic"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        auto NumberOfCharacters{0ul};
        ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if(Is_ASCII_Character_Alphabetic(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphabetic ASCII character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphanumeric"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        auto NumberOfCharacters{0ul};
        ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if((Is_ASCII_Character_Alphabetic(ReadResult.Data) == true) || (Is_ASCII_Character_DecimalDigit(ReadResult.Data) == true))
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric ASCII character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("alphanumeric or space"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        auto NumberOfCharacters{0ul};
        ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if((Is_ASCII_Character_Alphabetic(ReadResult.Data) == true) || (Is_ASCII_Character_DecimalDigit(ReadResult.Data) == true) || (Is_ASCII_Character_Space(ReadResult.Data) == true))
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an alphanumeric or space ASCII character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_String_Printable_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("printable"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        auto NumberOfCharacters{0ul};
        ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if(Is_ASCII_Character_Printable(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not a printable ASCII character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASCII_String_Printable_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("printables only"s);
    //reading
    if(Continue == true)
    {
        auto NumberOfCharacters{0ul};
        std::stringstream Value;
        ReadResult ReadResult;
        
        while(Continue == true)
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    Result->GetValue()->AddTag("ended by termination"s);
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
                    
                    break;
                }
                else if(Is_ASCII_Character_Printable(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not a printable ASCII character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
        if(Continue == false)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        }
        Result->GetValue()->SetData(Value.str());
    }
    //finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_ExtendedContentDescription_ContentDescriptor_Data(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto & DataType{std::any_cast<const std::string &>(Parameters.at("DataType"))};
        
        if(DataType == "Unicode string")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                auto & Name{std::any_cast<const std::string &>(Parameters.at("Name"))};
        
                if(Name == "WM/MediaPrimaryClassID")
                {
                    auto String{std::any_cast<const std::string &>(Result->GetValue()->GetData())};
                    auto GUID{Inspection::Get_GUID_FromString_WithCurlyBraces(String)};
                    
                    Result->GetValue()->AppendField("GUID", GUID);
                    Result->GetValue()->GetField("GUID")->AddTag("guid"s);
                    Result->GetValue()->GetField("GUID")->AddTag("string"s);
                    
                    auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
                    
                    Result->GetValue()->GetField("GUID")->AddTag("interpretation", GUIDInterpretation);
                }
            }
        }
        else if(DataType == "Byte array")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Boolean")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::g_TypeRepository.GetType({"ASF", "Boolean_32Bit_LittleEndian"})->Get(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 32bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 64bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                auto const & Name = std::any_cast<std::string const &>(Parameters.at("Name"));
            
                if(Name == "WM/EncodingTime")
                {
                    Result->GetValue()->AddTag("Microsoft filetime"s);
                    
                    auto InterpretationTagValue = Result->GetValue()->AddTag("interpretation", Inspection::Get_DateTime_FromMicrosoftFileTime(std::any_cast<std::uint64_t>(Result->GetValue()->GetData())));
                    
                    InterpretationTagValue->AddTag("date and time"s);
                }
            }
        }
        else if(DataType == "Unsigned integer 16bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_ExtendedStreamPropertiesObject_Flags(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const std::bitset<32> & Flags{std::any_cast<const std::bitset<32> &>(Result->GetValue()->GetData())};
        
        Result->GetValue()->AppendField("[0] Reliable", Flags[0]);
        Result->GetValue()->AppendField("[1] Seekable", Flags[1]);
        Result->GetValue()->AppendField("[2] No Cleanpoints", Flags[2]);
        Result->GetValue()->AppendField("[3] Resend Live Cleanpoints", Flags[3]);
        Result->GetValue()->AppendField("[4-31] Reserved", false);
        for(auto Index = 4; Index < 32; ++Index)
        {
            Continue &= !Flags[Index];
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_ExtendedStreamPropertiesObjectData(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StartTime", PartResult->ExtractValue());
        Result->GetValue()->GetField("StartTime")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("EndTime", PartResult->ExtractValue());
        Result->GetValue()->GetField("EndTime")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("DataBitrate", PartResult->ExtractValue());
        Result->GetValue()->GetField("DataBitrate")->AddTag("unit", "bits per second"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BufferSize", PartResult->ExtractValue());
        Result->GetValue()->GetField("BufferSize")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("InitialBufferFullness", PartResult->ExtractValue());
        Result->GetValue()->GetField("InitialBufferFullness")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AlternateDataBitrate", PartResult->ExtractValue());
        Result->GetValue()->GetField("AlternateDataBitrate")->AddTag("unit", "bits per second"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AlternateBufferSize", PartResult->ExtractValue());
        Result->GetValue()->GetField("AlternateBufferSize")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AlternateInitialBufferFullness", PartResult->ExtractValue());
        Result->GetValue()->GetField("AlternateInitialBufferFullness")->AddTag("unit", "milliseconds"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("MaximumObjectSize", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ASF_ExtendedStreamPropertiesObject_Flags(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Flags", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamNumber", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamLanguageIndex", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AverageTimePerFrame", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamNameCount", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PayloadExtensionSystemCount", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType({"ASF", "ExtendedStreamProperties", "StreamName"})}, {"ElementName", "StreamName"s}, {"NumberOfElements", static_cast<std::uint64_t>(std::any_cast<std::uint16_t>(Result->GetValue()->GetField("StreamNameCount")->GetData()))}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamNames", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType({"ASF", "ExtendedStreamProperties", "PayloadExtensionSystem"})}, {"ElementName", "PayloadExtensionSystems"s}, {"NumberOfElements", static_cast<std::uint64_t>(std::any_cast<std::uint16_t>(Result->GetValue()->GetField("PayloadExtensionSystemCount")->GetData()))}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PayloadExtensionSystems", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        if(Reader.HasRemaining() == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "StreamProperties"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("StreamPropertiesObject", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_GUID(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_GUID_LittleEndian(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto GUIDInterpretation{Get_GUID_Interpretation(std::any_cast< Inspection::GUID >(Result->GetValue()->GetData()))};
        
        Result->GetValue()->AddTag("interpretation", GUIDInterpretation);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_MetadataLibrary_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    auto & DataType{std::any_cast<const std::string &>(Parameters.at("DataType"))};
    
    // reading
    if(Continue == true)
    {
        if(DataType == "Unicode string")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Byte array")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Boolean")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::g_TypeRepository.GetType({"ASF", "Boolean_16Bit_LittleEndian"})->Get(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 32bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 64bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 16bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "GUID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_GUID_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                auto GUID{std::any_cast< const Inspection::GUID & >(Result->GetValue()->GetData())};
                auto GUIDInterpretation{Inspection::Get_GUID_Interpretation(GUID)};
                
                Result->GetValue()->AddTag("interpretation", GUIDInterpretation);
            }
        }
        else
        {
            Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_Metadata_DescriptionRecord_Data(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    auto & DataType{std::any_cast<const std::string &>(Parameters.at("DataType"))};
    
    // reading
    if(Continue == true)
    {
        if(DataType == "Unicode string")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Byte array")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Boolean")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::g_TypeRepository.GetType({"ASF", "Boolean_16Bit_LittleEndian"})->Get(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 32bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 64bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(DataType == "Unsigned integer 16bit")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "The type \"" + DataType + "\" is unknown.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_Object(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectHeader"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto Size = Inspection::Length{std::any_cast<std::uint64_t>(Result->GetValue()->GetField("Size")->GetData()), 0};
        const auto & GUID = std::any_cast<const Inspection::GUID &>(Result->GetValue()->GetField("GUID")->GetData());
        
        if(GUID == Inspection::g_ASF_CompatibilityObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "Compatibility"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_HeaderObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "Header"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_FilePropertiesObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "FileProperties"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_StreamPropertiesObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::Get_ASF_StreamPropertiesObjectData(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_CodecListObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "CodecList"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_HeaderExtensionObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "HeaderExtension"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_LanguageListObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "LanguageList"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_ExtendedStreamPropertiesObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::Get_ASF_ExtendedStreamPropertiesObjectData(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_MetadataObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "Metadata"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_IndexPlaceholderObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "IndexPlaceholder"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_PaddingObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_ExtendedContentDescriptionObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "ExtendedContentDescription"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_StreamBitratePropertiesObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "StreamBitrateProperties"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_ContentDescriptionObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "ContentDescription"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(GUID == Inspection::g_ASF_MetadataLibraryObjectGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "ObjectData", "MetadataLibrary"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader, Size - Reader.GetConsumedLength()};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_StreamBitrateProperties_BitrateRecord_Flags(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    //~ Buffer.SetPosition(Position);
    //~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
    //~ if(Continue == true)
    //~ {
        //~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 7}}};
        //~ auto FieldResult{Get_UnsignedInteger_7Bit(FieldReader)};
        //~ auto FieldValue{Result->GetValue()->AppendField("[0-6] StreamNumber", FieldResult->GetValue())};
        
        //~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
    //~ }
    //~ if(Continue == true)
    //~ {
        //~ auto ReservedResult{Get_Data_Unset_EndedByLength(Buffer, Inspection::Length(0ull, 9))};
        
        //~ Result->GetValue()->AppendField("[7-15] Reserved", ReservedResult->GetValue());
        //~ Continue = ReservedResult->GetSuccess();
    //~ }
    //~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_StreamProperties_Flags(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    //~ Buffer.SetPosition(Start);
    //~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::LeastSignificantBitFirst);
    //~ // reading
    //~ if(Continue == true)
    //~ {
        //~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 7}}};
        //~ auto FieldResult{Get_UnsignedInteger_7Bit(FieldReader)};
        //~ auto FieldValue{Result->GetValue()->AppendField("[0-6] StreamNumber", FieldResult->GetValue())};
        
        //~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
    //~ }
    //~ // reading
    //~ if(Continue == true)
    //~ {
        //~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 8}}};
        //~ auto FieldResult{Get_UnsignedInteger_8Bit(FieldReader)};
        //~ auto FieldValue{Result->GetValue()->AppendField("[7-14] Reserved", FieldResult->GetValue())};
        
        //~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
    //~ }
    //~ // interpretation
    //~ if(Continue == true)
    //~ {
        //~ Continue = std::any_cast<std::uint8_t>(Result->GetValue()->GetValueAny("[7-14] Reserved")) == 0x00;
    //~ }
    //~ // reading
    //~ if(Continue == true)
    //~ {
        //~ Inspection::Reader FieldReader{Buffer, Inspection::Length{0, 1}}};
        //~ auto FieldResult{Get_Boolean_1Bit(FieldReader)};
        //~ auto FieldValue{Result->GetValue()->AppendField("[15] EncryptedContentFlag", FieldResult->GetValue())};
        
        //~ UpdateState(Continue, Buffer, FieldResult, FieldReader);
    //~ }
    //~ Buffer.SetBitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst);
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("AudioMedia"s);
    Result->GetValue()->AddTag("WAVEFORMATEX"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"Microsoft", "WAVE", "WaveFormat_FormatTag"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("FormatTag", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("NumberOfChannels", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SamplesPerSecond", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AverageNumberOfBytesPerSecond", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BlockAlignment", PartResult->ExtractValue());
        Result->GetValue()->GetField("BlockAlignment")->AddTag("unit", "bytes"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BitsPerSample", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_16Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("CodecSpecificDataSize", PartResult->ExtractValue());
        Result->GetValue()->GetField("CodecSpecificDataSize")->AddTag("unit", "bytes"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & FormatTag = std::any_cast<const std::string &>(Result->GetValue()->GetField("FormatTag")->GetTag("constant name")->GetData());
        
        if(FormatTag == "WAVE_FORMAT_WMAUDIO2")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint16_t>(Result->GetValue()->GetField("CodecSpecificDataSize")->GetData()), 0}};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "StreamProperties", "TypeSpecificData_AudioMedia_CodecSpecificData_WAVE_FORMAT_WMAUDIO2"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("CodecSpecificData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint16_t>(Result->GetValue()->GetField("CodecSpecificDataSize")->GetData()), 0}};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("CodecSpecificData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ASF_StreamPropertiesObjectData(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ASF_GUID(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamType", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ASF_GUID(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ErrorCorrectionType", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("TimeOffset", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("TypeSpecificDataLength", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ErrorCorrectionDataLength", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ASF_StreamProperties_Flags(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Flags", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Reserved", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & StreamType = std::any_cast<const Inspection::GUID &>(Result->GetValue()->GetField("StreamType")->GetData());
        
        if(StreamType == Inspection::g_ASF_AudioMediaGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("TypeSpecificDataLength")->GetData()), 0}};
            auto PartResult = Inspection::Get_ASF_StreamProperties_TypeSpecificData_AudioMedia(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("TypeSpecificData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("TypeSpecificDataLength")->GetData()), 0}};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("TypeSpecificData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // reading
    if(Continue == true)
    {
        const auto & ErrorCorrectionType = std::any_cast<const Inspection::GUID &>(Result->GetValue()->GetField("ErrorCorrectionType")->GetData());
        
        if(ErrorCorrectionType == Inspection::g_ASF_AudioSpreadGUID)
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("ErrorCorrectionDataLength")->GetData()), 0}};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ASF", "StreamProperties", "ErrorCorrectionData_AudioSpread"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ErrorCorrectionData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("ErrorCorrectionDataLength")->GetData()), 0}};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ErrorCorrectionData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_BitSet_4Bit_MostSignificantBitFirst(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("bitset"s);
    Result->GetValue()->AddTag("4bit"s);
    Result->GetValue()->AddTag("most significant bit first"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read4Bits(ReadResult)) == true)
        {
            std::bitset<4> Value;
            
            Value[0] = (ReadResult.Data & 0x08) == 0x08;
            Value[1] = (ReadResult.Data & 0x04) == 0x04;
            Value[2] = (ReadResult.Data & 0x02) == 0x02;
            Value[3] = (ReadResult.Data & 0x01) == 0x01;
            Result->GetValue()->SetData(Value);
            Result->GetValue()->AddTag("data", std::vector<std::uint8_t>{ReadResult.Data});
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_BitSet_8Bit_LeastSignificantBitFirst(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("bitset"s);
    Result->GetValue()->AddTag("8bit"s);
    Result->GetValue()->AddTag("least significant bit first"s);
    // verification
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            std::bitset<8> Value;
            
            Value[0] = (ReadResult.Data & 0x01) == 0x01;
            Value[1] = (ReadResult.Data & 0x02) == 0x02;
            Value[2] = (ReadResult.Data & 0x04) == 0x04;
            Value[3] = (ReadResult.Data & 0x08) == 0x08;
            Value[4] = (ReadResult.Data & 0x10) == 0x10;
            Value[5] = (ReadResult.Data & 0x20) == 0x20;
            Value[6] = (ReadResult.Data & 0x40) == 0x40;
            Value[7] = (ReadResult.Data & 0x80) == 0x80;
            Result->GetValue()->SetData(Value);
            Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{ReadResult.Data});
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("bitset"s);
    Result->GetValue()->AddTag("16bit"s);
    Result->GetValue()->AddTag("big endian"s);
    Result->GetValue()->AddTag("least significant bit first per byte"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                std::bitset<16> Value;
                
                Value[0] = (ReadResult2.Data & 0x01) == 0x01;
                Value[1] = (ReadResult2.Data & 0x02) == 0x02;
                Value[2] = (ReadResult2.Data & 0x04) == 0x04;
                Value[3] = (ReadResult2.Data & 0x08) == 0x08;
                Value[4] = (ReadResult2.Data & 0x10) == 0x10;
                Value[5] = (ReadResult2.Data & 0x20) == 0x20;
                Value[6] = (ReadResult2.Data & 0x40) == 0x40;
                Value[7] = (ReadResult2.Data & 0x80) == 0x80;
                Value[8] = (ReadResult1.Data & 0x01) == 0x01;
                Value[9] = (ReadResult1.Data & 0x02) == 0x02;
                Value[10] = (ReadResult1.Data & 0x04) == 0x04;
                Value[11] = (ReadResult1.Data & 0x08) == 0x08;
                Value[12] = (ReadResult1.Data & 0x10) == 0x10;
                Value[13] = (ReadResult1.Data & 0x20) == 0x20;
                Value[14] = (ReadResult1.Data & 0x40) == 0x40;
                Value[15] = (ReadResult1.Data & 0x80) == 0x80;
                Result->GetValue()->SetData(Value);
                Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{ReadResult1.Data, ReadResult2.Data});
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("bitset"s);
    Result->GetValue()->AddTag("16bit"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("least significant bit first per byte"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                std::bitset<16> Value;
                
                Value[0] = (ReadResult1.Data & 0x01) == 0x01;
                Value[1] = (ReadResult1.Data & 0x02) == 0x02;
                Value[2] = (ReadResult1.Data & 0x04) == 0x04;
                Value[3] = (ReadResult1.Data & 0x08) == 0x08;
                Value[4] = (ReadResult1.Data & 0x10) == 0x10;
                Value[5] = (ReadResult1.Data & 0x20) == 0x20;
                Value[6] = (ReadResult1.Data & 0x40) == 0x40;
                Value[7] = (ReadResult1.Data & 0x80) == 0x80;
                Value[8] = (ReadResult2.Data & 0x01) == 0x01;
                Value[9] = (ReadResult2.Data & 0x02) == 0x02;
                Value[10] = (ReadResult2.Data & 0x04) == 0x04;
                Value[11] = (ReadResult2.Data & 0x08) == 0x08;
                Value[12] = (ReadResult2.Data & 0x10) == 0x10;
                Value[13] = (ReadResult2.Data & 0x20) == 0x20;
                Value[14] = (ReadResult2.Data & 0x40) == 0x40;
                Value[15] = (ReadResult2.Data & 0x80) == 0x80;
                Result->GetValue()->SetData(Value);
                Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{ReadResult1.Data, ReadResult2.Data});
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("bitset"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("least significant bit first per byte"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        std::bitset<32> Value;
                        
                        Value[0] = (ReadResult1.Data & 0x01) == 0x01;
                        Value[1] = (ReadResult1.Data & 0x02) == 0x02;
                        Value[2] = (ReadResult1.Data & 0x04) == 0x04;
                        Value[3] = (ReadResult1.Data & 0x08) == 0x08;
                        Value[4] = (ReadResult1.Data & 0x10) == 0x10;
                        Value[5] = (ReadResult1.Data & 0x20) == 0x20;
                        Value[6] = (ReadResult1.Data & 0x40) == 0x40;
                        Value[7] = (ReadResult1.Data & 0x80) == 0x80;
                        Value[8] = (ReadResult2.Data & 0x01) == 0x01;
                        Value[9] = (ReadResult2.Data & 0x02) == 0x02;
                        Value[10] = (ReadResult2.Data & 0x04) == 0x04;
                        Value[11] = (ReadResult2.Data & 0x08) == 0x08;
                        Value[12] = (ReadResult2.Data & 0x10) == 0x10;
                        Value[13] = (ReadResult2.Data & 0x20) == 0x20;
                        Value[14] = (ReadResult2.Data & 0x40) == 0x40;
                        Value[15] = (ReadResult2.Data & 0x80) == 0x80;
                        Value[16] = (ReadResult3.Data & 0x01) == 0x01;
                        Value[17] = (ReadResult3.Data & 0x02) == 0x02;
                        Value[18] = (ReadResult3.Data & 0x04) == 0x04;
                        Value[19] = (ReadResult3.Data & 0x08) == 0x08;
                        Value[20] = (ReadResult3.Data & 0x10) == 0x10;
                        Value[21] = (ReadResult3.Data & 0x20) == 0x20;
                        Value[22] = (ReadResult3.Data & 0x40) == 0x40;
                        Value[23] = (ReadResult3.Data & 0x80) == 0x80;
                        Value[24] = (ReadResult4.Data & 0x01) == 0x01;
                        Value[25] = (ReadResult4.Data & 0x02) == 0x02;
                        Value[26] = (ReadResult4.Data & 0x04) == 0x04;
                        Value[27] = (ReadResult4.Data & 0x08) == 0x08;
                        Value[28] = (ReadResult4.Data & 0x10) == 0x10;
                        Value[29] = (ReadResult4.Data & 0x20) == 0x20;
                        Value[30] = (ReadResult4.Data & 0x40) == 0x40;
                        Value[31] = (ReadResult4.Data & 0x80) == 0x80;
                        Result->GetValue()->SetData(Value);
                        Result->GetValue()->AddTag("data", std::vector< std::uint8_t >{ReadResult1.Data, ReadResult2.Data, ReadResult3.Data, ReadResult4.Data});
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Boolean_1Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("boolean"s);
    Result->GetValue()->AddTag("1bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read1Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData((ReadResult.Data & 0x01) == 0x01);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("buffer"s);
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("8bit values"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        std::vector< std::uint8_t > Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                Value.push_back(ReadResult.Data);
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value);
        AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("buffer"s);
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("8bit values"s);
    Result->GetValue()->AddTag("zeroed");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        std::vector< std::uint8_t > Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                Value.push_back(ReadResult.Data);
                if(ReadResult.Data != 0x00)
                {
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(Value.size()) + "th 8bit value was not zeroed.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value);
        AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Data_Set_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("set data"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue == Reader.Read1Bits(ReadResult)) == true)
            {
                Continue = ReadResult.Data == 0x01;
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
    }
    // interpretation
    if(Continue == true)
    {
        AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    }
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Data_SetOrUnset_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("any data"s);
    
    auto RemainingOutputLength{Reader.CalculateRemainingOutputLength()};
    
    Reader.AdvancePosition(RemainingOutputLength);
    AppendLengthTag(Result->GetValue(), RemainingOutputLength);
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Data_Unset_Until16BitAlignment(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("unset data"s);
    Result->GetValue()->AddTag("until 16bit alignment");
    
    Inspection::Length LengthUntil16BitAlignment;
    auto OutOfAlignmentBits{Reader.GetReadPositionInInput().GetTotalBits() % 16};
    
    if(OutOfAlignmentBits > 0)
    {
        LengthUntil16BitAlignment.Set(0, 16 - OutOfAlignmentBits);
        if(Reader.Has(LengthUntil16BitAlignment) == true)
        {
            Inspection::ReadResult ReadResult;
            
            while((Continue == true) && (Reader.GetConsumedLength() < LengthUntil16BitAlignment))
            {
                if((Continue = Reader.Read1Bits(ReadResult)) == true)
                {
                    Continue = ReadResult.Data == 0x00;
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult);
                }
            }
            if(Continue == false)
            {
                Result->GetValue()->AddTag("error", "Only " + to_string_cast(Reader.GetConsumedLength()) + " bytes and bits could be read as unset data.");
            }
        }
        else
        {
            Result->GetValue()->AddTag("error", "The next 16bit alignment is not inside the reader's available data length.");
        }
    }
    AppendLengthTag(Result->GetValue(), LengthUntil16BitAlignment);
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Data_Unset_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("unset data"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if((Continue = Reader.Read1Bits(ReadResult)) == true)
            {
                Continue = ReadResult.Data == 0x00;
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
    }
    // interpretation
    if(Continue == true)
    {
        AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    }
    else
    {
        Result->GetValue()->AddTag("error", "Only " + to_string_cast(Reader.GetConsumedLength() - Inspection::Length{0, 1}) + " could be read as unset data.");
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Data_Unset_Until8BitAlignment(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::Reader PartReader{Reader, Inspection::Length{0, static_cast< std::uint8_t >((8 - Reader.GetReadPositionInInput().GetBits()) % 8)}};
        auto PartResult{Inspection::Get_Data_Unset_EndedByLength(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Result->GetValue()->AddTag("until 8bit alignment"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_ApplicationBlock_Data(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_BigEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("RegisteredApplicationIdentifier", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ApplicationData", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Frame_Header(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_14Bit_BigEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SyncCode", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<std::uint16_t>(Result->GetValue()->GetField("SyncCode")->GetData()) == 0x3ffe;
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Reserved", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Reserved")->GetData()) == 0x00;
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Frame_Header_BlockingStrategy"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BlockingStrategy", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BlockSize", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto BlockSize = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("BlockSize")->GetData());
        
        if(BlockSize == 0x00)
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("reserved"s);
            Result->GetValue()->GetField("BlockSize")->AddTag("error", "The block size 0 MUST NOT be used."s);
            Continue = false;
        }
        else if(BlockSize == 0x01)
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("value", static_cast<std::uint16_t>(192));
            Result->GetValue()->GetField("BlockSize")->AddTag("unit", "samples"s);
        }
        else if((BlockSize > 0x01) && (BlockSize <= 0x05))
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("value", static_cast<std::uint16_t>(576 * (1 << (BlockSize - 2))));
            Result->GetValue()->GetField("BlockSize")->AddTag("unit", "samples"s);
        }
        else if(BlockSize == 0x06)
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("interpretation", "get 8bit (blocksize - 1) from end of header"s);
        }
        else if(BlockSize == 0x07)
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("interpretation", "get 16bit (blocksize - 1) from end of header"s);
        }
        else if((BlockSize > 0x07) && (BlockSize < 0x10))
        {
            Result->GetValue()->GetField("BlockSize")->AddTag("value", static_cast<std::uint16_t>(256 * (1 << (BlockSize - 8))));
            Result->GetValue()->GetField("BlockSize")->AddTag("unit", "samples"s);
        }
        else
        {
            // all possible values for a 4-bit value have been handled
            IMPOSSIBLE_CODE_REACHED("BlockSize == " + to_string_cast(BlockSize));
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Frame_Header_SampleRate"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SampleRate", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Frame_Header_ChannelAssignment"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ChannelAssignment", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Frame_Header_SampleSize"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SampleSize", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        
        // because we can't GetField() this second "Reserved" field
        auto PartValue = Result->GetValue()->AppendField("Reserved", PartResult->ExtractValue());
        
        Reader.AdvancePosition(PartReader.GetConsumedLength());
        // verification
        if(Continue == true)
        {
            Continue = std::any_cast<std::uint8_t>(PartValue->GetData()) == 0x00;
        }
    }
    // reading
    if(Continue == true)
    {
        auto BlockingStrategy = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("BlockingStrategy")->GetData());
        
        if(BlockingStrategy == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_31Bit_UTF_8_Coded(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("FrameNumber", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(BlockingStrategy == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_36Bit_UTF_8_Coded(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("SampleNumber", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "Unknown blocking strategy value " + to_string_cast(BlockingStrategy) + ".");
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto BlockSize = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("BlockSize")->GetData());
        
        if(BlockSize == 0x06)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_8Bit(Reader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("BlockSizeExplicit", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                auto BlockSizeValue{Result->GetValue()->GetField("BlockSize")};
                
                BlockSizeValue->AddTag("value", static_cast<std::uint16_t>(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("BlockSizeExplicit")->GetData()) + 1));
                BlockSizeValue->AddTag("unit", "samples"s);
            }
        }
        else if(BlockSize == 0x07)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_16Bit_BigEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("BlockSizeExplicit", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                auto BlockSizeValue{Result->GetValue()->GetField("BlockSize")};
                
                BlockSizeValue->AddTag("value", static_cast<std::uint16_t>(std::any_cast<std::uint16_t>(Result->GetValue()->GetField("BlockSizeExplicit")->GetData()) + 1));
                BlockSizeValue->AddTag("unit", "samples"s);
            }
        }
    }
    // reading
    if(Continue == true)
    {
        auto SampleRate = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("SampleRate")->GetData());
        
        if(SampleRate == 0xc0)
        {
            NOT_IMPLEMENTED("get 8bit sample rate in Hz");
        }
        else if(SampleRate == 0xd0)
        {
            NOT_IMPLEMENTED("get 16bit sample rate in Hz");
        }
        else if(SampleRate == 0xe0)
        {
            NOT_IMPLEMENTED("get 8bit sample rate in tens of Hz");
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("CRC-8", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_MetaDataBlock(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "MetaDataBlock_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Header", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & MetaDataBlockType = std::any_cast<const std::string &>(Result->GetValue()->GetField("Header")->GetField("BlockType")->GetTag("interpretation")->GetData());
        
        if(MetaDataBlockType == "StreamInfo")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData()), 0}};
            auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "StreamInfoBlock_Data"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MetaDataBlockType == "Padding")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData()), 0}};
            auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MetaDataBlockType == "Application")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData()), 0}};
            auto PartResult = Inspection::Get_FLAC_ApplicationBlock_Data(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MetaDataBlockType == "SeekTable")
        {
            auto MetaDataBlockDataLength = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData());
            
            if(MetaDataBlockDataLength % 18 == 0)
            {
                auto PartReader = Inspection::Reader{Reader, Inspection::Length{MetaDataBlockDataLength, 0}};
                auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "SeekTableBlock_Data"})->Get(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else
            {
                Continue = false;
            }
        }
        else if(MetaDataBlockType == "VorbisComment")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData()), 0}};
            auto PartResult = Inspection::Get_Ogg_Vorbis_CommentHeader_WithoutFramingFlag(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MetaDataBlockType == "Picture")
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Length")->GetData()), 0}};
            auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "PictureBlock_Data"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Stream_Header(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_String_ASCII_ByTemplate(PartReader, {{"Template", "fLaC"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("FLAC stream marker", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "StreamInfoBlock"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamInfoBlock", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto LastMetaDataBlock = std::any_cast<bool>(Result->GetValue()->GetField("StreamInfoBlock")->GetField("Header")->GetField("LastMetaDataBlock")->GetData());
        
        if(LastMetaDataBlock == false)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_EndedByPredicate(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"FLAC", "MetaDataBlock"})}, {"ElementName", "MetaDataBlock"s}, {"EndPredicate", std::function<bool (Inspection::Value *) >{[](Inspection::Value * PartValue) { return std::any_cast<bool>(PartValue->GetField("Header")->GetField("LastMetaDataBlock")->GetData()); }}}});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("MetaDataBlocks", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_FLAC_Subframe_Header(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Header", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto WastedBitsPerSampleFlag = std::any_cast<bool>(Result->GetValue()->GetField("Header")->GetField("WastedBitsPerSampleFlag")->GetData());
        
        if(WastedBitsPerSampleFlag == true)
        {
            BitsPerSample -= std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("WastedBitsPerSample")->GetTag("value")->GetData());
        }
    }
    // reading
    if(Continue == true)
    {
        const auto & SubframeType = std::any_cast<const std::string &>(Result->GetValue()->GetField("Header")->GetField("Type")->GetTag("interpretation")->GetData());
        
        if(SubframeType == "SUBFRAME_CONSTANT")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_SignedInteger_BigEndian(PartReader, {{"Bits", BitsPerSample}});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(SubframeType == "SUBFRAME_FIXED")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Subframe_Data_Fixed"})->Get(PartReader, {{"FrameBlockSize", FrameBlockSize}, {"BitsPerSample", BitsPerSample}, {"PredictorOrder", Result->GetValue()->GetField("Header")->GetField("Type")->GetField("Order")->GetData()}});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(SubframeType == "SUBFRAME_LPC")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_FLAC_Subframe_Data_LPC(PartReader, FrameBlockSize, BitsPerSample, static_cast< std::uint8_t >(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Header")->GetField("Type")->GetField("Order")->GetData()) + 1));
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_CalculateBitsPerSample(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto SubFrameIndex = std::any_cast<std::uint64_t>(Parameters.at("ElementIndexInArray"));
        auto ChannelAssignment = std::any_cast<std::uint8_t>(Parameters.at("ChannelAssignment"));
        auto BlockSize = std::any_cast<std::uint16_t>(Parameters.at("BlockSize"));
        auto BitsPerSample = std::any_cast<std::uint8_t>(Parameters.at("BitsPerSample"));
        
        if(((SubFrameIndex == 0) && (ChannelAssignment == 0x09)) || ((SubFrameIndex == 1) && ((ChannelAssignment == 0x08) || (ChannelAssignment == 0x0a))))
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_FLAC_Subframe(PartReader, BlockSize, BitsPerSample + 1);
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_FLAC_Subframe(PartReader, BlockSize, BitsPerSample);
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Data_LPC(Inspection::Reader & Reader, std::uint16_t FrameBlockSize, std::uint8_t BitsPerSample, std::uint8_t PredictorOrder)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "BigEndian"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Bits", BitsPerSample}}}, {"NumberOfElements", static_cast<std::uint64_t>(PredictorOrder)}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("WarmUpSamples", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("QuantizedLinearPredictorCoefficientsPrecision", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto QuantizedLinearPredictorCoefficientsPrecision = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("QuantizedLinearPredictorCoefficientsPrecision")->GetData());
        
        if(QuantizedLinearPredictorCoefficientsPrecision < 15)
        {
            Result->GetValue()->GetField("QuantizedLinearPredictorCoefficientsPrecision")->AddTag("value", static_cast<std::uint8_t>(QuantizedLinearPredictorCoefficientsPrecision + 1));
        }
        else
        {
            Result->GetValue()->GetField("QuantizedLinearPredictorCoefficientsPrecision")->AddTag("error", "The percision MUST NOT be 15."s);
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_SignedInteger_5Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("QuantizedLinearPredictorCoefficientShift", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "BigEndian"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Bits", Result->GetValue()->GetField("QuantizedLinearPredictorCoefficientsPrecision")->GetTag("value")->GetData()}}}, {"NumberOfElements", static_cast<std::uint64_t>(PredictorOrder)}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PredictorCoefficients", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_FLAC_Subframe_Residual(PartReader, {{"FrameBlockSize", FrameBlockSize}, {"PredictorOrder", PredictorOrder}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Residual", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Header(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::Reader PartReader{Reader, Inspection::Length{0, 1}};
        auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PaddingBit", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_FLAC_Subframe_Type(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Type", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Boolean_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("WastedBitsPerSampleFlag", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        auto WastedBitsPerSampleFlag = std::any_cast<bool>(Result->GetValue()->GetField("WastedBitsPerSampleFlag")->GetData());
        
        if(WastedBitsPerSampleFlag == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_AlternativeUnary(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("WastedBitsPerSample", PartResult->ExtractValue());
            Result->GetValue()->GetField("WastedBitsPerSample")->AddTag("value", static_cast<std::uint32_t>(std::any_cast<std::uint32_t>(Result->GetValue()->GetField("WastedBitsPerSample")->GetData()) + 1));
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Residual(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"FLAC", "Subframe_Residual_CodingMethod"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("CodingMethod", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto CodingMethod = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("CodingMethod")->GetData());
        auto & FrameBlockSize = Parameters.at("FrameBlockSize");
        auto & PredictorOrder = Parameters.at("PredictorOrder");
        
        if(CodingMethod == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_FLAC_Subframe_Residual_Rice(PartReader, {{"FrameBlockSize", FrameBlockSize}, {"PredictorOrder", PredictorOrder}});
            
            Continue = PartResult->GetSuccess();
            PartResult->GetValue()->AddTag("Rice"s);
            Result->GetValue()->AppendField("CodedResidual", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(CodingMethod == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_FLAC_Subframe_Residual_Rice2(PartReader, {{"FrameBlockSize", FrameBlockSize}, {"PredictorOrder", PredictorOrder}});
            
            Continue = PartResult->GetSuccess();
            PartResult->GetValue()->AddTag("Rice2"s);
            Result->GetValue()->AppendField("CodedResidual", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

/**
 * Parameters:
 * - "FrameBlockSize" => std::uint16_t
 * - "PredictorOrder" => std::uint8_t
 **/
std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Residual_Rice(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PartitionOrder", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto NumberOfPartitions = static_cast<std::uint16_t>(1 << std::any_cast<std::uint8_t>(Result->GetValue()->GetField("PartitionOrder")->GetData()));
        
        Result->GetValue()->GetField("PartitionOrder")->AddTag("number of partitions", NumberOfPartitions);
    }
    // reading
    if(Continue == true)
    {
        auto & PredictorOrder = Parameters.at("PredictorOrder");
        auto FrameBlockSize = std::any_cast<std::uint16_t>(Parameters.at("FrameBlockSize"));
        auto NumberOfPartitions = std::any_cast<std::uint16_t>(Result->GetValue()->GetField("PartitionOrder")->GetTag("number of partitions")->GetData());
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"FLAC", "Subframe_Residual_Rice_Partition"})}, {"NumberOfElements", static_cast<std::uint64_t>(NumberOfPartitions)}, {"ElementName", "Partition"s}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"NumberOfSamples", static_cast<std::uint32_t>(FrameBlockSize / NumberOfPartitions)}, {"PredictorOrder", PredictorOrder}}}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Partitions", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Residual_Rice_Partition(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("RiceParameter", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto Rice = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("RiceParameter")->GetData());
        auto ElementIndexInArray = std::any_cast<std::uint64_t>(Parameters.at("ElementIndexInArray"));
        auto NumberOfSamples = std::any_cast<std::uint32_t>(Parameters.at("NumberOfSamples"));
        auto PredictorOrder = std::any_cast<std::uint8_t>(Parameters.at("PredictorOrder"));
        
        if(ElementIndexInArray == 0)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "32Bit_RiceEncoded"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Rice", Rice}}}, {"NumberOfElements", static_cast<std::uint64_t>(NumberOfSamples - PredictorOrder)}});
            
            Continue = PartResult->GetSuccess();
            if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
            {
                Result->GetValue()->AppendField("Samples", PartResult->ExtractValue());
            }
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "32Bit_RiceEncoded"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Rice", Rice}}}, {"NumberOfElements", static_cast<std::uint64_t> (NumberOfSamples)}});
            
            Continue = PartResult->GetSuccess();
            if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
            {
                Result->GetValue()->AppendField("Samples", PartResult->ExtractValue());
            }
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Residual_Rice2(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PartitionOrder", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto NumberOfPartitions = static_cast<std::uint16_t>(1 << std::any_cast<std::uint8_t>(Result->GetValue()->GetField("PartitionOrder")->GetData()));
        
        Result->GetValue()->GetField("PartitionOrder")->AddTag("number of partitions", NumberOfPartitions);
    }
    // reading
    if(Continue == true)
    {
        auto & PredictorOrder = Parameters.at("PredictorOrder");
        auto FrameBlockSize = std::any_cast<std::uint16_t>(Parameters.at("FrameBlockSize"));
        auto NumberOfPartitions = std::any_cast<std::uint16_t>(Result->GetValue()->GetField("PartitionOrder")->GetTag("number of partitions")->GetData());
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"FLAC", "Subframe_Residual_Rice2_Partition"})}, {"NumberOfElements", static_cast<std::uint64_t>(NumberOfPartitions)}, {"ElementName", "Partition"s}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"NumberOfSamples", static_cast<std::uint32_t>(FrameBlockSize / NumberOfPartitions)}, {"PredictorOrder", PredictorOrder}}}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Partitions", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Residual_Rice2_Partition(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_5Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("RiceParameter", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto Rice = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("RiceParameter")->GetData());
        auto ElementIndexInArray = std::any_cast<std::uint64_t>(Parameters.at("ElementIndexInArray"));
        auto NumberOfSamples = std::any_cast<std::uint32_t>(Parameters.at("NumberOfSamples"));
        auto PredictorOrder = std::any_cast<std::uint8_t>(Parameters.at("PredictorOrder"));
        
        if(ElementIndexInArray == 0)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "32Bit_RiceEncoded"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Rice", Rice}}}, {"NumberOfElements", static_cast<std::uint64_t>(NumberOfSamples - PredictorOrder)}});
            
            Continue = PartResult->GetSuccess();
            if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
            {
                Result->GetValue()->AppendField("Samples", PartResult->ExtractValue());
            }
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Signed", "32Bit_RiceEncoded"})}, {"ElementParameters", std::unordered_map<std::string, std::any>{{"Rice", Rice}}}, {"NumberOfElements", static_cast<std::uint64_t> (NumberOfSamples)}});
            
            Continue = PartResult->GetSuccess();
            if(g_AppendFLACStream_Subframe_Residual_Rice_Partition_Samples == true)
            {
                Result->GetValue()->AppendField("Samples", PartResult->ExtractValue());
            }
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_FLAC_Subframe_Type(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader, Inspection::Length{0, 6}};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto SubframeType = std::any_cast<std::uint8_t>(Result->GetValue()->GetData());
        
        switch(SubframeType)
        {
        case 0x00:
            {
                Result->GetValue()->AddTag("interpretation", "SUBFRAME_LPC"s);
                
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::Get_UnsignedInteger_5Bit(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Order", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
                // interpretation
                if(Continue == true)
                {
                    Result->GetValue()->GetField("Order")->AddTag("value", static_cast<std::uint8_t>(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Order")->GetData()) + 1));
                }
                
                break;
            }
        case 0x02:
            {
                Result->GetValue()->AddTag("interpretation", "SUBFRAME_FIXED"s);
                
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::Get_UnsignedInteger_3Bit(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Order", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
                // interpretation and verification
                if(Continue == true)
                {
                    auto Order = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Order")->GetData());
                    
                    Result->GetValue()->GetField("Order")->AddTag("value", Order);
                    if(Order >= 5)
                    {
                        Result->GetValue()->AddTag("reserved");
                        Result->GetValue()->AddTag("error", "The subframe type is SUBFRAME_FIXED, and the order " + to_string_cast(Order) + " MUST NOT be used.");
                        Continue = false;
                    }
                }
                
                break;
            }
        case 0x01:
        case 0x03:
        case 0x04:
            {
                Result->GetValue()->AddTag("reserved"s);
                Result->GetValue()->AddTag("error", "This subframe type MUST NOT be used."s);
                Continue = false;
                
                break;
            }
        case 0x05:
            {
                NOT_IMPLEMENTED("SUBFRAME_VERBATIM");
            }
        case 0x06:
            {
                Result->GetValue()->AddTag("interpretation", "SUBFRAME_CONSTANT"s);
                
                break;
            }
        default:
            {
                // all possible vaues of a 6-bit unary have been handled
                IMPOSSIBLE_CODE_REACHED("SubframeType == " + to_string_cast(SubframeType));
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_GUID_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    GUID Value;
    
    Result->GetValue()->AddTag("guid"s);
    Result->GetValue()->AddTag("binary"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Value.Data1 = ReadResult1.Data | (static_cast<std::uint32_t>(ReadResult2.Data) << 8) | (static_cast<std::uint32_t>(ReadResult3.Data) << 16) | (static_cast<std::uint32_t>(ReadResult4.Data) << 24);
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Value.Data2 = ReadResult1.Data | (static_cast<std::uint32_t>(ReadResult2.Data) << 8);
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Value.Data3 = ReadResult1.Data | (static_cast<std::uint32_t>(ReadResult2.Data) << 8);
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        for(auto Index = 0; (Continue == true) && (Index < 8); ++Index)
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                Value.Data4[Index] = ReadResult.Data;
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
    }
    // post-processing
    if(Continue == true)
    {
        Result->GetValue()->SetData(Value);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_1_Genre(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_UnsignedInteger_8Bit(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto GenreNumber{std::any_cast<std::uint8_t>(Result->GetValue()->GetData())};
        
        try
        {
            auto Genre{Inspection::Get_ID3_1_Genre(GenreNumber)};
            
            Result->GetValue()->AddTag("interpretation", Genre);
            Result->GetValue()->AddTag("standard", "ID3v1"s);
        }
        catch(Inspection::UnknownValueException & Exception)
        {
            try
            {
                auto Genre{Inspection::Get_ID3_1_Winamp_Genre(GenreNumber)};
                
                Result->GetValue()->AddTag("interpretation", Genre);
                Result->GetValue()->AddTag("standard", "Winamp extension"s);
            }
            catch(Inspection::UnknownValueException & Exception)
            {
                Result->GetValue()->AddTag("interpretation", nullptr);
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_2_Frame(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "Frame_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ClaimedSize  = Inspection::Length(std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Size")->GetData()), 0);
        auto PartReader = Inspection::Reader{Reader, ClaimedSize};
        const auto & Identifier = std::any_cast<const std::string &>(Result->GetValue()->GetField("Identifier")->GetData());
        auto PartResult = std::unique_ptr<Inspection::Result>{};
        
        if(Identifier == "COM")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "FrameBody", "COM"})->Get(PartReader, {});
        }
        else if(Identifier == "PIC")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "FrameBody", "PIC"})->Get(PartReader, {});
        }
        else if((Identifier == "TAL") || (Identifier == "TCM") || (Identifier == "TCP") || (Identifier == "TEN") || (Identifier == "TP1") || (Identifier == "TP2") || (Identifier == "TPA") || (Identifier == "TRK") || (Identifier == "TT1") || (Identifier == "TT2") || (Identifier == "TYE"))
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "FrameBody", "T__"})->Get(PartReader, {});
        }
        else if(Identifier == "TCO")
        {
            PartResult = Inspection::Get_ID3_2_2_Frame_Body_TCO(PartReader, {});
        }
        else if(Identifier == "UFI")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "FrameBody", "UFI"})->Get(PartReader, {});
        }
        else
        {
            Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
            
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        if(PartResult != nullptr)
        {
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            if(PartReader.HasRemaining() == true)
            {
                Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size. See at the end of the frame for superfluous data."s);
                AppendLengthTag(Result->GetValue(), ClaimedSize, "claimed size");
                AppendLengthTag(Result->GetValue(), PartReader.GetConsumedLength(), "handled size");
                AppendLengthTag(Result->GetValue(), PartReader.CalculateRemainingOutputLength(), "rest size");
                
                auto RestReader = Inspection::Reader{PartReader};
                auto RestResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(RestReader, {});
                
                Continue = RestResult->GetSuccess();
                Result->GetValue()->AppendField("Rest", RestResult->ExtractValue());
                Result->GetValue()->GetField("Rest")->AddTag("error", "This is additional unparsed data at the end of the frame."s);
                Reader.AdvancePosition(RestReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_2_Frame_Body_TCO(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "FrameBody", "T__"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const auto & Information = std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetData());
        auto Interpretation = GetContentTypeInterpretation2_3(Information);
        
        if(std::get<0>(Interpretation) == true)
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", std::get<1>(Interpretation));
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_2_Tag_Body(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"ID3", "v2.2", "Frame"})}, {"ElementName", "Frame"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Frames", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        if(Reader.IsAtEnd() == false)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Padding", PartResult->ExtractValue());
            if(Continue == true)
            {
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "Could not read text with text encoding " + to_string_cast(TextEncoding) + ".");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "Frame_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ClaimedSize = Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Size")->GetData()), 0};
        auto PartReader = Inspection::Reader{Reader, ClaimedSize};
        const auto & Identifier = std::any_cast<const std::string &>(Result->GetValue()->GetField("Identifier")->GetData());
        auto PartResult = std::unique_ptr<Inspection::Result>{};
        
        if(Identifier == "APIC")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "APIC"})->Get(PartReader, {});
        }
        else if(Identifier == "COMM")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "COMM"})->Get(PartReader, {});
        }
        else if(Identifier == "GEOB")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "GEOB"})->Get(PartReader, {});
        }
        else if(Identifier == "MCDI")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "MCDI"})->Get(PartReader, {});
        }
        else if(Identifier == "PCNT")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_PCNT(PartReader, {});
        }
        else if(Identifier == "POPM")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_POPM(PartReader, {});
        }
        else if(Identifier == "PRIV")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_PRIV(PartReader, {});
        }
        else if(Identifier == "RGAD")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "RGAD"})->Get(PartReader, {});
        }
        else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCOP") || (Identifier == "TDAT") || (Identifier == "TDRC") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TEXT") || (Identifier == "TIME") || (Identifier == "TIT1") || (Identifier == "TIT2") || (Identifier == "TIT3") || (Identifier == "TLEN") || (Identifier == "TMED") || (Identifier == "TOAL") || (Identifier == "TOFN") || (Identifier == "TOPE") || (Identifier == "TOWN") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPE3") || (Identifier == "TPE4") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TRDA") || (Identifier == "TSIZ") || (Identifier == "TSO2") || (Identifier == "TSOA") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TSST") || (Identifier == "TYER"))
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {});
        }
        else if(Identifier == "TCMP")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_TCMP(PartReader, {});
        }
        else if(Identifier == "TCON")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_TCON(PartReader, {});
        }
        else if(Identifier == "TFLT")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_TFLT(PartReader, {});
        }
        else if(Identifier == "TLAN")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_TLAN(PartReader, {});
        }
        else if(Identifier == "TSRC")
        {
            PartResult = Inspection::Get_ID3_2_3_Frame_Body_TSRC(PartReader, {});
        }
        else if(Identifier == "TXXX")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "TXXX"})->Get(PartReader, {});
        }
        else if(Identifier == "UFID")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "UFID"})->Get(PartReader, {});
        }
        else if(Identifier == "USLT")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "USLT"})->Get(PartReader, {});
        }
        else if((Identifier == "WCOM") || (Identifier == "WOAF") || (Identifier == "WOAR"))
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "W___"})->Get(PartReader, {});
        }
        else if(Identifier == "WXXX")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "WXXX"})->Get(PartReader, {});
        }
        else
        {
            Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
            
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        if(PartResult != nullptr)
        {
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            if(PartReader.HasRemaining() == true)
            {
                Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size. See at the end of the frame for superfluous data."s);
                AppendLengthTag(Result->GetValue(), ClaimedSize, "claimed size");
                AppendLengthTag(Result->GetValue(), PartReader.GetConsumedLength(), "handled size");
                AppendLengthTag(Result->GetValue(), PartReader.CalculateRemainingOutputLength(), "rest size");
                
                auto RestReader = Inspection::Reader{PartReader};
                auto RestResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(RestReader, {});
                
                Continue = RestResult->GetSuccess();
                Result->GetValue()->AppendField("Rest", RestResult->ExtractValue());
                Result->GetValue()->GetField("Rest")->AddTag("error", "This is additional unparsed data at the end of the frame."s);
                Reader.AdvancePosition(RestReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_PCNT(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto RemainingLengthInReaderOutput = Reader.CalculateRemainingOutputLength();
        
        if(RemainingLengthInReaderOutput < Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Counter")->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput == Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_BigEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput > Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // reading
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_POPM(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("EMailToUser", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Rating", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Rating = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Rating")->GetData());
        
        if(Rating > 0)
        {
            Result->GetValue()->GetField("Rating")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Rating")->AddTag("interpretation", to_string_cast(Rating) + " / 255");
        }
        else
        {
            Result->GetValue()->GetField("Rating")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Rating")->AddTag("interpretation", nullptr);
        }
    }
    // reading
    if(Continue == true)
    {
        auto RemainingLengthInReaderOutput = Reader.CalculateRemainingOutputLength();
        
        if(RemainingLengthInReaderOutput == Inspection::Length{0, 0})
        {
            Result->GetValue()->AppendField("Counter")->AddTag("omitted"s);
        }
        else if(RemainingLengthInReaderOutput < Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Counter")->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput == Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_BigEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput > Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_PRIV(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("OwnerIdentifier", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & OwnerIdentifier = std::any_cast<const std::string &>(Result->GetValue()->GetField("OwnerIdentifier")->GetData());
        
        if(OwnerIdentifier == "AverageLevel")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("AverageLevel", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "PeakValue")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("PeakValue", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/MediaClassPrimaryID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("MediaClassPrimaryID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/MediaClassSecondaryID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("MediaClassSecondaryID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/WMCollectionGroupID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("CollectionGroupID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/WMCollectionID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("CollectionID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/WMContentID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ContentID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "ZuneAlbumArtistMediaID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ZuneAlbumArtistMediaID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "ZuneAlbumMediaID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ZuneAlbumMediaID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "ZuneCollectionID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ZuneCollectionID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "ZuneMediaID")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_GUID(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ZuneMediaID", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/Provider")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Provider", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(OwnerIdentifier == "WM/UniqueFileIdentifier")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("UniqueFileIdentifier", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("PrivateData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_TCMP(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto Information{std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetTag("value")->GetData())};
        
        if(Information == "1")
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", "yes, this is part of a compilation"s);
        }
        else if(Information == "0")
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", "no, this is not part of a compilation"s);
        }
        else
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", nullptr);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_TCON(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Information{std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetTag("value")->GetData())};
        auto Interpretation{GetContentTypeInterpretation2_3(Information)};
        
        if(std::get<0>(Interpretation) == true)
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", std::get<1>(Interpretation));
        }
    }
    // finalization
    Result->SetSuccess(true);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_TFLT(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Result->GetValue()->GetField("Information")->AddTag("standard", "ID3 2.3"s);
        
        auto Information{std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetTag("value")->GetData())};
        
        try
        {
            Result->GetValue()->GetField("Information")->AddTag("interpretation", Get_ID3_2_3_FileType_Interpretation(Information));
        }
        catch(Inspection::UnknownValueException & Exception)
        {
            if(Information == "/3")
            {
                Result->GetValue()->GetField("Information")->AddTag("error", "The file type could not be interpreted strictly according to the standard, but this seems plausible."s);
                Result->GetValue()->GetField("Information")->AddTag("interpretation", "MPEG 1/2 layer III");
            }
            else
            {
                Result->GetValue()->GetField("Information")->AddTag("error", "The file type could not be interpreted."s);
                Result->GetValue()->GetField("Information")->AddTag("interpretation", nullptr);
            }
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_TLAN(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        try
        {
            auto Information{std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetTag("value")->GetData())};
            
            Result->GetValue()->GetField("Information")->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
            Result->GetValue()->GetField("Information")->AddTag("interpretation", Inspection::Get_LanguageName_From_ISO_639_2_1998_Code(Information));
        }
        catch(...)
        {
            Result->GetValue()->GetField("Information")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Information")->AddTag("error", "The language frame needs to contain a three letter code from ISO 639-2:1998 (alpha-3)."s);
            Result->GetValue()->GetField("Information")->AddTag("interpretation", nullptr);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Body_TSRC(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Information{std::any_cast<const std::string &>(Result->GetValue()->GetField("Information")->GetTag("value")->GetData())};
        
        if(Information.length() == 12)
        {
            Result->GetValue()->GetField("Information")->AddTag("standard", "ISRC Bulletin 2015/01"s);
            Result->GetValue()->GetField("Information")->AddTag("DesignationCode", Information.substr(7, 5));
            Result->GetValue()->GetField("Information")->AddTag("YearOfReference", Information.substr(5, 2));
            Result->GetValue()->GetField("Information")->AddTag("RegistrantCode", Information.substr(2, 3));
            
            std::string CountryCode{Information.substr(0, 2)};
            auto CountryCodeValue{Result->GetValue()->GetField("Information")->AddTag("CountryCode", CountryCode)};
            
            try
            {
                CountryCodeValue->AddTag("standard", "ISO 3166-1 alpha-2"s);
                CountryCodeValue->AddTag("interpretation", Inspection::Get_CountryName_From_ISO_3166_1_Alpha_2_CountryCode(CountryCode));
            }
            catch(Inspection::UnknownValueException & Exception)
            {
                CountryCodeValue->AddTag("standard", "ISRC Bulletin 2015/01"s);
                CountryCodeValue->AddTag("error", "The ISRC string needs to contain a two letter country code from ISO 3166-1 alpha-2."s);
                CountryCodeValue->AddTag("interpretation", nullptr);
            }
        }
        else
        {
            Result->GetValue()->GetField("Information")->AddTag("standard", "ID3 2.3"s);
            Result->GetValue()->GetField("Information")->AddTag("error", "The TSRC frame needs to contain a twelve letter ISRC code from ISRC Bulletin 2015/01."s);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Frame_Header_Flags(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const std::bitset<16> & Flags{std::any_cast< const std::bitset<16> & >(Result->GetValue()->GetData())};
        Inspection::Value * FlagValue;
        
        FlagValue = Result->GetValue()->AppendField("TagAlterPreservation", Flags[15]);
        FlagValue->AddTag("bit index", 15);
        FlagValue->AddTag("bit name", "a"s);
        if(Flags[15] == true)
        {
            FlagValue->AddTag("interpretation", "Frame should be discarded."s);
        }
        else
        {
            FlagValue->AddTag("interpretation", "Frame should be preserved."s);
        }
        FlagValue = Result->GetValue()->AppendField("FileAlterPreservation", Flags[14]);
        FlagValue->AddTag("bit index", 14);
        FlagValue->AddTag("bit name", "b"s);
        if(Flags[14] == true)
        {
            FlagValue->AddTag("interpretation", "Frame should be discarded."s);
        }
        else
        {
            FlagValue->AddTag("interpretation", "Frame should be preserved."s);
        }
        FlagValue = Result->GetValue()->AppendField("ReadOnly", Flags[13]);
        FlagValue->AddTag("bit index", 13);
        FlagValue->AddTag("bit name", "c"s);
        FlagValue = Result->GetValue()->AppendField("Reserved", false);
        for(auto FlagIndex = 8; FlagIndex <= 12; ++FlagIndex)
        {
            FlagValue->AddTag("bit index", FlagIndex);
            Continue &= !Flags[FlagIndex];
        }
        FlagValue = Result->GetValue()->AppendField("Compression", Flags[7]);
        FlagValue->AddTag("bit index", 7);
        FlagValue->AddTag("bit name", "i"s);
        if(Flags[7] == true)
        {
            FlagValue->AddTag("interpretation", "Frame is compressed using ZLIB with 4 bytes for 'decompressed size' appended to the frame header."s);
            FlagValue->AddTag("error", "Frame compression is not yet implemented!");
        }
        else
        {
            FlagValue->AddTag("interpretation", "Frame is not compressed."s);
        }
        FlagValue = Result->GetValue()->AppendField("Encryption", Flags[6]);
        FlagValue->AddTag("bit index", 6);
        FlagValue->AddTag("bit name", "j"s);
        if(Flags[6] == true)
        {
            FlagValue->AddTag("interpretation", "Frame is encrypted."s);
            FlagValue->AddTag("error", "Frame encryption is not yet implemented!");
        }
        else
        {
            FlagValue->AddTag("interpretation", "Frame is not encrypted."s);
        }
        FlagValue = Result->GetValue()->AppendField("GroupingIdentity", Flags[5]);
        FlagValue->AddTag("bit index", 5);
        FlagValue->AddTag("bit name", "k"s);
        if(Flags[5] == true)
        {
            FlagValue->AddTag("interpretation", "Frame contains group information."s);
            FlagValue->AddTag("error", "Frame grouping is not yet implemented!");
        }
        else
        {
            FlagValue->AddTag("interpretation", "Frame does not contain group information."s);
        }
        FlagValue = Result->GetValue()->AppendField("Reserved", false);
        for(auto FlagIndex = 0; FlagIndex <= 4; ++FlagIndex)
        {
            FlagValue->AddTag("bit index", FlagIndex);
            Continue &= !Flags[FlagIndex];
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_Tag_Body(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        if(std::any_cast<bool>(Parameters.at("ExtendedHeader")) == true)
        {
            NOT_IMPLEMENTED("ID3 2.3 extended header");
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"ID3", "v2.3", "Frame"})}, {"ElementName", "Frame"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Frames", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        if(Reader.IsAtEnd() == false)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Padding", PartResult->ExtractValue());
            if(Continue == true)
            {
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetField("String")->GetData());
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetField("String")->GetData());
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Frame(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "Frame_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ClaimedSize = Inspection::Length(std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Size")->GetData()), 0);
        auto PartReader = Inspection::Reader{Reader, ClaimedSize};
        const auto & Identifier = std::any_cast<const std::string &>(Result->GetValue()->GetField("Identifier")->GetData());
        auto PartResult = std::unique_ptr<Inspection::Result>{};
        
        if(Identifier == "APIC")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "APIC"})->Get(PartReader, {});
        }
        else if(Identifier == "COMM")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "COMM"})->Get(PartReader, {});
        }
        else if(Identifier == "MCDI")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "MCDI"})->Get(PartReader, {});
        }
        else if(Identifier == "POPM")
        {
            PartResult = Inspection::Get_ID3_2_4_Frame_Body_POPM(PartReader, {});
        }
        else if(Identifier == "PRIV")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "PRIV"})->Get(PartReader, {});
        }
        else if((Identifier == "TALB") || (Identifier == "TBPM") || (Identifier == "TCOM") || (Identifier == "TCON") || (Identifier == "TCOP") || (Identifier == "TDRC") || (Identifier == "TDRL") || (Identifier == "TDTG") || (Identifier == "TENC") || (Identifier == "TIT2") || (Identifier == "TLAN") || (Identifier == "TLEN") || (Identifier == "TOAL") || (Identifier == "TPE1") || (Identifier == "TPE2") || (Identifier == "TPOS") || (Identifier == "TPUB") || (Identifier == "TRCK") || (Identifier == "TSOP") || (Identifier == "TSSE") || (Identifier == "TYER"))
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "T___"})->Get(PartReader, {});
        }
        else if(Identifier == "TCMP")
        {
            PartResult = Inspection::Get_ID3_2_4_Frame_Body_TCMP(PartReader, {});
        }
        else if(Identifier == "TXXX")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "TXXX"})->Get(PartReader, {});
        }
        else if(Identifier == "UFID")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "UFID"})->Get(PartReader, {});
        }
        else if(Identifier == "USLT")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "USLT"})->Get(PartReader, {});
        }
        else if(Identifier == "WCOM")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "W___"})->Get(PartReader, {});
        }
        else if(Identifier == "WXXX")
        {
            PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "WXXX"})->Get(PartReader, {});
        }
        else
        {
            Result->GetValue()->AddTag("error", "The frame identifier \"" + Identifier + "\" has no associated handler."s);
            
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        if(PartResult != nullptr)
        {
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            if(PartReader.HasRemaining() == true)
            {
                Result->GetValue()->AddTag("error", "The frame size is claimed larger than the actually handled size. See at the end of the frame for superfluous data."s);
                AppendLengthTag(Result->GetValue(), ClaimedSize, "claimed size");
                AppendLengthTag(Result->GetValue(), PartReader.GetConsumedLength(), "handled size");
                AppendLengthTag(Result->GetValue(), PartReader.CalculateRemainingOutputLength(), "rest size");
                
                auto RestReader = Inspection::Reader{PartReader};
                auto RestResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(RestReader, {});
                
                Continue = RestResult->GetSuccess();
                Result->GetValue()->AppendField("Rest", RestResult->ExtractValue());
                Result->GetValue()->GetField("Rest")->AddTag("error", "This is additional unparsed data at the end of the frame."s);
                Reader.AdvancePosition(RestReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Frame_Body_POPM(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("EMailToUser", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Rating", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Rating = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Rating")->GetData());
        
        if(Rating > 0)
        {
            Result->GetValue()->GetField("Rating")->AddTag("standard", "ID3 2.4"s);
            Result->GetValue()->GetField("Rating")->AddTag("interpretation", to_string_cast(Rating) + " / 255");
        }
        else
        {
            Result->GetValue()->GetField("Rating")->AddTag("standard", "ID3 2.4"s);
            Result->GetValue()->GetField("Rating")->AddTag("interpretation", nullptr);
        }
    }
    // reading
    if(Continue == true)
    {
        auto RemainingLengthInReaderOutput = Reader.CalculateRemainingOutputLength();
        
        if(RemainingLengthInReaderOutput == Inspection::Length{0, 0})
        {
            auto PartValue = Result->GetValue()->AppendField("Counter");
            
            PartValue->AddTag("omitted"s);
        }
        else if(RemainingLengthInReaderOutput < Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("standard", "ID3 2.4"s);
            Result->GetValue()->GetField("Counter")->AddTag("error", "The Counter field is too short, as it must be at least four bytes long."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput == Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_UnsignedInteger_32Bit_BigEndian(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(RemainingLengthInReaderOutput > Inspection::Length{4, 0})
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Counter", PartResult->ExtractValue());
            Result->GetValue()->GetField("Counter")->AddTag("error", "This program doesn't support printing a counter with more than four bytes yet."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Frame_Body_TCMP(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "FrameBody", "T___"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        for(auto & PartValue : Result->GetValue()->GetField("Informations")->GetFields())
        {
            auto Information{std::any_cast<const std::string &>(PartValue->GetTag("value")->GetData())};
            
            if(Information == "1")
            {
                PartValue->AddTag("interpretation", "yes, this is part of a compilation"s);
            }
            else if(Information == "0")
            {
                PartValue->AddTag("interpretation", "no, this is not part of a compilation"s);
            }
            else
            {
                PartValue->AddTag("error", "The value \"" + Information + "\" could not interpreted.");
                PartValue->AddTag("interpretation", nullptr);
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Tag_Body(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        if(std::any_cast<bool>(Parameters.at("ExtendedHeader")) == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_2_4_Tag_ExtendedHeader(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ExtendedHeader", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    if(Continue == true)
    {
        // reading
        if(Continue == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Array_AtLeastOne_EndedByFailureOrLength_ResetPositionOnFailure(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"ID3", "v2.4", "Frame"})}, {"ElementName", "Frame"s}});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Frames", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        // reading
        if(Continue == true)
        {
            if(Reader.IsAtEnd() == false)
            {
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Padding", PartResult->ExtractValue());
                if(Continue == true)
                {
                    Reader.AdvancePosition(PartReader.GetConsumedLength());
                }
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Tag_ExtendedHeader(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Size", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("NumberOfFlagBytes", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        if(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("NumberOfFlagBytes")->GetData()) != 0x01)
        {
            Result->GetValue()->AddTag("error", "According to the standard, the number of flag bytes must be equal to 1."s);
            Result->GetValue()->AddTag("standard", "ID3 2.4"s);
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ExtendedFlags", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    if(Continue == true)
    {
        if(std::any_cast<bool>(Result->GetValue()->GetField("ExtendedFlags")->GetField("TagIsAnUpdate")->GetData()) == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("TagIsAnUpdateData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    if(Continue == true)
    {
        if(std::any_cast<bool>(Result->GetValue()->GetField("ExtendedFlags")->GetField("CRCDataPresent")->GetData()) == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Data_CRCDataPresent"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("CRCDataPresentData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    if(Continue == true)
    {
        if(std::any_cast<bool>(Result->GetValue()->GetField("ExtendedFlags")->GetField("TagRestrictions")->GetData()) == true)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("TagRestrictionsData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagIsAnUpdate(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Header"})->Get(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Size")->GetData()) == 0x00;
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flag_Data_TagRestrictions(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "Tag_ExtendedHeader_Flag_Header"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        if(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Size")->GetData()) != 0x01)
        {
            Result->GetValue()->AddTag("error", "The size of the tag restriction flags is not equal to 1."s); 
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_BitSet_8Bit_LeastSignificantBitFirst(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Restrictions", PartResult->ExtractValue());
        PartResult->GetValue()->AddTag("error", "This program is missing the interpretation of the tag restriction flags."s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_Tag_ExtendedHeader_Flags(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_8Bit_LeastSignificantBitFirst(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Result->GetValue()->AddTag("synchsafe"s);
        
        const std::bitset<8> & Flags{std::any_cast<const std::bitset<8> &>(Result->GetValue()->GetData())};
        auto Flag{Result->GetValue()->AppendField("Reserved", Flags[7])};
        
        Flag->AddTag("bit index", 7);
        Flag = Result->GetValue()->AppendField("TagIsAnUpdate", Flags[6]);
        Flag->AddTag("bit index", 6);
        Flag->AddTag("bit name", "b"s);
        Flag = Result->GetValue()->AppendField("CRCDataPresent", Flags[5]);
        Flag->AddTag("bit index", 5);
        Flag->AddTag("bit name", "c"s);
        Flag = Result->GetValue()->AppendField("TagRestrictions", Flags[4]);
        Flag->AddTag("bit index", 4);
        Flag->AddTag("bit name", "d"s);
        Flag = Result->GetValue()->AppendField("Reserved", false);
        Flag->AddTag("bit index", 3);
        Flag->AddTag("bit index", 2);
        Flag->AddTag("bit index", 1);
        Flag->AddTag("bit index", 0);
        for(auto FlagIndex = 0; FlagIndex <= 3; ++FlagIndex)
        {
            Continue &= !Flags[FlagIndex];
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetField("String")->GetData());
            }
        }
        else if(TextEncoding == 0x02)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x03)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TextEncoding = std::any_cast<std::uint8_t>(Parameters.at("TextEncoding"));
        
        if(TextEncoding == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetField("String")->GetData());
            }
        }
        else if(TextEncoding == 0x02)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else if(TextEncoding == 0x03)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            // interpretation
            if(Continue == true)
            {
                Result->GetValue()->AddTag("value", Result->GetValue()->GetData());
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_Tag(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_2_Tag_Header(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("TagHeader", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ClaimedSize = Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("TagHeader")->GetField("Size")->GetData()), 0};
        auto PartReader = Inspection::Reader{Reader, ClaimedSize};
        auto MajorVersion = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("TagHeader")->GetField("MajorVersion")->GetData());
        auto PartResult = std::unique_ptr<Inspection::Result>{};
        
        if(MajorVersion == 0x02)
        {
            Result->GetValue()->AddTag("version", "v2.2"s);
            PartResult = Inspection::Get_ID3_2_2_Tag_Body(PartReader, {});
        }
        else if(MajorVersion == 0x03)
        {
            Result->GetValue()->AddTag("version", "v2.3"s);
            if(std::any_cast<bool>(Result->GetValue()->GetField("TagHeader")->GetField("Flags")->GetField("Unsynchronisation")->GetData()) == true)
            {
                auto ID3DeUnsynchronizationEagerFilter = Inspection::ID3DeUnsynchronizationEagerFilter{PartReader.GetBuffer(), PartReader.GetReadPositionInInput(), ClaimedSize};
                auto FilterReader = Inspection::Reader{ID3DeUnsynchronizationEagerFilter};
                
                PartResult = Inspection::Get_ID3_2_3_Tag_Body(FilterReader, {{"ExtendedHeader", Result->GetValue()->GetField("TagHeader")->GetField("Flags")->GetField("ExtendedHeader")->GetData()}});
                ASSERTION(FilterReader.HasRemaining() == false);
                PartReader.AdvancePosition(ClaimedSize);
            }
            else
            {
                PartResult = Inspection::Get_ID3_2_3_Tag_Body(PartReader, {{"ExtendedHeader", Result->GetValue()->GetField("TagHeader")->GetField("Flags")->GetField("ExtendedHeader")->GetData()}});
            }
        }
        else if(MajorVersion == 0x04)
        {
            Result->GetValue()->AddTag("version", "v2.3"s);
            PartResult = Inspection::Get_ID3_2_4_Tag_Body(PartReader, {{"ExtendedHeader", Result->GetValue()->GetField("TagHeader")->GetField("Flags")->GetField("ExtendedHeader")->GetData()}});
        }
        else
        {
            Result->GetValue()->AddTag("version", "unknown"s);
            Result->GetValue()->AddTag("error", "Unknown major version \"" + to_string_cast(MajorVersion) + "\".");
        }
        if(PartResult != nullptr)
        {
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            if(PartReader.IsAtEnd() == false)
            {
                Result->GetValue()->AddTag("error", "The tag size is claimed larger than the actually handled size. See at the end of the tag for superfluous data."s);
                AppendLengthTag(Result->GetValue(), ClaimedSize, "claimed size");
                AppendLengthTag(Result->GetValue(), PartReader.GetConsumedLength(), "handled size");
                AppendLengthTag(Result->GetValue(), PartReader.CalculateRemainingOutputLength(), "rest size");
                
                auto RestReader = Inspection::Reader{PartReader};
                auto RestResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(RestReader, {});
                
                Result->GetValue()->AppendField("Rest", RestResult->ExtractValue());
                Result->GetValue()->GetField("Rest")->AddTag("error", "This is additional unparsed data at the end of the tag."s);
                PartReader.AdvancePosition(RestReader.GetConsumedLength());
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_2_Tag_Header(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_String_ASCII_ByTemplate(PartReader, {{"Template", "ID3"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("FileIdentifier", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("MajorVersion", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("RevisionNumber", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto MajorVersion = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("MajorVersion")->GetData());
        
        if(MajorVersion == 0x02)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.2", "Tag_Header_Flags"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Flags", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MajorVersion == 0x03)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.3", "Tag_Header_Flags"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Flags", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(MajorVersion == 0x04)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "v2.4", "Tag_Header_Flags"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Flags", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "The major version of the tag (" + to_string_cast(MajorVersion) + ") cannot be handled!"s);
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Size", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_ReplayGainAdjustment(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("standard", "Hydrogenaudio ReplayGain"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "ReplayGainAdjustment_NameCode"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("NameCode", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "ReplayGainAdjustment_OriginatorCode"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("OriginatorCode", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "ReplayGainAdjustment_SignBit"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SignBit", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"ID3", "ReplayGainAdjustment_ReplayGainAdjustment"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ReplayGainAdjustment", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto SignBit = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("SignBit")->GetData());
        auto ReplayGainAdjustment = std::any_cast<float>(Result->GetValue()->GetField("ReplayGainAdjustment")->GetTag("interpretation")->GetData());
        
        if(SignBit == 0x01)
        {
            ReplayGainAdjustment *= -1.0f;
        }
        Result->GetValue()->AddTag("interpretation", to_string_cast(ReplayGainAdjustment) + " dB");
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("7bit value"s);
    Result->GetValue()->AddTag("8bit field"s);
    Result->GetValue()->AddTag("synchsafe"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            if((Continue = ((ReadResult.Data & 0x80) == 0x00)) == true)
            {
                Result->GetValue()->SetData(ReadResult.Data);
            }
            else
            {
                Result->GetValue()->AddTag("error", "The unsigned integer should start with an unset bit."s);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("28bit value"s);
    Result->GetValue()->AddTag("32bit field"s);
    Result->GetValue()->AddTag("synchsafe"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            if((Continue = ((ReadResult1.Data & 0x80) == 0x00)) == true)
            {
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    if((Continue = ((ReadResult2.Data & 0x80) == 0x00)) == true)
                    {
                        Inspection::ReadResult ReadResult3;
                        
                        if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                        {
                            if((Continue = ((ReadResult3.Data & 0x80) == 0x00)) == true)
                            {
                                Inspection::ReadResult ReadResult4;
                                
                                if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                                {
                                    if((Continue = ((ReadResult4.Data & 0x80) == 0x00)) == true)
                                    {
                                        Result->GetValue()->SetData((static_cast<std::uint32_t>(ReadResult1.Data) << 21) | (static_cast<std::uint32_t>(ReadResult2.Data) << 14) | (static_cast<std::uint32_t>(ReadResult3.Data) << 7) | static_cast<std::uint32_t>(ReadResult4.Data));
                                    }
                                    else
                                    {
                                        Result->GetValue()->AddTag("error", "The fourth byte of the unsigned integer should start with an unset bit."s);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult4);
                                }
                            }
                            else
                            {
                                Result->GetValue()->AddTag("error", "The third byte of the unsigned integer should start with an unset bit."s);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult3);
                        }
                    }
                    else
                    {
                        Result->GetValue()->AddTag("error", "The second byte of the unsigned integer should start with an unset bit."s);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else
            {
                Result->GetValue()->AddTag("error", "The first byte of the unsigned integer should start with an unset bit."s);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_UnsignedInteger_35Bit_SynchSafe_40Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("35bit value"s);
    Result->GetValue()->AddTag("40bit field"s);
    Result->GetValue()->AddTag("synchsafe"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            if((Continue = ((ReadResult1.Data & 0x80) == 0x00)) == true)
            {
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    if((Continue = ((ReadResult2.Data & 0x80) == 0x00)) == true)
                    {
                        Inspection::ReadResult ReadResult3;
                        
                        if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                        {
                            if((Continue = ((ReadResult3.Data & 0x80) == 0x00)) == true)
                            {
                                Inspection::ReadResult ReadResult4;
                                
                                if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                                {
                                    if((Continue = ((ReadResult4.Data & 0x80) == 0x00)) == true)
                                    {
                                        Inspection::ReadResult ReadResult5;
                                        
                                        if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                                        {
                                            if((Continue = ((ReadResult5.Data & 0x80) == 0x00)) == true)
                                            {
                                                Result->GetValue()->SetData((static_cast<std::uint64_t>(ReadResult1.Data) << 28) | (static_cast<std::uint64_t>(ReadResult2.Data) << 21) | (static_cast<std::uint64_t>(ReadResult3.Data) << 14) | (static_cast<std::uint64_t>(ReadResult4.Data) << 7) | static_cast<std::uint64_t>(ReadResult5.Data));
                                            }
                                            else
                                            {
                                                Result->GetValue()->AddTag("error", "The fourth byte of the unsigned integer should start with an unset bit."s);
                                            }
                                        }
                                        else
                                        {
                                            AppendReadErrorTag(Result->GetValue(), ReadResult5);
                                        }
                                    }
                                    else
                                    {
                                        Result->GetValue()->AddTag("error", "The fourth byte of the unsigned integer should start with an unset bit."s);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult4);
                                }
                            }
                            else
                            {
                                Result->GetValue()->AddTag("error", "The third byte of the unsigned integer should start with an unset bit."s);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult3);
                        }
                    }
                    else
                    {
                        Result->GetValue()->AddTag("error", "The second byte of the unsigned integer should start with an unset bit."s);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else
            {
                Result->GetValue()->AddTag("error", "The first byte of the unsigned integer should start with an unset bit."s);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ID3_GUID(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::Reader PartReader{Reader, Inspection::Length{16, 0}};
        auto PartResult{Inspection::Get_GUID_LittleEndian(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        try
        {
            const Inspection::GUID & GUID{std::any_cast< const Inspection::GUID & >(Result->GetValue()->GetData())};
            
            Result->GetValue()->AddTag("interpretation", Inspection::Get_GUID_Interpretation(GUID));
        }
        catch(Inspection::UnknownValueException & Exception)
        {
            Result->GetValue()->AddTag("interpretation", nullptr);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_IEC_60908_1999_TableOfContents_Track(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::Reader PartReader{Reader, Inspection::Length{1, 0}};
        auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Reserved", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ADR", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("ADR")->GetData()) == 1;
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_IEC_60908_1999_TableOfContents_Track_Control(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Control", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Number", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        if(std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Number")->GetData()) == 0xaa)
        {
            Result->GetValue()->GetField("Number")->AddTag("interpretation", "Lead-Out"s);
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader, Inspection::Length{1, 0}};
        auto PartResult = Inspection::Get_Data_Unset_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Reserved", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_BigEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StartAddress", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_IEC_60908_1999_TableOfContents_Track_Control(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult{Inspection::Get_BitSet_4Bit_MostSignificantBitFirst(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const std::bitset< 4 > & Control{std::any_cast< const std::bitset< 4 > & >(Result->GetValue()->GetData())};
        
        if(Control[1] == true)
        {
            if(Control[0] == true)
            {
                Continue = false;
                
                auto Value{Result->GetValue()->AppendField("Reserved", true)};
                
                Value->AddTag("error", "The track type is \"Data\" so this bit must be off.");
            }
            else
            {
                Result->GetValue()->AppendField("Reserved", false);
            }
            Result->GetValue()->AppendField("TrackType", "Data"s);
            Result->GetValue()->AppendField("DigitalCopyProhibited", !Control[2]);
            if(Control[3] == true)
            {
                Result->GetValue()->AppendField("DataRecorded", "incrementally"s);
            }
            else
            {
                Result->GetValue()->AppendField("DataRecorded", "uninterrupted"s);
            }
        }
        else
        {
            if(Control[0] == true)
            {
                Result->GetValue()->AppendField("NumberOfChannels", 4);
            }
            else
            {
                Result->GetValue()->AppendField("NumberOfChannels", 2);
            }
            Result->GetValue()->AppendField("TrackType", "Audio"s);
            Result->GetValue()->AppendField("DigitalCopyProhibited", !Control[2]);
            Result->GetValue()->AppendField("PreEmphasis", Control[3]);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_IEC_60908_1999_TableOfContents_Tracks(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto FirstTrackNumber = std::any_cast<std::uint8_t>(Parameters.at("FirstTrackNumber"));
        auto LastTrackNumber = std::any_cast<std::uint8_t>(Parameters.at("LastTrackNumber"));
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"IEC_60908_1999", "TableOfContents_Track"})}, {"ElementName", "Track"s}, {"NumberOfElements", static_cast<std::uint64_t>(LastTrackNumber - FirstTrackNumber + 1)}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Tracks", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"IEC_60908_1999", "TableOfContents_LeadOutTrack"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("LeadOutTrack", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_639_2_1998_Code(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::Reader PartReader{Reader, Inspection::Length{3, 0}};
        auto PartResult{Inspection::Get_ASCII_String_Alphabetic_EndedByLength(PartReader, {})};
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        Result->GetValue()->AddTag("standard", "ISO 639-2:1998 (alpha-3)"s);
        
        const std::string & Code{std::any_cast<const std::string &>(Result->GetValue()->GetData())};
        
        try
        {
            Result->GetValue()->AddTag("interpretation", Get_LanguageName_From_ISO_639_2_1998_Code(Code));
        }
        catch(...)
        {
            Result->GetValue()->AddTag("error", "The code \"" + Code + "\" is unknown.");
            Result->GetValue()->AddTag("interpretation", nullptr);
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_Character(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("character"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
            {
                Result->GetValue()->SetData(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data));
            }
            else
            {
                Result->GetValue()->AddTag("error", "The data is not an ISO/IEC 8859-1:1998 character."s);
                Result->GetValue()->GetTag("error")->AddTag("data", std::vector<std::uint8_t>{ReadResult.Data});
                Continue = false;
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        Inspection::ReadResult ReadResult;
        auto NumberOfCharacters{0ul};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data);
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the " + to_string_cast(NumberOfCharacters + 1) + "th character from " + to_string_cast(ReadResult.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        if(NumberOfCharacters == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    //reading
    if(Continue == true)
    {
        std::stringstream Value;
        Inspection::ReadResult ReadResult;
        auto NumberOfCharacters{0ul};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    if(NumberOfCharacters == 0)
                    {
                        Result->GetValue()->AddTag("empty"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data);
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the " + to_string_cast(NumberOfCharacters + 1) + "th character from " + to_string_cast(ReadResult.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    //finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        Inspection::ReadResult ReadResult;
        auto NumberOfCharacters{0ul};
        auto EndedByTermination{false};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
                {
                    NumberOfCharacters += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data);
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character is not an ISO/IEC 8859-1:1998 character.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the " + to_string_cast(NumberOfCharacters + 1) + "th character from " + to_string_cast(ReadResult.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        if(NumberOfCharacters == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        Inspection::ReadResult ReadResult;
        auto NumberOfCharacters{0ul};
        auto NumberOfTerminations{0ul};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    NumberOfTerminations += 1;
                }
                else if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
                {
                    if(NumberOfTerminations == 0)
                    {
                        NumberOfCharacters += 1;
                        Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data);
                    }
                    else
                    {
                        Result->GetValue()->AddTag("ended by error"s);
                        Result->GetValue()->AddTag("error", "After the first termination byte only terminations are allowed, but the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not."s);
                        Continue = false;
                    }
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not an ISO/IEC 8859-1:1998 character or termination.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte from " + to_string_cast(ReadResult.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        if(NumberOfCharacters > 0)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        }
        else
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(NumberOfTerminations > 0)
        {
            Result->GetValue()->AddTag("ended by termination"s);
            if(Reader.IsAtEnd() == true)
            {
                Result->GetValue()->AddTag(to_string_cast(NumberOfTerminations) + " terminations until length");
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by length"s);
            Result->GetValue()->AddTag("error", "The string must be ended by at least one termination."s);
            Continue = false;
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 8859-1:1998"s);
    Result->GetValue()->AddTag("encoding", "ISO/IEC 8859-1:1998"s);
    // reading
    if(Continue == true)
    {
        std::stringstream Value;
        Inspection::ReadResult ReadResult;
        auto NumberOfCharacters{0ul};
        auto NumberOfTerminations{0ul};
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            if(Reader.Read8Bits(ReadResult) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    NumberOfTerminations += 1;
                }
                else if(Is_ISO_IEC_8859_1_1998_Character(ReadResult.Data) == true)
                {
                    if(NumberOfTerminations == 0)
                    {
                        NumberOfCharacters += 1;
                        Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(ReadResult.Data);
                    }
                    else
                    {
                        Result->GetValue()->AddTag("ended by error"s);
                        Result->GetValue()->AddTag("error", "After the first termination byte only terminations are allowed, but the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not."s);
                        Continue = false;
                    }
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte is not an ISO/IEC 8859-1:1998 character or termination.");
                    Continue = false;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the " + to_string_cast(NumberOfCharacters + NumberOfTerminations + 1) + "th byte from " + to_string_cast(ReadResult.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        if(NumberOfCharacters > 0)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters");
        }
        else
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(NumberOfTerminations > 0)
        {
            Result->GetValue()->AddTag("ended by termination"s);
            if(Reader.IsAtEnd() == true)
            {
                Result->GetValue()->AddTag(to_string_cast(NumberOfTerminations) + " terminations until length");
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto Data = std::vector<std::uint8_t>{ReadResult1.Data, ReadResult2.Data};
                
                Result->GetValue()->SetData(Data);
                if((Data[0] == 0xfe) && (Data[1] == 0xff))
                {
                    Result->GetValue()->AddTag("interpretation", "BigEndian"s);
                }
                else if((Data[0] == 0xff) && (Data[1] == 0xfe))
                {
                    Result->GetValue()->AddTag("interpretation", "LittleEndian"s);
                }
                else
                {
                    Result->GetValue()->AddTag("error", "The byte combination is not a valid ISO/IEC 10646-1:1993 UCS-2 byte order mark."s);
                    Result->GetValue()->AddTag("interpretation", nullptr);
                    Continue = false;
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto CodePoint{static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 8) | static_cast<std::uint32_t>(ReadResult2.Data))};
                
                Result->GetValue()->AppendField("codepoint", CodePoint);
                Result->GetValue()->SetData(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_Character_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // verification
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto CodePoint{static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult2.Data) << 8) | static_cast<std::uint32_t>(ReadResult1.Data))};
                
                Result->GetValue()->AppendField("codepoint", CodePoint);
                Result->GetValue()->SetData(Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if(Reader.Read8Bits(ReadResult1) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if(Reader.Read8Bits(ReadResult2) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 8) | static_cast<std::uint32_t>(ReadResult2.Data)));
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the second byte from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            Result->GetValue()->AddTag("error", "Could not read the first byte from " + to_string_cast(ReadResult1.InputLength) + " bytes and bits of remaining data.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if(Reader.Read8Bits(ReadResult1) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if(Reader.Read8Bits(ReadResult2) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult2.Data) << 8) | static_cast<std::uint32_t>(ReadResult1.Data)));
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the second byte from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            Result->GetValue()->AddTag("error", "Could not read the first byte from " + to_string_cast(ReadResult1.InputLength) + " bytes and bits of remaining data.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    if(NumberOfCodePoints == 0)
                    {
                        Result->GetValue()->AddTag("empty"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    if(NumberOfCodePoints == 0)
                    {
                        Result->GetValue()->AddTag("empty"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UCS-2 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ByteOrderMark", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ByteOrderMark = std::any_cast<const std::string &>(Result->GetValue()->GetField("ByteOrderMark")->GetTag("interpretation")->GetData());
        
        if(ByteOrderMark == "BigEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(ByteOrderMark == "LittleEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTermination(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UCS-2"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_ByteOrderMark(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ByteOrderMark", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ByteOrderMark = std::any_cast<const std::string &>(Result->GetValue()->GetField("ByteOrderMark")->GetTag("interpretation")->GetData());
        
        if(ByteOrderMark == "BigEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_BigEndian_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(ByteOrderMark == "LittleEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if(Reader.Read8Bits(ReadResult1) == true)
        {
            if((ReadResult1.Data & 0x80) == 0x00)
            {
                Result->GetValue()->SetData(static_cast<std::uint32_t>(ReadResult1.Data));
            }
            else if((ReadResult1.Data & 0xe0) == 0xc0)
            {
                Inspection::ReadResult ReadResult2;
                
                if(Reader.Read8Bits(ReadResult2) == true)
                {
                    if((ReadResult2.Data & 0xc0) == 0x80)
                    {
                        Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x1f) << 6) | static_cast<std::uint32_t>(ReadResult2.Data & 0x3f));
                    }
                    else
                    {
                        Continue = false;
                    }
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "Could not read the second byte of the UTF-8 codepoint from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                    Continue = false;
                }
            }
            else if((ReadResult1.Data & 0xf0) == 0xe0)
            {
                Inspection::ReadResult ReadResult2;
                Inspection::ReadResult ReadResult3;
                
                if(Reader.Read8Bits(ReadResult2) == true)
                {
                    if(Reader.Read8Bits(ReadResult3) == true)
                    {
                        if(((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80))
                        {
                            Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x0f) << 12)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult3.Data & 0x3f));
                        }
                        else
                        {
                            Continue = false;
                        }
                    }
                    else
                    {
                        Result->GetValue()->AddTag("ended by error"s);
                        Result->GetValue()->AddTag("error", "Could not read the third byte of the UTF-8 codepoint from " + to_string_cast(ReadResult3.InputLength) + " bytes and bits of remaining data.");
                        Continue = false;
                    }
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "Could not read the second byte of the UTF-8 codepoint from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                    Continue = false;
                }
            }
            else if((ReadResult1.Data & 0xf8) == 0xf0)
            {
                Inspection::ReadResult ReadResult2;
                Inspection::ReadResult ReadResult3;
                Inspection::ReadResult ReadResult4;
                
                if(Reader.Read8Bits(ReadResult2) == true)
                {
                    if(Reader.Read8Bits(ReadResult3) == true)
                    {
                        if(Reader.Read8Bits(ReadResult4) == true)
                        {
                            if(((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80))
                            {
                                Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x07) << 18)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 12) | static_cast<std::uint32_t>((ReadResult3.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult4.Data & 0x3f));
                            }
                            else
                            {
                                Continue = false;
                            }
                        }
                        else
                        {
                            Result->GetValue()->AddTag("ended by error"s);
                            Result->GetValue()->AddTag("error", "Could not read the fourth byte of the UTF-8 codepoint from " + to_string_cast(ReadResult3.InputLength) + " bytes and bits of remaining data.");
                            Continue = false;
                        }
                    }
                    else
                    {
                        Result->GetValue()->AddTag("ended by error"s);
                        Result->GetValue()->AddTag("error", "Could not read the third byte of the UTF-8 codepoint from " + to_string_cast(ReadResult3.InputLength) + " bytes and bits of remaining data.");
                        Continue = false;
                    }
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "Could not read the second byte of the UTF-8 codepoint from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                    Continue = false;
                }
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            Result->GetValue()->AddTag("error", "Could not read the first byte of the UTF-8 codepoint from " + to_string_cast(ReadResult1.InputLength) + " bytes and bits of remaining data.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-8"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            NumberOfCodePoints += 1;
            Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData()));
            if(Continue == false)
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points"s);
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-8"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    if(NumberOfCodePoints == 0)
                    {
                        Result->GetValue()->AddTag("empty"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
                Continue = false;
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "Unicode/ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-8"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
    
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-8 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto Data = std::vector<std::uint8_t>{ReadResult1.Data, ReadResult2.Data};
                
                Result->GetValue()->SetData(Data);
                if((Data[0] == 0xfe) && (Data[1] == 0xff))
                {
                    Result->GetValue()->AddTag("interpretation", "BigEndian"s);
                }
                else if((Data[0] == 0xff) && (Data[1] == 0xfe))
                {
                    Result->GetValue()->AddTag("interpretation", "LittleEndian"s);
                }
                else
                {
                    Result->GetValue()->AddTag("error", "The byte combination is not a valid ISO/IEC 10646-1:1993 UTF-16 byte order mark."s);
                    Result->GetValue()->AddTag("interpretation", nullptr);
                    Continue = false;
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ByteOrderMark", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & ByteOrderMark = std::any_cast<const std::string &>(Result->GetValue()->GetField("ByteOrderMark")->GetTag("interpretation")->GetData());
        
        if(ByteOrderMark == "BigEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(PartReader, {});
        
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(ByteOrderMark == "LittleEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(PartReader, {});
        
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_String_WithByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16_ByteOrderMark(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("ByteOrderMark", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & ByteOrderMark = std::any_cast<const std::string &>(Result->GetValue()->GetField("ByteOrderMark")->GetTag("interpretation")->GetData());
        
        if(ByteOrderMark == "BigEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(PartReader, {});
        
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(ByteOrderMark == "LittleEndian")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(PartReader, {});
        
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("String", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto FirstCodeUnitResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {})};
        
        Continue = FirstCodeUnitResult->GetSuccess();
        if(Continue == true)
        {
            auto FirstCodeUnit{std::any_cast<std::uint16_t>(FirstCodeUnitResult->GetValue()->GetData())};
            
            if((FirstCodeUnit < 0xd800) || (FirstCodeUnit >= 0xe000))
            {
                std::uint32_t Value{FirstCodeUnit};
                
                Result->GetValue()->SetData(Value);
            }
            else if((FirstCodeUnit >= 0xd800) && (FirstCodeUnit < 0xdc00))
            {
                auto SecondCodeUnitResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Reader, {})};
                
                Continue = SecondCodeUnitResult->GetSuccess();
                if(Continue == true)
                {
                    auto SecondCodeUnit{std::any_cast<std::uint16_t>(SecondCodeUnitResult->GetValue()->GetData())};
                    
                    if((SecondCodeUnit >= 0xdc00) && (SecondCodeUnit < 0xe000))
                    {
                        std::uint32_t Value{(static_cast<std::uint32_t>(FirstCodeUnit - 0xd800) << 10) | static_cast<std::uint32_t>(SecondCodeUnit - 0xdc00)};
                        
                        Result->GetValue()->SetData(Value);
                    }
                    else
                    {
                        Continue = false;
                    }
                }
            }
            else
            {
                Continue = false;
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if(Reader.Read8Bits(ReadResult1) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if(Reader.Read8Bits(ReadResult2) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((static_cast<std::uint16_t>(ReadResult1.Data) << 8) | static_cast<std::uint16_t>(ReadResult2.Data)));
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the second byte from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            Result->GetValue()->AddTag("error", "Could not read the first byte from " + to_string_cast(ReadResult1.InputLength) + " bytes and bits of remaining data.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("big endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    if(NumberOfCodePoints == 0)
                    {
                        Result->GetValue()->AddTag("emtpy"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("big endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto FirstCodeUnitResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {})};
        
        Continue = FirstCodeUnitResult->GetSuccess();
        if(Continue == true)
        {
            auto FirstCodeUnit{std::any_cast<std::uint16_t>(FirstCodeUnitResult->GetValue()->GetData())};
            
            if((FirstCodeUnit < 0xd800) || (FirstCodeUnit >= 0xe000))
            {
                std::uint32_t Value{FirstCodeUnit};
                
                Result->GetValue()->SetData(Value);
            }
            else if((FirstCodeUnit >= 0xd800) && (FirstCodeUnit < 0xdc00))
            {
                auto SecondCodeUnitResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Reader, {})};
                
                Continue = SecondCodeUnitResult->GetSuccess();
                if(Continue == true)
                {
                    auto SecondCodeUnit{std::any_cast<std::uint16_t>(SecondCodeUnitResult->GetValue()->GetData())};
                    
                    if((SecondCodeUnit >= 0xdc00) && (SecondCodeUnit < 0xe000))
                    {
                        std::uint32_t Value{(static_cast<std::uint32_t>(FirstCodeUnit - 0xd800) << 10) | static_cast<std::uint32_t>(SecondCodeUnit - 0xdc00)};
                        
                        Result->GetValue()->SetData(Value);
                    }
                    else
                    {
                        Continue = false;
                    }
                }
            }
            else
            {
                Continue = false;
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if(Reader.Read8Bits(ReadResult1) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if(Reader.Read8Bits(ReadResult2) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((static_cast<std::uint16_t>(ReadResult2.Data) << 8) | static_cast<std::uint16_t>(ReadResult1.Data)));
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "Could not read the second byte from " + to_string_cast(ReadResult2.InputLength) + " bytes and bits of remaining data.");
                Continue = false;
            }
        }
        else
        {
            Result->GetValue()->AddTag("ended by error"s);
            Result->GetValue()->AddTag("error", "Could not read the first byte from " + to_string_cast(ReadResult1.InputLength) + " bytes and bits of remaining data.");
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{std::any_cast<std::uint64_t>(Parameters.at("NumberOfCodePoints"))};
        auto CodePointIndex{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true) && (CodePointIndex < NumberOfCodePoints))
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    Result->GetValue()->AddTag("error", "The string MUST NOT contain a termination."s);
                    Continue = false;
                    
                    break;
                }
                else
                {
                    CodePointIndex += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                    Reader.AdvancePosition(PartReader.GetConsumedLength());
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(CodePointIndex + 1) + "th code point is not a valid UTF-16 code point.");
                Continue = false;
            }
        }
        if(CodePointIndex == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        Result->GetValue()->AddTag(to_string_cast(CodePointIndex) + " code points");
        if(Reader.IsAtEnd() == true)
        {
            Result->GetValue()->AddTag("ended by length"s);
        }
        else
        {
            Result->GetValue()->AddTag("ended by number of code points"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTermination(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    if(NumberOfCodePoints == 0)
                    {
                        Result->GetValue()->AddTag("emtpy"s);
                    }
                    Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
                    Result->GetValue()->AddTag("ended by termination"s);
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
            }
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    if(Reader.HasRemaining() == true)
                    {
                        Result->GetValue()->AddTag("error", "The termination must be the last code point in the availble length."s);
                        Continue = false;
                    }
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
                Continue = false;
            }
        }
        if(EndedByTermination == false)
        {
            Result->GetValue()->AddTag("error", "The string must be ended by a termination."s);
            Continue = false;
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // reading
    if(Continue == true)
    {
        auto EndedByTermination{false};
        auto NumberOfCodePoints{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    
                    break;
                }
                else
                {
                    NumberOfCodePoints += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCodePoints + 1) + "th code point is not a valid UTF-16 code point.");
                Continue = false;
            }
        }
        if(NumberOfCodePoints == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(NumberOfCodePoints) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ISO/IEC 10646-1:1993"s);
    Result->GetValue()->AddTag("encoding", "UTF-16"s);
    Result->GetValue()->AddTag("little endian"s);
    Result->GetValue()->AddTag("without byte order mark"s);
    // no verification, because not ended by length
    // reading
    if(Continue == true)
    {
        auto NumberOfCodePoints{std::any_cast<std::uint64_t>(Parameters.at("NumberOfCodePoints"))};
        auto EndedByTermination{false};
        auto CodePointIndex{0ul};
        std::stringstream Value;
        
        while((Continue == true) && (Reader.HasRemaining() == true) && (CodePointIndex < NumberOfCodePoints))
        {
            auto PartResult{Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint(Reader, {})};
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                auto CodePoint{std::any_cast<std::uint32_t>(PartResult->GetValue()->GetData())};
                
                if(CodePoint == 0x00000000)
                {
                    EndedByTermination = true;
                    if(CodePointIndex + 1 != NumberOfCodePoints)
                    {
                        Result->GetValue()->AddTag("error", "With the termination code point, the string must contain exactly " + to_string_cast(NumberOfCodePoints) + " code points."s);
                        Continue = false;
                    }
                    
                    break;
                }
                else
                {
                    CodePointIndex += 1;
                    Value << Get_ISO_IEC_10646_1_1993_UTF_8_Character_FromUnicodeCodePoint(CodePoint);
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                Result->GetValue()->AddTag("error", "The " + to_string_cast(CodePointIndex + 1) + "th code point is not a valid UTF-16 code point.");
                Continue = false;
            }
        }
        if(EndedByTermination == false)
        {
            Result->GetValue()->AddTag("error", "The string must be ended by a termination."s);
            Continue = false;
        }
        if(CodePointIndex == 0)
        {
            Result->GetValue()->AddTag("empty"s);
        }
        if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag(to_string_cast(CodePointIndex) + " code points + termination");
        }
        else
        {
            Result->GetValue()->AddTag(to_string_cast(CodePointIndex) + " code points");
        }
        if(Reader.IsAtEnd() == true)
        {
            if(EndedByTermination == true)
            {
                Result->GetValue()->AddTag("ended by termination and length"s);
            }
            else
            {
                Result->GetValue()->AddTag("ended by length"s);
            }
        }
        else if(EndedByTermination == true)
        {
            Result->GetValue()->AddTag("ended by termination"s);
        }
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("floating point"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("standard", "ISO/IEC/IEEE-60559:2011 binary32"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        std::uint8_t Data[4];
                        
                        Data[0] = ReadResult1.Data;
                        Data[1] = ReadResult2.Data;
                        Data[2] = ReadResult3.Data;
                        Data[3] = ReadResult4.Data;
                        Result->GetValue()->SetData(*reinterpret_cast< const float * const >(Data));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_MPEG_1_Frame(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"MPEG", "1", "FrameHeader"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Header", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto ProtectionBit = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Header")->GetField("ProtectionBit")->GetData());
        
        if(ProtectionBit == 0x00)
        {
            auto PartReader = Inspection::Reader{Reader, Inspection::Length{0, 16}};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("ErrorCheck", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // reading
    if(Continue == true)
    {
        auto LayerDescription = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Header")->GetField("LayerDescription")->GetData());
        auto BitRate = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("BitRateIndex")->GetTag("value")->GetData());
        auto SamplingFrequency = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("SamplingFrequency")->GetTag("value")->GetData());
        auto PaddingBit = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("Header")->GetField("PaddingBit")->GetData());
        auto FrameLength = Inspection::Length{};
        
        if(LayerDescription == 0x03)
        {
            FrameLength.Set((12 * BitRate / SamplingFrequency + PaddingBit) * 4, 0);
        }
        else if((LayerDescription == 0x01) || (LayerDescription == 0x02))
        {
            FrameLength.Set(144 * BitRate / SamplingFrequency + PaddingBit, 0);
        }
        if(Reader.Has(FrameLength - Reader.GetConsumedLength()) == true)
        {
            auto PartReader = Inspection::Reader{Reader, FrameLength - Reader.GetConsumedLength()};
            auto PartResult = Inspection::Get_Data_SetOrUnset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("AudioData", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_MPEG_1_FrameHeader_BitRateIndex(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto LayerDescription = std::any_cast<std::uint8_t>(Parameters.at("LayerDescription"));
        auto BitRateIndex = std::any_cast<std::uint8_t>(Result->GetValue()->GetData());
        
        if(LayerDescription == 0x03)
        {
            if(BitRateIndex == 0x00)
            {
                Result->GetValue()->AddTag("interpretation", "free format"s);
            }
            else if(BitRateIndex == 0x01)
            {
                Result->GetValue()->AddTag("value", 32000u);
                Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
            }
            else if(BitRateIndex == 0x02)
            {
                Result->GetValue()->AddTag("value", 64000u);
                Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
            }
            else if(BitRateIndex == 0x03)
            {
                Result->GetValue()->AddTag("value", 96000u);
                Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
            }
            else if(BitRateIndex == 0x04)
            {
                Result->GetValue()->AddTag("value", 128000u);
                Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
            }
            else if(BitRateIndex == 0x05)
            {
                Result->GetValue()->AddTag("value", 160000u);
                Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
            }
            else if(BitRateIndex == 0x06)
            {
                Result->GetValue()->AddTag("value", 192000u);
                Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
            }
            else if(BitRateIndex == 0x07)
            {
                Result->GetValue()->AddTag("value", 224000u);
                Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
            }
            else if(BitRateIndex == 0x08)
            {
                Result->GetValue()->AddTag("value", 256000u);
                Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
            }
            else if(BitRateIndex == 0x09)
            {
                Result->GetValue()->AddTag("value", 288000u);
                Result->GetValue()->AddTag("interpretation", "288 kbit/s"s);
            }
            else if(BitRateIndex == 0x0a)
            {
                Result->GetValue()->AddTag("value", 320000u);
                Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
            }
            else if(BitRateIndex == 0x0b)
            {
                Result->GetValue()->AddTag("value", 352000u);
                Result->GetValue()->AddTag("interpretation", "352 kbit/s"s);
            }
            else if(BitRateIndex == 0x0c)
            {
                Result->GetValue()->AddTag("value", 384000u);
                Result->GetValue()->AddTag("interpretation", "384 kbit/s"s);
            }
            else if(BitRateIndex == 0x0d)
            {
                Result->GetValue()->AddTag("value", 416000u);
                Result->GetValue()->AddTag("interpretation", "416 kbit/s"s);
            }
            else if(BitRateIndex == 0x0e)
            {
                Result->GetValue()->AddTag("value", 448000u);
                Result->GetValue()->AddTag("interpretation", "448 kbit/s"s);
            }
            else if(BitRateIndex == 0x0f)
            {
                Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
                Result->GetValue()->AddTag("interpretation", nullptr);
                Continue = false;
            }
        }
        else if(LayerDescription == 0x02)
        {
            if(BitRateIndex == 0x00)
            {
                Result->GetValue()->AddTag("interpretation", "free format"s);
            }
            else if(BitRateIndex == 0x01)
            {
                Result->GetValue()->AddTag("value", 32000u);
                Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
            }
            else if(BitRateIndex == 0x02)
            {
                Result->GetValue()->AddTag("value", 48000u);
                Result->GetValue()->AddTag("interpretation", "48 kbit/s"s);
            }
            else if(BitRateIndex == 0x03)
            {
                Result->GetValue()->AddTag("value", 56000u);
                Result->GetValue()->AddTag("interpretation", "56 kbit/s"s);
            }
            else if(BitRateIndex == 0x04)
            {
                Result->GetValue()->AddTag("value", 64000u);
                Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
            }
            else if(BitRateIndex == 0x05)
            {
                Result->GetValue()->AddTag("value", 80000u);
                Result->GetValue()->AddTag("interpretation", "80 kbit/s"s);
            }
            else if(BitRateIndex == 0x06)
            {
                Result->GetValue()->AddTag("value", 96000u);
                Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
            }
            else if(BitRateIndex == 0x07)
            {
                Result->GetValue()->AddTag("value", 112000u);
                Result->GetValue()->AddTag("interpretation", "112 kbit/s"s);
            }
            else if(BitRateIndex == 0x08)
            {
                Result->GetValue()->AddTag("value", 128000u);
                Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
            }
            else if(BitRateIndex == 0x09)
            {
                Result->GetValue()->AddTag("value", 160000u);
                Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
            }
            else if(BitRateIndex == 0x0a)
            {
                Result->GetValue()->AddTag("value", 192000u);
                Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
            }
            else if(BitRateIndex == 0x0b)
            {
                Result->GetValue()->AddTag("value", 224000u);
                Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
            }
            else if(BitRateIndex == 0x0c)
            {
                Result->GetValue()->AddTag("value", 256000u);
                Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
            }
            else if(BitRateIndex == 0x0d)
            {
                Result->GetValue()->AddTag("value", 320000u);
                Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
            }
            else if(BitRateIndex == 0x0e)
            {
                Result->GetValue()->AddTag("value", 384000u);
                Result->GetValue()->AddTag("interpretation", "384 kbit/s"s);
            }
            else if(BitRateIndex == 0x0f)
            {
                Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
                Result->GetValue()->AddTag("interpretation", nullptr);
                Continue = false;
            }
        }
        else if(LayerDescription == 0x01)
        {
            if(BitRateIndex == 0x00)
            {
                Result->GetValue()->AddTag("interpretation", "free format"s);
            }
            else if(BitRateIndex == 0x01)
            {
                Result->GetValue()->AddTag("value", 32000u);
                Result->GetValue()->AddTag("interpretation", "32 kbit/s"s);
            }
            else if(BitRateIndex == 0x02)
            {
                Result->GetValue()->AddTag("value", 40000u);
                Result->GetValue()->AddTag("interpretation", "40 kbit/s"s);
            }
            else if(BitRateIndex == 0x03)
            {
                Result->GetValue()->AddTag("value", 48000u);
                Result->GetValue()->AddTag("interpretation", "48 kbit/s"s);
            }
            else if(BitRateIndex == 0x04)
            {
                Result->GetValue()->AddTag("value", 56000u);
                Result->GetValue()->AddTag("interpretation", "56 kbit/s"s);
            }
            else if(BitRateIndex == 0x05)
            {
                Result->GetValue()->AddTag("value", 64000u);
                Result->GetValue()->AddTag("interpretation", "64 kbit/s"s);
            }
            else if(BitRateIndex == 0x06)
            {
                Result->GetValue()->AddTag("value", 80000u);
                Result->GetValue()->AddTag("interpretation", "80 kbit/s"s);
            }
            else if(BitRateIndex == 0x07)
            {
                Result->GetValue()->AddTag("value", 96000u);
                Result->GetValue()->AddTag("interpretation", "96 kbit/s"s);
            }
            else if(BitRateIndex == 0x08)
            {
                Result->GetValue()->AddTag("value", 112000u);
                Result->GetValue()->AddTag("interpretation", "112 kbit/s"s);
            }
            else if(BitRateIndex == 0x09)
            {
                Result->GetValue()->AddTag("value", 128000u);
                Result->GetValue()->AddTag("interpretation", "128 kbit/s"s);
            }
            else if(BitRateIndex == 0x0a)
            {
                Result->GetValue()->AddTag("value", 160000u);
                Result->GetValue()->AddTag("interpretation", "160 kbit/s"s);
            }
            else if(BitRateIndex == 0x0b)
            {
                Result->GetValue()->AddTag("value", 192000u);
                Result->GetValue()->AddTag("interpretation", "192 kbit/s"s);
            }
            else if(BitRateIndex == 0x0c)
            {
                Result->GetValue()->AddTag("value", 224000u);
                Result->GetValue()->AddTag("interpretation", "224 kbit/s"s);
            }
            else if(BitRateIndex == 0x0d)
            {
                Result->GetValue()->AddTag("value", 256000u);
                Result->GetValue()->AddTag("interpretation", "256 kbit/s"s);
            }
            else if(BitRateIndex == 0x0e)
            {
                Result->GetValue()->AddTag("value", 320000u);
                Result->GetValue()->AddTag("interpretation", "320 kbit/s"s);
            }
            else if(BitRateIndex == 0x0f)
            {
                Result->GetValue()->AddTag("error", "The bit rate index \"" + to_string_cast(BitRateIndex) + "\" in layer \"" + to_string_cast(LayerDescription) + "\" is reserved and should not be used.");
                Result->GetValue()->AddTag("interpretation", nullptr);
                Continue = false;
            }
        }
        else
        {
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_MPEG_1_FrameHeader_Mode(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_2Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Mode = std::any_cast<std::uint8_t>(Result->GetValue()->GetData());
        
        switch(Mode)
        {
        case 0x00:
            {
                Result->GetValue()->AddTag("interpretation", "stereo"s);
                
                break;
            }
        case 0x01:
            {
                auto LayerDescription = std::any_cast<std::uint8_t>(Parameters.at("LayerDescription"));
                
                if((LayerDescription == 0x03) || (LayerDescription == 0x02))
                {
                    Result->GetValue()->AddTag("interpretation", "joint stereo (intensity_stereo)"s);
                }
                else if(LayerDescription == 0x01)
                {
                    Result->GetValue()->AddTag("interpretation", "joint stereo (intensity_stereo and/or ms_stereo)"s);
                }
                else
                {
                    // LayerDescription is a 2-bit value. Value 0 is reserved and an error will be produced when reading that value; 1, 2 and 3 are handled above.
                    IMPOSSIBLE_CODE_REACHED("LayerDescription == " + to_string_cast(LayerDescription));
                }
                
                break;
            }
        case 0x02:
            {
                Result->GetValue()->AddTag("interpretation", "dual_channel"s);
                
                break;
            }
        case 0x03:
            {
                Result->GetValue()->AddTag("interpretation", "single_channel"s);
                
                break;
            }
        default:
            {
                // all possile values of a 2-bit value have been handled
                IMPOSSIBLE_CODE_REACHED("Mode == " + to_string_cast(Mode));
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_MPEG_1_FrameHeader_ModeExtension(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_2Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto Mode = std::any_cast<std::uint8_t>(Parameters.at("Mode"));
        auto LayerDescription = std::any_cast<std::uint8_t>(Parameters.at("LayerDescription"));
        auto ModeExtension = std::any_cast<std::uint8_t>(Result->GetValue()->GetData());
        
        if(Mode == 0x01)
        {
            if((LayerDescription == 0x03) || (LayerDescription == 0x02))
            {
                if(ModeExtension == 0x00)
                {
                    Result->GetValue()->AddTag("subbands 4-31 in intensity_stereo, bound==4"s);
                }
                else if(ModeExtension == 0x01)
                {
                    Result->GetValue()->AddTag("subbands 8-31 in intensity_stereo, bound==8"s);
                }
                else if(ModeExtension == 0x02)
                {
                    Result->GetValue()->AddTag("subbands 12-31 in intensity_stereo, bound==12"s);
                }
                else if(ModeExtension == 0x03)
                {
                    Result->GetValue()->AddTag("subbands 16-31 in intensity_stereo, bound==16"s);
                }
                else
                {
                    // every 2-bit value is either 0, 1, 2 or 3
                    IMPOSSIBLE_CODE_REACHED("ModeExtension == " + to_string_cast(ModeExtension));
                }
            }
            else if(LayerDescription == 0x01)
            {
                if(ModeExtension == 0x00)
                {
                    Result->GetValue()->AddTag("ms_stereo", "off"s);
                    Result->GetValue()->AddTag("intensity_stereo", "off"s);
                }
                else if(ModeExtension == 0x01)
                {
                    Result->GetValue()->AddTag("ms_stereo", "off"s);
                    Result->GetValue()->AddTag("intensity_stereo", "on"s);
                }
                else if(ModeExtension == 0x02)
                {
                    Result->GetValue()->AddTag("ms_stereo", "on"s);
                    Result->GetValue()->AddTag("intensity_stereo", "off"s);
                }
                else if(ModeExtension == 0x03)
                {
                    Result->GetValue()->AddTag("ms_stereo", "on"s);
                    Result->GetValue()->AddTag("intensity_stereo", "on"s);
                }
                else
                {
                    // every 2-bit value is either 0, 1, 2 or 3
                    IMPOSSIBLE_CODE_REACHED("ModeExtension == " + to_string_cast(ModeExtension));
                }
            }
            else
            {
                // LayerDescription is a 2-bit value. Value 0 is reserved and an error will be produced when reading that value; 1, 2 and 3 are handled above.
                IMPOSSIBLE_CODE_REACHED("LayerDescription == " + to_string_cast(LayerDescription));
            }
        }
        else
        {
            Result->GetValue()->AddTag("<ignored>"s);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Packet(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        if(Reader.HasRemaining() == false)
        {
            Result->GetValue()->AddTag("interpretation", "Ogg nil"s);
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Ogg_Vorbis_AudioPacket(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            if(Continue == true)
            {
                Result->GetValue()->Extend(PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else
            {
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::Get_Ogg_Vorbis_HeaderPacket(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                if(Continue == true)
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    Reader.AdvancePosition(PartReader.GetConsumedLength());
                }
                else
                {
                    auto PartReader = Inspection::Reader{Reader};
                    auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
                    
                    Continue = PartResult->GetSuccess();
                    Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                    Result->GetValue()->AddTag("interpretation", "Ogg unknown"s);
                    Reader.AdvancePosition(PartReader.GetConsumedLength());
                }
            }
        }
    }
    // reading
    if(Continue == true)
    {
        if(Reader.GetReadPositionInInput().GetBits() > 0)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Data_Unset_Until8BitAlignment(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("EndOfPacketAlignment", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Page(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_String_ASCII_ByTemplate(PartReader, {{"Template", "OggS"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("CapturePattern", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("StreamStructureVersion", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Ogg_Page_HeaderType(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("HeaderType", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_64Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("GranulePosition", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BitStreamSerialNumber", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PageSequenceNumber", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Checksum", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PageSegments", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PageSegments = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("PageSegments")->GetData());
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Number", "Integer", "Unsigned", "8Bit"})}, {"NumberOfElements", static_cast<std::uint64_t>(PageSegments)}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("SegmentTable", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PacketLength = Inspection::Length{0, 0};
        
        for(const auto & SegmentTableEntryValue : Result->GetValue()->GetField("SegmentTable")->GetFields())
        {
            auto SegmentTableEntry = std::any_cast<std::uint8_t>(SegmentTableEntryValue->GetData());
            
            PacketLength += Inspection::Length{SegmentTableEntry, 0};
            if(SegmentTableEntry != 0xff)
            {
                auto PartReader = Inspection::Reader{Reader, PacketLength};
                // the packet ends here, read its content and try interpretation
                auto PartResult = Inspection::Get_Ogg_Packet(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Packet", PartResult->ExtractValue());
                ASSERTION(PacketLength == PartReader.GetConsumedLength());
                // No matter what data gets read before - successfully or unsuccessfully - we heed the values from the segment table!
                Reader.AdvancePosition(PartReader.GetConsumedLength());
                PacketLength = Inspection::Length{0, 0};
            }
        }
        if(PacketLength > Inspection::Length{0, 0})
        {
            auto PartReader = Inspection::Reader{Reader, PacketLength};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Result->GetValue()->AppendField("Packet", PartResult->ExtractValue());
            Result->GetValue()->GetField("Packet")->AddTag("error", "The packet spans multiple pages, which is not yet supported."s);
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Page_HeaderType(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_BitSet_8Bit_LeastSignificantBitFirst(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const auto & HeaderType = std::any_cast<const std::bitset<8> &>(Result->GetValue()->GetData());
        
        Result->GetValue()->AppendField("Continuation", HeaderType[0]);
        Result->GetValue()->AppendField("BeginOfStream", HeaderType[1]);
        Result->GetValue()->AppendField("EndOfStream", HeaderType[2]);
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Stream(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByPredicate(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Ogg", "Page"})}, {"ElementName", "Page"s}, {"EndPredicate", std::function<bool (Inspection::Value *)>{[](Inspection::Value * ElementValue) { return std::any_cast<bool>(ElementValue->GetField("HeaderType")->GetField("EndOfStream")->GetData()); }}}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Pages", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Vorbis_AudioPacket(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PacketType", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto PacketType = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("PacketType")->GetData());
        
        if(PacketType == 0x00)
        {
            Result->GetValue()->GetField("PacketType")->AddTag("interpretation", "Vorbis Audio"s);
        }
        else
        {
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Data_SetOrUnset_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Vorbis_CommentHeader(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Ogg_Vorbis_CommentHeader_WithoutFramingFlag(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader, Inspection::Length{0, 1}};
        auto PartResult = Inspection::Get_Boolean_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("FramingFlag", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<bool>(Result->GetValue()->GetField("FramingFlag")->GetData());
    }
    //finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("VendorLength", PartResult->ExtractValue());
        Result->GetValue()->GetField("VendorLength")->AddTag("unit", "bytes"s);
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader, Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("VendorLength")->GetData()), 0}};
        auto PartResult = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Vendor", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("UserCommentListLength", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto UserCommentListLength = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("UserCommentListLength")->GetData());
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Array_EndedByNumberOfElements(PartReader, {{"ElementType", Inspection::g_TypeRepository.GetType(std::vector<std::string>{"Ogg", "Vorbis", "CommentHeader_UserComment"})}, {"ElementName", "UserComment"s}, {"NumberOfElements", static_cast<std::uint64_t>(UserCommentListLength)}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("UserCommentList", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Vorbis_HeaderPacket(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"Ogg", "Vorbis", "HeaderPacket_Type"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("PacketType", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_String_ASCII_ByTemplate(PartReader, {{"Template", "vorbis"s}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("VorbisIdentifier", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PacketType = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("PacketType")->GetData());
        
        if(PacketType == 0x01)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Ogg_Vorbis_IdentificationHeader(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(PacketType == 0x03)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Ogg_Vorbis_CommentHeader(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(PacketType == 0x05)
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Data_SetOrUnset_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            Result->GetValue()->AddTag("error", "The packet type " + to_string_cast(PacketType) + " is not a valid packet type for a header packet.");
            Result->GetValue()->AddTag("standard", "Vorbis I specification"s);
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_Ogg_Vorbis_IdentificationHeader(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("VorbisVersion", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_8Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AudioChannels", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("AudioSampleRate", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_SignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BitrateMaximum", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_SignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BitrateNominal", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_SignedInteger_32Bit_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BitrateMinimum", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BlockSize0", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_4Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("BlockSize1", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_Boolean_1Bit(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("FramingFlag", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // verification
    if(Continue == true)
    {
        Continue = std::any_cast<bool>(Result->GetValue()->GetField("FramingFlag")->GetData());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_RIFF_Chunk(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkHeader"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("Header", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto PartStart = Reader.GetConsumedLength();
        auto ClaimedSize = Inspection::Length{std::any_cast<std::uint32_t>(Result->GetValue()->GetField("Header")->GetField("Size")->GetData()), 0};
        
        if(Reader.Has(ClaimedSize) == true)
        {
            const auto & ChunkIdentifier = std::any_cast<const std::string &>(Result->GetValue()->GetField("Header")->GetField("Identifier")->GetData());
            
            if(ChunkIdentifier == "RIFF")
            {
                auto PartReader = Inspection::Reader{Reader, ClaimedSize};
                auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "RIFF"})->Get(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else if(ChunkIdentifier == "inst")
            {
                auto PartReader = Inspection::Reader{Reader, ClaimedSize};
                auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "inst"})->Get(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else if(ChunkIdentifier == "fact")
            {
                auto PartReader = Inspection::Reader{Reader, ClaimedSize};
                auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "fact"})->Get(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else if(ChunkIdentifier == "fmt ")
            {
                auto PartReader = Inspection::Reader{Reader, ClaimedSize};
                auto PartResult = Inspection::Get_RIFF_ChunkData_fmt_(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            else
            {
                auto PartReader = Inspection::Reader{Reader, ClaimedSize};
                auto PartResult = Inspection::Get_Data_SetOrUnset_EndedByLength(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Data", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
            
            auto HandledSize = Reader.GetConsumedLength() - PartStart;
            
            if(HandledSize < ClaimedSize)
            {
                auto OutOfAlignmentBits = Reader.GetReadPositionInInput().GetTotalBits() % 16;
                
                if(HandledSize + Inspection::Length{0, OutOfAlignmentBits} == ClaimedSize)
                {
                    Result->GetValue()->AddTag("error", "The chunk data size is claimed larger than the actually handled size, because the padding was erroneously included in the chunk data size."s);
                }
                else
                {
                    Result->GetValue()->AddTag("error", "The chunk data size is claimed larger than the actually handled size."s);
                }
                Result->GetValue()->AddTag("claimed size", ClaimedSize);
                Result->GetValue()->AddTag("handled size", HandledSize);
            }
        }
        else
        {
            Result->GetValue()->AddTag("error", "The RIFF chunk claims to have a length of " + to_string_cast(ClaimedSize) + " bytes and bits but only " + to_string_cast(Reader.CalculateRemainingOutputLength()) + " bytes and bits are available.");
            Continue = false;
        }
    }
    // reading
    if(Continue == true)
    {
        if(Reader.HasRemaining() == true)
        {
            auto OutOfAlignmentBits = Reader.GetReadPositionInInput().GetTotalBits() % 16;
            
            if(OutOfAlignmentBits > 0)
            {
                auto PartReader = Inspection::Reader{Reader};
                auto PartResult = Inspection::Get_Data_Unset_Until16BitAlignment(PartReader, {});
                
                Continue = PartResult->GetSuccess();
                Result->GetValue()->AppendField("Padding", PartResult->ExtractValue());
                Reader.AdvancePosition(PartReader.GetConsumedLength());
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_RIFF_ChunkData_fmt_(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "fmt__CommonFields"})->Get(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        const auto & FormatTag = std::any_cast<const std::string &>(Result->GetValue()->GetField("FormatTag")->GetTag("constant name")->GetData());
        
        if(FormatTag == "WAVE_FORMAT_PCM")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "fmt__FormatSpecificFields_PCM"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else if(FormatTag == "WAVE_FORMAT_EXTENSIBLE")
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::g_TypeRepository.GetType({"RIFF", "ChunkData", "fmt__FormatSpecificFields_Extensible"})->Get(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
        }
        else
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength(PartReader, {});
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->AppendField("Rest", PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            Result->GetValue()->AddTag("error", "Unknown format tag " + FormatTag + ".");
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const auto & ChannelMask = std::any_cast<const std::bitset<32> &>(Result->GetValue()->GetData());
        
        if(ChannelMask[0] == true)
        {
            Result->GetValue()->AppendField("[0]", "SPEAKER_FRONT_LEFT"s);
        }
        if(ChannelMask[1] == true)
        {
            Result->GetValue()->AppendField("[1]", "SPEAKER_FRONT_RIGHT"s);
        }
        if(ChannelMask[2] == true)
        {
            Result->GetValue()->AppendField("[2]", "SPEAKER_FRONT_CENTER"s);
        }
        if(ChannelMask[3] == true)
        {
            Result->GetValue()->AppendField("[3]", "SPEAKER_LOW_FREQUENCY"s);
        }
        if(ChannelMask[4] == true)
        {
            Result->GetValue()->AppendField("[4]", "SPEAKER_BACK_LEFT"s);
        }
        if(ChannelMask[5] == true)
        {
            Result->GetValue()->AppendField("[5]", "SPEAKER_BACK_RIGHT"s);
        }
        if(ChannelMask[6] == true)
        {
            Result->GetValue()->AppendField("[6]", "SPEAKER_FRONT_LEFT_OF_CENTER"s);
        }
        if(ChannelMask[7] == true)
        {
            Result->GetValue()->AppendField("[7]", "SPEAKER_FRONT_RIGHT_OF_CENTER"s);
        }
        if(ChannelMask[8] == true)
        {
            Result->GetValue()->AppendField("[8]", "SPEAKER_BACK_CENTER"s);
        }
        if(ChannelMask[9] == true)
        {
            Result->GetValue()->AppendField("[9]", "SPEAKER_SIDE_LEFT"s);
        }
        if(ChannelMask[10] == true)
        {
            Result->GetValue()->AppendField("[10]", "SPEAKER_SIDE_RIGHT"s);
        }
        if(ChannelMask[11] == true)
        {
            Result->GetValue()->AppendField("[11]", "SPEAKER_TOP_CENTER"s);
        }
        if(ChannelMask[12] == true)
        {
            Result->GetValue()->AppendField("[12]", "SPEAKER_TOP_FRONT_LEFT"s);
        }
        if(ChannelMask[13] == true)
        {
            Result->GetValue()->AppendField("[13]", "SPEAKER_TOP_FRONT_CENTER"s);
        }
        if(ChannelMask[14] == true)
        {
            Result->GetValue()->AppendField("[14]", "SPEAKER_TOP_FRONT_RIGHT"s);
        }
        if(ChannelMask[15] == true)
        {
            Result->GetValue()->AppendField("[15]", "SPEAKER_TOP_BACK_LEFT"s);
        }
        if(ChannelMask[16] == true)
        {
            Result->GetValue()->AppendField("[16]", "SPEAKER_TOP_BACK_CENTER"s);
        }
        if(ChannelMask[17] == true)
        {
            Result->GetValue()->AppendField("[17]", "SPEAKER_TOP_BACK_RIGHT"s);
        }
        for(auto BitIndex = 18; BitIndex < 31; ++BitIndex)
        {
            Continue &= !ChannelMask[BitIndex];
        }
        if(ChannelMask[31] == true)
        {
            Result->GetValue()->AppendField("[31]", "SPEAKER_ALL"s);
        }
        /// @todo there are the following bit combinations which have special names
        //~ #define SPEAKER_MONO             (SPEAKER_FRONT_CENTER)
        //~ #define SPEAKER_STEREO           (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)
        //~ #define SPEAKER_2POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY)
        //~ #define SPEAKER_SURROUND         (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_BACK_CENTER)
        //~ #define SPEAKER_QUAD             (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
        //~ #define SPEAKER_4POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
        //~ #define SPEAKER_5POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)
        //~ #define SPEAKER_7POINT1          (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_FRONT_LEFT_OF_CENTER | SPEAKER_FRONT_RIGHT_OF_CENTER)
        //~ #define SPEAKER_5POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)
        //~ #define SPEAKER_7POINT1_SURROUND (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT  | SPEAKER_SIDE_RIGHT)
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_GUID_LittleEndian(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->Extend(PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        const auto & GUID = std::any_cast<const Inspection::GUID &>(Result->GetValue()->GetData());
        
        if(GUID == g_KSDATAFORMAT_SUBTYPE_PCM)
        {
            Result->GetValue()->AddTag("interpretation", "KSDATAFORMAT_SUBTYPE_PCM"s);
        }
        else
        {
            Result->GetValue()->AddTag("interpretation", nullptr);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    auto Bits{std::any_cast<std::uint8_t>(Parameters.at("Bits"))};
    
    // reading
    switch(Bits)
    {
    case 1:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_1Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 5:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_5Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 7:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_7Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 8:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_8Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 9:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_9Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 10:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_10Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 11:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_11Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 12:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_12Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 13:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_13Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 14:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_14Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 15:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_15Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 16:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_16Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 17:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_17Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 18:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_18Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 19:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_19Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 20:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_20Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 21:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_21Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 22:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_22Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 23:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_23Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 24:
       {
               auto PartReader = Inspection::Reader{Reader};
               auto PartResult{Inspection::Get_SignedInteger_24Bit_BigEndian(PartReader, {})};
               
               Continue = PartResult->GetSuccess();
               Result->GetValue()->Extend(PartResult->ExtractValue());
               Reader.AdvancePosition(PartReader.GetConsumedLength());
               
               break;
       }
    case 25:
       {
               auto PartReader = Inspection::Reader{Reader};
               auto PartResult{Inspection::Get_SignedInteger_25Bit_BigEndian(PartReader, {})};
               
               Continue = PartResult->GetSuccess();
               Result->GetValue()->Extend(PartResult->ExtractValue());
               Reader.AdvancePosition(PartReader.GetConsumedLength());
               
               break;
       }
    case 32:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_SignedInteger_32Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    default:
        {
            NOT_IMPLEMENTED("Reading " + to_string_cast(Bits) + " bits as a signed integer is not yet implemented in the generic function.");
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_1Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("1bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read1Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(static_cast<std::int8_t>(static_cast<std::int8_t>(ReadResult.Data << 7) >> 7));
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_5Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("5bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if(Reader.Read5Bits(ReadResult) == true)
        {
            Result->GetValue()->SetData(static_cast<std::int8_t>(static_cast<std::int8_t>(ReadResult.Data << 3) >> 3));
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_7Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("7bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if(Reader.Read7Bits(ReadResult) == true)
        {
            Result->GetValue()->SetData(static_cast<std::int8_t>(static_cast<std::int8_t>(ReadResult.Data << 1) >> 1));
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_8Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("8bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if(Reader.Read8Bits(ReadResult) == true)
        {
            Result->GetValue()->SetData(static_cast<std::int8_t>(ReadResult.Data));
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
            Continue = false;
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_9Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("9bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read1Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 15) >> 7) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_10Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("10bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read2Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 14) >> 6) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_11Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("11bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read3Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 13) >> 5) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_12Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("12bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read4Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 12) >> 4) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_13Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("13bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read5Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 11) >> 3) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_14Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("14bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read6Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
        
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 10) >> 2) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_15Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("15bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read7Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
        
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data << 9) >> 1) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_16Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("16bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::int16_t>(static_cast<std::int16_t>(static_cast<std::int16_t>(ReadResult1.Data) << 8) | static_cast<std::int16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_17Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("17bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read1Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 31) >> 15) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_18Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("18bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read2Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 30) >> 14) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_19Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("19bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read3Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 29) >> 13) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_20Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("20bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read4Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 28) >> 12) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_21Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("21bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read5Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 27) >> 11) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_22Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("22bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read6Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 26) >> 10) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_23Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("23bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read7Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 25) >> 9) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_24Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("24bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 24) >> 8) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(ReadResult3.Data)));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_25Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("25bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read1Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    auto ReadResult4 = Inspection::ReadResult{};
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data << 31) >> 7) | static_cast<std::int32_t>(ReadResult2.Data << 16) | static_cast<std::int32_t>(ReadResult3.Data << 8) | static_cast<std::int32_t>(ReadResult4.Data));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data) << 24) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 16) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult3.Data) << 8) | static_cast<std::int32_t>(ReadResult4.Data)));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("signed"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult1.Data) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult2.Data) << 8) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult3.Data) << 16) | static_cast<std::int32_t>(static_cast<std::int32_t>(ReadResult4.Data) << 24)));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_SignedInteger_32Bit_RiceEncoded(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_32Bit_AlternativeUnary(PartReader, {});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("MostSignificantBits", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // reading
    if(Continue == true)
    {
        auto Rice = std::any_cast<std::uint8_t>(Parameters.at("Rice"));
        auto PartReader = Inspection::Reader{Reader};
        auto PartResult = Inspection::Get_UnsignedInteger_BigEndian(PartReader, {{"Bits", Rice}});
        
        Continue = PartResult->GetSuccess();
        Result->GetValue()->AppendField("LeastSignificantBits", PartResult->ExtractValue());
        Reader.AdvancePosition(PartReader.GetConsumedLength());
    }
    // interpretation
    if(Continue == true)
    {
        auto MostSignificantBits = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("MostSignificantBits")->GetData());
        auto LeastSignificantBits = static_cast<std::uint32_t>(0);
        
        if(Result->GetValue()->GetField("LeastSignificantBits")->GetData().type() == typeid(std::uint8_t))
        {
            LeastSignificantBits = std::any_cast<std::uint8_t>(Result->GetValue()->GetField("LeastSignificantBits")->GetData());
        }
        else if(Result->GetValue()->GetField("LeastSignificantBits")->GetData().type() == typeid(std::uint16_t))
        {
            LeastSignificantBits = std::any_cast<std::uint16_t>(Result->GetValue()->GetField("LeastSignificantBits")->GetData());
        }
        else if(Result->GetValue()->GetField("LeastSignificantBits")->GetData().type() == typeid(std::uint32_t))
        {
            LeastSignificantBits = std::any_cast<std::uint32_t>(Result->GetValue()->GetField("LeastSignificantBits")->GetData());
        }
        else
        {
            UNEXPECTED_CASE("type of LeastSignificantBits is " + Inspection::to_string(Result->GetValue()->GetField("LeastSignificantBits")->GetData().type()));
        }
        
        auto Rice = std::any_cast<std::uint8_t>(Parameters.at("Rice"));
        auto Value = MostSignificantBits << Rice | LeastSignificantBits;
        
        if((Value & 0x00000001) == 0x00000001)
        {
            Result->GetValue()->SetData(static_cast<std::int32_t>(-(Value >> 1)- 1));
        }
        else
        {
            Result->GetValue()->SetData(static_cast<std::int32_t>(Value >> 1));
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_String_ASCII_ByTemplate(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto & Template{std::any_cast<const std::string &>(Parameters.at("Template"))};
    auto Continue = true;
    
    Result->GetValue()->AddTag("string"s);
    Result->GetValue()->AddTag("character set", "ASCII"s);
    Result->GetValue()->AddTag("encoding", "ASCII"s);
    Result->GetValue()->AddTag("template", Template);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        std::stringstream Value;
        auto NumberOfCharacters{0ul};
        
        for(auto TemplateCharacter : Template)
        {
            if((Continue = Reader.Read8Bits(ReadResult)) == true)
            {
                if(TemplateCharacter == ReadResult.Data)
                {
                    NumberOfCharacters += 1;
                    Value << ReadResult.Data;
                }
                else
                {
                    Result->GetValue()->AddTag("ended by error"s);
                    Result->GetValue()->AddTag("error", "The " + to_string_cast(NumberOfCharacters + 1) + "th character does not match the template.");
                    Result->GetValue()->AddTag("expected character", TemplateCharacter);
                    Result->GetValue()->AddTag("found character", static_cast<char>(ReadResult.Data));
                    Continue = false;
                    
                    break;
                }
            }
            else
            {
                Result->GetValue()->AddTag("ended by error"s);
                AppendReadErrorTag(Result->GetValue(), ReadResult);
                
                break;
            }
        }
        if(Continue == true)
        {
            Result->GetValue()->AddTag("ended by template"s);
        }
        Result->GetValue()->AddTag(to_string_cast(NumberOfCharacters) + " characters"s);
        Result->GetValue()->SetData(Value.str());
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    auto Bits{std::any_cast<std::uint8_t>(Parameters.at("Bits"))};
    
    // reading
    switch(Bits)
    {
    case 0:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_0Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 1:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_1Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 2:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_2Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 3:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_3Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 4:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_4Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 5:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_5Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 6:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_6Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 7:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_7Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 8:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_8Bit(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 9:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_9Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 10:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_10Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 11:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_11Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 12:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_12Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 13:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_13Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 14:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_14Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 15:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_15Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 16:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_16Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 17:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_17Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 18:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_18Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 19:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_19Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 20:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_20Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 21:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_21Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 22:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_22Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 23:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_23Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    case 24:
        {
            auto PartReader = Inspection::Reader{Reader};
            auto PartResult{Inspection::Get_UnsignedInteger_24Bit_BigEndian(PartReader, {})};
            
            Continue = PartResult->GetSuccess();
            Result->GetValue()->Extend(PartResult->ExtractValue());
            Reader.AdvancePosition(PartReader.GetConsumedLength());
            
            break;
        }
    default:
        {
            NOT_IMPLEMENTED("Reading " + to_string_cast(Bits) + " bits as an unsigned integer is not yet implemented in the generic function.");
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_0Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("0bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read0Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_1Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("1bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read1Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_2Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("2bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read2Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_3Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("3bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read3Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_4Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("4bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read4Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_5Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("5bit"s);
    // verification
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read5Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_6Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("6bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read6Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_7Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("7bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read7Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_8Bit(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("8bit"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            Result->GetValue()->SetData(ReadResult.Data);
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_8Bit_AlternativeUnary_BoundedByLength(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    std::uint8_t Value{0};
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("alternative unary"s);
    
    Inspection::ReadResult ReadResult;
    
    while(true)
    {
        if(Reader.Read1Bits(ReadResult) == true)
        {
            if(ReadResult.Data == 0x00)
            {
                Value += 1;
            }
            else
            {
                Result->SetSuccess(true);
                Result->GetValue()->AddTag(to_string_cast(Value + 1) + "bit"s);
                
                break;
            }
        }
        else
        {
            Result->SetSuccess(true);
            Result->GetValue()->AddTag(to_string_cast(Value) + "bit"s);
            Result->GetValue()->AddTag("ended by boundary"s);
            
            break;
        }
    }
    if(Result->GetSuccess() == true)
    {
        Result->GetValue()->SetData(Value);
    }
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_9Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("9bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read1Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((static_cast<std::uint16_t>(static_cast<std::uint16_t>(ReadResult1.Data) << 8)) | static_cast<std::uint16_t>(ReadResult2.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_10Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("10bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read2Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_11Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("11bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read3Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_12Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("12bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read4Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_13Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("13bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read5Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_14Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("14bit"s);
    Result->GetValue()->AddTag("big endian"s);
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read6Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_15Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("15bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read7Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_16Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("16bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((ReadResult1.Data << 8) | ReadResult2.Data));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("16bit"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Result->GetValue()->SetData(static_cast<std::uint16_t>((static_cast<std::uint16_t>(static_cast<std::uint16_t>(ReadResult2.Data) << 8)) | static_cast<std::uint16_t>(ReadResult1.Data)));
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_17Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("17bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read1Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 16) | (static_cast<std::uint32_t>(ReadResult2.Data) << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_18Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("18bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read2Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 16) | (static_cast<std::uint32_t>(ReadResult2.Data) << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_19Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("19bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read3Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 16) | (static_cast<std::uint32_t>(ReadResult2.Data) << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_20Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("20bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read4Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data << 16) | (ReadResult2.Data << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_21Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("21bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read5Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data << 16) | (ReadResult2.Data << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_22Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("22bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read6Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data << 16) | (ReadResult2.Data << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_23Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("23bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        auto ReadResult1 = Inspection::ReadResult{};
        
        if((Continue = Reader.Read7Bits(ReadResult1)) == true)
        {
            auto ReadResult2 = Inspection::ReadResult{};
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                auto ReadResult3 = Inspection::ReadResult{};
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data << 16) | (ReadResult2.Data << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_24Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("24bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data << 16) | (ReadResult2.Data << 8) | ReadResult3.Data));
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_31Bit_UTF_8_Coded(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer");
    Result->GetValue()->AddTag("unsigned");
    Result->GetValue()->AddTag("31bit");
    Result->GetValue()->AddTag("UTF-8 coded");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            if((ReadResult1.Data & 0x80) == 0x00)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{1, 0}, "indicated data length");
                Result->GetValue()->SetData(static_cast<std::uint32_t>(ReadResult1.Data));
            }
            else if((ReadResult1.Data & 0xe0) == 0xc0)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{2, 0}, "indicated data length");
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    if((Continue = ((ReadResult2.Data & 0xc0) == 0x80)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x1f) << 6) | static_cast<std::uint32_t>(ReadResult2.Data & 0x3f));
                    }
                    else
                    {
                        Result->GetValue()->AddTag("error", "The first byte indicated a length of two bytes, but the following byte did not have the correct format."s);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xf0) == 0xe0)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{3, 0}, "indicated data length");
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        if((Continue = (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80))) == true)
                        {
                            Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x0f) << 12)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult3.Data & 0x3f));
                        }
                        else
                        {
                            Result->GetValue()->AddTag("error", "The first byte indicated a length of three bytes, but the following bytes did not have the correct format."s);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xf8) == 0xf0)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{4, 0}, "indicated data length");
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80))) == true)
                            {
                                Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x07) << 18)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 12) | static_cast<std::uint32_t>((ReadResult3.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult4.Data & 0x3f));
                            }
                            else
                            {
                                Result->GetValue()->AddTag("error", "The first byte indicated a length of four bytes, but the following bytes did not have the correct format."s);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xfc) == 0xf8)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{5, 0}, "indicated data length");
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            Inspection::ReadResult ReadResult5;
                            
                            if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                            {
                                if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80) && ((ReadResult5.Data & 0xc0) == 0x80))) == true)
                                {
                                    Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x03) << 24)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 18) | static_cast<std::uint32_t>((ReadResult3.Data & 0x3f) << 12) | static_cast<std::uint32_t>((ReadResult4.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult5.Data & 0x3f));
                                }
                                else
                                {
                                    Result->GetValue()->AddTag("error", "The first byte indicated a length of five bytes, but the following bytes did not have the correct format."s);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult5);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xfe) == 0xfc)
            {
                AppendLengthTag(Result->GetValue(), Inspection::Length{6, 0}, "indicated data length");
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            Inspection::ReadResult ReadResult5;
                            
                            if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                            {
                                Inspection::ReadResult ReadResult6;
                                
                                if((Continue = Reader.Read8Bits(ReadResult6)) == true)
                                {
                                    if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80) && ((ReadResult5.Data & 0xc0) == 0x80) && ((ReadResult6.Data & 0xc0) == 0x80))) == true)
                                    {
                                        Result->GetValue()->SetData(static_cast<std::uint32_t>((ReadResult1.Data & 0x01) << 30)| static_cast<std::uint32_t>((ReadResult2.Data & 0x3f) << 24) | static_cast<std::uint32_t>((ReadResult3.Data & 0x3f) << 18) | static_cast<std::uint32_t>((ReadResult4.Data & 0x3f) << 12) | static_cast<std::uint32_t>((ReadResult5.Data & 0x3f) << 6) | static_cast<std::uint32_t>(ReadResult6.Data & 0x3f));
                                    }
                                    else
                                    {
                                        Result->GetValue()->AddTag("error", "The first byte indicated a length of six bytes, but the following bytes did not have the correct format."s);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult6);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult5);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else
            {
                Result->GetValue()->AddTag("error", "The first byte has an invalid format to indicate the length of the data.");
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_32Bit_AlternativeUnary(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("alternative unary"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult;
        std::uint32_t Value{0};
        
        while(Continue == true)
        {
            if((Continue = Reader.Read1Bits(ReadResult)) == true)
            {
                if(ReadResult.Data == 0x00)
                {
                    Value += 1;
                }
                else
                {
                    Result->GetValue()->SetData(Value);
                    AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
                    
                    break;
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult);
            }
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_32Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult1.Data) << 24) | (static_cast<std::uint32_t>(ReadResult2.Data) << 16) | (static_cast<std::uint32_t>(ReadResult3.Data) << 8) | (static_cast<std::uint32_t>(ReadResult4.Data))));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("32bit"s);
    Result->GetValue()->AddTag("little endian"s);
    // verification
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::uint32_t>((static_cast<std::uint32_t>(ReadResult4.Data) << 24) | (static_cast<std::uint32_t>(ReadResult3.Data) << 16) | (static_cast<std::uint32_t>(ReadResult2.Data) << 8) | (static_cast<std::uint32_t>(ReadResult1.Data))));
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_36Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("36bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read4Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue = Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Inspection::ReadResult ReadResult5;
                        
                        if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                        {
                            Result->GetValue()->SetData(static_cast<std::uint64_t>((static_cast<std::uint64_t>(ReadResult1.Data) << 32) | (static_cast<std::uint64_t>(ReadResult2.Data) << 24) | (static_cast<std::uint64_t>(ReadResult3.Data) << 16) | (static_cast<std::uint64_t>(ReadResult4.Data) << 8) | static_cast<std::uint64_t>(ReadResult5.Data)));
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult5);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_36Bit_UTF_8_Coded(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer");
    Result->GetValue()->AddTag("unsigned");
    Result->GetValue()->AddTag("36bit");
    Result->GetValue()->AddTag("UTF-8 coded");
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue = Reader.Read8Bits(ReadResult1)) == true)
        {
            if((ReadResult1.Data & 0x80) == 0x00)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{1, 0});
                Result->GetValue()->SetData(static_cast<std::uint64_t>(ReadResult1.Data));
            }
            else if((ReadResult1.Data & 0xe0) == 0xc0)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{2, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    if((Continue = ((ReadResult2.Data & 0xc0) == 0x80)) == true)
                    {
                        Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult1.Data & 0x1f) << 6) | static_cast<std::uint64_t>(ReadResult2.Data & 0x3f));
                    }
                    else
                    {
                        Result->GetValue()->AddTag("error", "The first byte indicated a length of two bytes, but the following byte did not have the correct format."s);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xf0) == 0xe0)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{3, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        if((Continue = (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80))) == true)
                        {
                            Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult1.Data & 0x0f) << 12)| static_cast<std::uint64_t>((ReadResult2.Data & 0x3f) << 6) | static_cast<std::uint64_t>(ReadResult3.Data & 0x3f));
                        }
                        else
                        {
                            Result->GetValue()->AddTag("error", "The first byte indicated a length of three bytes, but the following bytes did not have the correct format."s);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xf8) == 0xf0)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{4, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80))) == true)
                            {
                                Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult1.Data & 0x07) << 18)| static_cast<std::uint64_t>((ReadResult2.Data & 0x3f) << 12) | static_cast<std::uint64_t>((ReadResult3.Data & 0x3f) << 6) | static_cast<std::uint64_t>(ReadResult4.Data & 0x3f));
                            }
                            else
                            {
                                Result->GetValue()->AddTag("error", "The first byte indicated a length of four bytes, but the following bytes did not have the correct format."s);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xfc) == 0xf8)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{5, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            Inspection::ReadResult ReadResult5;
                            
                            if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                            {
                                if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80) && ((ReadResult5.Data & 0xc0) == 0x80))) == true)
                                {
                                    Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult1.Data & 0x03) << 24)| static_cast<std::uint64_t>((ReadResult2.Data & 0x3f) << 18) | static_cast<std::uint64_t>((ReadResult3.Data & 0x3f) << 12) | static_cast<std::uint64_t>((ReadResult4.Data & 0x3f) << 6) | static_cast<std::uint64_t>(ReadResult5.Data & 0x3f));
                                }
                                else
                                {
                                    Result->GetValue()->AddTag("error", "The first byte indicated a length of five bytes, but the following bytes did not have the correct format."s);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult5);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xfe) == 0xfc)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{6, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            Inspection::ReadResult ReadResult5;
                            
                            if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                            {
                                Inspection::ReadResult ReadResult6;
                                
                                if((Continue = Reader.Read8Bits(ReadResult6)) == true)
                                {
                                    if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80) && ((ReadResult5.Data & 0xc0) == 0x80) && ((ReadResult6.Data & 0xc0) == 0x80))) == true)
                                    {
                                        Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult1.Data & 0x01) << 30)| static_cast<std::uint64_t>((ReadResult2.Data & 0x3f) << 24) | static_cast<std::uint64_t>((ReadResult3.Data & 0x3f) << 18) | static_cast<std::uint64_t>((ReadResult4.Data & 0x3f) << 12) | static_cast<std::uint64_t>((ReadResult5.Data & 0x3f) << 6) | static_cast<std::uint64_t>(ReadResult6.Data & 0x3f));
                                    }
                                    else
                                    {
                                        Result->GetValue()->AddTag("error", "The first byte indicated a length of six bytes, but the following bytes did not have the correct format."s);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult6);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult5);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else if((ReadResult1.Data & 0xff) == 0xfe)
            {
                Result->GetValue()->AddTag("indicated length", Inspection::Length{7, 0});
                
                Inspection::ReadResult ReadResult2;
                
                if((Continue = Reader.Read8Bits(ReadResult2)) == true)
                {
                    Inspection::ReadResult ReadResult3;
                    
                    if((Continue = Reader.Read8Bits(ReadResult3)) == true)
                    {
                        Inspection::ReadResult ReadResult4;
                        
                        if((Continue = Reader.Read8Bits(ReadResult4)) == true)
                        {
                            Inspection::ReadResult ReadResult5;
                            
                            if((Continue = Reader.Read8Bits(ReadResult5)) == true)
                            {
                                Inspection::ReadResult ReadResult6;
                                
                                if((Continue = Reader.Read8Bits(ReadResult6)) == true)
                                {
                                    Inspection::ReadResult ReadResult7;
                                    
                                    if((Continue = Reader.Read8Bits(ReadResult7)) == true)
                                    {
                                        if((Continue == (((ReadResult2.Data & 0xc0) == 0x80) && ((ReadResult3.Data & 0xc0) == 0x80) && ((ReadResult4.Data & 0xc0) == 0x80) && ((ReadResult5.Data & 0xc0) == 0x80) && ((ReadResult6.Data & 0xc0) == 0x80) && ((ReadResult7.Data & 0xc0) == 0x80))) == true)
                                        {
                                            Result->GetValue()->SetData(static_cast<std::uint64_t>((ReadResult2.Data & 0x3f) << 30) | static_cast<std::uint64_t>((ReadResult3.Data & 0x3f) << 24) | static_cast<std::uint64_t>((ReadResult4.Data & 0x3f) << 18) | static_cast<std::uint64_t>((ReadResult5.Data & 0x3f) << 12) | static_cast<std::uint64_t>((ReadResult6.Data & 0x3f) << 6) | static_cast<std::uint64_t>(ReadResult7.Data & 0x3f));
                                        }
                                        else
                                        {
                                            Result->GetValue()->AddTag("error", "The first byte indicated a length of seven bytes, but the following bytes did not have the correct format."s);
                                        }
                                    }
                                    else
                                    {
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult6);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult5);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult4);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult3);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult2);
                }
            }
            else
            {
                Result->GetValue()->AddTag("error", "The first byte has an invalid format to indicate the length of the data.");
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_64Bit_BigEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("64bit"s);
    Result->GetValue()->AddTag("big endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue == Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue == Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue == Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue == Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Inspection::ReadResult ReadResult5;
                        
                        if((Continue == Reader.Read8Bits(ReadResult5)) == true)
                        {
                            Inspection::ReadResult ReadResult6;
                            
                            if((Continue == Reader.Read8Bits(ReadResult6)) == true)
                            {
                                Inspection::ReadResult ReadResult7;
                                
                                if((Continue == Reader.Read8Bits(ReadResult7)) == true)
                                {
                                    Inspection::ReadResult ReadResult8;
                                    
                                    if((Continue == Reader.Read8Bits(ReadResult8)) == true)
                                    {
                                        Result->GetValue()->SetData((static_cast<std::uint64_t>(ReadResult1.Data) << 56) | (static_cast<std::uint64_t>(ReadResult2.Data) << 48) | (static_cast<std::uint64_t>(ReadResult3.Data) << 40) | (static_cast<std::uint64_t>(ReadResult4.Data) << 32) | (static_cast<std::uint64_t>(ReadResult5.Data) << 24) | (static_cast<std::uint64_t>(ReadResult6.Data) << 16) | (static_cast<std::uint64_t>(ReadResult7.Data) << 8) | static_cast<std::uint64_t>(ReadResult8.Data));
                                    }
                                    else
                                    {
                                        AppendReadErrorTag(Result->GetValue(), ReadResult8);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult7);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult6);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult5);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

std::unique_ptr<Inspection::Result> Inspection::Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer"s);
    Result->GetValue()->AddTag("unsigned"s);
    Result->GetValue()->AddTag("64bit"s);
    Result->GetValue()->AddTag("little endian"s);
    // reading
    if(Continue == true)
    {
        Inspection::ReadResult ReadResult1;
        
        if((Continue == Reader.Read8Bits(ReadResult1)) == true)
        {
            Inspection::ReadResult ReadResult2;
            
            if((Continue == Reader.Read8Bits(ReadResult2)) == true)
            {
                Inspection::ReadResult ReadResult3;
                
                if((Continue == Reader.Read8Bits(ReadResult3)) == true)
                {
                    Inspection::ReadResult ReadResult4;
                    
                    if((Continue == Reader.Read8Bits(ReadResult4)) == true)
                    {
                        Inspection::ReadResult ReadResult5;
                        
                        if((Continue == Reader.Read8Bits(ReadResult5)) == true)
                        {
                            Inspection::ReadResult ReadResult6;
                            
                            if((Continue == Reader.Read8Bits(ReadResult6)) == true)
                            {
                                Inspection::ReadResult ReadResult7;
                                
                                if((Continue == Reader.Read8Bits(ReadResult7)) == true)
                                {
                                    Inspection::ReadResult ReadResult8;
                                    
                                    if((Continue == Reader.Read8Bits(ReadResult8)) == true)
                                    {
                                        Result->GetValue()->SetData(static_cast<std::uint64_t>(ReadResult1.Data) | (static_cast<std::uint64_t>(ReadResult2.Data) << 8) | (static_cast<std::uint64_t>(ReadResult3.Data) << 16) | (static_cast<std::uint64_t>(ReadResult4.Data) << 24) | (static_cast<std::uint64_t>(ReadResult5.Data) << 32) | (static_cast<std::uint64_t>(ReadResult6.Data) << 40) | (static_cast<std::uint64_t>(ReadResult7.Data) << 48) | (static_cast<std::uint64_t>(ReadResult8.Data) << 56));
                                    }
                                    else
                                    {
                                        AppendReadErrorTag(Result->GetValue(), ReadResult8);
                                    }
                                }
                                else
                                {
                                    AppendReadErrorTag(Result->GetValue(), ReadResult7);
                                }
                            }
                            else
                            {
                                AppendReadErrorTag(Result->GetValue(), ReadResult6);
                            }
                        }
                        else
                        {
                            AppendReadErrorTag(Result->GetValue(), ReadResult5);
                        }
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult4);
                    }
                }
                else
                {
                    AppendReadErrorTag(Result->GetValue(), ReadResult3);
                }
            }
            else
            {
                AppendReadErrorTag(Result->GetValue(), ReadResult2);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult1);
        }
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}
