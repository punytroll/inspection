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

#include <result.h>
#include <type.h>
#include <value.h>

#include "../execution_context.h"
#include "../type_definition.h"
#include "../xml_helper.h"
#include "array.h"
#include "helper.h"

using namespace std::string_literals;

Inspection::TypeDefinition::Array::Array() :
    Inspection::TypeDefinition::Part{Inspection::TypeDefinition::PartType::Array}
{
}

auto Inspection::TypeDefinition::Array::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    ExecutionContext.Push(*this, *Result, Reader, Parameters);
    Result->GetValue()->AddTag("array");
    switch(m_IterateType)
    {
    case Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength:
        {
            auto ElementParameters = GetElementParameters(ExecutionContext);
            
            ASSERTION(m_ElementType != nullptr);
            ASSERTION(m_ElementType->GetDataType() == Inspection::TypeDefinition::DataType::Type);
            
            auto ElementTypeAny = m_ElementType->GetAny(ExecutionContext);
            
            ASSERTION(ElementTypeAny.has_value() == true);
            ASSERTION(ElementTypeAny.type() == typeid(Inspection::TypeDefinition::Type const *));
            
            auto ElementType = std::any_cast<Inspection::TypeDefinition::Type const *>(ElementTypeAny);
            
            ASSERTION(ElementType != nullptr);
            
            auto ElementIndexInArray = static_cast<std::uint64_t>(0);
            
            while((Continue == true) && (Reader.HasRemaining() == true))
            {
                auto ElementReader = Inspection::Reader{Reader};
                
                ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
                
                auto ElementResult = ElementType->Get(ExecutionContext, ElementReader, ElementParameters);
                
                Continue = ElementResult->GetSuccess();
                if(Continue == true)
                {
                    ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                    if(m_ElementName.has_value() == true)
                    {
                        Result->GetValue()->AppendField(m_ElementName.value(), ElementResult->ExtractValue());
                    }
                    else
                    {
                        Result->GetValue()->AppendField(ElementResult->ExtractValue());
                    }
                    Reader.AdvancePosition(ElementReader.GetConsumedLength());
                }
                else
                {
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
                Continue = true;
            }
            else
            {
                Result->GetValue()->AddTag("error", "At least one element was expected."s);
                Continue = false;
            }
            Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
            
            break;
        }
    case Inspection::TypeDefinition::Array::IterateType::ForEachField:
        {
            auto ElementParameters = GetElementParameters(ExecutionContext);
            
            ASSERTION(m_IterateForEachField != nullptr);
            
            auto IterateField = ExecutionContext.GetFieldFromFieldReference(*m_IterateForEachField);
            
            ASSERTION(IterateField != nullptr);
            
            auto ElementProperties = std::vector<std::pair<Inspection::Length, Inspection::Length>>{};
            
            for(auto & Field : IterateField->GetFields())
            {
                ElementProperties.emplace_back(std::any_cast<const Inspection::Length &>(Field->GetTag("position")->GetData()), std::any_cast<const Inspection::Length &>(Field->GetTag("length")->GetData()));
            }
            std::sort(std::begin(ElementProperties), std::end(ElementProperties));
            ASSERTION(m_ElementType != nullptr);
            ASSERTION(m_ElementType->GetDataType() == Inspection::TypeDefinition::DataType::Type);
            
            auto ElementTypeAny = m_ElementType->GetAny(ExecutionContext);
            
            ASSERTION(ElementTypeAny.has_value() == true);
            ASSERTION(ElementTypeAny.type() == typeid(Inspection::TypeDefinition::Type const *));
            
            auto ElementType = std::any_cast<Inspection::TypeDefinition::Type const *>(ElementTypeAny);
            
            ASSERTION(ElementType != nullptr);
            
            auto NumberOfAppendedElements = static_cast<std::uint64_t>(0);
            
            for(auto ElementPropertiesIndex = static_cast<std::uint64_t>(0); (Continue == true) && (ElementPropertiesIndex < ElementProperties.size()); ++ElementPropertiesIndex)
            {
                auto & Properties = ElementProperties[ElementPropertiesIndex];
                
                ASSERTION(Reader.GetReadPositionInInput() == Properties.first);
                
                auto ElementReader = Inspection::Reader{Reader, Properties.second};
                auto ElementResult = ElementType->Get(ExecutionContext, ElementReader, ElementParameters);
                
                Continue = ElementResult->GetSuccess();
                if(Continue == true)
                {
                    if(ElementReader.HasRemaining() == true)
                    {
                        ElementResult->GetValue()->AddTag("error", "The element reader did not process as much data as requested by the iterator field!"s);
                        ElementResult->GetValue()->AddTag("requested data length", Properties.second);
                        ElementResult->GetValue()->AddTag("processed data length", ElementReader.GetConsumedLength());
                    }
                    if(m_ElementName.has_value() == true)
                    {
                        Result->GetValue()->AppendField(m_ElementName.value(), ElementResult->ExtractValue());
                    }
                    else
                    {
                        Result->GetValue()->AppendField(ElementResult->ExtractValue());
                    }
                    Reader.AdvancePosition(ElementReader.GetConsumedLength());
                    ++NumberOfAppendedElements;
                    if(ElementReader.HasRemaining() == true)
                    {
                        auto OtherDataLength = Properties.second - ElementReader.GetConsumedLength();
                        
                        AppendOtherData(Result->GetValue(), OtherDataLength);
                        Reader.AdvancePosition(OtherDataLength);
                    }
                }
            }
            Result->GetValue()->AddTag("number of elements", NumberOfAppendedElements);
            
            break;
        }
    case Inspection::TypeDefinition::Array::IterateType::NumberOfElements:
        {
            auto ElementParameters = GetElementParameters(ExecutionContext);
            
            ASSERTION(m_ElementType != nullptr);
            ASSERTION(m_ElementType->GetDataType() == Inspection::TypeDefinition::DataType::Type);
            
            auto ElementTypeAny = m_ElementType->GetAny(ExecutionContext);
            
            ASSERTION(ElementTypeAny.has_value() == true);
            ASSERTION(ElementTypeAny.type() == typeid(Inspection::TypeDefinition::Type const *));
            
            auto ElementType = std::any_cast<Inspection::TypeDefinition::Type const *>(ElementTypeAny);
            
            ASSERTION(ElementType != nullptr);
            
            auto NumberOfElementsAny = m_IterateNumberOfElements->GetAny(ExecutionContext);
            
            ASSERTION(NumberOfElementsAny.type() == typeid(std::uint64_t));
            
            auto NumberOfElements = std::any_cast<std::uint64_t>(NumberOfElementsAny);
            auto ElementIndexInArray = static_cast<std::uint64_t>(0);
            
            while(true)
            {
                if(ElementIndexInArray < NumberOfElements)
                {
                    auto ElementReader = Inspection::Reader{Reader};
                    
                    ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
                    
                    auto ElementResult = ElementType->Get(ExecutionContext, ElementReader, ElementParameters);
                    
                    Continue = ElementResult->GetSuccess();
                    ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                    if(m_ElementName.has_value() == true)
                    {
                        Result->GetValue()->AppendField(m_ElementName.value(), ElementResult->ExtractValue());
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
            Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
            
            break;
        }
    case Inspection::TypeDefinition::Array::IterateType::UntilFailureOrLength:
        {
            auto ElementParameters = GetElementParameters(ExecutionContext);
            
            ASSERTION(m_ElementType != nullptr);
            ASSERTION(m_ElementType->GetDataType() == Inspection::TypeDefinition::DataType::Type);
            
            auto ElementTypeAny = m_ElementType->GetAny(ExecutionContext);
            
            ASSERTION(ElementTypeAny.has_value() == true);
            ASSERTION(ElementTypeAny.type() == typeid(Inspection::TypeDefinition::Type const *));
            
            auto ElementType = std::any_cast<Inspection::TypeDefinition::Type const *>(ElementTypeAny);
            
            ASSERTION(ElementType != nullptr);
            
            auto ElementIndexInArray = static_cast<std::uint64_t>(0);
            
            while((Continue == true) && (Reader.HasRemaining() == true))
            {
                auto ElementReader = Inspection::Reader{Reader};
                
                ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
                
                auto ElementResult = ElementType->Get(ExecutionContext, ElementReader, ElementParameters);
                
                Continue = ElementResult->GetSuccess();
                if(Continue == true)
                {
                    ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                    if(m_ElementName.has_value() == true)
                    {
                        Result->GetValue()->AppendField(m_ElementName.value(), ElementResult->ExtractValue());
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
            Result->GetValue()->AddTag("number of elements", ElementIndexInArray);
            
            break;
        }
    case Inspection::TypeDefinition::Array::IterateType::UntilLength:
        {
            auto ElementParameters = GetElementParameters(ExecutionContext);
            
            ASSERTION(m_ElementType != nullptr);
            ASSERTION(m_ElementType->GetDataType() == Inspection::TypeDefinition::DataType::Type);
            
            auto ElementTypeAny = m_ElementType->GetAny(ExecutionContext);
            
            ASSERTION(ElementTypeAny.has_value() == true);
            ASSERTION(ElementTypeAny.type() == typeid(Inspection::TypeDefinition::Type const *));
            
            auto ElementType = std::any_cast<Inspection::TypeDefinition::Type const *>(ElementTypeAny);
            
            ASSERTION(ElementType != nullptr);
            
            auto ElementIndexInArray = static_cast<std::uint64_t>(0);
            
            while((Continue == true) && (Reader.HasRemaining() == true))
            {
                auto ElementReader = Inspection::Reader{Reader};
                
                ElementParameters["ElementIndexInArray"] = ElementIndexInArray;
                
                auto ElementResult = ElementType->Get(ExecutionContext, ElementReader, ElementParameters);
                
                Continue = ElementResult->GetSuccess();
                ElementResult->GetValue()->AddTag("element index in array", ElementIndexInArray++);
                if(m_ElementName.has_value() == true)
                {
                    Result->GetValue()->AppendField(m_ElementName.value(), ElementResult->ExtractValue());
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
            
            break;
        }
    }
    ExecutionContext.Pop();
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Array::Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Array>
{
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Array>{new Inspection::TypeDefinition::Array{}};
    
    ASSERTION(Element->HasAttribute("name") == true);
    Result->FieldName = Element->GetAttribute("name");
    
    return Result;
}

auto Inspection::TypeDefinition::Array::_LoadProperty(XML::Element const * Element) -> void
{
    if(Element->GetName() == "iterate")
    {
        ASSERTION(GetPartType() == Inspection::TypeDefinition::PartType::Array);
        ASSERTION(Element->HasAttribute("type") == true);
        if(Element->GetAttribute("type") == "at-least-one-until-failure-or-length")
        {
            m_IterateType = Inspection::TypeDefinition::Array::IterateType::AtLeastOneUntilFailureOrLength;
            ASSERTION(XML::HasChildNodes(Element) == false);
        }
        else if(Element->GetAttribute("type") == "for-each-field")
        {
            m_IterateType = Inspection::TypeDefinition::Array::IterateType::ForEachField;
            
            auto ChildElementsRange = Element->GetChildElements();
            auto ChildElements = std::vector<XML::Element const *>{ChildElementsRange.begin(), ChildElementsRange.end()};
            
            INVALID_INPUT_IF(ChildElements.size() == 0, "Missing field reference in for-each-field.");
            INVALID_INPUT_IF(ChildElements.size() > 1, "Too many field references in for-each-field.");
            m_IterateForEachField = Inspection::TypeDefinition::FieldReference::Load(ChildElements.front());
        }
        else if(Element->GetAttribute("type") == "number-of-elements")
        {
            m_IterateType = Inspection::TypeDefinition::Array::IterateType::NumberOfElements;
            m_IterateNumberOfElements = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
        }
        else if(Element->GetAttribute("type") == "until-failure-or-length")
        {
            m_IterateType = Inspection::TypeDefinition::Array::IterateType::UntilFailureOrLength;
            ASSERTION(XML::HasChildNodes(Element) == false);
        }
        else if(Element->GetAttribute("type") == "until-length")
        {
            m_IterateType = Inspection::TypeDefinition::Array::IterateType::UntilLength;
            ASSERTION(XML::HasChildNodes(Element) == false);
        }
        else
        {
            UNEXPECTED_CASE("Element->GetAttribute(\"type\") == " + Element->GetAttribute("type"));
        }
    }
    else if(Element->GetName() == "element-name")
    {
        ASSERTION(Element->GetChildNodes().size() == 1);
        
        auto ElementNameText = dynamic_cast<XML::Text const *>(Element->GetChildNode(0));
        
        ASSERTION(ElementNameText != nullptr);
        m_ElementName = ElementNameText->GetText();
    }
    else if(Element->GetName() == "element-type")
    {
        m_ElementType = Inspection::TypeDefinition::Expression::LoadFromWithin(Element);
    }
    else if(Element->GetName() == "element-parameters")
    {
        m_ElementParameters = Inspection::TypeDefinition::Parameters::Load(Element);
    }
    else
    {
        Inspection::TypeDefinition::Part::_LoadProperty(Element);
    }
}

auto Inspection::TypeDefinition::Array::GetElementParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>
{
    if(m_ElementParameters != nullptr)
    {
        return m_ElementParameters->GetParameters(ExecutionContext);
    }
    else
    {
        return std::unordered_map<std::string, std::any>{};
    }
}
