#include <common/read_result.h>
#include <common/value.h>

#include "helpers.h"

using namespace std::string_literals;

auto Inspection::AppendLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName) -> Inspection::Value *
{
    auto Result = Value->AddTag(LengthName, Length);
    
    Result->AddTag("unit", "bytes and bits"s);
    
    return Result;
}

auto Inspection::AppendReadErrorTag(Inspection::Value * Value, Inspection::ReadResult const & ReadResult) -> Inspection::Value *
{
    ASSERTION(ReadResult.Success == false);
    
    auto Result = Value->AddTag("error", "Could not read enough data."s);
    
    AppendLengthTag(Result, ReadResult.RequestedLength, "requested length");
    AppendLengthTag(Result, ReadResult.InputLength, "remaining length");
    
    return Result;
}
