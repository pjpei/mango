/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2021 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <mango/core/string.hpp>
#include <mango/core/exception.hpp>
#include <mango/filesystem/file.hpp>

namespace mango::filesystem
{

    // -----------------------------------------------------------------
    // File
    // -----------------------------------------------------------------

    File::File(const std::string& s)
    {
        // split s into pathname + filename
        size_t n = s.find_last_of("/\\:");
        std::string filename = s.substr(n + 1);
        std::string filepath = s.substr(0, n + 1);

        m_filename = filename;

        // create a internal path
        m_path = std::make_unique<Path>(filepath);

        Mapper& mapper = m_path->getMapper();

        FileMapper* x = mapper;
        if (x)
        {
            VirtualMemory* ptr = x->mmap(mapper.basepath() + m_filename);
            m_memory = std::unique_ptr<VirtualMemory>(ptr);
        }
    }

    File::File(const Path& path, const std::string& s)
    {
        // split s into pathname + filename
        size_t n = s.find_last_of("/\\:");
        std::string filename = s.substr(n + 1);
        std::string filepath = s.substr(0, n + 1);

        m_filename = filename;

        // create a internal path
        m_path = std::make_unique<Path>(path, filepath);

        Mapper& mapper = m_path->getMapper();

        FileMapper* x = mapper;
        if (x)
        {
            VirtualMemory* ptr = x->mmap(mapper.basepath() + m_filename);
            m_memory = std::unique_ptr<VirtualMemory>(ptr);
        }
    }

    File::File(ConstMemory memory, const std::string& extension, const std::string& filename)
    {
        std::string password;

        // create a internal path
        m_path = std::make_unique<Path>(memory, extension, password);

        Mapper& mapper = m_path->getMapper();

        // parse and create mappers
        std::string temp = filename; // parse modifies the filename; discard the unwanted changes
        m_filename = mapper.parse(temp, "");

        // memory map the file
        FileMapper* x = mapper;
        if (x)
        {
            VirtualMemory* ptr = x->mmap(m_filename);
            m_memory = std::unique_ptr<VirtualMemory>(ptr);
        }
    }

    File::~File()
    {
    }

    const Path& File::path() const
    {
        return *m_path;
    }

    const std::string& File::filename() const
    {
        return m_filename;
    }

    const std::string& File::pathname() const
    {
        return m_path->pathname();
    }

    File::operator ConstMemory () const
    {
        return getMemory();
    }

	File::operator const u8* () const
	{
        return getMemory().address;
	}

    const u8* File::data() const
    {
        return getMemory().address;
    }

    u64 File::size() const
    {
        return getMemory().size;
    }

    ConstMemory File::getMemory() const
    {
        return m_memory ? *m_memory : ConstMemory();
    }

} // namespace mango::filesystem
