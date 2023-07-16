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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__FUNCTION_CALL_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__FUNCTION_CALL_H

#include <memory>
#include <string>

#include "expression.h"

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
        
        class Parameters;
        
        class FunctionCall : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::FunctionCall>;
        public:
            virtual ~FunctionCall() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            FunctionCall() = default;
            FunctionCall(Inspection::TypeDefinition::FunctionCall const & FunctionCall) = delete;
            FunctionCall(Inspection::TypeDefinition::FunctionCall && FunctionCall) = delete;
            auto operator=(Inspection::TypeDefinition::FunctionCall const & FunctionCall) -> Inspection::TypeDefinition::FunctionCall & = delete;
            auto operator=(Inspection::TypeDefinition::FunctionCall && FunctionCall) -> Inspection::TypeDefinition::FunctionCall & = delete;
        private:
            std::string m_FunctionName;
            std::unique_ptr<Inspection::TypeDefinition::Parameters> m_CallParameters;
        };
    }
}

#endif
