#ifndef INSPECTION_COMMON_INSPECTOR_H
#define INSPECTION_COMMON_INSPECTOR_H

#include <cstdint>
#include <deque>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "result.h"

namespace Inspection
{
    class Inspector
    {
    public:
        Inspector();
        virtual ~Inspector(void);
        std::uint_fast32_t GetPathCount(void) const;
        auto GetReadFromStandardInput() -> bool;
        bool Process(void);
        void PushPath(const std::filesystem::path & Path);
        auto SetReadFromStandardInput() -> void;
    protected:
        virtual std::unique_ptr<Inspection::Result> _Getter(const Inspection::Buffer & Buffer) = 0;
        virtual void _Writer(std::unique_ptr< Inspection::Result > & Result);
        void _QueryWriter(Inspection::Value * Value, const std::string & Query);
        Inspection::Value * _AppendOtherData(Inspection::Value * Value, const Inspection::Length & Length);
        Inspection::Value * _AppendLengthField(Inspection::Value * Value, const std::string & FieldName, const Inspection::Length & Length);
    private:
        bool _ProcessPath(const std::filesystem::directory_entry & DirectoryEntry);
        bool _ProcessFile(const std::filesystem::directory_entry & DirectoryEntry);
        auto m_ProcessBuffer(const Inspection::Buffer & Buffer, std::string_view Name) -> bool;
        auto m_ReadBufferFromStandardInput() -> std::vector<std::uint8_t>;
        std::deque<std::filesystem::path> _Paths;
        bool m_ReadFromStandardInput;
    };
}

#endif
