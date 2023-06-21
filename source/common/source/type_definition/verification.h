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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__VERIFICATION_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__VERIFICATION_H

#include <memory>

#include "interpretation.h"

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
        
        class Verification : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Verification>;
        public:
            virtual ~Verification() = default;
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
        private:
            Verification() = default;
            Verification(Inspection::TypeDefinition::Verification const & Verification) = delete;
            Verification(Inspection::TypeDefinition::Verification && Verification) = delete;
            Inspection::TypeDefinition::Verification & operator=(Inspection::TypeDefinition::Verification const & Verification) = delete;
            Inspection::TypeDefinition::Verification & operator=(Inspection::TypeDefinition::Verification && Verification) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Expression;
        };
    }
}

#endif
