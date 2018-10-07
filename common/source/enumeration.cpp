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
				assert(EnumerationChildElement->GetChilds().size() == 0);
				
				auto Element{new Inspection::Enumeration::Element{}};
				
				Element->BaseValue = EnumerationChildElement->GetAttribute("base-value");
				Element->Interpretation = EnumerationChildElement->GetAttribute("interpretation");
				Elements.push_back(Element);
			}
			else
			{
				throw std::domain_error{EnumerationChildElement->GetName()};
			}
		}
	}
}
