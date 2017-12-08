#ifndef INSPECTION_COMMON_BUFFER_H
#define INSPECTION_COMMON_BUFFER_H

#include <cassert>

#include "length.h"

namespace Inspection
{
	class Buffer
	{
	private:
		class BitstreamReader
		{
		public:
			BitstreamReader(const std::uint8_t * Data, const Inspection::Length & Length, const Inspection::Length & Position) :
				_Data(Data),
				_Length(Length),
				_Position(Position)
			{
			}
			
			virtual ~BitstreamReader(void)
			{
			}
			
			const std::uint8_t * GetData(void) const
			{
				return _Data;
			}
			
			const Inspection::Length & GetLength(void) const
			{
				return _Length;
			}
			
			const Inspection::Length & GetPosition(void) const
			{
				return _Position;
			}
			
			bool Has(const Inspection::Length & Length)
			{
				return _Position + Length <= _Length;
			}
			
			void SetPosition(const Inspection::Length & Position)
			{
				_Position = Position;
			}
			
			virtual std::uint8_t Get0Bits(void) = 0;
			virtual std::uint8_t Get1Bits(void) = 0;
			virtual std::uint8_t Get2Bits(void) = 0;
			virtual std::uint8_t Get3Bits(void) = 0;
			virtual std::uint8_t Get4Bits(void) = 0;
			virtual std::uint8_t Get5Bits(void) = 0;
			virtual std::uint8_t Get6Bits(void) = 0;
			virtual std::uint8_t Get7Bits(void) = 0;
			virtual std::uint8_t Get8Bits(void) = 0;
			
			const std::uint8_t * _Data;
			Inspection::Length _Length;
			Inspection::Length _Position;
		};
		
		class MostSignificantBitFirstBitstreamReader : public BitstreamReader
		{
		public:
			MostSignificantBitFirstBitstreamReader(const std::uint8_t * Data, const Inspection::Length & Length, const Inspection::Length & Position) :
				BitstreamReader(Data, Length, Position)
			{
			}
			
			virtual std::uint8_t Get0Bits(void) override
			{
				std::uint8_t Result{0};
				
				return Result;
			}
			
			virtual std::uint8_t Get1Bits(void) override
			{
				std::uint8_t Result;
				
				Result = (*(_Data + _Position.GetBytes()) >> (7 - _Position.GetBits())) & 0x01;
				_Position += Inspection::Length(0ull, 1);
				
				return Result;
			}
			
