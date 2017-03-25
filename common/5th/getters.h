#ifndef COMMON_5TH_GETTERS_H
#define COMMON_5TH_GETTERS_H

#include "result.h"

namespace Inspection
{
	class Buffer;
	
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Alphabetical_EndedTemplateByLength(Inspection::Buffer & Buffer, const std::string & String);
	std::unique_ptr< Inspection::Result > Get_ASCII_String_Printable_EndedByByteLength(Inspection::Buffer & Buffer, std::uint64_t Length);
	std::unique_ptr< Inspection::Result > Get_BitSet_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_BitSet_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Boolean_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Buffer_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length);
	std::unique_ptr< Inspection::Result > Get_Buffer_Zeroed_UnsignedInteger_8Bit_EndedByLength(Inspection::Buffer & Buffer, std::uint64_t Length);
	std::unique_ptr< Inspection::Result > Get_GUID_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_SignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_1Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_3Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_4Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_5Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_7Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_8Bit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_16Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_20Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_24Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_32Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_36Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_BigEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UnsignedInteger_64Bit_LittleEndian(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UTF8_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UTF8_String_EndedByByteLength(Inspection::Buffer & Buffer, std::uint64_t Length);
	std::unique_ptr< Inspection::Result > Get_UTF16LE_Character(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UTF16LE_CodePoint(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UTF16LE_CodeUnit(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_UTF16LE_String_WithoutByteOrderMark_EndedByTerminationAndNumberOfCodePoints(Inspection::Buffer & Buffer, std::uint64_t NumberOfCodePoints);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserComment(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_UserCommentList(Inspection::Buffer & Buffer);
	std::unique_ptr< Inspection::Result > Get_Vorbis_CommentHeader_WithoutFramingFlag(Inspection::Buffer & Buffer);
}

#endif
