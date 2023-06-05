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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ADD_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__ADD_H

#include <memory>

#include "expression.h"

namespace std
{
    class any;
};

namespace Inspection
{
    namespace TypeDefinition
    {
        class Add : public Inspection::TypeDefinition::Expression
        {
        public:
            static std::unique_ptr<Inspection::TypeDefinition::Add> Load(const XML::Element * Element);
        public:
            virtual ~Add(void) = default;
            std::any GetAny(Inspection::ExecutionContext & ExecutionContext) const override;
            Inspection::TypeDefinition::DataType GetDataType(void) const override;
        private:
            Add(void) = default;
            Add(const Inspection::TypeDefinition::Add & Add) = delete;
            Add(Inspection::TypeDefinition::Add && Add) = delete;
            Inspection::TypeDefinition::Add & operator=(const Inspection::TypeDefinition::Add & Add) = delete;
            Inspection::TypeDefinition::Add & operator=(Inspection::TypeDefinition::Add && Add) = delete;
        private:
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Summand1;
            std::unique_ptr<Inspection::TypeDefinition::Expression> m_Summand2;
        };
    }
}

#endif
