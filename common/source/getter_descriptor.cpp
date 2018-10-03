#include <fstream>

#include "getter_descriptor.h"
#include "getter_repository.h"
#include "getters.h"
#include "not_implemented_exception.h"
#include "result.h"
#include "xml_puny_dom.h"

namespace Inspection
{
	enum class AppendType
	{
		Append
	};

	class PartDescriptor
	{
	public:
		std::vector< std::string > GetterModuleParts;
		std::string GetterIdentifier;
		Inspection::AppendType ValueAppendType;
		std::string ValueName;
	};
}

Inspection::GetterDescriptor::~GetterDescriptor(void)
{
	for(auto PartDescriptor : _PartDescriptors)
	{
		delete PartDescriptor;
	}
}

std::unique_ptr< Inspection::Result > Inspection::GetterDescriptor::Get(Inspection::Reader & Reader)
{
	if(_HardcodedGetter != nullptr)
	{
		return _HardcodedGetter(Reader);
	}
	else
	{
		auto Result{Inspection::InitializeResult(Reader)};
		auto Continue{true};
		
		// reading
		for(auto PartDescriptorIndex = 0ul; (Continue == true) && (PartDescriptorIndex < _PartDescriptors.size()); ++PartDescriptorIndex)
		{
			auto PartDescriptor{_PartDescriptors[PartDescriptorIndex]};
			Inspection::Reader PartReader{Reader};
			auto PartResult{g_GetterRepository.Get(PartDescriptor->GetterModuleParts, PartDescriptor->GetterIdentifier, PartReader)};
			
			Continue = PartResult->GetSuccess();
			if(PartDescriptor->ValueAppendType == Inspection::AppendType::Append)
			{
				Result->GetValue()->AppendValue(PartDescriptor->ValueName, PartResult->GetValue());
			}
			Reader.AdvancePosition(PartReader.GetConsumedLength());
		}
		// finalization
		Result->SetSuccess(Continue);
		Inspection::FinalizeResult(Result, Reader);
		
		return Result;
	}
}

void Inspection::GetterDescriptor::LoadGetterDescription(const std::string & GetterPath)
{
	std::ifstream InputFileStream{GetterPath};
	XML::Document Document{InputFileStream};
	auto DocumentElement{Document.GetDocumentElement()};
	
	assert(DocumentElement != nullptr);
	assert(DocumentElement->GetName() == "getter");
	for(auto GetterChildNode : DocumentElement->GetChilds())
	{
		if(GetterChildNode->GetNodeType() == XML::NodeType::Element)
		{
			auto GetterChildElement{dynamic_cast< const XML::Element * >(GetterChildNode)};
			
			if(GetterChildElement->GetName() == "part")
			{
				auto PartDescriptor{new Inspection::PartDescriptor{}};
				
				for(auto PartChildNode : GetterChildElement->GetChilds())
				{
					if(PartChildNode->GetNodeType() == XML::NodeType::Element)
					{
						auto PartChildElement{dynamic_cast< const XML::Element * >(PartChildNode)};
						
						if(PartChildElement->GetName() == "getter")
						{
							for(auto PartGetterChildNode : PartChildElement->GetChilds())
							{
								if(PartGetterChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartGetterChildElement{dynamic_cast< const XML::Element * >(PartGetterChildNode)};
									
									if(PartGetterChildElement->GetName() == "module")
									{
										for(auto PartGetterModuleChildNode : PartGetterChildElement->GetChilds())
										{
											if(PartGetterModuleChildNode->GetNodeType() == XML::NodeType::Element)
											{
												auto PartGetterModuleChildElement{dynamic_cast< const XML::Element * >(PartGetterModuleChildNode)};
												
												if(PartGetterModuleChildElement->GetName() == "part")
												{
													assert(PartGetterModuleChildElement->GetChilds().size() == 1);
													
													auto ModulePartText{dynamic_cast< const XML::Text * >(PartGetterModuleChildElement->GetChild(0))};
													
													assert(ModulePartText != nullptr);
													PartDescriptor->GetterModuleParts.push_back(ModulePartText->GetText());
												}
												else
												{
													throw std::domain_error{PartGetterModuleChildElement->GetName()};
												}
											}
										}
									}
									else if(PartGetterChildElement->GetName() == "identifier")
									{
										assert(PartGetterChildElement->GetChilds().size() == 1);
										
										auto IdentifierText{dynamic_cast< const XML::Text * >(PartGetterChildElement->GetChild(0))};
										
										assert(IdentifierText != nullptr);
										PartDescriptor->GetterIdentifier = IdentifierText->GetText();
									}
									else
									{
										throw std::domain_error{PartGetterChildElement->GetName()};
									}
								}
							}
						}
						else if(PartChildElement->GetName() == "value")
						{
							for(auto PartValueChildNode : PartChildElement->GetChilds())
							{
								if(PartValueChildNode->GetNodeType() == XML::NodeType::Element)
								{
									auto PartValueChildElement{dynamic_cast< const XML::Element * >(PartValueChildNode)};
									
									if(PartValueChildElement->GetName() == "append")
									{
										PartDescriptor->ValueAppendType = Inspection::AppendType::Append;
										for(auto PartValueAppendChildNode : PartValueChildElement->GetChilds())
										{
											if(PartValueAppendChildNode->GetNodeType() == XML::NodeType::Element)
											{
												auto PartValueAppendChildElement{dynamic_cast< const XML::Element * >(PartValueAppendChildNode)};
												
												if(PartValueAppendChildElement->GetName() == "name")
												{
													assert(PartValueAppendChildElement->GetChilds().size() == 1);
													
													auto NameText{dynamic_cast< const XML::Text * >(PartValueAppendChildElement->GetChild(0))};
													
													assert(NameText != nullptr);
													PartDescriptor->ValueName = NameText->GetText();
												}
												else
												{
													throw std::domain_error{PartValueAppendChildElement->GetName()};
												}
											}
										}
									}
									else
									{
										throw std::domain_error{PartValueChildElement->GetName()};
									}
								}
							}
						}
						else
						{
							throw std::domain_error{PartChildElement->GetName()};
						}
					}
				}
				_PartDescriptors.push_back(PartDescriptor);
			}
			else if(GetterChildElement->GetName() == "hardcoded-getter")
			{
				assert(GetterChildElement->GetChilds().size() == 1);
				
				auto HardcodedGetterText{dynamic_cast< const XML::Text * >(GetterChildElement->GetChild(0))};
				
				assert(HardcodedGetterText != nullptr);
				if(HardcodedGetterText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
				{
					_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_CreationDate")
				{
					_HardcodedGetter = Inspection::Get_ASF_CreationDate;
				}
				else if(HardcodedGetterText->GetText() == "Get_ASF_FileProperties_Flags")
				{
					_HardcodedGetter = Inspection::Get_ASF_FileProperties_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
				{
					_HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
				}
				else if(HardcodedGetterText->GetText() == "Get_GUID_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_GUID_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_Frame_Header_Flags")
				{
					_HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Flags;
				}
				else if(HardcodedGetterText->GetText() == "Get_ID3_2_3_Frame_Header_Identifier")
				{
					_HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Identifier;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_32Bit_BigEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_BigEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_32Bit_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_LittleEndian;
				}
				else if(HardcodedGetterText->GetText() == "Get_UnsignedInteger_64Bit_LittleEndian")
				{
					_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_LittleEndian;
				}
				else
				{
					throw std::domain_error{HardcodedGetterText->GetText()};
				}
			}
			else
			{
				throw std::domain_error{GetterChildElement->GetName()};
			}
		}
	}
}
