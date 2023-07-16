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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__APPLY_ENUMERATION_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__APPLY_ENUMERATION_H

#include <memory>

#include "interpretation.h"

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    
    namespace TypeDefinition
    {
        class Enumeration;
        
        class ApplyEnumeration : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::ApplyEnumeration>;
        public:
            virtual ~ApplyEnumeration() = default;
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
        private:
            ApplyEnumeration();
            ApplyEnumeration(Inspection::TypeDefinition::ApplyEnumeration const & ApplyEnumeration) = delete;
            ApplyEnumeration(Inspection::TypeDefinition::ApplyEnumeration && ApplyEnumeration) = delete;
            auto operator=(Inspection::TypeDefinition::ApplyEnumeration const & ApplyEnumeration) -> Inspection::TypeDefinition::ApplyEnumeration & = delete;
            auto operator=(Inspection::TypeDefinition::ApplyEnumeration && ApplyEnumeration) -> Inspection::TypeDefinition::ApplyEnumeration & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Enumeration> m_Enumeration;
        };
    }
}

#endif
