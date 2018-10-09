#ifndef INSPECTION_COMMON_GETTER_DESCRIPTOR_H
#define INSPECTION_COMMON_GETTER_DESCRIPTOR_H

#include <functional>
#include <memory>
#include <vector>

namespace Inspection
{
	class Enumeration;
	class EvaluationResult;
	class GetterRepository;
	class InterpretDescriptor;
	class PartDescriptor;
	class Reader;
	class Result;
	class Value;
	
	enum class ActionType
	{
		Interpret,
		Read
	};
	
	class GetterDescriptor
	{
	public:
		GetterDescriptor(GetterRepository * GetterRepository);
		~GetterDescriptor(void);
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader);
		void LoadGetterDescription(const std::string & GetterPath);
	private:
		EvaluationResult _ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target);
		Inspection::GetterRepository * _GetterRepository;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
		std::vector< Inspection::InterpretDescriptor * > _InterpretDescriptors;
		std::vector< Inspection::PartDescriptor * > _PartDescriptors;
		std::vector< std::pair< Inspection::ActionType, std::uint32_t > > _Actions;
	};
}

#endif
