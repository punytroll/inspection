#ifndef INSPECTION_COMMON_READER_H
#define INSPECTION_COMMON_READER_H

#include <cstdint>
#include <iostream>
#include <memory>

#include "length.h"
#include "read_result.h"

namespace Inspection
{
	class Buffer;
	class ID3DeUnsynchronizationEagerFilter;
	
	class Reader
	{
	public:
		enum class BitstreamType
		{
			LeastSignificantBitFirst,
			MostSignificantBitFirst
		};
		
		explicit Reader(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length);
		explicit Reader(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter);
		explicit Reader(const Inspection::Reader & Reader);
		explicit Reader(const Inspection::Reader & Reader, const Inspection::Length & Length);
		explicit Reader(const Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length);
		
		void AdvancePosition(const Inspection::Length & Offset);
		const Inspection::Buffer & GetBuffer(void) const;
		const Inspection::Length & GetReadPositionInInput(void) const;
		Inspection::Length GetConsumedLength(void) const;
		Inspection::Length CalculateRemainingOutputLength(void) const;
		Inspection::Length CalculateRemainingInputLength(void) const;
		bool Has(const Inspection::Length & Length) const;
		bool HasRemaining(void) const;
		bool IsAtEnd(void) const;
		void SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType);
		bool Read0Bits(Inspection::ReadResult & ReadResult);
		bool Read1Bits(Inspection::ReadResult & ReadResult);
		bool Read2Bits(Inspection::ReadResult & ReadResult);
		bool Read3Bits(Inspection::ReadResult & ReadResult);
		bool Read4Bits(Inspection::ReadResult & ReadResult);
		bool Read5Bits(Inspection::ReadResult & ReadResult);
		bool Read6Bits(Inspection::ReadResult & ReadResult);
		bool Read7Bits(Inspection::ReadResult & ReadResult);
		bool Read8Bits(Inspection::ReadResult & ReadResult);
	private:
		class BufferCore
		{
		public:
			BufferCore(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & EndPositionInInput, Inspection::Reader::BitstreamType BitstreamType);
			bool Read0Bits(Inspection::ReadResult & ReadResult);
			bool Read1Bits(Inspection::ReadResult & ReadResult);
			bool Read2Bits(Inspection::ReadResult & ReadResult);
			bool Read3Bits(Inspection::ReadResult & ReadResult);
			bool Read4Bits(Inspection::ReadResult & ReadResult);
			bool Read5Bits(Inspection::ReadResult & ReadResult);
			bool Read6Bits(Inspection::ReadResult & ReadResult);
			bool Read7Bits(Inspection::ReadResult & ReadResult);
			bool Read8Bits(Inspection::ReadResult & ReadResult);
			Inspection::Reader::BitstreamType _BitstreamType;
			const Inspection::Buffer & _Buffer;
			Inspection::Length _EndPositionInInput;
			Inspection::Length _ReadPositionInInput;
			Inspection::Length _StartPositionInInput;
		};
		
		class ID3DeUnsynchronizationEagerFilterCore
		{
		public:
			ID3DeUnsynchronizationEagerFilterCore(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter);
			Inspection::ID3DeUnsynchronizationEagerFilter & _ID3DeUnsynchronizationEagerFilter;
			Inspection::Length _ReadPositionInFilterOutput;
			Inspection::Length _EndPositionInFilterOutput;
		};
		
		std::unique_ptr<Inspection::Reader::BufferCore> _BufferCore;
		std::unique_ptr<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore> _ID3DeUnsynchronizationEagerFilterCore;
	};
}

#endif
