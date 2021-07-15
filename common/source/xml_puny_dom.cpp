/**
 * galactic-fall
 * Copyright (C) 2006-2018  Hagen Möbius
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

#include <algorithm>
#include <cassert>
#include <iostream>
#include <stack>
#include <stdexcept>

#include <xml_parser/xml_parser.h>

#include "xml_puny_dom.h"

namespace XML
{
	class DOMReader : public XMLParser
	{
	public:
		DOMReader(std::istream & Stream, XML::Document * Document, XML::Element ** RootElement);
		virtual void ElementStart(const std::string & Name, const std::map< std::string, std::string > & Attributes) override;
		virtual void ElementEnd(const std::string & Name) override;
		virtual void Text(const std::string & Text) override;
	private:
		XML::Document * _Document;
		XML::Element ** _RootElement;
		std::stack< XML::Element * > _ElementStack;
	};
}

XML::DOMReader::DOMReader(std::istream & Stream, XML::Document * Document, XML::Element ** RootElement) :
	XMLParser{Stream},
	_Document{Document},
	_RootElement{RootElement}
{
}

void XML::DOMReader::ElementStart(const std::string & Name, const std::map< std::string, std::string > & Attributes)
{
	if(*_RootElement == nullptr)
	{
		// we've got the root element
		_ElementStack.push(*_RootElement = new XML::Element{_Document, Name, Attributes});
	}
	else
	{
		_ElementStack.push(new XML::Element{_ElementStack.top(), Name, Attributes});
	}
}

void XML::DOMReader::ElementEnd(const std::string & Name)
{
	if(_ElementStack.size() == 0)
	{
		std::cerr << "Got '/" << Name << "' but stack is empty." << std::endl;
		
		throw std::domain_error{Name};
	}
	if(Name != _ElementStack.top()->GetName())
	{
		std::cerr << "Got '/" << Name << "' but expected '/" << _ElementStack.top()->GetName() << "'." << std::endl;
		
		throw std::domain_error{Name};
	}
	_ElementStack.pop();
}

void XML::DOMReader::Text(const std::string & Text)
{
	if(_ElementStack.size() > 0)
	{
		new XML::Text{_ElementStack.top(), Text};
	}
	else
	{
		std::cerr << "Misformed XML: Got text \"" << Text << "\" outside of root element." << std::endl;
		
		throw std::domain_error("NoElement");
	}
}

XML::Node::Node(XML::NodeType NodeType, XML::Node * Parent) :
	_NodeType{NodeType},
	_Parent{Parent}
{
	if(_Parent != nullptr)
	{
		_Parent->_Childs.push_back(this);
	}
}

XML::Node::~Node(void)
{
	// delete it from the parent's child vector
	if(_Parent != nullptr)
	{
		auto Iterator{std::find(_Parent->_Childs.begin(), _Parent->_Childs.end(), this)};
		
		assert(Iterator != _Parent->_Childs.end());
		_Parent->_Childs.erase(Iterator);
		_Parent = nullptr;
	}
	while(_Childs.empty() == false)
	{
		// erasing the child from the m_Childs vector will happen in the childs destructor
		delete _Childs.front();
	}
}

const std::vector< XML::Node * > & XML::Node::GetChilds(void) const
{
	return _Childs;
}

const XML::Node * XML::Node::GetChild(std::vector< XML::Node * >::size_type Index) const
{
	return _Childs[Index];
}

XML::NodeType XML::Node::GetNodeType(void) const
{
	return _NodeType;
}

XML::Element::Element(XML::Node * Parent, const std::string & Name, const std::map< std::string, std::string > & Attributes) :
	XML::Node{XML::NodeType::Element, Parent},
	_Name{Name},
	_Attributes{Attributes}
{
}

const std::string & XML::Element::GetName(void) const
{
	return _Name;
}

const XML::Node * XML::Element::GetParent(void) const
{
	return _Parent;
}

const std::map< std::string, std::string > & XML::Element::GetAttributes(void) const
{
	return _Attributes;
}

const std::string & XML::Element::GetAttribute(const std::string & AttributeName) const
{
	return _Attributes.find(AttributeName)->second;
}

bool XML::Element::HasAttribute(const std::string & AttributeName) const
{
	return _Attributes.find(AttributeName) != _Attributes.end();
}

XML::Text::Text(XML::Node * Parent, const std::string & Text) :
	XML::Node{XML::NodeType::Text, Parent},
	_Text{Text}
{
}

const std::string & XML::Text::GetText(void) const
{
	return _Text;
}

XML::Document::Document(std::istream & Stream) :
	XML::Node{XML::NodeType::Document, nullptr},
	_DocumentElement{nullptr}
{
	DOMReader DOMReader{Stream, this, &_DocumentElement};
	
	DOMReader.Parse();
}

XML::Document::~Document(void)
{
	delete _DocumentElement;
	_DocumentElement = nullptr;
}

const XML::Element * XML::Document::GetDocumentElement(void) const
{
	return _DocumentElement;
}
