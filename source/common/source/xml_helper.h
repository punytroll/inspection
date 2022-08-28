/**
 * Copyright (C) 2019-2022  Hagen Möbius
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

#ifndef INSPECTION__SOURCE_COMMON_SOURCE_XML_HELPER_H
#define INSPECTION__SOURCE_COMMON_SOURCE_XML_HELPER_H

#include <xml_puny_dom/xml_puny_dom.h>

namespace XML
{
    /**
     * @brief Retrieves the first child element, ignoring other child nodes.
     * @note This function is NOT implemented in terms of GetChildElements() but GetChildNodes(),
     *       so as to avoid building the temporary child element vector.
     **/
    inline auto GetFirstChildElement(XML::Element const * Element) -> XML::Element const *
    {
        for(auto ChildNode : Element->GetChildNodes())
        {
            if(ChildNode->GetNodeType() == XML::NodeType::Element)
            {
                return dynamic_cast<XML::Element const *>(ChildNode);
            }
        }
        
        return nullptr;
    }
    
    /**
     * @brief Checks, whether the given XML element has exactly one child element, ignoring other
     *        child nodes.
     * @note This function is NOT implemented in terms of GetChildElements() but GetChildNodes(),
     *       so as to avoid building the temporary child element vector.
     **/
    inline auto HasOneChildElement(XML::Element const * Element) -> bool
    {
        auto Found = false;
        
        for(auto ChildNode : Element->GetChildNodes())
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
    
    /**
     * @brief Checks, whether the given XML element contains any child elements, ignoring other
     *        child nodes.
     * @note This function is NOT implemented in terms of GetChildElements() but GetChildNodes(),
     *       so as to avoid building the temporary child element vector.
     **/
    inline auto HasChildElements(XML::Element const * Element) -> bool
    {
        for(auto ChildNode : Element->GetChildNodes())
        {
            if(ChildNode->GetNodeType() == XML::NodeType::Element)
            {
                return true;
            }
        }
        
        return false;
    }
    
    /**
     * @brief Checks, whether the given XML element contains any child nodes.
     **/
    inline auto HasChildNodes(XML::Element const * Element) -> bool
    {
        return Element->GetChildNodes().empty() == false;
    }
}

#endif
