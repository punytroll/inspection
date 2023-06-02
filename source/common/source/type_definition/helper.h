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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__HELPER_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__HELPER_H

#include <string>

namespace Inspection
{
    class Length;
    class Value;
    
    namespace TypeDefinition
    {
        auto AppendLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName = "length") -> Inspection::Value *;
        auto AppendBitLengthTag(Inspection::Value * Value, Inspection::Length const & Length, std::string const & LengthName = "length") -> Inspection::Value *;
        auto AppendLengthField(Inspection::Value * Value, std::string const & FieldName, Inspection::Length const & Length) -> Inspection::Value *;
        auto AppendOtherData(Inspection::Value * Value, Inspection::Length const & Length) -> Inspection::Value *;
    }
}

#endif
