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

#ifndef XML_PUNY_DOM_H
#define XML_PUNY_DOM_H

#include <istream>
#include <vector>
#include <map>
#include <string>

namespace XML
{
	enum class NodeType
	{
		Element,
		Document,
		Text
	};
	
	class Document;
	class Element;

	class Node
	{
		friend class XML::Element;
	public:
		Node(XML::NodeType NodeType, XML::Node * Parent);
		virtual ~Node(void);
		const std::vector< XML::Node * > & GetChilds(void) const;
		const XML::Node * GetChild(std::vector< XML::Node * >::size_type Index) const;
		XML::NodeType GetNodeType(void) const;
	protected:
		std::vector< XML::Node * > _Childs;
		XML::NodeType _NodeType;
		XML::Node * _Parent;
	};

	class Element : public XML::Node
	{
	public:
		Element(XML::Node * Parent, const std::string & Name, const std::map< std::string, std::string > & Attributes);
		const XML::Node * GetParent(void) const;
		const std::string & GetName(void) const;
		const std::map< std::string, std::string > & GetAttributes(void) const;
		const std::string & GetAttribute(const std::string & AttributeName) const;
		bool HasAttribute(const std::string & AttributeName) const;
	private:
		std::string _Name;
		std::map< std::string, std::string > _Attributes;
	};

	class Text : public XML::Node
	{
	public:
		Text(XML::Node * Parent, const std::string & Text);
		const std::string & GetText(void) const;
	private:
		std::string _Text;
	};

	class Document : public XML::Node
	{
	public:
		Document(std::istream & Stream);
		~Document(void);
		const XML::Element * GetDocumentElement(void) const;
	private:
		XML::Element * _DocumentElement;
	};
}

#endif
