#include <iostream>

#include <common/getters.h>

#include "helpers.h"

using namespace std::string_literals;

auto Inspection::Get_EBML_ElementID(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto ReadResult = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            auto OctetLengthMinusOne = std::countl_zero(ReadResult.Data);
            
            if(OctetLengthMinusOne < 8)
            {
                auto Value = std::vector<std::uint8_t>{};
                
                if(OctetLengthMinusOne < 7)
                {
                    Value.push_back(ReadResult.Data);
                }
                for(auto OctetIndex = OctetLengthMinusOne - 1; (Continue == true) && (OctetIndex >= 0); --OctetIndex)
                {
                    if((Continue = Reader.Read8Bits(ReadResult)) == true)
                    {
                        Value.push_back(ReadResult.Data);
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult);
                    }
                }
                Result->GetValue()->SetData(Value);
            }
            else
            {
                Result->GetValue()->AddTag("error", "Variable size integer starts with 8 zeroed bits, which is probably forbidden in EBML."s);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::Get_EBML_VariableSizeInteger(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    Result->GetValue()->AddTag("integer");
    Result->GetValue()->AddTag("unsigned");
    Result->GetValue()->AddTag("variable size");
    // reading
    if(Continue == true)
    {
        auto ReadResult = Inspection::ReadResult{};
        
        if((Continue = Reader.Read8Bits(ReadResult)) == true)
        {
            auto OctetLengthMinusOne = std::countl_zero(ReadResult.Data);
            
            if(OctetLengthMinusOne < 8)
            {
                auto Value = static_cast<std::uint64_t>(0);
                
                if(OctetLengthMinusOne < 7)
                {
                    Value += (static_cast<std::uint64_t>(ReadResult.Data) & (~(1 << (7 - OctetLengthMinusOne)))) << (8 * OctetLengthMinusOne);
                }
                for(auto OctetIndex = OctetLengthMinusOne - 1; (Continue == true) && (OctetIndex >= 0); --OctetIndex)
                {
                    if((Continue = Reader.Read8Bits(ReadResult)) == true)
                    {
                        Value += static_cast<std::uint64_t>(ReadResult.Data) << (8 * OctetIndex);
                    }
                    else
                    {
                        AppendReadErrorTag(Result->GetValue(), ReadResult);
                    }
                }
                Result->GetValue()->SetData(Value);
            }
            else
            {
                Result->GetValue()->AddTag("error", "Variable size integer starts with 8 zeroed bits, which is probably forbidden in EBML."s);
            }
        }
        else
        {
            AppendReadErrorTag(Result->GetValue(), ReadResult);
        }
    }
    AppendLengthTag(Result->GetValue(), Reader.GetConsumedLength());
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}
