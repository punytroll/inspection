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
	
	enum class ObjectType
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
		void _LoadInterpretDescriptor(Inspection::InterpretDescriptor & InterpretDescriptor, const XML::Element * InterpretElement);
		void _LoadValueDescriptorFromWithin(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement);
		void _LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ValueElement);
		EvaluationResult _ApplyEnumeration(Inspection::Enumeration * Enumeration, std::shared_ptr< Inspection::Value > Target);
		Inspection::GetterRepository * _GetterRepository;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > _HardcodedGetter;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) > _HardcodedGetterWithParameters;
		std::vector< Inspection::InterpretDescriptor * > _InterpretDescriptors;
		std::vector< Inspection::PartDescriptor * > _PartDescriptors;
		std::vector< std::pair< Inspection::ObjectType, std::uint32_t > > _Objects;
	};
}

#endif
