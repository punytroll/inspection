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

#include "xml_parser.h"
#include "xml_puny_dom.h"

class DOMReader : public XMLParser
{
public:
	DOMReader(std::istream & Stream, Document * Document, Element ** RootElement);
	virtual void ElementStart(const std::string & Name, const std::map< std::string, std::string > & Attributes) override;
	virtual void ElementEnd(const std::string & Name) override;
	virtual void Text(const std::string & Text) override;
private:
	Document * _Document;
	Element ** _RootElement;
	std::stack< Element * > _ElementStack;
};

DOMReader::DOMReader(std::istream & Stream, Document * Document, Element ** RootElement) :
	XMLParser(Stream),
	_Document(Document),
	_RootElement(RootElement)
{
}

Node::Node(Node * Parent) :
	_Parent(Parent)
{
	if(_Parent != nullptr)
	{
		_Parent->_Childs.push_back(this);
	}
}

Node::~Node(void)
{
	// delete it from the parent's child vector
	if(_Parent != nullptr)
	{
		auto Iterator(std::find(_Parent->_Childs.begin(), _Parent->_Childs.end(), this));
		
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

const std::vector< Node * > & Node::GetChilds(void) const
{
	return _Childs;
}

const Node * Node::GetChild(std::vector< Node * >::size_type Index) const
{
	return _Childs[Index];
}

void DOMReader::ElementStart(const std::string & Name, const std::map< std::string, std::string > & Attributes)
{
	if(*_RootElement == nullptr)
	{
		// we've got the root element
		_ElementStack.push(*_RootElement = new Element(_Document, Name, Attributes));
	}
	else
	{
		_ElementStack.push(new Element(_ElementStack.top(), Name, Attributes));
	}
}

void DOMReader::ElementEnd(const std::string & Name)
{
	if(_ElementStack.size() == 0)
	{
		std::cerr << "Got '/" << Name << "' but stack is empty." << std::endl;
		
		throw std::domain_error(Name);
	}
	if(Name != _ElementStack.top()->GetName())
	{
		std::cerr << "Got '/" << Name << "' but expected '/" << _ElementStack.top()->GetName() << "'." << std::endl;
		
		throw std::domain_error(Name);
	}
	_ElementStack.pop();
}

void DOMReader::Text(const std::string & Text)
{
	std::cout << "Got text \"" << Text << "\"." << std::endl;
}

Element::Element(Node * Parent, const std::string & Name, const std::map< std::string, std::string > & Attributes) :
	Node(Parent),
	_Name(Name),
	_Attributes(Attributes)
{
}

const std::string & Element::GetName(void) const
{
	return _Name;
}

const Node * Element::GetParent(void) const
{
	return _Parent;
}

const std::map< std::string, std::string > & Element::GetAttributes(void) const
{
	return _Attributes;
}

const std::string & Element::GetAttribute(const std::string & AttributeName) const
{
	return _Attributes.find(AttributeName)->second;
}

bool Element::HasAttribute(const std::string & AttributeName) const
{
	return _Attributes.find(AttributeName) != _Attributes.end();
}

Document::Document(std::istream & Stream) :
	Node(nullptr),
	_DocumentElement(nullptr)
{
	DOMReader DOMReader(Stream, this, &_DocumentElement);
	
	DOMReader.Parse();
}

Document::~Document(void)
{
	delete _DocumentElement;
	_DocumentElement = nullptr;
}
