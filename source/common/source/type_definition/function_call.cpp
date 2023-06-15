/**
 * Copyright (C) 2023  Hagen Möbius
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include <xml_puny_dom/xml_puny_dom.h>

#include <helper.h>
#include <value.h>

#include "../internal_output_operators.h"
#include "function_call.h"
#include "parameters.h"

using namespace std::string_literals;

auto Inspection::TypeDefinition::FunctionCall::GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any
{
    ASSERTION(m_FunctionName.empty() == false);
    if(m_FunctionName == "Get_DateTime_FromMicrosoftFileTime")
    {
        ASSERTION(m_CallParameters != nullptr);
        
        auto ActualParameters = m_CallParameters->GetParameters(ExecutionContext);
        auto const & MicrosoftFileTimeParameterIterator = ActualParameters.find("MicrosoftFileTime");
        
        INVALID_INPUT_IF(MicrosoftFileTimeParameterIterator == ActualParameters.end(), "Calling function \"Get_DateTime_FromMicrosoftFileTime\" requires a \"MicrosoftFileTime\" parameter of type \"unsigned integer 64bit\".");
        INVALID_INPUT_IF(MicrosoftFileTimeParameterIterator->second.type() != typeid(std::uint64_t), "The \"MicrosoftFileTime\" parameter needs to be of type \"unsigned integer 64bit\" not \""s + Inspection::to_string(MicrosoftFileTimeParameterIterator->second.type()) + "\".");
        
        return Inspection::Get_DateTime_FromMicrosoftFileTime(std::any_cast<std::uint64_t>(MicrosoftFileTimeParameterIterator->second));
    }
    else
    {
        UNEXPECTED_CASE("function name = \"" + m_FunctionName + '"');
    }
}

auto Inspection::TypeDefinition::FunctionCall::GetDataType() const -> Inspection::TypeDefinition::DataType
{
    NOT_IMPLEMENTED("Called GetDataType() on a FunctionCall expression.");
}

auto Inspection::TypeDefinition::FunctionCall::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::FunctionCall>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::FunctionCall>{new Inspection::TypeDefinition::FunctionCall{}};
    
    INVALID_INPUT_IF(Element->HasAttribute("function-name") == false, "A \"function-call\" requires a \"function-name\" attribute.");
    Result->m_FunctionName = Element->GetAttribute("function-name");
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "call-parameters")
        {
            Result->m_CallParameters = Inspection::TypeDefinition::Parameters::Load(ChildElement);
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
        INVALID_INPUT_IF(Result->m_CallParameters == nullptr, "A \"function-call\" requires a \"call-parameters\" child.");
    }
    
    return Result;
}
