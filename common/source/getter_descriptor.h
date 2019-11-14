#ifndef INSPECTION_COMMON_GETTER_DESCRIPTOR_H
#define INSPECTION_COMMON_GETTER_DESCRIPTOR_H

#include <experimental/any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

#include "xml_puny_dom.h"

namespace Inspection
{
	class Enumeration;
	class EvaluationResult;
	class GetterRepository;
	class Interpretation;
	class PartDescriptor;
	class Reader;
	class Result;
	class Tag;
	class Value;
	class ValueDescriptor;
	
	enum class ObjectType
	{
		Interpretation,
		Part
	};
	
	class GetterDescriptor
	{
	public:
		GetterDescriptor(GetterRepository * GetterRepository);
		~GetterDescriptor(void);
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		void LoadGetterDescription(const std::string & GetterPath);
	private:
		void _LoadEnumeration(Inspection::Enumeration & Enumeration, const XML::Element * EnumerationElement);
		void _LoadInterpretation(Inspection::Interpretation & Interpretation, const XML::Element * InterpretElement);
		void _LoadTag(Inspection::Tag & Tag, const XML::Element * TagElement);
		void _LoadValueDescriptorFromWithin(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement);
		void _LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ValueElement);
		Inspection::EvaluationResult _ApplyInterpretation(const Inspection::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		Inspection::EvaluationResult _ApplyEnumeration(const Inspection::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		Inspection::GetterRepository * _GetterRepository;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) > _HardcodedGetter;
		std::vector< Inspection::Interpretation * > _Interpretations;
		std::vector< Inspection::PartDescriptor * > _PartDescriptors;
		std::vector< std::pair< Inspection::ObjectType, std::uint32_t > > _Objects;
	};
}

#endif
