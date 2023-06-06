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

#include <assertion.h>
#include <type.h>

#include "add.h"
#include "and.h"
#include "cast.h"
#include "expression.h"
#include "function_call.h"
#include "../type_definition.h"
#include "../xml_helper.h"

auto Inspection::TypeDefinition::Expression::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Expression>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Expression>{};
    
    if(Element->GetName() == "add")
    {
        Result = Inspection::TypeDefinition::Add::Load(Element);
    }
    else if(Element->GetName() == "and")
    {
        Result = Inspection::TypeDefinition::And::Load(Element);
    }
    else if(Element->GetName() == "data-reference")
    {
        Result = Inspection::TypeDefinition::DataReference::Load(Element);
    }
    else if(Element->GetName() == "divide")
    {
        Result = Inspection::TypeDefinition::Divide::Load(Element);
    }
    else if(Element->GetName() == "equals")
    {
        Result = Inspection::TypeDefinition::Equals::Load(Element);
    }
    else if(Element->GetName() == "field-reference")
    {
        Result = Inspection::TypeDefinition::FieldReference::Load(Element);
    }
    else if(Element->GetName() == "length")
    {
        if(XML::HasOneChildElement(Element) == true)
        {
            // <length>-Element contains an expression (i.e. <subtract>)
            Result = Inspection::TypeDefinition::Expression::Load(XML::GetFirstChildElement(Element));
        }
        else
        {
            // <length>-Element IS a length with <bytes> and <bits>
            Result = Inspection::TypeDefinition::Length::Load(Element);
        }
    }
    else if(Element->GetName() == "less-than")
    {
        Result = Inspection::TypeDefinition::LessThan::Load(Element);
    }
    else if(Element->GetName() == "modulus")
    {
        Result = Inspection::TypeDefinition::Modulus::Load(Element);
    }
    else if(Element->GetName() == "multiply")
    {
        Result = Inspection::TypeDefinition::Multiply::Load(Element);
    }
    else if(Element->GetName() == "parameter-reference")
    {
        Result = Inspection::TypeDefinition::ParameterReference::Load(Element);
    }
    else if(Element->GetName() == "parameters")
    {
        Result = Inspection::TypeDefinition::Parameters::Load(Element);
    }
    else if(Element->GetName() == "subtract")
    {
        Result = Inspection::TypeDefinition::Subtract::Load(Element);
    }
    else if(Element->GetName() == "type")
    {
        Result = Inspection::TypeDefinition::TypeValue::Load(Element);
    }
    else if(Element->GetName() == "type-reference")
    {
        Result = Inspection::TypeDefinition::TypeReference::Load(Element);
    }
    else if((Element->GetName() == "unsigned-integer-8bit") && (XML::HasOneChildElement(Element) == true))
    {
        Result = Inspection::TypeDefinition::Cast::Load(Element);
    }
    else if((Element->GetName() == "unsigned-integer-64bit") && (XML::HasOneChildElement(Element) == true))
    {
        Result = Inspection::TypeDefinition::Cast::Load(Element);
    }
    else if((Element->GetName() == "single-precision-real") && (XML::HasOneChildElement(Element) == true))
    {
        Result = Inspection::TypeDefinition::Cast::Load(Element);
    }
    else if(Element->GetName() == "function-call")
    {
        Result = Inspection::TypeDefinition::FunctionCall::Load(Element);
    }
    else
    {
        Result = Inspection::TypeDefinition::Value::Load(Element);
    }
    
    return Result;
}

auto Inspection::TypeDefinition::Expression::LoadFromWithin(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Expression>
{
    ASSERTION(Element != nullptr);
    
    auto ChildElementsRange = Element->GetChildElements();
    auto ChildElements = std::vector<XML::Element const *>{ChildElementsRange.begin(), ChildElementsRange.end()};
    
    INVALID_INPUT_IF(ChildElements.size() == 0, "Missing expression.");
    INVALID_INPUT_IF(ChildElements.size() > 1, "Too many expressions.");
    
    return Inspection::TypeDefinition::Expression::Load(ChildElements.front());
}
