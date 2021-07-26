/**
 * Copyright (C) 2019  Hagen Möbius
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

#ifndef COMMON_XML_HELPER_H
#define COMMON_XML_HELPER_H

#include "xml_puny_dom.h"

namespace XML
{
	inline const XML::Element * GetFirstChildElement(const XML::Element * Element)
	{
		for(auto ChildNode : Element->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				return dynamic_cast<const XML::Element *>(ChildNode);
			}
		}
		
		return nullptr;
	}
	
	inline bool HasOneChildElement(const XML::Element * Element)
	{
		auto Found{false};
		
		for(auto ChildNode : Element->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				if(Found == false)
				{
					Found = true;
				}
				else
				{
					return false;
				}
			}
		}
		
		return Found;
	}
	
	inline bool HasChildElements(const XML::Element * Element)
	{
		for(auto ChildNode : Element->GetChilds())
		{
			if(ChildNode->GetNodeType() == XML::NodeType::Element)
			{
				return true;
			}
		}
		
		return false;
	}
	
	inline bool HasChildNodes(const XML::Element * Element)
	{
		return Element->GetChilds().size() > 0;
	}
}

#endif
