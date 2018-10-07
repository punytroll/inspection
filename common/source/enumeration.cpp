#include <cassert>
#include <fstream>

#include "enumeration.h"
#include "xml_puny_dom.h"

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
	auto DocumentElement{Document.GetDocumentElement()};
	
	assert(DocumentElement != nullptr);
	assert(DocumentElement->GetName() == "enumeration");
	BaseType = DocumentElement->GetAttribute("base-type");
	for(auto EnumerationChildNode : DocumentElement->GetChilds())
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
							Element->TagName = EnumerationElementChildElement->GetAttribute("name");
							Element->TagType = EnumerationElementChildElement->GetAttribute("type");
							
							assert(EnumerationElementChildElement->GetChilds().size() == 1);
							
							auto TagText{dynamic_cast< const XML::Text * >(EnumerationElementChildElement->GetChild(0))};
							
							assert(TagText != nullptr);
							Element->TagValue = TagText->GetText();
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
