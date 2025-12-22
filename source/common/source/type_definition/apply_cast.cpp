#include <xml_puny_dom/xml_puny_dom.h>

#include <common/assertion.h>
#include <common/value.h>

#include "../internal_output_operators.h"
#include "apply_cast.h"
#include "data_type.h"
#include "helper.h"

auto Inspection::TypeDefinition::ApplyCast::Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Value) const -> bool
{
    switch(m_DataType)
    {
    case Inspection::TypeDefinition::DataType::UnsignedInteger64Bit:
        {
            Value->SetData(Inspection::TypeDefinition::CastToUnsignedInteger64Bit(Value->GetData()));
            
            break;
        }
    default:
        {
            INVALID_INPUT("m_DataType == " + Inspection::to_string(m_DataType));
        }
    }
    
    return true;
}

auto Inspection::TypeDefinition::ApplyCast::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::ApplyCast>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::ApplyCast>(new Inspection::TypeDefinition::ApplyCast{});
    
    INVALID_INPUT_IF(Element->HasAttribute("data-type") == false, "An \"apply-cast\" interpretation must have a \"data-type\" attribute.");
    Result->m_DataType = Inspection::TypeDefinition::GetDataTypeFromString(Element->GetAttribute("data-type"));
    
    return Result;
}
