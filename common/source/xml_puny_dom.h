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

class Document;
class Element;

class Node
{
	friend class Element;
public:
	Node(Node * Parent);
	virtual ~Node(void);
	const std::vector< Node * > & GetChilds(void) const;
	const Node * GetChild(std::vector< Node * >::size_type Index) const;
protected:
	std::vector< Node * > _Childs;
	Node * _Parent;
};

class Element : public Node
{
public:
	Element(Node * Parent, const std::string & Name, const std::map< std::string, std::string > & Attributes);
	const Node * GetParent(void) const;
	const std::string & GetName(void) const;
	const std::map< std::string, std::string > & GetAttributes(void) const;
	const std::string & GetAttribute(const std::string & AttributeName) const;
	bool HasAttribute(const std::string & AttributeName) const;
private:
	std::string _Name;
	std::map< std::string, std::string > _Attributes;
};

class Document : public Node
{
public:
	Document(std::istream & Stream);
	~Document(void);
	const Element * GetDocumentElement(void) const;
private:
	Element * _DocumentElement;
};

inline const Element * Document::GetDocumentElement(void) const
{
	return _DocumentElement;
}

#endif
