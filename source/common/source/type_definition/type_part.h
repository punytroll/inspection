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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TYPE_PART_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TYPE_PART_H

#include <any>
#include <memory>
#include <string>
#include <unordered_map>

#include "part.h"

namespace XML
{
    class Element;
}

namespace Inspection
{
    class ExecutionContext;
    class Reader;
    class Result;
    
    namespace TypeDefinition
    {
        class TypePart : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Create() -> std::unique_ptr<Inspection::TypeDefinition::TypePart>;
        public:
            ~TypePart() override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            TypePart();
            TypePart(Inspection::TypeDefinition::TypePart const & TypePart) = delete;
            TypePart(Inspection::TypeDefinition::TypePart && TypePart) = delete;
            auto operator=(Inspection::TypeDefinition::TypePart const & TypePart) -> Inspection::TypeDefinition::TypePart & = delete;
            auto operator=(Inspection::TypeDefinition::TypePart && TypePart) -> Inspection::TypeDefinition::TypePart & = delete;
        };
    }
}

#endif
