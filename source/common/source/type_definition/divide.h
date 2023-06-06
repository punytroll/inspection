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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DIVIDE_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DIVIDE_H

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
        
        class Divide : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Divide>;
        public:
            virtual ~Divide() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            Divide() = default;
            Divide(Inspection::TypeDefinition::Divide const & Divide) = delete;
            Divide(Inspection::TypeDefinition::Divide && Divide) = delete;
            auto operator=(Inspection::TypeDefinition::Divide const & Divide) -> Inspection::TypeDefinition::Divide & = delete;
            auto operator=(Inspection::TypeDefinition::Divide && Divide) -> Inspection::TypeDefinition::Divide & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Dividend;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Divisor;
        };
    }
}

#endif
