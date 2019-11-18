#ifndef INSPECTION_COMMON_TYPE_H
#define INSPECTION_COMMON_TYPE_H

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
	class TypeRepository;
	class Interpretation;
	class PartDescriptor;
	class Reader;
	class Result;
	class Tag;
	class Value;
	class ValueDescriptor;
	
	class Type
	{
	public:
		Type(TypeRepository * TypeRepository);
		~Type(void);
		std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		void Load(const std::string & TypePath);
	private:
		void _LoadEnumeration(Inspection::Enumeration & Enumeration, const XML::Element * EnumerationElement);
		void _LoadInterpretation(Inspection::Interpretation & Interpretation, const XML::Element * InterpretElement);
		void _LoadTag(Inspection::Tag & Tag, const XML::Element * TagElement);
		void _LoadValueDescriptorFromWithin(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ParentElement);
		void _LoadValueDescriptor(Inspection::ValueDescriptor & ValueDescriptor, const XML::Element * ValueElement);
		Inspection::EvaluationResult _ApplyInterpretation(const Inspection::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		Inspection::EvaluationResult _ApplyEnumeration(const Inspection::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target, std::shared_ptr< Inspection::Value > CurrentValue, const std::unordered_map< std::string, std::experimental::any > & Parameters);
		Inspection::TypeRepository * _TypeRepository;
		std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) > _HardcodedGetter;
		std::vector< Inspection::PartDescriptor > _PartDescriptors;
	};
}

#endif
