// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//


#ifndef __FILESO_H_JG2112__
#define __FILESO_H_JG2112__
#if defined(_MSC_VER) && (_MSC_VER>=1020)
#   pragma once
#endif

#include <interfaces/stdtypes.h>
#include <interfaces/constants.h>
#include <string>
#include <boost/cstdint.hpp>


#ifdef JAG_WIN32
# include <core/jstd/win32/fileso.h>
#else
# include <core/jstd/other/fileso.h>
#endif

namespace jag {
namespace jstd {


bool file_exists(Char const*);
bool is_directory(Char const*);
boost::uintmax_t file_size(Char const*);

inline bool is_file(Char const* path) {
    return file_exists(path) && !is_directory(path);
}

/// portable file access implementation
class File
    : private FilePlatformSpecificBase
{
public:
    /// access rights specification (mask)
    enum OpenMode {
        READ =        1UL << 0,    ///< read access
        WRITE =        1UL << 1,    ///< write access
        TRUNCATE =  1UL << 2,    ///< truncate file
        APPEND =    1UL << 3,    ///< move to the end of the file
    };

    /// sharing mode (mask)
    enum Sharing {
        SHARE_NONE =        0,                          ///< no sharing at all
        SHARE_READ =        1L << 0,                    ///< allow sharing for reading
        SHARE_WRITE =        1L << 1,                    ///< allow sharing for writing
        SHARE_RW =            SHARE_READ | SHARE_WRITE    ///< allow sharing for writing/reading
    };

public:
    /// constructor
    File();
    /// destructor - closes the file (if still opened)
    ~File();

public:
    /**
     * @brief creates/opens a file
     *
     * @param file_name path to a file
     * @param open_mode combination of platform::File::OpenMode, allowed combinations are
     *        - READ
     *        - READ | WRITE
     *        - WRITE | TRUNCATE
     *        - READ | WRITE | TRUNCATE
     *        - WRITE | APPEND
     *        - READ | WRITE | APPEND
     * @param creation creation disposition
     *
     */
    void create(Char const*file_name, UInt open_mode, UInt sharing = SHARE_READ);

    /**
     * @brief writes data to the file
     *
     * @param data pointer to data to be written
     * @param num_bytes size of the buffer in bytes
     *
     * @exception io_error if an error occurs
     */
    void write(void const* data, ULong num_bytes) const;

    /**
     * @brief reads data from the file
     *
     * @param data destination buffer
     * @param size number of bytes to read
     * @param read number of bytes actually read (can be NULL)
     *
     * @return true if operation succeeded, false when EOF was reached
     */
    bool read(void* data, ULong size, ULong* nr_read) const;

    /**
     * @brief actual stream position
     *
     * @return actual stream position
     */
    ULong tell() const;

    /**
    * @brief flushes the intermediate buffers to the file
    *
    * @exception io_error if an error occurs
    */
    void flush();

    /**
     * @brief closes the file
     *
     * @exception io_error if an error occurs
     */
    void close();

    /**
     * @sets a file pointer
     *
     * @param offset offset to move
     * @param origin start position for offset
     */
    void seek(Int offset, StreamOffsetOrigin origin);

private:
    std::string              m_name;
};


}} //namespace jag::jstd


#endif //__FILESO_H_JG2112__

