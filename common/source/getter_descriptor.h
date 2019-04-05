#ifndef INSPECTION_COMMON_GETTER_DESCRIPTOR_H
#define INSPECTION_COMMON_GETTER_DESCRIPTOR_H

#include <experimental/any>
#include <functional>
#include <memory>
#include <unordered_map>
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
	class ValueDescriptor;
	
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
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		void LoadGetterDescription(const std::string & GetterPath);
	private:
		void _LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement);
		EvaluationResult _ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target);
		Inspection::GetterRepository * _GetterRepository;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) > _HardcodedGetterWithParameters;
		std::vector< Inspection::InterpretDescriptor * > _InterpretDescriptors;
		std::vector< Inspection::PartDescriptor * > _PartDescriptors;
		std::vector< std::pair< Inspection::ActionType, std::uint32_t > > _Actions;
	};
}

#endif
