// Copyright (c) 2005-2009 Jaroslav Gresula
//
// Distributed under the MIT license (See accompanying file
// LICENSE.txt or copy at http://jagpdf.org/LICENSE.txt)
//



#ifndef __SEQSTREAM_H__
#define __SEQSTREAM_H__

#include <interfaces/stdtypes.h>
#include <core/generic/noncopyable.h>
#include <interfaces/constants.h>

namespace jag
{

/**
 * @interface ISeqStreamOutput
 *
 * @brief sequential output stream abstraction
 */
class ISeqStreamOutput
    : public noncopyable
{
public:
    /**
     * @brief writes data
     *
     * @param data pointer to a buffer containing data
     * @param size size of the buffer in bytes
     *
     * @exception io_error when an error occurs
     */
    virtual void write(void const* data, ULong size) = 0;

    /**
    * @brief actual stream position
    *
    * @return actual stream position
    */
    virtual ULong tell() const = 0;

    /**
     * @brief closes the stream
     *
     * @exception io_error when an error occurs
     */
    virtual void flush() = 0;

public:
    virtual ~ISeqStreamOutput() {}
};


/**
 * @brief Allows close operation on ISeqStreamOutput
 */
class ISeqStreamOutputControl : public ISeqStreamOutput
{
public:
    virtual void close() = 0;

    virtual ~ISeqStreamOutputControl() {}
};

/**
* @interface ISeqStreamInput
*
* @brief sequential input stream abstraction
*/
class ISeqStreamInput
    : public noncopyable
{
public:
    /**
    * @brief Reads data from the stream.
    *
    * @param data buffer for read data
    * @param size size of the buffer in bytes
    * @param read number of bytes read, can be NULL
    *
    * @return true if operation succeeded, false if the end of the stream
    *              was reached, the data are still read up to the EOF
    * @exception io_error when an error occurs
    */
    virtual bool read(void* data, ULong size, ULong* nr_read=0) = 0;

    /**
    * @brief Actual stream position.
    *
    * @return actual stream position
    */
    virtual ULong tell() const = 0;

    virtual ~ISeqStreamInput() {}
};


//////////////////////////////////////////////////////////////////////////
class IStreamInput
    : public ISeqStreamInput
{
public:
    virtual void seek(Int offset, StreamOffsetOrigin origin) = 0;

    virtual ~IStreamInput() {}
};


} //namespace jag

#endif //__SEQSTREAM_H__
