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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__CAST_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__CAST_H

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
        
        class Cast : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Cast>;
        public:
            virtual ~Cast() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
            auto GetExpression() const -> Inspection::TypeDefinition::Expression const &;
        private:
            Cast(Inspection::TypeDefinition::DataType DataType);
            Cast(Inspection::TypeDefinition::Cast const & Cast) = delete;
            Cast(Inspection::TypeDefinition::Cast && Cast) = delete;
            auto operator=(Inspection::TypeDefinition::Cast const & Cast) -> Inspection::TypeDefinition::Cast & = delete;
            auto operator=(Inspection::TypeDefinition::Cast && Cast) -> Inspection::TypeDefinition::Cast & = delete;
        private:
            Inspection::TypeDefinition::DataType m_DataType;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
        };
    }
}

#endif
