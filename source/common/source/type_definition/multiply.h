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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__MULTIPLY_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__MULTIPLY_H

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
    namespace TypeDefinition
    {
        class Multiply : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Multiply>;
        public:
            virtual ~Multiply() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            Multiply() = default;
            Multiply(Inspection::TypeDefinition::Multiply const & Multiply) = delete;
            Multiply(Inspection::TypeDefinition::Multiply && Multiply) = delete;
            auto operator=(Inspection::TypeDefinition::Multiply const & Multiply) -> Inspection::TypeDefinition::Multiply & = delete;
            auto operator=(Inspection::TypeDefinition::Multiply && Multiply) -> Inspection::TypeDefinition::Multiply & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Multiplier;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Multiplicand;
        };
    }
}

#endif
