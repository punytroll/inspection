#include "buffer.h"
#include "read_result.h"
#include "reader.h"

Inspection::Reader::Reader(Inspection::Buffer & Buffer) :
	Inspection::Reader{&Buffer, Inspection::Length{0, 0}, Buffer.GetLength(), Inspection::Reader::BitstreamType::MostSignificantBitFirst}
{
}

Inspection::Reader::Reader(Inspection::Reader & Reader) :
	Inspection::Reader{Reader._Buffer, Reader._ReadPositionInInput, Reader._EndPositionInInput, Reader._BitstreamType}
{
	assert(Reader._ReadPositionInInput <= Reader._EndPositionInInput);
	assert(_EndPositionInInput <= Reader._EndPositionInInput);
}

Inspection::Reader::Reader(Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	Inspection::Reader{&Buffer, StartPositionInInput, StartPositionInInput + Length, Inspection::Reader::BitstreamType::MostSignificantBitFirst}
{
}

Inspection::Reader::Reader(Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
	Inspection::Reader{Reader._Buffer, StartPositionInInput, StartPositionInInput + Length, Reader._BitstreamType}
{
}

Inspection::Reader::Reader(Inspection::Buffer * Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & EndPositionInInput, Inspection::Reader::BitstreamType BitstreamType) :
	_BitstreamType{BitstreamType},
	_Buffer{Buffer},
	_EndPositionInInput{EndPositionInInput},
	_ReadPositionInInput{StartPositionInInput},
	_StartPositionInInput{StartPositionInInput}
{
	assert((Buffer == nullptr) || (_StartPositionInInput <= Buffer->GetLength()));
	assert((Buffer == nullptr) || (_EndPositionInInput <= Buffer->GetLength()));
}

Inspection::Reader::Reader(Inspection::Reader & Reader, const Inspection::Length & Length) :
	Inspection::Reader{Reader._Buffer, Reader._ReadPositionInInput, Reader._ReadPositionInInput + Length, Reader._BitstreamType}
{
	assert(Reader._ReadPositionInInput + Length <= Reader._EndPositionInInput);
	assert(_EndPositionInInput <= Reader._EndPositionInInput);
}

bool Inspection::Reader::Read0Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	ReadResult.InputLength = Inspection::Length{0, 0};
	ReadResult.OutputLength = Inspection::Length{0, 0};
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read1Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 1}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 1};
			ReadResult.OutputLength = Inspection::Length{0, 1};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (7 - _ReadPositionInInput.GetBits())) & 0x01;
			}
			else
			{
				ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x01;
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read2Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 2}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 2};
			ReadResult.OutputLength = Inspection::Length{0, 2};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 7)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (6 - _ReadPositionInInput.GetBits())) & 0x03;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 6)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (14 - _ReadPositionInInput.GetBits()))) & 0x03;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 7)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x03;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x03;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read3Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 3}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 3};
			ReadResult.OutputLength = Inspection::Length{0, 3};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 6)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (5 - _ReadPositionInInput.GetBits())) & 0x07;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 5)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (13 - _ReadPositionInInput.GetBits()))) & 0x07;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 6)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x07;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x07;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read4Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 4}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 4};
			ReadResult.OutputLength = Inspection::Length{0, 4};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 5)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (4 - _ReadPositionInInput.GetBits())) & 0x0f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 4)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (12 - _ReadPositionInInput.GetBits()))) & 0x0f;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 5)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x0f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x0f;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read5Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 5}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 5};
			ReadResult.OutputLength = Inspection::Length{0, 5};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 4)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (3 - _ReadPositionInInput.GetBits())) & 0x1f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 3)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (11 - _ReadPositionInInput.GetBits()))) & 0x1f;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 4)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x1f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x1f;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read6Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 6}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 6};
			ReadResult.OutputLength = Inspection::Length{0, 6};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 3)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (2 - _ReadPositionInInput.GetBits())) & 0x3f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 2)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (10 - _ReadPositionInInput.GetBits()))) & 0x3f;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 3)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x3f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x3f;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read7Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 7}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 7};
			ReadResult.OutputLength = Inspection::Length{0, 7};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() < 2)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> (1 - _ReadPositionInInput.GetBits())) & 0x7f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << (_ReadPositionInInput.GetBits() - 1)) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (9 - _ReadPositionInInput.GetBits()))) & 0x7f;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() < 2)
				{
					ReadResult.Data = (*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) & 0x7f;
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0x7f;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}

bool Inspection::Reader::Read8Bits(Inspection::ReadResult & ReadResult)
{
	ReadResult.Success = _Buffer != nullptr;
	ReadResult.Data = 0;
	if(ReadResult.Success == true)
	{
		if(Has(Inspection::Length{0, 8}) == true)
		{
			ReadResult.InputLength = Inspection::Length{0, 8};
			ReadResult.OutputLength = Inspection::Length{0, 8};
			if(_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
			{
				if(_ReadPositionInInput.GetBits() == 0)
				{
					ReadResult.Data = *(_Buffer->GetData() + _ReadPositionInInput.GetBytes());
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) << _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) >> (8 - _ReadPositionInInput.GetBits()))) & 0xff;
				}
			}
			else
			{
				if(_ReadPositionInInput.GetBits() == 0)
				{
					ReadResult.Data = *(_Buffer->GetData() + _ReadPositionInInput.GetBytes());
				}
				else
				{
					ReadResult.Data = ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes()) >> _ReadPositionInInput.GetBits()) | ((*(_Buffer->GetData() + _ReadPositionInInput.GetBytes() + 1)) << (8 - _ReadPositionInInput.GetBits()))) & 0xff;
				}
			}
			_ReadPositionInInput += ReadResult.InputLength;
		}
		else
		{
			ReadResult.InputLength = _EndPositionInInput - _ReadPositionInInput;
			ReadResult.OutputLength = Inspection::Length{0, 0};
			ReadResult.Success = false;
		}
	}
	else
	{
		ReadResult.InputLength = Inspection::Length{0, 0};
		ReadResult.OutputLength = Inspection::Length{0, 0};
	}
	
	return ReadResult.Success;
}
