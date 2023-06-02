/**
 * Copyright (C) 2023  Hagen Möbius
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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TAG_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TAG_H

#include <memory>
#include <string>

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Value;
    
    namespace TypeDefinition
    {
        class Expression;
        
        class Tag
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Tag>;
        public:
            auto Get(Inspection::ExecutionContext & ExecutionContext) const -> std::unique_ptr<Inspection::Value>;
        private:
            Tag() = default;
            Tag(Inspection::TypeDefinition::Tag const & Tag) = delete;
            Tag(Inspection::TypeDefinition::Tag && Tag) = delete;
            auto operator=(Inspection::TypeDefinition::Tag const & Tag) -> Inspection::TypeDefinition::Tag & = delete;
            auto operator=(Inspection::TypeDefinition::Tag && Tag) -> Inspection::TypeDefinition::Tag & = delete;
        private:
            std::string m_Name;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_ValueExpression;
        };
    }
}

#endif
