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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__EQUALS_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__EQUALS_H

#include <memory>

#include "expression.h"

namespace std
{
    class any;
};

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    
    namespace TypeDefinition
    {
        enum class DataType;
        
        class Equals : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Equals>;
        public:
            virtual ~Equals() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            Equals() = default;
            Equals(Inspection::TypeDefinition::Equals const & Equals) = delete;
            Equals(Inspection::TypeDefinition::Equals && Equals) = delete;
            auto operator=(Inspection::TypeDefinition::Equals const & Equals) -> Inspection::TypeDefinition::Equals & = delete;
            auto operator=(Inspection::TypeDefinition::Equals && Equals) -> Inspection::TypeDefinition::Equals & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression1;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression2;
        };
    }
}

#endif
