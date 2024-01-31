/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2023 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/buffer.hpp>
#include <mango/core/exception.hpp>

namespace mango
{

    // ----------------------------------------------------------------------------
    // Buffer
    // ----------------------------------------------------------------------------

    Buffer::Buffer()
        : m_memory()
        , m_capacity(0)
    {
    }

    Buffer::Buffer(size_t bytes)
        : m_memory(allocate(bytes), bytes)
        , m_capacity(bytes)
    {
    }

    Buffer::Buffer(size_t bytes, u8 value)
        : m_memory(allocate(bytes), bytes)
        , m_capacity(bytes)
    {
        std::memset(m_memory.address, value, bytes);
    }

    Buffer::Buffer(const void* source, size_t bytes)
        : m_memory(allocate(bytes), bytes)
        , m_capacity(bytes)
    {
        std::memcpy(m_memory.address, source, bytes);
    }

    Buffer::Buffer(ConstMemory memory)
        : m_memory(allocate(memory.size), memory.size)
        , m_capacity(memory.size)
    {
        std::memcpy(m_memory.address, memory.address, memory.size);
    }

    Buffer::Buffer(Stream& stream)
        : m_memory(allocate(size_t(stream.size())), size_t(stream.size()))
        , m_capacity(m_memory.size)
    {
        stream.seek(0, Stream::BEGIN);
        stream.read(m_memory.address, m_memory.size);
    }

    Buffer::~Buffer()
    {
        aligned_free(m_memory.address);
    }

    Buffer::operator ConstMemory () const
    {
        return m_memory;
    }

    Buffer::operator Memory () const
    {
        return m_memory;
    }

    Buffer::operator u8* () const
    {
        return m_memory.address;
    }

    u8* Buffer::data() const
    {
        return m_memory.address;
    }

    size_t Buffer::size() const
    {
        return m_memory.size;
    }

    size_t Buffer::capacity() const
    {
        return m_capacity;
    }

    void Buffer::reset()
    {
        aligned_free(m_memory.address);
        m_memory = Memory();
        m_capacity = 0;
    }

    void Buffer::reset(size_t bytes)
    {
        aligned_free(m_memory.address);
        m_memory = Memory(allocate(bytes), bytes);
        m_capacity = bytes;
    }

    void Buffer::reset(size_t bytes, u8 value)
    {
        reset(bytes);
        std::memset(m_memory.address, value, bytes);
    }

    void Buffer::resize(size_t bytes)
    {
        reserve(bytes);
        m_memory.size = bytes;
    }

    void Buffer::reserve(size_t bytes)
    {
        if (bytes > m_capacity)
        {
            u8* storage = allocate(bytes);
            if (m_memory.address)
            {
                std::memcpy(storage, m_memory.address, m_memory.size);
                aligned_free(m_memory.address);
            }
            m_memory.address = storage;
            m_capacity = bytes;
        }
    }

    u8* Buffer::append(size_t bytes)
    {
        size_t required = m_memory.size + bytes;
        if (required > m_capacity)
        {
            // grow 1.4x times the required capacity
            reserve((required * 7) / 5);
        }

        u8* temp = m_memory.address + m_memory.size;
        m_memory.size += bytes;

        return temp;
    }

    u8* Buffer::append(size_t bytes, u8 value)
    {
        u8* dest = append(bytes);
        std::memset(dest, value, bytes);
        return dest;
    }

    void Buffer::append(const void* source, size_t bytes)
    {
        u8* dest = append(bytes);
        if (source)
        {
            std::memcpy(dest, source, bytes);
        }
    }

    void Buffer::append(ConstMemory memory)
    {
        u8* dest = append(memory.size);
        if (memory.address)
        {
            std::memcpy(dest, memory.address, memory.size);
        }
    }

    Memory Buffer::acquire()
    {
        Memory memory = m_memory;

        m_memory = Memory();
        m_capacity = 0;

        return memory;
    }

    void Buffer::release(Memory memory)
    {
        aligned_free(memory.address);
    }

    u8* Buffer::allocate(size_t bytes) const
    {
        void* p = aligned_malloc(bytes, 64);
        return reinterpret_cast<u8*>(p);
    }

    // ----------------------------------------------------------------------------
    // BufferStream
    // ----------------------------------------------------------------------------

    BufferStream::BufferStream()
        : m_buffer()
        , m_offset(0)
    {
    }

    BufferStream::BufferStream(const u8* source, u64 bytes)
        : m_buffer(source, size_t(bytes))
        , m_offset(bytes)
    {
    }

    BufferStream::BufferStream(ConstMemory memory)
        : m_buffer(memory)
        , m_offset(memory.size)
    {
    }

    BufferStream::~BufferStream()
    {
    }

    BufferStream::operator ConstMemory () const
    {
        return m_buffer;
    }

    BufferStream::operator Memory () const
    {
        return m_buffer;
    }

    BufferStream::operator u8* () const
    {
        return m_buffer;
    }

    u8* BufferStream::data() const
    {
        return m_buffer.data();
    }

    u64 BufferStream::size() const
    {
        return u64(m_buffer.size());
    }

    u64 BufferStream::offset() const
    {
        return m_offset;
    }

    void BufferStream::seek(s64 distance, SeekMode mode)
    {
        const u64 size = m_buffer.size();

        switch (mode)
        {
            case BEGIN:
                m_offset = distance;
                break;

            case CURRENT:
                m_offset += distance;
                break;

            case END:
                m_offset = size + distance;
                break;
        }

        m_offset = std::max(u64(0), m_offset);
    }

    void BufferStream::read(void* dest, u64 bytes)
    {
        const u64 size = m_buffer.size();
        if (m_offset > size || (size - m_offset) < bytes)
        {
            MANGO_EXCEPTION("[BufferStream] Reading past end of buffer.");
        }

        std::memcpy(dest, m_buffer.data() + m_offset, size_t(bytes));
        m_offset += bytes;
    }

    void BufferStream::write(const void* source, u64 bytes)
    {
        const u64 size = m_buffer.size();
        if (m_offset > size)
        {
            // offset is past end of the stream ; write as many zeros as needed
            m_buffer.append(m_offset - size, 0);
        }

        const u64 left = std::min(bytes, m_buffer.size() - m_offset);
        const u64 right = bytes - left;
        std::memcpy(m_buffer.data() + m_offset, source, size_t(left));
        if (right > 0)
        {
            const u8* src = reinterpret_cast<const u8*>(source);
            m_buffer.append(src + left, size_t(right));
        }
        m_offset += bytes;
    }

} // namespace mango
