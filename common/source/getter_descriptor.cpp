#include <fstream>

#include "getter_descriptor.h"
#include "getters.h"
#include "not_implemented_exception.h"
#include "result.h"
#include "xml_parser.h"

class GetterDescriptionParser : public XMLParser
{
public:
	GetterDescriptionParser(Inspection::GetterDescriptor * GetterDescriptor, std::ifstream & InputFileStream) :
		XMLParser{InputFileStream},
		_GetterDescriptor{GetterDescriptor},
		_InGetter{false},
		_InHardcodedGetter{false}
	{
	}
protected:
	virtual void ElementStart(const std::string & TagName, const std::map< std::string, std::string > & Attributes)
	{
		if(TagName == "getter")
		{
			assert(_InGetter == false);
			assert(_InHardcodedGetter == false);
			_InGetter = true;
		}
		else if(TagName == "hardcoded-getter")
		{
			assert(_InGetter == true);
			assert(_InHardcodedGetter == false);
			_InHardcodedGetter = true;
		}
		else
		{
			assert(false);
		}
	}
	
	virtual void ElementEnd(const std::string & TagName)
	{
		if(TagName == "getter")
		{
			assert(_InGetter == true);
			assert(_InHardcodedGetter == false);
			_InGetter = false;
		}
		else if(TagName == "hardcoded-getter")
		{
			assert(_InGetter == true);
			assert(_InHardcodedGetter == true);
			_InHardcodedGetter = false;
		}
		else
		{
			assert(false);
		}
	}
	
	virtual void Text(const std::string & Text)
	{
		if(_InHardcodedGetter == true)
		{
			assert(_InGetter == true);
			assert(_GetterDescriptor != nullptr);
			if(Text == "Get_ASCII_String_Printable_EndedByTermination")
			{
				_GetterDescriptor->SetHardcodedGetter(Inspection::Get_ASCII_String_Printable_EndedByTermination);
			}
			else if(Text == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
			{
				_GetterDescriptor->SetHardcodedGetter(Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength);
			}
		}
	}
private:
	Inspection::GetterDescriptor * _GetterDescriptor;
	bool _InGetter;
	bool _InHardcodedGetter;
};

void Inspection::GetterDescriptor::LoadGetterDescription(const std::string & GetterPath)
{
	std::ifstream InputFileStream{GetterPath};
	GetterDescriptionParser Parser{this, InputFileStream};
	
	Parser.Parse();
}

void Inspection::GetterDescriptor::SetHardcodedGetter(std::function< std::unique_ptr< Inspection::Result > (Inspection::Reader & Reader) > HardcodedGetter)
{
	_HardcodedGetter = HardcodedGetter;
}

std::unique_ptr< Inspection::Result > Inspection::GetterDescriptor::Get(Inspection::Reader & Reader)
{
	if(_HardcodedGetter != nullptr)
	{
		return _HardcodedGetter(Reader);
	}
	else
	{
		throw Inspection::NotImplementedException{"Only hard coded getters work."};
	}
}
