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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__FIELD_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__FIELD_H

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
        class Field : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Field>;
        public:
            ~Field() override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext) const -> void override;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            Field();
            Field(Inspection::TypeDefinition::Field const & Field) = delete;
            Field(Inspection::TypeDefinition::Field && Field) = delete;
            auto operator=(Inspection::TypeDefinition::Field const & Field) -> Inspection::TypeDefinition::Field & = delete;
            auto operator=(Inspection::TypeDefinition::Field && Field) -> Inspection::TypeDefinition::Field & = delete;
        };
    }
}

#endif
