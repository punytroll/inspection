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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ENUMERATION_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ENUMERATION_H

#include <memory>
#include <optional>
#include <string>
#include <vector>

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
        class Tag;
        
        class Enumeration
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::Enumeration>;
        private:
            class Element
            {
            public:
                std::string BaseValue;
                std::vector<std::unique_ptr<Inspection::TypeDefinition::Tag>> Tags;
                bool Valid;
            };
        public:
            auto GetBaseDataType() const -> Inspection::TypeDefinition::DataType;
            auto GetElements() const -> std::vector<Inspection::TypeDefinition::Enumeration::Element> const &;
            auto GetFallbackElement() const -> std::optional<Inspection::TypeDefinition::Enumeration::Element> const &;
        private:
            Enumeration() = default;
            Enumeration(Inspection::TypeDefinition::Enumeration const & Enumeration) = delete;
            Enumeration(Inspection::TypeDefinition::Enumeration && Enumeration) = delete;
            auto operator=(Inspection::TypeDefinition::Enumeration const & Enumeration) -> Inspection::TypeDefinition::Enumeration & = delete;
            auto operator=(Inspection::TypeDefinition::Enumeration && Enumeration) -> Inspection::TypeDefinition::Enumeration & = delete;
        private:
            Inspection::TypeDefinition::DataType m_BaseDataType;
            std::vector<Inspection::TypeDefinition::Enumeration::Element> m_Elements;
            std::optional<Inspection::TypeDefinition::Enumeration::Element> m_FallbackElement;
        };
    }
}

#endif
