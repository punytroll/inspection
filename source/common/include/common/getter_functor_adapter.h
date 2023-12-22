/**
 * inspection
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

#ifndef INSPECTION__SOURCE__COMMON__INCLUDE__COMMON__GETTER_FUNCTOR_ADAPTER_H
#define INSPECTION__SOURCE__COMMON__INCLUDE__COMMON__GETTER_FUNCTOR_ADAPTER_H

#include "i_getter_adapter.h"

namespace Inspection
{
    class ExecutionContext;
    
    class GetterFunctorAdapter : public Inspection::IGetterAdapter
    {
    public:
        GetterFunctorAdapter(std::function<void (Inspection::ExecutionContext &)> GetterFunctor) :
            m_GetterFunctor{GetterFunctor}
        {
        }
        
        auto operator()(Inspection::ExecutionContext & ExecutionContext) const -> void override
        {
            m_GetterFunctor(ExecutionContext);
        }
    private:
        std::function<void (Inspection::ExecutionContext &)> m_GetterFunctor;
    };
}

#endif
