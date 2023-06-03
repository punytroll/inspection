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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__EXPRESSION_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__EXPRESSION_H

#include <memory>

namespace std
{
    class any;
}

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
        
        class Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Expression>;
            static auto LoadFromWithin(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Expression>;
        public:
            virtual ~Expression() = default;
            virtual auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any = 0;
            virtual auto GetDataType() const -> Inspection::TypeDefinition::DataType = 0;
        protected:
            Expression() = default;
        private:
            Expression(Inspection::TypeDefinition::Expression const & Expression) = delete;
            Expression(Inspection::TypeDefinition::Expression && Expression) = delete;
            auto operator=(Inspection::TypeDefinition::Expression const & Expression) -> Inspection::TypeDefinition::Expression & = delete;
            auto operator=(Inspection::TypeDefinition::Expression && Expression) -> Inspection::TypeDefinition::Expression & = delete;
        };
    }
}

#endif
