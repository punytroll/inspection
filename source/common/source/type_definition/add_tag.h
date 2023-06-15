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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ADD_TAG_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ADD_TAG_H

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
        class Tag;
        
        class AddTag : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::AddTag>;
        public:
            virtual ~AddTag() = default;
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
        private:
            AddTag() = default;
            AddTag(Inspection::TypeDefinition::AddTag const & AddTag) = delete;
            AddTag(Inspection::TypeDefinition::AddTag && AddTag) = delete;
            auto operator=(Inspection::TypeDefinition::AddTag const & AddTag) -> Inspection::TypeDefinition::AddTag & = delete;
            auto operator=(Inspection::TypeDefinition::AddTag && AddTag) -> Inspection::TypeDefinition::AddTag & = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Tag> m_Tag;
        };
    }
}

#endif
