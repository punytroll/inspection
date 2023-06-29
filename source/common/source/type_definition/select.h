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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__SELECT_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__SELECT_H

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
        class Expression;
        
        class Select : public Inspection::TypeDefinition::Part
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Select>;
        public:
            ~Select() override = default;
            auto Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result> override;
        private:
            class Case
            {
            public:
                std::unique_ptr<Inspection::TypeDefinition::Part> Part;
                std::unique_ptr<Inspection::TypeDefinition::Expression> When;
            };
        protected:
            auto _LoadProperty(XML::Element const * Element) -> void override;
        private:
            Select();
            Select(Inspection::TypeDefinition::Select const & Select) = delete;
            Select(Inspection::TypeDefinition::Select && Select) = delete;
            auto operator=(Inspection::TypeDefinition::Select const & Select) -> Inspection::TypeDefinition::Select & = delete;
            auto operator=(Inspection::TypeDefinition::Select && Select) -> Inspection::TypeDefinition::Select & = delete;
        private:
            std::vector<Inspection::TypeDefinition::Select::Case> m_Cases;
            std::unique_ptr<Inspection::TypeDefinition::Part> m_ElsePart;
        };
    }
}

#endif
