#include <common/assertion.h>
#include <common/buffer.h>
#include <common/id3_de_unsynchronization_eager_filter.h>
#include <common/output_operators.h>
#include <common/read_result.h>
#include <common/reader.h>

Inspection::Reader::Reader(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length) :
    m_BufferCore{std::make_unique<Inspection::Reader::BufferCore>(Buffer, StartPositionInInput, StartPositionInInput + Length, Inspection::Reader::BitstreamType::MostSignificantBitFirst)}
{
}

Inspection::Reader::Reader(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter) :
    m_ID3DeUnsynchronizationEagerFilterCore{std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(ID3DeUnsynchronizationEagerFilter)}
{
    m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput = Inspection::Length{0, 0};
    m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput = ID3DeUnsynchronizationEagerFilter.GetOutputLength();
    m_ID3DeUnsynchronizationEagerFilterCore->m_ProducedLengthInOutput = Inspection::Length{0, 0};
}

Inspection::Reader::Reader(const Inspection::Reader & Reader)
{
    if(Reader.m_BufferCore != nullptr)
    {
        ASSERTION(Reader.m_BufferCore->m_ReadPositionInBuffer <= Reader.m_BufferCore->m_EndPositionInBuffer);
        m_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader.m_BufferCore->m_Buffer, Reader.m_BufferCore->m_ReadPositionInBuffer, Reader.m_BufferCore->m_EndPositionInBuffer, Reader.m_BufferCore->m_BitstreamType);
        ASSERTION(m_BufferCore->m_StartPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= Reader.m_BufferCore->m_EndPositionInBuffer);
    }
    else if(Reader.m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        m_ID3DeUnsynchronizationEagerFilterCore = std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_ID3DeUnsynchronizationEagerFilter);
        m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput = Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput;
        m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput = Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput;
        m_ID3DeUnsynchronizationEagerFilterCore->m_ProducedLengthInOutput = Inspection::Length{0, 0};
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & Length)
{
    if(Reader.m_BufferCore != nullptr)
    {
        ASSERTION_MESSAGE(Reader.m_BufferCore->m_ReadPositionInBuffer + Length <= Reader.m_BufferCore->m_EndPositionInBuffer, std::format("{} + {} <= {}", Reader.m_BufferCore->m_ReadPositionInBuffer, Length, Reader.m_BufferCore->m_EndPositionInBuffer));
        m_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader.m_BufferCore->m_Buffer, Reader.m_BufferCore->m_ReadPositionInBuffer, Reader.m_BufferCore->m_ReadPositionInBuffer + Length, Reader.m_BufferCore->m_BitstreamType);
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= Reader.m_BufferCore->m_EndPositionInBuffer);
        ASSERTION(m_BufferCore->m_StartPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
    }
    else if(Reader.m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        m_ID3DeUnsynchronizationEagerFilterCore = std::make_unique<Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore>(Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_ID3DeUnsynchronizationEagerFilter);
        m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput = Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput;
        m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput = Reader.m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput + Length;
        m_ID3DeUnsynchronizationEagerFilterCore->m_ProducedLengthInOutput = Inspection::Length{0, 0};
        ASSERTION(m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput <= m_ID3DeUnsynchronizationEagerFilterCore->m_ID3DeUnsynchronizationEagerFilter.GetOutputLength());
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Reader::Reader(const Inspection::Reader & Reader, const Inspection::Length & StartPositionInInput, const Inspection::Length & Length)
{
    if(Reader.m_BufferCore != nullptr)
    {
        ASSERTION(Reader.m_BufferCore->m_ReadPositionInBuffer + Length <= Reader.m_BufferCore->m_EndPositionInBuffer);
        m_BufferCore = std::make_unique<Inspection::Reader::BufferCore>(Reader.m_BufferCore->m_Buffer, StartPositionInInput, StartPositionInInput + Length, Reader.m_BufferCore->m_BitstreamType);
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= Reader.m_BufferCore->m_EndPositionInBuffer);
        ASSERTION(m_BufferCore->m_StartPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
        ASSERTION(m_BufferCore->m_EndPositionInBuffer <= m_BufferCore->m_Buffer.GetLength());
    }
    else if(Reader.m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("creating a new reader from a reader with an ID3DeUnsynchronizationEagerFilterCore, given a start position and a length");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

void Inspection::Reader::AdvancePosition(const Inspection::Length & Offset)
{
    if(m_BufferCore != nullptr)
    {
        ASSERTION(m_BufferCore->m_ReadPositionInBuffer + Offset <= m_BufferCore->m_EndPositionInBuffer);
        m_BufferCore->m_ReadPositionInBuffer += Offset;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        ASSERTION(m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput + Offset <= m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput);
        m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput += Offset;
        m_ID3DeUnsynchronizationEagerFilterCore->m_ProducedLengthInOutput += Offset;
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

const Inspection::Buffer & Inspection::Reader::GetBuffer(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_Buffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("GetBuffer() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

const Inspection::Length & Inspection::Reader::GetReadPositionInInput(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_ReadPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("GetReadPositionInInput() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Length Inspection::Reader::GetConsumedLength(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_ReadPositionInBuffer - m_BufferCore->m_StartPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->m_ProducedLengthInOutput;
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Length Inspection::Reader::CalculateRemainingOutputLength(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_EndPositionInBuffer - m_BufferCore->m_ReadPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput - m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput;
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Length Inspection::Reader::CalculateRemainingInputLength(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_EndPositionInBuffer - m_BufferCore->m_ReadPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("CalculateRemainingInputLength() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Has(const Inspection::Length & Length) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_ReadPositionInBuffer + Length <= m_BufferCore->m_EndPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Has() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::HasRemaining(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_ReadPositionInBuffer < m_BufferCore->m_EndPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput < m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput;
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::IsAtEnd(void) const
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->m_ReadPositionInBuffer == m_BufferCore->m_EndPositionInBuffer;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->m_ReadPositionInFilterOutput == m_ID3DeUnsynchronizationEagerFilterCore->m_EndPositionInFilterOutput;
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

void Inspection::Reader::SetBitstreamType(Inspection::Reader::BitstreamType BitstreamType)
{
    if(m_BufferCore != nullptr)
    {
        ASSERTION(m_BufferCore->m_ReadPositionInBuffer.GetBits() == 0);
        m_BufferCore->m_BitstreamType = BitstreamType;
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("SetBitstreamType() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read0Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read0Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read0Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read1Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read1Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->Read1Bits(ReadResult);
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read2Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read2Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read2Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read3Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read3Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read3Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read4Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read4Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->Read4Bits(ReadResult);
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read5Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read5Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read5Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read6Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read6Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read6Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read7Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read7Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        NOT_IMPLEMENTED("Read7Bits() from an ID3DeUnsynchronizationEagerFilterCore");
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

bool Inspection::Reader::Read8Bits(Inspection::ReadResult & ReadResult)
{
    if(m_BufferCore != nullptr)
    {
        return m_BufferCore->Read8Bits(ReadResult);
    }
    else if(m_ID3DeUnsynchronizationEagerFilterCore != nullptr)
    {
        return m_ID3DeUnsynchronizationEagerFilterCore->Read8Bits(ReadResult);
    }
    else
    {
        UNEXPECTED_CASE("BufferCore AND ID3DeUnsynchronizationEagerFilterCore are both null.");
    }
}

Inspection::Reader::BufferCore::BufferCore(const Inspection::Buffer & Buffer, const Inspection::Length & StartPositionInBuffer, const Inspection::Length & EndPositionInBuffer, Inspection::Reader::BitstreamType BitstreamType) :
    m_BitstreamType{BitstreamType},
    m_Buffer{Buffer},
    m_EndPositionInBuffer{EndPositionInBuffer},
    m_ReadPositionInBuffer{StartPositionInBuffer},
    m_StartPositionInBuffer{StartPositionInBuffer}
{
}

bool Inspection::Reader::BufferCore::Read0Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.Success = true;
    ReadResult.Data = 0;
    ReadResult.RequestedLength = Inspection::Length{0, 0};
    ReadResult.InputLength = Inspection::Length{0, 0};
    ReadResult.OutputLength = Inspection::Length{0, 0};
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read1Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 1};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 1} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 1};
        ReadResult.OutputLength = Inspection::Length{0, 1};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (7 - m_ReadPositionInBuffer.GetBits())) & 0x01;
        }
        else
        {
            ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x01;
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
        
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read2Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 2};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 2} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 2};
        ReadResult.OutputLength = Inspection::Length{0, 2};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 7)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (6 - m_ReadPositionInBuffer.GetBits())) & 0x03;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 6)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (14 - m_ReadPositionInBuffer.GetBits()))) & 0x03;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 7)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x03;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x03;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read3Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 3};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 3} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 3};
        ReadResult.OutputLength = Inspection::Length{0, 3};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 6)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (5 - m_ReadPositionInBuffer.GetBits())) & 0x07;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 5)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (13 - m_ReadPositionInBuffer.GetBits()))) & 0x07;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 6)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x07;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x07;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read4Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 4};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 4} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 4};
        ReadResult.OutputLength = Inspection::Length{0, 4};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 5)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (4 - m_ReadPositionInBuffer.GetBits())) & 0x0f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 4)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (12 - m_ReadPositionInBuffer.GetBits()))) & 0x0f;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 5)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x0f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x0f;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read5Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 5};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 5} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 5};
        ReadResult.OutputLength = Inspection::Length{0, 5};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 4)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (3 - m_ReadPositionInBuffer.GetBits())) & 0x1f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 3)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (11 - m_ReadPositionInBuffer.GetBits()))) & 0x1f;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 4)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x1f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x1f;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read6Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 6};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 6} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 6};
        ReadResult.OutputLength = Inspection::Length{0, 6};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 3)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (2 - m_ReadPositionInBuffer.GetBits())) & 0x3f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 2)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (10 - m_ReadPositionInBuffer.GetBits()))) & 0x3f;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 3)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x3f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x3f;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read7Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 7};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 7} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 7};
        ReadResult.OutputLength = Inspection::Length{0, 7};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() < 2)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> (1 - m_ReadPositionInBuffer.GetBits())) & 0x7f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << (m_ReadPositionInBuffer.GetBits() - 1)) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (9 - m_ReadPositionInBuffer.GetBits()))) & 0x7f;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() < 2)
            {
                ReadResult.Data = (*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) & 0x7f;
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0x7f;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

bool Inspection::Reader::BufferCore::Read8Bits(Inspection::ReadResult & ReadResult)
{
    ReadResult.RequestedLength = Inspection::Length{0, 8};
    if(m_ReadPositionInBuffer + Inspection::Length{0, 8} <= m_EndPositionInBuffer)
    {
        ReadResult.InputLength = Inspection::Length{0, 8};
        ReadResult.OutputLength = Inspection::Length{0, 8};
        if(m_BitstreamType == Inspection::Reader::BitstreamType::MostSignificantBitFirst)
        {
            if(m_ReadPositionInBuffer.GetBits() == 0)
            {
                ReadResult.Data = *(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes());
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) << m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) >> (8 - m_ReadPositionInBuffer.GetBits()))) & 0xff;
            }
        }
        else
        {
            if(m_ReadPositionInBuffer.GetBits() == 0)
            {
                ReadResult.Data = *(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes());
            }
            else
            {
                ReadResult.Data = ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes()) >> m_ReadPositionInBuffer.GetBits()) | ((*(m_Buffer.GetData() + m_ReadPositionInBuffer.GetBytes() + 1)) << (8 - m_ReadPositionInBuffer.GetBits()))) & 0xff;
            }
        }
        ReadResult.Success = true;
        m_ReadPositionInBuffer += ReadResult.InputLength;
    }
    else
    {
        ReadResult.InputLength = m_EndPositionInBuffer - m_ReadPositionInBuffer;
        ReadResult.OutputLength = Inspection::Length{0, 0};
        ReadResult.Data = 0;
        ReadResult.Success = false;
    }
    
    return ReadResult.Success;
}

Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::ID3DeUnsynchronizationEagerFilterCore(Inspection::ID3DeUnsynchronizationEagerFilter & ID3DeUnsynchronizationEagerFilter) :
    m_ID3DeUnsynchronizationEagerFilter{ID3DeUnsynchronizationEagerFilter}
{
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read1Bits(Inspection::ReadResult & ReadResult)
{
    auto Result = m_ID3DeUnsynchronizationEagerFilter.Read1Bits(m_ReadPositionInFilterOutput, ReadResult);
    
    if(Result == true)
    {
        m_ReadPositionInFilterOutput += ReadResult.OutputLength;
        m_ProducedLengthInOutput += ReadResult.OutputLength;
    }
    
    return Result;
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read4Bits(Inspection::ReadResult & ReadResult)
{
    auto Result = m_ID3DeUnsynchronizationEagerFilter.Read4Bits(m_ReadPositionInFilterOutput, ReadResult);
    
    if(Result == true)
    {
        m_ReadPositionInFilterOutput += ReadResult.OutputLength;
        m_ProducedLengthInOutput += ReadResult.OutputLength;
    }
    
    return Result;
}

bool Inspection::Reader::ID3DeUnsynchronizationEagerFilterCore::Read8Bits(Inspection::ReadResult & ReadResult)
{
    auto Result = m_ID3DeUnsynchronizationEagerFilter.Read8Bits(m_ReadPositionInFilterOutput, ReadResult);
    
    if(Result == true)
    {
        m_ReadPositionInFilterOutput += ReadResult.OutputLength;
        m_ProducedLengthInOutput += ReadResult.OutputLength;
    }
    
    return Result;
}
