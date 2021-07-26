#ifndef INSPECTION_COMMON_TYPE_H
#define INSPECTION_COMMON_TYPE_H

#include <any>
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
		class Add;
		class Cast;
		class Divide;
		class Enumeration;
		class Equals;
		class FieldReference;
		class Interpretation;
		class Length;
		class Parameters;
		class Part;
		class Subtract;
		class Statement;
		class Tag;
		class TypeReference;
		class Value;
	
		class Type
		{
		public:
			Type(const std::vector<std::string> & PathParts, TypeRepository * TypeRepository);
			~Type(void);
			std::unique_ptr<Inspection::Result> Get(Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			const std::vector<std::string> GetPathParts(void) const;
			void Load(std::istream & InputStream);
		private:
			std::unique_ptr<Inspection::Result> _GetAlternative(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Alternative, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			std::unique_ptr<Inspection::Result> _GetArray(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Array, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			std::unique_ptr<Inspection::Result> _GetField(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Field, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			std::unique_ptr<Inspection::Result> _GetFields(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Fields, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			std::unique_ptr<Inspection::Result> _GetForward(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Forward, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			std::unique_ptr<Inspection::Result> _GetSequence(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Part & Sequence, Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters) const;
			void _LoadPart(Inspection::TypeDefinition::Part & Part, const XML::Element * PartElement);
			void _LoadType(Inspection::TypeDefinition::Type & Type, const XML::Element * TypeElement);
			Inspection::EvaluationResult _ApplyInterpretation(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Interpretation & Interpretation, Inspection::Value * Target) const;
			Inspection::EvaluationResult _ApplyEnumeration(Inspection::ExecutionContext & ExecutionContext, const Inspection::TypeDefinition::Enumeration & Enumeration, Inspection::Value * Target) const;
			std::function<std::unique_ptr<Inspection::Result> (Inspection::Reader & Reader, const std::unordered_map<std::string, std::any> & Parameters)> _HardcodedGetter;
			Inspection::TypeDefinition::Part * _Part;
			std::vector<std::string> _PathParts;
			Inspection::TypeRepository * _TypeRepository;
		};
	}
}

#endif
