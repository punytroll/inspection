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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__SUBTRACT_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__SUBTRACT_H

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
        
        class Subtract : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Subtract>;
        public:
            virtual ~Subtract() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            Subtract() = default;
            Subtract(Inspection::TypeDefinition::Subtract const & Subtract) = delete;
            Subtract(Inspection::TypeDefinition::Subtract && Subtract) = delete;
            auto operator=(Inspection::TypeDefinition::Subtract const & Subtract) -> Inspection::TypeDefinition::Subtract & = delete;
            auto operator=(Inspection::TypeDefinition::Subtract && Subtract) -> Inspection::TypeDefinition::Subtract & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Minuend;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Subtrahend;
        };
    }
}

#endif