			virtual std::uint8_t Get2Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 7)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (6 - _Position.GetBits())) & 0x03;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 6)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (14 - _Position.GetBits()))) & 0x03;
				}
				_Position += Inspection::Length(0ull, 2);
				
				return Result;
			}
			
			virtual std::uint8_t Get3Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 6)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (5 - _Position.GetBits())) & 0x07;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 5)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (13 - _Position.GetBits()))) & 0x07;
				}
				_Position += Inspection::Length(0ull, 3);
				
				return Result;
			}
			
			virtual std::uint8_t Get4Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 5)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (4 - _Position.GetBits())) & 0x0f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 4)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (12 - _Position.GetBits()))) & 0x0f;
				}
				_Position += Inspection::Length(0ull, 4);
				
				return Result;
			}
			
			virtual std::uint8_t Get5Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 4)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (3 - _Position.GetBits())) & 0x1f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 3)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (11 - _Position.GetBits()))) & 0x1f;
				}
				_Position += Inspection::Length(0ull, 5);
				
				return Result;
			}
			
			virtual std::uint8_t Get6Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 3)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (2 - _Position.GetBits())) & 0x3f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 2)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (10 - _Position.GetBits()))) & 0x3f;
				}
				_Position += Inspection::Length(0ull, 6);
				
				return Result;
			}
			
			virtual std::uint8_t Get7Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 2)
				{
					Result = (*(_Data + _Position.GetBytes()) >> (1 - _Position.GetBits())) & 0x7f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << (_Position.GetBits() - 1)) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (9 - _Position.GetBits()))) & 0x7f;
				}
				_Position += Inspection::Length(0ull, 7);
				
				return Result;
			}
			
			virtual std::uint8_t Get8Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() == 0)
				{
					Result = *(_Data + _Position.GetBytes());
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) << _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) >> (8 - _Position.GetBits()))) & 0xff;
				}
				_Position += Inspection::Length(0ull, 8);
				
				return Result;
			}
		};
		
		class LeastSignificantBitFirstBitstreamReader : public BitstreamReader
		{
		public:
			LeastSignificantBitFirstBitstreamReader(const std::uint8_t * Data, const Inspection::Length & Length, const Inspection::Length & Position) :
				BitstreamReader(Data, Length, Position)
			{
			}
			
			virtual std::uint8_t Get0Bits(void) override
			{
				std::uint8_t Result{0};
				
				return Result;
			}
			
			virtual std::uint8_t Get1Bits(void) override
			{
				std::uint8_t Result;
				
				Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x01;
				_Position += Inspection::Length(0ull, 1);
				
				return Result;
			}
			
			virtual std::uint8_t Get2Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 7)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x03;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x03;
				}
				_Position += Inspection::Length(0ull, 2);
				
				return Result;
			}
			
			virtual std::uint8_t Get3Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 6)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x07;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x07;
				}
				_Position += Inspection::Length(0ull, 3);
				
				return Result;
			}
			
			virtual std::uint8_t Get4Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 5)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x0f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x0f;
				}
				_Position += Inspection::Length(0ull, 4);
				
				return Result;
			}
			
			virtual std::uint8_t Get5Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 4)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x1f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x1f;
				}
				_Position += Inspection::Length(0ull, 5);
				
				return Result;
			}
			
			virtual std::uint8_t Get6Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 3)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x3f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x3f;
				}
				_Position += Inspection::Length(0ull, 6);
				
				return Result;
			}
			
			virtual std::uint8_t Get7Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() < 2)
				{
					Result = (*(_Data + _Position.GetBytes()) >> _Position.GetBits()) & 0x7f;
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0x7f;
				}
				_Position += Inspection::Length(0ull, 7);
				
				return Result;
			}
			
			virtual std::uint8_t Get8Bits(void) override
			{
				std::uint8_t Result;
				
				if(_Position.GetBits() == 0)
				{
					Result = *(_Data + _Position.GetBytes());
				}
				else
				{
					Result = ((*(_Data + _Position.GetBytes()) >> _Position.GetBits()) | ((*(_Data + _Position.GetBytes() + 1ull)) << (8 - _Position.GetBits()))) & 0xff;
				}
				_Position += Inspection::Length(0ull, 8);
				
				return Result;
			}
		};
	public:
		enum class BitstreamType
		{
			MostSignificantBitFirst,
			LeastSignificantBitFirst
		};
			
		Buffer(const std::uint8_t * Data, const Inspection::Length & Length) :
			_BitstreamReader(new MostSignificantBitFirstBitstreamReader(Data, Length, Inspection::Length(0ull, 0))),
			_BitstreamType(Inspection::Buffer::BitstreamType::MostSignificantBitFirst)
		{
		}
		
		void SetBitstreamType(Inspection::Buffer::BitstreamType BitstreamType)
		{
			if(BitstreamType != _BitstreamType)
			{
				Inspection::Buffer::BitstreamReader * NewBitstreamReader{nullptr};
				
				if(_BitstreamReader != nullptr)
				{
					assert(_BitstreamReader->GetPosition().GetBits() == 0);
				}
				if(BitstreamType == Inspection::Buffer::BitstreamType::MostSignificantBitFirst)
				{
					NewBitstreamReader = new Inspection::Buffer::MostSignificantBitFirstBitstreamReader(_BitstreamReader->_Data, _BitstreamReader->_Length, _BitstreamReader->_Position);
				}
				else if(BitstreamType == Inspection::Buffer::BitstreamType::LeastSignificantBitFirst)
				{
					NewBitstreamReader = new Inspection::Buffer::LeastSignificantBitFirstBitstreamReader(_BitstreamReader->_Data, _BitstreamReader->_Length, _BitstreamReader->_Position);
				}
				_BitstreamType = BitstreamType;
				delete _BitstreamReader;
				_BitstreamReader = NewBitstreamReader;
			}
		}
		
		std::uint8_t Get0Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 0)) == true);
			
			return _BitstreamReader->Get0Bits();
		}
		
		std::uint8_t Get1Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 1)) == true);
			
			return _BitstreamReader->Get1Bits();
		}
		
		std::uint8_t Get2Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 2)) == true);
			
			return _BitstreamReader->Get2Bits();
		}
		
		std::uint8_t Get3Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 3)) == true);
			
			return _BitstreamReader->Get3Bits();
		}
		
		std::uint8_t Get4Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 4)) == true);
			
			return _BitstreamReader->Get4Bits();
		}
		
		std::uint8_t Get5Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 5)) == true);
			
			return _BitstreamReader->Get5Bits();
		}
		
		std::uint8_t Get6Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 6)) == true);
			
			return _BitstreamReader->Get6Bits();
		}
		
		std::uint8_t Get7Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 7)) == true);
			
			return _BitstreamReader->Get7Bits();
		}
		
		std::uint8_t Get8Bits(void)
		{
			assert(_BitstreamReader != nullptr);
			assert(_BitstreamReader->Has(Inspection::Length(0ull, 8)) == true);
			
			return _BitstreamReader->Get8Bits();
		}
		
		Inspection::Buffer::BitstreamType GetBitstreamType(void) const
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamType;
		}
		
		const std::uint8_t * GetData(void) const
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamReader->GetData();
		}
		
		const Inspection::Length & GetLength(void) const
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamReader->GetLength();
		}
		
		const Inspection::Length & GetPosition(void) const
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamReader->GetPosition();
		}
		
		bool Has(std::uint64_t Bytes, std::uint8_t Bits)
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamReader->Has(Inspection::Length(Bytes, Bits));
		}
		
		bool Has(const Inspection::Length & Length)
		{
			assert(_BitstreamReader != nullptr);
			
			return _BitstreamReader->Has(Length);
		}
		
		void SetPosition(const Inspection::Length & Position)
		{
			assert(_BitstreamReader != nullptr);
			
			_BitstreamReader->SetPosition(Position);
		}
	private:
		BitstreamReader * _BitstreamReader;
		BitstreamType _BitstreamType;
	};
}

#endif
