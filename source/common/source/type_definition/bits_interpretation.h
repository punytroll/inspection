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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__BITS_INTERPRETATION_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__BITS_INTERPRETATION_H

#include <memory>

#include "interpretation.h"

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
        enum class DataType;
        enum class DataVerification;
        
        class BitsInterpretation : public Inspection::TypeDefinition::Interpretation
        {
        public:
            static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::BitsInterpretation>;
        public:
            auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Target) const -> bool override;
            auto GetAsDataType() const -> Inspection::TypeDefinition::DataType;
            auto GetBeginIndex(void) const -> std::uint64_t;
            auto GetDataVerification() const -> Inspection::TypeDefinition::DataVerification;
            auto GetInterpretations(void) const -> std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> const &;
            auto GetLength(void) const -> std::uint64_t;
            auto GetName(void) const -> std::string const &;
        private:
            BitsInterpretation(void) = default;
            BitsInterpretation(Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation) = delete;
            BitsInterpretation(Inspection::TypeDefinition::BitsInterpretation && BitsInterpretation) = delete;
            auto operator=(Inspection::TypeDefinition::BitsInterpretation const & BitsInterpretation) -> Inspection::TypeDefinition::BitsInterpretation & = delete;
            auto operator=(Inspection::TypeDefinition::BitsInterpretation && BitsInterpretation) -> Inspection::TypeDefinition::BitsInterpretation & = delete;
        private:
            Inspection::TypeDefinition::DataType m_AsDataType;
            std::uint64_t m_BeginIndex;
            std::vector<std::unique_ptr<Inspection::TypeDefinition::Interpretation>> m_Interpretations;
            std::uint64_t m_Length;
            Inspection::TypeDefinition::DataVerification m_DataVerification;
            std::string m_Name;
        };
    }
}

#endif
