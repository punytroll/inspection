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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__VALUE_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__VALUE_H

#include <memory>
#include <variant>

#include <common/guid.h>

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
        
        class Value : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Value>;
        public:
            virtual ~Value() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            Value() = default;
            Value(Inspection::TypeDefinition::Value const & Value) = delete;
            Value(Inspection::TypeDefinition::Value && Value) = delete;
            auto operator=(Inspection::TypeDefinition::Value const & Value) -> Inspection::TypeDefinition::Value & = delete;
            auto operator=(Inspection::TypeDefinition::Value && Value) -> Inspection::TypeDefinition::Value & = delete;
        private:
            std::variant<bool, Inspection::GUID, float, std::string, std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t> m_Data;
            Inspection::TypeDefinition::DataType m_DataType;
        };
    }
}

#endif
