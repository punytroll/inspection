/**
 * Copyright (C) 2006, 2007  Hagen Möbius
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

/**
 * This is version 1.5.2 of the xml parser.
 **/

#ifndef XML_PARSER_H
#define XML_PARSER_H

#include <istream>
#include <map>

class XMLParser
{
public:
	XMLParser(std::istream & InputStream);
	virtual ~XMLParser(void);
	void Parse(void);
	virtual void ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes);
	virtual void ElementEnd(const std::string & TagName);
	virtual void Text(const std::string & Text);
private:
	std::istream & _InputStream;
};

#endif
