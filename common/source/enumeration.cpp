#include <cassert>
#include <fstream>

#include "enumeration.h"
#include "xml_puny_dom.h"

Inspection::Enumeration::Element::~Element(void)
{
	for(auto Tag : Tags)
	{
		delete Tag;
	}
}

Inspection::Enumeration::~Enumeration(void)
{
	for(auto Element : Elements)
	{
		delete Element;
	}
}

void Inspection::Enumeration::Load(const std::string & Path)
{
	std::ifstream InputFileStream{Path};
	XML::Document Document{InputFileStream};
	
	Load(Document.GetDocumentElement());
}

void Inspection::Enumeration::Load(const XML::Element * EnumerationElement)
{
	assert(EnumerationElement != nullptr);
	assert(EnumerationElement->GetName() == "enumeration");
	BaseType = EnumerationElement->GetAttribute("base-type");
	for(auto EnumerationChildNode : EnumerationElement->GetChilds())
	{
		if(EnumerationChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto EnumerationChildElement{dynamic_cast< const XML::Element * >(EnumerationChildNode)};
			
			if(EnumerationChildElement->GetName() == "element")
			{
				auto Element{new Inspection::Enumeration::Element{}};
				
				Element->BaseValue = EnumerationChildElement->GetAttribute("base-value");
				for(auto EnumerationElementChildNode : EnumerationChildElement->GetChilds())
				{
					if(EnumerationElementChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto EnumerationElementChildElement{dynamic_cast< const XML::Element * >(EnumerationElementChildNode)};
						
						if(EnumerationElementChildElement->GetName() == "tag")
						{
							auto Tag{new Inspection::Enumeration::Element::Tag{}};
							
							Tag->Name = EnumerationElementChildElement->GetAttribute("name");
							Tag->Type = EnumerationElementChildElement->GetAttribute("type");
							
							assert(EnumerationElementChildElement->GetChilds().size() == 1);
							
							auto TagText{dynamic_cast< const XML::Text * >(EnumerationElementChildElement->GetChild(0))};
							
							assert(TagText != nullptr);
							Tag->Value = TagText->GetText();
							Element->Tags.push_back(Tag);
						}
						else
						{
							throw std::domain_error{EnumerationElementChildElement->GetName()};
						}
					}
				}
				Elements.push_back(Element);
			}
			else
			{
				throw std::domain_error{EnumerationChildElement->GetName()};
			}
		}
	}
}
