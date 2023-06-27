/**
 * Copyright (C) 2022  Hagen Möbius
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**/

#include <any>

#include <xml_puny_dom/xml_puny_dom.h>

#include "assertion.h"
#include "execution_context.h"
#include "getters.h"
#include "internal_output_operators.h"
#include "result.h"
#include "type.h"
#include "type_definition/interpretation.h"
#include "type_definition/parameters.h"
#include "type_definition/part_type.h"
#include "type_definition/type_reference.h"
#include "type_definition.h"
#include "type_repository.h"
#include "xml_helper.h"

using namespace std::string_literals;

Inspection::TypeDefinition::Type::Type(std::vector<std::string> const & PathParts, Inspection::TypeRepository * TypeRepository) :
    m_PathParts{PathParts},
    m_TypeRepository{TypeRepository}
{
}

Inspection::TypeDefinition::Type::~Type()
{
}

auto Inspection::TypeDefinition::Type::Get(Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto ExecutionContext = Inspection::ExecutionContext{*this, *m_TypeRepository};
    auto Result = Get(ExecutionContext, Reader, Parameters);
    
    ASSERTION(ExecutionContext.GetExecutionStackSize() == 0);
    
    return Result;
}

auto Inspection::TypeDefinition::Type::Get(Inspection::ExecutionContext & ExecutionContext, Inspection::Reader & Reader, std::unordered_map<std::string, std::any> const & Parameters) const -> std::unique_ptr<Inspection::Result>
{
    auto Result = std::make_unique<Inspection::Result>();
    auto Continue = true;
    
    // reading
    if(Continue == true)
    {
        auto TypePart = Inspection::TypeDefinition::TypePart::Create();
        
        ExecutionContext.Push(*TypePart, *Result, Reader, Parameters);
        if(m_HardcodedGetter != nullptr)
        {
            auto HardcodedResult = m_HardcodedGetter(Reader, Parameters);
            
            Continue = HardcodedResult->GetSuccess();
            Result->GetValue()->Extend(HardcodedResult->ExtractValue());
        }
        else if(m_Part != nullptr)
        {
            ASSERTION(m_TypeRepository != nullptr);
            
            auto PartReader = std::unique_ptr<Inspection::Reader>{};
            
            if(m_Part->Length != nullptr)
            {
                PartReader = std::make_unique<Inspection::Reader>(Reader, std::any_cast<Inspection::Length const &>(m_Part->Length->GetAny(ExecutionContext)));
            }
            else
            {
                PartReader = std::make_unique<Inspection::Reader>(Reader);
            }
            
            auto PartParameters = m_Part->GetParameters(ExecutionContext);
            auto PartResult = m_Part->Get(ExecutionContext, *PartReader, PartParameters);
            
            Continue = PartResult->GetSuccess();
            switch(m_Part->GetPartType())
            {
            case Inspection::TypeDefinition::PartType::Alternative:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Array:
                {
                    ASSERTION(m_Part->FieldName.has_value() == true);
                    Result->GetValue()->AppendField(m_Part->FieldName.value(), PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Field:
                {
                    ASSERTION(m_Part->FieldName.has_value() == true);
                    Result->GetValue()->AppendField(m_Part->FieldName.value(), PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Forward:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Select:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Sequence:
                {
                    Result->GetValue()->Extend(PartResult->ExtractValue());
                    
                    break;
                }
            case Inspection::TypeDefinition::PartType::Type:
                {
                    // a type inside a type should be excluded earlier
                    IMPOSSIBLE_CODE_REACHED("m_Part->GetPartType() == " + Inspection::to_string(m_Part->GetPartType()));
                }
            }
            Reader.AdvancePosition(PartReader->GetConsumedLength());
        }
        else
        {
            UNEXPECTED_CASE("m_HardcodedGetter and m_Part are both null");
        }
        ExecutionContext.Pop();
    }
    // finalization
    Result->SetSuccess(Continue);
    
    return Result;
}

auto Inspection::TypeDefinition::Type::GetPathParts(void) const -> std::vector<std::string> const &
{
    return m_PathParts;
}

auto Inspection::TypeDefinition::Type::Load(XML::Element const * Element, std::vector<std::string> const & PathParts) -> std::unique_ptr<Inspection::TypeDefinition::Type>
{
    return Inspection::TypeDefinition::Type::Load(Element, PathParts, nullptr);
}

auto Inspection::TypeDefinition::Type::Load(XML::Element const * Element, std::vector<std::string> const & PathParts, TypeRepository * TypeRepository) -> std::unique_ptr<Inspection::TypeDefinition::Type>
{
    ASSERTION(Element != nullptr);
    
    auto Result = std::unique_ptr<Inspection::TypeDefinition::Type>{new Inspection::TypeDefinition::Type{PathParts, TypeRepository}};
    
    for(auto ChildElement : Element->GetChildElements())
    {
        ASSERTION(ChildElement != nullptr);
        if(ChildElement->GetName() == "hardcoded")
        {
            ASSERTION(ChildElement->GetChildNodes().size() == 1);
            
            auto HardcodedText = dynamic_cast<XML::Text const *>(ChildElement->GetChildNode(0));
            
            ASSERTION(HardcodedText != nullptr);
            if(HardcodedText->GetText() == "Get_Apple_AppleDouble_File")
            {
                Result->m_HardcodedGetter = Inspection::Get_Apple_AppleDouble_File;
            }
            else if(HardcodedText->GetText() == "Get_Array_EndedByNumberOfElements")
            {
                Result->m_HardcodedGetter = Inspection::Get_Array_EndedByNumberOfElements;
            }
            else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumeric_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumeric_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_ASCII_String_AlphaNumericOrSpace_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_ASCII_String_Printable_EndedByTermination")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASCII_String_Printable_EndedByTermination;
            }
            else if(HardcodedText->GetText() == "Get_ASF_ExtendedContentDescription_ContentDescriptor_Data")
            {
                Result->m_HardcodedGetter = Get_ASF_ExtendedContentDescription_ContentDescriptor_Data;
            }
            else if(HardcodedText->GetText() == "Get_ASF_Metadata_DescriptionRecord_Data")
            {
                Result->m_HardcodedGetter = Get_ASF_Metadata_DescriptionRecord_Data;
            }
            else if(HardcodedText->GetText() == "Get_ASF_MetadataLibrary_DescriptionRecord_Data")
            {
                Result->m_HardcodedGetter = Get_ASF_MetadataLibrary_DescriptionRecord_Data;
            }
            else if(HardcodedText->GetText() == "Get_ASF_Object")
            {
                Result->m_HardcodedGetter = Get_ASF_Object;
            }
            else if(HardcodedText->GetText() == "Get_ASF_StreamPropertiesObjectData")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASF_StreamPropertiesObjectData;
            }
            else if(HardcodedText->GetText() == "Get_ASF_GUID")
            {
                Result->m_HardcodedGetter = Inspection::Get_ASF_GUID;
            }
            else if(HardcodedText->GetText() == "Get_BitSet_8Bit_LeastSignificantBitFirst")
            {
                Result->m_HardcodedGetter = Inspection::Get_BitSet_8Bit_LeastSignificantBitFirst;
            }
            else if(HardcodedText->GetText() == "Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte")
            {
                Result->m_HardcodedGetter = Inspection::Get_BitSet_16Bit_BigEndian_LeastSignificantBitFirstPerByte;
            }
            else if(HardcodedText->GetText() == "Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte")
            {
                Result->m_HardcodedGetter = Inspection::Get_BitSet_16Bit_LittleEndian_LeastSignificantBitFirstPerByte;
            }
            else if(HardcodedText->GetText() == "Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte")
            {
                Result->m_HardcodedGetter = Inspection::Get_BitSet_32Bit_LittleEndian_LeastSignificantBitFirstPerByte;
            }
            else if(HardcodedText->GetText() == "Get_Boolean_1Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_Boolean_1Bit;
            }
            else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_Data_Set_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_Data_Set_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_Data_SetOrUnset_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_Data_SetOrUnset_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_Data_Unset_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_Data_Unset_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_Data_Unset_Until8BitAlignment")
            {
                Result->m_HardcodedGetter = Inspection::Get_Data_Unset_Until8BitAlignment;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Frame_Header")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Frame_Header;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_MetaDataBlock")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_MetaDataBlock;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Stream_Header")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Stream_Header;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Subframe_CalculateBitsPerSample")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Subframe_CalculateBitsPerSample;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual_Rice_Partition")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual_Rice_Partition;
            }
            else if(HardcodedText->GetText() == "Get_FLAC_Subframe_Residual_Rice2_Partition")
            {
                Result->m_HardcodedGetter = Inspection::Get_FLAC_Subframe_Residual_Rice2_Partition;
            }
            else if(HardcodedText->GetText() == "Get_GUID_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_GUID_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_ID3_1_Genre")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_1_Genre;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_Tag")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_Tag;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_2_Frame")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_2_Frame;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTermination;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_2_TextStringAccordingToEncoding_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_3_Frame;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_3_Frame_Header_Flags")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_3_Frame_Header_Flags;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTermination;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_3_TextStringAccordingToEncoding_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_4_Frame")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_4_Frame;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTermination;
            }
            else if(HardcodedText->GetText() == "Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_2_4_TextStringAccordingToEncoding_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ID3_ReplayGainAdjustment")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_ReplayGainAdjustment;
            }
            else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit;
            }
            else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_ID3_UnsignedInteger_35Bit_SynchSafe_40Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_ID3_UnsignedInteger_35Bit_SynchSafe_40Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Track")
            {
                Result->m_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Track;
            }
            else if(HardcodedText->GetText() == "Get_IEC_60908_1999_TableOfContents_Tracks")
            {
                Result->m_HardcodedGetter = Inspection::Get_IEC_60908_1999_TableOfContents_Tracks;
            }
            else if(HardcodedText->GetText() == "Get_ISO_639_2_1998_Code")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_639_2_1998_Code;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTermination")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTermination;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UCS_2_String_WithoutByteOrderMark_LittleEndian_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_8_String_EndedByLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByNumberOfCodePoints;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_10646_1_1993_UTF_16LE_String_WithoutByteOrderMark_EndedByTerminationOrLength;
            }
            else if(HardcodedText->GetText() == "Get_ISO_IEC_IEEE_60559_2011_binary32")
            {
                Result->m_HardcodedGetter = Inspection::Get_ISO_IEC_IEEE_60559_2011_binary32;
            }
            else if(HardcodedText->GetText() == "Get_MPEG_1_Frame")
            {
                Result->m_HardcodedGetter = Inspection::Get_MPEG_1_Frame;
            }
            else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_BitRateIndex")
            {
                Result->m_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_BitRateIndex;
            }
            else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_Mode")
            {
                Result->m_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_Mode;
            }
            else if(HardcodedText->GetText() == "Get_MPEG_1_FrameHeader_ModeExtension")
            {
                Result->m_HardcodedGetter = Inspection::Get_MPEG_1_FrameHeader_ModeExtension;
            }
            else if(HardcodedText->GetText() == "Get_Ogg_Page")
            {
                Result->m_HardcodedGetter = Inspection::Get_Ogg_Page;
            }
            else if(HardcodedText->GetText() == "Get_Ogg_Stream")
            {
                Result->m_HardcodedGetter = Inspection::Get_Ogg_Stream;
            }
            else if(HardcodedText->GetText() == "Get_RIFF_Chunk")
            {
                Result->m_HardcodedGetter = Inspection::Get_RIFF_Chunk;
            }
            else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask")
            {
                Result->m_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_ChannelMask;
            }
            else if(HardcodedText->GetText() == "Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat")
            {
                Result->m_HardcodedGetter = Inspection::Get_RIFF_ChunkData_fmt__FormatSpecificFields_Extensible_SubFormat;
            }
            else if(HardcodedText->GetText() == "Get_SignedInteger_8Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_SignedInteger_8Bit;
            }
            else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_SignedInteger_32Bit_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_SignedInteger_32Bit_RiceEncoded")
            {
                Result->m_HardcodedGetter = Inspection::Get_SignedInteger_32Bit_RiceEncoded;
            }
            else if(HardcodedText->GetText() == "Get_SignedInteger_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_SignedInteger_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_String_ASCII_ByTemplate")
            {
                Result->m_HardcodedGetter = Inspection::Get_String_ASCII_ByTemplate;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_1Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_1Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_2Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_2Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_3Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_3Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_4Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_4Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_5Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_5Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_7Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_7Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_8Bit")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_8Bit;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_9Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_9Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_16Bit_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_16Bit_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_20Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_20Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_24Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_24Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_24Bit_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_24Bit_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_32Bit_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_32Bit_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_36Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_36Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_BigEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_64Bit_LittleEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_64Bit_LittleEndian;
            }
            else if(HardcodedText->GetText() == "Get_UnsignedInteger_BigEndian")
            {
                Result->m_HardcodedGetter = Inspection::Get_UnsignedInteger_BigEndian;
            }
            else
            {
                UNEXPECTED_CASE("HardcodedText->GetText() == " + HardcodedText->GetText());
            }
        }
        else if((ChildElement->GetName() == "alternative") || (ChildElement->GetName() == "array") || (ChildElement->GetName() == "sequence") || (ChildElement->GetName() == "field") || (ChildElement->GetName() == "fields") || (ChildElement->GetName() == "forward"))
        {
            ASSERTION(Result->m_Part == nullptr);
            Result->m_Part = Inspection::TypeDefinition::Part::Load(ChildElement);
        }
        else
        {
            UNEXPECTED_CASE("ChildElement->GetName() == " + ChildElement->GetName());
        }
    }
    
    return Result;
}

auto Inspection::TypeDefinition::Type::SetTypeRepository(Inspection::TypeRepository & TypeRepository) -> void
{
    m_TypeRepository = &TypeRepository;
}
