#ifndef INSPECTION_COMMON_VALUE_H
#define INSPECTION_COMMON_VALUE_H

#include <algorithm>
#include <any>
#include <list>
#include <memory>
#include <stdexcept>

#include "assertion.h"

namespace Inspection
{
    class Value
    {
    public:
        Value()
        {
        }
        
        auto AppendField(std::unique_ptr<Inspection::Value> Field) -> Inspection::Value *
        {
            auto Result = Field.get();
            
            m_Fields.push_back(std::move(Field));
            
            return Result;
        }
        
        auto AppendField(std::string_view Name, std::unique_ptr<Inspection::Value> Field) -> Inspection::Value *
        {
            Field->SetName(Name);
            
            return AppendField(std::move(Field));
        }
        
        auto AppendField(std::string_view Name) -> Inspection::Value *
        {
            auto Field = std::make_unique<Inspection::Value>();
            
            Field->SetName(Name);
            
            return AppendField(std::move(Field));
        }
        
        template<typename DataType>
        auto AppendField(std::string_view Name, DataType const & Data) -> Inspection::Value *
        {
            auto Result = AppendField(Name);
            
            Result->SetData(Data);
            
            return Result;
        }
        
        /**
         * Extends this Value by moving the fields and tags from the other Value to here.
         * If the name on the other value is set, the name on this value MUST be empty!
         * If the data on the other value is set, the data on this value must be empty!
         **/
        auto Extend(std::unique_ptr<Inspection::Value> Value) -> void
        {
            if(Value->m_Data.has_value() == true)
            {
                ASSERTION(m_Data.has_value() == false);
                m_Data = std::move(Value->m_Data);
            }
            if(Value->m_Name.empty() == false)
            {
                ASSERTION(m_Name.empty() == true);
                m_Name = std::move(Value->m_Name);
            }
            for(auto & Field : Value->m_Fields)
            {
                m_Fields.push_back(std::move(Field));
            }
            for(auto & Tag : Value->m_Tags)
            {
                m_Tags.push_back(std::move(Tag));
            }
        }
        
        auto AddTag(std::unique_ptr<Inspection::Value> Tag) -> Inspection::Value *
        {
            auto Result = Tag.get();
            
            m_Tags.push_back(std::move(Tag));
            
            return Result;
        }
        
        auto AddTag(std::string_view Name) -> Inspection::Value *
        {
            auto Tag = std::make_unique<Inspection::Value>();
            
            Tag->SetName(Name);
            
            return AddTag(std::move(Tag));
        }
        
        template<typename DataType>
        auto AddTag(std::string_view Name, DataType const & Data) -> Inspection::Value *
        {
            auto Tag = AddTag(Name);
            
            Tag->SetData(Data);
            
            return Tag;
        }
        
        auto GetData() const -> std::any const &
        {
            return m_Data;
        }
        
        auto GetFieldCount() const -> std::uint32_t
        {
            return m_Fields.size();
        }
        
        auto GetName() const -> std::string const &
        {
            return m_Name;
        }
        
        auto GetTags() const -> std::list<std::unique_ptr<Inspection::Value>> const &
        {
            return m_Tags;
        }
        
        auto GetTag(std::string_view Name) -> Inspection::Value *
        {
            for(auto & Tag : m_Tags)
            {
                if(Tag->GetName() == Name)
                {
                    return Tag.get();
                }
            }
            
            throw std::invalid_argument("Could not find a tag named \"" + std::string{Name} + "\".");
        }
        
        auto GetField(std::string_view Name) -> Inspection::Value *
        {
            for(auto & Field : m_Fields)
            {
                if(Field->GetName() == Name)
                {
                    return Field.get();
                }
            }
            
            throw std::invalid_argument("Could not find a field named \"" + std::string{Name} + "\".");
        }
        
        auto GetFields() const -> std::list<std::unique_ptr<Inspection::Value>> const &
        {
            return m_Fields;
        }
        
        auto HasTag(std::string_view Name) const -> bool
        {
            return std::find_if(std::begin(m_Tags), std::end(m_Tags), [&Name](auto const & Tag) { return Tag->GetName() == Name; }) != std::end(m_Tags);
        }
        
        auto HasField(std::string_view Name) const -> bool
        {
            return std::find_if(std::begin(m_Fields), std::end(m_Fields), [&Name](auto const & Field) { return Field->GetName() == Name; }) != std::end(m_Fields);
        }
        
        auto SetData(std::any const & Data) -> void
        {
            m_Data = Data;
        }
        
        auto SetName(std::string_view Name) -> void
        {
            m_Name = Name;
        }
        
        auto ClearFields() -> void
        {
            m_Fields.clear();
        }
    private:
        std::any m_Data;
        std::string m_Name;
        std::list<std::unique_ptr<Inspection::Value>> m_Tags;
        std::list<std::unique_ptr<Inspection::Value>> m_Fields;
    };
}

#endif
