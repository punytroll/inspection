#ifndef INSPECTION_COMMON_TYPE_H
#define INSPECTION_COMMON_TYPE_H

#include <experimental/any>
#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace XML
{
	class Element;
}

namespace Inspection
{
	class EvaluationResult;
	class ExecutionContext;
	class TypeRepository;
	class Reader;
	class Result;
	class Value;
	
	namespace TypeDefinition
	{
		class Cast;
		class Divide;
		class Enumeration;
		class Equals;
		class Interpretation;
		class Length;
		class Parameter;
		class Parameters;
		class Part;
		class Statement;
		class Tag;
		class TypeReference;
		class Value;
	
		class Type
		{
		public:
			Type(TypeRepository * TypeRepository);
			~Type(void);
			std::unique_ptr< Inspection::Result > Get(Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const;
			void Load(const std::string & TypePath);
		private:
			std::unique_ptr< Inspection::Result > _GetField(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Field, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const;
			std::unique_ptr< Inspection::Result > _GetFields(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Fields, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const;
			std::unique_ptr< Inspection::Result > _GetForward(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Forward, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const;
			std::unique_ptr< Inspection::Result > _GetSequence(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Sequence, Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) const;
			void _LoadCast(Inspection::TypeDefinition::Cast & Cast, const XML::Element * CastElement);
			void _LoadDivide(Inspection::TypeDefinition::Divide & Divide, const XML::Element * DivideElement);
			void _LoadEnumeration(Inspection::TypeDefinition::Enumeration & Enumeration, const XML::Element * EnumerationElement);
			void _LoadEquals(Inspection::TypeDefinition::Equals & Equals, const XML::Element * EqualsElement);
			void _LoadInterpretation(Inspection::TypeDefinition::Interpretation & Interpretation, const XML::Element * InterpretElement);
			void _LoadLength(Inspection::TypeDefinition::Length & Length, const XML::Element * LengthElement);
			void _LoadParameter(Inspection::TypeDefinition::Parameter & Parameter, const XML::Element * ParameterElement);
			void _LoadParameters(Inspection::TypeDefinition::Parameters & Parameters, const XML::Element * ParametersElement);
			void _LoadPart(Inspection::TypeDefinition::Part & Part, const XML::Element * PartElement);
			void _LoadStatement(Inspection::TypeDefinition::Statement & Statement, const XML::Element * StatementElement);
			void _LoadStatementFromWithin(Inspection::TypeDefinition::Statement & Statement, const XML::Element * ParentElement);
			void _LoadTag(Inspection::TypeDefinition::Tag & Tag, const XML::Element * TagElement);
			void _LoadType(Inspection::TypeDefinition::Type & Type, const XML::Element * TypeElement);
			void _LoadTypeReference(Inspection::TypeDefinition::TypeReference & TypeReference, const XML::Element * TypeReferenceElement);
			void _LoadValue(Inspection::TypeDefinition::Value & Value, const XML::Element * ValueElement);
			void _LoadValueFromWithin(Inspection::TypeDefinition::Value & Value, const XML::Element * ParentElement);
			Inspection::EvaluationResult _ApplyInterpretation(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Interpretation & Interpretation, std::shared_ptr< Inspection::Value > Target) const;
			Inspection::EvaluationResult _ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, std::shared_ptr< Inspection::Value > Target) const;
			std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader, const std::unordered_map< std::string, std::experimental::any > & Parameters) > _HardcodedGetter;
			Inspection::TypeDefinition::Part * _Part;
			Inspection::TypeRepository * _TypeRepository;
		};
	}
}

#endif
