#ifndef INSPECTION_COMMON_INSPECTOR_H
#define INSPECTION_COMMON_INSPECTOR_H

#include <cstdint>
#include <deque>
#include <filesystem>
#include <unordered_map>

#include "result.h"

namespace Inspection
{
	class Inspector
	{
	public:
		virtual ~Inspector(void);
		std::uint_fast32_t GetPathCount(void) const;
		void Process(void);
		void PushPath(const std::filesystem::path & Path);
	protected:
		virtual std::tuple< Inspection::Length, Inspection::Length > _GetStartAndEnd(const Inspection::Buffer & Buffer, const Inspection::Length & Length);
		virtual std::unique_ptr< Inspection::Result > _Getter(Inspection::Reader & Reader, const std::unordered_map< std::string, std::any > & Parameters) = 0;
		virtual void _Writer(std::unique_ptr< Inspection::Result > & Result);
	private:
		void _ProcessPath(const std::filesystem::directory_entry & DirectoryEntry);
		void _ProcessFile(const std::filesystem::directory_entry & DirectoryEntry);
		std::deque< std::filesystem::path > _Paths;
	};
}

#endif
