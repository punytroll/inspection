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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DATA_REFERENCE_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DATA_REFERENCE_H

#include <memory>
#include <string>
#include <vector>

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
    namespace TypeDefinition
    {
        class DataReference : public Inspection::TypeDefinition::Expression
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::DataReference>;
        public:
            class Part
            {
            public:
                enum class Type
                {
                    Field,
                    Tag
                };
            public:
                Part(Inspection::TypeDefinition::DataReference::Part::Type Type, std::string_view Name);
            public:
                auto GetName() const -> std::string const &;
                auto GetType() const -> Inspection::TypeDefinition::DataReference::Part::Type;
            private:
                std::string m_Name;
                Inspection::TypeDefinition::DataReference::Part::Type m_Type;
            };
            
            enum class Root
            {
                Current,
                Type
            };
        public:
            virtual ~DataReference() = default;
            auto GetAny(Inspection::ExecutionContext & ExecutionContext) const -> std::any override;
            auto GetDataType() const -> Inspection::TypeDefinition::DataType override;
        private:
            DataReference() = default;
            DataReference(Inspection::TypeDefinition::DataReference const & DataReference) = delete;
            DataReference(Inspection::TypeDefinition::DataReference && DataReference) = delete;
            auto operator=(Inspection::TypeDefinition::DataReference const & DataReference) -> Inspection::TypeDefinition::DataReference & = delete;
            auto operator=(Inspection::TypeDefinition::DataReference && DataReference) -> Inspection::TypeDefinition::DataReference & = delete;
        private:
            std::vector<Inspection::TypeDefinition::DataReference::Part> m_Parts;
            Inspection::TypeDefinition::DataReference::Root m_Root;
        };
    }
}

#endif
