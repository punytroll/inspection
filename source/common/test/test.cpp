#include <cassert>

#include <common/buffer.h>
#include <common/getters.h>
#include <common/result.h>

#include "tests.h"

int main(void)
{
    // Inspection::Reader
    Inspection::Test::ReaderRead2Bits();
    Inspection::Test::ReaderRead3Bits();
    Inspection::Test::ReaderRead6Bits();
    Inspection::Test::ReaderRead7Bits();
    
	// Inspection::Getters
    Inspection::Test::Get_ASCII_Character_Alphabetic();
    Inspection::Test::Get_ASCII_Character_AlphaNumeric();
    Inspection::Test::Get_ASCII_Character_AlphaNumericOrSpace();
    Inspection::Test::Get_ASCII_String_Alphabetic_EndedByLength();
    Inspection::Test::Get_ASCII_String_AlphaNumeric_EndedByLength();
    Inspection::Test::Get_ASCII_String_AlphaNumericOrSpace_EndedByLength();
    Inspection::Test::Get_ASCII_String_Printable_EndedByLength();
    Inspection::Test::Get_ASCII_String_Printable_EndedByTermination();
    Inspection::Test::Get_Buffer_UnsignedInteger_8Bit_EndedByLength();
    Inspection::Test::Get_Buffer_UnsignedInteger_8Bit_Zeroed_EndedByLength();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_Character();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByLength();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTermination();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationOrLength();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLength();
    Inspection::Test::Get_ISO_IEC_8859_1_1998_String_EndedByTerminationUntilLengthOrLength();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_BigEndian();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UCS_2_CodePoint_LittleEndian();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_8_CodePoint();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodeUnit();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16BE_CodePoint();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodeUnit();
    Inspection::Test::Get_ISO_IEC_10646_1_1993_UTF_16LE_CodePoint();
    Inspection::Test::Get_SignedInteger_1Bit();
    Inspection::Test::Get_SignedInteger_5Bit();
    Inspection::Test::Get_SignedInteger_7Bit();
    Inspection::Test::Get_SignedInteger_8Bit();
    Inspection::Test::Get_SignedInteger_9Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_10Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_11Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_12Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_14Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_15Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_16Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_17Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_18Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_19Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_20Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_21Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_22Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_23Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_24Bit_BigEndian();
    Inspection::Test::Get_SignedInteger_25Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_0Bit();
    Inspection::Test::Get_UnsignedInteger_5Bit();
    Inspection::Test::Get_UnsignedInteger_6Bit();
    Inspection::Test::Get_UnsignedInteger_9Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_10Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_11Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_12Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_13Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_14Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_16Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_16Bit_LittleEndian();
    Inspection::Test::Get_UnsignedInteger_18Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_19Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_21Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_22Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_23Bit_BigEndian();
    Inspection::Test::Get_UnsignedInteger_36Bit_BigEndian();
    Inspection::Test::Get_ID3_UnsignedInteger_7Bit_SynchSafe_8Bit();
    Inspection::Test::Get_ID3_UnsignedInteger_28Bit_SynchSafe_32Bit_BigEndian();
	
	std::cout << "All tests successful." << std::endl;
}
