#ifndef INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__APPLY_CAST_H
#define INSPECTION__SOURCE__COMMON__SOURCE__TYPE_DEFINITION__APPLY_CAST_H

#include "interpretation.h"

namespace XML
{
    class Element;
}

namespace Inspection::TypeDefinition
{
    enum class DataType;
    
    class ApplyCast : public Inspection::TypeDefinition::Interpretation
    {
    public:
        static auto Load(XML::Element const * Element) -> std::unique_ptr<Inspection::TypeDefinition::ApplyCast>;
    public:
        auto Apply(Inspection::ExecutionContext & ExecutionContext, Inspection::Value * Value) const -> bool override;
    private:
        Inspection::TypeDefinition::DataType m_DataType;
    };
}

#endif
