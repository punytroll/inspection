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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TYPE_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__TYPE_H

#include <any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include <type.h>

#include "expression.h"
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
    class TypeRepository;
    
    namespace TypeDefinition
    {
        class Part;
        
        class Type : public Inspection::Type, public Inspection::TypeDefinition::Expression, public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element, std::vector<std::string> const & PathParts) -> std::unique_ptr<Inspection::TypeDefinition::Type>;
        public:
            ~Type() override;
            auto Get(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
            auto GetPathParts() const -> std::vector<std::string> const & override;
        private:
            Type(std::vector<std::string> const & PathParts);
            std::function<std::unique_ptr<Inspection::Result> (Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters)> m_HardcodedGetter;
            std::unique_ptr<Inspection::TypeDefinition::Part> m_Part;
            std::vector<std::string> m_PathParts;
        };
    }
}

#endif
