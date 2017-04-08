#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <deque>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <tuple>

#include "../common/5th.h"
#include "../common/file_handling.h"

std::tuple< bool, unsigned int, std::string > GetMPEGFrameInfo(uint8_t * Buffer, unsigned int Length)
{
	std::tuple< bool, unsigned int, std::string > Result(false, 0, "");
	
	if(Length >= 4)
	{
		unsigned int Index(0);
		
		if((Buffer[Index] == 0xFF) && ((Buffer[Index + 1] & 0xF0) == 0xF0))
		{
			std::stringstream Stream;
			
			Stream << "MPEG Audio Frame" << std::endl;
			
			unsigned int IDValue((Buffer[Index + 1] & 0x08) >> 3);
			
			switch(IDValue)
			{
				case 1:
				{
					Stream << "  ID: MPEG Audio Version 1 (" << IDValue << ')' << std::endl;
					
					break;
				}
				default:
				{
					std::cerr << "*** ERROR: Invalid ID (" << IDValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int LayerValue((Buffer[Index + 1] & 0x06) >> 1);
			
			switch(LayerValue)
			{
			case 1:
				{
					Stream << "  Layer: Layer III (" << LayerValue << ')' << std::endl;
					
					break;
				}
			case 2:
				{
					Stream << "  Layer: Layer II (" << LayerValue << ')' << std::endl;
					
					break;
				}
			case 3:
				{
					Stream << "  Layer: Layer I (" << LayerValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid Layer (" << LayerValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int ProtectionBitValue((Buffer[Index + 1] & 0x01) >> 0);
			
			switch(ProtectionBitValue)
			{
			case 0:
				{
					Stream << "  Protection: yes (" << ProtectionBitValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					Stream << "  Protection: no (" << ProtectionBitValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid Protection (" << ProtectionBitValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int BitRateIndexValue((Buffer[Index + 2] & 0xF0) >> 4);
			unsigned int BitRate;
			
			switch(LayerValue)
			{
			case 1: // Layer III
				{
					switch(BitRateIndexValue)
					{
					case 0:
						{
							Stream << "  Bit Rate: free format (" << BitRateIndexValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Bit Rate: 32 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 32000;
							
							break;
						}
					case 2:
						{
							Stream << "  Bit Rate: 40 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 40000;
							
							break;
						}
					case 3:
						{
							Stream << "  Bit Rate: 48 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 48000;
							
							break;
						}
					case 4:
						{
							Stream << "  Bit Rate: 56 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 56000;
							
							break;
						}
					case 5:
						{
							Stream << "  Bit Rate: 64 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 64000;
							
							break;
						}
					case 6:
						{
							Stream << "  Bit Rate: 80 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 80000;
							
							break;
						}
					case 7:
						{
							Stream << "  Bit Rate: 96 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 96000;
							
							break;
						}
					case 8:
						{
							Stream << "  Bit Rate: 112 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 112000;
							
							break;
						}
					case 9:
						{
							Stream << "  Bit Rate: 128 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 128000;
							
							break;
						}
					case 10:
						{
							Stream << "  Bit Rate: 160 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 160000;
							
							break;
						}
					case 11:
						{
							Stream << "  Bit Rate: 192 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 192000;
							
							break;
						}
					case 12:
						{
							Stream << "  Bit Rate: 224 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 224000;
							
							break;
						}
					case 13:
						{
							Stream << "  Bit Rate: 256 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 256000;
							
							break;
						}
					case 14:
						{
							Stream << "  Bit Rate: 320 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 320000;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid bit rate (" << BitRateIndexValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			case 2: // Layer II
				{
					switch(BitRateIndexValue)
					{
					case 0:
						{
							Stream << "  Bit Rate: free format (" << BitRateIndexValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Bit Rate: 32 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 32000;
							
							break;
						}
					case 2:
						{
							Stream << "  Bit Rate: 48 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 48000;
							
							break;
						}
					case 3:
						{
							Stream << "  Bit Rate: 56 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 56000;
							
							break;
						}
					case 4:
						{
							Stream << "  Bit Rate: 64 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 64000;
							
							break;
						}
					case 5:
						{
							Stream << "  Bit Rate: 80 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 80000;
							
							break;
						}
					case 6:
						{
							Stream << "  Bit Rate: 96 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 96000;
							
							break;
						}
					case 7:
						{
							Stream << "  Bit Rate: 112 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 112000;
							
							break;
						}
					case 8:
						{
							Stream << "  Bit Rate: 128 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 128000;
							
							break;
						}
					case 9:
						{
							Stream << "  Bit Rate: 160 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 160000;
							
							break;
						}
					case 10:
						{
							Stream << "  Bit Rate: 192 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 192000;
							
							break;
						}
					case 11:
						{
							Stream << "  Bit Rate: 224 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 224000;
							
							break;
						}
					case 12:
						{
							Stream << "  Bit Rate: 256 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 256000;
							
							break;
						}
					case 13:
						{
							Stream << "  Bit Rate: 320 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 320000;
							
							break;
						}
					case 14:
						{
							Stream << "  Bit Rate: 384 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 384000;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid bit rate (" << BitRateIndexValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			case 3: // Layer I
				{
					switch(BitRateIndexValue)
					{
					case 0:
						{
							Stream << "  Bit Rate: free format (" << BitRateIndexValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Bit Rate: 32 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 32000;
							
							break;
						}
					case 2:
						{
							Stream << "  Bit Rate: 64 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 64000;
							
							break;
						}
					case 3:
						{
							Stream << "  Bit Rate: 96 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 96000;
							
							break;
						}
					case 4:
						{
							Stream << "  Bit Rate: 128 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 128000;
							
							break;
						}
					case 5:
						{
							Stream << "  Bit Rate: 160 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 160000;
							
							break;
						}
					case 6:
						{
							Stream << "  Bit Rate: 192 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 192000;
							
							break;
						}
					case 7:
						{
							Stream << "  Bit Rate: 224 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 224000;
							
							break;
						}
					case 8:
						{
							Stream << "  Bit Rate: 256 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 256000;
							
							break;
						}
					case 9:
						{
							Stream << "  Bit Rate: 288 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 288000;
							
							break;
						}
					case 10:
						{
							Stream << "  Bit Rate: 320 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 320000;
							
							break;
						}
					case 11:
						{
							Stream << "  Bit Rate: 352 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 352000;
							
							break;
						}
					case 12:
						{
							Stream << "  Bit Rate: 384 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 384000;
							
							break;
						}
					case 13:
						{
							Stream << "  Bit Rate: 416 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 416000;
							
							break;
						}
					case 14:
						{
							Stream << "  Bit Rate: 448 kbits/s (" << BitRateIndexValue << ')' << std::endl;
							BitRate = 448000;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid bit rate (" << BitRateIndexValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid layer (" << LayerValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int SamplingRateFrequencyValue((Buffer[Index + 2] & 0x0C) >> 2);
			unsigned int SamplingRateFrequency(0);
			
			switch(SamplingRateFrequencyValue)
			{
			case 0:
				{
					Stream << "  Sampling Rate Frequency: 44.1 kHz (" << SamplingRateFrequencyValue << ')' << std::endl;
					SamplingRateFrequency = 44100;
					
					break;
				}
			case 1:
				{
					Stream << "  Sampling Rate Frequency: 48 kHz (" << SamplingRateFrequencyValue << ')' << std::endl;
					SamplingRateFrequency = 48000;
					
					break;
				}
			case 2:
				{
					Stream << "  Sampling Rate Frequency: 32 kHz (" << SamplingRateFrequencyValue << ')' << std::endl;
					SamplingRateFrequency = 32000;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid sampling rate frequency (" << SamplingRateFrequencyValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int PaddingBitValue((Buffer[Index + 2] & 0x02) >> 1);
			
			switch(PaddingBitValue)
			{
			case 0:
				{
					Stream << "  Padding: No, frame contains no extra slot. (" << PaddingBitValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					Stream << "  Padding: Yes, frame contains extra slot. (" << PaddingBitValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid padding value (" << PaddingBitValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int PrivateBitValue((Buffer[Index + 2] & 0x01) >> 0);
			
			Stream << "  Private Bit: " << PrivateBitValue << std::endl;
			
			unsigned int ModeValue((Buffer[Index + 3] & 0xC0) >> 6);
			
			switch(ModeValue)
			{
			case 0:
				{
					Stream << "  Mode: stereo (" << ModeValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					switch(LayerValue)
					{
					case 1: // Layer III
						{
							Stream << "  Mode: joint stereo (intensity stereo and/or m/s stereo) (" << ModeValue << ')' << std::endl;
							
							break;
						}
					case 2: // Layer II
						{
							Stream << "  Mode: joint stereo (intensity stereo) (" << ModeValue << ')' << std::endl;
							
							break;
						}
					case 3: // Layer I
						{
							Stream << "  Mode: joint stereo (intensity stereo) (" << ModeValue << ')' << std::endl;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid layer (" << LayerValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			case 2:
				{
					Stream << "  Mode: dual channel (" << ModeValue << ')' << std::endl;
					
					break;
				}
			case 3:
				{
					Stream << "  Mode: single channel (" << ModeValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid mode (" << ModeValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int ModeExtensionValue((Buffer[Index + 3] & 0x30) >> 4);
			
			switch(LayerValue)
			{
			case 1: // Layer III
				{
					switch(ModeExtensionValue)
					{
					case 0:
						{
							Stream << "  Mode Extension: intensity stereo OFF, m/s stereo OFF (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Mode Extension: intensity stereo ON, m/s stereo OFF (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 2:
						{
							Stream << "  Mode Extension: intensity stereo OFF, m/s stereo ON (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 3:
						{
							Stream << "  Mode Extension: intensity stereo ON, m/s stereo ON (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid mode extension (" << ModeExtensionValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			case 2: // Layer II
				{
					switch(ModeExtensionValue)
					{
					case 0:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 4-31, bound==4 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 8-31, bound==8 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 2:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 12-31, bound==12 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 3:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 16-31, bound==16 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid mode extension (" << ModeExtensionValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			case 3: // Layer I
				{
					switch(ModeExtensionValue)
					{
					case 0:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 4-31, bound==4 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 1:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 8-31, bound==8 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 2:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 12-31, bound==12 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					case 3:
						{
							Stream << "  Mode Extension: intensity stereo in subbands 16-31, bound==16 (" << ModeExtensionValue << ')' << std::endl;
							
							break;
						}
					default:
						{
							std::cerr << "*** ERROR: Invalid mode extension (" << ModeExtensionValue << ") found." << std::endl;
							
							return Result;
						}
					}
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid layer (" << LayerValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int CopyrightValue((Buffer[Index + 3] & 0x08) >> 3);
			
			switch(CopyrightValue)
			{
			case 0:
				{
					Stream << "  Copyright: audio is not copyrighted (" << CopyrightValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					Stream << "  Copyright: audio is copyrighted (" << CopyrightValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid copyright (" << CopyrightValue << ") found." << std::endl;
					
					
					return Result;
				}
			}
			
			unsigned int OriginalValue((Buffer[Index + 3] & 0x04) >> 2);
			
			switch(OriginalValue)
			{
			case 0:
				{
					Stream << "  Original: copied media (" << OriginalValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					Stream << "  Original: original media (" << OriginalValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid original (" << OriginalValue << ") found." << std::endl;
					
					return Result;
				}
			}
			
			unsigned int EmphasisValue((Buffer[Index + 3] & 0x03) >> 0);
			
			switch(EmphasisValue)
			{
			case 0:
				{
					Stream << "  Emphasis: no emphasis (" << EmphasisValue << ')' << std::endl;
					
					break;
				}
			case 1:
				{
					Stream << "  Emphasis: 50/15 microsec. emphasis (" << EmphasisValue << ')' << std::endl;
					
					break;
				}
			case 3:
				{
					Stream << "  Emphasis: CCITT J.7 (" << EmphasisValue << ')' << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Invalid emphasis (" << EmphasisValue << ") found." << std::endl;
					
					return Result;
				}
			}
			std::get< 0 >(Result) = true;
			switch(LayerValue)
			{
			case 1:
			case 2:
				{
					std::get< 1 >(Result) = 144 * BitRate / SamplingRateFrequency + ((PaddingBitValue == 0) ? (0) : (1));
					Stream << "  Frame length in bytes: " << std::get< 1 >(Result) << std::endl;
					
					break;
				}
			case 3:
				{
					std::get< 1 >(Result) = (12 * BitRate / SamplingRateFrequency + ((PaddingBitValue == 0) ? (0) : (1))) * 4;
					Stream << "  Frame length in bytes: " << std::get< 1 >(Result) << std::endl;
					
					break;
				}
			default:
				{
					std::cerr << "*** ERROR: Couldn't calculate frame length because of invalid layer value (" << LayerValue << ")." << std::endl;
					
					break;
				}
			}
			std::get< 2 >(Result) = Stream.str();
		}
	}
	
	return Result;
}

void ReadFile(const std::string & Path)
{
	int FileDescriptor(open(Path.c_str(), O_RDONLY));
	
	if(FileDescriptor == -1)
	{
		std::cerr << "Could not open the file \"" << Path << "\"." << std::endl;
	}
	else
	{
		int FileSize(GetFileSize(Path));
		
		if(FileSize != -1)
		{
			uint8_t * Address((uint8_t *)mmap(NULL, FileSize, PROT_READ, MAP_PRIVATE, FileDescriptor, 0));
			
			if(Address == (uint8_t *)(-1))
			{
				std::cerr << "Could not map the file \"" + Path + "\" into memory." << std::endl;
			}
			else
			{
				int Index(0);
				int NoFrameSince(0);
				
				while(Index < FileSize)
				{
					std::tuple< bool, unsigned int, std::string > MPEGFrameInfo(GetMPEGFrameInfo(Address + Index, FileSize - Index));
					
					if(std::get< 0 >(MPEGFrameInfo) == true)
					{
						if(NoFrameSince != Index)
						{
							std::cout << "No frames between " << NoFrameSince << " and " << (Index - 1) << "." << std::endl;
						}
						std::cout << "Frame at: " << Index << std::endl;
						std::cout << std::get< 2 >(MPEGFrameInfo) << std::endl;
						Index += std::get< 1 >(MPEGFrameInfo);
						NoFrameSince = Index;
					}
					else
					{
						Index += 1;
					}
				}
				if(NoFrameSince != Index)
				{
					std::cout << "No frames between " << NoFrameSince << " and " << (Index - 1) << "." << std::endl;
				}
				
				munmap(Address, FileSize);
			}
		}
		close(FileDescriptor);
	}
}

int main(int argc, char ** argv)
{
	std::cout << "This program is intentionally strict according to ISO/IEC 11172-3!\n" << std::endl;
	
	std::deque< std::string > Paths;
	unsigned int Arguments(argc);
	unsigned int Argument(0);
	
	while(++Argument < Arguments)
	{
		Paths.push_back(argv[Argument]);
	}
	if(Paths.size() == 0)
	{
		std::cerr << "Usage: " << argv[0] << " <paths> ..." << std::endl;

		return 1;
	}
	while(Paths.begin() != Paths.end())
	{
		ReadItem(Paths.front());
		Paths.pop_front();
	}
	
	return 0;
}
