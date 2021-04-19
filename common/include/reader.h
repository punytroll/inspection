#ifndef INSPECTION_COMMON_READER_H
#define INSPECTION_COMMON_READER_H

#include <cstdint>
#include <iostream>

#include "length.h"
#include "read_result.h"

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
		
		explicit Reader(const Inspection::Reader & Reader);
		explicit Reader(const Inspection::Reader & Reader, const Inspection::Length & Length);
		explicit Reader(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length);
		explicit Reader(const Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length);
		bool Read0Bits(Inspection::ReadResult & ReadResult);
		bool Read1Bits(Inspection::ReadResult & ReadResult);
		bool Read2Bits(Inspection::ReadResult & ReadResult);
		bool Read3Bits(Inspection::ReadResult & ReadResult);
		bool Read4Bits(Inspection::ReadResult & ReadResult);
		bool Read5Bits(Inspection::ReadResult & ReadResult);
		bool Read6Bits(Inspection::ReadResult & ReadResult);
		bool Read7Bits(Inspection::ReadResult & ReadResult);
		bool Read8Bits(Inspection::ReadResult & ReadResult);
		
		void AdvancePosition(const Inspection::Length & Offset)
		{
			assert(_ReadPositionInInput + Offset <= _EndPositionInInput);
			_ReadPositionInInput += Offset;
		}
		
		const Inspection::Length & GetReadPositionInInput(void) const
		{
			return _ReadPositionInInput;
		}
		
		Inspection::Length GetConsumedLength(void) const
		{
			return _ReadPositionInInput - _StartPositionInInput;
		}
		
		/**
		 * @brief This function calculates and returns the length of the data, that can still be
		 *     read from the data source, using this reader.
		 * @note This call might be very expensive, if the reader is based on a data source that
		 *     transform its data.
		 **/
		Inspection::Length CalculateRemainingOutputLength(void) const
		{
			return _EndPositionInInput - _ReadPositionInInput;
		}
		
		/**
		 * @brief This function calculates and returns the length of the data, that needs to be
		 *     processed on the base data source, in order to reach the end of this reader.
		 * @note If the result is larger than 0.0 bytes and bits, this is not saying, that
		 *     something can actually be read from the reader. All remaining input data might be
		 *     filtered out when transforming to output data.
		 * @note This call might be very expensive, if the reader is based on a data source that
		 *     transforms its data.
		 **/
		Inspection::Length CalculateRemainingInputLength(void) const
		{
			return _EndPositionInInput - _ReadPositionInInput;
		}
		
		bool Has(const Inspection::Length & Length) const
		{
			return _ReadPositionInInput + Length <= _EndPositionInInput;
		}
		
		/**
		 * This function is valid for filters as well. It returns true, as long
		 * as the read position is not equal to the end position.
		 * Caveat: For generatoring filters (those that may create data out of
		 *   thin air once they reach the end of the data) this may return true
		 *   even if the positions are equal!
		 **/
		bool HasRemaining(void) const
		{
			return _ReadPositionInInput < _EndPositionInInput;
		}
		
		/**
		 * This function is valid for filters as well. It return true, if the
		 * read position equals the end position.
		 **/
		bool IsAtEnd(void) const
		{
			return _ReadPositionInInput == _EndPositionInInput;
		}
		
		void SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType)
		{
			assert(_ReadPositionInInput.GetBits() == 0);
			_BitstreamType = BitstreamType;
		}
	private:
		Reader(const Inspection::Buffer * Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & EndPositionInInput, Inspection::Reader::BitstreamType BitstreamType);
		Inspection::Reader::BitstreamType _BitstreamType;
		const Inspection::Buffer * _Buffer;
		Inspection::Length _EndPositionInInput;
		Inspection::Length _ReadPositionInInput;
		Inspection::Length _StartPositionInInput;
	};
}

#endif
