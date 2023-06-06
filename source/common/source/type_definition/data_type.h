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

#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DATA_TYPE_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__DATA_TYPE_H

namespace Inspection::TypeDefinition
{
    enum class DataType
    {
        Boolean,
        GUID,
        Nothing,
        Length,
        Parameters,
        SinglePrecisionReal,
        String,
        Type,
        UnsignedInteger8Bit,
        UnsignedInteger16Bit,
        UnsignedInteger32Bit,
        UnsignedInteger64Bit
    };
}

#endif