#ifndef INSPECTION_COMMON_READER_H
#define INSPECTION_COMMON_READER_H

#include <cstdint>
#include <iostream>

#include "length.h"

namespace Inspection
{
	class Buffer;
	
	class Reader
	{
	public:
		enum class BitstreamType
		{
			LeastSignificantBitFirst,
			MostSignificantBitFirst
		};
		
		friend class Inspection::Buffer;
		
		explicit Reader(Inspection::Buffer & Buffer);
		explicit Reader(Inspection::Reader & Reader);
		explicit Reader(Inspection::Buffer & Buffer, const Inspection::Length & Length);
		explicit Reader(Inspection::Reader & Reader, const Inspection::Length & Length);
		explicit Reader(Inspection::Reader & Reader, const Inspection::Length & OffsetInBuffer, const Inspection::Length & Length);
		std::uint8_t Get0Bits(void);
		std::uint8_t Get1Bits(void);
		std::uint8_t Get2Bits(void);
		std::uint8_t Get3Bits(void);
		std::uint8_t Get4Bits(void);
		std::uint8_t Get5Bits(void);
		std::uint8_t Get6Bits(void);
		std::uint8_t Get7Bits(void);
		std::uint8_t Get8Bits(void);
		
		void AdvancePosition(const Inspection::Length & Offset)
		{
			assert(_PositionInBuffer + Offset <= _BoundaryInBuffer);
			_PositionInBuffer += Offset;
		}
		
		void MoveBackPosition(const Inspection::Length & Offset)
		{
			assert(_PositionInBuffer - Offset >= _OffsetInBuffer);
			_PositionInBuffer -= Offset;
		}
		
		const Inspection::Length & GetPositionInBuffer(void) const
		{
			return _PositionInBuffer;
		}
		
		Inspection::Length GetConsumedLength(void) const
		{
			return _PositionInBuffer - _OffsetInBuffer;
		}
		
		Inspection::Length GetRemainingLength(void) const
		{
			return _BoundaryInBuffer - _PositionInBuffer;
		}
		
		Inspection::Length GetCompleteLength(void) const
		{
			return _BoundaryInBuffer - _OffsetInBuffer;
		}
		
		bool Has(const Inspection::Length & Length) const
		{
			return _PositionInBuffer + Length <= _BoundaryInBuffer;
		}
		
		bool HasRemaining(void) const
		{
			return _PositionInBuffer < _BoundaryInBuffer;
		}
		
		bool IsAtEnd(void) const
		{
			return _PositionInBuffer == _BoundaryInBuffer;
		}
		
		void SetPosition(const Inspection::Length & Position)
		{
			_PositionInBuffer = _OffsetInBuffer + Position;
			assert(_PositionInBuffer >= _OffsetInBuffer);
			assert(_PositionInBuffer <= _BoundaryInBuffer);
		}
		
		const Inspection::Buffer & GetBuffer(void) const
		{
			assert(_Buffer != nullptr);
			return *_Buffer;
		}
		
		void SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType)
		{
			assert(_PositionInBuffer.GetBits() == 0);
			_BitstreamType = BitstreamType;
		}
	private:
		Reader(Inspection::Buffer * Buffer, const Inspection::Length & OffsetInBuffer, const Inspection::Length & Length, Inspection::Reader::BitstreamType BitstreamType);
		Inspection::Reader::BitstreamType _BitstreamType;
		Inspection::Length _BoundaryInBuffer;
		Inspection::Buffer * _Buffer;
		Inspection::Length _OffsetInBuffer;
		Inspection::Length _PositionInBuffer;
	};
}

#endif
