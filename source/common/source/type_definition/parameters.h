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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__PARAMETERS_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__PARAMETERS_H

#include <memory>
#include <unordered_map>

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
        
        class Parameters : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Parameters>;
        public:
            virtual ~Parameters() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
            auto GetParameters(Inspection::ExecutionContext & ExecutionContext) const -> std::unordered_map<std::string, std::any>;
        private:
            class Parameter
            {
            public:
                std::string Name;
                std::unique_ptr<Inspection::TypeDefinition::Expression> Expression;
            };
        private:
            Parameters() = default;
            Parameters(Inspection::TypeDefinition::Parameters const& Parameters) = delete;
            Parameters(Inspection::TypeDefinition::Parameters && Parameters) = delete;
            auto operator=(Inspection::TypeDefinition::Parameters const & Parameters) -> Inspection::TypeDefinition::Parameters & = delete;
            auto operator=(Inspection::TypeDefinition::Parameters && Parameters) -> Inspection::TypeDefinition::Parameters & = delete;
        private:
            std::vector<Inspection::TypeDefinition::Parameters::Parameter> m_Parameters;
        };
    }
}

#endif
